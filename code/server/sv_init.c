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
	Z_Free( sv.configstrings[index] );
	sv.configstrings[index] = CopyString( val );

	// save config strings to demo
	if (sv.demoState == DS_RECORDING) {
		SV_DemoWriteConfigString( index, val );
	}

	// send it to all the clients if we aren't
	// spawning a new server
	if ( sv.state == SS_GAME || sv.restarting ) {

		// send the data to all relevent clients
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

	// Save userinfo changes to demo (also in SV_UpdateUserinfo_f() in sv_client.c)
	if ( sv.demoState == DS_RECORDING ) {
		SV_DemoWriteClientUserinfo( &svs.clients[index], val );
	}

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
static void SV_CreateBaseline( void ) {
	sharedEntity_t *ent;
	int				entnum;	

	for ( entnum = 0; entnum < sv.num_entities ; entnum++ ) {
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
static void SV_BoundMaxClients( int minimum ) {
	// get the current maxclients value
	Cvar_Get( "sv_maxclients", "8", 0 );

	sv_maxclients->modified = qfalse;

	if ( sv_maxclients->integer < minimum ) {
		Cvar_Set( "sv_maxclients", va("%i", minimum) );
	}
}


/*
===============
SV_SetSnapshotParams
===============
*/
static void SV_SetSnapshotParams( void ) 
{
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

	svs.clients = Z_TagMalloc( sv_maxclients->integer * sizeof( client_t ), TAG_CLIENTS );
	Com_Memset( svs.clients, 0, sv_maxclients->integer * sizeof( client_t ) );
	SV_SetSnapshotParams();
	svs.initialized = qtrue;

	// Don't respect sv_killserver unless a server is actually running
	if ( sv_killserver->integer ) {
		Cvar_Set( "sv_killserver", "0" );
	}

	Cvar_Set( "sv_running", "1" );
	
	// Join the ipv6 multicast group now that a map is running so clients can scan for us on the local network.
	NET_JoinMulticast6();
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
	svs.clients = Z_TagMalloc( sv_maxclients->integer * sizeof(client_t), TAG_CLIENTS );
	Com_Memset( svs.clients, 0, sv_maxclients->integer * sizeof(client_t) );

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
==================
SV_DemoChangeMaxClients
change sv_maxclients and move real clients slots when a demo is playing or stopped
==================
*/
void SV_DemoChangeMaxClients( void ) {
  int             oldMaxClients, oldDemoClients;
  int             i, j, k;
  client_t        *oldClients = NULL;
  int             count;
  //qboolean firstTime = svs.clients == NULL;


	// == Checking the prerequisites
	// Note: we check  here that we have enough slots to fit all clients, and that it doesn't overflow the MAX_CLIENTS the engine can support. Also, we save the oldMaxClients and oldDemoClients values.

        // -- Get the highest client number in use
	count = 0;
	for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
		if ( svs.clients[i].state >= CS_CONNECTED ) {
			if (i > count)
				count = i;
		}
	}
	count++;

	// -- Save the previous oldMaxClients and oldDemoClients values, and update

	// Save the previous sv_maxclients value before updating it
  oldMaxClients = sv_maxclients->integer;
  // update the cvars
  Cvar_Get( "sv_maxclients", "8", 0 );
  Cvar_Get( "sv_democlients", "0", 0 ); // unnecessary now that sv_democlients is not latched anymore?
	// Save the previous sv_democlients (since it's updated instantly, we cannot get it directly), we use a trick by computing the difference between the new and previous sv_maxclients (the difference should indeed be the exact value of sv_democlients)
	oldDemoClients = (oldMaxClients - sv_maxclients->integer);
	if (oldDemoClients < 0) // if the difference is negative, this means that before it was set to 0 (because the newer sv_maxclients is greater than the old)
		oldDemoClients = 0;

	// -- Check limits
	// never go below the highest client number in use (make sure we have enough room for all players)
	SV_BoundMaxClients( count );

  // -- Change check: if still the same, we just quit, there's nothing to do
  if ( sv_maxclients->integer == oldMaxClients ) {
    return;
  }


	// == Memorizing clients
	// Note: we save in a temporary variables the clients, because after we will wipe completely the svs.clients struct

	// copy the clients to hunk memory
	oldClients = Hunk_AllocateTempMemory( (sv_maxclients->integer - sv_democlients->integer) * sizeof(client_t) ); // we allocate just enough memory for the real clients (not counting in the democlients)
	// For all previous clients slots, we copy the entire client into a temporary var
	for ( i = 0, j = 0, k = sv_privateClients->integer ; i < oldMaxClients ; i++ ) { // for all the previously connected clients, we copy them to a temporary var
		// If there is a real client in this slot
		if ( svs.clients[i].state >= CS_CONNECTED ) {
			// if the client is in a privateClient reserved slot, we move him on the reserved slots
			if (i >= oldDemoClients && i < oldDemoClients + sv_privateClients->integer) {
				oldClients[j++] = svs.clients[i];
			// else the client is not a privateClient, and we move him to the first available slot after the privateClients slots
			} else {
				oldClients[k++] = svs.clients[i];
			}
		}
	}

	// Fill in the remaining clients slots with empty clients (else the engine crash when copying into memory svs.clients)
	for (i=j; i < sv_privateClients->integer; i++) { // Fill the privateClients empty slots
		Com_Memset(&oldClients[i], 0, sizeof(client_t));
	}
	for (i=k; i < (sv_maxclients->integer - sv_democlients->integer); i++) { // Fill the other normal clients slots
		Com_Memset(&oldClients[i], 0, sizeof(client_t));
	}

	// free old clients arrays
	Z_Free( svs.clients );


	// == Allocating the new svs.clients and moving the saved clients over from the temporary var

  // allocate new svs.clients
  svs.clients = Z_Malloc( sv_maxclients->integer * sizeof(client_t) );
  Com_Memset( svs.clients, 0, sv_maxclients->integer * sizeof(client_t) );

	// copy the clients over (and move them depending on sv_democlients: if >0, move them upwards, if == 0, move them to their original slots)
	Com_Memcpy( svs.clients + sv_democlients->integer, oldClients, (sv_maxclients->integer - sv_democlients->integer) * sizeof(client_t) );

	// free the old clients on the hunk
	Hunk_FreeTempMemory( oldClients );


	// == Allocating snapshot entities

  // allocate new snapshot entities
  if ( com_dedicated->integer ) {
    svs.numSnapshotEntities = sv_maxclients->integer * PACKET_BACKUP * 64;
  } else {
          // we don't need nearly as many when playing locally
    svs.numSnapshotEntities = sv_maxclients->integer * 4 * 64;
  }


	// == Server-side demos management
	SV_SetSnapshotParams();

	// set demostate to none if it was just waiting to set maxclients and move real clients slots
	if (sv.demoState == DS_WAITINGSTOP) {
		sv.demoState = DS_NONE;
		Cvar_SetValue("sv_demoState", DS_NONE);
	}

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
qboolean killBots;
static qboolean startingServer = qfalse;

void SV_SpawnServer( const char *mapname, qboolean kb ) {
	int			i;
#ifndef EMSCRIPTEN
	int			checksum;
	qboolean	isBot;
	const char	*p;
#endif
	if(startingServer) {
		Com_Printf( "SpawnServer: Already starting\n" );
		return;
	}
	startingServer = qtrue;
	killBots = kb;

	// shut down the existing game if it is running
	SV_ShutdownGameProgs();

	Com_Printf( "------ Server Initialization ------\n" );
	Com_Printf( "Server: %s\n", mapname );

	Sys_SetStatus( "Initializing server..." );

#ifndef DEDICATED
	// if not running a dedicated server CL_MapLoading will connect the client to the server
	// also print some status stuff
	CL_MapLoading();

	// make sure all the client stuff is unloaded
#ifndef EMSCRIPTEN
	CL_ShutdownAll();
#else
	S_DisableSounds();
#endif
#endif

#ifndef EMSCRIPTEN
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
		if ( sv_maxclients->modified ) {
			// If we are playing/waiting to play/waiting to stop a demo, we use a specialized function that will move real clients slots (so that democlients will be put to their original slots they were affected at the time of the real game)
			if (sv.demoState == DS_WAITINGPLAYBACK || sv.demoState == DS_PLAYBACK || sv.demoState == DS_WAITINGSTOP)
				SV_DemoChangeMaxClients();
			else
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

	// toggle the server bit so clients can detect that a
	// server has changed
	svs.snapFlagServerBit ^= SNAPFLAG_SERVERCOUNT;

	// set nextmap to the same map, but it may be overriden
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
	Cvar_Get( "sv_pure", "1", 0 );

	// get a new checksum feed and restart the file system
	srand( Com_Milliseconds() );
	Com_RandomBytes( (byte*)&sv.checksumFeed, sizeof( sv.checksumFeed ) );
	FS_Restart( sv.checksumFeed );

	// set serverinfo visible name
	Cvar_Set( "mapname", mapname );

#ifdef EMSCRIPTEN
	Cvar_Set("sv_running", "0");
	Com_Frame_Callback(Sys_FS_Shutdown, SV_SpawnServer_After_Shutdown);
}

void SV_SpawnServer_After_Shutdown( void ) {
	FS_Startup();
	Com_Frame_Callback(Sys_FS_Startup, SV_SpawnServer_After_Startup);
}

void SV_SpawnServer_After_Startup( void ) {
	int			i;
	int			checksum;
	qboolean	isBot;
	const char	*p;
	const char *mapname = Cvar_VariableString("mapname");
	FS_Restart_After_Async();
	Cvar_Set("sv_running", "1");
#endif
;

	Sys_SetStatus( va("Loading map %s", mapname) );
	CM_LoadMap( va( "maps/%s.bsp", mapname ), qfalse, &checksum );

	Cvar_Set( "sv_mapChecksum", va( "%i",checksum ) );

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
	SV_InitGameProgs();

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

			// connect the client again
			denied = GVM_ArgPtr( VM_Call( gvm, 3, GAME_CLIENT_CONNECT, i, qfalse, isBot ) );	// firstTime = qfalse
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
		infolen = strlen( Cvar_InfoString_Big( CVAR_SYSTEMINFO, &infoTruncated ) );

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
	SV_SetConfigstring( CS_SYSTEMINFO, Cvar_InfoString_Big( CVAR_SYSTEMINFO, NULL ) );
	cvar_modifiedFlags &= ~CVAR_SYSTEMINFO;

	SV_SetConfigstring( CS_SERVERINFO, Cvar_InfoString( CVAR_SERVERINFO, NULL ) );
	cvar_modifiedFlags &= ~CVAR_SERVERINFO;

	// any media configstring setting now should issue a warning
	// and any configstring changes should be reliably transmitted
	// to all clients
	sv.state = SS_GAME;

	// send a heartbeat now so the master will get up to date info
	SV_Heartbeat_f();

	Hunk_SetMark();
	
#ifdef EMSCRIPTEN
	svShuttingDown = qfalse;
	//CL_StartHunkUsers( );
#endif

	Com_Printf ("-----------------------------------\n");
	
	Sys_SetStatus( va("Running map %s", mapname) );
	startingServer = qfalse;

	// start recording a demo
	if ( sv_autoDemo->integer ) {
		SV_DemoAutoDemoRecord();
	}

}


/*
===============
SV_Init

Only called at main exe startup, not for each game
===============
*/
void SV_Init( void )
{
	int index;

#ifdef EMSCRIPTEN
	if(com_dedicated->integer) {
		SV_AddOperatorCommands();

		if ( com_dedicated->integer )
			SV_AddDedicatedCommands();
	}
#else
	SV_AddOperatorCommands();

	if ( com_dedicated->integer )
		SV_AddDedicatedCommands();
#endif
	// serverinfo vars
	Cvar_Get ("dmflags", "0", CVAR_SERVERINFO);
	Cvar_Get ("fraglimit", "20", CVAR_SERVERINFO);
	Cvar_Get ("timelimit", "0", CVAR_SERVERINFO);
	sv_gametype = Cvar_Get ("g_gametype", "0", CVAR_SERVERINFO | CVAR_LATCH );
	Cvar_Get ("sv_keywords", "", CVAR_SERVERINFO);
	Cvar_Get ("protocol", va("%i", PROTOCOL_VERSION), CVAR_SERVERINFO | CVAR_ROM);
	sv_mapname = Cvar_Get ("mapname", "nomap", CVAR_SERVERINFO | CVAR_ROM);
	sv_privateClients = Cvar_Get( "sv_privateClients", "0", CVAR_SERVERINFO );
	Cvar_CheckRange( sv_privateClients, "0", va( "%i", MAX_CLIENTS-1 ), CV_INTEGER );
	sv_hostname = Cvar_Get ("sv_hostname", "noname", CVAR_SERVERINFO | CVAR_ARCHIVE );
	sv_maxclients = Cvar_Get ("sv_maxclients", "8", CVAR_SERVERINFO | CVAR_LATCH);
	Cvar_CheckRange( sv_maxclients, "1", XSTRING(MAX_CLIENTS), CV_INTEGER );

	sv_maxclientsPerIP = Cvar_Get( "sv_maxclientsPerIP", "3", CVAR_ARCHIVE );
	Cvar_CheckRange( sv_maxclientsPerIP, "1", NULL, CV_INTEGER );
	Cvar_SetDescription( sv_maxclientsPerIP, "Limits number of simultaneous connections from the same IP address." );

	sv_clientTLD = Cvar_Get( "sv_clientTLD", "0", CVAR_ARCHIVE_ND );
	Cvar_CheckRange( sv_clientTLD, NULL, NULL, CV_INTEGER );

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

	// server vars
	sv_rconPassword = Cvar_Get ("rconPassword", "", CVAR_TEMP );
	sv_privatePassword = Cvar_Get ("sv_privatePassword", "", CVAR_TEMP );
	sv_fps = Cvar_Get ("sv_fps", "20", CVAR_TEMP );
	Cvar_CheckRange( sv_fps, "10", "125", CV_INTEGER );
	sv_timeout = Cvar_Get( "sv_timeout", "200", CVAR_TEMP );
	Cvar_CheckRange( sv_timeout, "4", NULL, CV_INTEGER );
	Cvar_SetDescription( sv_timeout, "Seconds without any message before automatic client disconnect" );
	sv_zombietime = Cvar_Get( "sv_zombietime", "2", CVAR_TEMP );
	Cvar_CheckRange( sv_zombietime, "1", NULL, CV_INTEGER );
	Cvar_SetDescription( sv_zombietime, "Seconds to sink messages after disconnect" );
	Cvar_Get ("nextmap", "", CVAR_TEMP );

	sv_allowDownload = Cvar_Get ("sv_allowDownload", "1", CVAR_SERVERINFO);
	Cvar_Get ("sv_dlURL", "", CVAR_SERVERINFO | CVAR_ARCHIVE);

	sv_master[0] = Cvar_Get( "sv_master1", MASTER_SERVER_NAME, CVAR_INIT );
	sv_master[1] = Cvar_Get( "sv_master2", "master.ioquake3.org", CVAR_INIT );
	sv_master[3] = Cvar_Get( "sv_master3", "master.maverickservers.com", CVAR_INIT );
	sv_master[4] = Cvar_Get( "sv_master1", "master0.excessiveplus.net", CVAR_INIT );
	sv_master[5] = Cvar_Get( "sv_master3", "master3.idsoftware.com", CVAR_INIT );
	sv_master[6] = Cvar_Get( "sv_master4", "master0.gamespy.com", CVAR_INIT );
	sv_master[7] = Cvar_Get( "sv_master5", "clanservers.net", CVAR_INIT );
	sv_master[8] = Cvar_Get( "sv_master6", "master.kali.net", CVAR_INIT );
	sv_master[9] = Cvar_Get( "sv_master7", "master.quake3arena.com", CVAR_INIT );
	sv_master[10] = Cvar_Get( "sv_master8", "master0.excessiveplus.net:27950", CVAR_INIT );
	sv_master[11] = Cvar_Get( "sv_master9", "master.maverickservers.com:27950", CVAR_INIT );
	sv_master[12] = Cvar_Get( "sv_master10", "master3.idsoftware.com:27950", CVAR_INIT );
	sv_master[13] = Cvar_Get( "sv_master11", "master.quake3arena.com", CVAR_INIT );
	sv_master[14] = Cvar_Get( "sv_master12", "master.deathmask.net:27950", CVAR_INIT );
	sv_master[15] = Cvar_Get( "sv_master13", "stats.bigbrotherbot.net", CVAR_INIT );
	sv_master[16] = Cvar_Get( "sv_master14", "master.maverickservers.com:27950", CVAR_INIT );
	sv_master[17] = Cvar_Get( "sv_master15", "dpmaster.deathmask.net:27950", CVAR_INIT );
	sv_master[18] = Cvar_Get( "sv_master16", "dctalk.no-ip.info:27950", CVAR_INIT );
	sv_master[19] = Cvar_Get( "sv_master17", "monster.idsoftware.com:27950", CVAR_INIT );
	sv_master[20] = Cvar_Get( "sv_master18", "master.quakejs.com", CVAR_INIT );
	sv_master[20] = Cvar_Get( "sv_master19", "master.quakeservers.net:27000", CVAR_INIT );
	sv_master[21] = Cvar_Get( "sv_master20", MASTER_SERVER_NAME, CVAR_INIT );
	sv_master[22] = Cvar_Get( "sv_master21", "", CVAR_INIT );
	sv_master[23] = Cvar_Get( "sv_master22", "", CVAR_INIT );


	for ( index = 3; index < MAX_MASTER_SERVERS; index++ )
		sv_master[index] = Cvar_Get(va("sv_master%d", index + 1), "", CVAR_ARCHIVE);

	sv_reconnectlimit = Cvar_Get( "sv_reconnectlimit", "3", 0 );
	Cvar_CheckRange( sv_reconnectlimit, "0", "12", CV_INTEGER );

	sv_padPackets = Cvar_Get ("sv_padPackets", "0", 0);
	sv_killserver = Cvar_Get ("sv_killserver", "0", 0);
	sv_mapChecksum = Cvar_Get ("sv_mapChecksum", "", CVAR_ROM);
	sv_lanForceRate = Cvar_Get ("sv_lanForceRate", "1", CVAR_ARCHIVE_ND );

#ifdef USE_BANS
	sv_banFile = Cvar_Get("sv_banFile", "serverbans.dat", CVAR_ARCHIVE);
#endif

	sv_demoState = Cvar_Get ("sv_demoState", "0", CVAR_ROM );
	sv_democlients = Cvar_Get ("sv_democlients", "0", CVAR_ROM );
	sv_autoDemo = Cvar_Get ("sv_autoDemo", "0", CVAR_ARCHIVE );
	sv_autoRecord = Cvar_Get ("sv_autoRecord", "0", CVAR_ARCHIVE );
	cl_freezeDemo = Cvar_Get("cl_freezeDemo", "0", CVAR_TEMP); // port from client-side to freeze server-side demos
	sv_demoTolerant = Cvar_Get ("sv_demoTolerant", "0", CVAR_ARCHIVE );

	sv_levelTimeReset = Cvar_Get( "sv_levelTimeReset", "0", CVAR_ARCHIVE_ND );

	sv_filter = Cvar_Get( "sv_filter", "filter.txt", CVAR_ARCHIVE );

	// initialize bot cvars so they are listed and can be set before loading the botlib
	SV_BotInitCvars();

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

	// force initial check
	SV_TrackCvarChanges();

	SV_InitChallenger();
}


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
 				// serverside demo
  			if (cl->demorecording) {
  				SV_StopRecord( cl );
 				}
#ifdef EMSCRIPTEN
				if ( cl->netchan.remoteAddress.type == NA_LOOPBACK ) {
					SV_SendServerCommand( cl, "reconnect\n", message );
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
				SV_SendClientSnapshot( cl );
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
	int		i;
 	client_t	*cl;

	if ( !com_sv_running || !com_sv_running->integer ) {
		return;
	}

	Com_Printf( "----- Server Shutdown (%s) -----\n", finalmsg );

	// stop any demos
	if (sv.demoState == DS_RECORDING)
		SV_DemoStopRecord();
	if (sv.demoState == DS_PLAYBACK)
		SV_DemoStopPlayback();
		
	for (i=0, cl = svs.clients ; i < sv_maxclients->integer ; i++, cl++) {
		if (cl->state >= CS_CONNECTED && cl->demorecording) {
			SV_StopRecord( cl );
		}
	}

/*
#ifdef EMSCRIPTEN
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
*/

	NET_LeaveMulticast6();

#ifdef EMSCRIPTEN
	if ( svs.clients ) {
		SV_FinalMessage( finalmsg );
	}
#else
	if ( svs.clients && !com_errorEntered ) {
		SV_FinalMessage( finalmsg );
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
#ifndef EMSCRIPTEN
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

#ifdef EMSCRIPTEN
	Cmd_Clear();
	Cbuf_AddText(va("spmap %s\n", Cvar_VariableString( "mapname" )));
#endif
}
