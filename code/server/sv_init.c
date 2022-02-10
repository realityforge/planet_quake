/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "server.h"

qboolean svShuttingDown = qfalse;

extern void Cvar_SetServerDescriptions( void );
extern void Cvar_SetBotDescriptions( void );

#ifndef BUILD_SLIM_CLIENT

/*
===============
SV_SendConfigstring

Creates and sends the server command necessary to update the CS index for the
given client
===============
*/
static void SV_SendConfigstring(client_t *client, int index)
{
	int maxChunkSize = MAX_STRING_CHARS - 24;
	int len;

	len = strlen(sv.configstrings[index]);

	if( len >= maxChunkSize ) {
		int		sent = 0;
		int		remaining = len;
		char	*cmd;
		char	buf[MAX_STRING_CHARS];

		while (remaining > 0 ) {
			if ( sent == 0 ) {
				cmd = "bcs0";
			}
			else if( remaining < maxChunkSize ) {
				cmd = "bcs2";
			}
			else {
				cmd = "bcs1";
			}
			Q_strncpyz( buf, &sv.configstrings[index][sent],
				maxChunkSize );

			SV_SendServerCommand( client, "%s %i \"%s\"", cmd,
				index, buf );

			sent += (maxChunkSize - 1);
			remaining -= (maxChunkSize - 1);
		}
	} else {
		// standard cs, just send it
		SV_SendServerCommand( client, "cs %i \"%s\"", index,
			sv.configstrings[index] );
	}
}

/*
===============
SV_UpdateConfigstrings

Called when a client goes from CS_PRIMED to CS_ACTIVE.  Updates all
Configstring indexes that have changed while the client was in CS_PRIMED
===============
*/
void SV_UpdateConfigstrings(client_t *client)
{
	int index;

	for( index = 0; index < MAX_CONFIGSTRINGS; index++ ) {
		// if the CS hasn't changed since we went to CS_PRIMED, ignore
		if(!client->csUpdated[index])
			continue;

		// do not always send server info to all clients
		if ( index == CS_SERVERINFO && ( SV_GentityNum( client - svs.clients )->r.svFlags & SVF_NOSERVERINFO ) ) {
			continue;
		}

		SV_SendConfigstring(client, index);
		client->csUpdated[index] = qfalse;
	}
}

/*
===============
SV_SetConfigstring

===============
*/
void SV_SetConfigstring (int index, const char *val) {
	int		i;
	client_t	*client;

	if ( index < 0 || index >= MAX_CONFIGSTRINGS ) {
		Com_Error (ERR_DROP, "SV_SetConfigstring: bad index %i", index);
	}

	if ( !val ) {
		val = "";
	}

	// don't bother broadcasting an update if no change
	if ( !strcmp( val, sv.configstrings[ index ] ) ) {
		return;
	}

	// change the string in sv
  if(sv.configstrings[index])
    Z_Free( sv.configstrings[index] );
	sv.configstrings[index] = CopyString( val );

#ifdef USE_DEMO_SERVER
	// save config strings to demo
	if (sv.demoState == DS_RECORDING) {
		SV_DemoWriteConfigString( index, val );
	}
#endif

#ifdef USE_MULTIVM_SERVER
  if(sv.gentitySizes[gvmi] == 0) {
    // still starting up
    return;
  }
	if(index >= CS_PLAYERS && index < CS_PLAYERS + MAX_CLIENTS) {
		Com_Printf("SV_SetConfigstring: client configstring %i: %.*s\n", index,
			(int)strlen(sv.configstrings[index]), sv.configstrings[index]);
	}
#endif

	// send it to all the clients if we aren't
	// spawning a new server
	if ( sv.state == SS_GAME || sv.restarting ) {

		// send the data to all relevant clients
		for (i = 0, client = svs.clients; i < sv_maxclients->integer ; i++, client++) {
			if ( client->state < CS_ACTIVE ) {
				if ( client->state == CS_PRIMED )
					client->csUpdated[ index ] = qtrue;
				continue;
			}
			// do not always send server info to all clients
			if ( index == CS_SERVERINFO && ( SV_GentityNum( i )->r.svFlags & SVF_NOSERVERINFO ) ) {
				continue;
			}
		
			SV_SendConfigstring(client, index);
		}
	}
}


/*
===============
SV_GetConfigstring
===============
*/
void SV_GetConfigstring( int index, char *buffer, int bufferSize ) {
	if ( bufferSize < 1 ) {
		Com_Error( ERR_DROP, "SV_GetConfigstring: bufferSize == %i", bufferSize );
	}
	if ( index < 0 || index >= MAX_CONFIGSTRINGS ) {
		Com_Error (ERR_DROP, "SV_GetConfigstring: bad index %i", index);
	}
	if ( !sv.configstrings[index] ) {
		buffer[0] = '\0';
		return;
	}

	Q_strncpyz( buffer, sv.configstrings[index], bufferSize );
}


/*
===============
SV_SetUserinfo

===============
*/
void SV_SetUserinfo( int index, const char *val ) {
	if ( index < 0 || index >= sv_maxclients->integer ) {
		Com_Error (ERR_DROP, "SV_SetUserinfo: bad index %i", index);
	}

	if ( !val ) {
		val = "";
	}

#ifdef USE_DEMO_SERVER
	// Save userinfo changes to demo (also in SV_UpdateUserinfo_f() in sv_client.c)
	if ( sv.demoState == DS_RECORDING ) {
		SV_DemoWriteClientUserinfo( &svs.clients[index], val );
	}
#endif

	Q_strncpyz( svs.clients[index].userinfo, val, sizeof( svs.clients[ index ].userinfo ) );
	Q_strncpyz( svs.clients[index].name, Info_ValueForKey( val, "name" ), sizeof(svs.clients[index].name) );
}



/*
===============
SV_GetUserinfo

===============
*/
void SV_GetUserinfo( int index, char *buffer, int bufferSize ) {
	if ( bufferSize < 1 ) {
		Com_Error( ERR_DROP, "SV_GetUserinfo: bufferSize == %i", bufferSize );
	}
	if ( index < 0 || index >= sv_maxclients->integer ) {
		Com_Error (ERR_DROP, "SV_GetUserinfo: bad index %i", index);
	}
	Q_strncpyz( buffer, svs.clients[ index ].userinfo, bufferSize );
}


/*
================
SV_CreateBaseline

Entity baselines are used to compress non-delta messages
to the clients -- only the fields that differ from the
baseline will be transmitted
================
*/
void SV_CreateBaseline( void ) {
	sharedEntity_t *ent;
	int				entnum;	

#ifdef USE_MULTIVM_SERVER
  for ( entnum = 0; entnum < sv.num_entitiesWorlds[gvmi] ; entnum++ ) {
#else
	for ( entnum = 0; entnum < sv.num_entities ; entnum++ ) {
#endif
		ent = SV_GentityNum( entnum );
		if ( !ent->r.linked ) {
			continue;
		}
		ent->s.number = entnum;

		//
		// take current state as baseline
		//
		sv.svEntities[ entnum ].baseline = ent->s;
		sv.baselineUsed[ entnum ] = 1;
	}
}


/*
===============
SV_BoundMaxClients
===============
*/
void SV_BoundMaxClients( int minimum ) {
	// get the current maxclients value
	Cvar_Get( "sv_maxclients", "8", 0 );

	sv_maxclients->modified = qfalse;

	if ( sv_maxclients->integer < minimum ) {
		Cvar_Set( "sv_maxclients", va("%i", minimum) );
	}
	
#ifdef USE_MV
	SV_MV_BoundMaxClients();
#endif
}


/*
===============
SV_SetSnapshotParams
===============
*/
void SV_SetSnapshotParams( void ) 
{
#ifdef USE_MV
	SV_MV_SetSnapshotParams();
#endif
	// PACKET_BACKUP frames is just about 6.67MB so use that even on listen servers
	svs.numSnapshotEntities = PACKET_BACKUP * MAX_GENTITIES;
}


/*
===============
SV_Startup

Called when a host starts a map when it wasn't running
one before.  Successive map or map_restart commands will
NOT cause this to be called, unless the game is exited to
the menu system first.
===============
*/
static void SV_Startup( void ) {
	if ( svs.initialized ) {
		Com_Error( ERR_FATAL, "SV_Startup: svs.initialized" );
	}
	SV_BoundMaxClients( 1 );

#ifdef USE_LNBITS
	maxInvoices = Z_Malloc( ( sv_maxclients->integer + 10 ) * sizeof(invoice_t) );
	Com_Memset( maxInvoices, 0, ( sv_maxclients->integer + 10 ) * sizeof(invoice_t) );
	numInvoices = 0;
#endif
#ifdef USE_MV
	svs.clients = Z_TagMalloc( ( sv_maxclients->integer + 1 ) * sizeof( client_t ), TAG_CLIENTS ); // +1 client slot for recorder
	Com_Memset( svs.clients, 0, ( sv_maxclients->integer + 1 ) * sizeof( client_t ) );
#else
	svs.clients = Z_TagMalloc( sv_maxclients->integer * sizeof( client_t ), TAG_CLIENTS );
	Com_Memset( svs.clients, 0, sv_maxclients->integer * sizeof( client_t ) );
#endif
	SV_SetSnapshotParams();
	svs.initialized = qtrue;

	// Don't respect sv_killserver unless a server is actually running
	if ( sv_killserver->integer ) {
		Cvar_Set( "sv_killserver", "0" );
	}

	Cvar_Set( "sv_running", "1" );
	
	// Join the ipv6 multicast group now that a map is running so clients can scan for us on the local network.
#ifdef USE_IPV6
	NET_JoinMulticast6();
#endif
}


/*
==================
SV_ChangeMaxClients
==================
*/
void SV_ChangeMaxClients( void ) {
	int		oldMaxClients;
	int		i;
	client_t	*oldClients;
	int		count;

	// get the highest client number in use
	count = 0;
	for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
		if ( svs.clients[i].state >= CS_CONNECTED ) {
			if (i > count)
				count = i;
		}
	}
	count++;

	oldMaxClients = sv_maxclients->integer;
	// never go below the highest client number in use
	SV_BoundMaxClients( count );
	// if still the same
	if ( sv_maxclients->integer == oldMaxClients ) {
		return;
	}

	oldClients = Hunk_AllocateTempMemory( count * sizeof(client_t) );
	// copy the clients to hunk memory
	for ( i = 0 ; i < count ; i++ ) {
		if ( svs.clients[i].state >= CS_CONNECTED ) {
			oldClients[i] = svs.clients[i];
		}
		else {
			Com_Memset(&oldClients[i], 0, sizeof(client_t));
		}
	}

	// free old clients arrays
	Z_Free( svs.clients );

	// allocate new clients
#ifdef USE_LNBITS
	maxInvoices = Z_Malloc( ( sv_maxclients->integer + 10 ) * sizeof(invoice_t) );
	Com_Memset( maxInvoices, 0, ( sv_maxclients->integer + 10 ) * sizeof(invoice_t) );
	numInvoices = 0;
#endif
#ifdef USE_MV
	svs.clients = Z_TagMalloc( ( sv_maxclients->integer + 1 ) * sizeof(client_t), TAG_CLIENTS );
	Com_Memset( svs.clients, 0, ( sv_maxclients->integer + 1 ) * sizeof(client_t) );
#else
	svs.clients = Z_TagMalloc( sv_maxclients->integer * sizeof(client_t), TAG_CLIENTS );
	Com_Memset( svs.clients, 0, sv_maxclients->integer * sizeof(client_t) );
#endif

	// copy the clients over
	for ( i = 0 ; i < count ; i++ ) {
		if ( oldClients[i].state >= CS_CONNECTED ) {
			svs.clients[i] = oldClients[i];
		}
	}

	// free the old clients on the hunk
	Hunk_FreeTempMemory( oldClients );
	
	SV_SetSnapshotParams();
}


/*
================
SV_ClearServer
================
*/
static void SV_ClearServer( void ) {
	int i;

	for ( i = 0 ; i < MAX_CONFIGSTRINGS ; i++ ) {
		if ( sv.configstrings[i] ) {
			Z_Free( sv.configstrings[i] );
		}
	}

	if ( !sv_levelTimeReset->integer ) {
		i = sv.time; 
		Com_Memset( &sv, 0, sizeof( sv ) );
		sv.time = i;
	} else {
		Com_Memset( &sv, 0, sizeof( sv ) );
	}
}


/*
================
SV_SpawnServer

Change the server to a new map, taking all connected
clients along with it.
This is NOT called for map_restart
================
*/
static qboolean startingServer = qfalse;
void SV_SpawnServer( const char *mapname, qboolean killBots ) {
	int			i;
	int			checksum;
	qboolean	isBot;
	const char	*p;
	if(startingServer) {
		Com_Printf( "SpawnServer: Already starting\n" );
		return;
	}
	startingServer = qtrue;

	// shut down the existing game if it is running
	SV_ShutdownGameProgs();

	Com_Printf( "------ Server Initialization ------\n" );
	Com_Printf( "Server: %s\n", mapname );

	Sys_SetStatus( "Initializing server..." );

#ifdef USE_MV
	SV_LoadRecordCache();
#endif

#ifndef DEDICATED
	// if not running a dedicated server CL_MapLoading will connect the client to the server
	// also print some status stuff
	CL_MapLoading();

	// make sure all the client stuff is unloaded
#ifndef USE_LAZY_MEMORY
	CL_ShutdownAll();
#else
	S_DisableSounds();
#endif
#endif

#ifndef USE_LAZY_MEMORY
	// clear the whole hunk because we're (re)loading the server
	Hunk_Clear();
#endif

	// clear collision map data
	CM_ClearMap();

	// timescale can be updated before SV_Frame() and cause division-by-zero in SV_RateMsec()
	Cvar_CheckRange( com_timescale, "0.001", NULL, CV_FLOAT );

	// Restart renderer?
	// CL_StartHunkUsers( );

	// init client structures and svs.numSnapshotEntities 
	if ( !Cvar_VariableIntegerValue( "sv_running" ) ) {
		SV_Startup();
	} else {
		// check for maxclients change
#ifdef USE_MV
		if ( sv_maxclients->modified || sv_mvClients->modified )
#else
		if ( sv_maxclients->modified )
#endif
    {
#ifdef USE_DEMO_SERVER
			// If we are playing/waiting to play/waiting to stop a demo, we use a specialized function that will move real clients slots (so that democlients will be put to their original slots they were affected at the time of the real game)
			if (sv.demoState == DS_WAITINGPLAYBACK || sv.demoState == DS_PLAYBACK || sv.demoState == DS_WAITINGSTOP)
				SV_DemoChangeMaxClients();
			else
#endif
				SV_ChangeMaxClients();
		}
	}

#ifndef DEDICATED
	// remove pure paks that may left from client-side
	FS_PureServerSetLoadedPaks( "", "" );
	FS_PureServerSetReferencedPaks( "", "" );
#endif

	// clear pak references
	FS_ClearPakReferences( 0 );

	// allocate the snapshot entities on the hunk
	svs.snapshotEntities = Hunk_Alloc( sizeof(entityState_t)*svs.numSnapshotEntities, h_high );

	// initialize snapshot storage
	SV_InitSnapshotStorage();

#ifdef USE_MV
	// MV protocol support
	if ( svs.numSnapshotPSF ) // can be zero?
		svs.snapshotPSF = Hunk_Alloc( sizeof(psFrame_t)*svs.numSnapshotPSF, h_high );
	else
		svs.snapshotPSF = NULL;

	svs.nextSnapshotPSF = 0;
#endif

#ifdef USE_LNBITS
	// reset invoice array so clients have to pay again in between matches
  if(sv_lnPerMatch->integer)
    Com_Memset( maxInvoices, 0, ( sv_maxclients->integer + 10 ) * sizeof(invoice_t) );
#endif

	// toggle the server bit so clients can detect that a
	// server has changed
	svs.snapFlagServerBit ^= SNAPFLAG_SERVERCOUNT;

	// set nextmap to the same map, but it may be overridden
	// by the game startup or another console command
	Cvar_Set( "nextmap", "map_restart 0" );
//	Cvar_Set( "nextmap", va("map %s", server) );

	// try to reset level time if server is empty
	if ( !sv_levelTimeReset->integer && !sv.restartTime ) {
		for ( i = 0; i < sv_maxclients->integer; i++ ) {
			if ( svs.clients[ i ].state != CS_FREE )
				break;
		}
		if ( i == sv_maxclients->integer ) {
			sv.time = 0;
		}
	}

	for ( i = 0; i < sv_maxclients->integer; i++ ) {
		// save when the server started for each client already connected
		if ( svs.clients[i].state >= CS_CONNECTED && sv_levelTimeReset->integer ) {
			svs.clients[i].oldServerTime = sv.time;
		} else {
			svs.clients[i].oldServerTime = 0;
		}
	}

	// wipe the entire per-level structure
	SV_ClearServer();
	for ( i = 0 ; i < MAX_CONFIGSTRINGS ; i++ ) {
		sv.configstrings[i] = CopyString("");
	}

	// make sure we are not paused
#ifndef DEDICATED
	Cvar_Set( "cl_paused", "0" );
#endif

	// get latched value
	Cvar_Get( "sv_pure", "1", CVAR_SYSTEMINFO | CVAR_LATCH );

	// get a new checksum feed and restart the file system
	srand( Com_Milliseconds() );
	Com_RandomBytes( (byte*)&sv.checksumFeed, sizeof( sv.checksumFeed ) );
	FS_Restart( sv.checksumFeed );

	// set serverinfo visible name
	Cvar_Set( "mapname", mapname );
#ifdef USE_MULTIVM_SERVER
  Cvar_Get( va("mapname_%i", gvmi), mapname, CVAR_TAGGED_SPECIFIC );
  Cvar_Set( va("mapname_%i", gvmi), mapname );
#endif

	Sys_SetStatus( "Loading map %s", mapname );
#ifdef USE_MULTIVM_SERVER
	gameWorlds[gvmi] = CM_LoadMap( va( "maps/%s.bsp", mapname ), qfalse, &checksum );
#else
  gameWorlds[0] = CM_LoadMap( va( "maps/%s.bsp", mapname ), qfalse, &checksum );
#endif

	Cvar_Set( "sv_mapChecksum", va( "%i", checksum ) );
#ifdef USE_MULTIVM_SERVER
  Cvar_Set( va("sv_mapChecksum_%i", gvmi), va( "%i", checksum ) );
  Cvar_Get( va("sv_mapChecksum_%i", gvmi), "", CVAR_TAGGED_SPECIFIC );
#endif

	// serverid should be different each time
	sv.serverId = com_frameTime;
	sv.restartedServerId = sv.serverId; // I suppose the init here is just to be safe
	sv.checksumFeedServerId = sv.serverId;
	Cvar_Set( "sv_serverid", va( "%i", sv.serverId ) );

	// clear physics interaction links
	SV_ClearWorld();
	
	// media configstring setting should be done during
	// the loading stage, so connected clients don't have
	// to load during actual gameplay
	sv.state = SS_LOADING;

	// make sure that level time is not zero
	sv.time = sv.time ? sv.time : 8;

	// load and spawn all other entities
	SV_InitGameProgs( qfalse );

	// don't allow a map_restart if game is modified
	sv_gametype->modified = qfalse;

	sv_pure->modified = qfalse;

	// run a few frames to allow everything to settle
	for ( i = 0; i < 3; i++ )
	{
		sv.time += 100;
		VM_Call( gvm, 1, GAME_RUN_FRAME, sv.time );
		SV_BotFrame( sv.time );
	}

	// create a baseline for more efficient communications
	SV_CreateBaseline();

	for ( i = 0; i < sv_maxclients->integer; i++ ) {
#ifdef USE_MULTIVM_SERVER
		// also clear the entity type because this is how multiworld 
		//   figures out of a client has been there before to send gamestates
		SV_SetConfigstring(CS_PLAYERS + i, "");
#endif

		// send the new gamestate to all connected clients
		if ( svs.clients[i].state >= CS_CONNECTED ) {
			const char *denied;

			if ( svs.clients[i].netchan.remoteAddress.type == NA_BOT ) {
				if ( killBots ) {
					SV_DropClient( &svs.clients[i], "was kicked" );
					continue;
				}
				isBot = qtrue;
			}
			else {
				isBot = qfalse;
			}

#ifdef USE_LNBITS
			if(sv_lnWallet->string[0] && sv_lnMatchPrice->integer > 0) {
				// reconnect clients so they have to repay
				SV_SendServerCommand( &svs.clients[i], "reconnect\n" );
				continue;
			}
#endif

			// connect the client again
#ifndef BUILD_GAME_STATIC
			denied = GVM_ArgPtr( VM_Call( gvm, 3, GAME_CLIENT_CONNECT, i, qfalse, isBot ) );	// firstTime = qfalse
#else
      denied = (void *)VM_Call( gvm, 3, GAME_CLIENT_CONNECT, i, qfalse, isBot );	// firstTime = qfalse
#endif
			if ( denied ) {
				// this generally shouldn't happen, because the client
				// was connected before the level change
				SV_DropClient( &svs.clients[i], denied );
			} else {
				if( !isBot ) {
					// when we get the next packet from a connected client,
					// the new gamestate will be sent
					svs.clients[i].state = CS_CONNECTED;
				}
				else {
					client_t		*client;
					sharedEntity_t	*ent;

					client = &svs.clients[i];
					client->state = CS_ACTIVE;
					ent = SV_GentityNum( i );
					ent->s.number = i;
					client->gentity = ent;

					client->deltaMessage = -1;
					client->lastSnapshotTime = svs.time - 9999; // generate a snapshot immediately

					VM_Call( gvm, 1, GAME_CLIENT_BEGIN, i );
				}
			}
		}
	}

	// run another frame to allow things to look at all the players
	sv.time += 100;
	VM_Call( gvm, 1, GAME_RUN_FRAME, sv.time );
	SV_BotFrame( sv.time );
	svs.time += 100;

	// we need to touch the cgame and ui qvm because they could be in
	// separate pk3 files and the client will need to download the pk3
	// files with the latest cgame and ui qvm to pass the pure check
	FS_TouchFileInPak( "vm/cgame.qvm" );
	FS_TouchFileInPak( "vm/ui.qvm" );

	// the server sends these to the clients so they can figure
	// out which pk3s should be auto-downloaded
	p = FS_ReferencedPakNames();
	if ( FS_ExcludeReference() ) {
		// \fs_excludeReference may mask our current ui/cgame qvms
		FS_TouchFileInPak( "vm/cgame.qvm" );
		FS_TouchFileInPak( "vm/ui.qvm" );
		// rebuild referenced paks list
		p = FS_ReferencedPakNames();
	}
	Cvar_Set( "sv_referencedPakNames", p );

	p = FS_ReferencedPakChecksums();
	Cvar_Set( "sv_referencedPaks", p );

	Cvar_Set( "sv_paks", "" );
	Cvar_Set( "sv_pakNames", "" ); // not used on client-side

	if ( sv_pure->integer ) {
		int freespace, pakslen, infolen;
		qboolean overflowed = qfalse;
		qboolean infoTruncated = qfalse;

		p = FS_LoadedPakChecksums( &overflowed );

		pakslen = strlen( p ) + 9; // + strlen( "\\sv_paks\\" )
		freespace = SV_RemainingGameState();
#ifdef USE_MULTIVM_SERVER
    infolen = strlen( Cvar_InfoString_Big( CVAR_SYSTEMINFO, &infoTruncated, gvmi ) );
#else
#ifdef USE_MULTIVM_CLIENT
		infolen = strlen( Cvar_InfoString_Big( CVAR_SYSTEMINFO, &infoTruncated, 0 ) );
#else
		infolen = strlen( Cvar_InfoString_Big( CVAR_SYSTEMINFO, &infoTruncated ) );
#endif
#endif

		if ( infoTruncated ) {
			Com_Printf( S_COLOR_YELLOW "WARNING: truncated systeminfo!\n" );
		}

		if ( pakslen > freespace || infolen + pakslen >= BIG_INFO_STRING || overflowed ) {
			// switch to degraded pure mode
			// this could *potentially* lead to a false "unpure client" detection
			// which is better than guaranteed drop
			Com_DPrintf( S_COLOR_YELLOW "WARNING: skipping sv_paks setup to avoid gamestate overflow\n" );
		} else {
			// the server sends these to the clients so they will only
			// load pk3s also loaded at the server
			Cvar_Set( "sv_paks", p );
			if ( *p == '\0' ) {
				Com_Printf( S_COLOR_YELLOW "WARNING: sv_pure set but no PK3 files loaded\n" );
			}
		}
	}

	// save systeminfo and serverinfo strings
#ifdef USE_MULTIVM_SERVER
	SV_SetConfigstring( CS_SYSTEMINFO, Cvar_InfoString_Big( CVAR_SYSTEMINFO, NULL, gvmi ) );
	cvar_modifiedFlags &= ~CVAR_SYSTEMINFO;

	SV_SetConfigstring( CS_SERVERINFO, Cvar_InfoString( CVAR_SERVERINFO, NULL, gvmi ) );
	cvar_modifiedFlags &= ~CVAR_SERVERINFO;
#else
#ifdef USE_MULTIVM_CLIENT
	SV_SetConfigstring( CS_SYSTEMINFO, Cvar_InfoString_Big( CVAR_SYSTEMINFO, NULL, 0 ) );
	cvar_modifiedFlags &= ~CVAR_SYSTEMINFO;

	SV_SetConfigstring( CS_SERVERINFO, Cvar_InfoString( CVAR_SERVERINFO, NULL, 0 ) );
	cvar_modifiedFlags &= ~CVAR_SERVERINFO;
#else
  SV_SetConfigstring( CS_SYSTEMINFO, Cvar_InfoString_Big( CVAR_SYSTEMINFO, NULL ) );
  cvar_modifiedFlags &= ~CVAR_SYSTEMINFO;

  SV_SetConfigstring( CS_SERVERINFO, Cvar_InfoString( CVAR_SERVERINFO, NULL ) );
  cvar_modifiedFlags &= ~CVAR_SERVERINFO;
#endif
#endif

	// any media configstring setting now should issue a warning
	// and any configstring changes should be reliably transmitted
	// to all clients
	sv.state = SS_GAME;

	// send a heartbeat now so the master will get up to date info
	SV_Heartbeat_f();

	Hunk_SetMark();

#ifdef USE_LOCAL_DED
	svShuttingDown = qfalse;
#else
  // Brian "megamind" Cullinan - if on the off chance there is a server error
  //   preventing a connection for single player in SV_DirectConnect gamestate
  //   will never be sent and won't restart client after shutdown above
#ifndef DEDICATED
	//if ( com_dedicated->integer ) {
	// restart renderer in order to show console for dedicated servers
	// launched through the regular binary
	CL_StartHunkUsers( );
	//}
#endif
#endif

#ifdef USE_MULTIVM_SERVER
	Com_Printf ("---------------- Finished Starting Map (%i) -------------------\n", gvmi);
#else
  Com_Printf ("---------------- Finished Starting Map -------------------\n");
#endif

	Sys_SetStatus( "Running map %s", mapname );
	startingServer = qfalse;
#ifdef USE_RECENT_EVENTS
  SV_RecentEvent(va(RECENT_TEMPLATE_STR, sv.time, SV_EVENT_MAPCHANGE, mapname));
#endif

	if ( com_dedicated->integer && sv_activeAction->string[0] ) {
		Cbuf_AddText( sv_activeAction->string );
		Cbuf_AddText( "\n" );
		Cvar_Set( "activeAction", "" );
	}

#ifdef USE_DEMO_SERVER
	// start recording a demo
	if ( sv_autoDemo->integer ) {
		SV_DemoAutoDemoRecord();
	}
#endif
}


#ifdef USE_SERVER_ROLES
void SV_InitUserRoles (void) {
	int roleCount = 0;
	// force 3 roles to be available?
	char *roles = Cmd_TokenizeAlphanumeric(va("referee moderator admin %s", sv_roles->string), &roleCount);
  if(roleCount > MAX_CLIENT_ROLES) {
    Com_Printf("WARNING: may have exceeded max role count (%i).", MAX_CLIENT_ROLES);
  }
  for(int i = 0; i < roleCount; i++) {
    if(roles[0] == '\0') continue;
		sv_role[i] = Cvar_Get(va("sv_%s", roles), "", CVAR_ARCHIVE);
		sv_rolePassword[i] = Cvar_Get(va("%sPassword", roles), "", CVAR_TEMP);
		roles = &roles[strlen(roles)+1];
	}
  sv_roles->modified = qfalse;
}


void SV_UserRolesModified(char *oldValue, char *newValue, cvar_t *cv) {
  SV_InitUserRoles();
}

#endif


#ifdef USE_CVAR_UNCHEAT
void SV_InitBanCheats( void ) {
  int cheatCount;
  char *cheats = Cmd_TokenizeAlphanumeric(sv_banCheats->string, &cheatCount);
  for(int i = 0; i < ARRAY_LEN(svUncheats); i++) {
    // set userinfo on all the cheated values
    if(svUncheats[i]) {
      Z_Free(svUncheats[i]);
      svUncheats[i] = NULL;
    }
    if(i >= cheatCount || cheats[0] == '\0') continue;
    svUncheats[i] = CopyString(cheats);
    cheats = &cheats[strlen(cheats)+1];
  }
  sv_banCheats->modified = qfalse;
}

void SV_BanCheatsModified(char *oldValue, char *newValue, cvar_t *cv) {
  SV_InitBanCheats();
}
#endif


#endif // BUILD_SLIM_CLIENT

/*
===============
SV_Init

Only called at main exe startup, not for each game
===============
*/
void SV_Init( void )
{
	int index;

#ifndef BUILD_SLIM_CLIENT
#if defined(USE_CMD_CONNECTOR) && defined(USE_LOCAL_DED)
	// if using a local dedicated server and these commands are not present, 
	//   then they will automatically be forwarded to the local dedicated server
	//   via `CL_ForwardCommandToServer()`
	if(com_dedicated->integer) {
		SV_AddOperatorCommands();
		SV_AddDedicatedCommands();
	}
#else
	SV_AddOperatorCommands();

	if ( com_dedicated->integer )
		SV_AddDedicatedCommands();
#endif
#endif

	// serverinfo vars
	Cvar_Get ("dmflags", "0", CVAR_SERVERINFO);
	Cvar_Get ("fraglimit", "20", CVAR_SERVERINFO);
	Cvar_Get ("timelimit", "0", CVAR_SERVERINFO);
	sv_gametype = Cvar_Get ("g_gametype", "0", CVAR_SERVERINFO | CVAR_LATCH );
	Cvar_Get ("sv_keywords", "", CVAR_SERVERINFO);
	//Cvar_Get ("protocol", va("%i", PROTOCOL_VERSION), CVAR_SERVERINFO | CVAR_ROM);
#ifdef USE_MULTIVM_SERVER
	sv_mapname = Cvar_Get ("mapname", "nomap", CVAR_SERVERINFO | CVAR_ROM | CVAR_TAGGED_ORIGINAL);
#else
	sv_mapname = Cvar_Get ("mapname", "nomap", CVAR_SERVERINFO | CVAR_ROM);
#endif
	sv_privateClients = Cvar_Get( "sv_privateClients", "0", CVAR_SERVERINFO );
	Cvar_CheckRange( sv_privateClients, "0", va( "%i", MAX_CLIENTS-1 ), CV_INTEGER );
	sv_hostname = Cvar_Get ("sv_hostname", "noname", CVAR_SERVERINFO | CVAR_ARCHIVE );
	sv_maxclients = Cvar_Get ("sv_maxclients", "8", CVAR_SERVERINFO | CVAR_LATCH);
	Cvar_CheckRange( sv_maxclients, "1", XSTRING(MAX_CLIENTS), CV_INTEGER );

	sv_maxclientsPerIP = Cvar_Get( "sv_maxclientsPerIP", "3", CVAR_ARCHIVE );
	Cvar_CheckRange( sv_maxclientsPerIP, "1", NULL, CV_INTEGER );

	sv_clientTLD = Cvar_Get( "sv_clientTLD", "0", CVAR_ARCHIVE_ND );
	Cvar_CheckRange( sv_clientTLD, NULL, NULL, CV_INTEGER );

#ifdef USE_MV
	Cvar_Get( "mvproto", va( "%i", MV_PROTOCOL_VERSION ), CVAR_SERVERINFO | CVAR_ROM );
	sv_mvAutoRecord = Cvar_Get( "sv_mvAutoRecord", "0", CVAR_ARCHIVE | CVAR_SERVERINFO );
	sv_demoFlags = Cvar_Get( "sv_mvFlags", "3", CVAR_ARCHIVE );
	sv_mvClients = Cvar_Get( "sv_mvClients", "8", CVAR_ARCHIVE | CVAR_LATCH );
	Cvar_CheckRange( sv_mvClients, "0", NULL, CV_INTEGER );
	sv_mvPassword = Cvar_Get( "sv_mvPassword", "", CVAR_ARCHIVE );

	sv_mvFileCount = Cvar_Get( "sv_mvFileCount", "1024", CVAR_ARCHIVE );
	Cvar_CheckRange( sv_mvFileCount, "0", XSTRING( MAX_MV_FILES ), CV_INTEGER );

	sv_mvFolderSize = Cvar_Get( "sv_mvFolderSize", "768", CVAR_ARCHIVE );
	Cvar_CheckRange( sv_mvFolderSize, "0", "2048", CV_INTEGER );

#ifndef BUILD_SLIM_CLIENT
	SV_LoadRecordCache();
#endif
#endif
#ifdef USE_MULTIVM_SERVER
	sv_mvWorld = Cvar_Get("sv_mvWorld", "1", CVAR_ARCHIVE | CVAR_SERVERINFO | CVAR_TAGGED_ORIGINAL);
	Cvar_CheckRange( sv_mvWorld, "0", "1", CV_INTEGER );
	sv_mvSyncPS = Cvar_Get("sv_mvSyncPS", "0", CVAR_ARCHIVE);
	Cvar_CheckRange( sv_mvSyncPS, "0", "1", CV_INTEGER );
	sv_mvSyncXYZ = Cvar_Get("sv_mvSyncXYZ", "0", CVAR_ARCHIVE);
	Cvar_CheckRange( sv_mvSyncXYZ, "0", "1", CV_INTEGER );
	sv_mvSyncMove = Cvar_Get("sv_mvSyncMove", "0", CVAR_ARCHIVE);
	Cvar_CheckRange( sv_mvSyncMove, "0", "1", CV_INTEGER );
	sv_mvOmnipresent = Cvar_Get("sv_mvOmnipresent", "0", CVAR_ARCHIVE | CVAR_SERVERINFO);
	Cvar_CheckRange( sv_mvOmnipresent, "-1", "1", CV_INTEGER );
#endif

	sv_minRate = Cvar_Get ("sv_minRate", "0", CVAR_ARCHIVE_ND | CVAR_SERVERINFO );
	sv_maxRate = Cvar_Get ("sv_maxRate", "0", CVAR_ARCHIVE_ND | CVAR_SERVERINFO );
	sv_dlRate = Cvar_Get("sv_dlRate", "100", CVAR_ARCHIVE | CVAR_SERVERINFO);
	sv_floodProtect = Cvar_Get ("sv_floodProtect", "1", CVAR_ARCHIVE | CVAR_SERVERINFO );

	// systeminfo
	Cvar_Get( "sv_cheats", "1", CVAR_SYSTEMINFO | CVAR_ROM );
	sv_serverid = Cvar_Get( "sv_serverid", "0", CVAR_SYSTEMINFO | CVAR_ROM );
	sv_pure = Cvar_Get( "sv_pure", "1", CVAR_SYSTEMINFO | CVAR_LATCH );
	Cvar_Get( "sv_paks", "", CVAR_SYSTEMINFO | CVAR_ROM );
	Cvar_Get( "sv_pakNames", "", CVAR_SYSTEMINFO | CVAR_ROM );
	Cvar_Get( "sv_referencedPaks", "", CVAR_SYSTEMINFO | CVAR_ROM );
	sv_referencedPakNames = Cvar_Get( "sv_referencedPakNames", "", CVAR_SYSTEMINFO | CVAR_ROM );

#ifdef USE_SERVER_ROLES
	sv_roles = Cvar_Get( "sv_roles", "referee moderator admin", CVAR_ARCHIVE);
	for ( index = 0; index < MAX_CLIENT_ROLES; index++ )
		sv_clientRoles[index] = Cvar_Get(va("sv_role%d", index + 1), "", CVAR_ARCHIVE);
	sv_role[0] = Cvar_Get( "sv_referee", "ban kick restart map", CVAR_ARCHIVE);
	sv_role[1] = Cvar_Get( "sv_moderator", "kick ban timelimit fraglimit capturelimit shuffle mute map nextmap map_restart", CVAR_ARCHIVE);
  Cvar_SetModifiedFunc(sv_roles, SV_UserRolesModified);
	SV_InitUserRoles();
#endif

#ifdef USE_REFEREE_CMDS
	sv_frozen = Cvar_Get("sv_frozen", "0", CVAR_ROM);
	sv_lock[0] = Cvar_Get("sv_lockRed", "0", CVAR_TEMP);
	sv_lock[1] = Cvar_Get("sv_lockBlue", "0", CVAR_TEMP);
  sv_thawTime = Cvar_Get("g_thawTime", "180", CVAR_ARCHIVE);
#endif

#ifdef USE_PERSIST_CLIENT
	sv_clSessions = Cvar_Get("sv_clSessions", "0", CVAR_ARCHIVE);
	Cvar_CheckRange( sv_clSessions, "-1", NULL, CV_INTEGER );
#endif

#ifdef USE_RECENT_EVENTS
	sv_recentPassword = Cvar_Get ("recentPassword", "", CVAR_TEMP );
#endif

	if(com_dedicated->integer) {
		sv_activeAction = Cvar_Get( "activeAction", "", CVAR_TEMP );
	}

	// server vars
	sv_rconPassword = Cvar_Get ("rconPassword", "", CVAR_TEMP );
	sv_privatePassword = Cvar_Get ("sv_privatePassword", "", CVAR_TEMP );
  sv_gamedir = Cvar_Get("fs_game", "", 0);
#ifdef BUILD_GAME_STATIC
  sv_fps = Cvar_Get ("sv_fps", "100", CVAR_TEMP | CVAR_SYSTEMINFO );
	Cvar_CheckRange( sv_fps, "10", "300", CV_INTEGER );
#else
#ifdef USE_MULTIVM_SERVER
	sv_fps = Cvar_Get ("sv_fps", "40", CVAR_TEMP | CVAR_SYSTEMINFO );
	Cvar_CheckRange( sv_fps, "10", "200", CV_INTEGER );
#else
	sv_fps = Cvar_Get ("sv_fps", "20", CVAR_TEMP | CVAR_SYSTEMINFO );
	Cvar_CheckRange( sv_fps, "10", "125", CV_INTEGER );
#endif
#endif
	sv_timeout = Cvar_Get( "sv_timeout", "200", CVAR_TEMP );
	Cvar_CheckRange( sv_timeout, "4", NULL, CV_INTEGER );
	sv_zombietime = Cvar_Get( "sv_zombietime", "2", CVAR_TEMP );
	Cvar_CheckRange( sv_zombietime, "1", NULL, CV_INTEGER );
  // share with client for predictive loading
#ifdef USE_MULTIVM_SERVER
	Cvar_Get ("nextmap", "", CVAR_TEMP | CVAR_SERVERINFO );
#else
  Cvar_Get ("nextmap", "", CVAR_TEMP );
#endif

	sv_allowDownload = Cvar_Get ("sv_allowDownload", "1", CVAR_SERVERINFO);
	Cvar_Get ("sv_dlURL", "", CVAR_SERVERINFO | CVAR_ARCHIVE);

	// moved to Com_Init()
	//sv_master[0] = Cvar_Get( "sv_master1", MASTER_SERVER_NAME, CVAR_INIT | CVAR_ARCHIVE_ND );
	//sv_master[1] = Cvar_Get( "sv_master2", "master.ioquake3.org", CVAR_INIT | CVAR_ARCHIVE_ND );
	//sv_master[2] = Cvar_Get( "sv_master3", "master.maverickservers.com", CVAR_INIT | CVAR_ARCHIVE_ND );

	for ( index = 0; index < MAX_MASTER_SERVERS; index++ )
		sv_master[ index ] = Cvar_Get( va( "sv_master%d", index + 1 ), "", CVAR_ARCHIVE_ND );

	sv_reconnectlimit = Cvar_Get( "sv_reconnectlimit", "3", 0 );
	Cvar_CheckRange( sv_reconnectlimit, "0", "12", CV_INTEGER );

	sv_padPackets = Cvar_Get ("sv_padPackets", "0", 0);
	sv_killserver = Cvar_Get ("sv_killserver", "0", 0);
	sv_mapChecksum = Cvar_Get ("sv_mapChecksum", "", CVAR_ROM);
#ifdef USE_MULTIVM_SERVER
  sv_mapChecksum->flags |= CVAR_TAGGED_ORIGINAL;
#endif
	sv_lanForceRate = Cvar_Get ("sv_lanForceRate", "1", CVAR_ARCHIVE_ND );

#ifdef USE_CVAR_UNCHEAT
  // TODO: set default to all client values with CVAR_CHEAT
  sv_banCheats = Cvar_Get("sv_banCheats", "", CVAR_ARCHIVE | CVAR_SERVERINFO | CVAR_USERINFO);
  Cvar_SetModifiedFunc(sv_banCheats, SV_BanCheatsModified);
  SV_InitBanCheats();
#endif

#ifdef USE_MEMORY_MAPS
	sv_memoryMaps = Cvar_Get( "sv_memoryMaps", "1", CVAR_ARCHIVE );
	sv_bspLight = Cvar_Get( "sv_bspLight", "0", CVAR_TEMP );
	sv_bspMinimap = Cvar_Get( "sv_bspMinimap", "0", CVAR_TEMP );
	sv_bspAAS = Cvar_Get( "sv_bspAAS", "0", CVAR_TEMP );
	sv_bspMap = Cvar_Get( "sv_bspMap", "0", CVAR_TEMP );
	sv_bspSplice = Cvar_Get( "sv_bspSplice", "", CVAR_TEMP );
	sv_bspRebuild = Cvar_Get( "sv_bspRebuild", "0", CVAR_TEMP );
	sv_bspMiniSize = Cvar_Get( "sv_bspMiniSize", "64", CVAR_TEMP );
	sv_bspMiniGrid = Cvar_Get( "sv_bspMiniGrid", "16", CVAR_TEMP );
#endif

#ifdef USE_BANS
	sv_banFile = Cvar_Get("sv_banFile", "serverbans.dat", CVAR_ARCHIVE);
#endif

#ifdef USE_LNBITS
  sv_lnPerMatch = Cvar_Get("sv_lnPerMatch", "1", CV_INTEGER | CVAR_SERVERINFO | CVAR_ARCHIVE)
	sv_lnMatchPrice = Cvar_Get("sv_lnMatchPrice", "0", CV_INTEGER | CVAR_SERVERINFO | CVAR_ARCHIVE);
	sv_lnMatchCut = Cvar_Get("sv_lnMatchCut", "0", CV_INTEGER | CVAR_SERVERINFO | CVAR_ARCHIVE);
	sv_lnMatchReward = Cvar_Get("sv_lnMatchReward", "0", CV_INTEGER | CVAR_SERVERINFO | CVAR_TEMP);
	sv_lnWallet = Cvar_Get("sv_lnWallet", "", CVAR_ARCHIVE);
	sv_lnKey = Cvar_Get("sv_lnKey", "", CVAR_ARCHIVE);
	sv_lnAPI = Cvar_Get("sv_lnAPI", "https://lnbits.com/api/v1", CVAR_SERVERINFO | CVAR_ARCHIVE);
	sv_lnWithdraw = Cvar_Get("sv_lnWithdraw", "https://lnbits.com/withdraw/api/v1", CVAR_SERVERINFO | CVAR_ARCHIVE);
#endif

#ifdef DEDICATED
#ifdef USE_CURL
	cl_dlDirectory = Cvar_Get( "cl_dlDirectory", "0", CVAR_ARCHIVE_ND );
	Cvar_CheckRange( cl_dlDirectory, "0", "1", CV_INTEGER );
#endif
#ifdef USE_CURL_DLOPEN
	cl_cURLLib = Cvar_Get( "cl_cURLLib", DEFAULT_CURL_LIB, 0 );
#endif
#endif

	sv_demoState = Cvar_Get ("sv_demoState", "0", CVAR_ROM );
	sv_democlients = Cvar_Get ("sv_democlients", "0", CVAR_ROM );

#ifdef USE_DEMO_SERVER
	sv_autoDemo = Cvar_Get ("sv_autoDemo", "0", CVAR_ARCHIVE );
	// port from client-side to freeze server-side demos
	cl_freezeDemo = Cvar_Get("cl_freezeDemo", "0", CVAR_TEMP);
	sv_demoTolerant = Cvar_Get ("sv_demoTolerant", "0", CVAR_ARCHIVE );
#endif
#ifdef USE_DEMO_CLIENTS
	sv_autoRecord = Cvar_Get ("sv_autoRecord", "0", CVAR_ARCHIVE );
	sv_autoRecordThreshold = Cvar_Get("sv_autoRecordThreshold", "0.9", CVAR_ARCHIVE );
#endif

	sv_levelTimeReset = Cvar_Get( "sv_levelTimeReset", "0", CVAR_ARCHIVE_ND );

	sv_filter = Cvar_Get( "sv_filter", "filter.txt", CVAR_ARCHIVE );
  Cvar_SetServerDescriptions();

	// initialize bot cvars so they are listed and can be set before loading the botlib
	SV_BotInitCvars();
  Cvar_SetBotDescriptions();

	// init the botlib here because we need the pre-compiler in the UI
	SV_BotInitBotLib();

#ifdef USE_BANS	
	// Load saved bans
	Cbuf_AddText("rehashbans\n");
#endif

	// track group cvar changes
	Cvar_SetGroup( sv_lanForceRate, CVG_SERVER );
	Cvar_SetGroup( sv_minRate, CVG_SERVER );
	Cvar_SetGroup( sv_maxRate, CVG_SERVER );
	Cvar_SetGroup( sv_fps, CVG_SERVER );

#ifndef BUILD_SLIM_CLIENT
#ifdef USE_LOCAL_DED
	if(com_dedicated->integer)
#endif
	{
	// force initial check
	SV_TrackCvarChanges();

	SV_InitChallenger();
	}
#endif
}


#ifndef BUILD_SLIM_CLIENT

/*
==================
SV_FinalMessage

Used by SV_Shutdown to send a final message to all
connected clients before the server goes down.  The messages are sent immediately,
not just stuck on the outgoing message list, because the server is going
to totally exit after returning from this function.
==================
*/
void SV_FinalMessage( const char *message ) {
	int			i, j;
	client_t	*cl;
	
	// send it twice, ignoring rate
	for ( j = 0 ; j < 2 ; j++ ) {
		for (i=0, cl = svs.clients ; i < sv_maxclients->integer ; i++, cl++) {
			if (cl->state >= CS_CONNECTED ) {
#ifdef USE_DEMO_SERVER
 				// serverside demo
  			if (cl->demorecording) {
  				SV_StopRecord( cl );
 				}
#endif
#ifdef USE_LOCAL_DED
				if ( cl->netchan.remoteAddress.type == NA_LOOPBACK ) {
					SV_SendServerCommand( cl, "reconnect\nprint \"%s\n\"\n", message );
				}
#endif

				// don't send a disconnect to a local client
				if ( cl->netchan.remoteAddress.type != NA_LOOPBACK ) {
					SV_SendServerCommand( cl, "print \"%s\n\"\n", message );
					SV_SendServerCommand( cl, "disconnect \"%s\"", message );
				}
				// force a snapshot to be sent
				cl->lastSnapshotTime = svs.time - 9999; // generate a snapshot immediately
				cl->state = CS_ZOMBIE; // skip delta generation
				SV_SendClientSnapshot( cl, qfalse );
			}
		}
	}
}


/*
================
SV_Shutdown

Called when each game quits,
before Sys_Quit or Sys_Error
================
*/
void SV_Shutdown( const char *finalmsg ) {
	if ( !com_sv_running || !com_sv_running->integer ) {
		return;
	}
	
#ifdef USE_LOCAL_DED
	if(!svs.clients || !com_dedicated->integer) {
		return;
	}
#endif

	Com_Printf( "----- Server Shutdown (%s) -----\n", finalmsg );

#ifdef USE_DEMO_SERVER
	// stop any demos
	if (sv.demoState == DS_RECORDING)
		SV_DemoStopRecord();
	if (sv.demoState == DS_PLAYBACK)
		SV_DemoStopPlayback();
#endif

#ifdef USE_DEMO_CLIENTS
  int		i;
  client_t	*cl;
	for (i=0, cl = svs.clients ; i < sv_maxclients->integer ; i++, cl++) {
		if (cl->state >= CS_CONNECTED && cl->demorecording) {
			SV_StopRecord( cl );
		}
	}
#endif

#ifdef USE_IPV6
	NET_LeaveMulticast6();
#endif

#ifdef USE_LOCAL_DED
	if ( svs.clients ) {
		SV_FinalMessage( finalmsg );
	}
#else
	if ( svs.clients && !com_errorEntered ) {
		SV_FinalMessage( finalmsg );
	}
#endif

#ifdef USE_MV
	if ( sv_demoFile != FS_INVALID_HANDLE ) {
		// finalize record
		if ( svs.clients[ sv_maxclients->integer ].multiview.recorder ) {
			SV_SendClientSnapshot( &svs.clients[ sv_maxclients->integer ], qfalse );
		}
		SV_MultiViewStopRecord_f();
	}

	SV_SaveRecordCache();
#endif

#ifdef USE_LOCAL_DED
	// Local server is "always on"
	if(!svShuttingDown) {
		svShuttingDown = qtrue;
		startingServer = qfalse;
		SV_ShutdownGameProgs();
		Cvar_Set( "sv_running", "0" );
		svs.initialized = qfalse;
		Cmd_Clear();
		Cbuf_AddText("spmap q3dm0\n");
		return;
	}
#endif

	SV_RemoveOperatorCommands();
	SV_MasterShutdown();
	SV_ShutdownGameProgs();
	SV_InitChallenger();

	// free current level
	SV_ClearServer();

	SV_FreeIP4DB();

	// free server static data
	if ( svs.clients ) {
		int index;

		for ( index = 0; index < sv_maxclients->integer; index++ )
			SV_FreeClient( &svs.clients[ index ] );
		
		Z_Free( svs.clients );
	}
	Com_Memset( &svs, 0, sizeof( svs ) );
	sv.time = 0;

	Cvar_Set( "sv_running", "0" );

	// allow setting timescale 0 for demo playback
	Cvar_CheckRange( com_timescale, "0", NULL, CV_FLOAT );

#ifndef DEDICATED
	Cvar_Set( "ui_singlePlayerActive", "0" );
#endif

	Com_Printf( "---------------------------\n" );

#ifndef DEDICATED
#ifndef USE_LOCAL_DED
	// disconnect any local clients
	if ( sv_killserver->integer != 2 )
		CL_Disconnect( qfalse, qtrue );
#endif
#endif

	// clean some server cvars
	Cvar_Set( "sv_referencedPaks", "" );
	Cvar_Set( "sv_referencedPakNames", "" );
	Cvar_Set( "sv_mapChecksum", "" );
	Cvar_Set( "sv_serverid", "0" );

	Sys_SetStatus( "Server is not running" );

#ifdef USE_LOCAL_DED
	Cmd_Clear();
	Cbuf_AddText(va("spmap q3dm0\n"));
#endif
}

#endif
