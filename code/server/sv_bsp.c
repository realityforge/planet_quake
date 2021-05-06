
#ifdef USE_MEMORY_MAPS

#include "server.h"
#include "../qcommon/cm_public.h"
#define MAIN_C
#include "../tools/q3map2/q3map2.h"
#undef MAIN_C



static dheader_t header;
static char skybox[4096*1024];


static char *SV_MakeWall( int p1[3], int p2[3] ) {
	char *wall = &skybox[strlen(skybox)];
	Q_strcat(wall, sizeof(wall), "{\n");
	Q_strcat(wall, sizeof(wall),
		va("( %i %i %i ) ( %i %i %i ) ( %i %i %i ) e1u1/sky1 0 0 0 1 1 0 0 0\n",
		p1[0], p1[1], p2[2], p1[0], p1[1], p1[2], p1[0], p2[1], p1[2]
	));
	Q_strcat(wall, sizeof(wall),
		va("( %i %i %i ) ( %i %i %i ) ( %i %i %i ) e1u1/sky1 0 0 0 1 1 0 0 0\n",
		p2[0], p2[1], p2[2], p2[0], p2[1], p1[2], p2[0], p1[1], p1[2]
	));
	Q_strcat(wall, sizeof(wall),
		va("( %i %i %i ) ( %i %i %i ) ( %i %i %i ) e1u1/sky1 0 0 0 1 1 0 0 0\n",
		p2[0], p1[1], p2[2], p2[0], p1[1], p1[2], p1[0], p1[1], p1[2]
	));
	Q_strcat(wall, sizeof(wall),
		va("( %i %i %i ) ( %i %i %i ) ( %i %i %i ) e1u1/sky1 0 0 0 1 1 0 0 0\n",
		p1[0], p2[1], p2[2], p1[0], p2[1], p1[2], p2[0], p2[1], p1[2]
	));
	Q_strcat(wall, sizeof(wall),
		va("( %i %i %i ) ( %i %i %i ) ( %i %i %i ) e1u1/sky1 0 0 0 1 1 0 0 0\n",
		p1[0], p2[1], p1[2], p1[0], p1[1], p1[2], p2[0], p1[1], p1[2]
	));
	Q_strcat(wall, sizeof(wall),
		va("( %i %i %i ) ( %i %i %i ) ( %i %i %i ) e1u1/sky1 0 0 0 1 1 0 0 0\n",
		p1[0], p1[1], p2[2], p1[0], p2[1], p2[2], p2[0], p2[1], p2[2]
	));
	Q_strcat(wall, sizeof(wall), "}\n");
	return wall;
}


static char *SV_MakeBox( vec3_t min, vec3_t max ) {
	char *box = &skybox[strlen(skybox)];
	vec3_t  vs[2] = {
		{*min, *max}
	};
	
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

	for(int i = 0; i < 6; i++) {
		int *p1 = points[i*2];
		int *p2 = points[i*2+1];
		SV_MakeWall(p1, p2);
	}

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

	skybox[0] = '\0';
	Q_strcat(skybox, sizeof(skybox), "{\n"
		"\"classname\" \"worldspawn\"\n");
	
	SV_MakeBox(vs[0], vs[1]);
	
	Q_strcat(skybox, sizeof(skybox), 
		"}\n"
		"{\n"
		"\"classname\" \"info_player_start\"\n"
		"\"origin\" \"16 64 -52\"\n"
		"}\n");

	return (char *)skybox;
}


// TODO
static char *SV_MakeMaze( void ) {
	return "";
}


static char *SV_MakeHypercube( void ) {
	return SV_MakeSkybox();
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
	Com_Printf("Vis bytes: %i\n", numBSPVisBytes);
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
	while (cl->downloadCurrentBlock - cl->downloadClientBlock < MAX_DOWNLOAD_WINDOW &&
		cl->downloadSize != cl->downloadCount) {

		curindex = (cl->downloadCurrentBlock % MAX_DOWNLOAD_WINDOW);

		if (!cl->downloadBlocks[curindex]) {
			Com_Printf("Alloc %i\n", curindex);
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
					int filelen = (lump->filelen + 3) & ~3;
					int fileofs = (lump->fileofs + 3) & ~3;
					//if(j != 15) {
						memcpy(&cl->downloadBlocks[curindex][8 + j * 8], &fileofs, sizeof(int));
						memcpy(&cl->downloadBlocks[curindex][12 + j * 8], &filelen, sizeof(int));
					//}
				}
				memcpy(&cl->downloadBlocks[curindex][sizeof(header)], marker, strlen(marker) + 1);
				cl->downloadCount = lump->fileofs;
			}

			if(lump->fileofs - cl->downloadCount > 1) {
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
					//Com_Printf("Lump fill: %i, %i, %i\n", lumpsStupidOrder[i], diffLength, cl->downloadCount - lump->fileofs);
					cl->downloadCount += diffLength;
					break;
				} else {
					// fill partially with this lump, then loop and fill with next lump
					int remainingLength = (lump->fileofs + lump->filelen) - cl->downloadCount;
					cl->downloadBlockSize[curindex] += remainingLength;
					if(remainingLength > 0) {
						memcpy(&cl->downloadBlocks[curindex][fillStart], &data[cl->downloadCount - lump->fileofs], remainingLength);
					}
					//Com_Printf("Lump end: %i, %i, %i\n", lumpsStupidOrder[i], remainingLength, cl->downloadCount - lump->fileofs);
					cl->downloadCount += remainingLength;
					// loop back around and start on new lump
				}
			}
		}

		// Load in next block
		if(cl->downloadCount == cl->downloadSize) {
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

	char *skybox = SV_MakeSkybox();
	
	int result = CM_LoadMapFromMemory();

	fileHandle_t mapfile = FS_SV_FOpenFileWrite( va("*memory%i.map", result) );
	FS_Write( skybox, strlen(skybox), mapfile );    // overwritten later
	FS_FCloseFile( mapfile );

	BSPMemory(skybox, result);

  SV_LoadMapFromMemory();
	
	return result;
}

#endif
