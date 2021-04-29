

#include "server.h"
#include "../q3map2/bspfile.h"



static char skybox[4096*1024];
char *SV_MakeSkybox( void ) {
	vec3_t  vs[2];
	if(!com_sv_running || !com_sv_running->integer) {
		vs[0][0] = vs[0][1] = vs[0][2] = -1000;
		vs[1][0] = vs[1][1] = vs[1][2] = 1000;
	} else {
		int h = CM_InlineModel( 0, 2, gvm );
		CM_ModelBounds( h, vs[0], vs[1] );
	}

	int  points[12][3] = {
		{vs[0][0], vs[0][1], vs[0][2]-16},
		{vs[1][0], vs[1][1], vs[0][2]},
		
		{vs[0][0]-16, vs[0][1], vs[0][2]},
		{vs[0][0],    vs[1][1], vs[1][2]},
		
		{vs[0][0], vs[0][1]-16, vs[0][2]},
		{vs[1][0], vs[0][1],    vs[1][2]},
		
		
		{vs[0][0], vs[0][1], vs[1][2]},
		{vs[1][0], vs[1][1], vs[1][2]+16},
		
		{vs[1][0],    vs[0][1], vs[0][2]},
		{vs[1][0]+16, vs[1][1], vs[1][2]},
		
		{vs[0][0], vs[1][1],    vs[0][2]},
		{vs[1][0], vs[1][1]+16, vs[1][2]}
	};

	Q_strcat(skybox, sizeof(skybox), "{\n"
		"\"classname\" \"worldspawn\"\n");

	for(int i = 0; i < 6; i++) {
		int *p1 = points[i*2];
		int *p2 = points[i*2+1];
		Q_strcat(skybox, sizeof(skybox), "{\n");
		Q_strcat(skybox, sizeof(skybox),
			va("( %i %i %i ) ( %i %i %i ) ( %i %i %i ) e1u1/sky1 0 0 0 1 1 0 0 0\n",
			p1[0], p1[1], p2[2], p1[0], p1[1], p1[2], p1[0], p2[1], p1[2]
		));
		Q_strcat(skybox, sizeof(skybox),
			va("( %i %i %i ) ( %i %i %i ) ( %i %i %i ) e1u1/sky1 0 0 0 1 1 0 0 0\n",
			p2[0], p2[1], p2[2], p2[0], p2[1], p1[2], p2[0], p1[1], p1[2]
		));
		Q_strcat(skybox, sizeof(skybox),
			va("( %i %i %i ) ( %i %i %i ) ( %i %i %i ) e1u1/sky1 0 0 0 1 1 0 0 0\n",
			p2[0], p1[1], p2[2], p2[0], p1[1], p1[2], p1[0], p1[1], p1[2]
		));
		Q_strcat(skybox, sizeof(skybox),
			va("( %i %i %i ) ( %i %i %i ) ( %i %i %i ) e1u1/sky1 0 0 0 1 1 0 0 0\n",
			p1[0], p2[1], p2[2], p1[0], p2[1], p1[2], p2[0], p2[1], p1[2]
		));
		Q_strcat(skybox, sizeof(skybox),
			va("( %i %i %i ) ( %i %i %i ) ( %i %i %i ) e1u1/sky1 0 0 0 1 1 0 0 0\n",
			p1[0], p2[1], p1[2], p1[0], p1[1], p1[2], p2[0], p1[1], p1[2]
		));
		Q_strcat(skybox, sizeof(skybox),
			va("( %i %i %i ) ( %i %i %i ) ( %i %i %i ) e1u1/sky1 0 0 0 1 1 0 0 0\n",
			p1[0], p1[1], p2[2], p1[0], p2[1], p2[2], p2[0], p2[1], p2[2]
		));
		Q_strcat(skybox, sizeof(skybox), "}\n");
	}
	
	Q_strcat(skybox, sizeof(skybox), "}\n"
		"{\n"
		"\"classname\" \"info_player_start\"\n"
		"\"origin\" \"16 64 -52\"\n"
		"}\n");

	return (char *)skybox;
}


void SV_LoadMapFromMemory( void ) {
  dheader_t header;
  // do the same prep that multiworld `load game` command does
  
  // pick the next clipmap slot
  
  // load all the lumps as if they came from the file
Com_Printf("MakeMap: %i\n", numShaders);
  header.ident = BSP_IDENT;
  header.version = bsp_version;
  header.lumps[LUMP_SHADERS].fileofs = (uint)&dshaders - (uint)&header;
  header.lumps[LUMP_SHADERS].filelen = numShaders * sizeof( dshader_t );
  header.lumps[LUMP_PLANES].fileofs = (uint)&dplanes - (uint)&header;
  header.lumps[LUMP_PLANES].filelen = numplanes * sizeof( dplane_t );
  header.lumps[LUMP_LEAFS].fileofs = (uint)&dleafs - (uint)&header;
  header.lumps[LUMP_LEAFS].filelen = numleafs * sizeof( dleaf_t );
  header.lumps[LUMP_NODES].fileofs = (uint)&dnodes - (uint)&header;
  header.lumps[LUMP_NODES].filelen = numnodes * sizeof( dnode_t );
  header.lumps[LUMP_BRUSHES].fileofs = (uint)&dbrushes - (uint)&header;
  header.lumps[LUMP_BRUSHES].filelen = numbrushes * sizeof( dbrush_t );
  header.lumps[LUMP_BRUSHSIDES].fileofs = (uint)&dbrushsides - (uint)&header;
  header.lumps[LUMP_BRUSHSIDES].filelen = numbrushsides * sizeof( dbrushside_t );
  header.lumps[LUMP_LEAFSURFACES].fileofs = (uint)&dleafsurfaces - (uint)&header;
  header.lumps[LUMP_LEAFSURFACES].filelen = numleafsurfaces * sizeof( dleafsurfaces[0] );
  header.lumps[LUMP_LEAFBRUSHES].fileofs = (uint)&dleafbrushes - (uint)&header;
  header.lumps[LUMP_LEAFBRUSHES].filelen = numleafbrushes * sizeof( dleafbrushes[0] );
  header.lumps[LUMP_MODELS].fileofs = (uint)&dmodels - (uint)&header;
  header.lumps[LUMP_MODELS].filelen = nummodels * sizeof( dmodel_t );
  header.lumps[LUMP_DRAWVERTS].fileofs = (uint)&drawVerts - (uint)&header;
  header.lumps[LUMP_DRAWVERTS].filelen = numDrawVerts * sizeof( drawVert_t );
  header.lumps[LUMP_SURFACES].fileofs = (uint)&drawSurfaces - (uint)&header;
  header.lumps[LUMP_SURFACES].filelen = numDrawSurfaces * sizeof( dsurface_t );
  header.lumps[LUMP_VISIBILITY].fileofs = (uint)&visBytes - (uint)&header;
  header.lumps[LUMP_VISIBILITY].filelen = numVisBytes;
  header.lumps[LUMP_LIGHTMAPS].fileofs = (uint)&lightBytes - (uint)&header;
  header.lumps[LUMP_LIGHTMAPS].filelen = numLightBytes;
  header.lumps[LUMP_LIGHTGRID].fileofs = (uint)&gridData - (uint)&header;
  header.lumps[LUMP_LIGHTGRID].filelen = 8 * numGridPoints;
  header.lumps[LUMP_ENTITIES].fileofs = (uint)&dentdata - (uint)&header;
  header.lumps[LUMP_ENTITIES].filelen = entdatasize;
  header.lumps[LUMP_FOGS].fileofs = (uint)&dfogs - (uint)&header;
  header.lumps[LUMP_FOGS].filelen = numFogs * sizeof( dfog_t );
  header.lumps[LUMP_DRAWINDEXES].fileofs = (uint)&drawIndexes - (uint)&header;
  header.lumps[LUMP_DRAWINDEXES].filelen = numDrawIndexes * sizeof( drawIndexes[0] );

  CM_LoadMapFromMemory(&header);
    
  // TODO: detect memory map from gamestate on client and download over UDP
  
  // TODO: make a copy of the map in memory in case a client requests, it can be sent

}



void SV_MakeMap( void ) {
	qboolean onlyents = qfalse;

	/* init model library */
	PicoInit();
	PicoSetMallocFunc( safe_malloc );
	PicoSetFreeFunc( free );
	PicoSetPrintFunc( PicoPrintFunc );
	PicoSetLoadFileFunc( PicoLoadFileFunc );
	PicoSetFreeFileFunc( free );

	/* set number of threads */
	ThreadSetDefault();

	/* generate sinusoid jitter table */
	for ( int i = 0; i < MAX_JITTERS; i++ )
	{
		jitters[ i ] = sin( i * 139.54152147 );
		//%	Sys_Printf( "Jitter %4d: %f\n", i, jitters[ i ] );
	}

	/* ydnar: new path initialization */
	// TODO: InitPaths( &argc, argv );

	/* note it */
	Com_Printf( "--- BSP ---\n" );

	SetDrawSurfacesBuffer();
	mapDrawSurfs = safe_malloc( sizeof( mapDrawSurface_t ) * MAX_MAP_DRAW_SURFS );
	memset( mapDrawSurfs, 0, sizeof( mapDrawSurface_t ) * MAX_MAP_DRAW_SURFS );
	numMapDrawSurfs = 0;

	/* set standard game flags */
	maxSurfaceVerts = game->maxSurfaceVerts;
	maxSurfaceIndexes = game->maxSurfaceIndexes;
	emitFlares = game->emitFlares;
	
	//game->shaderPath = "/Applications/ioquake3/baseq3/scripts";
	meta = qtrue;

	/* ydnar: set default sample size */
	SetDefaultSampleSize( sampleSize );

	/* if onlyents, just grab the entites and resave */
	if ( onlyents ) {
		OnlyEnts();
		return;
	}

	/* load shaders */
	LoadShaderInfo();

	char *skybox = SV_MakeSkybox();
  
	strcpy( source, "*memory" ); // give the map a special name so clients can download it

	LoadMap( skybox, qfalse );

	/* ydnar: decal setup */
	ProcessDecals();

	/* ydnar: cloned brush model entities */
	SetCloneModelNumbers();

	/* process world and submodels */
	ProcessModels();

	/* set light styles from targetted light entities */
	SetLightStyles();

	/* process in game advertisements */
	ProcessAdvertisements();

	/* finish and write bsp */
  EndBSPFile();
  //SV_LoadMapFromMemory();
}
