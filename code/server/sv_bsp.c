#include "server.h"

#ifdef USE_MEMORY_MAPS

#include "../qcommon/vm_local.h"
#include "../qcommon/cm_public.h"
#include "../game/bg_public.h"

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

int SV_MakeMap( const char **map ) {
	static char memoryMap[MAX_QPATH];
	char *mapPath;
	char *bspPath;
  fileHandle_t mapfile;
	int length = 0;
	Q_strncpyz( memoryMap, *map, sizeof(memoryMap) );

	// early exit unless we force rebuilding
	if(!sv_bspRebuild->integer && FS_RealPath( va("maps/%s.bsp", memoryMap) )) {
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
	mapPath = FS_RealPath( va("maps/%s.bsp", memoryMap) );
	if(sv_bspMap->integer && mapPath
		// don't do it if we already extracted
		// && FS_RealPath( va("maps/%s_converted.map", memoryMap) )
	) {
		// someone extracted the bsp file intentionally?
		Cvar_Set( "buildingMap", memoryMap );
		char *compileMap[] = {
			"q3map2",
			"-v",
			"-fs_basepath",
			(char *)Cvar_VariableString("fs_basepath"),
			"-game",
			"quake3",
			"-fs_game",
			(char *)FS_GetCurrentGameDir(),
			"-convert",
			"-keeplights",
			"-format",
			"map",
			// "-readmap", // TODO: use for normalizing mbspc bsp2map conversions
			mapPath
		};
		Q3MAP2Main(ARRAY_LEN(compileMap), compileMap);
		Cvar_Set( "buildingMap", "" );
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
	
	if (!mapPath) {
		// no bsp file exists, try to make one, check for .map file
		mapPath = FS_RealPath( va("maps/%s.map", memoryMap) );
		if(sv_bspRebuild->integer && mapPath) {
			Cvar_Set( "buildingMap", memoryMap );
			char *compileMeta[] = {
				"q3map2",
				"-meta",
				"-v",
				"-fs_basepath",
				(char *)Cvar_VariableString("fs_basepath"),
				"-game",
				"quake3",
				"-fs_game",
				(char *)FS_GetCurrentGameDir(),
		    //"-patchmeta", // makes compile  much slower
				"-keeplights",
				mapPath
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
				"-fs_basepath",
				(char *)Cvar_VariableString("fs_basepath"),
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
				mapPath
			};
			Q3MAP2Main(ARRAY_LEN(compileLight), compileLight);
		}
	}

	if(sv_bspLight->integer
		&& ((length = FS_FOpenFileRead( va("maps/%s/lm_0000.tga", memoryMap), NULL, qtrue )) == -1
		|| sv_bspSplice->string[0] != '\0') // always light spliced maps
	) {
		// then we can decide not to update LM?
		bspPath = FS_RealPath( va("maps/%s.bsp", memoryMap) );
		char *compileLight[] = {
				"q3map2",
				"-light",
				"-external",
				"-fs_basepath",
				(char *)Cvar_VariableString("fs_basepath"),
				"-game",
				"quake3",
				"-fs_game",
				(char *)FS_GetCurrentGameDir(),
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
				bspPath
			};
			Q3MAP2Main(ARRAY_LEN(compileLight), compileLight);
	}


	if (sv_bspMinimap->integer) {
#if 0
		// no bsp file exists, try to make one, check for .map file
		mapPath = FS_RealPath( va("maps/%s.map", memoryMap) );
		if(sv_bspRebuild->integer && mapPath) {
			Cvar_Set( "buildingMap", memoryMap );
			char *compileMap[] = {
				"q3map2",
				"-v",
				"-fs_basepath",
				(char *)Cvar_VariableString("fs_basepath"),
				"-game",
				"quake3",
				"-fs_game",
				(char *)FS_GetCurrentGameDir(),
				"-minimap",
				mapPath
			};
			Q3MAP2Main(ARRAY_LEN(compileMap), compileMap);
		}
#else
		//MiniMapBSPMain(va("maps/%s.bsp", memoryMap));
#endif
	}


	// TODO: generate AAS file for bots, missing, or updated maps
	if(sv_bspAAS->integer
		&& (length = FS_FOpenFileRead( va("maps/%s.aas", memoryMap), NULL, qtrue )) == -1) {
		
	}

  Cvar_Set( "buildingMap", "" );
	return length;
}

#endif
