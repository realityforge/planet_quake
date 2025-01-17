// config include file common for all Windows builds

// map BSD/Linux/OSX string compare function to Win pendant
#define strcasecmp strcmpi

// for visual studio 2005: use secure template overloads of strcopy and the like
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1

// and disable warnings about those calls that can't be converted. We may want to look at
// them later, though.
#define _CRT_SECURE_NO_DEPRECATE 1

// include of windows.h needed for consistency of silly windows #defines ( SetPort -> SetPortA, GetUserName -> GetUserNameA )
#include  <windows.h>

// include common non-autoconf config file
#include "config_ide.h"

// disable long identifier warnings, they are common in STL
#pragma warning ( disable: 4786 )

// disable POD initialization behavior change warning in VisualC++ 2005
#pragma warning ( disable: 4345 )

// compatibility with later mingw versions
#if defined(__GNUC__) && __GNUC__ >= 4
#define HAVE_ISBLANK
#endif

// activate dirty OpenGL initialization option
#ifndef DIRTY
#define DIRTY
#endif

// Define if this is a Windows OS.
#ifndef WIN32
#define WIN32
#endif

// exists in windows headers
#define HAVE_ISBLANK

// does not exist
#undef HAVE_CLEARENV

// defines for data directories in Windows
#ifndef DEBUG
#define USER_DATA_DIR  "${APPDATA}/Armagetron" 
#define SCREENSHOT_DIR "${MYPICTURES}/Armagetron" 
#else
#define USER_DATA_DIR  "." 
#endif

// Define this for the particle library
#define PARTICLEDLL_EXPORTS
