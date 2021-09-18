
#ifdef USE_RMLUI
#include "client.h"
#include <RmlUi/Wrapper.h>

#ifdef USE_RMLUI_DLOPEN
static void	*rmlLib;
static char dllName[ MAX_OSPATH ];
Rml_ContextRender_t Rml_ContextRender = NULL;
Rml_ContextUpdate_t Rml_ContextUpdate = NULL;
Rml_Shutdown_t      Rml_Shutdown      = NULL;
#endif

static fileHandle_t CL_RmlOpen(const char * filename) {
	fileHandle_t h;
	/*int size = */ FS_FOpenFileRead(filename, &h, qfalse);
	return h;
}

static void CL_RmlClose(fileHandle_t h) {
	FS_FCloseFile( h );
}

static size_t CL_RmlRead(void* buffer, size_t size, fileHandle_t file) {
	return FS_Read(buffer, size, file);
}

static qboolean CL_RmlSeek(fileHandle_t file, long offset, int origin) {
	return FS_Seek(file, offset, origin);
}

static int CL_RmlTell(fileHandle_t file) {
	return FS_FTell(file);
}

static int CL_RmlLength(fileHandle_t h) {
	int pos = FS_FTell( h );
	FS_Seek( h, 0, FS_SEEK_END );
	int end = FS_FTell( h );
	FS_Seek( h, pos, FS_SEEK_SET );
	return end;
}

int CL_RmlLoadFile( const char *qpath, char **buffer )
{
	Com_Printf("Load file: %s\n", qpath);
	return FS_ReadFile(qpath, (void **)buffer);
}

typedef enum 
{
  LT_ALWAYS = 0,
  LT_ERROR,
  LT_ASSERT,
  LT_WARNING,
  LT_INFO,
  LT_DEBUG,
  LT_MAX
} rmlLog_t;

static qboolean CL_RmlLogMessage(int type, const char *message) {
	switch(type) {
		case LT_ALWAYS:
		Com_Printf("RMLUI: %s\n", message);
		break;
	  case LT_ERROR:
		Com_Error(ERR_FATAL, "RMLUI: %s\n", message);
		break;
	  case LT_ASSERT:
		Com_Error(ERR_FATAL, "RMLUI: %s\n", message);
		break;
	  case LT_WARNING:
		Com_Printf(S_COLOR_YELLOW "RMLUI: %s\n", message);
		break;
	  case LT_INFO:
		Com_Printf(S_COLOR_WHITE "RMLUI: %s\n", message);
		break;
	  case LT_DEBUG:
		Com_DPrintf("RMLUI: %s\n", message);
		break;
	  case LT_MAX:
		Com_Error(ERR_FATAL, "RMLUI: %s\n", message);
	}
	return qtrue;
}

static double CL_RmlGetElapsedTime( void ) {
	return Sys_Milliseconds() / 1000;
}

static qhandle_t CL_RmlLoadTexture(int *dimensions, const char *source) {
  qhandle_t result = re.RegisterImage(dimensions, source);
  //dimensions[0] = dimensions[0] * (640/480);
	//dimensions[1] = dimensions[1] * (480/640);
  return result;
}

static int imgCount = 0;
static qhandle_t CL_RmlGenerateTexture(const byte *source, const int *source_dimensions) {
	return re.CreateShaderFromRaw(va("rml_%i", ++imgCount), source, source_dimensions[0], source_dimensions[1]);
}

static void CL_RmlRenderGeometry(void *vertices, int num_vertices, int* indices, 
  int num_indices, qhandle_t texture, const vec2_t translation)
{
  re.RenderGeometry(vertices, num_vertices, indices, num_indices, texture, translation);
}

static qhandle_t CL_RmlCompileGeometry(void *vertices, int num_vertices, int* indices, 
  int num_indices, qhandle_t texture)
{
  return 0;
}

void CL_UIContextRender(void) {
  Rml_ContextRender(0);
}


void CL_InitRmlUi( void ) {
#ifdef USE_RMLUI_DLOPEN

#if defined (__linux__) && defined(__i386__)
#define REND_ARCH_STRING "x86"
#else
#define REND_ARCH_STRING ARCH_STRING
#endif // __linux__

	Com_sprintf( dllName, sizeof( dllName ), "libRmlCore_" REND_ARCH_STRING DLL_EXT );
	rmlLib = FS_LoadLibrary( dllName );

	if ( !rmlLib )
	{
		Com_Error( ERR_FATAL, "Failed to load rmlui %s", dllName );
	}

	Rml_SetFileInterface_t Rml_SetFileInterface = Sys_LoadFunction( rmlLib, "Rml_SetFileInterface" );
	if( !Rml_SetFileInterface )
	{
		Com_Error( ERR_FATAL, "Can't load symbol Rml_SetFileInterface" );
		return;
	}
  Rml_SetSystemInterface_t Rml_SetSystemInterface = Sys_LoadFunction( rmlLib, "Rml_SetSystemInterface" );
  Rml_Initialize_t Rml_Initialize = Sys_LoadFunction( rmlLib, "Rml_Initialize" );
  Rml_SetRenderInterface_t Rml_SetRenderInterface = Sys_LoadFunction( rmlLib, "Rml_SetRenderInterface" );
  Rml_CreateContext_t Rml_CreateContext = Sys_LoadFunction( rmlLib, "Rml_CreateContext" );
  Rml_LoadDocument_t Rml_LoadDocument = Sys_LoadFunction( rmlLib, "Rml_LoadDocument" );
  Rml_ShowDocument_t Rml_ShowDocument = Sys_LoadFunction( rmlLib, "Rml_ShowDocument" );
  Rml_Shutdown_t Rml_Shutdown = Sys_LoadFunction( rmlLib, "Rml_Shutdown" );
  Rml_LoadFontFace_t Rml_LoadFontFace = Sys_LoadFunction( rmlLib, "Rml_LoadFontFace" );
  Rml_ContextRender = Sys_LoadFunction( rmlLib, "Rml_ContextRender" );
  Rml_ContextUpdate = Sys_LoadFunction( rmlLib, "Rml_ContextUpdate" );
  Rml_Shutdown = Sys_LoadFunction( rmlLib, "Rml_Shutdown" );
#endif // USE_BOTLIB_DLOPEN
;
	static RmlFileInterface files;
	files.Open = CL_RmlOpen;
	files.Close = CL_RmlClose;
	files.Read = CL_RmlRead;
	files.Seek = CL_RmlSeek;
	files.Tell = CL_RmlTell;
	files.Length = CL_RmlLength;
	files.LoadFile = CL_RmlLoadFile;
	static RmlRenderInterface renderer;
	renderer.LoadTexture = CL_RmlLoadTexture;
  renderer.GenerateTexture = CL_RmlGenerateTexture;
  renderer.RenderGeometry = CL_RmlRenderGeometry;
  //renderer.CompileGeometry = CL_RmlCompileGeometry;
	static RmlSystemInterface system;
	system.LogMessage = CL_RmlLogMessage;
	system.GetElapsedTime = CL_RmlGetElapsedTime;
	Rml_SetFileInterface(&files);
	Rml_SetRenderInterface(&renderer);
	Rml_SetSystemInterface(&system);
	if(!Rml_Initialize()) {
		Com_Printf("RMLUI: Error initializing.");
	}  
  
	struct FontFace {
		char *filename;
		qboolean fallback_face;
	};
	struct FontFace font_faces[] = {
		{ "LatoLatin-Regular.ttf",    qfalse },
		{ "LatoLatin-Italic.ttf",     qfalse },
		{ "LatoLatin-Bold.ttf",       qfalse },
		{ "LatoLatin-BoldItalic.ttf", qfalse },
		{ "NotoEmoji-Regular.ttf",    qtrue  },
	};

	for (int i = 0; i < ARRAY_LEN(font_faces); i++)
	{
		Rml_LoadFontFace(va("assets/%s", font_faces[i].filename), font_faces[i].fallback_face);
	}

  //qhandle_t ctx = Rml_CreateContext("default", 640, 480);
	qhandle_t ctx = Rml_CreateContext("default", cls.glconfig.vidWidth, cls.glconfig.vidWidth);
	
	qhandle_t doc = Rml_LoadDocument(ctx, "assets/demo.rml");

	if (doc)
	{
		Rml_ShowDocument(doc);
		Com_Printf("RMLUI: Document loaded: %i\n", ctx);
    cls.rmlStarted = qtrue;
		
		Rml_ContextRender(ctx);

		Rml_ContextUpdate(ctx);
	}
	else
	{
		Com_Printf("RMLUI: Document failed\n");
		Rml_Shutdown();
		cls.rmlStarted = qfalse;
	}
}


void CL_ShutdownRmlUi(void) {
	//if(cls.rmlStarted)
	//	Rml_Shutdown();
}


/*

RmlUiSDL2Renderer Renderer(renderer, screen);

RmlUiSDL2SystemInterface SystemInterface;

// Rml::String root = Shell::FindSamplesRoot();
// ShellFileInterface FileInterface(root);

Rml::SetFileInterface(&FileInterface);
Rml::SetRenderInterface(&Renderer);
Rml::SetSystemInterface(&SystemInterface);

if (!Rml::Initialise())
	return 1;
	struct FontFace {
	Rml::String filename;
	bool fallback_face;
};

FontFace font_faces[] = {
	{ "LatoLatin-Regular.ttf",    false },
	{ "LatoLatin-Italic.ttf",     false },
	{ "LatoLatin-Bold.ttf",       false },
	{ "LatoLatin-BoldItalic.ttf", false },
	{ "NotoEmoji-Regular.ttf",    true  },
};

for (const FontFace& face : font_faces)
{
	Rml::LoadFontFace("assets/" + face.filename, face.fallback_face);
}

	Rml::Context* Context = Rml::CreateContext(,
		Rml::Vector2i(window_width, window_height));

	Rml::ElementDocument* Document = Context->LoadDocument("assets/demo.rml");
			
	if (Document)
	{
		Document->Show();
		fprintf(stdout, "\nDocument loaded");
	}
	else
	{
		fprintf(stdout, "\nDocument is nullptr");
	}

	bool done = false;

	while (!done)
	{
		SDL_Event event;

		SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
		SDL_RenderClear(renderer);

		Context->Render();
		SDL_RenderPresent(renderer);

		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				done = true;
				break;

			case SDL_MOUSEMOTION:
				Context->ProcessMouseMove(event.motion.x, event.motion.y, SystemInterface.GetKeyModifiers());
				break;
			case SDL_MOUSEBUTTONDOWN:
				Context->ProcessMouseButtonDown(SystemInterface.TranslateMouseButton(event.button.button), SystemInterface.GetKeyModifiers());
				break;

			case SDL_MOUSEBUTTONUP:
				Context->ProcessMouseButtonUp(SystemInterface.TranslateMouseButton(event.button.button), SystemInterface.GetKeyModifiers());
				break;

			case SDL_MOUSEWHEEL:
				Context->ProcessMouseWheel(float(event.wheel.y), SystemInterface.GetKeyModifiers());
				break;

			case SDL_KEYDOWN:
			{
				// Intercept F8 key stroke to toggle RmlUi's visual debugger tool
				if (event.key.keysym.sym == SDLK_F8)
				{
					Rml::Debugger::SetVisible(!Rml::Debugger::IsVisible());
					break;
				}

				Context->ProcessKeyDown(SystemInterface.TranslateKey(event.key.keysym.sym), SystemInterface.GetKeyModifiers());
				break;
			}

			default:
				break;
			}
		}
		Context->Update();
	}

	Rml::Shutdown();

*/
#endif
