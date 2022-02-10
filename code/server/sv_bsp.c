#include "server.h"

#ifdef USE_MEMORY_MAPS

#ifdef _DEBUG
#ifndef _WIN32
#ifndef  __WASM__
#include <execinfo.h>
#endif
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#endif

#include <setjmp.h>

static jmp_buf q3map2done;	// an ERR_DROP occurred, exit the entire frame

#include "../qcommon/vm_local.h"
#include "../qcommon/cm_public.h"
#include "../game/bg_public.h"

#include "../qcommon/cm_local.h"

char *FS_RealPath(const char *localPath);

char stroke[MAX_QPATH] = "";

char output[2 * 1024 * 1024] = ""; // 2MB TODO: make alloc and optional
int brushC = 0;

void SV_SetStroke( const char *path ) {
	memcpy(stroke, path, sizeof(stroke));
}



// TODO: wall is just a square platform
//static char *SV_MakePlatform(vec3_t p1, vec3_t p2, vec3_t p3, vec3_t p4) {
//	static char plat[4096];
	// TODO: make nice with edges like Edge of Oblivion on quake 3 space maps
//	return plat;
//}


/*
static void SV_FlipHorizontal(vec3_t *flip) {
  int temp0 = flip[0][0];
  int temp1 = flip[1][0];
  flip[0][0] = flip[3][0];
  flip[1][0] = flip[2][0];
  flip[2][0] = temp1;
  flip[3][0] = temp0;
  temp0 = flip[0][2];
  temp1 = flip[1][2];
  flip[0][2] = flip[3][2];
  flip[1][2] = flip[2][2];
  flip[2][2] = temp1;
  flip[3][2] = temp0;
}


static void SV_FlipVertical(vec3_t *flip) {
  int temp0 = flip[0][1];
  int temp3 = flip[3][1];
  flip[0][1] = flip[1][1];
  flip[3][1] = flip[2][1];
  flip[1][1] = temp0;
  flip[2][1] = temp3;
  temp0 = flip[0][2];
  temp3 = flip[3][2];
  flip[0][2] = flip[1][2];
  flip[3][2] = flip[2][2];
  flip[1][2] = temp0;
  flip[2][2] = temp3;
}
*/



int SV_MakeHypercube( void );
int SV_MakeAtlantis( void );
int SV_MakeMaze( void );
int SV_MakeChutesAndLadders( void );
int SV_MakeMonacoF1( void );


void SV_SpliceBSP(const char *memoryMap, const char *altName);

extern int Q3MAP2Main( int argc, char **argv );

static void SV_MapError(const char *error) {
	Com_Printf("Q3MAP2 ERROR: %s\n", error);
#ifdef _DEBUG
#if defined(__linux__) || defined(__APPLE__)
	{
		void *syms[10];
		const size_t size = backtrace( syms, ARRAY_LEN( syms ) );
		backtrace_symbols_fd( syms, size, STDERR_FILENO );
	}
#endif // linux
#endif // win32

	Q_longjmp(q3map2done, 1); // unwind everything in C, TODO: shutdown q3map2?
}

static FILE *SV_OpenWrite(const char *path) {
	// sometimes q3map2 will all process directory
	printf("writing: %s\n", path);
	if(Q_stristr(path, Sys_Pwd())) {
		path += strlen(Sys_Pwd()) + 1;
	}
	return Sys_FOpen(FS_BuildOSPath(Cvar_VariableString("fs_homepath"), FS_GetCurrentGameDir(), path), "wb");
}

static FILE *SV_OpenRead(const char *path) {
	// sometimes q3map2 will all process directory
	if(Q_stristr(path, Sys_Pwd())) {
		path += strlen(Sys_Pwd()) + 1;
	}
	return Sys_FOpen(FS_BuildOSPath(Cvar_VariableString("fs_homepath"), FS_GetCurrentGameDir(), path), "rb");
}

static int SV_ReadFile(const char *path, void **buffer) {
	// sometimes q3map2 will all process directory
	if(Q_stristr(path, Sys_Pwd())) {
		path += strlen(Sys_Pwd()) + 1;
	}
	return FS_ReadFile(path, buffer);
}

void SV_ExportMap(void) {
	static char strippedName[MAX_QPATH];
	if(cm.name[0] == '\0') {
		return;
	}

	if ( Q_setjmp( q3map2done ) ) {
		return;			// an ERR_DROP was thrown
	}
	// someone extracted the bsp file intentionally?
	// don't do it if we already extracted
	// if( FS_RealPath( va("maps/%s_converted.map", memoryMap) ) {
	//	return;
	//}
	Sys_SetStatus("Building map %s", cm.name);
	Com_Printf("Building map %s\n", cm.name);

	COM_StripExtension(cm.name, strippedName, sizeof(strippedName));
	// touch the file so q3map2 can definitely access it
	FS_SV_FOpenFileWrite(va("%s/%s_converted.map", FS_GetCurrentGameDir(), strippedName));

	Cvar_Set( "buildingMap", cm.name );
	char *compileMap[] = {
		"q3map2",
		"-v",
		"-error",
		(char *)SV_MapError,
		"-fs_basepath",
		(char *)Cvar_VariableString("fs_basepath"),
		"-game",
		"quake3",
		"-fs_game",
		(char *)FS_GetCurrentGameDir(),
		"-vfs",
		(char *)SV_ReadFile,
		(char *)SV_OpenWrite,
		(char *)SV_OpenRead,
		"-convert",
		"-keeplights",
		"-format",
		"map",
		// "-readmap", // TODO: use for normalizing mbspc bsp2map conversions
		cm.name
	};
	Q3MAP2Main(ARRAY_LEN(compileMap), compileMap);
	Cvar_Set( "buildingMap", "" );
}


void SV_LightMap(void) {
	int length;
	if(cm.name[0] == '\0') {
		return;
	}

	if((length = FS_FOpenFileRead( va("maps/%s/lm_0000.tga", cm.name), NULL, qtrue )) != -1) {
		return;
	}
	// then we can decide not to update LM?
	char *compileLight[] = {
		"q3map2",
		"-v",
		"-error",
		(char *)SV_MapError,
		"-light",
		"-external",
		"-fs_basepath",
		(char *)Cvar_VariableString("fs_basepath"),
		"-game",
		"quake3",
		"-fs_game",
		(char *)FS_GetCurrentGameDir(),
		"-vfs",
		(char *)SV_ReadFile,
		(char *)SV_OpenWrite,
		(char *)SV_OpenRead,
		"-fast",
		"-patchshadows",
		//"-gridsize",
		//"512.0 512.0 512.0",
		"-bounce",
		"2", // really decent lighting, but not fast enough
		//"-bouncegrid",
		//"-trisoup",
		"-samplesize",
		"8",
		cm.name
	};
	Q3MAP2Main(ARRAY_LEN(compileLight), compileLight);
}


void SV_MakeBSP(char *memoryMap) {
	char *mapname = (char *)va("maps/%s.map", memoryMap);
	// no bsp file exists, try to make one, check for .map file
	if(!FS_RealPath( mapname )) {
		return;
	}

	Cvar_Set( "buildingMap", memoryMap );
	char *compileMeta[] = {
		"q3map2",
		"-meta",
		"-v",
		"-error",
		(char *)SV_MapError,
		"-fs_basepath",
		(char *)Cvar_VariableString("fs_basepath"),
		"-vfs",
		(char *)SV_ReadFile,
		(char *)SV_OpenWrite,
		(char *)SV_OpenRead,
		"-game",
		"quake3",
		"-fs_game",
		(char *)FS_GetCurrentGameDir(),
		//"-patchmeta", // makes compile  much slower
		"-keeplights",
		mapname
	};
	Q3MAP2Main(ARRAY_LEN(compileMeta), compileMeta);

	/*
	char *compileVis[] = {
		"q3map2",
		"-v",
		"-fs_basepath",
		(char *)Cvar_VariableString("fs_basepath"),
		"-game",
		"quake3",
		"-fs_game",
		(char *)FS_GetCurrentGameDir(),
		"-vis",
		mapPath
	};
	Q3MAP2Main(ARRAY_LEN(compileVis), compileVis);
	*/

	char *compileLight[] = {
		"q3map2",
		"-light",
		"-v",
		"-error",
		(char *)SV_MapError,
		"-fs_basepath",
		(char *)Cvar_VariableString("fs_basepath"),
		"-vfs",
		(char *)SV_ReadFile,
		(char *)SV_OpenWrite,
		(char *)SV_OpenRead,
		"-game",
		"quake3",
		"-fs_game",
		(char *)FS_GetCurrentGameDir(),
		"-faster",
		"-cheap",
		"-nolm",
		"-notrace",
		"-samplesize",
		"128",
		mapname
	};
	Q3MAP2Main(ARRAY_LEN(compileLight), compileLight);
}


int SV_MakeMap( const char **map ) {
	static char memoryMap[MAX_QPATH];
	char *mapPath;
  fileHandle_t mapfile;
	int length = 0;
	Q_strncpyz( memoryMap, *map, sizeof(memoryMap) );

	if ( Q_setjmp( q3map2done ) ) {
		return 0;			// an ERR_DROP was thrown
	}

	// early exit unless we force rebuilding
	mapPath = FS_RealPath( va("maps/%s.bsp", memoryMap) );
	if(!sv_bspRebuild->integer && mapPath) {
		return 1;
	}

	brushC = 0;
	if(Q_stricmp(memoryMap, "megamaze") == 0) {
		length = SV_MakeMaze();
	} else if (Q_stricmp(memoryMap, "megacube") == 0) {
		length = SV_MakeHypercube();
	} else if (Q_stricmp(memoryMap, "megachutes") == 0) {
		length = SV_MakeChutesAndLadders();
	} else if (Q_stricmp(memoryMap, "megaf1") == 0) {
		length = SV_MakeMonacoF1();
	} else if (Q_stricmp(memoryMap, "megalantis") == 0) {
		length = SV_MakeAtlantis();
	} else if (Q_stricmp(memoryMap, "test") == 0) {
	}
  
	if(length) {
		// TODO: overwrite if make-time is greater than 1 min?
		// always writes to home directory
		mapfile = FS_FOpenFileWrite( va("maps/%s.map", memoryMap) );
		FS_Write( output, length, mapfile );    // overwritten later
		FS_FCloseFile( mapfile );
		mapPath = FS_BuildOSPath( Cvar_VariableString("fs_homepath"), 
    Cvar_VariableString("fs_game"), va("maps/%s.map", memoryMap) );
	}

	// TODO: use virtual_fs to load .map from pk3, more of a q3map2 problem
	// TODO: detect prior formats and decompile it to .map
	// TODO: compare file time with BSP
	// TODO: check if lightmap exists and compare with BSP time
	// TODO: copy to home directory?
	// TODO: need an API to get exact paths?

  //gamedir = Cvar_VariableString( "fs_game" );
	//basegame = Cvar_VariableString( "fs_basegame" );
	// if there is no map file for it, try to make one!
	if(sv_bspMap->integer) {
		SV_ExportMap();
	}
	
	if(sv_bspSplice->string[0] != '\0') {
		if(sv_bspMap->integer && mapPath) {
			SV_SpliceBSP(va("maps/%s_converted.map", memoryMap), va("maps/%s_spliced.map", memoryMap));
		} else {
			SV_SpliceBSP(va("maps/%s.map", memoryMap), va("maps/%s_spliced.map", memoryMap));
		}
		// force the rest to rebuild the bsp
		// TODO: make optional to spawn in the spliced map
		Q_strncpyz( memoryMap, va("%s_spliced", memoryMap), sizeof(memoryMap) );
		*map = memoryMap;
		mapPath = NULL;
	}

	// TODO: sv_bspScale that resizes a map like backyard/kitchen/billiardroom/etc
	//   into small regular size rooms, and opposite, making small maps giant

	// TODO: add levelshot camera location
	
	if (sv_bspRebuild->integer || !mapPath) {
		SV_MakeBSP(memoryMap);
	}

	if(sv_bspLight->integer
		|| sv_bspSplice->string[0] != '\0' // always light spliced maps
	) {
		SV_LightMap();
	}

	// TODO: generate AAS file for bots, missing, or updated maps
	if(sv_bspAAS->integer
		&& (length = FS_FOpenFileRead( va("maps/%s.aas", memoryMap), NULL, qtrue )) == -1) {
		
	}

  Cvar_Set( "buildingMap", "" );
	return length;
}

#endif
