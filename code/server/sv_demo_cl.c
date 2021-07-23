
#ifdef USE_DEMO_CLIENTS

#include "server.h"


/*
=======================================================================

SERVER-SIDE CLIENT DEMO RECORDING

=======================================================================
*/
/*
====================
SV_WriteDemoMessage

Dumps the current net message, prefixed by the length
====================
*/

void SV_WriteDemoMessage (client_t *cl, msg_t *msg, int headerBytes ) {
 	int		len, swlen;

 	MSG_WriteByte( msg, svc_EOF );//temporarily write svc_EOF
 	// write the packet sequence
 	FS_Write (&LittleLong( cl->netchan.outgoingSequence ), 4, cl->demofile);

 	// skip the packet sequencing information
 	len = msg->cursize - headerBytes;
 	swlen = LittleLong(len);
 	FS_Write (&swlen, 4, cl->demofile);
 	FS_Write ( msg->data + headerBytes, len, cl->demofile );
 	msg->cursize--;// forget about svc_EOF
}


/*
====================
SV_StopRecording_f

stoprecord <slot>

stop recording a demo
====================
*/
void SV_StopRecord( client_t	*cl ) {
 	int		len;
 	int         clientnum;
 	playerState_t	*ps;
 	clientnum = cl - svs.clients;
 	cvar_t	*fraglimit;

 	if ( !cl->demorecording || !cl->demofile ) {
 		Com_Printf ("DEMO: Not recording a demo for client %i.\n", clientnum);
 		return;
 	}

 	// finish up
 	len = -1;
 	FS_Write (&len, 4, cl->demofile);
 	FS_Write (&len, 4, cl->demofile);
 	FS_FCloseFile (cl->demofile);


 	if (!sv_autoRecord->integer) {
 		Com_Printf ("DEMO: Stopped demo for client %i.\n", clientnum);
 	} else {
 		ps = SV_GameClientNum( clientnum );
 		Com_Printf ("DEMO: ps->persistant[PERS_SCORE] = %i\n",ps->persistant[PERS_SCORE]);
 		fraglimit = Cvar_Get("fraglimit", "30", CVAR_SERVERINFO);
 		if (cl->savedemo 
      || ((ps->pm_type == PM_INTERMISSION || ps->clientNum==clientnum)
      && (sv_autoRecordThreshold->value == 0
        || ps->persistant[PERS_SCORE]>=
          (int)((float)fraglimit->integer*sv_autoRecordThreshold->value)))) {
 			// keep demo
 			Com_Printf ("DEMO: Stoprecord: %i: stop\n", clientnum);
			Com_Printf ("DEMO: %s's serverside demo was recorded as %s\n", cl->name, cl->demoName);
			// SV_SendServerCommand(NULL, "print \"%s^7's game was recorded as ^2http:/ /bb.game-host.org/oa/^3%s\n\"", cl->name, cl->demoName);
 		} else {
 			Com_Printf ("DEMO: Stoprecord: %i: abort\n", clientnum);
			//FS_HomeRemove(cl->demoName);
 		}
 	}

 	cl->demofile = 0;
 	cl->demorecording = qfalse;
}


void SV_StopRecord_f( void ) {
 	client_t	*cl;

 	// make sure server is running
 	if ( !com_sv_running->integer ) {
 		Com_Printf( "Server is not running.\n" );
 		return;
 	}

 	if ( Cmd_Argc() < 2 ) {
 		Com_Printf ("Usage: stoprecord <clientnumber>\n");
 		return;
 	}

 	cl = SV_GetPlayerByNum();
 	if ( !cl ) {
 		// error message was printed by SV_GetPlayerByNum
 		return;
 	}
 	SV_StopRecord(cl);
}



/*
====================
SV_Record_f

record <slot> [<demoname>]

Begins recording a demo from the current position
====================
*/
void SV_Record( client_t	*cl, char *s ) {
 	char		name[MAX_OSPATH];
 	char		name_zip[MAX_OSPATH];
 	byte		bufData[MAX_MSGLEN];
 	msg_t	buf;
 	int			i;
 	int			len;
 	entityState_t	*ent;
	entityState_t	nullstate;
 	int         clientnum;
 	char	*guid;
 	char	prefix[MAX_OSPATH];

 	clientnum = cl - svs.clients;

#ifdef USE_DEMO_SERVER
  if( sv.demoState == DS_PLAYBACK ) {
    Com_Printf ("DEMO: not recording playback during playback\n");
    return;
  }
#endif

 	if ( cl->demorecording ) {
 		Com_Printf ("DEMO: Already recording client %i.\n", clientnum);
 		return;
 	}

//	if ( cl->state != CS_ACTIVE ) {
//		Com_Printf ("You must be in a level to record.\n");
//		return;
//	}
//  // sync 0 doesn't prevent recording, so not forcing it off .. everyone does g_sync 1 ; record ; g_sync 0 ..
//	if ( NET_IsLocalAddress( cl->serverAddress ) && !Cvar_VariableValue( "g_synchronousClients" ) ) {
//		Com_Printf (S_COLOR_YELLOW "WARNING: You should set 'g_synchronousClients 1' for smoother demo recording\n");
//	}

 	if ( s ) {
 		Com_sprintf (name, sizeof(name), "demos/%s.dm_%d", s, PROTOCOL_VERSION );
 	} else {
 		int		number,n,a,b,c,d;
 		guid = Info_ValueForKey(cl->userinfo, "cl_guid");
 		if (!Q_stricmp(guid, "")) {
 			guid = "LONGGONE";
		}
 		Q_strncpyz(prefix, guid, 9);

 		// scan for a free demo name
 		for ( number = 0 ; number <= 9999 ; number++ ) {
 			if(number < 0 || number > 9999)
 				number = 9999;
 			n = number;
 			a = n / 1000;
 			n -= a*1000;
 			b = n / 100;
 			n -= b*100;
 			c = n / 10;
 			n -= c*10;
 			d = n;

 			Com_sprintf (name, sizeof(name), "demos/%s_%s_%i%i%i%i.dm_%d", prefix, sv_mapname->string, a, b, c, d, PROTOCOL_VERSION );
 			Com_sprintf (name_zip, sizeof(name_zip), "demos/%s_%s_%i%i%i%i.dm_%d.zip", prefix, sv_mapname->string, a, b, c, d, PROTOCOL_VERSION );
 			if (!FS_FileExists(name) && !FS_FileExists(name_zip)) {
 				break;	// file doesn't exist
 			}
 		}
 	}

 	// open the demo file

 	if (!sv_autoRecord->integer) {
 		Com_Printf ("DEMO: recording client %i to %s.\n", clientnum, name);
 	} else {
 		Com_Printf ("DEMO: Record: %i: %s\n", clientnum, name);
 	}

 	cl->demofile = FS_FOpenFileWrite( name );
 	if ( !cl->demofile ) {
 		Com_Printf ("ERROR: couldn't open.\n");
 		return;
 	}
 	// don't start saving messages until a non-delta compressed message is received
 	cl->demowaiting = qtrue;
 	cl->demorecording = qtrue;

 	Q_strncpyz( cl->demoName, name, sizeof( cl->demoName ) );

 	// write out the gamestate message
 	MSG_Init (&buf, bufData, sizeof(bufData));
 	MSG_Bitstream(&buf);

 	// NOTE, MRE: all server->client messages now acknowledge
 	MSG_WriteLong( &buf, cl->lastClientCommand );// 0007 - 000A

 	MSG_WriteByte (&buf, svc_gamestate);// 000B
 	MSG_WriteLong (&buf, cl->reliableSequence );// 000C - 000F


 	// write the configstrings
 	for ( i = 0 ; i < MAX_CONFIGSTRINGS ; i++ ) {
 		if (sv.configstrings[i][0]) {
 			MSG_WriteByte( &buf, svc_configstring );
 			MSG_WriteShort( &buf, i );
 			MSG_WriteBigString( &buf, sv.configstrings[i] );
 		}
 	}

 	// write the baselines
 	Com_Memset( &nullstate, 0, sizeof( nullstate ) );
 	for ( i = 0 ; i < MAX_GENTITIES; i++ ) {
 		ent = &sv.svEntities[i].baseline;
 		if ( !ent->number ) {
 			continue;
 		}
 		MSG_WriteByte( &buf, svc_baseline );
 		MSG_WriteDeltaEntity( &buf, &nullstate, ent, qtrue );
 	}

 	MSG_WriteByte( &buf, svc_EOF );

 	// finished writing the gamestate stuff

 	// write the client num
 	MSG_WriteLong(&buf, clientnum);
 	// write the checksum feed
 	MSG_WriteLong(&buf, sv.checksumFeed);

 	// finished writing the client packet
 	MSG_WriteByte( &buf, svc_EOF );

 	// write it to the demo file
 	len = LittleLong( cl->netchan.outgoingSequence-1 );
 	FS_Write (&len, 4, cl->demofile);// 0000 - 0003

 	len = LittleLong (buf.cursize);
 	FS_Write (&len, 4, cl->demofile);// 0004 - 0007
 	FS_Write (buf.data, buf.cursize, cl->demofile);// 0007 - ...

 	// the rest of the demo file will be copied from net messages
}


void SV_Record_f( void ) {
 	char		*s;
 	client_t	*cl;

 	s = 0;
 	// make sure server is running
 	if ( !com_sv_running->integer ) {
 		Com_Printf( "Server is not running.\n" );
 		return;
 	}

 	if ( Cmd_Argc() < 2 || Cmd_Argc() > 3) {
 		Com_Printf ("record <clientnumber> [<demoname>]\n");
 		return;
 	}

 	cl = SV_GetPlayerByNum();
 	if ( !cl ) {
 		// error message was printed by SV_GetPlayerByNum
 		return;
 	}

 	if ( Cmd_Argc() == 3 ) {
 		s = Cmd_Argv(2);
		// Com_sprintf (name, sizeof(name), "demos/%s.dm_%d", s, PROTOCOL_VERSION );
 		SV_Record(cl,s);
 	} else {
 		SV_Record(cl,0);
 	}
}


void SV_SaveRecord_f( void ) {
 	char		*s;
 	client_t	*cl;

 	s = 0;
 	// make sure server is running
 	if ( !com_sv_running->integer ) {
 		Com_Printf( "Server is not running.\n" );
 		return;
 	}

 	if ( Cmd_Argc() != 2) {
 		Com_Printf ("saverecord <clientnumber>\n");
 		return;
 	}

 	cl = SV_GetPlayerByNum();
 	if ( !cl ) {
 		// error message was printed by SV_GetPlayerByNum
 		return;
 	}
 	if ( !cl->demorecording ) {
 		Com_Printf ("Not recording a demo for client %li.\n", cl - svs.clients);
 		return;
 	}
 	cl->savedemo = qtrue;
	Com_Printf ("Saverecord: %li\n", cl - svs.clients);
}

#endif
