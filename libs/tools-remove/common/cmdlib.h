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

// cmdlib.h

#ifndef __Q_SHARED_H
#include "../../qcommon/q_shared.h"
#endif
#include "../../qcommon/qcommon.h"
#include "../../qcommon/qfiles.h"


#ifdef __Q_SHARED_H
#ifdef _MSC_VER
#define Error(x, ...) Com_Error(ERR_DROP, x, __VA_ARGS__)
#else
#define Error(x, args...) Com_Error( ERR_DROP, x, ##args )
#endif
#ifdef _MSC_VER
#define Error(x, ...) Com_Error(ERR_DROP, __VA_ARGS__)
#else
#define FPrintf(x, y, args...) Com_Printf( y, ##args )
#endif
#define Sys_FPrintf FPrintf
#define Sys_Printf Com_Printf
#define copystring CopyString
#define StripExtension(x) COM_StripExtension(x, x, sizeof(x))
#define ExtractFileExtension( x, y ) const char *extpos = COM_GetExtension(x); \
	strcpy(y, extpos);
#define VectorNormalize VectorNormalize2
#define DefaultExtension(x, y) COM_DefaultExtension(x, sizeof(x), y)
/*
#ifdef ZONE_DEBUG
#define safe_malloc(size) Z_MallocDebug(size, #size, __FILE__, __LINE__)
#define safe_malloc_info(size, y) Z_MallocDebug(size, #size, __FILE__, __LINE__)
#else
#define safe_malloc(x) Z_Malloc(x)
#define safe_malloc_info(x, y) Z_Malloc(x)
#endif
#define free Z_Free
*/
#define safe_malloc malloc
#define safe_malloc_info(x, y) malloc(x)

#define _NO_GLIB
#define MAX_OS_PATH     4096
#define Random random
#define SafeOpenRead(x) Sys_FOpen(x, "rb")
#define SafeOpenWrite(x) Sys_FOpen(x, "wb")
#define SafeWrite(f, buffer, count) fwrite( buffer, 1, count, f )
#define SafeRead(f, buffer, count) fread( buffer, 1, count, f )
#define LoadFile FS_ReadFile
#define SaveFile FS_WriteFile
#define ExpandArg 
#define ExpandPath
#define FileExists(x) FS_FileExists(x)
#define Q_mkdir( a ) Sys_Mkdir( a )
#endif

#ifndef __Q_SHARED_H
#ifndef __CMDLIB__
#define __CMDLIB__

#include "bytebool.h"

#ifdef _WIN32
#pragma warning(disable : 4244)     // MIPS
#pragma warning(disable : 4136)     // X86
#pragma warning(disable : 4051)     // ALPHA

#pragma warning(disable : 4018)     // signed/unsigned mismatch
#pragma warning(disable : 4305)     // truncate from double to float

#pragma check_stack(off)

#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>

#ifdef _WIN32

#pragma intrinsic( memset, memcpy )

#endif


#define MAX_OS_PATH     4096
#define MEM_BLOCKSIZE 4096

#define SAFE_MALLOC
#ifdef SAFE_MALLOC
void *safe_malloc( size_t size );
void *safe_malloc_info( size_t size, char* info );
#else
#define safe_malloc( a ) malloc( a )
#endif /* SAFE_MALLOC */

// set these before calling CheckParm
extern int myargc;
extern char **myargv;

char *strlower( char *in );
int Q_strncasecmp( const char *s1, const char *s2, int n );
int Q_stricmp( const char *s1, const char *s2 );
void Q_getwd( char *out );

int Q_filelength( FILE *f );
int FileTime( const char *path );

void    Q_mkdir( const char *path );

extern char qdir[1024];
extern char gamedir[1024];
extern char writedir[1024];
extern char    *moddirparam;
void SetQdirFromPath( const char *path );
char *ExpandArg( const char *path );    // from cmd line
char *ExpandPath( const char *path );   // from scripts
void ExpandWildcards( int *argc, char ***argv );


double I_FloatTime( void );

void    Error( const char *error, ... );
int     CheckParm( const char *check );

FILE    *SafeOpenWrite( const char *filename );
FILE    *SafeOpenRead( const char *filename );
void    SafeRead( FILE *f, void *buffer, int count );
void    SafeWrite( FILE *f, const void *buffer, int count );

int     LoadFile( const char *filename, void **bufferptr );
int   LoadFileBlock( const char *filename, void **bufferptr );
int     TryLoadFile( const char *filename, void **bufferptr );
void    SaveFile( const char *filename, const void *buffer, int count );
qboolean    FileExists( const char *filename );

void    DefaultExtension( char *path, const char *extension );
void    DefaultPath( char *path, const char *basepath );
void    StripFilename( char *path );
void    StripExtension( char *path );

void    ExtractFilePath( const char *path, char *dest );
void    ExtractFileBase( const char *path, char *dest );
void    ExtractFileExtension( const char *path, char *dest );

int     ParseNum( const char *str );

short   BigShort( short l );
short   LittleShort( short l );
int     BigLong( int l );
int     LittleLong( int l );
float   BigFloat( float l );
float   LittleFloat( float l );


char *COM_Parse( char *data );

extern char com_token[1024];
extern qboolean com_eof;

char *copystring( const char *s );


void CRC_Init( unsigned short *crcvalue );
void CRC_ProcessByte( unsigned short *crcvalue, byte data );
unsigned short CRC_Value( unsigned short crcvalue );

void    CreatePath( const char *path );
void    QCopyFile( const char *from, const char *to );

extern qboolean archive;
extern char archivedir[1024];

// sleep for the given amount of milliseconds
void Sys_Sleep( int n );

// for compression routines
typedef struct
{
	void    *data;
	int count, width, height;
} cblock_t;


#endif
#endif
