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
// cmodel.c -- model loading
#ifdef USE_BSP1

#include "cm_local.h"
#include "cm_load_bsp1.h"


/*
===============================================================================

					MAP LOADING

===============================================================================
*/

static int CheckLump(lump_t *l, char *lumpName, int size)
{
	if (l->filelen % size)
		Com_Error(ERR_DROP, "%s: incorrect lump size (%s)", __func__, lumpName);

	//*ptr = (void *)(cmod_base + l->fileofs);

	return l->filelen / size;
}

void LoadQ1Map(const char *name) {
	dBsp1Hdr_t		header;

	Com_DPrintf("CM_Load: Loading Quake 2 map.\n");
	header = *(dBsp1Hdr_t *)cmod_base;

#if !LITTLE_ENDIAN
	// swap the header
	for (int i = 0; i < sizeof(dBsp1Hdr_t) / 4; i++)
		((int *)cmod_base)[i] = LittleLong(((int *)cmod_base)[i]);
#endif

#define C(num,field,count,type) \
	CheckLump(&header.lumps[LUMP_Q1_##num], XSTRING(num), sizeof(type))
	C(LIGHTING, lighting, lightDataSize, byte);
	C(VERTEXES, vertexes2, numVertexes, vec3_t);
	C(PLANES, planes2, numPlanes, dBsp2Plane_t);
	C(LEAFS, leafs2, numLeafs, dBsp2Leaf_t);
	C(NODES, nodes2, numNodes, dBsp2Node_t);
	C(TEXINFO, texinfo2, numTexinfo, dBsp2Texinfo_t);
	C(FACES, faces2, numFaces, dBspFace_t);
	C(LEAFFACES, leaffaces2, numLeaffaces, unsigned short);
	C(LEAFBRUSHES, leafbrushes2, numLeafbrushes, unsigned short);
	C(SURFEDGES, surfedges, numSurfedges, int);
	C(EDGES, edges, numEdges, short[2]);
	C(BRUSHES, brushes2, numBrushes, dBsp2Brush_t);
	C(BRUSHSIDES, brushsides2, numBrushSides, dBsp2Brushside_t);
	C(ZONES, zones, numZones, dZone_t);
	C(ZONEPORTALS, zonePortals, numZonePortals, dZonePortal_t);
	C(MODELS, models2, numModels, dBsp2Model_t);
#undef C
	// load into heap
	CMod_LoadShaders1( &header.lumps[LUMP_Q1_TEXTURES] );
	CMod_LoadLeafs1 (&header.lumps[LUMP_Q1_LEAFS]);
	CMod_LoadLeafBrushes1 (&header.lumps[LUMP_Q1_LEAFBRUSHES]);
	CMod_LoadLeafSurfaces1 (&header.lumps[LUMP_Q1_LEAFFACES]);
	CMod_LoadPlanes1 (&header.lumps[LUMP_Q1_PLANES]);
	CMod_LoadBrushSides1 (&header.lumps[LUMP_Q1_BRUSHSIDES]);
	CMod_LoadBrushes1 (&header.lumps[LUMP_Q1_BRUSHES]);
	CMod_LoadSubmodels1 (&header.lumps[LUMP_Q1_MODELS]);
	CMod_LoadNodes1 (&header.lumps[LUMP_Q1_NODES]);
	CMod_LoadEntityString (&header.lumps[LUMP_Q1_ENTITIES], name);
	CMod_LoadVisibility1( &header.lumps[LUMP_Q1_VISIBILITY] );
	// TODO: area portals and area mask stuff
	CMod_LoadAreas( &header.lumps[LUMP_Q1_ZONES] );
	//CMod_LoadAreaPortals( &header.lumps[LUMP_Q2_ZONEPORTALS] );
	//
	//;
	CMod_LoadPatches1( &header.lumps[LUMP_Q1_FACES], &header.lumps[LUMP_Q1_VERTEXES] );

}

#endif
