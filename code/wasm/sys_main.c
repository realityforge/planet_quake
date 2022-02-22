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
#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"

#include "sys_local.h"

#ifndef DEDICATED
#include "../client/client.h"
#endif

#ifndef Q_EXPORT
#define Q_EXPORT __attribute__((visibility("default")))
#endif

extern void IN_Shutdown( void );
extern void IN_Init( void );
extern void IN_Frame( void );


// =======================================================================
// General routines
// =======================================================================

// some things are easier to leave here instead of in javscript?

qboolean Sys_LowPhysicalMemory( void ) { return qfalse; }
void Sys_BeginProfiling( void ) { }
const char *Sys_SteamPath( void ) { return ""; }
void Sys_SendKeyEvents( void ) { } // moved to push in sys_in.js
void Sys_ShowConsole( int visLevel, qboolean quitOnClose ) { }
char *Sys_ConsoleInput( void ) { return NULL; }
const char *Sys_DefaultBasePath( void ) { return "/base"; }
qboolean Sys_ResetReadOnlyAttribute( const char *ospath ) { return qfalse; }
const char *Sys_DefaultHomePath( void ) { return "/base/home"; }



/*
=================
Sys_FreeFileList
=================
*/
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


__attribute__((import_module("SYS"), import_name("Sys_Exit")))
void Sys_Exit( int code );

__attribute__((import_module("SYS"), import_name("dlopen")))
void *try_dlopen( const char* base, const char* gamedir, const char* fname );

__attribute__((import_module("SYS"), import_name("dlerror")))
char *dlerror( void );

__attribute__((import_module("SYS"), import_name("dlsym")))
void *dlsym( void *handle, char *symbol );

__attribute__((import_module("SYS"), import_name("dlclose")))
void *dlclose( void *handle );

const char *Sys_Pwd( void ) { return "/base"; }

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
}



#undef stdout
#define stdout 0

void Sys_PrintBinVersion( const char* name )
{
	const char *date = __DATE__;
	const char *time = __TIME__;
	const char *sep = "==============================================================";

	fprintf( stdout, "\n\n%s\n", sep );
#ifdef DEDICATED
	fprintf( stdout, "WAST Quake3 Dedicated Server [%s %s]\n", date, time );
#else
	fprintf( stdout, "WAST Quake3 Full Executable  [%s %s]\n", date, time );
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
#ifdef __WASM__
Q_EXPORT
#endif
void Sys_Frame( void ) {
	IN_Frame();
	Com_Frame( CL_NoDelay() );
}


Q_EXPORT int RunGame( int argc, char* argv[] )
{
	char con_title[ MAX_CVAR_VALUE_STRING ];
	int xpos, ypos;
	//qboolean useXYpos;
	char  *cmdline;
	int   len, i;

	if(!argc) {
		Sys_Error("No startup options specified.");
		return 1;
	}

	if ( Sys_ParseArgs( argc, argv ) ) // added this for support
		return 0;

	// merge the command line, this is kinda silly
	for ( len = 1, i = 1; i < argc; i++ ) {
		len += strlen( argv[i] ) + 1;
	}

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

	Sys_SetStatus("Starting up...\n");

	Com_Init( cmdline );
	NET_Init();

	// JavaScript console doesn't report input
  Cvar_Set( "ttycon", "0" );

  //Browser.requestAnimationFrame(_Sys_Frame);
  //var timeUntilNextTick = Math.max(0, Browser.mainLoop.tickStartTime + value - Sys_Milliseconds)|0;

	//emscripten_set_main_loop(Sys_Frame, 160, 0);
	return 0;
}


/*
#define VA_ARGS(numargs, pointer) \
intptr_t	args[numargs]; \
va_list	ap; \
va_start( ap, pointer ); \
for (int i = 0; i < ARRAY_LEN( args ); i++ ) \
  args[ i ] = va_arg( ap, intptr_t ); \
va_end( ap );
*/

