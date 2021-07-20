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

#include "g_bot.h"

void G_BotThinkHuman( gentity_t *self )
{
	int distance = 0;
	int clicksToStopChase = 30; //5 seconds
	int tooCloseDistance = 100; // about 1/3 of turret range
	int forwardMove = 127; // max speed
	int tempEntityIndex = -1;
	qboolean followFriend = qfalse;
	
	self->client->pers.cmd.buttons = 0;
	self->client->pers.cmd.forwardmove = 0;
	self->client->pers.cmd.upmove = 0;
	self->client->pers.cmd.rightmove = 0;
	
	// reset botEnemy if enemy is dead
	if(self->botEnemy->health <= 0) {
		self->botEnemy = NULL;
	}
	
	// if friend dies, reset status to regular
	if(self->botFriend->health <= 0) {
		self->botCommand = BOT_REGULAR;
		self->botFriend = NULL;
	}
	
	// what mode are we in?
	/*
	BOT_NONE = 1,				//does absolutely nothing
	BOT_TARGET_PRACTICE,		//move to a pre-specified location and stand there
	BOT_DODGE,				//move to a pre-specified location and dodge around
	BOT_DODGE_WITH_ATTACK,	//move to a pre-specified location and dodge around and fire on enemies
	BOT_ATTACK,				//attack the enemy
	BOT_DEFENSIVE,			//defend the base
	BOT_FOLLOW_FRIEND_PROTECT, //follow a friend and protect them 
	BOT_FOLLOW_FRIEND_ATTACK,	 //follow a friend and attack enemies
	BOT_FOLLOW_FRIEND_IDLE,	 //follow a friend and do nothing
	BOT_BUILD				//build
	BOT_TEAM_KILLER			//shoot up your teammates
	*/
	switch(self->botCommand) {
		case BOT_NONE:
			//do absolutely nothing
			break;
		case BOT_TARGET_PRACTICE:
		case BOT_DODGE:
		case BOT_DODGE_WITH_ATTACK:
			 break; 
		case BOT_DEFENSIVE:
			// if there is enemy around, rush them and attack.
			if(self->botEnemy) {
				// we already have an enemy. See if still in LOS.
				if(!botTargetInRange(self,self->botEnemy)) {
					// if it's been over clicksToStopChase clicks since we last seen him in LOS then do nothing, else follow him!
					if(self->botEnemyLastSeen > clicksToStopChase) {
						// forget him!
						self->botEnemy = NULL;
						self->botEnemyLastSeen = 0;
					} else {
						//chase him
						self->botEnemyLastSeen++;
					}
				} else {
					// we see him!
					self->botEnemyLastSeen = 0;
				}
			}
			
			if(!self->botEnemy) {
				// try to find closest enemy
				tempEntityIndex = botFindClosestEnemy(self, qfalse);
				if(tempEntityIndex >= 0) {
					self->botEnemy = &g_entities[tempEntityIndex];
				}
			}
			
			if(!self->botEnemy) {
				// no enemy
			} else {
				// enemy!
				distance = botGetDistanceBetweenPlayer(self, self->botEnemy);
				botAimAtTarget(self, self->botEnemy);
				
				// enable wallwalk
				if( BG_ClassHasAbility( self->client->ps.stats[ STAT_PCLASS ], SCA_WALLCLIMBER ) ) {
					self->client->pers.cmd.upmove = -1;
				}
				
				botShootIfTargetInRange(self,self->botEnemy);
				self->client->pers.cmd.forwardmove = forwardMove;
				self->client->pers.cmd.rightmove = -100;
				if(self->client->time1000 >= 500)
					self->client->pers.cmd.rightmove = 100;
			}
			
			break;
			
		case BOT_IDLE:
			// just stand there and look pretty.
			break;
			
		case BOT_ATTACK:
			// .. not sure ..
			break;
			
		case BOT_STAND_GROUND:
			// stand ground but attack enemies if you can reach.
			if(self->botEnemy) {
				// we already have an enemy. See if still in LOS.
				if(!botTargetInRange(self,self->botEnemy)) {
					//we are not in LOS
					self->botEnemy = NULL;
				}
			}
			
			if(!self->botEnemy) {
				// try to find closest enemy
				tempEntityIndex = botFindClosestEnemy(self, qfalse);
				if(tempEntityIndex >= 0)
					self->botEnemy = &g_entities[tempEntityIndex];
			}
			
			if(!self->botEnemy) {
				// no enemy
			} else {
				// enemy!
				distance = botGetDistanceBetweenPlayer(self, self->botEnemy);
				botAimAtTarget(self, self->botEnemy);
				
				// enable wallwalk
				if( BG_ClassHasAbility( self->client->ps.stats[ STAT_PCLASS ], SCA_WALLCLIMBER ) ) {
					self->client->pers.cmd.upmove = -1;
				}
				
				botShootIfTargetInRange(self,self->botEnemy);
			}
			
			break;
		/*	
		case BOT_DEFENSIVE:
			// if there is an enemy around, rush them but not too far from where you are standing when given this command
			break;
		*/	
		case BOT_FOLLOW_FRIEND_PROTECT:
			// run towards friend, attack enemy
			break;
			
		case BOT_FOLLOW_FRIEND_ATTACK:
			// run with friend until enemy spotted, then rush enemy
			if(self->botEnemy) {
				// we already have an enemy. See if still in LOS.
				if(!botTargetInRange(self,self->botEnemy)) {
					// if it's been over clicksToStopChase clicks since we last seen him in LOS then do nothing, else follow him!
					if(self->botEnemyLastSeen > clicksToStopChase) {
						// forget him!
						self->botEnemy = NULL;
						self->botEnemyLastSeen = 0;
					} else {
						//chase him
						self->botEnemyLastSeen++;
					}
				} else {
					// we see him!
					self->botEnemyLastSeen = 0;
				}
				
				//if we are chasing enemy, reset counter for friend LOS .. if its true
				if(self->botEnemy) {
					if(botTargetInRange(self,self->botFriend)) {
						self->botFriendLastSeen = 0;
					} else {
						self->botFriendLastSeen++;
					}
				}
			}
			
			if(!self->botEnemy) {
				// try to find closest enemy
				tempEntityIndex = botFindClosestEnemy(self, qfalse);
				if(tempEntityIndex >= 0)
					self->botEnemy = &g_entities[tempEntityIndex];
			}
			
			if(!self->botEnemy) {
				// no enemy
				if(self->botFriend) {
					// see if our friend is in LOS
					if(botTargetInRange(self,self->botFriend)) {
						// go to him!
						followFriend = qtrue;
						self->botFriendLastSeen = 0;
					} else {
						// if it's been over clicksToStopChase clicks since we last seen him in LOS then do nothing, else follow him!
						if(self->botFriendLastSeen > clicksToStopChase) {
							// forget him!
							followFriend = qfalse;
						} else {
							self->botFriendLastSeen++;
							followFriend = qtrue;
						}
					}
					
					if(followFriend) {
						distance = botGetDistanceBetweenPlayer(self, self->botFriend);
						botAimAtTarget(self, self->botFriend);
						
						// enable wallwalk
						if( BG_ClassHasAbility( self->client->ps.stats[ STAT_PCLASS ], SCA_WALLCLIMBER ) ) {
							self->client->pers.cmd.upmove = -1;
						}
						
						//botShootIfTargetInRange(self,self->botEnemy);
						if(distance>tooCloseDistance) {
							self->client->pers.cmd.forwardmove = forwardMove;
							self->client->pers.cmd.rightmove = -100;
							if(self->client->time1000 >= 500)
								self->client->pers.cmd.rightmove = 100;
						}
					}
				}
			} else {
				// enemy!
				distance = botGetDistanceBetweenPlayer(self, self->botEnemy);
				botAimAtTarget(self, self->botEnemy);
				
				// enable wallwalk
				if( BG_ClassHasAbility( self->client->ps.stats[ STAT_PCLASS ], SCA_WALLCLIMBER ) ) {
					self->client->pers.cmd.upmove = -1;
				}
				
				botShootIfTargetInRange(self,self->botEnemy);
				self->client->pers.cmd.forwardmove = forwardMove;
				self->client->pers.cmd.rightmove = -100;
				if(self->client->time1000 >= 500)
					self->client->pers.cmd.rightmove = 100;
			}
			
			break;
			
		case BOT_FOLLOW_FRIEND_IDLE:
			// run with friend and stick with him no matter what. no attack mode.
			if(self->botFriend) {
				// see if our friend is in LOS
				if(botTargetInRange(self,self->botFriend)) {
					// go to him!
					followFriend = qtrue;
					self->botFriendLastSeen = 0;
				} else {
					// if it's been over clicksToStopChase clicks since we last seen him in LOS then do nothing, else follow him!
					if(self->botFriendLastSeen > clicksToStopChase) {
						// forget him!
						followFriend = qfalse;
					} else {
						//chase him
						self->botFriendLastSeen++;
						followFriend = qtrue;
					}
					
				}
				
				if(followFriend) {
					distance = botGetDistanceBetweenPlayer(self, self->botFriend);
					botAimAtTarget(self, self->botFriend);
					
					// enable wallwalk
					if( BG_ClassHasAbility( self->client->ps.stats[ STAT_PCLASS ], SCA_WALLCLIMBER ) ) {
						self->client->pers.cmd.upmove = -1;
					}
					
					//botShootIfTargetInRange(self,self->botFriend);
					if(distance>tooCloseDistance) {
						self->client->pers.cmd.forwardmove = forwardMove;
						//does an annoying zig-zagging. Took it out.
						//self->client->pers.cmd.rightmove = -100;
						//if(self->client->time1000 >= 500)
						//	self->client->pers.cmd.rightmove = 100;
					}
				}
			}
			
			break;
			
		case BOT_TEAM_KILLER:
			// attack enemies, then teammates!
			if(self->botEnemy) {
				// we already have an enemy. See if still in LOS.
				if(!botTargetInRange(self,self->botEnemy)) {
					// if it's been over clicksToStopChase clicks since we last seen him in LOS then do nothing, else follow him!
					if(self->botEnemyLastSeen > clicksToStopChase) {
						// forget him!
						self->botEnemy = NULL;
						self->botEnemyLastSeen = 0;
					} else {
						//chase him
						self->botEnemyLastSeen++;
					}
				} else {
					// we see him!
					self->botEnemyLastSeen = 0;
				}
			}
			
			if(!self->botEnemy) {
				// try to find closest enemy
				tempEntityIndex = botFindClosestEnemy(self, qtrue);
				if(tempEntityIndex >= 0)
					self->botEnemy = &g_entities[tempEntityIndex];
			}
			
			if(!self->botEnemy) {
				// no enemy, we're all alone :(
			} else {
				// enemy!
				distance = botGetDistanceBetweenPlayer(self, self->botEnemy);
				botAimAtTarget(self, self->botEnemy);
				
				// enable wallwalk
				if( BG_ClassHasAbility( self->client->ps.stats[ STAT_PCLASS ], SCA_WALLCLIMBER ) ) {
					self->client->pers.cmd.upmove = -1;
				}
				
				botShootIfTargetInRange(self,self->botEnemy);
				self->client->pers.cmd.forwardmove = forwardMove;
				self->client->pers.cmd.rightmove = -100;
				if(self->client->time1000 >= 500)
					self->client->pers.cmd.rightmove = 100;
			}
			
			break;
			
		default:
			// dunno.
			break;
	}
}
