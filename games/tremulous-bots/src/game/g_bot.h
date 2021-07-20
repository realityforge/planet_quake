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
#include "../botlib/botlib.h"

#define BOT_SPAWN_QUEUE_DEPTH	16

typedef struct {
	int		clientNum;
	int		spawnTime;
} botSpawnQueue_t;


//g_bot.c functions
void G_BotAdd( gentity_t *ent, char *name, int team, int skill, char *botinfo );
void G_BotDel( gentity_t *ent, int clientNum );

void G_BotThink( gentity_t *self );
void G_BotSpectatorThink( gentity_t *self );
void G_BotClearCommands( gentity_t *self );
qboolean botAimAtTarget( gentity_t *self, gentity_t *target );
qboolean botTargetInRange( gentity_t *self, gentity_t *target );
int botFindClosestEnemy( gentity_t *self, qboolean includeTeam );
int botGetDistanceBetweenPlayer( gentity_t *self, gentity_t *player );
void G_RemoveQueuedBotBegin( int clientNum );
void G_AddRandomBot(gentity_t *ent, int team , char *name);
int G_RemoveRandomBot( int team );
int G_CountHumanPlayers( int team );
int G_CountBotPlayers( int team );
void G_CheckMinimumPlayers( void );
void G_CheckBotSpawn( void );
qboolean G_BotConnect( int clientNum, qboolean restart );
char *G_GetBotInfoByName( const char *name );


/* g_bothuman.c functions */

/* g_botalien.c functions */

/* g_botutils.c functions */
//void botClearCommands( gentity_t* self );


