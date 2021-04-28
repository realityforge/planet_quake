/*
   Copyright (C) 1999-2007 id Software, Inc. and contributors.
   For a list of contributors, see the accompanying CONTRIBUTORS file.

   This file is part of GtkRadiant.

   GtkRadiant is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   GtkRadiant is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GtkRadiant; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

// cmdlib.c
// TTimo 09/30/2000
// from an intial copy of common/cmdlib.c
// stripped out the Com_Printf Com_Printf stuff

// SPoG 05/27/2001
// merging alpha branch into trunk
// replaced qprintf with Com_Printf

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"
#include "cmdlib.h"
#include "mathlib.h"
#include "inout.h"
#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#endif

#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
#include <unistd.h>
#endif

#ifdef NeXT
#include <libc.h>
#endif

#define BASEDIRNAME "quake"     // assumed to have a 2 or 3 following
#define PATHSEPERATOR   '/'

#ifdef SAFE_MALLOC
void *safe_malloc( size_t size ){
	void *p;

	p = malloc( size );
	if ( !p ) {
		Com_Error(ERR_DROP, "safe_malloc failed on allocation of %li bytes", size );
	}

	return p;
}

void *safe_malloc_info( size_t size, char* info ){
	void *p;

	p = malloc( size );
	if ( !p ) {
		Com_Error(ERR_DROP, "%s: safe_malloc failed on allocation of %li bytes", info, size );
	}

	return p;
}
#endif

// set these before calling CheckParm
int myargc;
char **myargv;

char com_token[1024];
qboolean com_eof;

qboolean archive;
char archivedir[1024];


/*
   ===================
   ExpandWildcards

   Mimic unix command line expansion
   ===================
 */
#define MAX_EX_ARGC 1024
int ex_argc;
char    *ex_argv[MAX_EX_ARGC];
#ifdef _WIN32
#include "io.h"
void ExpandWildcards( int *argc, char ***argv ){
	struct _finddata_t fileinfo;
	int handle;
	int i;
	char filename[1024];
	char filebase[1024];
	char    *path;

	ex_argc = 0;
	for ( i = 0 ; i < *argc ; i++ )
	{
		path = ( *argv )[i];
		if ( path[0] == '-'
			 || ( !strstr( path, "*" ) && !strstr( path, "?" ) ) ) {
			ex_argv[ex_argc++] = path;
			continue;
		}

		handle = _findfirst( path, &fileinfo );
		if ( handle == -1 ) {
			return;
		}

		ExtractFilePath( path, filebase );

		do
		{
			sprintf( filename, "%s%s", filebase, fileinfo.name );
			ex_argv[ex_argc++] = CopyString( filename );
		} while ( _findnext( handle, &fileinfo ) != -1 );

		_findclose( handle );
	}

	*argc = ex_argc;
	*argv = ex_argv;
}
#else
void ExpandWildcards( int *argc, char ***argv ){
}
#endif

void Q_getwd( char *out ){
	int i = 0;

#ifdef WIN32
	_getcwd( out, 256 );
	strcat( out, "\\" );
#else
	// Gef: Changed from getwd() to getcwd() to avoid potential buffer overflow
	if ( !getcwd( out, 256 ) ) {
		*out = 0;
	}
	strcat( out, "/" );
#endif
	while ( out[i] != 0 )
	{
		if ( out[i] == '\\' ) {
			out[i] = '/';
		}
		i++;
	}
}

/*

   qdir will hold the path up to the quake directory, including the slash

   f:\quake\
   /raid/quake/

   gamedir will hold qdir + the game directory (id1, id2, etc)

 */

char qdir[1024];
char gamedir[1024];
char writedir[1024];

void SetQdirFromPath( const char *path ){
	char temp[1024];
	const char  *c;
	const char *sep;
	int len, count;

	if ( !( path[0] == '/' || path[0] == '\\' || path[1] == ':' ) ) { // path is partial
		Q_getwd( temp );
		strcat( temp, path );
		path = temp;
	}

	// search for "quake2" in path

	len = strlen( BASEDIRNAME );
	for ( c = path + strlen( path ) - 1 ; c != path ; c-- )
	{
		int i;

		if ( !Q_strncasecmp( c, BASEDIRNAME, len ) ) {
			//
			//strncpy (qdir, path, c+len+2-path);
			// the +2 assumes a 2 or 3 following quake which is not the
			// case with a retail install
			// so we need to add up how much to the next separator
			sep = c + len;
			count = 1;
			while ( *sep && *sep != '/' && *sep != '\\' )
			{
				sep++;
				count++;
			}
			strncpy( qdir, path, c + len + count - path );
			Com_Printf( "qdir: %s\n", qdir );
			for ( i = 0; i < (int) strlen( qdir ); i++ )
			{
				if ( qdir[i] == '\\' ) {
					qdir[i] = '/';
				}
			}

			c += len + count;
			while ( *c )
			{
				if ( *c == '/' || *c == '\\' ) {
					strncpy( gamedir, path, c + 1 - path );

					for ( i = 0; i < (int) strlen( gamedir ); i++ )
					{
						if ( gamedir[i] == '\\' ) {
							gamedir[i] = '/';
						}
					}

					Com_Printf( "gamedir: %s\n", gamedir );

					if ( !writedir[0] ) {
						strcpy( writedir, gamedir );
					}
					else if ( writedir[strlen( writedir ) - 1] != '/' ) {
						writedir[strlen( writedir )] = '/';
						writedir[strlen( writedir ) + 1] = 0;
					}

					return;
				}
				c++;
			}
			Com_Error(ERR_DROP, "No gamedir in %s", path );
			return;
		}
	}
	Com_Error(ERR_DROP, "SetQdirFromPath: no '%s' in %s", BASEDIRNAME, path );
}

char *ExpandArg( const char *path ){
	static char full[1024];

	if ( path[0] != '/' && path[0] != '\\' && path[1] != ':' ) {
		Q_getwd( full );
		strcat( full, path );
	}
	else{
		strcpy( full, path );
	}
	return full;
}




/*
   ================
   I_FloatTime
   ================
 */
double I_FloatTime( void ){
	time_t t;

	time( &t );

	return t;
#if 0
// more precise, less portable
	struct timeval tp;
	struct timezone tzp;
	static int secbase;

	gettimeofday( &tp, &tzp );

	if ( !secbase ) {
		secbase = tp.tv_sec;
		return tp.tv_usec / 1000000.0;
	}

	return ( tp.tv_sec - secbase ) + tp.tv_usec / 1000000.0;
#endif
}


/*
   ============
   FileTime

   returns -1 if not present
   ============
 */
int FileTime( const char *path ){
	struct  stat buf;

	if ( stat( path,&buf ) == -1 ) {
		return -1;
	}

	return buf.st_mtime;
}


// NOTE TTimo when switching to Multithread DLL (Release/Debug) in the config
//   started getting warnings about that function, prolly a duplicate with the runtime function
//   maybe we still need to have it in linux builds
/*
   char *strupr (char *start)
   {
    char	*in;
    in = start;
    while (*in)
    {
   *in = toupper(*in);
        in++;
    }
    return start;
   }
 */

char *strlower( char *start ){
	char    *in;
	in = start;
	while ( *in )
	{
		*in = tolower( *in );
		in++;
	}
	return start;
}


/*
   =============================================================================

                        MISC FUNCTIONS

   =============================================================================
 */


/*
   =================
   CheckParm

   Checks for the given parameter in the program's command line arguments
   Returns the argument number (1 to argc-1) or 0 if not present
   =================
 */
int CheckParm( const char *check ){
	int i;

	for ( i = 1; i < myargc; i++ )
	{
		if ( !Q_stricmp( check, myargv[i] ) ) {
			return i;
		}
	}

	return 0;
}



/*
   ================
   Q_filelength
   ================
 */
int Q_filelength( FILE *f ){
	int pos;
	int end;

	pos = ftell( f );
	fseek( f, 0, SEEK_END );
	end = ftell( f );
	fseek( f, pos, SEEK_SET );

	return end;
}


FILE *SafeOpenWrite( const char *filename ){
	FILE    *f;

	f = fopen( filename, "wb" );

	if ( !f ) {
		Com_Error(ERR_DROP, "Error opening %s: %s",filename,strerror( errno ) );
	}

	return f;
}

FILE *SafeOpenRead( const char *filename ){
	FILE    *f;

	f = fopen( filename, "rb" );

	if ( !f ) {
		Com_Error(ERR_DROP, "Error opening %s: %s",filename,strerror( errno ) );
	}

	return f;
}


void SafeRead( FILE *f, void *buffer, int count ){
	if ( fread( buffer, 1, count, f ) != (size_t)count ) {
		Com_Error(ERR_DROP, "File read failure" );
	}
}


void SafeWrite( FILE *f, const void *buffer, int count ){
	if ( fwrite( buffer, 1, count, f ) != (size_t)count ) {
		Com_Error(ERR_DROP, "File write failure" );
	}
}


/*
   ==============
   FileExists
   ==============
 */
qboolean    FileExists( const char *filename ){
	FILE    *f;

	f = fopen( filename, "r" );
	if ( !f ) {
		return qfalse;
	}
	fclose( f );
	return qtrue;
}

/*
   ==============
   LoadFile
   ==============
 */
int    LoadFile( const char *filename, void **bufferptr ){
	FILE    *f;
	int length;
	void    *buffer;

	f = SafeOpenRead( filename );
	length = Q_filelength( f );
	buffer = safe_malloc( length + 1 );
	( (char *)buffer )[length] = 0;
	SafeRead( f, buffer, length );
	fclose( f );

	*bufferptr = buffer;
	return length;
}


/*
   ==============
   LoadFileBlock
   -
   rounds up memory allocation to 4K boundry
   -
   ==============
 */
#define MEM_BLOCKSIZE 4096
int    LoadFileBlock( const char *filename, void **bufferptr ){
	FILE    *f;
	int length, nBlock, nAllocSize;
	void    *buffer;

	f = SafeOpenRead( filename );
	length = Q_filelength( f );
	nAllocSize = length;
	nBlock = nAllocSize % MEM_BLOCKSIZE;
	if ( nBlock > 0 ) {
		nAllocSize += MEM_BLOCKSIZE - nBlock;
	}
	buffer = safe_malloc( nAllocSize + 1 );
	memset( buffer, 0, nAllocSize + 1 );
	SafeRead( f, buffer, length );
	fclose( f );

	*bufferptr = buffer;
	return length;
}


/*
   ==============
   TryLoadFile

   Allows failure
   ==============
 */
int    TryLoadFile( const char *filename, void **bufferptr ){
	FILE    *f;
	int length;
	void    *buffer;

	*bufferptr = NULL;

	f = fopen( filename, "rb" );
	if ( !f ) {
		return -1;
	}
	length = Q_filelength( f );
	buffer = safe_malloc( length + 1 );
	( (char *)buffer )[length] = 0;
	SafeRead( f, buffer, length );
	fclose( f );

	*bufferptr = buffer;
	return length;
}


/*
   ==============
   SaveFile
   ==============
 */
void    SaveFile( const char *filename, const void *buffer, int count ){
	FILE    *f;

	f = SafeOpenWrite( filename );
	SafeWrite( f, buffer, count );
	fclose( f );
}



void DefaultPath( char *path, const char *basepath ){
	char temp[128];

	if ( path[ 0 ] == '/' || path[ 0 ] == '\\' ) {
		return;                   // absolute path location
	}
	strcpy( temp,path );
	strcpy( path,basepath );
	strcat( path,temp );
}


void    StripFilename( char *path ){
	int length;

	length = strlen( path ) - 1;
	while ( length > 0 && path[length] != '/' && path[ length ] != '\\' )
		length--;
	path[length] = 0;
}

void    StripExtension( char *path ){
	int length;

	length = strlen( path ) - 1;
	while ( length > 0 && path[length] != '.' )
	{
		length--;
		if ( path[length] == '/' || path[ length ] == '\\' ) {
			return;     // no extension
		}
	}
	if ( length ) {
		path[length] = 0;
	}
}


/*
   ====================
   Extract file parts
   ====================
 */
// FIXME: should include the slash, otherwise
// backing to an empty path will be wrong when appending a slash
void ExtractFilePath( const char *path, char *dest ){
	const char    *src;

	src = path + strlen( path ) - 1;

//
// back up until a \ or the start
//
	while ( src != path && *( src - 1 ) != '\\' && *( src - 1 ) != '/' )
		src--;

	memcpy( dest, path, src - path );
	dest[src - path] = 0;
}

void ExtractFileBase( const char *path, char *dest ){
	const char    *src;

	src = path + strlen( path ) - 1;

//
// back up until a \ or the start
//
	while ( src != path && *( src - 1 ) != '/' && *( src - 1 ) != '\\' )
		src--;

	while ( *src && *src != '.' )
	{
		*dest++ = *src++;
	}
	*dest = 0;
}

void ExtractFileExtension( const char *path, char *dest ){
	const char    *src;

	src = path + strlen( path ) - 1;

//
// back up until a . or the start
//
	while ( src != path && *( src - 1 ) != '.' )
		src--;
	if ( src == path ) {
		*dest = 0;  // no extension
		return;
	}

	strcpy( dest,src );
}


/*
===============
ParseHex
===============
*/
static int	ParseHex( const char *text ) {
	int		value;
	int		c;

	value = 0;
	while ( ( c = *text++ ) != 0 ) {
		if ( c >= '0' && c <= '9' ) {
			value = value * 16 + c - '0';
			continue;
		}
		if ( c >= 'a' && c <= 'f' ) {
			value = value * 16 + 10 + c - 'a';
			continue;
		}
		if ( c >= 'A' && c <= 'F' ) {
			value = value * 16 + 10 + c - 'A';
			continue;
		}
	}

	return value;
}

int ParseNum( const char *str ){
	if ( str[0] == '$' ) {
		return ParseHex( str + 1 );
	}
	if ( str[0] == '0' && str[1] == 'x' ) {
		return ParseHex( str + 2 );
	}
	return atol( str );
}



/*
   ============================================================================

                    BYTE ORDER FUNCTIONS

   ============================================================================
 */

#ifdef _SGI_SOURCE
#define __BIG_ENDIAN__
#endif

#ifdef __BIG_ENDIAN__

short   BigShort( short l ){
	return l;
}


int    BigLong( int l ){
	return l;
}


#else


short   BigShort( short l ){
	byte b1,b2;

	b1 = l & 255;
	b2 = ( l >> 8 ) & 255;

	return ( b1 << 8 ) + b2;
}


int    BigLong( int l ){
	byte b1,b2,b3,b4;

	b1 = l & 255;
	b2 = ( l >> 8 ) & 255;
	b3 = ( l >> 16 ) & 255;
	b4 = ( l >> 24 ) & 255;

	return ( (int)b1 << 24 ) + ( (int)b2 << 16 ) + ( (int)b3 << 8 ) + b4;
}


#endif


//=============================================================================

/*
   ============
   CreatePath
   ============
 */
void    CreatePath( const char *path ){
	const char  *ofs;
	char c;
	char dir[1024];

#ifdef _WIN32
	int olddrive = -1;

	if ( path[1] == ':' ) {
		olddrive = _getdrive();
		_chdrive( toupper( path[0] ) - 'A' + 1 );
	}
#endif

	if ( path[1] == ':' ) {
		path += 2;
	}

	for ( ofs = path + 1 ; *ofs ; ofs++ )
	{
		c = *ofs;
		if ( c == '/' || c == '\\' ) { // create the directory
			memcpy( dir, path, ofs - path );
			dir[ ofs - path ] = 0;
			Q_mkdir( dir );
		}
	}

#ifdef _WIN32
	if ( olddrive != -1 ) {
		_chdrive( olddrive );
	}
#endif
}


/*
   ============
   QCopyFile

   Used to archive source files
   ============
 */
void QCopyFile( const char *from, const char *to ){
	void    *buffer;
	int length;

	length = LoadFile( from, &buffer );
	CreatePath( to );
	SaveFile( to, buffer, length );
	free( buffer );
}
