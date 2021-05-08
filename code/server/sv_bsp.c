
#ifdef USE_MEMORY_MAPS

#include "server.h"
#include "../qcommon/cm_public.h"
#define MAIN_C
#include "../tools/q3map2/q3map2.h"
#undef MAIN_C



static dheader_t header;
static char skybox[4096*4096];
static int brushC = 0;

static char *SV_MakeWall( int p1[3], int p2[3] ) {
	static char wall[4096];
	int minMaxMap[6][3][3] = {
		{{p1[0], p1[1], p2[2]}, {p1[0], p1[1], p1[2]}, {p1[0], p2[1], p1[2]}},
		{{p2[0], p2[1], p2[2]}, {p2[0], p2[1], p1[2]}, {p2[0], p1[1], p1[2]}},
		{{p2[0], p1[1], p2[2]}, {p2[0], p1[1], p1[2]}, {p1[0], p1[1], p1[2]}},
		{{p1[0], p2[1], p2[2]}, {p1[0], p2[1], p1[2]}, {p2[0], p2[1], p1[2]}},
		{{p1[0], p2[1], p1[2]}, {p1[0], p1[1], p1[2]}, {p2[0], p1[1], p1[2]}},
		{{p1[0], p1[1], p2[2]}, {p1[0], p2[1], p2[2]}, {p2[0], p2[1], p2[2]}},
	};
	wall[0] = '\0';
	brushC++;
	Q_strcat(wall, sizeof(wall), va("// brush %i\n"
		"{\n", brushC));
	for(int i = 0; i < ARRAY_LEN(minMaxMap); i++) {
		Q_strcat(wall, sizeof(wall),
			va("( %i %i %i ) ( %i %i %i ) ( %i %i %i ) e1u1/sky10 0 0 0 1 1 0 0 0\n",
			minMaxMap[i][0][0], minMaxMap[i][0][1], minMaxMap[i][0][2],
			minMaxMap[i][1][0], minMaxMap[i][1][1], minMaxMap[i][1][2],
			minMaxMap[i][2][0], minMaxMap[i][2][1], minMaxMap[i][2][2]
		));
	}
	Q_strcat(wall, sizeof(wall), "}\n");
	return wall;
}


static char *SV_MakeBox( vec3_t min, vec3_t max ) {
	static char box[4096*1024];
	box[0] = '\0';	
	int  wallMap[12][3] = {
		{min[0], min[1], min[2]-16},
		{max[0], max[1], min[2]},
		
		{min[0]-16, min[1], min[2]},
		{min[0],    max[1], max[2]},
		
		{min[0], min[1]-16, min[2]},
		{max[0], min[1],    max[2]},
		
		
		{min[0], min[1], max[2]},
		{max[0], max[1], max[2]+16},
		
		{max[0],    min[1], min[2]},
		{max[0]+16, max[1], max[2]},
		
		{min[0], max[1],    min[2]},
		{max[0], max[1]+16, max[2]}
	};

	//Q_strcat(skybox, sizeof(skybox), "{\n"
	//	"\"classname\" \"func_group\"\n");

	for(int i = 0; i < 6; i++) {
		int *p1 = wallMap[i*2];
		int *p2 = wallMap[i*2+1];
		Q_strcat(box, sizeof(box), SV_MakeWall(p1, p2));
	}

	//Q_strcat(skybox, sizeof(skybox), "}\n");

	return box;
}


static char *SV_MakeSkybox( void ) {
	vec3_t  vs[2];
	if(!com_sv_running || !com_sv_running->integer
		|| sv.state != SS_GAME) {
		vs[0][0] = vs[0][1] = vs[0][2] = -2000;
		vs[1][0] = vs[1][1] = vs[1][2] = 2000;
	} else {
		int h = CM_InlineModel( 0, 2, gvm );
		CM_ModelBounds( h, vs[0], vs[1] );
	}

	brushC = 0;
	skybox[0] = '\0';
	Q_strcat(skybox, sizeof(skybox), "// Game: Quake 3\n"
		"// Format: Quake3 (legacy)\n"
		"// entity 0\n"
		"{\n"
		"\"classname\" \"worldspawn\"\n");
	
	Q_strcat(skybox, sizeof(skybox), SV_MakeBox(vs[0], vs[1]));
	
	Q_strcat(skybox, sizeof(skybox), "}\n");
	
	Q_strcat(skybox, sizeof(skybox), 
		"{\n"
		"\"classname\" \"info_player_start\"\n"
		"\"origin\" \"16 64 -52\"\n"
		"}\n");

	return skybox;
}


// TODO
static char *SV_MakeMaze( void ) {
	return "";
}


// TODO: wall is just a square platform
static char *SV_MakePlatform(vec3_t p1, vec3_t p2, vec3_t p3, vec3_t p4) {
	static char plat[4096];
	int quadMap[6][3][3] = {
		{{p4[0], p4[1], p4[2]+16}, {p1[0], p1[1], p1[2]+16}, {p1[0], p1[1], p1[2]}},
		{{p4[0], p4[1], p4[2]},    {p3[0], p3[1], p3[2]},    {p3[0], p3[1], p3[2]+16}},
		{{p4[0], p4[1], p4[2]+16}, {p3[0], p3[1], p3[2]+16}, {p2[0], p2[1], p2[2]+16}},
		{{p2[0], p2[1], p2[2]},    {p3[0], p3[1], p3[2]},    {p4[0], p4[1], p4[2]}},
		{{p2[0], p2[1], p2[2]+16}, {p2[0], p2[1], p2[2]},    {p1[0], p1[1], p1[2]}},
		{{p3[0], p3[1], p3[2]+16}, {p3[0], p3[1], p3[2]},    {p2[0], p2[1], p2[2]}},
	};
	plat[0] = '\0';
	brushC++;
	Q_strcat(plat, sizeof(plat), va("// brush %i\n"
		"{\n", brushC));
	for(int i = 0; i < ARRAY_LEN(quadMap); i++) {
		Q_strcat(plat, sizeof(plat),
			va("( %i %i %i ) ( %i %i %i ) ( %i %i %i ) e1u1/sky10 0 0 0 1 1 0 0 0\n",
			quadMap[i][0][0], quadMap[i][0][1], quadMap[i][0][2],
			quadMap[i][1][0], quadMap[i][1][1], quadMap[i][1][2],
			quadMap[i][2][0], quadMap[i][2][1], quadMap[i][2][2]
		));
	}
	Q_strcat(plat, sizeof(plat), "}\n");
	return plat;
}


static char *SV_MakePortal( float radius, int x, int y, int width, int height ) {
	static char portal[4096*1024];
	float splits = 18.0;
	//splits = ((int)ceil(sqrt(splits))) ^ 2;
	float angle = 360.0 / splits;
	float padX = (width - radius * 2) / 2;
	float padY = (height - radius * 2) / 2;
	float splitsPerSide = splits / 4.0; // sides
	int offset = floor(splits / 8.0);
	portal[0] = '\0';
	for(int i = -offset; i < splits - offset; i++) {
		// alternate which corners are used to form the circle as it goes around, this keeps the brush uniform
		// start from the top left corner
		int x1 = x + radius * sin(M_PI * 2 * (angle * (i - 0)) / 360.0);
		int y1 = y - radius * (1.0 - cos(M_PI * 2 * (angle * (i - 0)) / 360.0)) + padY;
		int x2 = x + radius * sin(M_PI * 2 * (angle * (i + 1)) / 360.0);
		int y2 = y - radius * (1.0 - cos(M_PI * 2 * (angle * (i + 1)) / 360.0)) + padY;
		int x3 = x + (width / splitsPerSide) * (i + 1);
		int y3 = y + (height / 2);
		int x4 = x + (width / splitsPerSide) * i;
		int y4 = y + (height / 2);
		
		if(i > (splitsPerSide * 3.0 - offset)) {
			x3 = x2;
			y3 = y2;
			x2 = x1;
			y2 = y1;
			x1 = x - (width / 2);
			y1 = y + (height / splitsPerSide) * (i - (splitsPerSide * 3 + 0));
			x4 = x - (width / 2);
			y4 = y + (height / splitsPerSide) * (i - (splitsPerSide * 3 - 1));
		} else if(i > (splitsPerSide * 2.0 - offset)) {
			x3 = x1;
			y3 = y1;
			x4 = x2;
			y4 = y2;
			x1 = x + (width / splitsPerSide) * (splitsPerSide * 2 - 1 - i);
			y1 = y - (height / 2);
			x2 = x + (width / splitsPerSide) * (splitsPerSide * 2 - i);
			y2 = y - (height / 2);
		} else if(i > (splitsPerSide - offset)) {
			x4 = x1;
			y4 = y1;
			x1 = x2;
			y1 = y2;
			x3 = x + (width / 2);
			y3 = y + (height / splitsPerSide) * (splitsPerSide - i);
			x2 = x + (width / 2);
			y2 = y + (height / splitsPerSide) * (splitsPerSide - 1 - i);
		} else {
		}
		Q_strcat(portal, sizeof(portal), SV_MakePlatform(
			(vec3_t){x1, y1, 500}, (vec3_t){x2, y2, 500}, (vec3_t){x3, y3, 500}, (vec3_t){x4, y4, 500}
		));
	}
	return portal;
}


static char *SV_MakeHypercube( void ) {
	int width = 400;
	int height = 400;
	int spacing = 300;
	int rows = 3;
	int cols = 3;
	int totalWidth = width * cols + spacing * (cols - 1);
	int totalHeight = height * rows + spacing * (rows - 1);
	vec3_t  vs[2];

	vs[0][0] = vs[0][1] = vs[0][2] = -2000;
	vs[1][0] = vs[1][1] = vs[1][2] = 2000;

	brushC = 0;
	skybox[0] = '\0';
	Q_strcat(skybox, sizeof(skybox), "// Game: Quake 3\n"
		"// Format: Quake3 (legacy)\n"
		"// entity 0\n"
		"{\n"
		"\"classname\" \"worldspawn\"\n");

	Q_strcat(skybox, sizeof(skybox), SV_MakeBox(vs[0], vs[1]));

	for(int i = 0; i < rows * cols; i++) {
		int y = i / cols;
		int x = i % cols;
		vs[0][0] = -(totalWidth / 2) + (x * (width + spacing));
		vs[1][0] = -(totalWidth / 2) + (x * (width + spacing)) + width;

		vs[0][1] = -(totalHeight / 2) + (y * (height + spacing));
		vs[1][1] = -(totalHeight / 2) + (y * (height + spacing)) + height;

		vs[0][2] = -(width / 2);
		vs[1][2] = (height / 2);
		Q_strcat(skybox, sizeof(skybox), SV_MakeBox(vs[0], vs[1]));
	}

	Q_strcat(skybox, sizeof(skybox), SV_MakePortal(100.0, 0, 0, 400, 400));

	Q_strcat(skybox, sizeof(skybox), "}\n");
	
	for(int i = 0; i < rows * cols; i++) {
		int y = i / cols;
		int x = i % cols;
	
		Q_strcat(skybox, sizeof(skybox), 
			va("{\n"
			"\"classname\" \"info_player_start\"\n"
			"\"origin\" \"%i %i %i\"\n"
			"}\n", -(totalWidth / 2) + (x * (width + spacing)) + width / 2,
			 -(totalHeight / 2) + (y * (height + spacing)) + height / 2,
			 0));
	}

	Q_strcat(skybox, sizeof(skybox), 
		va("{\n"
		"\"classname\" \"info_player_start\"\n"
		"\"origin\" \"%i %i %i\"\n"
		"}\n", -width * 2,
		 -height * 2,
		 -width - height));

	return skybox;
}


static drawVert_t *AddDrawVertsLump( void ){
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
  header.lumps[LUMP_DRAWVERTS].filelen = size;
	
	return buffer;
}


static dbrushside_t *AddBrushSidesLump( void )
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
  header.lumps[LUMP_BRUSHSIDES].filelen = size;
	
	return buffer;
}


static dsurface_t *AddDrawSurfacesLump( void ){
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
  header.lumps[LUMP_SURFACES].filelen = size;
	
	return buffer;
}


static byte *AddLightGridLumps( void ){
	int i;
	bspGridPoint_t  *in;
	byte *buffer, *out;

	if(dGridPoints) {
		free(dGridPoints);
	}

	/* dummy check */
	if ( bspGridPoints == NULL ) {
		return 0;
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

	//dGridPoints = buffer;
  header.lumps[LUMP_LIGHTGRID].filelen = 0; //numBSPGridPoints * sizeof( *out );
	
	return buffer;
}

static int lumpsStupidOrder[] = {
	LUMP_SHADERS, LUMP_PLANES, LUMP_LEAFS, LUMP_NODES,
	LUMP_BRUSHES, LUMP_BRUSHSIDES, LUMP_LEAFSURFACES,
	LUMP_LEAFBRUSHES, LUMP_MODELS, LUMP_DRAWVERTS,
	LUMP_SURFACES, LUMP_VISIBILITY, LUMP_LIGHTMAPS, LUMP_LIGHTGRID,
	LUMP_ENTITIES, LUMP_FOGS, LUMP_DRAWINDEXES, 
};

static void SV_AssignMemoryDatas( void ) {
	memset( &header, 0, sizeof( header ) );
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
	AddBrushSidesLump();
	dLeafSurfaces = (void *)bspLeafSurfaces;
  header.lumps[LUMP_LEAFSURFACES].filelen = numBSPLeafSurfaces * sizeof( int );
  dLeafBrushes = (void *)bspLeafBrushes;
  header.lumps[LUMP_LEAFBRUSHES].filelen = numBSPLeafBrushes * sizeof( int );
  dModels = (void *)bspModels;
  header.lumps[LUMP_MODELS].filelen = numBSPModels * sizeof( dmodel_t );
	AddDrawVertsLump();
	AddDrawSurfacesLump();
	//dModels = (void *)drawSurfaces;
	//header.lumps[LUMP_SURFACES].filelen = numBSPDrawSurfaces * sizeof( dsurface_t );
  dVisBytes = (void *)bspVisBytes;
  header.lumps[LUMP_VISIBILITY].filelen = numBSPVisBytes;
  dLightBytes = (void *)bspLightBytes;
  header.lumps[LUMP_LIGHTMAPS].filelen = numBSPLightBytes;
	AddLightGridLumps();
  dEntData = (void *)&bspEntData;
  header.lumps[LUMP_ENTITIES].filelen = bspEntDataSize;
  dFogs = (void *)bspFogs;
  header.lumps[LUMP_FOGS].filelen = numBSPFogs * sizeof( dfog_t );
  dDrawIndexes = (void *)bspDrawIndexes;
  header.lumps[LUMP_DRAWINDEXES].filelen = numBSPDrawIndexes * sizeof( int );

	for(int i = 0; i < HEADER_LUMPS; i++) {
		header.lumps[i].fileofs = 0;
	}
}


static void SV_LoadMapFromMemory( void ) {
	SV_AssignMemoryDatas();

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


void SV_WriteMemoryMapToClient(client_t *cl, int slot) {
	char marker[ 1024 ];
	int curindex;

	SV_AssignMemoryDatas();
	
	time_t t = I_FloatTime();
	//sprintf( marker, "I LOVE QUAKE.GAMES %s on %s)", Q3MAP_VERSION, asctime( localtime( &t ) ) );
	sprintf( marker, "I LOVE MY Q3MAP2 %s on %s)", Q3MAP_VERSION, asctime( localtime( &t ) ) );

	for(int i = 0; i < HEADER_LUMPS; i++) {
		lump_t *lump = &header.lumps[lumpsStupidOrder[i]];
		lump_t *prev = &header.lumps[lumpsStupidOrder[i - 1]];
		if(i == 0)
			lump->fileofs = ((sizeof(dheader_t) + strlen(marker) + 1) + 3) & ~3;
		else
			lump->fileofs = prev->fileofs + ((prev->filelen + 3) & ~3);
//Com_Printf("Lump: %i - %i\n", lump->fileofs, lump->filelen);
	}

	void *orderedLumpDatas[] = {
		dShaders,
		dPlanes,
		dLeafs,
		dNodes,
		dBrushes,
		dBrushSides,
		dLeafSurfaces,
		dLeafBrushes,
		dModels,
		dDrawVerts,
		dDrawSurfaces,
		dVisBytes,
		dLightBytes,
		dGridPoints,
		dEntData,
		dFogs,
		dDrawIndexes
	};

	cl->downloadSize = header.lumps[lumpsStupidOrder[HEADER_LUMPS-1]].fileofs + header.lumps[lumpsStupidOrder[HEADER_LUMPS-1]].filelen;
	// regenerate entire file
	//cl->downloadCurrentBlock = 0;
	//cl->downloadCount = 0;
	Com_Printf("Download %i > %i\n", cl->downloadSize, cl->downloadCount);
	while (cl->downloadCurrentBlock - cl->downloadClientBlock < MAX_DOWNLOAD_WINDOW &&
		cl->downloadSize > cl->downloadCount) {

		curindex = (cl->downloadCurrentBlock % MAX_DOWNLOAD_WINDOW);
		cl->downloadBlockSize[curindex] = 0;

		if (!cl->downloadBlocks[curindex]) {
			cl->downloadBlocks[curindex] = Z_Malloc( MAX_DOWNLOAD_BLKSIZE );
		}

		for(int i = 0; i < HEADER_LUMPS; i++) {
			lump_t *lump = &header.lumps[lumpsStupidOrder[i]];
			void *data = orderedLumpDatas[i];
			if(lump->fileofs + lump->filelen < cl->downloadCount) {
				// already past this lump in the download setup
				continue;
			}

			if(cl->downloadCurrentBlock == 0 && cl->downloadCount == 0 && i == 0) {
Com_Printf("Beginning header\n");
				memcpy(&cl->downloadBlocks[curindex][0], &header.ident, sizeof(int));
				memcpy(&cl->downloadBlocks[curindex][4], &header.version, sizeof(int));
				for(int j = 0; j < HEADER_LUMPS; j++) {
					lump_t *lump = &header.lumps[j];
					//if(j != 15) {
						memcpy(&cl->downloadBlocks[curindex][8 + j * 8], &lump->fileofs, sizeof(int));
						memcpy(&cl->downloadBlocks[curindex][12 + j * 8], &lump->filelen, sizeof(int));
					//}
				}
				memcpy(&cl->downloadBlocks[curindex][sizeof(header)], marker, strlen(marker) + 1);
				cl->downloadCount = lump->fileofs;
			}

			//Com_Printf("Lump ofs: %i, %i, %i\n", lumpsStupidOrder[i], lump->fileofs, cl->downloadCount - lump->fileofs);
			if(lump->fileofs - cl->downloadCount > 3) {
				Com_Error(ERR_DROP, "Should never happen because the previous loop should fill or break.");
			} else {
				int fillStart = (cl->downloadCount % MAX_DOWNLOAD_BLKSIZE);
				if(lump->fileofs + lump->filelen > (cl->downloadCurrentBlock + 1) * MAX_DOWNLOAD_BLKSIZE) {
					// fill the whole block
					cl->downloadBlockSize[curindex] = MAX_DOWNLOAD_BLKSIZE;
					// diff from count because previous loop might have been a partial lump
					int diffLength = MAX_DOWNLOAD_BLKSIZE - fillStart;
					if(diffLength > 0) {
						memcpy(&cl->downloadBlocks[curindex][fillStart], &data[cl->downloadCount - lump->fileofs], diffLength);
					}
					Com_Printf("Lump fill (%i, %i): %i, %i (%i left)\n", cl->downloadCurrentBlock, lumpsStupidOrder[i],
					  cl->downloadBlockSize[curindex], lump->filelen,
					  (lump->fileofs + lump->filelen) - (cl->downloadCurrentBlock + 1) * MAX_DOWNLOAD_BLKSIZE);
					cl->downloadCount += diffLength;
					break;
				} else {
					// fill partially with this lump, then loop and fill with next lump
					int remainingLength = (lump->fileofs + lump->filelen) - cl->downloadCount;
					cl->downloadBlockSize[curindex] += remainingLength;
					if(remainingLength > 0) {
						memcpy(&cl->downloadBlocks[curindex][fillStart], &data[cl->downloadCount - lump->fileofs], remainingLength);
					}
					Com_Printf("Lump end (%i, %i): %i, %i\n", cl->downloadCurrentBlock, lumpsStupidOrder[i],
					 	cl->downloadBlockSize[curindex], lump->filelen);
					cl->downloadCount += remainingLength;
					// loop back around and start on new lump
					if(cl->downloadBlockSize[curindex] >= MAX_DOWNLOAD_BLKSIZE) {
						break;
					}
				}
			}
		}
		if(cl->downloadClientBlock * MAX_DOWNLOAD_BLKSIZE > cl->downloadSize + MAX_DOWNLOAD_BLKSIZE) {
			Com_Error(ERR_DROP, "Should never happen!\n");
		}

		// Load in next block
		if(cl->downloadCount >= cl->downloadSize) {
			cl->downloadCurrentBlock++;
			break;
		} else if (cl->downloadBlockSize[curindex] == MAX_DOWNLOAD_BLKSIZE) {
			cl->downloadCurrentBlock++;
		} else {
			Com_Error(ERR_DROP, "Should never happen because block should be filled with data\n");
		}
	}
}


int SV_MakeMap( void ) {

	char *skybox = SV_MakeHypercube();
	
	int result = CM_LoadMapFromMemory();

	fileHandle_t mapfile = FS_SV_FOpenFileWrite( va("*memory%i.map", result) );
	FS_Write( skybox, strlen(skybox), mapfile );    // overwritten later
	FS_FCloseFile( mapfile );

	BSPMemory(skybox, result);

  SV_LoadMapFromMemory();
	
	return result;
}

#endif
