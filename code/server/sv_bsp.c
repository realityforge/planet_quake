

#include "server.h"
#include "../qcommon/cm_public.h"
#include "../q3map2/q3map2.h"
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


static void AddDrawVertsLump( dheader_t *header ){
	int i, size;
	bspDrawVert_t   *in;
	drawVert_t  *buffer, *out;

	if(dDrawVerts) {
		free(dDrawVerts);
	}

	/* allocate output buffer */
	size = numBSPDrawVerts * sizeof( *buffer );
	buffer = safe_malloc( size );
	memset( buffer, 0, size );

	/* convert */
	in = bspDrawVerts;
	out = buffer;
	for ( i = 0; i < numBSPDrawVerts; i++ )
	{
		VectorCopy( in->xyz, out->xyz );
		out->st[ 0 ] = in->st[ 0 ];
		out->st[ 1 ] = in->st[ 1 ];

		out->lightmap[ 0 ] = in->lightmap[ 0 ][ 0 ];
		out->lightmap[ 1 ] = in->lightmap[ 0 ][ 1 ];

		VectorCopy( in->normal, out->normal );

		out->color[ 0 ] = in->color[ 0 ][ 0 ];
		out->color[ 1 ] = in->color[ 0 ][ 1 ];
		out->color[ 2 ] = in->color[ 0 ][ 2 ];
		out->color[ 3 ] = in->color[ 0 ][ 3 ];

		in++;
		out++;
	}

	dDrawVerts = buffer;
  header->lumps[LUMP_DRAWVERTS].filelen = size;
}


static void AddBrushSidesLump( dheader_t *header )
{
	int i, size;
	bspBrushSide_t  *in;
	dbrushside_t *buffer, *out;

	if(dBrushSides) {
		free(dBrushSides);
	}

	/* allocate output buffer */
	size = numBSPBrushSides * sizeof( *buffer );
	buffer = safe_malloc( size );
	memset( buffer, 0, size );

	/* convert */
	in = bspBrushSides;
	out = buffer;
	for ( i = 0; i < numBSPBrushSides; i++ )
	{
		out->planeNum = in->planeNum;
		out->shaderNum = in->shaderNum;
		in++;
		out++;
	}

	dBrushSides = buffer;
  header->lumps[LUMP_BRUSHSIDES].filelen = size;
}


static void AddDrawSurfacesLump( dheader_t *header ){
	int i, size;
	bspDrawSurface_t    *in;
	dsurface_t   *buffer, *out;

	if(dDrawSurfaces) {
		free(dDrawSurfaces);
	}

	/* allocate output buffer */
	size = numBSPDrawSurfaces * sizeof( *buffer );
	buffer = safe_malloc( size );
	memset( buffer, 0, size );

	/* convert */
	in = bspDrawSurfaces;
	out = buffer;
	for ( i = 0; i < numBSPDrawSurfaces; i++ )
	{
		out->shaderNum = in->shaderNum;
		out->fogNum = in->fogNum;
		out->surfaceType = in->surfaceType;
		out->firstVert = in->firstVert;
		out->numVerts = in->numVerts;
		out->firstIndex = in->firstIndex;
		out->numIndexes = in->numIndexes;

		out->lightmapNum = in->lightmapNum[ 0 ];
		out->lightmapX = in->lightmapX[ 0 ];
		out->lightmapY = in->lightmapY[ 0 ];
		out->lightmapWidth = in->lightmapWidth;
		out->lightmapHeight = in->lightmapHeight;

		VectorCopy( in->lightmapOrigin, out->lightmapOrigin );
		VectorCopy( in->lightmapVecs[ 0 ], out->lightmapVecs[ 0 ] );
		VectorCopy( in->lightmapVecs[ 1 ], out->lightmapVecs[ 1 ] );
		VectorCopy( in->lightmapVecs[ 2 ], out->lightmapVecs[ 2 ] );

		out->patchWidth = in->patchWidth;
		out->patchHeight = in->patchHeight;

		in++;
		out++;
	}

	dDrawSurfaces = buffer;
  header->lumps[LUMP_SURFACES].filelen = size;
}


static void AddLightGridLumps( dheader_t *header ){
	int i;
	bspGridPoint_t  *in;
	byte *buffer, *out;

	if(dGridPoints) {
		free(dGridPoints);
	}

	/* dummy check */
	if ( bspGridPoints == NULL ) {
		return;
	}

	/* allocate temporary buffer */
	buffer = safe_malloc( numBSPGridPoints * sizeof( *out ) * 8 );

	/* convert */
	in = bspGridPoints;
	out = buffer;
	for ( i = 0; i < numBSPGridPoints; i++ )
	{
		VectorCopy( in->ambient[ 0 ], &out[0] );
		VectorCopy( in->directed[ 0 ], &out[3] );

		out[ 6 ] = in->latLong[ 0 ];
		out[ 7 ] = in->latLong[ 1 ];

		in++;
		out++;
	}

	dGridPoints = buffer;
  header->lumps[LUMP_LIGHTGRID].filelen = numBSPGridPoints * sizeof( *out );
}


void SV_LoadMapFromMemory( void ) {
  dheader_t header;
  // TODO: do the same prep that multiworld `load game` command does
  // load all the lumps as if they came from the file
  header.ident = BSP_IDENT;
  header.version = BSP3_VERSION;
  dShaders = (void *)bspShaders;
  header.lumps[LUMP_SHADERS].filelen = numBSPShaders * sizeof( dshader_t );
	dPlanes = (void *)bspPlanes;
  header.lumps[LUMP_PLANES].filelen = numBSPPlanes * sizeof( dplane_t );
	dLeafs = (void *)bspLeafs;
  header.lumps[LUMP_LEAFS].filelen = numBSPLeafs * sizeof( dleaf_t );
	dNodes = (void *)bspNodes;
  header.lumps[LUMP_NODES].filelen = numBSPNodes * sizeof(dnode_t );
	dBrushes = (void *)bspBrushes;
  header.lumps[LUMP_BRUSHES].filelen = numBSPBrushes * sizeof( dbrush_t );
	AddBrushSidesLump(&header);
	dLeafSurfaces = (void *)bspLeafSurfaces;
  header.lumps[LUMP_LEAFSURFACES].filelen = numBSPLeafSurfaces * sizeof( int );
  dLeafBrushes = (void *)bspLeafBrushes;
  header.lumps[LUMP_LEAFBRUSHES].filelen = numBSPLeafBrushes * sizeof( int );
  dModels = (void *)bspModels;
  header.lumps[LUMP_MODELS].filelen = numBSPModels * sizeof( dmodel_t );
	AddDrawVertsLump(&header);
	AddDrawSurfacesLump(&header);
  dVisBytes = bspVisBytes;
  header.lumps[LUMP_VISIBILITY].filelen = numBSPVisBytes;
  dLightBytes = bspLightBytes;
  header.lumps[LUMP_LIGHTMAPS].filelen = numBSPLightBytes;
	AddLightGridLumps(&header);
  dEntData = (void *)bspEntData;
  header.lumps[LUMP_ENTITIES].filelen = bspEntDataSize;
  dFogs = (void *)bspFogs;
  header.lumps[LUMP_FOGS].filelen = numBSPFogs * sizeof( dfog_t );
  dDrawIndexes = (void *)bspDrawIndexes;
  header.lumps[LUMP_DRAWINDEXES].filelen = numBSPDrawIndexes * sizeof( int );

	CM_LoadMapFromMemory(&header);

	// load into heap
	CMod_LoadShaders( &header.lumps[LUMP_SHADERS] );
	CMod_LoadLeafs (&header.lumps[LUMP_LEAFS]);
	CMod_LoadLeafBrushes (&header.lumps[LUMP_LEAFBRUSHES]);
	CMod_LoadLeafSurfaces (&header.lumps[LUMP_LEAFSURFACES]);
	CMod_LoadPlanes (&header.lumps[LUMP_PLANES]);
	CMod_LoadBrushSides (&header.lumps[LUMP_BRUSHSIDES]);
	CMod_LoadBrushes (&header.lumps[LUMP_BRUSHES]);
	CMod_LoadSubmodels (&header.lumps[LUMP_MODELS]);
	CMod_LoadNodes (&header.lumps[LUMP_NODES]);
	CMod_LoadEntityString (&header.lumps[LUMP_ENTITIES], "\0");
	CMod_LoadVisibility( &header.lumps[LUMP_VISIBILITY] );
	CMod_LoadPatches( &header.lumps[LUMP_SURFACES], &header.lumps[LUMP_DRAWVERTS] );
	
	/* advertisements */
	//AddLump( file, (bspHeader_t*) header, LUMP_ADVERTISEMENTS, bspAds, numBSPAds * sizeof( bspAdvertisement_t ) );

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
  //EndBSPFile();
  SV_LoadMapFromMemory();
}
