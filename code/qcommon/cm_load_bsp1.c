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

#include "cm_local.h"

#ifdef BSPC

#include "../bspc/l_qfiles.h"

void SetPlaneSignbits( cplane_t *out ) {
	int	bits, j;

	// for fast box on planeside test
	bits = 0;
	for ( j = 0; j < 3; j++) {
		if ( out->normal[j] < 0 ) {
			bits |= 1<<j;
		}
	}
	out->signbits = bits;
}
#endif //BSPC

// to allow boxes to be treated as brush models, we allocate
// some extra indexes along with those needed by the map
#define	BOX_BRUSHES		1
#define	BOX_SIDES		6
#define	BOX_LEAFS		2
#define	BOX_PLANES		12

#define	LL(x) x=LittleLong(x)

clipMap_t	cms[MAX_NUM_MAPS];
int     cm = 0;
int			c_pointcontents;
int			c_traces, c_brush_traces, c_patch_traces;


byte		*cmod_base;

#ifndef BSPC
cvar_t		*cm_noAreas;
cvar_t		*cm_noCurves;
cvar_t		*cm_playerCurveClip;
cvar_t    *cm_saveEnts;
#endif

cmodel_t	box_model[MAX_NUM_MAPS];
cplane_t	*box_planes[MAX_NUM_MAPS];
cbrush_t	*box_brush[MAX_NUM_MAPS];



void	CM_InitBoxHull (void);
void	CM_FloodAreaConnections (void);


/*
===============================================================================

					MAP LOADING

===============================================================================
*/

/*
=================
CMod_LoadShaders
=================
*/
void CMod_LoadShaders( lump_t *l ) {
	dshader_t	*in, *out;
	int			i, count;

	in = (void *)(cmod_base + l->fileofs);
	if (l->filelen % sizeof(*in)) {
		Com_Error (ERR_DROP, "CMod_LoadShaders: funny lump size");
	}
	count = l->filelen / sizeof(*in);

	if (count < 1) {
		Com_Error (ERR_DROP, "Map with no shaders");
	}
	cms[cm].shaders = Hunk_Alloc( count * sizeof( *cms[cm].shaders ), h_high );
	cms[cm].numShaders = count;

	Com_Memcpy( cms[cm].shaders, in, count * sizeof( *cms[cm].shaders ) );

	out = cms[cm].shaders;
	for ( i=0 ; i<count ; i++, in++, out++ ) {
		out->contentFlags = LittleLong( out->contentFlags );
		out->surfaceFlags = LittleLong( out->surfaceFlags );
	}
}


/*
=================
CMod_LoadSubmodels
=================
*/
void CMod_LoadSubmodels( lump_t *l ) {
	dmodel_t	*in;
	cmodel_t	*out;
	int			i, j, count;
	int			*indexes;

	in = (void *)(cmod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Com_Error (ERR_DROP, "CMod_LoadSubmodels: funny lump size");
	count = l->filelen / sizeof(*in);

	if (count < 1)
		Com_Error (ERR_DROP, "Map with no models");
	cms[cm].cmodels = Hunk_Alloc( count * sizeof( *cms[cm].cmodels ), h_high );
	cms[cm].numSubModels = count;

	if ( count > MAX_SUBMODELS ) {
		Com_Error( ERR_DROP, "MAX_SUBMODELS exceeded" );
	}

	for ( i=0 ; i<count ; i++, in++)
	{
		out = &cms[cm].cmodels[i];

		for (j=0 ; j<3 ; j++)
		{	// spread the mins / maxs by a pixel
			out->mins[j] = LittleFloat (in->mins[j]) - 1;
			out->maxs[j] = LittleFloat (in->maxs[j]) + 1;
		}

		if ( i == 0 ) {
			continue;	// world model doesn't need other info
		}

		// make a "leaf" just to hold the model's brushes and surfaces
		out->leaf.numLeafBrushes = LittleLong( in->numBrushes );
		indexes = Hunk_Alloc( out->leaf.numLeafBrushes * 4, h_high );
		out->leaf.firstLeafBrush = indexes - cms[cm].leafbrushes;
		for ( j = 0 ; j < out->leaf.numLeafBrushes ; j++ ) {
			indexes[j] = LittleLong( in->firstBrush ) + j;
		}

		out->leaf.numLeafSurfaces = LittleLong( in->numSurfaces );
		indexes = Hunk_Alloc( out->leaf.numLeafSurfaces * 4, h_high );
		out->leaf.firstLeafSurface = indexes - cms[cm].leafsurfaces;
		for ( j = 0 ; j < out->leaf.numLeafSurfaces ; j++ ) {
			indexes[j] = LittleLong( in->firstSurface ) + j;
		}
	}
}


/*
=================
CMod_LoadNodes

=================
*/
void CMod_LoadNodes( lump_t *l ) {
	dnode_t	*in;
	int		child;
	cNode_t	*out;
	int		i, j, count;

	in = (void *)(cmod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size" );

	count = l->filelen / sizeof(*in);

	if (count < 1)
		Com_Error( ERR_DROP, "Map has no nodes" );
	cms[cm].nodes = Hunk_Alloc( count * sizeof( *cms[cm].nodes ), h_high );
	cms[cm].numNodes = count;

	out = cms[cm].nodes;

	for ( i = 0; i < count; i++, out++, in++ )
	{
		out->plane = cms[cm].planes + LittleLong( in->planeNum );
		for ( j = 0; j < 2; j++ )
		{
			child = LittleLong( in->children[j] );
			out->children[j] = child;
		}
	}

}

/*
=================
CM_BoundBrush

=================
*/
void CM_BoundBrush( cbrush_t *b ) {
	b->bounds[0][0] = -b->sides[0].plane->dist;
	b->bounds[1][0] = b->sides[1].plane->dist;

	b->bounds[0][1] = -b->sides[2].plane->dist;
	b->bounds[1][1] = b->sides[3].plane->dist;

	b->bounds[0][2] = -b->sides[4].plane->dist;
	b->bounds[1][2] = b->sides[5].plane->dist;
}


/*
=================
CMod_LoadBrushes

=================
*/
void CMod_LoadBrushes( lump_t *l ) {
	dbrush_t	*in;
	cbrush_t	*out;
	int			i, count;

	in = (void *)(cmod_base + l->fileofs);
	if (l->filelen % sizeof(*in)) {
		Com_Error (ERR_DROP, "MOD_LoadBmodel: funny lump size");
	}
	count = l->filelen / sizeof(*in);

	cms[cm].brushes = Hunk_Alloc( ( BOX_BRUSHES + count ) * sizeof( *cms[cm].brushes ), h_high );
	cms[cm].numBrushes = count;

	out = cms[cm].brushes;

	for ( i = 0; i < count; i++, out++, in++ ) {
		out->sides = cms[cm].brushsides + LittleLong( in->firstSide );
		out->numsides = LittleLong( in->numSides );

		out->shaderNum = LittleLong( in->shaderNum );
		if ( out->shaderNum < 0 || out->shaderNum >= cms[cm].numShaders ) {
			Com_Error( ERR_DROP, "CMod_LoadBrushes: bad shaderNum: %i", out->shaderNum );
		}
		out->contents = cms[cm].shaders[out->shaderNum].contentFlags;

		CM_BoundBrush( out );
	}

}


/*
=================
CMod_LoadLeafs
=================
*/
void CMod_LoadLeafs( lump_t *l )
{
	int			i;
	cLeaf_t		*out;
	dleaf_t 	*in;
	int			count;

	in = (void *)(cmod_base + l->fileofs);
	if ( l->filelen % sizeof(*in) )
		Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size" );

	count = l->filelen / sizeof(*in);

	if ( count < 1 )
		Com_Error( ERR_DROP, "Map with no leafs" );

	cms[cm].leafs = Hunk_Alloc( ( BOX_LEAFS + count ) * sizeof( *cms[cm].leafs ), h_high );
	cms[cm].numLeafs = count;

	out = cms[cm].leafs;
	for ( i = 0; i < count; i++, in++, out++ )
	{
		out->cluster = LittleLong( in->cluster );
		out->area = LittleLong( in->area );
		out->firstLeafBrush = LittleLong( in->firstLeafBrush );
		out->numLeafBrushes = LittleLong( in->numLeafBrushes );
		out->firstLeafSurface = LittleLong( in->firstLeafSurface );
		out->numLeafSurfaces = LittleLong( in->numLeafSurfaces );

		if ( out->cluster >= cms[cm].numClusters )
			cms[cm].numClusters = out->cluster + 1;
		if ( out->area >= cms[cm].numAreas )
			cms[cm].numAreas = out->area + 1;
	}

	cms[cm].areas = Hunk_Alloc( cms[cm].numAreas * sizeof( *cms[cm].areas ), h_high );
	cms[cm].areaPortals = Hunk_Alloc( cms[cm].numAreas * cms[cm].numAreas * sizeof( *cms[cm].areaPortals ), h_high );
}


/*
=================
CMod_LoadPlanes
=================
*/
void CMod_LoadPlanes( const lump_t *l )
{
	int			i, j;
	cplane_t	*out;
	dplane_t 	*in;
	int			count;
	int			bits;

	in = (void *)(cmod_base + l->fileofs);
	if ( l->filelen % sizeof(*in) )
		Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size" );

	count = l->filelen / sizeof(*in);

	if ( count < 1 )
		Com_Error( ERR_DROP, "Map with no planes" );

	cms[cm].planes = Hunk_Alloc( ( BOX_PLANES + count ) * sizeof( *cms[cm].planes ), h_high );
	cms[cm].numPlanes = count;

	out = cms[cm].planes;

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
CMod_LoadLeafBrushes
=================
*/
void CMod_LoadLeafBrushes( const lump_t *l )
{
	int i;
	int *out;
	int *in;
	int count;

	in = (void *)(cmod_base + l->fileofs);
	if ( l->filelen % sizeof(*in) )
		Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size" );

	count = l->filelen / sizeof(*in);

	cms[cm].leafbrushes = Hunk_Alloc( (count + BOX_BRUSHES) * sizeof( *cms[cm].leafbrushes ), h_high );
	cms[cm].numLeafBrushes = count;

	out = cms[cm].leafbrushes;

	for ( i = 0; i < count; i++, in++, out++ ) {
		*out = LittleLong( *in );
	}
}


/*
=================
CMod_LoadLeafSurfaces
=================
*/
void CMod_LoadLeafSurfaces( const lump_t *l )
{
	int i;
	int *out;
	int *in;
	int count;

	in = (void *)(cmod_base + l->fileofs);
	if ( l->filelen % sizeof(*in) )
		Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size" );

	count = l->filelen / sizeof(*in);

	cms[cm].leafsurfaces = Hunk_Alloc( count * sizeof( *cms[cm].leafsurfaces ), h_high );
	cms[cm].numLeafSurfaces = count;

	out = cms[cm].leafsurfaces;

	for ( i = 0; i < count; i++, in++, out++ ) {
		*out = LittleLong( *in );
	}
}


/*
=================
CMod_CheckLeafBrushes
=================
*/
void CMod_CheckLeafBrushes( void )
{
	int	i;

	for ( i = 0; i < cms[cm].numLeafBrushes; i++ ) {
		if ( cms[cm].leafbrushes[ i ] < 0 || cms[cm].leafbrushes[ i ] >= cms[cm].numBrushes ) {
			Com_DPrintf( S_COLOR_YELLOW "[%i] invalid leaf brush %08x\n", i, cms[cm].leafbrushes[i] );
			cms[cm].leafbrushes[ i ] = 0;
		}
	}
}


/*
=================
CMod_LoadBrushSides
=================
*/
void CMod_LoadBrushSides (lump_t *l)
{
	int				i;
	cbrushside_t	*out;
	dbrushside_t	*in;
	int				count;
	int				num;

	in = (void *)(cmod_base + l->fileofs);
	if ( l->filelen % sizeof(*in) ) {
		Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size" );
	}
	count = l->filelen / sizeof(*in);

	cms[cm].brushsides = Hunk_Alloc( ( BOX_SIDES + count ) * sizeof( *cms[cm].brushsides ), h_high );
	cms[cm].numBrushSides = count;

	out = cms[cm].brushsides;

	for ( i= 0; i < count; i++, in++, out++ ) {
		num = LittleLong( in->planeNum );
		out->plane = &cms[cm].planes[num];
		out->shaderNum = LittleLong( in->shaderNum );
		if ( out->shaderNum < 0 || out->shaderNum >= cms[cm].numShaders ) {
			Com_Error( ERR_DROP, "CMod_LoadBrushSides: bad shaderNum: %i", out->shaderNum );
		}
		out->surfaceFlags = cms[cm].shaders[out->shaderNum].surfaceFlags;
	}
}


/*
=================
CMod_LoadEntityString
=================
*/
void CMod_LoadEntityString( lump_t *l, const char *name ) {
	fileHandle_t h;
	char entName[MAX_QPATH];
	size_t entNameLen = 0;
	int entFileLen = 0;

	// Attempt to load entities from an external .ent file if available
	Q_strncpyz(entName, name, sizeof(entName));
	entNameLen = strlen(entName);
	entName[entNameLen - 3] = 'e';
	entName[entNameLen - 2] = 'n';
	entName[entNameLen - 1] = 't';
	entFileLen = FS_FOpenFileRead( entName, &h, qtrue );
	if (h && entFileLen > 0)
	{
		cms[cm].entityString = (char *)Hunk_Alloc(entFileLen + 1, h_high );
		cms[cm].numEntityChars = entFileLen + 1;
		FS_Read( cms[cm].entityString, entFileLen, h );
		FS_FCloseFile(h);
		cms[cm].entityString[entFileLen] = '\0';
		Com_Printf( S_COLOR_CYAN "Loaded entities from %s\n", entName );
		return;
	}

	cms[cm].entityString = Hunk_Alloc( l->filelen, h_high );
	cms[cm].numEntityChars = l->filelen;
	memcpy( cms[cm].entityString, cmod_base + l->fileofs, l->filelen );
	if(cm_saveEnts->integer) {
		FS_WriteFile(entName, cms[cm].entityString, cms[cm].numEntityChars);
	}
}


/*
=================
CMod_LoadVisibility
=================
*/
#define	VIS_HEADER	8
void CMod_LoadVisibility( lump_t *l ) {
	int		len;
	byte	*buf;

	len = l->filelen;
	if ( !len ) {
		cms[cm].clusterBytes = ( cms[cm].numClusters + 31 ) & ~31;
		cms[cm].visibility = Hunk_Alloc( cms[cm].clusterBytes, h_high );
		Com_Memset( cms[cm].visibility, 255, cms[cm].clusterBytes );
		return;
	}
	buf = cmod_base + l->fileofs;

	cms[cm].vised = qtrue;
	cms[cm].visibility = Hunk_Alloc( len, h_high );
	cms[cm].numClusters = LittleLong( ((int *)buf)[0] );
	cms[cm].clusterBytes = LittleLong( ((int *)buf)[1] );
	Com_Memcpy (cms[cm].visibility, buf + VIS_HEADER, len - VIS_HEADER );
}

//==================================================================


/*
=================
CMod_LoadPatches
=================
*/
#define	MAX_PATCH_VERTS		1024
void CMod_LoadPatches( lump_t *surfs, lump_t *verts ) {
	drawVert_t	*dv, *dv_p;
	dsurface_t	*in;
	int			count;
	int			i, j;
	int			c;
	cPatch_t	*patch;
	vec3_t		points[MAX_PATCH_VERTS];
	int			width, height;
	int			shaderNum;

	in = (void *)(cmod_base + surfs->fileofs);
	if (surfs->filelen % sizeof(*in))
		Com_Error (ERR_DROP, "MOD_LoadBmodel: funny lump size");
	cms[cm].numSurfaces = count = surfs->filelen / sizeof(*in);
	cms[cm].surfaces = Hunk_Alloc( cms[cm].numSurfaces * sizeof( cms[cm].surfaces[0] ), h_high );

	dv = (void *)(cmod_base + verts->fileofs);
	if (verts->filelen % sizeof(*dv))
		Com_Error (ERR_DROP, "MOD_LoadBmodel: funny lump size");

	// scan through all the surfaces, but only load patches,
	// not planar faces
	for ( i = 0 ; i < count ; i++, in++ ) {
		if ( LittleLong( in->surfaceType ) != MST_PATCH ) {
			continue;		// ignore other surfaces
		}
		// FIXME: check for non-colliding patches

		cms[cm].surfaces[ i ] = patch = Hunk_Alloc( sizeof( *patch ), h_high );

		// load the full drawverts onto the stack
		width = LittleLong( in->patchWidth );
		height = LittleLong( in->patchHeight );
		c = width * height;
		if ( c > MAX_PATCH_VERTS ) {
			Com_Error( ERR_DROP, "ParseMesh: MAX_PATCH_VERTS" );
		}

		dv_p = dv + LittleLong( in->firstVert );
		for ( j = 0 ; j < c ; j++, dv_p++ ) {
			points[j][0] = LittleFloat( dv_p->xyz[0] );
			points[j][1] = LittleFloat( dv_p->xyz[1] );
			points[j][2] = LittleFloat( dv_p->xyz[2] );
		}

		shaderNum = LittleLong( in->shaderNum );
		patch->contents = cms[cm].shaders[shaderNum].contentFlags;
		patch->surfaceFlags = cms[cm].shaders[shaderNum].surfaceFlags;

		// create the internal facet structure
		patch->pc = CM_GeneratePatchCollide( width, height, points );
	}
}
