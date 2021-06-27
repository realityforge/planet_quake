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

#include "../client/client.h"
#include "../renderercommon/tr_public.h"
#include "./sys_glw.h"

typedef enum {
RSERR_OK,
RSERR_INVALID_FULLSCREEN,
RSERR_INVALID_MODE,
RSERR_FATAL_ERROR,
RSERR_UNKNOWN
} rserr_t;

glwstate_t glw_state;

SDL_Window *SDL_window = NULL;
void *SDL_glContext = NULL;

cvar_t *r_stereoEnabled;
cvar_t *in_nograb;


EM_JS(void *, SYS_CreateContext, ( void *window ), 
{ return Sys_GL_CreateContext(window) });
void *SDL_GL_CreateContext(void *window)
{ return SYS_CreateContext(window); }

EM_JS(int, SYS_SDL_Init, ( uint32_t flags ), 
{ return Sys_GLimpInit(flags) });
int SDL_Init(uint32_t flags)
{ return SYS_SDL_Init(flags); }

EM_JS(char *, SYS_SDL_GetError, ( void ), 
{ return SDL_GetError() });
char *SDL_GetError( void )
{ return SYS_SDL_GetError(); }

EM_JS(int, SYS_DisplayMode, ( int displayIndex, SDL_DisplayMode *mode ), 
{ return Sys_GetDisplayMode(displayIndex, mode) });
int SDL_GetDesktopDisplayMode(int displayIndex, SDL_DisplayMode *mode)
{ return SYS_DisplayMode(displayIndex, mode); }

EM_JS(int, SYS_GetWindowDisplayMode, ( void *window, SDL_DisplayMode *mode ), 
{ return Sys_GetDisplayMode(window, mode) });
int SDL_GetWindowDisplayMode(void *window, SDL_DisplayMode *mode)
{ return SYS_GetWindowDisplayMode(window, mode); }

EM_JS(int, SYS_GL_SetAttribute, ( int attr, int value ), 
{ return Sys_GL_SetAttribute(attr, value) });
int SDL_GL_SetAttribute(int attr, int value)
{ return SYS_GL_SetAttribute(attr, value); }

EM_JS(void *, SYS_SDL_CreateWindow, ( const char *title,
                              int x, int y, int w,
                              int h, uint32_t flags ), 
{ return SDL_CreateWindow(title, x, y, w, h, flags) });
void *SDL_CreateWindow(const char *title,
                              int x, int y, int w,
                              int h, uint32_t flags)
{ return SYS_SDL_CreateWindow(title, x, y, w, h, flags); }

EM_JS(int, SYS_SetDisplay, ( void *window, const SDL_DisplayMode *mode ), 
{ return SDL_SetWindowDisplayMode(window, mode) });
int SDL_SetWindowDisplayMode(void *window, const SDL_DisplayMode *mode)
{ return SYS_SetDisplay(window, mode); }

EM_JS(void, SYS_GetDrawable, ( void *window, int *w, int *h ), 
{ SDL_GL_GetDrawableSize(window, w, h) });
void SDL_GL_GetDrawableSize(void *window, int *w, int *h)
{ SYS_GetDrawable(window, w, h); }

EM_JS(uint32_t, SYS_WasInit, ( uint32_t flags ), 
{ return SDL_WasInit(flags) });
uint32_t SDL_WasInit(uint32_t flags)
{ return SYS_WasInit(flags); }
/*
SDL_GetCurrentVideoDriver
SDL_GL_SwapWindow
SDL_ShowCursor
SDL_SetWindowGrab
SDL_SetRelativeMouseMode
SDL_GetWindowFlags
SDL_StopTextInput
*/

void  SDL_WarpMouseInWindow(void *window, int x, int y) {
  // Javascript can't do this yet
}

int SDL_GL_SetSwapInterval(int interval) {
  // set values for request animation frame?
  return 0;
}

void SDL_DestroyWindow(void *window) {
  // destroy window?
}

void SDL_GL_DeleteContext(void *context) {
  // delete context?  we don't currently do this because of lazy mode
}

void *GL_GetProcAddress( const char *name ) { return NULL; }


/*
===============
GLimp_Shutdown
===============
*/
void GLimp_Shutdown( qboolean unloadDLL )
{
#ifdef USE_LAZY_MEMORY
  // never shutdown here, will shut down when platform exits
  return;
#else
  IN_Shutdown();

  SDL_DestroyWindow( SDL_window );
  SDL_window = NULL;

  if ( glw_state.isFullscreen )
    SDL_WarpMouseGlobal( glw_state.desktop_width / 2, glw_state.desktop_height / 2 );

  if ( unloadDLL )
    SDL_QuitSubSystem( SDL_INIT_VIDEO );
#endif
}


/*
===============
GLimp_Minimize

Minimize the game so that user is back at the desktop
===============
*/
void GLimp_Minimize( void )
{
  SDL_MinimizeWindow( SDL_window );
}


/*
===============
GLimp_LogComment
===============
*/
void GLimp_LogComment( char *comment )
{
}


/*
===============
GLimp_SetMode
===============
*/
static int GLW_SetMode( int mode, const char *modeFS, qboolean fullscreen, qboolean webGL2 )
{
  glconfig_t *config = glw_state.config;
  int perChannelColorBits;
  int colorBits, depthBits, stencilBits;
  int i;
  SDL_DisplayMode desktopMode;
  int display;
  int x;
  int y;

  Com_Printf( "Initializing OpenGL display\n");

  // If a window exists, note its display index
  if ( SDL_window != NULL )
  {
    display = 0;
  }
  else
  {
    x = vid_xpos->integer;
    y = vid_ypos->integer;

    // find out to which display our window belongs to
    // according to previously stored \vid_xpos and \vid_ypos coordinates
    display = 0;

    //Com_Printf("Selected display: %i\n", display );
  }

  if ( display >= 0 && SDL_GetDesktopDisplayMode( display, &desktopMode ) == 0 )
  {
    glw_state.desktop_width = desktopMode.w;
    glw_state.desktop_height = desktopMode.h;
  }
  else
  {
    glw_state.desktop_width = 640;
    glw_state.desktop_height = 480;
  }

  config->isFullscreen = fullscreen;
  glw_state.isFullscreen = fullscreen;

  Com_Printf( "...setting mode %d:", mode );

  if ( !CL_GetModeInfo( &config->vidWidth, &config->vidHeight, &config->windowAspect, mode, modeFS, glw_state.desktop_width, glw_state.desktop_height, fullscreen ) )
  {
    Com_Printf( " invalid mode\n" );
    return RSERR_INVALID_MODE;
  }

  Com_Printf( " %d %d\n", config->vidWidth, config->vidHeight );

  // Destroy existing state if it exists
  if ( SDL_glContext != NULL )
  {
    SDL_GL_DeleteContext( SDL_glContext );
    SDL_glContext = NULL;
  }

  gw_active = qfalse;
  gw_minimized = qtrue;

  colorBits = r_colorbits->value;

  if ( colorBits == 0 || colorBits > 24 )
    colorBits = 24;

  if ( cl_depthbits->integer == 0 )
  {
    // implicitly assume Z-buffer depth == desktop color depth
    if ( colorBits > 16 )
    depthBits = 24;
    else
      depthBits = 16;
  }
  else
    depthBits = cl_depthbits->integer;

  stencilBits = cl_stencilbits->integer;

  // do not allow stencil if Z-buffer depth likely won't contain it
  if ( depthBits < 24 )
    stencilBits = 0;

  for ( i = 0; i < 16; i++ )
  {
    int testColorBits, testDepthBits, testStencilBits;
    int realColorBits[3];

    // 0 - default
    // 1 - minus colorBits
    // 2 - minus depthBits
    // 3 - minus stencil
    if ((i % 4) == 0 && i)
    {
      // one pass, reduce
      switch (i / 4)
      {
        case 2 :
          if (colorBits == 24)
            colorBits = 16;
          break;
        case 1 :
          if (depthBits == 24)
            depthBits = 16;
          else if (depthBits == 16)
            depthBits = 8;
        case 3 :
          if (stencilBits == 24)
            stencilBits = 16;
          else if (stencilBits == 16)
            stencilBits = 8;
      }
    }

    testColorBits = colorBits;
    testDepthBits = depthBits;
    testStencilBits = stencilBits;

    if ((i % 4) == 3)
    { // reduce colorBits
      if (testColorBits == 24)
        testColorBits = 16;
    }

    if ((i % 4) == 2)
    { // reduce depthBits
      if (testDepthBits == 24)
        testDepthBits = 16;
      else if (testDepthBits == 16)
        testDepthBits = 8;
    }

    if ((i % 4) == 1)
    { // reduce stencilBits
      if (testStencilBits == 24)
        testStencilBits = 16;
      else if (testStencilBits == 16)
        testStencilBits = 8;
      else
        testStencilBits = 0;
    }

    if ( testColorBits == 24 )
      perChannelColorBits = 8;
    else
      perChannelColorBits = 4;

    {

#ifdef __sgi /* Fix for SGIs grabbing too many bits of color */
      if (perChannelColorBits == 4)
        perChannelColorBits = 0; /* Use minimum size for 16-bit color */

      /* Need alpha or else SGIs choose 36+ bit RGB mode */
      SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 1 );
#endif

      SDL_GL_SetAttribute( SDL_GL_RED_SIZE, perChannelColorBits );
      SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, perChannelColorBits );
      SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, perChannelColorBits );
      SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, testDepthBits );
      SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, testStencilBits );

      SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 0 );
      SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 0 );

      if ( r_stereoEnabled->integer )
      {
        config->stereoEnabled = qtrue;
        SDL_GL_SetAttribute( SDL_GL_STEREO, 1 );
      }
      else
      {
        config->stereoEnabled = qfalse;
        SDL_GL_SetAttribute( SDL_GL_STEREO, 0 );
      }
    
      SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

      if ( !r_allowSoftwareGL->integer )
        SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );
    }

    if ( ( SDL_window = SDL_CreateWindow( cl_title, x, y, config->vidWidth, config->vidHeight, fullscreen ? SDL_WINDOW_FULLSCREEN : 0 ) ) == NULL )
    {
      Com_DPrintf( "SDL_CreateWindow failed: %s\n", SDL_GetError() );
      continue;
    }

    if ( fullscreen )
    {
      SDL_DisplayMode mode;

      switch ( testColorBits )
      {
        case 16: mode.format = 16; break;
        case 24: mode.format = 24;  break;
        default: Com_DPrintf( "testColorBits is %d, can't fullscreen\n", testColorBits ); continue;
      }

      mode.w = config->vidWidth;
      mode.h = config->vidHeight;
      mode.refresh_rate = /* config->displayFrequency = */ Cvar_VariableIntegerValue( "r_displayRefresh" );
      mode.driverdata = NULL;

      if ( SDL_SetWindowDisplayMode( SDL_window, &mode ) < 0 )
      {
        Com_DPrintf( "SDL_SetWindowDisplayMode failed: %s\n", SDL_GetError( ) );
        continue;
      }

      if ( SDL_GetWindowDisplayMode( SDL_window, &mode ) >= 0 )
      {
        config->displayFrequency = mode.refresh_rate;
        config->vidWidth = mode.w;
        config->vidHeight = mode.h;
      }
    }

    {
      if ( !SDL_glContext )
      {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        if(webGL2) {
          SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
          SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        } else {
          SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
          SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        }
        if ( ( SDL_glContext = SDL_GL_CreateContext( SDL_window ) ) == NULL )
        {
          Com_DPrintf( "SDL_GL_CreateContext failed: %s\n", SDL_GetError( ) );
          SDL_DestroyWindow( SDL_window );
          SDL_window = NULL;
          continue;
        }
      }

      if ( SDL_GL_SetSwapInterval( r_swapInterval->integer ) == -1 )
      {
        Com_DPrintf( "SDL_GL_SetSwapInterval failed: %s\n", SDL_GetError( ) );
      }

#if 0
      SDL_GL_GetAttribute( SDL_GL_RED_SIZE, &realColorBits[0] );
      SDL_GL_GetAttribute( SDL_GL_GREEN_SIZE, &realColorBits[1] );
      SDL_GL_GetAttribute( SDL_GL_BLUE_SIZE, &realColorBits[2] );
      SDL_GL_GetAttribute( SDL_GL_DEPTH_SIZE, &config->depthBits );
      SDL_GL_GetAttribute( SDL_GL_STENCIL_SIZE, &config->stencilBits );

      config->colorBits = realColorBits[0] + realColorBits[1] + realColorBits[2];
#endif
    }


    Com_Printf( "Using %d color bits, %d depth, %d stencil display.\n",	config->colorBits, config->depthBits, config->stencilBits );

    break;
  }

  if ( SDL_window )
  {
#ifdef USE_ICON
    SDL_Surface *icon = SDL_CreateRGBSurfaceFrom(
      (void *)CLIENT_WINDOW_ICON.pixel_data,
      CLIENT_WINDOW_ICON.width,
      CLIENT_WINDOW_ICON.height,
      CLIENT_WINDOW_ICON.bytes_per_pixel * 8,
      CLIENT_WINDOW_ICON.bytes_per_pixel * CLIENT_WINDOW_ICON.width,
#ifdef Q3_LITTLE_ENDIAN
      0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000
#else
      0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF
#endif
    );
    if ( icon )
    {
      SDL_SetWindowIcon( SDL_window, icon );
  SDL_FreeSurface( icon );
    }
#endif
  }
  else
  {
    Com_Printf( "Couldn't get a visual\n" );
    return RSERR_INVALID_MODE;
  }

  SDL_GL_GetDrawableSize( SDL_window, &config->vidWidth, &config->vidHeight );

  // save render dimensions as renderer may change it in advance
  glw_state.window_width = config->vidWidth;
  glw_state.window_height = config->vidHeight;

  SDL_WarpMouseInWindow( SDL_window, glw_state.window_width / 2, glw_state.window_height / 2 );

  return RSERR_OK;
}


/*
===============
GLimp_StartDriverAndSetMode
===============
*/
static rserr_t GLimp_StartDriverAndSetMode( int mode, const char *modeFS, qboolean fullscreen, qboolean webGL2 )
{
  rserr_t err;

  if ( fullscreen && in_nograb->integer )
  {
    Com_Printf( "Fullscreen not allowed with \\in_nograb 1\n");
    Cvar_Set( "r_fullscreen", "0" );
    r_fullscreen->modified = qfalse;
    fullscreen = qfalse;
  }

  if ( !SDL_WasInit( SDL_INIT_VIDEO ) )
  {
    const char *driverName;

    if ( SDL_Init( SDL_INIT_VIDEO ) != 0 )
    {
      Com_Printf( "SDL_Init( SDL_INIT_VIDEO ) FAILED (%s)\n", SDL_GetError() );
      return RSERR_FATAL_ERROR;
    }

    driverName = SDL_GetCurrentVideoDriver();

    Com_Printf( "SDL using driver \"%s\"\n", driverName );
  }

#if 0
  if(SDL_window) {
    SDL_DisplayMode desktopMode;
    int display = SDL_GetWindowDisplayIndex( SDL_window );
    if ( SDL_GetDesktopDisplayMode( display, &desktopMode ) == 0 )
    {
      glw_state.config->vidWidth = glw_state.desktop_width = desktopMode.w;
      glw_state.config->vidHeight = glw_state.desktop_height = desktopMode.h;
    }
    else
    {
      glw_state.config->vidWidth = glw_state.desktop_width = 640;
      glw_state.config->vidHeight = glw_state.desktop_height = 480;
    }
    SDL_GL_GetDrawableSize( SDL_window, &glw_state.config->vidWidth, &glw_state.config->vidHeight );
    Com_Printf( "...setting mode %d: %d %d\n", mode, glw_state.config->vidWidth, glw_state.config->vidHeight );
    return RSERR_OK;
  }
#endif

  err = GLW_SetMode( mode, modeFS, fullscreen, webGL2 );

  switch ( err )
  {
    case RSERR_INVALID_FULLSCREEN:
      Com_Printf( "...WARNING: fullscreen unavailable in this mode\n" );
      return err;
    case RSERR_INVALID_MODE:
      Com_Printf( "...WARNING: could not set the given mode (%d)\n", mode );
      return err;
    default:
      break;
  }

  return RSERR_OK;
}


/*
=============
RE_UpdateMode
=============
*/
void GLimp_UpdateMode( glconfig_t *config ) {
  GLW_SetMode( r_mode->integer, r_modeFullscreen->string, r_fullscreen->integer, qtrue );
}


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

  glw_state.config = config; // feedback renderer configuration

  in_nograb = Cvar_Get( "in_nograb", "0", CVAR_ARCHIVE );
  Cvar_SetDescription(in_nograb, "Don't grab mouse when client in not in fullscreen mode\nDefault: 0");

  r_allowSoftwareGL = Cvar_Get( "r_allowSoftwareGL", "0", CVAR_LATCH );
  Cvar_SetDescription(r_allowSoftwareGL, "Toggle the use of the default software OpenGL driver\nDefault: 0");

  r_swapInterval = Cvar_Get( "r_swapInterval", "0", CVAR_ARCHIVE | CVAR_LATCH );
  Cvar_SetDescription( r_swapInterval, "Toggle frame swapping\nDefault: 0" );
  r_stereoEnabled = Cvar_Get( "r_stereoEnabled", "0", CVAR_ARCHIVE | CVAR_LATCH );
  Cvar_SetDescription( r_stereoEnabled, "Enable stereo rendering for use with virtual reality headsets\nDefault: 0");

  // Create the window and set up the context
  err = GLimp_StartDriverAndSetMode( r_mode->integer, r_modeFullscreen->string, r_fullscreen->integer, qtrue );
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
    if ( GLimp_StartDriverAndSetMode( r_mode->integer, "", r_fullscreen->integer, qfalse ) != RSERR_OK )
    {
      // Nothing worked, give up
      Com_Error( ERR_FATAL, "GLimp_Init() - could not load OpenGL subsystem" );
      return;
    }
  }

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
    SDL_GL_SwapWindow( SDL_window );
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
