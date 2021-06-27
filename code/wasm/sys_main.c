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
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <errno.h>
#include <libgen.h> // dirname

#include <dlfcn.h>

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"

#include "sys_local.h"

#ifndef DEDICATED
#include "../client/client.h"
#endif

#ifdef Q_EXPORT
#define Q_EXPORT __attribute__((visibility("default")))
#endif

unsigned sys_frame_time;

qboolean stdin_active = qfalse;
int      stdin_flags = 0;

// =======================================================================
// General routines
// =======================================================================

// bk001207 
#define MEM_THRESHOLD 96*1024*1024

/*
==================
Sys_LowPhysicalMemory()
==================
*/
qboolean Sys_LowPhysicalMemory( void )
{
	//MEMORYSTATUS stat;
	//GlobalMemoryStatus (&stat);
	//return (stat.dwTotalPhys <= MEM_THRESHOLD) ? qtrue : qfalse;
	return qfalse; // bk001207 - FIXME
}


void Sys_BeginProfiling( void )
{

}


/*
=================
Sys_In_Restart_f

Restart the input subsystem
=================
*/
#ifndef DEDICATED
void Sys_In_Restart_f( void )
{
	IN_Shutdown();
	IN_Init();
}
#endif


// =============================================================
// tty console routines
// NOTE: if the user is editing a line when something gets printed to the early console then it won't look good
//   so we provide tty_Clear and tty_Show to be called before and after a stdout or stderr output
// =============================================================


void CON_SigTStp( int signum )
{
	sigset_t mask;

	sigemptyset( &mask );
	sigaddset( &mask, SIGTSTP );
	sigprocmask( SIG_UNBLOCK, &mask, NULL );
	
	signal( SIGTSTP, SIG_DFL );
	
	kill( getpid(),  SIGTSTP );
}

void Sys_Exit( int code ) __attribute((noreturn));

void Sys_Quit( void )
{
#ifndef DEDICATED
	CL_Shutdown( "", qtrue );
#endif

	Sys_Exit( 0 );
}


void Sys_Init( void )
{

#ifndef DEDICATED
	Cmd_AddCommand( "in_restart", Sys_In_Restart_f );
	Cmd_SetDescription( "in_restart", "Restart all the input drivers, dinput, joystick, etc\nUsage: in_restart" );
#endif

	Cvar_Set( "arch", OS_STRING " " ARCH_STRING );

	//IN_Init();   // rcg08312005 moved into glimp.
}


void Sys_Error( const char *format, ... )
{
	va_list     argptr;
	char        text[1024];

	// change stdin to non blocking
	// NOTE TTimo not sure how well that goes with tty console mode
	if ( stdin_active )
	{
//		fcntl( STDIN_FILENO, F_SETFL, fcntl( STDIN_FILENO, F_GETFL, 0) & ~FNDELAY );
		fcntl( STDIN_FILENO, F_SETFL, stdin_flags );
	}

	va_start( argptr, format );
	Q_vsnprintf( text, sizeof( text ), format, argptr );
	va_end( argptr );

#ifndef DEDICATED
	CL_Shutdown( text, qtrue );
#endif

	fprintf( stderr, "Sys_Error: %s\n", text );

	Sys_Exit( 1 ); // bk010104 - use single exit point.
}


void floating_point_exception_handler( int whatever )
{
	signal( SIGFPE, floating_point_exception_handler );
}


void Sys_SendKeyEvents( void )
{

}


char *do_dlerror( void )
{
	return dlerror();
}


void Sys_UnloadDll( void *dllHandle ) {

	if ( !dllHandle )
	{
		Com_Printf( "Sys_UnloadDll(NULL)\n" );
		return;
	}

	dlclose( dllHandle );
	{
		const char* err; // rb010123 - now const
		err = dlerror();
		if ( err != NULL )
			Com_Printf ( "Sys_UnloadDLL failed on dlclose: \"%s\"!\n", err );
	}
}


static void* try_dlopen( const char* base, const char* gamedir, const char* fname )
{
	void* libHandle;
	char* fn;

	fn = FS_BuildOSPath( base, gamedir, fname );
	Com_Printf( "Sys_LoadDll(%s)... \n", fn );

	libHandle = dlopen( fn, RTLD_NOW );

	if( !libHandle ) 
	{
    	Com_Printf( "Sys_LoadDll(%s) failed:\n\"%s\"\n", fn, do_dlerror() );
		return NULL;
	}

	Com_Printf ( "Sys_LoadDll(%s): succeeded ...\n", fn );

	return libHandle;
}


void *Sys_LoadDll( const char *name, dllSyscall_t *entryPoint, dllSyscall_t systemcalls )
{
	void		*libHandle;
	dllEntry_t	dllEntry;
#ifdef DEBUG
	char		currpath[MAX_OSPATH];
#endif
	char		fname[MAX_OSPATH];
	const char	*basepath;
	const char	*homepath;
	const char	*gamedir;
	const char	*err = NULL;

	assert( name ); // let's have some paranoia

	snprintf( fname, sizeof( fname ), "%s" ARCH_STRING DLL_EXT, name );

	// TODO: use fs_searchpaths from files.c
	basepath = Cvar_VariableString( "fs_basepath" );
	homepath = Cvar_VariableString( "fs_homepath" );
	gamedir = Cvar_VariableString( "fs_game" );
	if ( !*gamedir ) {
		gamedir = Cvar_VariableString( "fs_basegame" );
	}

#ifdef DEBUG
	if ( getcwd( currpath, sizeof( currpath ) ) )
		libHandle = try_dlopen( currpath, gamedir, fname );
	else
#endif
	libHandle = NULL;

	if ( !libHandle && homepath && homepath[0] )
		libHandle = try_dlopen( homepath, gamedir, fname );

	if( !libHandle && basepath && basepath[0] )
		libHandle = try_dlopen( basepath, gamedir, fname );

	if ( !libHandle ) 
	{
		Com_Printf ( "Sys_LoadDll(%s) failed dlopen() completely!\n", name );
		return NULL;
	}

	dllEntry = dlsym( libHandle, "dllEntry" );
	*entryPoint = dlsym( libHandle, "vmMain" );

	if ( !*entryPoint || !dllEntry )
	{
		err = do_dlerror();
#ifndef NDEBUG // bk001206 - in debug abort on failure
		Com_Error ( ERR_FATAL, "Sys_LoadDll(%s) failed dlsym(vmMain):\n\"%s\" !\n", name, err );
#else
		Com_Printf ( "Sys_LoadDll(%s) failed dlsym(vmMain):\n\"%s\" !\n", name, err );
#endif
		dlclose( libHandle );
		err = do_dlerror();
		if ( err != NULL ) 
		{
			Com_Printf( "Sys_LoadDll(%s) failed dlcose:\n\"%s\"\n", name, err );
		}
		return NULL;
	}

	Com_Printf( "Sys_LoadDll(%s) found **vmMain** at %p\n", name, *entryPoint );
	dllEntry( systemcalls );
	Com_Printf( "Sys_LoadDll(%s) succeeded!\n", name );

	return libHandle;
}


static struct Q3ToAnsiColorTable_s
{
	const char Q3color;
	const char *ANSIcolor;
} tty_colorTable[ ] =
{
	{ COLOR_BLACK,    "30" },
	{ COLOR_RED,      "31" },
	{ COLOR_GREEN,    "32" },
	{ COLOR_YELLOW,   "33" },
	{ COLOR_BLUE,     "34" },
	{ COLOR_CYAN,     "36" },
	{ COLOR_MAGENTA,  "35" },
	{ COLOR_WHITE,    "0" }
};


void Sys_ANSIColorify( const char *msg, char *buffer, int bufferSize )
{
  int   msgLength;
  int   i, j;
  const char *escapeCode;
  char  tempBuffer[ 7 ];

  if( !msg || !buffer )
    return;

  msgLength = strlen( msg );
  i = 0;
  buffer[ 0 ] = '\0';

  while( i < msgLength )
  {
    if( msg[ i ] == '\n' )
    {
      Com_sprintf( tempBuffer, 7, "%c[0m\n", 0x1B );
      strncat( buffer, tempBuffer, bufferSize - 1);
      i++;
    }
    else if( msg[ i ] == Q_COLOR_ESCAPE )
    {
      i++;

      if( i < msgLength )
      {
        escapeCode = NULL;
        for( j = 0; j < ARRAY_LEN( tty_colorTable ); j++ )
        {
          if( msg[ i ] == tty_colorTable[ j ].Q3color )
          {
            escapeCode = tty_colorTable[ j ].ANSIcolor;
            break;
          }
        }

        if( escapeCode )
        {
          Com_sprintf( tempBuffer, 7, "%c[%sm", 0x1B, escapeCode );
          strncat( buffer, tempBuffer, bufferSize - 1);
        }

        i++;
      }
    }
    else
    {
      Com_sprintf( tempBuffer, 7, "%c", msg[ i++ ] );
      strncat( buffer, tempBuffer, bufferSize - 1);
    }
  }
}


void Sys_Print( const char *msg )
{
  fputs( msg, stderr );
}


void Sys_ConfigureFPU( void )  // bk001213 - divide by zero
{
#ifdef __linux__
#ifdef __i386
#ifndef NDEBUG
	// bk0101022 - enable FPE's in debug mode
	static int fpu_word = _FPU_DEFAULT & ~(_FPU_MASK_ZM | _FPU_MASK_IM);
	int current = 0;
	_FPU_GETCW( current );
	if ( current!=fpu_word)
	{
#if 0
		Com_Printf("FPU Control 0x%x (was 0x%x)\n", fpu_word, current );
		_FPU_SETCW( fpu_word );
		_FPU_GETCW( current );
		assert(fpu_word==current);
#endif
	}
#else // NDEBUG
	static int fpu_word = _FPU_DEFAULT;
	_FPU_SETCW( fpu_word );
#endif // NDEBUG
#endif // __i386 
#endif // __linux
}


void Sys_PrintBinVersion( const char* name )
{
	const char *date = __DATE__;
	const char *time = __TIME__;
	const char *sep = "==============================================================";

	fprintf( stdout, "\n\n%s\n", sep );
#ifdef DEDICATED
	fprintf( stdout, "Linux Quake3 Dedicated Server [%s %s]\n", date, time );
#else
	fprintf( stdout, "Linux Quake3 Full Executable  [%s %s]\n", date, time );
#endif
	fprintf( stdout, " local install: %s\n", name );
	fprintf( stdout, "%s\n\n", sep );
}


const char *Sys_BinName( const char *arg0 )
{
	static char   dst[ PATH_MAX ];

	Q_strncpyz( dst, arg0, PATH_MAX );

	return dst;
}


int Sys_ParseArgs( int argc, char* argv[] )
{
	if ( argc == 2 )
	{
		if ( ( !strcmp( argv[1], "--version" ) ) || ( !strcmp( argv[1], "-v" ) ) )
		{
			Sys_PrintBinVersion( Sys_BinName( argv[0] ) );
			return 1;
		}
	}

	return 0;
}

/*
=================
Sys_Frame
=================
*/
void Sys_Frame( void ) {
	IN_Frame();
	Com_Frame( CL_NoDelay() );
}

Q_EXPORT int main( int argc, char* argv[] )
{
	char con_title[ MAX_CVAR_VALUE_STRING ];
	int xpos, ypos;
	//qboolean useXYpos;
	char  *cmdline;
	int   len, i;
	
	// bullshit because onRuntimeInitialized does execute consistently
	//   held up by some sort of WarningHandler race condition
	argc = Sys_CmdArgsC();
	argv = Sys_CmdArgs();
		
	if ( Sys_ParseArgs( argc, argv ) ) // added this for support
		return 0;

	// merge the command line, this is kinda silly
	for ( len = 1, i = 1; i < argc; i++ )
		len += strlen( argv[i] ) + 1;

	cmdline = malloc( len );
	*cmdline = '\0';
	for ( i = 1; i < argc; i++ )
	{
		if ( i > 1 )
			strcat( cmdline, " " );
		strcat( cmdline, argv[i] );
	}

	/*useXYpos = */ Com_EarlyParseCmdLine( cmdline, con_title, sizeof( con_title ), &xpos, &ypos );

	// get the initial time base
	Sys_Milliseconds();

	Com_Init( cmdline );
	NET_Init();

	// JavaScript console doesn't report input
  Cvar_Set( "ttycon", "0" );

	//emscripten_set_main_loop(Sys_Frame, 160, 0);
	return 0;
}


qboolean Sys_GetFileStats( const char *filename, fileOffset_t *size, fileTime_t *mtime, fileTime_t *ctime ) {
	struct stat s;

	if ( stat( filename, &s ) == 0 ) {
		*size = (fileOffset_t)s.st_size;
		*mtime = (fileTime_t)s.st_mtime;
		*ctime = (fileTime_t)s.st_ctime;
		return qtrue;
	} else {
		*size = 0;
		*mtime = *ctime = 0;
		return qfalse;
	}
}


void Sys_FreeFileList( char **list ) {
	int		i;

	if ( !list ) {
		return;
	}

	for ( i = 0 ; list[i] ; i++ ) {
		Z_Free( list[i] );
	}

	Z_Free( list );
}

void Sys_ShowConsole( int visLevel, qboolean quitOnClose ) { }

char *Sys_ConsoleInput( void ) { return NULL; }

void Sys_Mkdir( const char *path ) { mkdir( path, 0750 ); }

const char *Sys_DefaultBasePath( void ) { return "/base"; }

const char *Sys_Pwd( void ) { return "/base"; }

qboolean Sys_ResetReadOnlyAttribute( const char *ospath ) { return qfalse; }

const char *Sys_DefaultHomePath( void ) { return "/base/home"; }


#define VA_ARGS(numargs, pointer) \
intptr_t	args[numargs]; \
va_list	ap; \
va_start( ap, pointer ); \
for (int i = 0; i < ARRAY_LEN( args ); i++ ) \
  args[ i ] = va_arg( ap, intptr_t ); \
va_end( ap );


__attribute__((nothrow))
EM_JS(int, ASM_const_int, ( const char* code, const char* arg_sigs, intptr_t *args ), 
{ return asm_const_int(code, arg_sigs, args) });
int asm_const_int(const char* code, const char* arg_sigs, ...)
{
  VA_ARGS(14, arg_sigs);
  return ASM_const_int(code, arg_sigs, args); 
}

EM_JS(void, SYS_SocksConnect, ( void ), 
{ Sys_SocksConnect() });
void Sys_SocksConnect( void )
{ SYS_SocksConnect(); }

EM_JS(void, SYS_NET_MulticastLocal, ( int sock, int length, const int *data ), 
{ return SYS_NET_MulticastLocal(sock, length, data) });
void Sys_NET_MulticastLocal( int sock, int length, const int *data )
{ return SYS_NET_MulticastLocal(sock, length, data); }

EM_JS(void *, SYS_LoadFunction, ( void *handle, const char *name ), 
{ return Sys_LoadFunction(handle, name) });
void *Sys_LoadFunction( void *handle, const char *name )
{ return SYS_LoadFunction(handle, name); }

EM_JS(void, SYS_UnloadLibrary, ( void *handle ), 
{ return Sys_UnloadLibrary(handle) });
void Sys_UnloadLibrary( void *handle )
{ return SYS_UnloadLibrary(handle); }

EM_JS(void *, SYS_LoadLibrary, ( const char *name ), 
{ return Sys_LoadLibrary(name) });
void *Sys_LoadLibrary( const char *name )
{ return SYS_LoadLibrary(name); }

EM_JS(char **, SYS_CmdArgs, ( void ), 
{ return Sys_Main_CmdArgs() });
char **Sys_CmdArgs( void )
{ return SYS_CmdArgs(); }

EM_JS(int, SYS_CmdArgsC, ( void ), 
{ return Sys_Main_CmdArgsC() });
int Sys_CmdArgsC( void )
{ return SYS_CmdArgsC(); }

EM_JS(void, SYS_FS_Offline, ( void ), 
{ Sys_FS_Offline() });
void Sys_FS_Offline( void )
{ SYS_FS_Offline(); }

EM_JS(void, SYS_Debug, ( void ), 
{ return Sys_Debug() });
void Sys_Debug( void )
{ return SYS_Debug(); }

EM_JS(char *, SYS_GetClipboardData, ( void ), 
{ return Sys_GetClipboardData() });
char *Sys_GetClipboardData( void )
{ return SYS_GetClipboardData(); }

EM_JS(void, SYS_BeginDownload, ( void ), 
{ Sys_BeginDownload() });
void Sys_BeginDownload( void )
{ SYS_BeginDownload(); }

EM_JS(void, SYS_DownloadLocalFile, ( char *fileName ), 
{ return Sys_Main_DownloadLocalFile(fileName) });
void Sys_DownloadLocalFile( char *fileName )
{ return SYS_DownloadLocalFile(fileName); }

EM_JS(void, SYS_Main_PlatformExit, ( int code ), 
{ Sys_PlatformExit(code) });
void Sys_Exit( int code )
{ SYS_Main_PlatformExit(code); exit(code); }

EM_JS(qboolean, SYS_Main_RandomBytes, ( byte *string, int len ), 
{ return Sys_RandomBytes(string, len) });
qboolean Sys_RandomBytes( byte *string, int len )
{ return SYS_Main_RandomBytes(string, len); }

EM_JS(void, SYS_Main_SetStatus, ( const char *format, intptr_t *args ), 
{ Sys_SetStatus.apply(null, [format].concat(args)) });
void QDECL Sys_SetStatus( const char *format, ... )
{
  VA_ARGS(14, format);
  SYS_Main_SetStatus(format, args); 
}

EM_JS(char **, SYS_FS_List, ( const char *directory, const char *extension, const char *filter, int *numfiles, qboolean wantsubs ), 
{ return Sys_ListFiles(directory, extension, filter, numfiles, wantsubs) });
char **Sys_ListFiles( const char *directory, const char *extension, const char *filter, int *numfiles, qboolean wantsubs )
{ return SYS_FS_List(directory, extension, filter, numfiles, wantsubs); }

EM_JS(FILE *, SYS_FS_FOpen, (const char *ospath, const char *mode), 
{ return Sys_FOpen(ospath, mode) });
FILE *Sys_FOpen( const char *ospath, const char *mode )
{ return SYS_FS_FOpen(ospath, mode); }

EM_JS(void, SYS_FS_Shutdown, (void), { Sys_FS_Shutdown() });
void Sys_FS_Shutdown(void) { return SYS_FS_Shutdown(); }

EM_JS(void, SYS_FS_Startup, (void), { Sys_FS_Startup() });
void Sys_FS_Startup(void) { return SYS_FS_Startup(); }

EM_JS(void, SYS_SetClipboardData, (void *field), { Sys_Input_SetClipboardData(field) });
void Sys_SetClipboardData(void *field) { return SYS_SetClipboardData(field); }

EM_JS(int, SYS_Milliseconds, (void), { return Sys_Main_Milliseconds() });
int Sys_Milliseconds(void) { return SYS_Milliseconds(); }