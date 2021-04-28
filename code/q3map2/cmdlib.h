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

//
// start of shared cmdlib stuff
//

#ifndef __CMDLIB__
#define __CMDLIB__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>
#include <limits.h>
#ifdef _WIN32
  #define PATH_MAX 260
#endif

#define MAX_OS_PATH     1024

#define SAFE_MALLOC
#ifdef SAFE_MALLOC
void *safe_malloc( size_t size );
void *safe_malloc_info( size_t size, char* info );
#else
#define safe_malloc( a ) malloc( a )
#endif /* SAFE_MALLOC */

// some easy portability crap
#ifdef _WIN32
  #include <direct.h>
  #define Q_mkdir( a ) _mkdir( a )
#else
  #include <sys/stat.h>
  #define Q_mkdir( a ) mkdir( a, 0755 )
#endif

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"

void    DefaultPath( char *path, const char *basepath );
void    StripFilename( char *path );
void    StripExtension( char *path );
void    ExtractFilePath( const char *path, char *dest );
void    ExtractFileName( const char *path, char *dest );
void    ExtractFileBase( const char *path, char *dest );
void    ExtractFileExtension( const char *path, char *dest );
double I_FloatTime( void );
/*!
   \brief create all directories leading to a file path. if you pass a directory, terminate it with a '/'
 */
void  CreateDirectoryPath( const char *path );

short   BigShort( short l );
int     BigLong( int l );
void *qmalloc( size_t size );
void* qblockmalloc( size_t nSize );

void ConvertDOSToUnixName( char *dst, const char *src );
#ifdef __cplusplus
char* StrDup( char* pStr );
#endif
char* StrDup( const char* pStr );
int    LoadFile( const char *filename, void **bufferptr );
FILE *SafeOpenWrite( const char *filename );
void SafeWrite( FILE *f, const void *buffer, int count );
FILE *SafeOpenRead( const char *filename );
void    SaveFile( const char *filename, const void *buffer, int count );

// TTimo started adding portability code:
// return true if spawning was successful, false otherwise
// on win32 we have a bCreateConsole flag to create a new console or run inside the current one
//boolean Q_Exec(const char* pCmd, boolean bCreateConsole);
// execute a system command:
//   cmd: the command to run
//   cmdline: the command line
// NOTE TTimo following are win32 specific:
//   execdir: the directory to execute in
//   bCreateConsole: spawn a new console or not
// return values;
//   if the spawn was fine
//   TODO TTimo add functionality to track the process until it dies

#endif
