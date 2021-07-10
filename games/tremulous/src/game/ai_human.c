/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2000-2006 Tim Angus

This file is part of Tremulous.

Tremulous is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Tremulous is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Tremulous; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "g_local.h"
#include "ai_main.h"
#include "ai_chat.h"

typedef enum{
	HS_NONE,
	HS_SPAWN,
	HS_GEAR,
	HS_RUSH,
	HS_MOVE,
	HS_KILL
}hstates_t;
char *stateNames[ ] =
{
"HS_NONE",
"HS_SPAWN",
"HS_GEAR",
"HS_RUSH",
"HS_MOVE",
"HS_KILL"
};
/*
==================
BotAimAtEnemy
==================
*/
void HBotAimAtEnemy(bot_state_t *bs, int enemy) {
	int i, enemyvisible;
	float dist, /*f,*/ aim_skill, aim_accuracy, speed, reactiontime;
	vec3_t dir, bestorigin, end, start, groundtarget, cmdmove, enemyvelocity;
	vec3_t mins = {-4,-4,-4}, maxs = {4, 4, 4};
	weaponinfo_t wi;
	aas_entityinfo_t entinfo;
	//bot_goal_t goal;
	bsp_trace_t trace;
	vec3_t target;

	//if the bot has no enemy
	if (enemy < 0 ) {
		return;
	}
	//get the enemy entity information
	BotEntityInfo(enemy, &entinfo);
	//if this is not a player (should be an obelisk)
	if (enemy >= MAX_CLIENTS) {
		//if the obelisk is visible
		VectorCopy(entinfo.origin, target);
		//aim at the obelisk
		VectorSubtract(target, bs->eye, dir);
		vectoangles(dir, bs->ideal_viewangles);
		//set the aim target before trying to attack
		VectorCopy(target, bs->aimtarget);
		return;
	}
	//
	//BotAI_Print(PRT_MESSAGE, "client %d: aiming at client %d\n", bs->entitynum, bs->enemy);
	//
	aim_skill = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_SKILL, 0, 1);
	aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY, 0, 1);
	//
	if (aim_skill > 0.95) {
		//don't aim too early
		reactiontime = 0.5 * trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_REACTIONTIME, 0, 1);
		if (bs->enemysight_time > FloatTime() - reactiontime) return;
		if (bs->teleport_time > FloatTime() - reactiontime) return;
	}
	
	//get the weapon information
	trap_BotGetWeaponInfo(bs->ws, bs->inventory[BI_WEAPON], &wi);
	
	//get the weapon specific aim accuracy and or aim skill
	if (wi.number == WP_MACHINEGUN) {
		aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_MACHINEGUN, 0, 1);
	}
	else if (wi.number == WP_PAIN_SAW) {
		aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_PAIN_SAW, 0, 1);
	}
	else if (wi.number == WP_SHOTGUN) {
		aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_SHOTGUN, 0, 1);
	}
	else if (wi.number == WP_LAS_GUN) {
		aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_LAS_GUN, 0, 1);
		//aim_skill = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_SKILL_GRENADELAUNCHER, 0, 1);
	}
	else if (wi.number == WP_MASS_DRIVER) {
		aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_MASS_DRIVER, 0, 1);
		//aim_skill = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_SKILL_ROCKETLAUNCHER, 0, 1);
	}
	else if (wi.number == WP_CHAINGUN) {
		aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_CHAINGUN, 0, 1);
	}
	else if (wi.number == WP_PULSE_RIFLE) {
		aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_PULSE_RIFLE, 0, 1);
		aim_skill = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_SKILL_PULSE_RIFLE, 0, 1); //Auriga: these will take skill i think
	}
	else if (wi.number == WP_FLAMER) {
		aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_FLAMER, 0, 1);
		//aim_skill = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_SKILL_FLAMER, 0, 1); 
	}
	else if (wi.number == WP_LUCIFER_CANNON) {
		aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_LUCIFERCANNON, 0, 1);
		aim_skill = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_SKILL_LUCIFERCANNON, 0, 1);
	}
	//
	if (aim_accuracy <= 0) aim_accuracy = 0.0001f;
	//get the enemy entity information
	BotEntityInfo(bs->enemy, &entinfo);
	//if the enemy is invisible then shoot crappy most of the time
	if (EntityIsInvisible(&entinfo)) {
		if (random() > 0.1) aim_accuracy *= 0.4f;
	}
	//
	VectorSubtract(entinfo.origin, entinfo.lastvisorigin, enemyvelocity);
	VectorScale(enemyvelocity, 1 / entinfo.update_time, enemyvelocity);
	//enemy origin and velocity is remembered every 0.5 seconds
	if (bs->enemyposition_time < FloatTime()) {
		//
		bs->enemyposition_time = FloatTime() + 0.5;
		VectorCopy(enemyvelocity, bs->enemyvelocity);
		VectorCopy(entinfo.origin, bs->enemyorigin);
	}
	//if not extremely skilled
	if (aim_skill < 0.9) {
		VectorSubtract(entinfo.origin, bs->enemyorigin, dir);
		//if the enemy moved a bit
		if (VectorLengthSquared(dir) > Square(48)) {
			//if the enemy changed direction
			if (DotProduct(bs->enemyvelocity, enemyvelocity) < 0) {
				//aim accuracy should be worse now
				aim_accuracy *= 0.7f;
			}
		}
	}
	//check visibility of enemy
	enemyvisible = BotEntityVisible(bs->entitynum, bs->eye, bs->viewangles, 360, bs->enemy);
	//if the enemy is visible
	if (enemyvisible) {
		//
		VectorCopy(entinfo.origin, bestorigin);
		bestorigin[2] += 8;
		//get the start point shooting from
		//NOTE: the x and y projectile start offsets are ignored
		VectorCopy(bs->origin, start);
		start[2] += bs->cur_ps.viewheight;
		//start[2] += wi.offset[2];
		//
		BotAI_Trace(&trace, start, mins, maxs, bestorigin, bs->entitynum, MASK_SHOT);
		//if the enemy is NOT hit
		if (trace.fraction <= 1 && trace.ent != entinfo.number) {
			bestorigin[2] += 16;
		}
		//if it is not an instant hit weapon the bot might want to predict the enemy
		if (wi.speed) {
			//
			VectorSubtract(bestorigin, bs->origin, dir);
			dist = VectorLength(dir);
			VectorSubtract(entinfo.origin, bs->enemyorigin, dir);
			//if the enemy is NOT pretty far away and strafing just small steps left and right
			if (!(dist > 100 && VectorLengthSquared(dir) < Square(32))) {
				//if skilled anough do exact prediction
				if (aim_skill > 0.8 &&
						//if the weapon is ready to fire
						bs->cur_ps.weaponstate == WEAPON_READY) {
					aas_clientmove_t move;
					vec3_t origin;

					VectorSubtract(entinfo.origin, bs->origin, dir);
					//distance towards the enemy
					dist = VectorLength(dir);
					//direction the enemy is moving in
					VectorSubtract(entinfo.origin, entinfo.lastvisorigin, dir);
					//
					VectorScale(dir, 1 / entinfo.update_time, dir);
					//
					VectorCopy(entinfo.origin, origin);
					origin[2] += 1;
					//
					VectorClear(cmdmove);
					//AAS_ClearShownDebugLines();
					trap_AAS_PredictClientMovement(&move, bs->enemy, origin,
														PRESENCE_CROUCH, qfalse,
														dir, cmdmove, 0,
														dist * 10 / wi.speed, 0.1f, 0, 0, qfalse);
					VectorCopy(move.endpos, bestorigin);
					//BotAI_Print(PRT_MESSAGE, "%1.1f predicted speed = %f, frames = %f\n", FloatTime(), VectorLength(dir), dist * 10 / wi.speed);
				}
				//if not that skilled do linear prediction
				else if (aim_skill > 0.4) {
					VectorSubtract(entinfo.origin, bs->origin, dir);
					//distance towards the enemy
					dist = VectorLength(dir);
					//direction the enemy is moving in
					VectorSubtract(entinfo.origin, entinfo.lastvisorigin, dir);
					dir[2] = 0;
					//
					speed = VectorNormalize(dir) / entinfo.update_time;
					//botimport.Print(PRT_MESSAGE, "speed = %f, wi->speed = %f\n", speed, wi->speed);
					//best spot to aim at
					VectorMA(entinfo.origin, (dist / wi.speed) * speed, dir, bestorigin);
				}
			}
		}
		
		//if the projectile does radial damage
		if (aim_skill > 0.6 && wi.proj.damagetype & DAMAGETYPE_RADIAL) {
			//if the enemy isn't standing significantly higher than the bot
			if (entinfo.origin[2] < bs->origin[2] + 16) {
				//try to aim at the ground in front of the enemy
				VectorCopy(entinfo.origin, end);
				end[2] -= 64;
				BotAI_Trace(&trace, entinfo.origin, NULL, NULL, end, entinfo.number, MASK_SHOT);
				//
				VectorCopy(bestorigin, groundtarget);
				if (trace.startsolid) groundtarget[2] = entinfo.origin[2] - 16;
				else groundtarget[2] = trace.endpos[2] - 8;
				//trace a line from projectile start to ground target
				BotAI_Trace(&trace, start, NULL, NULL, groundtarget, bs->entitynum, MASK_SHOT);
				//if hitpoint is not vertically too far from the ground target
				if (fabs(trace.endpos[2] - groundtarget[2]) < 50) {
					VectorSubtract(trace.endpos, groundtarget, dir);
					//if the hitpoint is near anough the ground target
					if (VectorLengthSquared(dir) < Square(60)) {
						VectorSubtract(trace.endpos, start, dir);
						//if the hitpoint is far anough from the bot
						if (VectorLengthSquared(dir) > Square(100)) {
							//check if the bot is visible from the ground target
							trace.endpos[2] += 1;
							BotAI_Trace(&trace, trace.endpos, NULL, NULL, entinfo.origin, entinfo.number, MASK_SHOT);
							if (trace.fraction >= 1) {
								//botimport.Print(PRT_MESSAGE, "%1.1f aiming at ground\n", AAS_Time());
								VectorCopy(groundtarget, bestorigin);
							}
						}
					}
				}
			}
		}
		
		bestorigin[0] += 20 * crandom() * (1 - aim_accuracy);
		bestorigin[1] += 20 * crandom() * (1 - aim_accuracy);
		bestorigin[2] += 10 * crandom() * (1 - aim_accuracy);
	}
	/* //Auriga: use the prediction from CoW later
	else {
		//
		VectorCopy(bs->lastenemyorigin, bestorigin);
		bestorigin[2] += 8;
		//if the bot is skilled anough
		if (aim_skill > 0.5) {
			//do prediction shots around corners
			if (wi.number == WP_BFG ||
				wi.number == WP_ROCKET_LAUNCHER ||
				wi.number == WP_GRENADE_LAUNCHER) {
				//create the chase goal
				goal.entitynum = bs->client;
				goal.areanum = bs->areanum;
				VectorCopy(bs->eye, goal.origin);
				VectorSet(goal.mins, -8, -8, -8);
				VectorSet(goal.maxs, 8, 8, 8);
				//
				if (trap_BotPredictVisiblePosition(bs->lastenemyorigin, bs->lastenemyareanum, &goal, TFL_DEFAULT, target)) {
					VectorSubtract(target, bs->eye, dir);
					if (VectorLengthSquared(dir) > Square(80)) {
						VectorCopy(target, bestorigin);
						bestorigin[2] -= 20;
					}
				}
				aim_accuracy = 1;
			}
		}
	}
	*/
	//
	if (enemyvisible) {
		BotAI_Trace(&trace, bs->eye, NULL, NULL, bestorigin, bs->entitynum, MASK_SHOT);
		VectorCopy(trace.endpos, bs->aimtarget);
	}
	else {
		VectorCopy(bestorigin, bs->aimtarget);
	}
	//get aim direction
	VectorSubtract(bestorigin, bs->eye, dir);
	//
	/*
	if (wi.number == WP_MACHINEGUN ||
		wi.number == WP_SHOTGUN ||
		wi.number == WP_LIGHTNING ||
		wi.number == WP_RAILGUN) {
		//distance towards the enemy
		dist = VectorLength(dir);
		if (dist > 150) dist = 150;
		f = 0.6 + dist / 150 * 0.4;
		aim_accuracy *= f;
	}*/
	//add some random stuff to the aim direction depending on the aim accuracy
	if (aim_accuracy < 0.8) {
		VectorNormalize(dir);
		for (i = 0; i < 3; i++) dir[i] += 0.3 * crandom() * (1 - aim_accuracy);//Auriga: this is what makes the bot "shaky"
	}
	//set the ideal view angles
	vectoangles(dir, bs->ideal_viewangles);
	//take the weapon spread into account for lower skilled bots
	bs->ideal_viewangles[PITCH] += 6 * 0 * crandom() * (1 - aim_accuracy);
	bs->ideal_viewangles[PITCH] = AngleMod(bs->ideal_viewangles[PITCH]);
	bs->ideal_viewangles[YAW] += 6 * 0 * crandom() * (1 - aim_accuracy);
	bs->ideal_viewangles[YAW] = AngleMod(bs->ideal_viewangles[YAW]);
	//if the bots should be really challenging
	if (bot_challenge.integer) {
		//if the bot is really accurate and has the enemy in view for some time
		if (aim_accuracy > 0.9 && bs->enemysight_time < FloatTime() - 1) {
			//set the view angles directly
			if (bs->ideal_viewangles[PITCH] > 180) bs->ideal_viewangles[PITCH] -= 360;
			VectorCopy(bs->ideal_viewangles, bs->viewangles);
			trap_EA_View(bs->client, bs->viewangles);
		}
	}
}

/*
==================
BotCheckAttack
==================
*/
void HBotCheckAttack(bot_state_t *bs, int attackentity) {
        float fov;
        //int attackentity;
        bsp_trace_t bsptrace;
        vec3_t forward, start, end, dir, angles;
        bsp_trace_t trace;
        aas_entityinfo_t entinfo;
        vec3_t mins = {-8, -8, -8}, maxs = {8, 8, 8};

        //attackentity = bs->enemy;
        //
        trap_AAS_EntityInfo(attackentity, &entinfo);
        
        // if not attacking a player
        if (attackentity >= MAX_CLIENTS) {

        }

        VectorSubtract(bs->aimtarget, bs->eye, dir);

        if (VectorLengthSquared(dir) < Square(100))
                fov = 120;
        else
                fov = 50;
        //
        vectoangles(dir, angles);
        if (!BotInFieldOfVision(bs->viewangles, fov, angles))
                return;
        BotAI_Trace(&bsptrace, bs->eye, NULL, NULL, bs->aimtarget, bs->client, CONTENTS_SOLID|CONTENTS_PLAYERCLIP);
        if (bsptrace.fraction < 1 && bsptrace.ent != attackentity)
                return;

        //get the start point shooting from
        VectorCopy(bs->origin, start);
        start[2] += bs->cur_ps.viewheight;

        AngleVectors(bs->viewangles, forward, NULL, NULL);
        //end point aiming at
        VectorMA(start, 1000, forward, end);
        //a little back to make sure not inside a very close enemy
        VectorMA(start, -12, forward, start);
        BotAI_Trace(&trace, start, mins, maxs, end, bs->entitynum, MASK_SHOT);
        //if the entity is a client
        if (trace.ent > 0 && trace.ent <= MAX_CLIENTS) {
                if (trace.ent != attackentity) {
                        //if a teammate is hit
                        if (BotSameTeam(bs, trace.ent))
                                return;
                }
        }
        
        // TODO avoid radial damage
        
        /*
        //if fire has to be release to activate weapon
        if (wi.flags & WFL_FIRERELEASED) {
                if (bs->flags & BFL_ATTACKED) {
                        trap_EA_Attack(bs->client);
                }
        }
        else {
        */
                trap_EA_Attack(bs->client);
        /*}
        bs->flags ^= BFL_ATTACKED;
        */

}

void BBotCheckClientAttack(bot_state_t *bs) {
	float points, reactiontime, fov, firethrottle;
	int attackentity;
	bsp_trace_t bsptrace;
	//float selfpreservation;
	vec3_t forward, right, start, end, dir, angles;
	weaponinfo_t wi;
	bsp_trace_t trace;
	aas_entityinfo_t entinfo;
	vec3_t mins = {-8, -8, -8}, maxs = {8, 8, 8};

	attackentity = bs->enemy;
	//
	BotEntityInfo(attackentity, &entinfo);
	// if not attacking a player
	if (attackentity >= MAX_CLIENTS) {
	}
	//
	reactiontime = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_REACTIONTIME, 0, 1);
	if (bs->enemysight_time > FloatTime() - reactiontime) return;
	if (bs->teleport_time > FloatTime() - reactiontime) return;
	//if changing weapons
	//if (bs->weaponchange_time > FloatTime() - 0.1) return;
	//check fire throttle characteristic
	if (bs->firethrottlewait_time > FloatTime()) return;
	firethrottle = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_FIRETHROTTLE, 0, 1);
	if (bs->firethrottleshoot_time < FloatTime()) {
		if (random() > firethrottle) {
			bs->firethrottlewait_time = FloatTime() + firethrottle;
			bs->firethrottleshoot_time = 0;
		}
		else {
			bs->firethrottleshoot_time = FloatTime() + 1 - firethrottle;
			bs->firethrottlewait_time = 0;
		}
	}
	//
	//
	VectorSubtract(bs->aimtarget, bs->eye, dir);
	//
	if (bs->inventory[BI_WEAPON] == WP_PAIN_SAW) {
		if (VectorLengthSquared(dir) > Square(60)) {
			return;
		}
	}
	if (VectorLengthSquared(dir) < Square(100))
		fov = 120;
	else
		fov = 50;
	//
	vectoangles(dir, angles);
	if (!BotInFieldOfVision(bs->viewangles, fov, angles))
		return;
	BotAI_Trace(&bsptrace, bs->eye, NULL, NULL, bs->aimtarget, bs->client, CONTENTS_SOLID|CONTENTS_PLAYERCLIP);
	if (bsptrace.fraction < 1 && bsptrace.ent != attackentity)
		return;

	//get the weapon info
	trap_BotGetWeaponInfo(bs->ws, bs->inventory[BI_WEAPON], &wi);
	//get the start point shooting from
	VectorCopy(bs->origin, start);
	start[2] += bs->cur_ps.viewheight;
	AngleVectors(bs->viewangles, forward, right, NULL);
	start[0] += forward[0] * wi.offset[0] + right[0] * wi.offset[1];
	start[1] += forward[1] * wi.offset[0] + right[1] * wi.offset[1];
	start[2] += forward[2] * wi.offset[0] + right[2] * wi.offset[1] + wi.offset[2];
	//end point aiming at
	VectorMA(start, 1000, forward, end);
	//a little back to make sure not inside a very close enemy
	VectorMA(start, -12, forward, start);
	BotAI_Trace(&trace, start, mins, maxs, end, bs->entitynum, MASK_SHOT);
	//if the entity is a client
	if (trace.ent > 0 && trace.ent <= MAX_CLIENTS) {
		if (trace.ent != attackentity) {
			//if a teammate is hit
			if (BotSameTeam(bs, trace.ent))
				return;
		}
	}
	//if won't hit the enemy or not attacking a player (obelisk)
	if (trace.ent != attackentity || attackentity >= MAX_CLIENTS) {
		//if the projectile does radial damage
		if (wi.proj.damagetype & DAMAGETYPE_RADIAL) {
			if (trace.fraction * 1000 < wi.proj.radius) {
				points = (wi.proj.damage - 0.5 * trace.fraction * 1000) * 0.5;
				if (points > 0) {
					return;
				}
			}
			//FIXME: check if a teammate gets radial damage
		}
	}
	//if fire has to be release to activate weapon 
	if (wi.flags & WFL_FIRERELEASED) { //Auriga: needs change for LC to prevent backfire
		if (bs->flags & BFL_ATTACKED) {
			trap_EA_Attack(bs->client);
		}
	}
	else {
		trap_EA_Attack(bs->client);
	}
	bs->flags ^= BFL_ATTACKED;
}

qboolean botWeaponHasLowAmmo(bot_state_t *bs) {
    switch(bs->inventory[BI_WEAPON]){
		case WP_MACHINEGUN:
		case WP_PULSE_RIFLE:
			if(!bs->inventory[BI_CLIPS]) return qfalse;
			else
				return qtrue;
			break;
		case WP_LAS_GUN:
		case WP_CHAINGUN:
			if(bs->inventory[BI_AMMO] < 10) return qfalse;
			break;
	}
	if( bs->inventory[BI_WEAPON] < WP_MACHINEGUN || bs->inventory[BI_WEAPON] > WP_LUCIFER_CANNON) {
		return qfalse;
	}
}

qboolean HBotEquipOK(bot_state_t* bs){
	int totalcredit;
	if(G_BuildableRange( bs->cur_ps.origin, 1000, BA_H_ARMOURY )){
		totalcredit = BG_GetValueOfEquipment( &bs->cur_ps ) + bs->inventory[BI_CREDITS];
		switch(g_humanStage.integer){
			case 0:
				if( totalcredit >= 420 && bs->inventory[BI_WEAPON] != WP_LAS_GUN) return qfalse;
				break;
			case 1:
				if( totalcredit >= 660 && bs->inventory[BI_WEAPON] != WP_PULSE_RIFLE ) return qfalse;
				break;
			case 2:
				if( totalcredit >= 800 && bs->inventory[BI_WEAPON] != WP_CHAINGUN) return qfalse;
				break;
		}
	}
	switch(bs->inventory[BI_WEAPON]){
		case WP_MACHINEGUN:
		case WP_PULSE_RIFLE:
			if(!bs->inventory[BI_CLIPS]) return qfalse;
			else
				return qtrue;
			break;
		case WP_LAS_GUN:
		case WP_CHAINGUN:
			if(bs->inventory[BI_AMMO] < 10) return qfalse;
			break;
	}
	if( bs->inventory[BI_WEAPON] < WP_MACHINEGUN || bs->inventory[BI_WEAPON] > WP_LUCIFER_CANNON) {
		return qfalse;
	}
		
	else return qtrue;
	/*
if(botWeaponHasLowAmmo(bs)) //Auriga: adapted from cowbot
        return qtrue;
    if(BG_InventoryContainsWeapon(WP_HBUILD,bs->ent->client->ps.stats))
        return qtrue;
    //see if we can afford lightarmor and we dont have any on currently
    if(g_humanStage.integer == S1 || g_humanStage.integer == S2 || g_humanStage.integer == S3) {
        
        //100 is the highest minimum amount of credits needed to buy something new
        if((short) bs->inventory[BI_CREDITS] > BG_FindPriceForUpgrade(UP_LIGHTARMOUR) && 
        !BG_InventoryContainsUpgrade(UP_LIGHTARMOUR, bs->ent->client->ps.stats))
            return qtrue;
    }
    //see if we can afford a helmet and we dont have any on currently
    if(g_humanStage.integer == S2 || g_humanStage.integer == S3) {
        if((short) bs->inventory[BI_CREDITS] > BG_FindPriceForUpgrade(UP_HELMET) &&
        !BG_InventoryContainsUpgrade(UP_HELMET, bs->ent->client->ps.stats))
            return qtrue;
    }
    return qfalse;*/
}

void HBotCheckReload(bot_state_t* bs){
	if( !bs->inventory[BI_AMMO] && bs->inventory[BI_CLIPS] ){
		trap_EA_Command(bs->client, "reload");
	} else if (!bs->inventory[BI_CLIPS] && !bs->inventory[BI_AMMO] && bs->ent->client->ps.weapon != WP_BLASTER){
		trap_EA_Command(bs->client, "itemtoggle blaster" );
	}
}

void HBotCheckRespawn(bot_state_t* bs){
	char buf[144];
	
	//BotChatTest(bs);
	//on the spawn queue?
	//if( bs->cur_ps.pm_flags & PMF_QUEUED) return;
	
	// respawned, but not spawned.. send class cmd
	if( BotIntermission(bs) ){
		// rifle || ckit || akit
			Com_sprintf(buf, sizeof(buf), "class rifle" );//bots dont build yet, so spawn with rifle!
		/*
    	if( BG_WeaponIsAllowed( WP_HBUILD2 ) && BG_FindStagesForWeapon( WP_HBUILD2, g_humanStage.integer ) )
    		Com_sprintf(buf, sizeof(buf), "class ackit");
    	else
    		Com_sprintf(buf, sizeof(buf), "class ckit" );
			*/
    	trap_EA_Command(bs->client, buf );
    	return;
	}
	
	// trigger respawn if dead
  	//if( bs->cur_ps.pm_type == PM_DEAD || bs->cur_ps.pm_type == PM_SPECTATOR ){
	if( bs->inventory[BI_HEALTH] <= 0){
		trap_EA_Attack(bs->client);
	}
}

int G_BotBuyUpgrade ( bot_state_t *bs, int upgrade )
{
    //already got this?
    if( BG_InventoryContainsUpgrade( upgrade, bs->ent->client->ps.stats ) )
        return qfalse;

    //can afford this?
    if( BG_FindPriceForUpgrade( upgrade ) > (short)bs->inventory[BI_CREDITS] )
        return qfalse;

    //have space to carry this?
    if( BG_FindSlotsForUpgrade( upgrade ) & bs->ent->client->ps.stats[ STAT_SLOTS ] )
        return qfalse;

    //are we /allowed/ to buy this?
    if( !BG_FindPurchasableForUpgrade( upgrade ) )
        return qfalse;

    //are we /allowed/ to buy this?
    if( !BG_FindStagesForUpgrade( upgrade, g_humanStage.integer ) || !BG_UpgradeIsAllowed( upgrade ) )
        return qfalse;

    if( upgrade == UP_AMMO )
        G_GiveClientMaxAmmo( bs->ent, qfalse );
    else
        //add to inventory
        trap_EA_Command(bs->client, "buy battpack");

    if( upgrade == UP_BATTPACK )
        G_GiveClientMaxAmmo( bs->ent, qtrue );

    //subtract from funds
    G_AddCreditToClient( bs->ent->client, -(short)BG_FindPriceForUpgrade( upgrade ), qfalse );

    return qtrue;

	HBotUpdateInventory(bs);
}

int G_BotBuyWeapon ( bot_state_t *bs, int weapon )
{
    int maxAmmo, maxClips;
	char buf[MAX_STRING_CHARS];

    //already got this?
    if( BG_InventoryContainsWeapon( weapon, bs->ent->client->ps.stats ) )
        return qfalse;

    //can afford this?
    if( BG_FindPriceForWeapon( weapon ) > (short)bs->inventory[BI_CREDITS] )
        return qfalse;

    //have space to carry this?
    if( BG_FindSlotsForWeapon( weapon ) & bs->ent->client->ps.stats[ STAT_SLOTS ] )
        return qfalse;

    //are we /allowed/ to buy this?
    if( !BG_FindPurchasableForWeapon( weapon ) )
        return qfalse;

    //are we /allowed/ to buy this?
    if( !BG_FindStagesForWeapon( weapon, g_humanStage.integer ) || !BG_WeaponIsAllowed( weapon ) )
        return qfalse;
    
    //does server allow us to buy this?
    switch(weapon) {
        case WP_MACHINEGUN:
            if(bot_rifle.integer == 0)
                return qfalse;
            break;
        case WP_PAIN_SAW:
            if(bot_psaw.integer == 0)
                return qfalse;
            break;
        case WP_SHOTGUN:
            if(bot_shotgun.integer == 0)
                return qfalse;
            break;
        case WP_LAS_GUN:
            if(bot_las.integer == 0)
                return qfalse;
            break;
        case WP_MASS_DRIVER:
            if(bot_mass.integer == 0)
                return qfalse;
            break;
        case WP_CHAINGUN:
            if(bot_chain.integer == 0)
                return qfalse;
            break;
        case WP_PULSE_RIFLE:
            if(bot_pulse.integer == 0)
                return qfalse;
            break;
        case WP_FLAMER:
            if(bot_flamer.integer == 0)
                return qfalse;
            break;
        case WP_LUCIFER_CANNON:
            if(bot_luci.integer == 0)
                return qfalse;
            break;
        default: break;
    }
            

    //add to inventory
    //BG_AddWeaponToInventory( weapon, bs->ent->client->ps.stats );
    //BG_FindAmmoForWeapon( weapon, &maxAmmo, &maxClips );
	//Com_sprintf(buf, BG_FindNameForWeapon(weapon), sizeof(buf));
	trap_EA_Command(bs->client, va( "buy %s", BG_FindNameForWeapon(weapon)));
	
    //BG_PackAmmoArray( weapon, bs->ent->client->ps.ammo, bs->ent->client->ps.misc,
    //                  maxAmmo, maxClips );

    //G_ForceWeaponChange( bs->ent, weapon );

    //set build delay/pounce etc to 0
    //bs->ent->client->ps.stats[ STAT_MISC ] = 0;

    //subtract from funds
    //G_AddCreditToClient( bs->ent->client, -(short)BG_FindPriceForWeapon( weapon ), qfalse );


    return qtrue;
}

// armoury AI: buy stuff depending on credits (stage, situation)
void HBotShop(bot_state_t* bs){
	int totalcredit;
	totalcredit = BG_GetValueOfEquipment( &bs->cur_ps ) + bs->inventory[BI_CREDITS];
	switch(g_humanStage.integer){
		case 0:
			if( totalcredit >= 420){
				trap_EA_Command(bs->client, "sell weapons" );
				trap_EA_Command(bs->client, "sell upgrades" );
				trap_EA_Command(bs->client, "buy larmour" );
				trap_EA_Command(bs->client, "buy battpack" );
				trap_EA_Command(bs->client, "buy lgun" );
			}break;
		case 1:
			if( totalcredit >= 660){
				trap_EA_Command(bs->client, "sell weapons" );
				trap_EA_Command(bs->client, "sell upgrades" );
				trap_EA_Command(bs->client, "buy larmour" );
				trap_EA_Command(bs->client, "buy battpack" );
				trap_EA_Command(bs->client, "buy helmet" );
				trap_EA_Command(bs->client, "buy prifle" );
			}break;
		case 2:
			if( totalcredit >= 800){
				trap_EA_Command(bs->client, "sell weapons" );
				trap_EA_Command(bs->client, "sell upgrades" );
				trap_EA_Command(bs->client, "buy bsuit" );
				trap_EA_Command(bs->client, "buy chaingun" );
				//trap_EA_Command(bs->client, "buy lgun" ); 
			}break;
	}
	if(bs->ent->client->ps.weapon == WP_BLASTER)
		trap_EA_Command(bs->client, "itemtoggle blaster" );
	//trap_EA_Command(bs->client, "sell ckit" );
	//trap_EA_Command(bs->client, "buy rifle" );
	trap_EA_Command(bs->client, "buy ammo" );
	
	//now update your inventory
	/*HBotUpdateInventory(bs);

        //try to buy helmet/lightarmour
		trap_EA_Command(bs->client, "sell weapons" );
		trap_EA_Command(bs->client, "sell upgrades" );
        G_BotBuyUpgrade( bs, UP_HELMET);
        G_BotBuyUpgrade( bs, UP_LIGHTARMOUR);
        
        // buy most expensive first, then one cheaper, etc, dirty but working way
        if( !G_BotBuyWeapon( bs, WP_LUCIFER_CANNON ) )
            if( !G_BotBuyWeapon( bs, WP_FLAMER ) )
                if( !G_BotBuyWeapon( bs, WP_PULSE_RIFLE ) )
                    if( !G_BotBuyWeapon( bs, WP_CHAINGUN ) )
                        if( !G_BotBuyWeapon( bs, WP_MASS_DRIVER ) )
                            if( !G_BotBuyWeapon( bs, WP_LAS_GUN ) )
                                if( !G_BotBuyWeapon( bs, WP_SHOTGUN ) )
                                    if( !G_BotBuyWeapon( bs, WP_PAIN_SAW ) )
                                        G_BotBuyWeapon( bs, WP_MACHINEGUN );
                                    
        //buy ammo/batpack
        if( BG_FindUsesEnergyForWeapon( bs->inventory[BI_WEAPON] )) {
            G_BotBuyUpgrade( bs, UP_BATTPACK );
        }else {
            G_BotBuyUpgrade( bs, UP_AMMO );
        }*/
	
	//now update your inventory
	HBotUpdateInventory(bs);
	
}


qboolean HBotAttack(bot_state_t* bs){
	gentity_t* ent;
	// report
	//BotAddInfo(bs, "task", "attack");
	// reload ?
	HBotCheckReload(bs);
	// find target
	if(bs->enemy >=0){
		ent = &g_entities[  bs->enemy ];
		// shoot if target in sight	
		// aim and check attack
		//HBotStrafe(bs);
		if(ent->health <= 0){
			BotChat_Kill(bs);
		}
	}
	bs->enemy = BotFindEnemy(bs, bs->enemy);
	if(bs->enemy >=0){
		
		BotAddInfo(bs, "action", "shoot");
		HBotAimAtEnemy(bs, bs->enemy);
		BBotCheckClientAttack(bs);
		return qtrue;
	} else {
		return qfalse;
	}
}

void HBotStrafe(bot_state_t *bs){
	float attack_dist, attack_range;
	int movetype, i;
	vec3_t forward, backward, sideward, hordir, up = {0, 0, 1};
	aas_entityinfo_t entinfo;
	float dist, strafechange_time;
	
	attack_dist = IDEAL_ATTACKDIST;
	attack_range = 40;
	BotEntityInfo(bs->enemy, &entinfo);
	//direction towards the enemy
	VectorSubtract(entinfo.origin, bs->origin, forward);
	//the distance towards the enemy
	dist = VectorNormalize(forward);
	VectorNegate(forward, backward);
	//increase the strafe time
	bs->attackstrafe_time += bs->thinktime;
	//get the strafe change time
	strafechange_time = 0.4 + (1 - 0.6) * 0.2;
	if (0.6 > 0.7) strafechange_time += crandom() * 0.2;
	//if the strafe direction should be changed
	if (bs->attackstrafe_time > strafechange_time) {
		//some magic number :)
		if (random() > 0.935) {
			//flip the strafe direction
			bs->flags ^= BFL_STRAFERIGHT;
			bs->attackstrafe_time = 0;
		}
	}
	//
	for (i = 0; i < 2; i++) {
		hordir[0] = forward[0];
		hordir[1] = forward[1];
		hordir[2] = 0;
		VectorNormalize(hordir);
		//get the sideward vector
		CrossProduct(hordir, up, sideward);
		//reverse the vector depending on the strafe direction
		if (bs->flags & BFL_STRAFERIGHT) VectorNegate(sideward, sideward);
		//randomly go back a little
		if (random() > 0.9) {
			VectorAdd(sideward, backward, sideward);
		}
		else {
			//walk forward or backward to get at the ideal attack distance
			if (dist > attack_dist + attack_range) {
				VectorAdd(sideward, forward, sideward);
			}
			else if (dist < attack_dist - attack_range) {
				VectorAdd(sideward, backward, sideward);
			}
		}
		//perform the movement
		//if (trap_BotMoveInDirection(bs->ms, sideward, 400, movetype))
		if (trap_BotMoveInDirection(bs->ms, sideward, 400, movetype))
			break;
		//movement failed, flip the strafe direction
		bs->flags ^= BFL_STRAFERIGHT;
		bs->attackstrafe_time = 0;
	}
}

// HBotFindEnemy
// if enemy is found: bs->enemy and return qtrue

qboolean HBotFindEnemy(bot_state_t* bs){
	// return 0 if no alien building can be found
	
	// todo: find alien in FOV
	
	//see if there's an enemy in range, and go for it
	if( BotGoalForEnemy( bs , &bs->goal ) ) return qtrue;
	//{
	// go for acid tubes
	if( BotGoalForClosestBuildable(  bs, &bs->goal, BA_A_ACIDTUBE ) ) return qtrue;	
	// go for eggs
	if( BotGoalForClosestBuildable(  bs, &bs->goal, BA_A_SPAWN ) ) return qtrue;	
	// go for OM
	if( BotGoalForClosestBuildable(  bs, &bs->goal, BA_A_OVERMIND ) )return qtrue;	
	// haven't returned yet so no enemy found > dont do anything
	BotAddInfo(bs, "enemy", "none");
	return qfalse;
	//}
	//bs->enemy = bs->goal.entitynum;
	//BotAddInfo(bs, "enemy", va("# %d", bs->enemy) );
	
}

qboolean HBotRush(bot_state_t* bs){
	if(HBotFindEnemy(bs)){
			bs->state = HS_MOVE;
			bs->nextstate = HS_KILL;
			bs->nextstatetype = NEXT_STATE_LOS;
			bs->nextstatetypelosdist = 0;
		}
}


qboolean HBotKill(bot_state_t* bs){
	gentity_t* ent = &g_entities[ bs->goal.entitynum ];
	//state transitions	
	// dead -> spawn
	if( !BotIsAlive(bs) ){
		bs->state = HS_SPAWN;
		return qfalse;
	}	
	// not satisfied with current equip? -> got get equipment!
	if( !HBotEquipOK(bs) && bs->nextstate != HS_GEAR ){
		bs->state = HS_GEAR;
		return qfalse;
	}
	// report
	BotAddInfo(bs, "task", "KILL!!!!");
	// reload ?
	HBotCheckReload(bs);
	// shoot if target in sight
	if(ent->health <= 0){
		bs->state = HS_RUSH;
		//bs->nextstate = HS_KILL;
		bs->nextstatetype = NEXT_STATE_NONE;
		//bs->nextstatetypelosdist = 0;
	}
	if(HBotAttack(bs)){
		return qtrue;	
	}else if( BotEntityVisible(bs->entitynum, bs->eye, bs->viewangles, 360, bs->goal.entitynum) ){		
		// aim and check attack
		//HBotStrafe(bs); //Auriga: disabled for now
		HBotKeepDistance(bs);
		BotAddInfo(bs, "action", "shoot");
		HBotAimAtEnemy(bs, bs->goal.entitynum);
		HBotCheckAttack(bs, bs->goal.entitynum);
	} else {
		bs->state = HS_RUSH;
		//bs->nextstate = HS_KILL;
		bs->nextstatetype = NEXT_STATE_NONE;
		//bs->nextstatetypelosdist = 0;
	}
	return qtrue;
}

// inventory becomes quite useless, thanks to the BG_Inventory functions..
void HBotUpdateInventory(bot_state_t* bs){
	weapon_t i;
	int ammo, clips;
	
	//Bot_Print(BPMSG, "\nUpdating Inventory");
	// why exclude WP_HBUILD ?
	bs->inventory[BI_WEAPON] = WP_NONE;
	for(i= WP_MACHINEGUN; i <= WP_LUCIFER_CANNON; i++){
		if( BG_InventoryContainsWeapon( i, bs->cur_ps.stats ) ){
			bs->inventory[BI_WEAPON] = i;
			break;
		}
	}
	// ammo and clips
	BG_FindAmmoForWeapon( bs->inventory[BI_WEAPON], &ammo, &clips );
	//BG_UnpackAmmoArray( bs->inventory[BI_WEAPON], bs->cur_ps.ammo, bs->cur_ps.misc, &ammo, &clips );		// get maximum with BG_FindAmmoForWeapon
	bs->inventory[BI_AMMO] = ammo;
	bs->inventory[BI_CLIPS] = clips;
	//Bot_Print(BPMSG, "\nI have %d ammo and %d clips", ammo, clips);
	
	bs->inventory[BI_HEALTH] = bs->cur_ps.stats[STAT_HEALTH];
	bs->inventory[BI_CREDITS] = bs->cur_ps.persistant[ PERS_CREDIT ];
	bs->inventory[BI_STAMINA] = bs->cur_ps.stats[ STAT_STAMINA ];
	//Bot_Print(BPMSG, "\nI have %d health, %d credits and %d stamina", bs->cur_ps.stats[STAT_HEALTH],
	//		bs->cur_ps.persistant[ PERS_CREDIT ],
	//		bs->cur_ps.stats[ STAT_STAMINA ]);
	
	bs->inventory[BI_GRENADE] = BG_InventoryContainsUpgrade( UP_GRENADE, bs->cur_ps.stats );
	bs->inventory[BI_MEDKIT] = BG_InventoryContainsUpgrade( UP_MEDKIT, bs->cur_ps.stats );
	bs->inventory[BI_JETPACK] = BG_InventoryContainsUpgrade( UP_JETPACK, bs->cur_ps.stats );
	bs->inventory[BI_BATTPACK] = BG_InventoryContainsUpgrade( UP_BATTPACK, bs->cur_ps.stats );
	bs->inventory[BI_LARMOR] = BG_InventoryContainsUpgrade( UP_LIGHTARMOUR, bs->cur_ps.stats );
	bs->inventory[BI_HELMET] = BG_InventoryContainsUpgrade( UP_HELMET, bs->cur_ps.stats );
	bs->inventory[BI_BSUIT] = BG_InventoryContainsUpgrade( UP_BATTLESUIT, bs->cur_ps.stats );
}

void HBotAvoid(bot_state_t *bs){
	// Adds acid tubes to the avoid list.
	int i;
	gentity_t* ent;	
	trap_BotAddAvoidSpot(bs->ms, ent->s.origin, 160, AVOID_CLEAR);
	for( i = 1, ent = g_entities + i ; i < level.num_entities ; i++, ent++ ){
		// filter with s.eType = ET_BUILDABLE ?
		// skip dead buildings
		if(ent->health <= 0) continue;
		if( ent->s.modelindex == BA_A_ACIDTUBE) trap_BotAddAvoidSpot(bs->ms, ent->s.origin, 160, AVOID_ALWAYS);
		if( ent->s.modelindex == BA_A_OVERMIND) trap_BotAddAvoidSpot(bs->ms, ent->s.origin, 60, AVOID_ALWAYS);
		//else trap_BotAddAvoidSpot(bs->ms, ent->s.origin, 25, AVOID_ALWAYS);
	}
	return;
}



qboolean HBotCheckNextState(bot_state_t* bs){
	switch(bs->nextstatetype){
		case NEXT_STATE_LOS:
			if(BotEntityVisible(bs->entitynum, bs->eye, bs->viewangles, 360, bs->goal.entitynum))
				bs->state = bs->nextstate;
				bs->nextstate = HS_NONE;
				break;
		case NEXT_STATE_DISTANCE:
			if(BotEntityDistance(bs, bs->goal.entitynum) <= bs->nextstatetypelosdist)
				bs->state = bs->nextstate;
				bs->nextstate = HS_NONE;
				break;
		default:
			return qfalse;
	}
	return qtrue;
}

qboolean HBotMove(bot_state_t* bs){
	bot_moveresult_t moveresult;
	vec3_t target, dir;
	//aas_entityinfo_t entinfo;
	
	///state transitions
	// dead -> spawn
	if( !BotIsAlive(bs) ){
		bs->state = HS_SPAWN;
		return qfalse;
	}
	
	// not satisfied with current equip? -> got get equipment!
	if( !HBotEquipOK(bs) && bs->nextstate != HS_GEAR ){
		bs->state = HS_GEAR;
		return qfalse;
	}
	if(bs->nextstate == HS_KILL)
		HBotRush(bs);
	// report
	//BotAddInfo(bs, "task", "attack");
	
	// reload ?
	HBotCheckReload(bs);
	
	//BotEntityInfo(bs->enemy, &entinfo);
	
	//bs->tfl = TFL_DEFAULT;
	BotAddInfo(bs, "action", va("movetogoal %d", trap_AAS_AreaTravelTimeToGoalArea(bs->areanum, bs->cur_ps.origin, bs->goal.areanum, bs->tfl)) );
	BotSetupForMovement(bs);
	// Avoid
	//HBotAvoid(bs);
	//move towards the goal
	trap_BotMoveToGoal(&moveresult, bs->ms, &bs->goal, bs->tfl);
	BotAIBlocked(bs, &moveresult, qtrue);
	//BotMovementViewTarget(int movestate, bot_goal_t *goal, int travelflags, float lookahead, vec3_t target)
	if(!HBotAttack(bs)){
		if (trap_BotMovementViewTarget(bs->ms, &bs->goal, bs->tfl, 300, target)) {
			//Bot_Print(BPMSG, "I'm uh...fixing my view angle?");
			VectorSubtract(target, bs->origin, dir);
			vectoangles(dir, bs->ideal_viewangles);	
		}
	}
	HBotCheckNextState(bs);
	return qtrue;
}


// go for arm and gear up
qboolean HBotGear(bot_state_t* bs){
	//bot_goal_t goal;
	//bot_moveresult_t moveresult;
	//vec3_t target, dir;
	
	//state transitions
	
	// dead -> spawn
	if( !BotIsAlive(bs) ){
		bs->state = HS_SPAWN;
		return qfalse;
	}
	
	// satisfied with current equip? -> attack!
	if( HBotEquipOK(bs) ){
		bs->state = HS_RUSH;
		return qfalse;
	}
	if( G_BuildableRange( bs->cur_ps.origin, 100, BA_H_ARMOURY ) ){
		// shopping time!
		HBotShop(bs);
		BotAddInfo(bs, "task", "shopping!");
		//now that we've bought ammo, move to the next state
		bs->state = HS_RUSH;
		bs->nextstate = HS_NONE;
		bs->nextstatetype = NEXT_STATE_NONE;
		bs->nextstatetypelosdist = 0;
		return qfalse;
	}
	// find armory
	if( BotGoalForBuildable( bs, &bs->goal, BA_H_ARMOURY) ){
		bs->state = HS_MOVE;
		bs->nextstate = HS_GEAR;
		bs->nextstatetype = NEXT_STATE_DISTANCE;
		bs->nextstatetypelosdist = 100;
		return qtrue;
	}
	return qtrue;
}
	
qboolean HBotSpawn(bot_state_t* bs){
	//state transitions
	if( BotIsAlive(bs) ){
		bs->state = HS_GEAR;
		return qfalse;
	}
	
	HBotCheckRespawn(bs);
	return qtrue;
}


// hbotrunstate: return qfalse if state is changed, 
// so new state can be executed within the same frame
qboolean HBotRunState(bot_state_t* bs){
	switch(bs->state){
		case HS_SPAWN:
			return HBotSpawn(bs);
		case HS_GEAR:
			return HBotGear(bs);
		case HS_RUSH:
			return HBotRush(bs);
		case HS_MOVE:
			return HBotMove(bs);
		case HS_KILL:
			return HBotKill(bs);
		default:
			Bot_Print( BPERROR, "bs->state irregular value %d \n", bs->state);
			bs->state = HS_SPAWN;
			return qtrue;
	}
	return qtrue;
}

void HBotKeepDistance(bot_state_t *bs) {

}

/*
==================
BotHumanAI
==================
*/
void BotHumanAI(bot_state_t *bs, float thinktime) {
	//char buf[144];
	
	/*bot_goal_t goal;
	bot_moveresult_t moveresult;
	int tt;
	vec3_t target, dir;
	*/
	//char userinfo[MAX_INFO_STRING];

	//if the bot has just been setup
	if (bs->setupcount > 0) {
		bs->setupcount--;
		if (bs->setupcount > 0) return;

		trap_EA_Command(bs->client, "team humans");
		//
		//bs->lastframe_health = bs->inventory[INVENTORY_HEALTH];
		//bs->lasthitcount = bs->cur_ps.persistant[PERS_HITS];
		bs->setupcount = 0;
	}	
	
	// update knowledge base and inventory
	HBotUpdateInventory(bs);
	HBotAvoid(bs);
	
	// run the FSM
	bs->statecycles = 0;
	while( !HBotRunState(bs) ){
		if( ++(bs->statecycles) > 5){
			BotAddInfo(bs, "botstates", "loop");
			break;
		}
	}
	
	// update the hud
	if(bot_developer.integer){
		// fsm state
		BotAddInfo(bs, "state", va("%s",stateNames[ bs->state ]) );
		BotAddInfo(bs, "nextstate", va("%s" ,stateNames[ bs->nextstate ]) );
		
		// weapon
		BotAddInfo(bs, "skill", va("%f",trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_SKILL, 0, 1) ) );
		// ammo
		//BotAddInfo(bs, "ammo", va("%d",bs->inventory[BI_AMMO]) );
		//target
		BotAddInfo(bs, "goal", va("%d",bs->goal.entitynum) );
		//Enemy Info
		if(bs->enemyent->client)
			BotAddInfo(bs, "enemyname", va("%s",bs->enemyent->client->pers.netname) );
		// copy config string
		trap_SetConfigstring( CS_BOTINFOS + bs->client, bs->hudinfo);
	}
}
