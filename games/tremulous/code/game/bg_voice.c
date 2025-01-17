/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2000-2013 Darklegion Development
Copyright (C) 2008      Tony J. White
Copyright (C) 2015-2019 GrangerHub

This file is part of Tremulous.

Tremulous is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 3 of the License,
or (at your option) any later version.

Tremulous is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Tremulous; if not, see <https://www.gnu.org/licenses/>

===========================================================================
*/

// bg_voice.c -- both games voice functions
#include "../qcommon/q_shared.h"
#include "bg_public.h"
#include "bg_local.h"

#if 1
  #define FS_FOpenFileByMode trap_FS_FOpenFile
  #define FS_GetFileList trap_FS_GetFileList
  #define Parse_LoadSourceHandle trap_Parse_LoadSource
  #define Parse_FreeSourceHandle trap_Parse_FreeSource
  #define Parse_ReadTokenHandle trap_Parse_ReadToken
  #define Parse_SourceFileAndLine trap_Parse_SourceFileAndLine
  extern int FS_FOpenFileByMode( const char *qpath, fileHandle_t *f, fsMode_t mode );
  extern int FS_GetFileList( const char *path, const char *extension, char *listbuf, int bufsize );
  extern int Parse_LoadSourceHandle( const char *filename );
  extern int Parse_FreeSourceHandle( int handle );
  extern int Parse_ReadTokenHandle( int handle, pc_token_t *pc_token );
  extern int Parse_SourceFileAndLine( int handle, char *filename, int *line );
#else
  int FS_FOpenFileByMode( const char *qpath, fileHandle_t *f, fsMode_t mode );
  int FS_GetFileList( const char *path, const char *extension, char *listbuf, int bufsize );
  int Parse_LoadSourceHandle( const char *filename );
  int Parse_FreeSourceHandle( int handle );
  int Parse_ReadTokenHandle( int handle, pc_token_t *pc_token );
  int Parse_SourceFileAndLine( int handle, char *filename, int *line );
#endif

#ifdef CGAME
sfxHandle_t trap_S_RegisterSound( const char *sample, qboolean compressed );
int trap_S_SoundDuration( sfxHandle_t handle );
#endif


/*
============
BG_VoiceParseError
============
*/
static void BG_VoiceParseError( fileHandle_t handle, char *err )
{
  int line;
  char filename[ MAX_QPATH ];

  Parse_SourceFileAndLine( handle, filename, &line );
  Parse_FreeSourceHandle( handle );
  Com_Error( ERR_FATAL, "%s on line %d of %s", err, line, filename );
}

/*
============
BG_VoiceList
============
*/
static voice_t *BG_VoiceList( void )
{
  char fileList[ MAX_VOICES * ( MAX_VOICE_NAME_LEN + 8 ) ] = {""};
  int numFiles, i, fileLen = 0;
  int count = 0;
  char *filePtr;
  voice_t *voices = NULL;
  voice_t *top = NULL;

  numFiles = FS_GetFileList( "voice", ".voice", fileList,
    sizeof( fileList ) );

  if( numFiles < 1 )
    return NULL;

  // special case for default.voice.  this file is REQUIRED and will
  // always be loaded first in the event of overflow of voice definitions
  if( !FS_FOpenFileByMode( "voice/default.voice", NULL, FS_READ ) )
  {
    Com_Printf( "voice/default.voice missing, voice system disabled." );
    return NULL;
  }

  voices = (voice_t*)BG_Alloc0( sizeof( voice_t ) );
  Q_strncpyz( voices->name, "default", sizeof( voices->name ) );
  voices->cmds = NULL;
  voices->next = NULL;
  count = 1;

  top = voices;

  filePtr = fileList;
  for( i = 0; i < numFiles; i++, filePtr += fileLen + 1 )
  {
    fileLen = strlen( filePtr );

    // accounted for above
    if( !Q_stricmp( filePtr, "default.voice" ) )
      continue;

    if( fileLen > MAX_VOICE_NAME_LEN + 8 ) {
      Com_Printf( S_COLOR_YELLOW "WARNING: MAX_VOICE_NAME_LEN is %d. "
        "skipping \"%s\", filename too long", MAX_VOICE_NAME_LEN, filePtr );
      continue;
    }

    // FS_GetFileList() buffer has overflowed
    if( !FS_FOpenFileByMode( va( "voice/%s", filePtr ), NULL, FS_READ ) )
    {
      Com_Printf( S_COLOR_YELLOW "WARNING: BG_VoiceList(): detected "
        "an invalid .voice file \"%s\" in directory listing.  You have "
        "probably named one or more .voice files with outrageously long "
        "names.  gjbs", filePtr );
      break;
    }

    if( count >= MAX_VOICES )
    {
      Com_Printf( S_COLOR_YELLOW "WARNING: .voice file overflow.  "
        "%d of %d .voice files loaded.  MAX_VOICES is %d",
        count, numFiles, MAX_VOICES );
      break;
    }

    voices->next = (voice_t*)BG_Alloc0( sizeof( voice_t ) );
    voices = voices->next;

    Q_strncpyz( voices->name, filePtr, sizeof( voices->name ) );
    // strip extension
    voices->name[ fileLen - 6 ] = '\0';
    voices->cmds = NULL;
    voices->next = NULL;
    count++;
  }
  return top;
}

/*
============
BG_VoiceParseTrack
============
*/
static qboolean BG_VoiceParseTrack( int handle, voiceTrack_t *voiceTrack )
{
  pc_token_t token;
  qboolean found = qfalse;
  qboolean foundText = qfalse;
  qboolean foundToken = qfalse;

  foundToken = Parse_ReadTokenHandle( handle, &token );
  while( foundToken )
  {
    if( token.string[ 0 ] == '}' )
    {
      if( foundText )
        return qtrue;
      else
      {
        BG_VoiceParseError( handle, "BG_VoiceParseTrack(): "
          "missing text attribute for track" );
      }
    }
    else if( !Q_stricmp( token.string, "team" ) )
    {
      foundToken = Parse_ReadTokenHandle( handle, &token );
      found = qfalse;
      while( foundToken && token.type == TT_NUMBER )
      {
        found = qtrue;
        if( voiceTrack->team < 0 )
          voiceTrack->team = 0;
        voiceTrack->team |= ( 1 << token.intvalue );
        foundToken = Parse_ReadTokenHandle( handle, &token );
      }
      if( !found )
      {
        BG_VoiceParseError( handle,
          "BG_VoiceParseTrack(): missing \"team\" value" );
      }
      continue;
    }
    else if( !Q_stricmp( token.string, "class" ) )
    {
      foundToken = Parse_ReadTokenHandle( handle, &token );
      found = qfalse;
      while( foundToken && token.type == TT_NUMBER )
      {
        found = qtrue;
        if( voiceTrack->class < 0 )
          voiceTrack->class = 0;
        voiceTrack->class |= ( 1 << token.intvalue );
        foundToken = Parse_ReadTokenHandle( handle, &token );
      }
      if( !found )
      {
        BG_VoiceParseError( handle,
          "BG_VoiceParseTrack(): missing \"class\" value" );
      }
      continue;
    }
    else if( !Q_stricmp( token.string, "weapon" ) )
    {
      foundToken = Parse_ReadTokenHandle( handle, &token );
      found = qfalse;
      while( foundToken && token.type == TT_NUMBER )
      {
        found = qtrue;
        if( voiceTrack->weapon < 0 )
          voiceTrack->weapon = 0;
        voiceTrack->weapon |= ( 1 << token.intvalue );
        foundToken = Parse_ReadTokenHandle( handle, &token );
      }
      if( !found )
      {
        BG_VoiceParseError( handle,
          "BG_VoiceParseTrack(): missing \"weapon\" value");
      }
      continue;
    }
    else if( !Q_stricmp( token.string, "text" ) )
    {
      if( foundText )
      {
        BG_VoiceParseError( handle, "BG_VoiceParseTrack(): "
          "duplicate \"text\" definition for track" );
      }
      foundToken = Parse_ReadTokenHandle( handle, &token );
      if( !foundToken )
      {
        BG_VoiceParseError( handle, "BG_VoiceParseTrack(): "
          "missing \"text\" value" );
      }
      foundText = qtrue;
      if( strlen( token.string ) >= MAX_SAY_TEXT )
      {
        BG_VoiceParseError( handle, va( "BG_VoiceParseTrack(): "
          "\"text\" value " "\"%s\" exceeds MAX_SAY_TEXT length",
          token.string ) );
      }

      voiceTrack->text = (char *)BG_Alloc0( strlen( token.string ) + 1 );
      Q_strncpyz( voiceTrack->text, token.string, strlen( token.string ) + 1 );
      foundToken = Parse_ReadTokenHandle( handle, &token );
      continue;
    }
    else if( !Q_stricmp( token.string, "enthusiasm" ) )
    {
      foundToken = Parse_ReadTokenHandle( handle, &token );
      if( token.type == TT_NUMBER )
      {
        voiceTrack->enthusiasm = token.intvalue;
      }
      else
      {
        BG_VoiceParseError( handle, "BG_VoiceParseTrack(): "
          "missing \"enthusiasm\" value" );
      }
      foundToken = Parse_ReadTokenHandle( handle, &token );
      continue;
    }
    else
    {
        BG_VoiceParseError( handle, va( "BG_VoiceParseTrack():"
          " unknown token \"%s\"", token.string ) );
    }
  }
  return qfalse;
}

/*
============
BG_VoiceParseCommand
============
*/
static voiceTrack_t *BG_VoiceParseCommand( int handle )
{
  pc_token_t token;
  qboolean parsingTrack = qfalse;
  voiceTrack_t *voiceTracks = NULL;
  voiceTrack_t *top = NULL;

  while( Parse_ReadTokenHandle( handle, &token ) )
  {
    if( !parsingTrack && token.string[ 0 ] == '}' )
      return top;

    if( parsingTrack )
    {
      if( token.string[ 0 ] == '{' )
      {
        BG_VoiceParseTrack( handle, voiceTracks );
        parsingTrack = qfalse;
        continue;

      }
      else
      {
        BG_VoiceParseError( handle, va( "BG_VoiceParseCommand(): "
          "parse error at \"%s\"", token.string ) );
      }
    }


    if( top == NULL )
    {
      voiceTracks = BG_Alloc0( sizeof( voiceTrack_t ) );
      top = voiceTracks;
    }
    else
    {
      voiceTracks->next = BG_Alloc0( sizeof( voiceCmd_t ) );
      voiceTracks = voiceTracks->next;
    }

    if( !FS_FOpenFileByMode( token.string, NULL, FS_READ ) )
    {
        int line;
        char filename[ MAX_QPATH ];

        Parse_SourceFileAndLine( handle, filename, &line );
        Com_Printf( S_COLOR_YELLOW "WARNING: BG_VoiceParseCommand(): "
          "track \"%s\" referenced on line %d of %s does not exist\n",
          token.string, line, filename );
    }
    else
    {
#ifdef CGAME
      voiceTracks->track = trap_S_RegisterSound( token.string, qfalse );
      voiceTracks->duration = trap_S_SoundDuration( voiceTracks->track );
#endif
    }

    voiceTracks->team = -1;
    voiceTracks->class = -1;
    voiceTracks->weapon = -1;
    voiceTracks->enthusiasm = 0;
    voiceTracks->text = NULL;
    voiceTracks->next = NULL;
    parsingTrack = qtrue;

  }
  return NULL;
}

/*
============
BG_VoiceParse
============
*/
static voiceCmd_t *BG_VoiceParse( char *name )
{
  voiceCmd_t *voiceCmds = NULL;
  voiceCmd_t *top = NULL;
  pc_token_t token;
  qboolean parsingCmd = qfalse;
  int handle;

  handle = Parse_LoadSourceHandle( va( "voice/%s.voice", name ) );
  if( !handle )
    return NULL;

  while( Parse_ReadTokenHandle( handle, &token ) )
  {
    if( parsingCmd )
    {
      if( token.string[ 0 ] == '{' )
      {
        voiceCmds->tracks = BG_VoiceParseCommand( handle );
        parsingCmd = qfalse;
	continue;
      }
      else
      {
        int line;
        char filename[ MAX_QPATH ];

        Parse_SourceFileAndLine( handle, filename, &line );
        Com_Error( ERR_FATAL, "BG_VoiceParse(): "
          "parse error on line %d of %s", line, filename );
      }
    }

    if( strlen( token.string ) >= MAX_VOICE_CMD_LEN )
    {
        int line;
        char filename[ MAX_QPATH ];

        Parse_SourceFileAndLine( handle, filename, &line );
        Com_Error( ERR_FATAL, "BG_VoiceParse(): "
          "command \"%s\" exceeds MAX_VOICE_CMD_LEN (%d) on line %d of %s",
          token.string, MAX_VOICE_CMD_LEN, line, filename );
    }

    if( top == NULL )
    {
      voiceCmds = BG_Alloc0( sizeof( voiceCmd_t ) );
      top = voiceCmds;
    }
    else
    {
      voiceCmds->next = BG_Alloc0( sizeof( voiceCmd_t ) );
      voiceCmds = voiceCmds->next;
    }

    Q_strncpyz( voiceCmds->cmd, token.string, sizeof( voiceCmds->cmd ) );
    voiceCmds->next = NULL;
    parsingCmd = qtrue;

  }

  Parse_FreeSourceHandle( handle );

  return top;
}

/*
============
BG_VoiceInit
============
*/
voice_t *BG_VoiceInit( void )
{
  voice_t *voices;
  voice_t *voice;

  voices = BG_VoiceList();

  voice = voices;
  while( voice )
  {
    voice->cmds = BG_VoiceParse( voice->name );
    voice = voice->next;
  }

  return voices;
}


/*
============
BG_PrintVoices
============
*/
void BG_PrintVoices( voice_t *voices, int debugLevel )
{
  voice_t *voice = voices;
  voiceCmd_t *voiceCmd;
  voiceTrack_t *voiceTrack;

  int cmdCount;
  int trackCount;

  if( voice == NULL )
  {
    Com_Printf( "voice list is empty\n" );
    return;
  }

  while( voice != NULL )
  {
    if( debugLevel > 0 )
      Com_Printf( "voice \"%s\"\n", voice->name );
    voiceCmd = voice->cmds;
    cmdCount = 0;
    trackCount = 0;
    while( voiceCmd != NULL )
    {
      if( debugLevel > 0 )
        Com_Printf( "  %s\n", voiceCmd->cmd );
      voiceTrack = voiceCmd->tracks;
      cmdCount++;
      while ( voiceTrack != NULL )
      {
        if( debugLevel > 1 )
          Com_Printf( "    text -> %s\n", voiceTrack->text );
        if( debugLevel > 2 )
        {
          Com_Printf( "    team -> %d\n", voiceTrack->team );
          Com_Printf( "    class -> %d\n", voiceTrack->class );
          Com_Printf( "    weapon -> %d\n", voiceTrack->weapon );
          Com_Printf( "    enthusiasm -> %d\n", voiceTrack->enthusiasm );
#ifdef CGAME
          Com_Printf( "    duration -> %d\n", voiceTrack->duration );
#endif
        }
        if( debugLevel > 1 )
          Com_Printf( "\n" );
        trackCount++;
        voiceTrack = voiceTrack->next;
      }
      voiceCmd = voiceCmd->next;
    }

    if( !debugLevel )
    {
      Com_Printf( "voice \"%s\": %d commands, %d tracks\n",
        voice->name, cmdCount, trackCount );
    }
    voice = voice->next;
  }
}

/*
============
BG_VoiceByName
============
*/
voice_t *BG_VoiceByName( voice_t *head, char *name )
{
  voice_t *v = head;

  while( v )
  {
    if( !Q_stricmp( v->name, name ) )
      return v;
    v = v->next;
  }
  return NULL;
}

/*
============
BG_VoiceCmdFind
============
*/
voiceCmd_t *BG_VoiceCmdFind( voiceCmd_t *head, char *name, int *cmdNum )
{
  voiceCmd_t *vc = head;
  int i = 0;

  while( vc )
  {
    i++;
    if( !Q_stricmp( vc->cmd, name ) )
    {
      *cmdNum = i;
      return vc;
    }
    vc = vc->next;
  }
  return NULL;
}

/*
============
BG_VoiceCmdByNum
============
*/
voiceCmd_t *BG_VoiceCmdByNum( voiceCmd_t *head, int num )
{
  voiceCmd_t *vc = head;
  int i = 0;

  while( vc )
  {
    i++;
    if( i == num )
      return vc;
    vc = vc->next;
  }
  return NULL;
}

/*
============
BG_VoiceTrackByNum
============
*/
voiceTrack_t *BG_VoiceTrackByNum( voiceTrack_t *head, int num )
{
  voiceTrack_t *vt = head;
  int i = 0;

  while( vt )
  {
    i++;
    if( i == num )
      return vt;
    vt = vt->next;
  }
  return NULL;
}

/*
============
BG_VoiceTrackFind
============
*/
voiceTrack_t *BG_VoiceTrackFind( voiceTrack_t *head, team_t team,
                                 class_t class, weapon_t weapon,
                                 int enthusiasm, int *trackNum )
{
  voiceTrack_t *vt = head;
  int highestMatch = 0;
  int matchCount = 0;
  int selectedMatch = 0;
  int i = 0;
  int j = 0;

  // find highest enthusiasm without going over
  while( vt )
  {
    if( ( vt->team >= 0 && !( vt->team  & ( 1 << team ) ) ) ||
        ( vt->class >= 0 && !( vt->class & ( 1 << class ) ) ) ||
        ( vt->weapon >= 0 && !( vt->weapon & ( 1 << weapon ) ) ) ||
        vt->enthusiasm > enthusiasm )
    {
      vt = vt->next;
      continue;
    }

    if( vt->enthusiasm > highestMatch )
    {
      matchCount = 0;
      highestMatch = vt->enthusiasm;
    }
    if( vt->enthusiasm == highestMatch )
      matchCount++;
    vt = vt->next;
  }

  if( !matchCount )
    return NULL;

  // return randomly selected match
  selectedMatch = rand() / ( RAND_MAX / matchCount + 1 );
  vt = head;
  i = 0;
  j = 0;
  while( vt )
  {
    j++;
    if( ( vt->team >= 0 && !( vt->team  & ( 1 << team ) ) ) ||
        ( vt->class >= 0 && !( vt->class & ( 1 << class ) ) ) ||
        ( vt->weapon >= 0 && !( vt->weapon & ( 1 << weapon ) ) ) ||
        vt->enthusiasm != highestMatch )
    {
      vt = vt->next;
      continue;
    }
    if( i == selectedMatch )
    {
      *trackNum = j;
      return vt;
    }
    i++;
    vt = vt->next;
  }
  return NULL;
}
