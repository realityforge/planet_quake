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

#ifndef BLIB_HEADERS
#define BLIB_HEADERS
#include "../botlib/botlib.h"

#include "../botlib/be_aas.h"		// TFL_*
#include "../botlib/be_ai_goal.h"	// bot_goal_t
#include "../botlib/be_ai_move.h"	// bot_initmove_t
#include "../botlib/be_ai_weap.h"	// weaponinfo_t
#include "chars.h"				//characteristics
#endif

#define MAX_ITEMS					256
extern float floattime;
#define FloatTime() floattime
//bot flags
#define BFL_STRAFERIGHT				1	//strafe to the right
#define BFL_ATTACKED				2	//bot has attacked last ai frame
#define BFL_ATTACKJUMPED			4	//bot jumped during attack last frame
#define BFL_AIMATENEMY				8	//bot aimed at the enemy this frame
#define BFL_AVOIDRIGHT				16	//avoid obstacles by going to the right
#define BFL_IDEALVIEWSET			32	//bot has ideal view angles set
#define BFL_FIGHTSUICIDAL			64	//bot is in a suicidal fight

#define IDEAL_ATTACKDIST			140

//copied from the aas file header
#define PRESENCE_NONE				1
#define PRESENCE_NORMAL				2
#define PRESENCE_CROUCH				4

extern vmCvar_t bot_developer;

// print types
typedef enum{
	BPMSG,
	BPERROR,
	BPDEBUG
}botprint_t;

// inventory
typedef enum{
	BI_HEALTH,
	BI_CREDITS,		// evos for alien bots
	BI_WEAPON,
	BI_AMMO,
	BI_CLIPS,		
	BI_STAMINA,		// == boost time for aliens?
	BI_GRENADE,
	BI_MEDKIT,
	BI_JETPACK,
	BI_BATTPACK,
	BI_LARMOR,
	BI_HELMET,
	BI_BSUIT,
	BI_SIZE,
	BI_CLASS
} bot_inventory_t;

typedef enum
{
	NEXT_STATE_NONE, 
  NEXT_STATE_LOS, // Go to next state when u can see the goal
  NEXT_STATE_DISTANCE //Go to the next state when u are close enough to the goal.
} nextstatetype_t;

//bot state
typedef struct bot_state_s
{
	int inuse;										//true if this state is used by a bot client
	int botthink_residual;							//residual for the bot thinks
	int client;										//client number of the bot
	int entitynum;									//entity number of the bot
	playerState_t cur_ps;							//current player state
	int last_eFlags;								//last ps flags
	usercmd_t lastucmd;								//usercmd from last frame
	int entityeventTime[1024];						//last entity event time
	//
	float thinktime;								//time the bot thinks this frame
	float ltime;
	vec3_t origin;									//origin of the bot
	vec3_t velocity;								//velocity of the bot
	vec3_t viewangles;
	vec3_t ideal_viewangles;
	int presencetype;								//presence type of the bot
	vec3_t eye;										//eye coordinates of the bot
	int areanum;									//the number of the area the bot is in
	int inventory[BI_SIZE];							//string with items amounts the bot has
	int tfl;										//the travel flags the bot uses
	int flags;
	
	int lasthealth;									//health value previous frame
	int lastkilledplayer;							//last killed player
	int lastkilledby;								//player that last killed this bot
	
	//
	int character;									//the bot character
	int ms;											//move state of the bot
	int gs;											//goal state of the bot
	int cs;											//chat state of the bot
	int ws;											//weapon state of the bot
	
	int setupcount;
	int botdeathtype;								//the death type of the bot
	int enemydeathtype;								//the death type of the enemy
	
	float notblocked_time;							//last time the bot was not blocked
	float entergame_time;
	int team;
	int enemy;
	gentity_t *enemyent;
	gentity_t *ent;
	float attackstrafe_time;						//time the bot is strafing in one dir
	int state;
	int nextstate;
	nextstatetype_t nextstatetype;
	float nextstatetypelosdist;
	int statecycles;
	char hudinfo[MAX_INFO_STRING];
	bot_goal_t goal;
	vec3_t aimtarget;
	vec3_t enemyvelocity;							//enemy velocity 0.5 secs ago during battle
	vec3_t enemyorigin;								//enemy origin 0.5 secs ago during battle
	float enemysight_time;							//time before reacting to enemy
	float teleport_time;							//last time the bot teleported
	float killedenemy_time;							//time the bot killed the enemy
	float enemyposition_time;						//time the position and velocity of the enemy were stored
	float weaponchange_time;						//time the bot started changing weapons
	float firethrottlewait_time;					//amount of time to wait
	float firethrottleshoot_time;					//amount of time to shoot
	int chatto;										//chat to all or team
	int botsuicide;									//true when the bot suicides
	int enemysuicide;								//true when the enemy of the bot suicides
	int num_deaths;									//number of time this bot died
	int num_kills;									//number of kills of this bot
	float lastchat_time;							//time the bot last selected a chat
} bot_state_t;

// ai_utils.c
void BotChangeViewAngles(bot_state_t *bs, float thinktime);
int BotPointAreaNum(vec3_t origin);
void QDECL Bot_Print(botprint_t type, char *fmt, ...);
void BotShowViewAngles(bot_state_t* bs);
void BotTestAAS(vec3_t origin);
int BotAI_GetClientState( int clientNum, playerState_t *state );
void BotSetupForMovement(bot_state_t *bs);
qboolean BotIsAlive(bot_state_t* bs);
qboolean BotIntermission(bot_state_t *bs);
qboolean BotGoalForBuildable(bot_state_t* bs, bot_goal_t* goal, int bclass);
qboolean BotGoalForNearestEnemy(bot_state_t* bs, bot_goal_t* goal);
void BotAddInfo(bot_state_t* bs, char* key, char* value );
char *ClientName(int client, char *name, int size);
int BotFindEnemy(bot_state_t *bs, int curenemy);
qboolean EntityIsInvisible(aas_entityinfo_t *entinfo);

//int CheckAreaForGoal(vec3_t origin);
qboolean BotInFieldOfVision(vec3_t viewangles, float fov, vec3_t angles);
float BotEntityVisible(int viewer, vec3_t eye, vec3_t viewangles, float fov, int ent);
int BotSameTeam(bot_state_t *bs, int entnum);
float BotEntityDistance(bot_state_t* bs, int ent);
void	BotAI_Trace(bsp_trace_t *bsptrace, vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, int passent, int contentmask);
qboolean BotGoalForClosestBuildable(bot_state_t* bs, bot_goal_t* goal, int bclass);
qboolean BotGoalForNearestEnemy(bot_state_t* bs, bot_goal_t* goal);

// ai_main.c
void BotEntityInfo(int entnum, aas_entityinfo_t *info);
void BotCheckDeath(int target, int attacker, int mod);
void BotAIBlocked(bot_state_t *bs, bot_moveresult_t *moveresult, int activate);

// ai_human.c
void BotHumanAI(bot_state_t* bs, float thinktime);


// ai_alien.c
void BotAlienAI(bot_state_t* bs, float thinktime);

//when human bots reach this ammo percentage left or less(and no enemy), they will head back to the base to refuel ammo when in range of arm as defined by BOT_ARM_RANGE
#define BOT_LOW_AMMO 0.50f