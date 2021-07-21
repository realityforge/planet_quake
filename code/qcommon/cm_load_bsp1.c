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

extern void CMod_LoadEntityString( lump_t *l, const char *name );

#define	BOX_BRUSHES		1
#define	BOX_SIDES		6
#define	BOX_LEAFS		2
#define	BOX_PLANES		12

void CMod_LoadShaders1( lump_t *l ) {	
	int		i, count;
  dBsp1Miptex_t *in;
	dshader_t	*out;
  miptex_t *mt;
	
	in = (void *)(cmod_base + l->fileofs);
  count = LittleLong ( in->nummiptex );
	out = Hunk_Alloc ( count*sizeof(*out), h_low );

	cm.shaders = out;
	cm.numShaders = count;

	for ( i=0 ; i<count ; i++ ) {
    if (in->dataofs[i] == -1) {
      continue;
    }

    mt = (miptex_t *)((byte *)in + in->dataofs[i]);
    memcpy(out[i].shader, mt->name, 16);
		out[i].surfaceFlags = 0;
    if (strstr(mt->name, "water") || strstr(mt->name, "mwat")) {
      out[i].contentFlags = CONTENTS_WATER;
    }
    else if (strstr(mt->name, "slime")) {
      out[i].contentFlags = CONTENTS_SLIME;
    }
    else if (strstr(mt->name, "lava")) {
      out[i].contentFlags = CONTENTS_LAVA;
    }
  }
}


/*
=================
CMod_LoadLeafs
=================
*/
void CMod_LoadLeafs1( lump_t *l )
{
	int			i;
	cLeaf_t		*out;
	dBsp1Leaf_t 	*in;
	int			count;

	in = (void *)(cmod_base + l->fileofs);
	if ( l->filelen % sizeof(*in) )
		Com_Error( ERR_DROP, "%s: funny lump size", __func__ );

	count = l->filelen / sizeof(*in);
	if ( count < 1 )
		Com_Error( ERR_DROP, "%s: map with no leafs", __func__ );

	cm.leafs = Hunk_Alloc( ( BOX_LEAFS + count ) * sizeof( *cm.leafs ), h_high );
	cm.numLeafs = count;

	out = cm.leafs;
	for ( i = 0; i < count; i++, in++, out++ )
	{
		out->cluster = -1; //LittleLong( in->cluster );
		out->area = -1; //LittleLong( in->area );
		//out->firstLeafBrush = LittleLong( in->firstLeafBrush );
		//out->numLeafBrushes = LittleLong( in->numLeafBrushes );
		out->firstLeafSurface = in->firstmarksurface;
		out->numLeafSurfaces = in->nummarksurfaces;

		if ( out->cluster >= cm.numClusters )
			cm.numClusters = out->cluster + 1;
		if ( out->area >= cm.numAreas )
			cm.numAreas = out->area + 1;
	}

	cm.areas = Hunk_Alloc( cm.numAreas * sizeof( *cm.areas ), h_high );
	cm.areaPortals = Hunk_Alloc( cm.numAreas * cm.numAreas * sizeof( *cm.areaPortals ), h_high );
}


/*
=================
CMod_LoadLeafBrushes
=================
*/
void CMod_LoadLeafBrushes1( const lump_t *l )
{
	int i;
	int *out;
	short *in;
	int count;

	in = (void *)(cmod_base + l->fileofs);
	if ( l->filelen % sizeof(*in) )
		Com_Error( ERR_DROP, "%s: funny lump size", __func__ );

	count = l->filelen / sizeof(*in);

	cm.leafbrushes = Hunk_Alloc( (count + BOX_BRUSHES) * sizeof( *cm.leafbrushes ), h_high );
	cm.numLeafBrushes = count;

	out = cm.leafbrushes;

	for ( i = 0; i < count; i++, in++, out++ ) {
		*out = *in;
	}
}


/*
=================
CMod_LoadLeafSurfaces
=================
*/
void CMod_LoadLeafSurfaces1( const lump_t *l )
{
	int i;
	int *out;
	short *in;
	int count;

	in = (void *)(cmod_base + l->fileofs);
	if ( l->filelen % sizeof(*in) )
		Com_Error( ERR_DROP, "%s: funny lump size", __func__ );

	count = l->filelen / sizeof(*in);

	cm.leafsurfaces = Hunk_Alloc( count * sizeof( *cm.leafsurfaces ), h_high );
	cm.numLeafSurfaces = count;

	out = cm.leafsurfaces;

	for ( i = 0; i < count; i++, in++, out++ ) {
		*out = *in;
	}
}


/*
=================
CMod_LoadPlanes
=================
*/
void CMod_LoadPlanes1( const lump_t *l )
{
	int			i, j;
	cplane_t	*out;
	dBsp1Plane_t 	*in;
	int			count;
	int			bits;

	in = (void *)(cmod_base + l->fileofs);
	if ( l->filelen % sizeof(*in) )
		Com_Error( ERR_DROP, "%s: funny lump size", __func__ );

	count = l->filelen / sizeof(*in);
	if ( count < 1 )
		Com_Error( ERR_DROP, "%s: map with no planes", __func__ );

	cm.planes = Hunk_Alloc( ( BOX_PLANES + count ) * sizeof( *cm.planes ), h_high );
	cm.numPlanes = count;

	out = cm.planes;

	for ( i = 0; i < count; i++, in++, out++ )
	{
		bits = 0;
		for ( j = 0; j < 3; j++ )
		{
			out->normal[j] = LittleFloat( in->normal[j] );
			if ( out->normal[j] < 0 )
				bits |= 1<<j;
		}

		out->dist = LittleFloat( in->dist );
		out->type = PlaneTypeForNormal( out->normal );
		out->signbits = bits;
	}
}


/*
=================
CMod_LoadSubmodels
=================
*/
void CMod_LoadSubmodels1( lump_t *l ) {
	dBsp1Model_t	*in;
	cmodel_t	*out;
	int			i, j, count;
	int			*indexes;

	in = (void *)(cmod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Com_Error( ERR_DROP, "%s: funny lump size", __func__ );

	count = l->filelen / sizeof(*in);
	if ( count < 1 )
		Com_Error( ERR_DROP, "%s: map with no models", __func__ );

	cm.cmodels = Hunk_Alloc( count * sizeof( *cm.cmodels ), h_high );
	cm.numSubModels = count;

	if ( count > MAX_SUBMODELS )
		Com_Error( ERR_DROP, "%s: MAX_SUBMODELS exceeded", __func__ );

	for ( i=0 ; i<count ; i++, in++)
	{
		out = &cm.cmodels[i];

		for (j=0 ; j<3 ; j++)
		{	// spread the mins / maxs by a pixel
			out->mins[j] = LittleFloat (in->mins[j]) - 1;
			out->maxs[j] = LittleFloat (in->maxs[j]) + 1;
		}

		if ( i == 0 ) {
			continue;	// world model doesn't need other info
		}

    /*
    TODO: MAX_MAP_HULLS? headnode?
		// make a "leaf" just to hold the model's brushes and surfaces
		out->leaf.numLeafBrushes = LittleLong( in->numBrushes );
		indexes = Hunk_Alloc( out->leaf.numLeafBrushes * 4, h_high );
		out->leaf.firstLeafBrush = indexes - cm.leafbrushes;
		for ( j = 0 ; j < out->leaf.numLeafBrushes ; j++ ) {
			indexes[j] = LittleLong( in->firstBrush ) + j;
		}
    */

		out->leaf.numLeafSurfaces = LittleLong( in->numfaces );
		indexes = Hunk_Alloc( out->leaf.numLeafSurfaces * 4, h_high );
		out->leaf.firstLeafSurface = indexes - cm.leafsurfaces;
		for ( j = 0 ; j < out->leaf.numLeafSurfaces ; j++ ) {
			indexes[j] = LittleLong( in->firstface ) + j;
		}
	}
}


/*
=================
CMod_LoadNodes

=================
*/
void CMod_LoadNodes1( lump_t *l ) {
	dBsp1Node_t	*in;
	int		child;
	cNode_t	*out;
	int		i, j, count;

  in = (dBsp1Node_t *)(cmod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Com_Error( ERR_DROP, "%s: funny lump size", __func__ );

	count = l->filelen / sizeof(*in);
	if ( count < 1 )
		Com_Error( ERR_DROP, "%s: map has no nodes", __func__ );

	cm.nodes = Hunk_Alloc( count * sizeof( *cm.nodes ), h_high );
	cm.numNodes = count;

	out = cm.nodes;

	for ( i = 0; i < count; i++, out++, in++ )
	{
		out->plane = cm.planes + LittleLong( in->planenum );
		for ( j = 0; j < 2; j++ )
		{
			child = in->children[j];
			out->children[j] = child;
		}
	}

}


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
  C(PLANES, planes2, numPlanes, dBsp1Plane_t);
  //C(TEXTURES, texinfo2, numTexinfo, dBsp1Miptex_t);
  C(VERTEXES, vertexes2, numVertexes, vec3_t);
  //C(VISIBILITY, brushes2, numBrushes, dBsp2Brush_t);
  C(NODES, nodes2, numNodes, dBsp1Node_t);
  C(TEXINFO, texinfo2, numTexinfo, dBsp1Texinfo_t);
  C(FACES, faces2, numFaces, dBsp1Face_t);
	C(LIGHTING, lighting, lightDataSize, byte);
  C(CLIPNODES, leafbrushes2, numLeafbrushes, unsigned short);
	C(LEAFS, leafs2, numLeafs, dBsp1Leaf_t);
	C(MARKSURFACES, leaffaces2, numLeaffaces, unsigned short);
  C(EDGES, edges, numEdges, short[2]);
	C(SURFEDGES, surfedges, numSurfedges, int);
	//C(BRUSHES, brushes2, numBrushes, dBsp2Brush_t);
	//C(BRUSHSIDES, brushsides2, numBrushSides, dBsp2Brushside_t);
	//C(ZONES, zones, numZones, dZone_t);
	//C(ZONEPORTALS, zonePortals, numZonePortals, dZonePortal_t);
	C(MODELS, models2, numModels, dBsp1Model_t);
#undef C

	// load into heap
	CMod_LoadShaders1( &header.lumps[LUMP_Q1_TEXTURES] );
	CMod_LoadLeafs1 (&header.lumps[LUMP_Q1_LEAFS]);
	CMod_LoadLeafBrushes1 (&header.lumps[LUMP_Q1_CLIPNODES]);
	CMod_LoadLeafSurfaces1 (&header.lumps[LUMP_Q1_MARKSURFACES]);
	CMod_LoadPlanes1 (&header.lumps[LUMP_Q1_PLANES]);
	//CMod_LoadBrushSides1 (&header.lumps[LUMP_Q1_BRUSHSIDES]);
	//CMod_LoadBrushes1 (&header.lumps[LUMP_Q1_BRUSHES]);
	CMod_LoadSubmodels1 (&header.lumps[LUMP_Q1_MODELS]);
	CMod_LoadNodes1 (&header.lumps[LUMP_Q1_NODES]);
	CMod_LoadEntityString (&header.lumps[LUMP_Q1_ENTITIES], name);
	//CMod_LoadVisibility1( &header.lumps[LUMP_Q1_VISIBILITY] );
	// TODO: area portals and area mask stuff
	//CMod_LoadAreas( &header.lumps[LUMP_Q1_ZONES] );
	//CMod_LoadAreaPortals( &header.lumps[LUMP_Q2_ZONEPORTALS] );
	//CMod_LoadPatches1( &header.lumps[LUMP_Q1_FACES], &header.lumps[LUMP_Q1_VERTEXES] );

}

#endif
