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
#include "../client/snd_local.h"
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


// =============================================================
// tty console routines
// NOTE: if the user is editing a line when something gets printed to the early console then it won't look good
//   so we provide tty_Clear and tty_Show to be called before and after a stdout or stderr output
// =============================================================
void Sys_Exit( int code ) __attribute((noreturn));
void *try_dlopen( const char* base, const char* gamedir, const char* fname );
char *dlerror( void );
void *dlsym( void *handle, char *symbol );
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
	// TODO: 
	//Cmd_AddCommand( "in_restart", Sys_In_Restart_f );
	//Cmd_SetDescription( "in_restart", "Restart all the input drivers, dinput, joystick, etc\nUsage: in_restart" );
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


typedef enum {
RSERR_OK,
RSERR_INVALID_FULLSCREEN,
RSERR_INVALID_MODE,
RSERR_FATAL_ERROR,
RSERR_UNKNOWN
} rserr_t;

cvar_t *r_stereoEnabled;
cvar_t *in_nograb;

void SDL_GL_SwapWindow(void *window) {
  // this probably prevents that flashing when changing mods
}



int SDL_GL_SetSwapInterval(int interval) {
  // set values for request animation frame?
  return 0;
}


/*
===============
GLimp_Shutdown
===============
*/




/*
===============
GLimp_LogComment
===============
*/
void GLimp_LogComment( char *comment )
{
}

extern void  GL_GetDrawableSize(int *w, int *h);
rserr_t GLimp_StartDriverAndSetMode(int mode, const char *modeFS, qboolean fullscreen, qboolean fallback);
void GLimp_Shutdown( qboolean unloadDLL );


/*
===============
GLimp_Init

This routine is responsible for initializing the OS specific portions
of OpenGL
===============
*/
void GLimp_Init( glconfig_t *config )
{
  rserr_t err;

#if 0
  InitSig();
#endif

  Com_DPrintf( "GLimp_Init()\n" );

  in_nograb = Cvar_Get( "in_nograb", "0", CVAR_ARCHIVE );

  r_allowSoftwareGL = Cvar_Get( "r_allowSoftwareGL", "0", CVAR_LATCH );

  r_swapInterval = Cvar_Get( "r_swapInterval", "0", CVAR_ARCHIVE | CVAR_LATCH );
  r_stereoEnabled = Cvar_Get( "r_stereoEnabled", "0", CVAR_ARCHIVE | CVAR_LATCH );

  // Create the window and set up the context
  err = GLimp_StartDriverAndSetMode( r_mode->integer, r_modeFullscreen->string, r_fullscreen->integer, qfalse );
  if ( err != RSERR_OK )
  {
    if ( err == RSERR_FATAL_ERROR )
    {
      Com_Error( ERR_FATAL, "GLimp_Init() - could not load OpenGL subsystem" );
      return;
    }

    // instead of changing r_mode, which is just the default resolution,
    //   we try a different opengl version
    Com_Printf( "Setting \\r_mode %d failed, falling back on \\r_mode %d\n", r_mode->integer, 3 );
    if ( GLimp_StartDriverAndSetMode( r_mode->integer, "", r_fullscreen->integer, qtrue ) != RSERR_OK )
    {
      // Nothing worked, give up
      Com_Error( ERR_FATAL, "GLimp_Init() - could not load OpenGL subsystem" );
      return;
    }
  }

  GL_GetDrawableSize( &config->vidWidth, &config->vidHeight );

  // These values force the UI to disable driver selection
  config->driverType = GLDRV_ICD;
  config->hardwareType = GLHW_GENERIC;

  Key_ClearStates();

  // This depends on SDL_INIT_VIDEO, hence having it here
  IN_Init();
}


/*
===============
GLimp_EndFrame

Responsible for doing a swapbuffers
===============
*/
void GLimp_EndFrame( void )
{
  // don't flip if drawing to front buffer
  if ( Q_stricmp( cl_drawBuffer->string, "GL_FRONT" ) != 0 )
  {
    //SDL_GL_SwapWindow( SDL_window );
  }
}


void GLimp_InitGamma( glconfig_t *config )
{
}


void GLimp_SetGamma( unsigned char red[256], unsigned char green[256], unsigned char blue[256] )
{
}


/*
===============
Sys_SetClipboardBitmap
===============
*/
void Sys_SetClipboardBitmap( const byte *bitmap, int length )
{

}



static cvar_t *in_keyboardDebug;

static cvar_t *in_mouse;

static cvar_t *in_joystick;
static cvar_t *in_joystickThreshold;
#ifdef USE_JOYSTICK
static cvar_t *in_joystickNo;
static cvar_t *in_joystickUseAnalog;

static cvar_t *j_pitch;
static cvar_t *j_yaw;
static cvar_t *j_forward;
static cvar_t *j_side;
static cvar_t *j_up;
static cvar_t *j_pitch_axis;
static cvar_t *j_yaw_axis;
static cvar_t *j_forward_axis;
static cvar_t *j_side_axis;
static cvar_t *j_up_axis;
#endif

static cvar_t *cl_consoleKeys;


#if 0

/**
 *  \brief Touch finger event structure (event.tfinger.*)
 */
typedef struct SDL_TouchFingerEvent
{
    uint32_t type;        /**< ::SDL_FINGERMOTION or ::SDL_FINGERDOWN or ::SDL_FINGERUP */
    uint32_t timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
    uint32_t touchId;       /**< The touch device id */
    uint32_t fingerId;
    float x;            /**< Normalized in the range 0...1 */
    float y;            /**< Normalized in the range 0...1 */
    float dx;           /**< Normalized in the range -1...1 */
    float dy;           /**< Normalized in the range -1...1 */
    float pressure;     /**< Normalized in the range 0...1 */
} SDL_TouchFingerEvent;


void IN_PushTouchFinger(SDL_TouchFingerEvent e)
{
	if(e.type == SDL_FINGERMOTION) {
		float ratio = (float)cls.glconfig.vidWidth / (float)cls.glconfig.vidHeight;
		touchhats[e.fingerId][0] = (e.x * ratio) * 50;
		touchhats[e.fingerId][1] = e.y * 50;
	}
	else if (e.type == SDL_FINGERDOWN) {
		if((Key_GetCatcher( ) & KEYCATCH_UI) && e.fingerId == 3) {
			Com_QueueEvent( in_eventTime, SE_MOUSE_ABS, e.x * cls.glconfig.vidWidth, e.y * cls.glconfig.vidHeight, 0, NULL );
		}
		Com_QueueEvent( in_eventTime+1, SE_FINGER_DOWN, K_MOUSE1, e.fingerId, 0, NULL );
	}
	else if(e.type == SDL_FINGERUP) {
		//Com_QueueEvent( in_eventTime+1, SE_KEY, K_MOUSE1, qfalse, 0, NULL );
		Com_QueueEvent( in_eventTime+1, SE_FINGER_UP, K_MOUSE1, e.fingerId, 0, NULL );
		touchhats[e.fingerId][0] = 0;
		touchhats[e.fingerId][1] = 0;
	}
}

/**
 *  \brief An event used to request a file open by the system (event.drop.*)
 *         This event is enabled by default, you can disable it with SDL_EventState().
 *  \note If this event is enabled, you must free the filename in the event.
 */
typedef struct SDL_DropEvent
{
    uint32_t type;        /**< ::SDL_DROPBEGIN or ::SDL_DROPFILE or ::SDL_DROPTEXT or ::SDL_DROPCOMPLETE */
    uint32_t timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
    char *file;         /**< The file name, which should be freed with SDL_free(), is NULL on begin/complete */
    uint32_t windowID;    /**< The window that was dropped on, if any */
} SDL_DropEvent;

void IN_PushDropEvent(SDL_DropEvent e)
{
	char file[MAX_OSPATH];
	if(e.type == SDL_DROPBEGIN) {
		// TODO: show the full console
		if(!(Key_GetCatcher() & KEYCATCH_CONSOLE))
			Key_SetCatcher( Key_GetCatcher() | KEYCATCH_CONSOLE );

		Com_Printf("Dropping files:\n");
	}
	if(e.type == SDL_DROPFILE) {
		// show the contents of the dropped file and offer to load something
		Com_Printf("Opening file: %s\n", e.file);
		Q_strncpyz(file, e.file, MAX_OSPATH);
	}
	if(e.type == SDL_DROPCOMPLETE) {
		Con_ClearNotify();
		memcpy(&g_consoleField.buffer, "", sizeof(g_consoleField.buffer));
		Field_AutoComplete( &g_consoleField );
		g_consoleField.cursor = strlen(g_consoleField.buffer);
	}
}

/*
===============
IN_Frame
===============
*/
void IN_Frame( void ) {
  // TODO
	//for(i = 1; i < 4; i++) {
		/*
		if(i == 2 && !(Key_GetCatcher( ) & KEYCATCH_UI)) {
			if(touchhats[i][0] != 0 || touchhats[i][1] != 0) {
				Com_QueueEvent( in_eventTime, SE_MOUSE, touchhats[i][0], touchhats[i][1], 0, NULL );
			}
		}
		*/
		// TODO: make config options for this?
		if(i == 2 && !(Key_GetCatcher( ) & KEYCATCH_UI)) {
			if(touchhats[i][0] != 0 || touchhats[i][1] != 0) {
				Com_QueueEvent( in_eventTime, SE_MOUSE, touchhats[i][0], 0, 0, NULL );
			}
		}
	}
}
#endif

/*
===============
IN_Init
===============
*/
void IN_Init( void )
{
	Com_DPrintf( "\n------- Input Initialization -------\n" );

	in_keyboardDebug = Cvar_Get( "in_keyboardDebug", "0", CVAR_ARCHIVE );

	// mouse variables
	in_mouse = Cvar_Get( "in_mouse", "1", CVAR_ARCHIVE );
	Cvar_CheckRange( in_mouse, "-1", "1", CV_INTEGER );

	in_joystick = Cvar_Get( "in_joystick", "0", CVAR_ARCHIVE );
	in_joystickThreshold = Cvar_Get( "joy_threshold", "0.15", CVAR_ARCHIVE );
#ifdef USE_JOYSTICK
	j_pitch =        Cvar_Get( "j_pitch",        "0.022", CVAR_ARCHIVE_ND );
	j_yaw =          Cvar_Get( "j_yaw",          "-0.022", CVAR_ARCHIVE_ND );
	j_forward =      Cvar_Get( "j_forward",      "-0.25", CVAR_ARCHIVE_ND );
	j_side =         Cvar_Get( "j_side",         "0.25", CVAR_ARCHIVE_ND );
	j_up =           Cvar_Get( "j_up",           "0", CVAR_ARCHIVE_ND );

	j_pitch_axis =   Cvar_Get( "j_pitch_axis",   "3", CVAR_ARCHIVE_ND );
	j_yaw_axis =     Cvar_Get( "j_yaw_axis",     "2", CVAR_ARCHIVE_ND );
	j_forward_axis = Cvar_Get( "j_forward_axis", "1", CVAR_ARCHIVE_ND );
	j_side_axis =    Cvar_Get( "j_side_axis",    "0", CVAR_ARCHIVE_ND );
	j_up_axis =      Cvar_Get( "j_up_axis",      "4", CVAR_ARCHIVE_ND );

	Cvar_CheckRange( j_pitch_axis,   "0", va("%i",MAX_JOYSTICK_AXIS-1), CV_INTEGER );
	Cvar_CheckRange( j_yaw_axis,     "0", va("%i",MAX_JOYSTICK_AXIS-1), CV_INTEGER );
	Cvar_CheckRange( j_forward_axis, "0", va("%i",MAX_JOYSTICK_AXIS-1), CV_INTEGER );
	Cvar_CheckRange( j_side_axis,    "0", va("%i",MAX_JOYSTICK_AXIS-1), CV_INTEGER );
	Cvar_CheckRange( j_up_axis,      "0", va("%i",MAX_JOYSTICK_AXIS-1), CV_INTEGER );
#endif

	// ~ and `, as keys and characters
	cl_consoleKeys = Cvar_Get( "cl_consoleKeys", "~ ` 0x7e 0x60", CVAR_ARCHIVE );

	// TODO: activate text input for text fields
	//SDL_StartTextInput();

	Com_DPrintf( "------------------------------------\n" );
}



#define SDL_INIT_TIMER          0x00000001
#define SDL_INIT_AUDIO          0x00000010
#define SDL_INIT_VIDEO          0x00000020
#define SDL_INIT_JOYSTICK       0x00000200
#define SDL_INIT_HAPTIC         0x00001000
#define SDL_INIT_NOPARACHUTE    0x00100000      /**< Don't catch fatal signals */
#define SDL_INIT_EVERYTHING     0x0000FFFF


#define SDL_AUDIO_MASK_BITSIZE       (0xFF)
#define SDL_AUDIO_MASK_DATATYPE      (1<<8)
#define SDL_AUDIO_MASK_ENDIAN        (1<<12)
#define SDL_AUDIO_MASK_SIGNED        (1<<15)
#define SDL_AUDIO_BITSIZE(x)         (x & SDL_AUDIO_MASK_BITSIZE)
#define SDL_AUDIO_ISFLOAT(x)         (x & SDL_AUDIO_MASK_DATATYPE)
#define SDL_AUDIO_ISBIGENDIAN(x)     (x & SDL_AUDIO_MASK_ENDIAN)
#define SDL_AUDIO_ISSIGNED(x)        (x & SDL_AUDIO_MASK_SIGNED)
#define SDL_AUDIO_ISINT(x)           (!SDL_AUDIO_ISFLOAT(x))
#define SDL_AUDIO_ISLITTLEENDIAN(x)  (!SDL_AUDIO_ISBIGENDIAN(x))
#define SDL_AUDIO_ISUNSIGNED(x)      (!SDL_AUDIO_ISSIGNED(x))

#define SDL_AUDIO_ALLOW_FREQUENCY_CHANGE    0x00000001
#define SDL_AUDIO_ALLOW_FORMAT_CHANGE       0x00000002
#define SDL_AUDIO_ALLOW_CHANNELS_CHANGE     0x00000004
#define SDL_AUDIO_ALLOW_SAMPLES_CHANGE      0x00000008
#define SDL_AUDIO_ALLOW_ANY_CHANGE          (SDL_AUDIO_ALLOW_FREQUENCY_CHANGE|SDL_AUDIO_ALLOW_FORMAT_CHANGE|SDL_AUDIO_ALLOW_CHANNELS_CHANGE|SDL_AUDIO_ALLOW_SAMPLES_CHANGE)


#define AUDIO_U8        0x0008  /**< Unsigned 8-bit samples */
#define AUDIO_S8        0x8008  /**< Signed 8-bit samples */
#define AUDIO_U16LSB    0x0010  /**< Unsigned 16-bit samples */
#define AUDIO_S16LSB    0x8010  /**< Signed 16-bit samples */
#define AUDIO_U16MSB    0x1010  /**< As above, but big-endian byte order */
#define AUDIO_S16MSB    0x9010  /**< As above, but big-endian byte order */
#define AUDIO_U16       AUDIO_U16LSB
#define AUDIO_S16       AUDIO_S16LSB

#define AUDIO_F32LSB    0x8120  /**< 32-bit floating point samples */
#define AUDIO_F32MSB    0x9120  /**< As above, but big-endian byte order */
#define AUDIO_F32       AUDIO_F32LSB

/**
 *  \name Native audio byte ordering
 */
/* @{ */
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define AUDIO_U16SYS    AUDIO_U16LSB
#define AUDIO_S16SYS    AUDIO_S16LSB
#define AUDIO_S32SYS    AUDIO_S32LSB
#define AUDIO_F32SYS    AUDIO_F32LSB
#else
#define AUDIO_U16SYS    AUDIO_U16MSB
#define AUDIO_S16SYS    AUDIO_S16MSB
#define AUDIO_S32SYS    AUDIO_S32MSB
#define AUDIO_F32SYS    AUDIO_F32MSB
#endif

/**
 *  The calculated values in this structure are calculated by SDL_OpenAudio().
 *
 *  For multi-channel audio, the default SDL channel mapping is:
 *  2:  FL FR                       (stereo)
 *  3:  FL FR LFE                   (2.1 surround)
 *  4:  FL FR BL BR                 (quad)
 *  5:  FL FR FC BL BR              (quad + center)
 *  6:  FL FR FC LFE SL SR          (5.1 surround - last two can also be BL BR)
 *  7:  FL FR FC LFE BC SL SR       (6.1 surround)
 *  8:  FL FR FC LFE BL BR SL SR    (7.1 surround)
 */
typedef uint32_t SDL_AudioDeviceID;
typedef unsigned short SDL_AudioFormat;
typedef void (*SDL_AudioCallback) (void *userdata, uint8_t * stream, int len);

typedef struct SDL_AudioSpec
{
    int freq;                   /**< DSP frequency -- samples per second */
    SDL_AudioFormat format;     /**< Audio data format */
    uint8_t channels;             /**< Number of channels: 1 mono, 2 stereo */
    uint8_t silence;              /**< Audio buffer silence value (calculated) */
    uint16_t samples;             /**< Audio buffer size in sample FRAMES (total samples divided by channel count) */
    uint16_t padding;             /**< Necessary for some compile environments */
    uint32_t size;                /**< Audio buffer size in bytes (calculated) */
    SDL_AudioCallback callback; /**< Callback that feeds the audio device (NULL to use SDL_QueueAudio()). */
    void *userdata;             /**< Userdata passed to callback (ignored for NULL callbacks). */
} SDL_AudioSpec;
int SDL_OpenAudioDevice(const char *device, int iscapture,
                    const SDL_AudioSpec * desired, SDL_AudioSpec * obtained,
                    int allowed_changes);
void SDL_PauseAudioDevice(int dev, int pause_on);
void SDL_CloseAudioDevice(int dev);

extern dma_t	dma;

qboolean snd_inited = qfalse;

extern cvar_t *s_khz;
cvar_t *s_sdlBits;
cvar_t *s_sdlChannels;
cvar_t *s_sdlDevSamps;
cvar_t *s_sdlMixSamps;

/* The audio callback. All the magic happens here. */
static int dmapos = 0;
static int dmasize = 0;

static SDL_AudioDeviceID sdlPlaybackDevice;

// TODO: krunker.io supports VOIP
//#define USE_SDL_AUDIO_CAPTURE

#ifdef USE_SDL_AUDIO_CAPTURE
static SDL_AudioDeviceID sdlCaptureDevice;
static cvar_t *s_sdlCapture;
static float sdlMasterGain = 1.0f;
#endif

void SDL_LockAudioDevice(SDL_AudioDeviceID dev) { }

void SDL_UnlockAudioDevice(SDL_AudioDeviceID dev) { }

void SDL_QuitSubSystem(uint32_t flags) { }

const char *SDL_GetCurrentAudioDriver(void) {
  return "wasm";
}

/*
===============
SNDDMA_AudioCallback
===============
*/
static void SNDDMA_AudioCallback(void *userdata, uint8_t *stream, int len)
{
	int pos = (dmapos * (dma.samplebits/8));
	if (pos >= dmasize)
		dmapos = pos = 0;

	if (!snd_inited)  /* shouldn't happen, but just in case... */
	{
		memset(stream, '\0', len);
		return;
	}
	else
	{
		int tobufend = dmasize - pos;  /* bytes to buffer's end. */
		int len1 = len;
		int len2 = 0;

		if (len1 > tobufend)
		{
			len1 = tobufend;
			len2 = len - len1;
		}
		memcpy(stream, dma.buffer + pos, len1);
		if (len2 <= 0)
			dmapos += (len1 / (dma.samplebits/8));
		else  /* wraparound? */
		{
			memcpy(stream+len1, dma.buffer, len2);
			dmapos = (len2 / (dma.samplebits/8));
		}
	}

	if (dmapos >= dmasize)
		dmapos = 0;

#ifdef USE_SDL_AUDIO_CAPTURE
	if (sdlMasterGain != 1.0f)
	{
		int i;
		if (dma.isfloat && (dma.samplebits == 32))
		{
			float *ptr = (float *) stream;
			len /= sizeof (*ptr);
			for (i = 0; i < len; i++, ptr++)
			{
				*ptr *= sdlMasterGain;
			}
		}
		else if (dma.samplebits == 16)
		{
			int16_t *ptr = (int16_t *) stream;
			len /= sizeof (*ptr);
			for (i = 0; i < len; i++, ptr++)
			{
				*ptr = (int16_t) (((float) *ptr) * sdlMasterGain);
			}
		}
		else if (dma.samplebits == 8)
		{
			uint8_t *ptr = (uint8_t *) stream;
			len /= sizeof (*ptr);
			for (i = 0; i < len; i++, ptr++)
			{
				*ptr = (uint8_t) (((float) *ptr) * sdlMasterGain);
			}
		}
	}
#endif
}

static const struct
{
	uint16_t	enumFormat;
	const char	*stringFormat;
} formatToStringTable[ ] =
{
	{ AUDIO_U8,     "AUDIO_U8" },
	{ AUDIO_S8,     "AUDIO_S8" },
	{ AUDIO_U16LSB, "AUDIO_U16LSB" },
	{ AUDIO_S16LSB, "AUDIO_S16LSB" },
	{ AUDIO_U16MSB, "AUDIO_U16MSB" },
	{ AUDIO_S16MSB, "AUDIO_S16MSB" },
	{ AUDIO_F32LSB, "AUDIO_F32LSB" },
	{ AUDIO_F32MSB, "AUDIO_F32MSB" }
};

static int formatToStringTableSize = ARRAY_LEN( formatToStringTable );

/*
===============
SNDDMA_PrintAudiospec
===============
*/
static void SNDDMA_PrintAudiospec(const char *str, const SDL_AudioSpec *spec)
{
	const char *fmt = NULL;
	int i;

	Com_Printf( "%s:\n", str );

	for ( i = 0; i < formatToStringTableSize; i++ ) {
		if( spec->format == formatToStringTable[ i ].enumFormat ) {
			fmt = formatToStringTable[ i ].stringFormat;
		}
	}

	if ( fmt ) {
		Com_Printf( "  Format:   %s\n", fmt );
	} else {
		Com_Printf( "  Format:   " S_COLOR_RED "UNKNOWN\n");
	}

	Com_Printf( "  Freq:     %d\n", (int) spec->freq );
	Com_Printf( "  Samples:  %d\n", (int) spec->samples );
	Com_Printf( "  Channels: %d\n", (int) spec->channels );
}


static int SNDDMA_KHzToHz( int khz )
{
	switch ( khz )
	{
		default:
		case 22: return 22050;
		case 48: return 48000;
		case 44: return 44100;
		case 11: return 11025;
	}
}


/*
===============
SNDDMA_Init
===============
*/
qboolean SNDDMA_Init( void )
{
	SDL_AudioSpec desired;
	SDL_AudioSpec obtained;
	int tmp;

	if ( snd_inited )
		return qtrue;

	//if ( !s_sdlBits )
	{
		s_sdlBits = Cvar_Get( "s_sdlBits", "16", CVAR_ARCHIVE_ND | CVAR_LATCH );
		Cvar_CheckRange( s_sdlBits, "8", "16", CV_INTEGER );

		s_sdlChannels = Cvar_Get( "s_sdlChannels", "2", CVAR_ARCHIVE_ND | CVAR_LATCH );
		Cvar_CheckRange( s_sdlChannels, "1", "2", CV_INTEGER );

		s_sdlDevSamps = Cvar_Get( "s_sdlDevSamps", "0", CVAR_ARCHIVE_ND | CVAR_LATCH );
		s_sdlMixSamps = Cvar_Get( "s_sdlMixSamps", "0", CVAR_ARCHIVE_ND | CVAR_LATCH );
	}

	Com_Printf( "SDL_Init( SDL_INIT_AUDIO )... " );

/*
	if ( SDL_Init( SDL_INIT_AUDIO ) != 0 )
	{
		return qfalse;
	}
*/

	Com_Printf( "OK\n" );

	Com_Printf( "SDL audio driver is \"%s\".\n", SDL_GetCurrentAudioDriver() );

	memset( &desired, '\0', sizeof (desired) );
	memset( &obtained, '\0', sizeof (obtained) );

	desired.freq = SNDDMA_KHzToHz( s_khz->integer );
	if ( desired.freq == 0 )
		desired.freq = 22050;

	tmp = s_sdlBits->integer;
	if ( tmp < 16 )
		tmp = 8;

	desired.format = ((tmp == 16) ? AUDIO_S16SYS : AUDIO_U8);

	// I dunno if this is the best idea, but I'll give it a try...
	//  should probably check a cvar for this...
	if ( s_sdlDevSamps->integer )
		desired.samples = s_sdlDevSamps->value;
	else
	{
		// just pick a sane default.
		if (desired.freq <= 11025)
			desired.samples = 256;
		else if (desired.freq <= 22050)
			desired.samples = 512;
		else if (desired.freq <= 44100)
			desired.samples = 1024;
		else
			desired.samples = 2048;  // (*shrug*)
	}

	desired.channels = s_sdlChannels->integer;
	desired.callback = SNDDMA_AudioCallback;

	sdlPlaybackDevice = SDL_OpenAudioDevice( NULL, qfalse, &desired, &obtained, SDL_AUDIO_ALLOW_ANY_CHANGE );
	if ( sdlPlaybackDevice == 0 )
	{
		SDL_QuitSubSystem( SDL_INIT_AUDIO );
		return qfalse;
	}

	SNDDMA_PrintAudiospec( "SDL_AudioSpec", &obtained );

	// dma.samples needs to be big, or id's mixer will just refuse to
	//  work at all; we need to keep it significantly bigger than the
	//  amount of SDL callback samples, and just copy a little each time
	//  the callback runs.
	// 32768 is what the OSS driver filled in here on my system. I don't
	//  know if it's a good value overall, but at least we know it's
	//  reasonable...this is why I let the user override.
	tmp = s_sdlMixSamps->integer;
	if ( !tmp )
		tmp = (obtained.samples * obtained.channels) * 10;

	// samples must be divisible by number of channels
	tmp -= tmp % obtained.channels;
	// round up to next power of 2
	tmp = log2pad( tmp, 1 );

	dmapos = 0;
	dma.samplebits = SDL_AUDIO_BITSIZE( obtained.format );
	dma.isfloat = SDL_AUDIO_ISFLOAT( obtained.format );
	dma.channels = obtained.channels;
	dma.samples = tmp;
	dma.fullsamples = dma.samples / dma.channels;
	dma.submission_chunk = 1;
	dma.speed = obtained.freq;
	dmasize = (dma.samples * (dma.samplebits/8));
	dma.buffer = calloc(1, dmasize);

#ifdef USE_SDL_AUDIO_CAPTURE
	// !!! FIXME: some of these SDL_OpenAudioDevice() values should be cvars.
	s_sdlCapture = Cvar_Get( "s_sdlCapture", "1", CVAR_ARCHIVE | CVAR_LATCH );
	// !!! FIXME: pulseaudio capture records audio the entire time the program is running. https://bugzilla.libsdl.org/show_bug.cgi?id=4087
	if (Q_stricmp(SDL_GetCurrentAudioDriver(), "pulseaudio") == 0)
	{
		Com_Printf("SDL audio capture support disabled for pulseaudio (https://bugzilla.libsdl.org/show_bug.cgi?id=4087)\n");
	}
	else if (!s_sdlCapture->integer)
	{
		Com_Printf("SDL audio capture support disabled by user ('+set s_sdlCapture 1' to enable)\n");
	}
#if USE_MUMBLE
	else if (cl_useMumble->integer)
	{
		Com_Printf("SDL audio capture support disabled for Mumble support\n");
	}
#endif
	else
	{
		/* !!! FIXME: list available devices and let cvar specify one, like OpenAL does */
		SDL_AudioSpec spec;
		SDL_zero(spec);
		spec.freq = 48000;
		spec.format = AUDIO_S16SYS;
		spec.channels = 1;
		spec.samples = VOIP_MAX_PACKET_SAMPLES * 4;
		sdlCaptureDevice = SDL_OpenAudioDevice(NULL, SDL_TRUE, &spec, NULL, 0);
		Com_Printf( "SDL capture device %s.\n",
				    (sdlCaptureDevice == 0) ? "failed to open" : "opened");
	}

	sdlMasterGain = 1.0f;
#endif

	Com_Printf("Starting SDL audio callback...\n");
	SDL_PauseAudioDevice(sdlPlaybackDevice, 0);  // start callback.
	// don't unpause the capture device; we'll do that in StartCapture.

	Com_Printf("SDL audio initialized.\n");
	snd_inited = qtrue;
	return qtrue;
}


/*
===============
SNDDMA_GetDMAPos
===============
*/
int SNDDMA_GetDMAPos( void )
{
	return dmapos;
}


/*
===============
SNDDMA_Shutdown
===============
*/
void SNDDMA_Shutdown( void )
{
	if (sdlPlaybackDevice != 0)
	{
		Com_Printf("Closing SDL audio playback device...\n");
		SDL_CloseAudioDevice(sdlPlaybackDevice);
		Com_Printf("SDL audio playback device closed.\n");
		sdlPlaybackDevice = 0;
	}

#ifdef USE_SDL_AUDIO_CAPTURE
	if (sdlCaptureDevice)
	{
		Com_Printf("Closing SDL audio capture device...\n");
		SDL_CloseAudioDevice(sdlCaptureDevice);
		Com_Printf("SDL audio capture device closed.\n");
		sdlCaptureDevice = 0;
	}
#endif

	SDL_QuitSubSystem(SDL_INIT_AUDIO);
	free(dma.buffer);
	dma.buffer = NULL;
	dmapos = dmasize = 0;
	snd_inited = qfalse;
	Com_Printf("SDL audio shut down.\n");
}


/*
===============
SNDDMA_Submit

Send sound to device if buffer isn't really the dma buffer
===============
*/
void SNDDMA_Submit( void )
{
	SDL_UnlockAudioDevice( sdlPlaybackDevice );
}


/*
===============
SNDDMA_BeginPainting
===============
*/
void SNDDMA_BeginPainting( void )
{
	SDL_LockAudioDevice( sdlPlaybackDevice );
}


#ifdef USE_VOIP
void SNDDMA_StartCapture(void)
{
#ifdef USE_SDL_AUDIO_CAPTURE
	if (sdlCaptureDevice)
	{
		SDL_ClearQueuedAudio(sdlCaptureDevice);
		SDL_PauseAudioDevice(sdlCaptureDevice, 0);
	}
#endif
}


int SNDDMA_AvailableCaptureSamples(void)
{
#ifdef USE_SDL_AUDIO_CAPTURE
	// divided by 2 to convert from bytes to (mono16) samples.
	return sdlCaptureDevice ? (SDL_GetQueuedAudioSize(sdlCaptureDevice) / 2) : 0;
#else
	return 0;
#endif
}


void SNDDMA_Capture(int samples, byte *data)
{
#ifdef USE_SDL_AUDIO_CAPTURE
	// multiplied by 2 to convert from (mono16) samples to bytes.
	if (sdlCaptureDevice)
	{
		SDL_DequeueAudio(sdlCaptureDevice, data, samples * 2);
	}
	else
#endif
	{
		SDL_memset(data, '\0', samples * 2);
	}
}

void SNDDMA_StopCapture(void)
{
#ifdef USE_SDL_AUDIO_CAPTURE
	if (sdlCaptureDevice)
	{
		SDL_PauseAudioDevice(sdlCaptureDevice, 1);
	}
#endif
}

void SNDDMA_MasterGain( float val )
{
#ifdef USE_SDL_AUDIO_CAPTURE
	sdlMasterGain = val;
#endif
}
#endif

