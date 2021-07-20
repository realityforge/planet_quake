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

static int		g_numBots;
static char		*g_botInfos[MAX_BOTS];


int				g_numArenas;
static char		*g_arenaInfos[MAX_ARENAS];

vmCvar_t bot_minplayers;

#define MAX_NUMBER_BOTS 20

static botSpawnQueue_t	botSpawnQueue[BOT_SPAWN_QUEUE_DEPTH];




void G_BotDel(gentity_t *ent, int clientNum ) {
	gentity_t *bot;
	const char  *teamstring;
	bot = &g_entities[clientNum];
	if( !( bot->r.svFlags & SVF_BOT ) ) {
		trap_Printf( va("'^7%s^7' is not a bot\n", bot->client->pers.netname) );
		return;
	}
	switch(bot->client->pers.teamSelection)
    {
      case PTE_ALIENS:
        teamstring = "^1Alien team^7";
        break;

      case PTE_HUMANS:
        teamstring = "^4Human team^7";
        break;
        
      case PTE_NONE:
        teamstring = "^2Spectator team^7";
        break;
       
      case PTE_NUM_TEAMS:
    	  teamstring = "Team error - you shouldn't see this";
    	  break;
    }
	level.numBots--;
    ClientDisconnect( clientNum );
	//this needs to be done to free up the client slot - I think - Ender Feb 18 2008
	trap_DropClient( clientNum, va( "was deleted by ^7%s^7 from the %s^7\n\"", ( ent ) ? ent->client->pers.netname : "The Console" , teamstring ) );
}

void G_BotRemoveAll( gentity_t *ent ) {
	int i;
	
	for( i = 0; i < level.num_entities; i++ ) {
		if( g_entities[ i ].r.svFlags & SVF_BOT ) {
			G_BotDel(ent, i);
		}
	}
	AP( va( "print \"^3!bot removeall: ^7%s^7 deleted all bots\n\"", ( ent ) ? ent->client->pers.netname : "The Console"));
}



/*
Depending on the flag set in bot->botCommand, control the behavior of the bot.
This function should go out and call the various functions in g_bothuman.c and
g_botalien.c
*/
void G_BotThink( gentity_t *self )
{
	//clear out the current bot commands
	//G_BotClearCommands( self );
		
	// what mode are we in?
	/*
	BOT_NONE = 1,				//does absolutely nothing
	BOT_TARGET_PRACTICE,		//move to a pre-specified location and stand there
	BOT_DODGE,					//move to a pre-specified location and dodge around
	BOT_DODGE_WITH_ATTACK,		//move to a pre-specified location and dodge around and fire on enemies
	BOT_ATTACK,					//attack the enemy
	BOT_DEFENSIVE,				//defend the base
	BOT_FOLLOW_FRIEND_PROTECT,	//follow a friend and protect them 
	BOT_FOLLOW_FRIEND_ATTACK,	//follow a friend and attack enemies
	BOT_FOLLOW_FRIEND_IDLE,	 	//follow a friend and do nothing
	BOT_BUILD					//build
	BOT_TEAM_KILLER				//shoot up your teammates
	*/
	

}

void G_BotSpectatorThink( gentity_t *self ) {
 if( self->client->ps.pm_flags & PMF_QUEUED) {
	  //we're queued to spawn, all good
	  return;
 }
 
 if( self->client->sess.sessionTeam == TEAM_SPECTATOR ) {
	int teamnum = self->client->pers.teamSelection;
	int clientNum = self->client->ps.clientNum;
	
   if( teamnum == PTE_HUMANS ) {
   	self->client->pers.classSelection = PCL_HUMAN;
   	self->client->ps.stats[ STAT_PCLASS ] = PCL_HUMAN;
   	self->client->pers.humanItemSelection = WP_MACHINEGUN;
   	G_PushSpawnQueue( &level.humanSpawnQueue, clientNum );
	} else if( teamnum == PTE_ALIENS) { //Auriga: a hack atm, needs to be removed when aliens work correctly
		self->client->pers.classSelection = PCL_ALIEN_LEVEL0;
		self->client->ps.stats[ STAT_PCLASS ] = PCL_ALIEN_LEVEL0;
		G_PushSpawnQueue( &level.alienSpawnQueue, clientNum );
	}
 }
}

qboolean botAimAtTarget( gentity_t *self, gentity_t *target ) {
	vec3_t dirToTarget, angleToTarget;
	vec3_t top = { 0, 0, 0};
	int vh = 0;
	BG_FindViewheightForClass(  self->client->ps.stats[ STAT_PCLASS ], &vh, NULL );
	top[2]=vh;
	VectorAdd( self->s.pos.trBase, top, top);
	VectorSubtract( target->s.pos.trBase, top, dirToTarget );
	VectorNormalize( dirToTarget );
	vectoangles( dirToTarget, angleToTarget );
 	self->client->ps.delta_angles[ 0 ] = ANGLE2SHORT( angleToTarget[ 0 ] );
	self->client->ps.delta_angles[ 1 ] = ANGLE2SHORT( angleToTarget[ 1 ] );
	self->client->ps.delta_angles[ 2 ] = ANGLE2SHORT( angleToTarget[ 2 ] );
	return qtrue;
}

qboolean botTargetInRange( gentity_t *self, gentity_t *target ) {
	trace_t   trace;
	gentity_t *traceEnt;
	//int myGunRange;
	//myGunRange = MGTURRET_RANGE * 3;
	
	if( !self || !target )
		return qfalse;

	if( !self->client || !target->client )
		return qfalse;

	if( target->client->ps.stats[ STAT_STATE ] & SS_HOVELING )
		return qfalse;

	if( target->health <= 0 )
		return qfalse;
	
	//if( Distance( self->s.pos.trBase, target->s.pos.trBase ) > myGunRange )
	//	return qfalse;

	//draw line between us and the target and see what we hit
	trap_Trace( &trace, self->s.pos.trBase, NULL, NULL, target->s.pos.trBase, self->s.number, MASK_SHOT );
	traceEnt = &g_entities[ trace.entityNum ];
	
	// check that we hit a human and not an object
	//if( !traceEnt->client )
	//	return qfalse;
	
	//check our target is in LOS
	if(!(traceEnt == target))
		return qfalse;

	return qtrue;
}

int botFindClosestEnemy( gentity_t *self, qboolean includeTeam ) {
	// return enemy entity index, or -1
	int vectorRange = MGTURRET_RANGE * 3;	
	int i;
	int total_entities;
	int entityList[ MAX_GENTITIES ];
	vec3_t    range;
	vec3_t    mins, maxs;
	gentity_t *target;
	
	VectorSet( range, vectorRange, vectorRange, vectorRange );
	VectorAdd( self->client->ps.origin, range, maxs );
	VectorSubtract( self->client->ps.origin, range, mins );
	
	total_entities = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );
	
	// check list for enemies
	for( i = 0; i < total_entities; i++ ) {
		target = &g_entities[ entityList[ i ] ];

		if( target->client && self != target && target->client->ps.stats[ STAT_PTEAM ] != self->client->ps.stats[ STAT_PTEAM ] ) {
			// aliens ignore if it's in LOS because they have radar
			if(self->client->ps.stats[ STAT_PTEAM ] == PTE_ALIENS) {
				trap_SendServerCommand(-1, "print \"Found enemy, I'm on it.\n\"");
				return entityList[ i ];
			} else {
				if( botTargetInRange( self, target ) ) {
					trap_SendServerCommand(-1, "print \"I see an enemy, going for it.\n\"");
					return entityList[ i ];
				}
			}
		}
	}
	
	if(includeTeam) {
		// check list for enemies in team
		for( i = 0; i < total_entities; i++ ) {
			target = &g_entities[ entityList[ i ] ];
			
			if( target->client && self !=target && target->client->ps.stats[ STAT_PTEAM ] == self->client->ps.stats[ STAT_PTEAM ] ) {
				// aliens ignore if it's in LOS because they have radar
				if(self->client->ps.stats[ STAT_PTEAM ] == PTE_ALIENS) {
					return entityList[ i ];
				} else {
					if( botTargetInRange( self, target ) ) {
						return entityList[ i ];
					}
				}
			}
		}
	}
	
	return -1;
}

// really an int? what if it's too long?
int botGetDistanceBetweenPlayer( gentity_t *self, gentity_t *player ) {
	return Distance( self->s.pos.trBase, player->s.pos.trBase );
}

qboolean botShootIfTargetInRange( gentity_t *self, gentity_t *target ) {
	if(botTargetInRange(self,target)) {
		self->client->pers.cmd.buttons |= BUTTON_ATTACK;
		return qtrue;
	}
	return qfalse;
}


/*
===============
G_RemoveQueuedBotBegin

Called on client disconnect to make sure the delayed spawn
doesn't happen on a freed index
===============
*/
void G_RemoveQueuedBotBegin( int clientNum ) {
	int		n;

	for( n = 0; n < BOT_SPAWN_QUEUE_DEPTH; n++ ) {
		if( botSpawnQueue[n].clientNum == clientNum ) {
			botSpawnQueue[n].spawnTime = 0;
			return;
		}
	}
}

/*
===============
G_AddRandomBot
===============
*/
void G_AddRandomBot(gentity_t *ent, int team , char *name) {
	int		i, n, num;
	float	skill;
	char	*value, netname[36], *teamstr;
	char			*botinfo;
	gclient_t	*cl;

	num = 0;
	for ( n = 0; n < g_numBots ; n++ ) {
		value = Info_ValueForKey( g_botInfos[n], "name" );
		//
		for ( i=0 ; i< g_maxclients.integer ; i++ ) {
			cl = level.clients + i;
			if ( cl->pers.connected != CON_CONNECTED ) {
				continue;
			}
			if ( !(g_entities[cl->ps.clientNum].r.svFlags & SVF_BOT) ) {
				continue;
			}
			if ( team >= 0 && cl->sess.sessionTeam != team ) {
				continue;
			}
			if ( !Q_stricmp( value, cl->pers.netname ) ) {
				break;
			}
		}
		if (i >= g_maxclients.integer) {
			num++;
		}
	}
	num = random() * num;
	for ( n = 0; n < g_numBots ; n++ ) {
		value = Info_ValueForKey( g_botInfos[n], "name" );
		//
		for ( i=0 ; i< g_maxclients.integer ; i++ ) {
			cl = level.clients + i;
			if ( cl->pers.connected != CON_CONNECTED ) {
				continue;
			}
			if ( !(g_entities[cl->ps.clientNum].r.svFlags & SVF_BOT) ) {
				continue;
			}
			if ( team >= 0 && cl->sess.sessionTeam != team ) {
				continue;
			}
			if ( !Q_stricmp( value, cl->pers.netname ) ) {
				break;
			}
		}
		if (i >= g_maxclients.integer) {
			num--;
			if (num <= 0) {
				skill = 2; //trap_Cvar_VariableValue( "g_spSkill" );
				if (team == PTE_ALIENS) teamstr = "aliens";
				else if (team == PTE_HUMANS) teamstr = "humans";
				else teamstr = "";
				
				strncpy(netname, value, sizeof(netname)-1);
				netname[sizeof(netname)-1] = '\0';
				Q_CleanStr(netname);
				botinfo = G_GetBotInfoByName( netname );
				G_BotAdd(ent, (name) ? name: netname, team, skill, botinfo);
				//trap_SendConsoleCommand( EXEC_INSERT, va("addbot %s %f %s %i\n", netname, skill, teamstr, 0) );
				return;
			}
		}
	}
}	

/*
===============
G_RemoveRandomBot
===============
*/
int G_RemoveRandomBot( int team ) {
	gclient_t	*cl;
	int i;

	for ( i=0 ; i< g_maxclients.integer ; i++ ) {
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		if ( !(g_entities[cl->ps.clientNum].r.svFlags & SVF_BOT) ) {
			continue;
		}
		if ( team >= 0 && cl->ps.stats[ STAT_PTEAM ] != team ) {
			continue;
		}
		//strcpy(netname, cl->pers.netname);
		//Q_CleanStr(netname);
		//trap_SendConsoleCommand( EXEC_INSERT, va("kick %s\n", netname) );
		trap_SendConsoleCommand( EXEC_INSERT, va("clientkick %d\n", cl->ps.clientNum) );
		return qtrue;
	}
	return qfalse;
}

/*
===============
G_CountHumanPlayers
===============
*/
int G_CountHumanPlayers( int team ) {
	int i, num;
	gclient_t	*cl;

	num = 0;
	for ( i=0 ; i< g_maxclients.integer ; i++ ) {
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		if ( g_entities[cl->ps.clientNum].r.svFlags & SVF_BOT ) {
			continue;
		}
		if ( team >= 0 && cl->ps.stats[ STAT_PTEAM ] != team ) {
			continue;
		}
		num++;
	}
	return num;
}

/*
===============
G_CountBotPlayers
===============
*/
int G_CountBotPlayers( int team ) {
	int i, n, num;
	gclient_t	*cl;

	num = 0;
	for ( i=0 ; i< g_maxclients.integer ; i++ ) {
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		if ( !(g_entities[cl->ps.clientNum].r.svFlags & SVF_BOT) ) {
			continue;
		}
		if ( team >= 0 && cl->ps.stats[ STAT_PTEAM ] != team ) {
			continue;
		}
		num++;
	}
	for( n = 0; n < BOT_SPAWN_QUEUE_DEPTH; n++ ) {
		if( !botSpawnQueue[n].spawnTime ) {
			continue;
		}
		if ( botSpawnQueue[n].spawnTime > level.time ) {
			continue;
		}
		num++;
	}
	return num;
}

void G_CheckMinimumPlayers( void ) {
	int minaliens, minhumans;
	int humanplayers, botplayers;
	static int checkminimumplayers_time;

	if (level.intermissiontime) return;
	
	//only check once each 5 seconds
	if (checkminimumplayers_time > level.time - 1000) {
		return;
	}
	checkminimumplayers_time = level.time;
	
	// check alien team
	trap_Cvar_Update(&bot_minaliens);
	minaliens = bot_minaliens.integer;
	if (minaliens <= 0) return;
	
	if (minaliens >= g_maxclients.integer / 2) {
		minaliens = (g_maxclients.integer / 2) -1;
	}

	humanplayers = G_CountHumanPlayers( PTE_ALIENS );
	botplayers = G_CountBotPlayers(	PTE_ALIENS );
	//
	if (humanplayers + botplayers < minaliens ) {
		G_AddRandomBot(NULL, PTE_ALIENS, NULL );
	} else if (humanplayers + botplayers > minaliens && botplayers) {
		G_RemoveRandomBot( PTE_ALIENS );
	}
	
	// check human team
	trap_Cvar_Update(&bot_minhumans);
	minhumans = bot_minhumans.integer;
	if (minhumans <= 0) return;
	
	if (minhumans >= g_maxclients.integer / 2) {
		minhumans = (g_maxclients.integer / 2) -1;
	}
	humanplayers = G_CountHumanPlayers( PTE_HUMANS );
	botplayers = G_CountBotPlayers( PTE_HUMANS );
	//
	if (humanplayers + botplayers < minhumans ) {
		G_AddRandomBot(NULL, PTE_HUMANS , NULL);
	} else if (humanplayers + botplayers > minhumans && botplayers) {
		G_RemoveRandomBot( PTE_HUMANS );
	}
}


void G_CheckBotSpawn( void ) {
	int		n;

	G_CheckMinimumPlayers();

	for( n = 0; n < BOT_SPAWN_QUEUE_DEPTH; n++ ) {
		if( !botSpawnQueue[n].spawnTime ) {
			continue;
		}
		if ( botSpawnQueue[n].spawnTime > level.time ) {
			continue;
		}
		ClientBegin( botSpawnQueue[n].clientNum );
		botSpawnQueue[n].spawnTime = 0;
	}
}

/*
===============
G_BotConnect
===============
*/
qboolean G_BotConnect( int clientNum, qboolean restart ) {
	bot_settings_t	settings;
	char			teamstr[MAX_FILEPATH];
	char			userinfo[MAX_INFO_STRING];
	int 			team;

	trap_GetUserinfo( clientNum, userinfo, sizeof(userinfo) );

	Q_strncpyz( settings.characterfile, Info_ValueForKey( userinfo, "characterfile" ), sizeof(settings.characterfile) );
	settings.skill = atof( Info_ValueForKey( userinfo, "skill" ) );
	Q_strncpyz( settings.team, Info_ValueForKey( userinfo, "team" ), sizeof(settings.team) );
	
	// At this point, it's quite possible that the bot doesn't have a team, as in it
	// was just created
	if( !Q_stricmp( teamstr, "aliens" ) )
		team = PTE_ALIENS;
	else if( !Q_stricmp( teamstr, "humans" ) )
		team = PTE_HUMANS;
	else{
		team = PTE_NONE;
	}
		
	Com_Printf("Trying BotAISetupClient\n");
	if (!BotAISetupClient( clientNum, &settings, restart )) {
		trap_DropClient( clientNum, "BotAISetupClient failed" );
		return qfalse;
	}

	return qtrue;
}

// Start of bot charactor stuff

/*
===============
G_ParseInfos
===============
*/
int G_ParseInfos( char *buf, int max, char *infos[] ) {
	char	*token;
	int		count;
	char	key[MAX_TOKEN_CHARS];
	char	info[MAX_INFO_STRING];

	count = 0;

	while ( 1 ) {
		token = COM_Parse( &buf );
		if ( !token[0] ) {
			break;
		}
		if ( strcmp( token, "{" ) ) {
			Com_Printf( "Missing { in info file\n" );
			break;
		}

		if ( count == max ) {
			Com_Printf( "Max infos exceeded\n" );
			break;
		}

		info[0] = '\0';
		while ( 1 ) {
			token = COM_ParseExt( &buf, qtrue );
			if ( !token[0] ) {
				Com_Printf( "Unexpected end of info file\n" );
				break;
			}
			if ( !strcmp( token, "}" ) ) {
				break;
			}
			Q_strncpyz( key, token, sizeof( key ) );

			token = COM_ParseExt( &buf, qfalse );
			if ( !token[0] ) {
				strcpy( token, "<NULL>" );
			}
			Info_SetValueForKey( info, key, token );
		}
		//NOTE: extra space for arena number
		infos[count] = G_Alloc(strlen(info) + strlen("\\num\\") + strlen(va("%d", MAX_ARENAS)) + 1);
		if (infos[count]) {
			strcpy(infos[count], info);
			count++;
		}
	}
	return count;
}

/*
===============
G_LoadBotsFromFile
===============
*/
static void G_LoadBotsFromFile( char *filename ) {
	int				len;
	fileHandle_t	f;
	char			buf[MAX_BOTS_TEXT];

	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( !f ) {
		trap_Printf( va( S_COLOR_RED "file not found: %s\n", filename ) );
		return;
	}
	if ( len >= MAX_BOTS_TEXT ) {
		trap_Printf( va( S_COLOR_RED "file too large: %s is %i, max allowed is %i", filename, len, MAX_BOTS_TEXT ) );
		trap_FS_FCloseFile( f );
		return;
	}

	trap_FS_Read( buf, len, f );
	buf[len] = 0;
	trap_FS_FCloseFile( f );

	g_numBots += G_ParseInfos( buf, MAX_BOTS - g_numBots, &g_botInfos[g_numBots] );
}

/*
===============
G_LoadBots
===============
*/
static void G_LoadBots( void ) {
	int			numdirs;
	char		filename[128];
	char		dirlist[1024];
	char*		dirptr;
	int			i;
	int			dirlen;

	if ( !trap_Cvar_VariableIntegerValue( "bot_enable" ) ) {
		return;
	}

	g_numBots = 0;

	if( *botsFile.string ) {
		G_LoadBotsFromFile(botsFile.string);
	}
	else {
		G_LoadBotsFromFile("scripts/bots.txt");
	}

	// get all bots from .bot files
	numdirs = trap_FS_GetFileList("scripts", ".bot", dirlist, 1024 );
	dirptr  = dirlist;
	for (i = 0; i < numdirs; i++, dirptr += dirlen+1) {
		dirlen = strlen(dirptr);
		strcpy(filename, "scripts/");
		strcat(filename, dirptr);
		G_LoadBotsFromFile(filename);
	}
	trap_Printf( va( "%i bots parsed\n", g_numBots ) );
}



/*
===============
G_GetBotInfoByNumber
===============
*/
char *G_GetBotInfoByNumber( int num ) {
	if( num < 0 || num >= g_numBots ) {
		trap_Printf( va( S_COLOR_RED "Invalid bot number: %i\n", num ) );
		return NULL;
	}
	return g_botInfos[num];
}


/*
===============
G_GetBotInfoByName
===============
*/
char *G_GetBotInfoByName( const char *name ) {
	int		n;
	char	*value;

	Com_Printf(" Searching %i bot infos for '%s'...\n", g_numBots, name);
	for ( n = 0; n < g_numBots ; n++ ) {
		value = Info_ValueForKey( g_botInfos[n], "name" );
		if ( !Q_stricmp( value, name ) ) {
			return g_botInfos[n];
		} else {
			Com_Printf("Bot is not %s...\n", value);
			
		}
	}

	return NULL;
}

/*
===============
G_InitBots
===============
*/
void G_InitBots( qboolean restart ) {
	int			fragLimit;
	int			timeLimit;
	const char	*arenainfo;
	char		*strValue;
	int			basedelay;
	char		map[MAX_QPATH];
	char		serverinfo[MAX_INFO_STRING];

	if(!restart) {
	Com_Printf("------------- Loading Bots --------------\n");
	G_LoadBots();
	Com_Printf("-----------------------------------------\n");
	}
	//G_LoadArenas();
}

void G_BotAdd( gentity_t *ent, char *name, int team, int skill, char *botinfo ) {
	int i, botsuffixnumber;
	int clientNum;
	//char			*botinfo;
	char userinfo[MAX_INFO_STRING];
	char err[ MAX_STRING_CHARS ];
	char newname[ MAX_NAME_LENGTH ];
	int reservedSlots = 0;
	char  *teamstring, *teamstr;
	gentity_t *bot;
	
	Com_Printf("Trying to add bot '%s'...\n", name);
	reservedSlots = trap_Cvar_VariableIntegerValue( "sv_privateclients" );
	/*
	// get the botinfo from bots.txt
	botinfo = G_GetBotInfoByName( name );
	if ( !botinfo ) {
		G_Printf( S_COLOR_RED "Error: Bot '%s' not defined\n", name );
		return;
	}*/
	
	// find what clientNum to use for bot
	clientNum = trap_BotAllocateClient();
	if ( clientNum == -1 ) {
		G_Printf( S_COLOR_RED "Unable to add bot.  All player slots are in use.\n" );
		G_Printf( S_COLOR_RED "Start server with more 'open' slots (or check setting of sv_maxclients cvar).\n" );
		return;
	}
	newname[0] = '\0';
	switch(team)
  {
    case PTE_ALIENS:
      teamstring = "^1Alien team^7";      
      teamstr = "aliens";
      Q_strcat( newname, sizeof(newname), "^1[BOT]^7");
      Q_strcat( newname, MAX_NAME_LENGTH, name );
      break;

    case PTE_HUMANS:
      teamstring = "^4Human team";
      teamstr = "humans";
      Q_strcat( newname, sizeof(newname), "^4[BOT]^7");
            Q_strcat( newname, MAX_NAME_LENGTH, name );
            break;
  }
	//now make sure that we can add bots of the same name, but just incremented
	//numerically. We'll now use name as a temp buffer, since we have the
	//real name in newname.
	botsuffixnumber = 1;
    if (!G_admin_name_check( NULL, newname, err, sizeof( err ) )){
      while( botsuffixnumber < MAX_NUMBER_BOTS )
        {
          strcpy( name, va( "%s%d", newname, botsuffixnumber ) );
          if ( G_admin_name_check( NULL, name, err, sizeof( err ) ) )
          {
        	  strcpy( newname, name );
              break;
          }
          botsuffixnumber++; // Only increments if the last requested name was used.
        }
  }
	
	bot = &g_entities[ clientNum ];
	bot->r.svFlags |= SVF_BOT;
	bot->inuse = qtrue;
	
	// register user information
	userinfo[0] = '\0';
	Info_SetValueForKey( userinfo, "characterfile", Info_ValueForKey( botinfo, "aifile" ) );
	Info_SetValueForKey( userinfo, "name", newname );
	Info_SetValueForKey( userinfo, "rate", "25000" );
	Info_SetValueForKey( userinfo, "snaps", "20" );
	Info_SetValueForKey( userinfo, "skill", va("%d", skill ) );
	Info_SetValueForKey( userinfo, "teamstr", teamstr );

	trap_SetUserinfo( clientNum, userinfo );

	// have it connect to the game as a normal client
	if(ClientConnect(clientNum, qtrue, qtrue) != NULL ) {
		// won't let us join
		
		return;
	}
	if( team == PTE_HUMANS)
		AP( va( "print \"^3!bot add: ^7%s^7 added bot: %s^7 to the %s^7 with character: %s and with skill level: %d\n\"", ( ent ) ? ent->client->pers.netname : "The Console" , bot->client->pers.netname, teamstring,Info_ValueForKey( botinfo, "name" ) , skill ));
	else {
		AP( va( "print \"^3!bot add: ^7%s^7 added bot: %s^7 to the %s^7\n\"", ( ent ) ? ent->client->pers.netname : "The Console" , bot->client->pers.netname, teamstring));
	}
	BotBegin( clientNum );
	G_ChangeTeam( bot, team );
	level.numBots++;
}