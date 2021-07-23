/*
===========================================================================
Copyright (C) 2006 Tony J. White (tjw@tjw.org)

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

#ifdef USE_CURL
#include "client.h"
cvar_t *cl_cURLLib;

#define ALLOWED_PROTOCOLS ( CURLPROTO_HTTP | CURLPROTO_HTTPS | CURLPROTO_FTP | CURLPROTO_FTPS )


/*
==================================

Common CURL downloading functions

==================================
*/


/*
==================================
replace1
==================================
*/
int replace1( const char src, const char dst, char *str )
{
	int count;

	if ( !str ) 
		return 0;

	count = 0;

	while ( *str != '\0' )
	{
		if ( *str == src )
		{
			*str = dst;
			count++;
		}
		str++;
	}

	return count;
}


/*
=================
Com_DL_Done
=================
*/
void Com_DL_Done( download_t *dl ) 
{
	if ( dl->func.lib )
		Sys_UnloadLibrary( dl->func.lib );
	dl->func.lib = NULL;
	memset( &dl->func, 0, sizeof( dl->func ) );
}


/*
=================
Com_DL_Init
=================
*/
qboolean Com_DL_Init( download_t *dl )
{
#ifdef USE_CURL_DLOPEN
	Com_Printf( "Loading \"%s\"...", cl_cURLLib->string );
	if( ( dl->func.lib = Sys_LoadLibrary( cl_cURLLib->string ) ) == NULL )
	{
#ifdef _WIN32
		return qfalse;
#else
		char fn[1024];

		Q_strncpyz( fn, Sys_Pwd(), sizeof( fn ) );
		strncat( fn, "/", sizeof( fn ) - strlen( fn ) - 1 );
		strncat( fn, cl_cURLLib->string, sizeof( fn ) - strlen( fn ) - 1 );

		if ( ( dl->func.lib = Sys_LoadLibrary( fn ) ) == NULL )
		{
#ifdef ALTERNATE_CURL_LIB
			// On some linux distributions there is no libcurl.so.3, but only libcurl.so.4. That one works too.
			if( ( dl->func.lib = Sys_LoadLibrary( ALTERNATE_CURL_LIB ) ) == NULL )
			{
				return qfalse;
			}
#else
			return qfalse;
#endif
		}
#endif /* _WIN32 */
	}

	Sys_LoadFunctionErrors(); // reset error count;

	dl->func.version = Sys_LoadFunction( dl->func.lib, "curl_version" );
	dl->func.easy_escape = Sys_LoadFunction( dl->func.lib, "curl_easy_escape" );
	dl->func.free = Sys_LoadFunction( dl->func.lib, "curl_free" );

	dl->func.easy_init = Sys_LoadFunction( dl->func.lib, "curl_easy_init" );
	dl->func.easy_setopt = Sys_LoadFunction( dl->func.lib, "curl_easy_setopt" );
	dl->func.easy_perform = Sys_LoadFunction( dl->func.lib, "curl_easy_perform" );
	dl->func.easy_cleanup = Sys_LoadFunction( dl->func.lib, "curl_easy_cleanup" );
	dl->func.easy_getinfo = Sys_LoadFunction( dl->func.lib, "curl_easy_getinfo" );
	dl->func.easy_strerror = Sys_LoadFunction( dl->func.lib, "curl_easy_strerror" );
	
	dl->func.multi_init = Sys_LoadFunction( dl->func.lib, "curl_multi_init" );
	dl->func.multi_add_handle = Sys_LoadFunction( dl->func.lib, "curl_multi_add_handle" );
	dl->func.multi_remove_handle = Sys_LoadFunction( dl->func.lib, "curl_multi_remove_handle" );
	dl->func.multi_perform = Sys_LoadFunction( dl->func.lib, "curl_multi_perform" );
	dl->func.multi_cleanup = Sys_LoadFunction( dl->func.lib, "curl_multi_cleanup" );
	dl->func.multi_info_read = Sys_LoadFunction( dl->func.lib, "curl_multi_info_read" );
	dl->func.multi_strerror = Sys_LoadFunction( dl->func.lib, "curl_multi_strerror" );

	if ( Sys_LoadFunctionErrors() )
	{
		Com_DL_Done( dl );
		Com_Printf( "FAIL: One or more symbols not found\n" );
		return qfalse;
	}

	Com_Printf( "OK\n" );

	return qtrue;
#else

	dl->func.lib = NULL;

	dl->func.version = curl_version;
	dl->func.easy_escape = curl_easy_escape;
	dl->func.free = (void (*)(char *))curl_free; // cast to silence warning

	dl->func.easy_init = curl_easy_init;
	dl->func.easy_setopt = curl_easy_setopt;
	dl->func.easy_perform = curl_easy_perform;
	dl->func.easy_cleanup = curl_easy_cleanup;
	dl->func.easy_getinfo = curl_easy_getinfo;
	dl->func.easy_strerror = curl_easy_strerror;
	
	dl->func.multi_init = curl_multi_init;
	dl->func.multi_add_handle = curl_multi_add_handle;
	dl->func.multi_remove_handle = curl_multi_remove_handle;
	dl->func.multi_perform = curl_multi_perform;
	dl->func.multi_cleanup = curl_multi_cleanup;
	dl->func.multi_info_read = curl_multi_info_read;
	dl->func.multi_strerror = curl_multi_strerror;

	return qtrue;
#endif /* USE_CURL_DLOPEN */
}


/*
=================
Com_DL_Cleanup
=================
*/
qboolean Com_DL_InProgress( const download_t *dl )
{
	if ( dl->cURL && dl->URL[0] )
		return qtrue;
	else
		return qfalse;
}


/*
=================
Com_DL_Cleanup
=================
*/
void Com_DL_Cleanup( download_t *dl )
{
	if( dl->cURLM )
	{
		if ( dl->cURL )
		{
			dl->func.multi_remove_handle( dl->cURLM, dl->cURL );
			dl->func.easy_cleanup( dl->cURL );
		}
		dl->func.multi_cleanup( dl->cURLM );
		dl->cURLM = NULL;
		dl->cURL = NULL;
	}
	else if( dl->cURL )
	{
		dl->func.easy_cleanup( dl->cURL );
		dl->cURL = NULL;
	}
	if ( dl->fHandle != FS_INVALID_HANDLE )
	{
		FS_FCloseFile( dl->fHandle );
		dl->fHandle = FS_INVALID_HANDLE;
	}

	if ( dl->mapAutoDownload )
	{
		Cvar_Set( "cl_downloadName", "" );
		Cvar_Set( "cl_downloadSize", "0" );
		Cvar_Set( "cl_downloadCount", "0" );
		Cvar_Set( "cl_downloadTime", "0" );
	}

	dl->Size = 0;
	dl->Count = 0;

	dl->URL[0] = '\0';
	dl->Name[0] = '\0';
	if ( dl->TempName[0] )
	{
		FS_Remove( dl->TempName );
	}
	dl->TempName[0] = '\0';
	dl->progress[0] = '\0';
	dl->headerCheck = qfalse;
	dl->mapAutoDownload = qfalse;

	Com_DL_Done( dl );
}


static const char *sizeToString( int size )
{
	static char buf[ 32 ];
	if ( size < 1024 ) {
		sprintf( buf, "%iB", size );
	} else if ( size < 1024*1024 ) {
		sprintf( buf, "%iKB", size / 1024 );
	} else {
		sprintf( buf, "%i.%iMB", size / (1024*1024), (size / (1024*1024/10 )) % 10 );
	}
	return buf;
}


/*
=================
Com_DL_CallbackProgress
=================
*/
static int Com_DL_CallbackProgress( void *data, double dltotal, double dlnow, double ultotal, double ulnow )
{
	double percentage, speed;
	download_t *dl = (download_t *)data;

	dl->Size = (int)dltotal;
	dl->Count = (int)dlnow;

	if ( dl->mapAutoDownload && cls.state == CA_CONNECTED )
	{
		if ( Key_IsDown( K_ESCAPE ) )
		{
			Com_Printf( "%s: aborted\n", dl->Name );
			return -1;
		}
		Cvar_Set( "cl_downloadSize", va( "%i", dl->Size ) );
		Cvar_Set( "cl_downloadCount", va( "%i", dl->Count ) );
	}

	if ( dl->Size ) {
		percentage = ( dlnow / dltotal ) * 100.0;
		sprintf( dl->progress, " downloading %s: %s (%i%%)", dl->Name, sizeToString( dl->Count ), (int)percentage );
	} else {
		sprintf( dl->progress, " downloading %s: %s", dl->Name, sizeToString( dl->Count ) );
	}

	if ( dl->func.easy_getinfo( dl->cURL, CURLINFO_SPEED_DOWNLOAD, &speed ) == CURLE_OK ) {
		Q_strcat( dl->progress, sizeof( dl->progress ), va( " %s/s", sizeToString( (int)speed ) ) );
	}

	return 0;
}


/*
=================
Com_DL_CallbackWrite
=================
*/
static size_t Com_DL_CallbackWrite( void *ptr, size_t size, size_t nmemb, void *userdata )
{
	download_t *dl;

	dl = (download_t *)userdata;

	if ( dl->fHandle == FS_INVALID_HANDLE )
	{
		if ( !CL_ValidPakSignature( ptr, size*nmemb ) ) 
		{
			Com_Printf( S_COLOR_YELLOW "Com_DL_CallbackWrite(): invalid pak signature for %s.\n",
				dl->Name );
			return (size_t)-1;
		}

		dl->fHandle = FS_SV_FOpenFileWrite( dl->TempName );
		if ( dl->fHandle == FS_INVALID_HANDLE ) 
		{
			return (size_t)-1;
		}
	}

	FS_Write( ptr, size*nmemb, dl->fHandle );

	return (size * nmemb);
}


/*
=================
Com_DL_ValidFileName
=================
*/
qboolean Com_DL_ValidFileName( const char *fileName )
{
	int c;
	while ( (c = *fileName++) != '\0' )
	{
		if ( c == '/' || c == '\\' || c == ':' )
			return qfalse;
		if ( c < ' ' || c > '~' )
			return qfalse;
	}
	return qtrue;
}


/*
=================
Com_DL_HeaderCallback
=================
*/
static size_t Com_DL_HeaderCallback( void *ptr, size_t size, size_t nmemb, void *userdata )
{
	char name[MAX_OSPATH];
	char header[1024], *s, quote, *d;
	download_t *dl;
	int len;

	if ( size*nmemb >= sizeof( header ) )
	{
		Com_Printf( S_COLOR_RED "Com_DL_HeaderCallback: header is too large." );
		return (size_t)-1;
	}

	dl = (download_t *)userdata;
	
	memcpy( header, ptr, size*nmemb+1 );
	header[ size*nmemb ] = '\0';

	//Com_Printf( "h: %s\n--------------------------\n", header );

	s = (char*)Q_stristr( header, "content-disposition:" );
	if ( s ) 
	{
		s += 20; // strlen( "content-disposition:" )
		s = (char*)Q_stristr( s, "filename=" );
		if ( s ) 
		{
			s += 9; // strlen( "filename=" )
			
			d = name;
			replace1( '\r', '\0', s );
			replace1( '\n', '\0', s );

			// prevent overflow
			if ( strlen( s ) >= sizeof( name ) )
				s[ sizeof( name ) - 1 ] = '\0';

			if ( *s == '\'' || *s == '"' )
				quote = *s++;
			else
				quote = '\0';

			// copy filename
			while ( *s != '\0' && *s != quote ) 
				*d++ = *s++;
			len = d - name;
			*d++ = '\0';

			// validate
			if ( len < 5 || !Q_stristr( name + len - 4, ".pk3" ) || !Com_DL_ValidFileName( name ) )
			{
				Com_Printf( S_COLOR_RED "Com_DL_HeaderCallback: bad file name '%s'\n", name );
				return (size_t)-1;
			}

			// strip extension
			FS_StripExt( name, ".pk3" );

			// store in
			strcpy( dl->Name, name );
		}
	}

	return size*nmemb;
}


/*
===============================================================
Com_DL_Begin()

Start downloading file from remoteURL and save it under fs_game/localName
==============================================================
*/
qboolean Com_DL_Begin( download_t *dl, const char *localName, const char *remoteURL, qboolean autoDownload )
{
	char *s;

	if ( Com_DL_InProgress( dl ) )
	{
		Com_Printf( S_COLOR_YELLOW " already downloading %s\n", dl->Name );
		return qfalse;
	}

	Com_DL_Cleanup( dl );

	if ( !Com_DL_Init( dl ) ) 
	{
		Com_Printf( S_COLOR_YELLOW "Error initializing cURL library\n" );
		return qfalse;
	}

	dl->cURL = dl->func.easy_init();
	if ( !dl->cURL ) 
	{
		Com_Printf( S_COLOR_RED "Com_DL_Begin: easy_init() failed\n" );
		Com_DL_Cleanup( dl );
		return qfalse;
	}

	{
		char *escapedName = dl->func.easy_escape( dl->cURL, localName, 0 );
		if ( !escapedName ) 
		{
			Com_Printf( S_COLOR_RED "Com_DL_Begin: easy_escape() failed\n" );
			Com_DL_Cleanup( dl );
			return qfalse;
		}

		Q_strncpyz( dl->URL, remoteURL, sizeof( dl->URL ) );

		if ( !Q_replace( "%1", escapedName, dl->URL, sizeof( dl->URL ) ) )
		{
			if ( dl->URL[strlen(dl->URL)] != '/' )
				Q_strcat( dl->URL, sizeof( dl->URL ), "/" );
			Q_strcat( dl->URL, sizeof( dl->URL ), escapedName );
			dl->headerCheck = qfalse;
		}
		else
		{
			dl->headerCheck = qtrue;
		}
		dl->func.free( escapedName );
	}

	Com_Printf( "URL: %s\n", dl->URL );

	if ( cl_dlDirectory->integer ) {
		Q_strncpyz( dl->gameDir, FS_GetBaseGameDir(), sizeof( dl->gameDir ) );
	} else {
		Q_strncpyz( dl->gameDir, FS_GetCurrentGameDir(), sizeof( dl->gameDir ) );
	}

	// try to extract game path from localName
	// dl->Name should contain only pak name without game dir and extension
	s = strrchr( localName, '/' );
	if ( s ) 
		Q_strncpyz( dl->Name, s+1, sizeof( dl->Name ) );
	else
		Q_strncpyz( dl->Name, localName, sizeof( dl->Name ) );

	FS_StripExt( dl->Name, ".pk3" );
	if ( !dl->Name[0] )
	{
		Com_Printf( S_COLOR_YELLOW " empty filename after extension strip.\n" );
		return qfalse;
	}

	Com_sprintf( dl->TempName, sizeof( dl->TempName ), 
		"%s%c%s.%08x.tmp", dl->gameDir, PATH_SEP, dl->Name, rand() | (rand() << 16) );

	if ( com_developer->integer )
		dl->func.easy_setopt( dl->cURL, CURLOPT_VERBOSE, 1 );

	dl->func.easy_setopt( dl->cURL, CURLOPT_URL, dl->URL );
	dl->func.easy_setopt( dl->cURL, CURLOPT_TRANSFERTEXT, 0 );
	//dl->func.easy_setopt( dl->cURL, CURLOPT_REFERER, "q3a://127.0.0.1" );
	dl->func.easy_setopt( dl->cURL, CURLOPT_REFERER, dl->URL );
	dl->func.easy_setopt( dl->cURL, CURLOPT_USERAGENT, Q3_VERSION );
	dl->func.easy_setopt( dl->cURL, CURLOPT_WRITEFUNCTION, Com_DL_CallbackWrite );
	dl->func.easy_setopt( dl->cURL, CURLOPT_WRITEDATA, dl );
	if ( dl->headerCheck ) 
	{
		dl->func.easy_setopt( dl->cURL, CURLOPT_HEADERFUNCTION, Com_DL_HeaderCallback );
		dl->func.easy_setopt( dl->cURL, CURLOPT_HEADERDATA, dl );
	}
	dl->func.easy_setopt( dl->cURL, CURLOPT_NOPROGRESS, 0 );
	dl->func.easy_setopt( dl->cURL, CURLOPT_PROGRESSFUNCTION, Com_DL_CallbackProgress );
	dl->func.easy_setopt( dl->cURL, CURLOPT_PROGRESSDATA, dl );
	dl->func.easy_setopt( dl->cURL, CURLOPT_FAILONERROR, 1 );
	dl->func.easy_setopt( dl->cURL, CURLOPT_FOLLOWLOCATION, 1 );
	dl->func.easy_setopt( dl->cURL, CURLOPT_MAXREDIRS, 5 );
	dl->func.easy_setopt( dl->cURL, CURLOPT_PROTOCOLS, ALLOWED_PROTOCOLS );

	dl->cURLM = dl->func.multi_init();

	if ( !dl->cURLM )
	{
		Com_DL_Cleanup( dl );
		Com_Printf( S_COLOR_RED "Com_DL_Begin: multi_init() failed\n" );
		return qfalse;
	}

	if ( dl->func.multi_add_handle( dl->cURLM, dl->cURL ) != CURLM_OK ) 
	{
		Com_DL_Cleanup( dl );
		Com_Printf( S_COLOR_RED "Com_DL_Begin: multi_add_handle() failed\n" );
		return qfalse;
	}

	dl->mapAutoDownload = autoDownload;

	if ( dl->mapAutoDownload )
	{
		Cvar_Set( "cl_downloadName", dl->Name );
		Cvar_Set( "cl_downloadSize", "0" );
		Cvar_Set( "cl_downloadCount", "0" );
		Cvar_Set( "cl_downloadTime", va( "%i", cls.realtime ) );
	}

	return qtrue;
}


qboolean Com_DL_Perform( download_t *dl )
{
	char name[ sizeof( dl->TempName ) ];
	CURLMcode res;
	CURLMsg *msg;
	long code;
	int c, n;
	int i;

	res = dl->func.multi_perform( dl->cURLM, &c );

	n = 128;

	i = 0;
	while( res == CURLM_CALL_MULTI_PERFORM && i < n )
	{
		res = dl->func.multi_perform( dl->cURLM, &c );
		i++;
	}
	if( res == CURLM_CALL_MULTI_PERFORM )
	{
		return qtrue;
	}

	msg = dl->func.multi_info_read( dl->cURLM, &c );
	if( msg == NULL )
	{
		return qtrue;
	}

	if ( dl->fHandle != FS_INVALID_HANDLE )
	{
		FS_FCloseFile( dl->fHandle );
		dl->fHandle = FS_INVALID_HANDLE;
	}

	if ( msg->msg == CURLMSG_DONE && msg->data.result == CURLE_OK )
	{
		qboolean autoDownload = dl->mapAutoDownload;

		Com_sprintf( name, sizeof( name ), "%s%c%s.pk3", dl->gameDir, PATH_SEP, dl->Name );

		if ( !FS_SV_FileExists( name ) )
		{
			FS_SV_Rename( dl->TempName, name );
		}
		else
		{
			n = FS_GetZipChecksum( name );
			Com_sprintf( name, sizeof( name ), "%s%c%s.%08x.pk3", dl->gameDir, PATH_SEP, dl->Name, n );

			if ( FS_SV_FileExists( name ) )
				FS_Remove( name );

			FS_SV_Rename( dl->TempName, name );
		}

		Com_DL_Cleanup( dl );
		FS_Reload(); //clc.downloadRestart = qtrue;
		Com_Printf( S_COLOR_GREEN "%s downloaded\n", name );
		if ( autoDownload )
		{
			if ( cls.state == CA_CONNECTED && !clc.demoplaying )
			{
				CL_AddReliableCommand( "donedl", qfalse ); // get new gamestate info from server
			} 
			else if ( clc.demoplaying )
			{
				// FIXME: there might be better solution than vid_restart
				cls.startCgame = qtrue;
				Cbuf_ExecuteText( EXEC_APPEND, "vid_restart\n" );
			}
		}
		return qfalse;
	}
	else
	{
		qboolean autoDownload = dl->mapAutoDownload;
		dl->func.easy_getinfo( msg->easy_handle, CURLINFO_RESPONSE_CODE, &code );
		Com_Printf( S_COLOR_RED "Download Error: %s Code: %ld\n",
			dl->func.easy_strerror( msg->data.result ), code );
		strcpy( name, dl->TempName );
		Com_DL_Cleanup( dl );
		FS_Remove( name );
		if ( autoDownload )
		{
			if ( cls.state == CA_CONNECTED )
			{
				Com_Error( ERR_DROP, "%s\n", "download error" );
			}
		}
	}

	return qtrue;
}

#endif /* USE_CURL */
