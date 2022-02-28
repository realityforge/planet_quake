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
#include "cm_load_bsp2.h"

#ifdef USE_BSP2

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
void CMod_LoadShaders2( lump_t *l ) {
	dBsp2Texinfo_t *in;
	dshader_t *out;
	int			i, count;

	in = (void *)(cmod_base + l->fileofs);
	count = l->filelen / sizeof(*in);

	if (count < 1) {
		Com_Error (ERR_DROP, "Map with no shaders");
	}
	cm.shaders = Hunk_Alloc( count * sizeof( *cm.shaders ), h_high );
	cm.numShaders = count;

	out = cm.shaders;
	for ( i=0 ; i<count ; i++, in++, out++ ) {
		memcpy(&out->shader, va("textures/%s", in->texture[0] == '.' || in->texture[0] == '/' ? &in->texture[1] : in->texture), sizeof(out->shader));
		out->shader[sizeof(out->shader)-1] = '\0';
		out->contentFlags = LittleLong( in->flags );
		if(in->flags & Q2_SURF_SKY)
			out->surfaceFlags |= SURF_SKY;
		if(in->flags & Q2_SURF_NODRAW)
			out->surfaceFlags |= SURF_NODRAW;
		if (in->flags & MATERIAL_METAL)
			out->surfaceFlags |= SURF_METALSTEPS;
		if (in->flags & MATERIAL_SILENT)
			out->surfaceFlags |= SURF_NOSTEPS;
	}
}


/*
=================
CMod_LoadSubmodels
=================
*/
void CMod_LoadSubmodels2( lump_t *l ) {
	dBsp2Model_t	*in;
	cmodel_t	*out;
	int			i, j, count;
	int			*indexes;

	in = (void *)(cmod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Com_Error (ERR_DROP, "CMod_LoadSubmodels: funny lump size");
	count = l->filelen / sizeof(*in);

	if (count < 1)
		Com_Error (ERR_DROP, "Map with no models");
	cm.cmodels = Hunk_Alloc( count * sizeof( *cm.cmodels ), h_high );
	cm.numSubModels = count;

	if ( count > MAX_SUBMODELS ) {
		Com_Error( ERR_DROP, "MAX_SUBMODELS exceeded" );
	}

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

		int leafNum = in->headnode + 1;
		// make a "leaf" just to hold the model's brushes and surfaces
		out->leaf.numLeafBrushes = LittleLong( cm.leafs[leafNum].numLeafBrushes );
		indexes = Hunk_Alloc( out->leaf.numLeafBrushes * 4, h_high );
		out->leaf.firstLeafBrush = indexes - cm.leafbrushes;
		for ( j = 0 ; j < out->leaf.numLeafBrushes ; j++ ) {
			indexes[j] = LittleLong( in->headnode ) + j;
		}

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
CMod_LoadBrushes

=================
*/
void CMod_LoadBrushes2( lump_t *l ) {
	dBsp2Brush_t	*in;
	cbrush_t	*out;
	int			i, count;

	in = (void *)(cmod_base + l->fileofs);
	if (l->filelen % sizeof(*in)) {
		Com_Error (ERR_DROP, "MOD_LoadBmodel: funny lump size");
	}
	count = l->filelen / sizeof(*in);

	cm.brushes = Hunk_Alloc( ( BOX_BRUSHES + count ) * sizeof( *cm.brushes ), h_high );
	cm.numBrushes = count;

	out = cm.brushes;

	for ( i = 0; i < count; i++, out++, in++ ) {
		out->sides = cm.brushsides + LittleLong( in->firstside );
		out->numsides = LittleLong( in->numsides );
		
		out->shaderNum = (*out->sides).shaderNum;
		if ( out->shaderNum < 0 || out->shaderNum >= cm.numShaders ) {
			Com_Error( ERR_DROP, "CMod_LoadBrushes: bad shaderNum: %i", out->shaderNum );
		}
		out->contents = cm.shaders[out->shaderNum].contentFlags;

		CM_BoundBrush( out );
	}

}


/*
=================
CMod_LoadLeafs
=================
*/
void CMod_LoadLeafs2( lump_t *l )
{
	int			i;
	cLeaf_t		*out;
	dBsp2Leaf_t 	*in;
	int			count;

	in = (void *)(cmod_base + l->fileofs);
	count = l->filelen / sizeof(*in);

	if ( count < 1 )
		Com_Error( ERR_DROP, "Map with no leafs" );

	cm.leafs = Hunk_Alloc( ( BOX_LEAFS + count ) * sizeof( *cm.leafs ), h_high );
	cm.numLeafs = count;

	out = cm.leafs;
	for ( i = 0; i < count; i++, in++, out++ )
	{
		out->cluster = LittleLong( in->cluster );
		out->area = 0;
		out->firstLeafBrush = LittleLong( in->firstleafbrush );
		out->numLeafBrushes = LittleLong( in->numleafbrushes );
		out->firstLeafSurface = LittleLong( in->firstleafface );
		out->numLeafSurfaces = LittleLong( in->numleaffaces );

/*

	// get numClusters (have in dBsp2Vis_t, but Q2 recomputes this ...)
	cm.numClusters = 0;
	for (i = 0; i < cm.numLeafs; i++)
		if (cm.leafs2[i].cluster >= cm.numClusters)
			cm.numClusters = cm.leafs2[i].cluster + 1;

*/
		if ( out->cluster >= cm.numClusters )
			cm.numClusters = out->cluster + 1;
		if ( out->area >= cm.numAreas )
			cm.numAreas = out->area + 1;
	}
}


static void CMod_LoadAreas( lump_t *l )
{
	dZone_t 	*in;
	int			count;

	in = (void *)(cmod_base + l->fileofs);
	count = l->filelen / sizeof(*in);
// this code is for Quake3 from gildor2,
// so if quake 3 has no zones and its recreated,
// maybe its ok without it?

//!!	LoadZones(bsp->zones, bsp->numZones);
//!!	LoadZonePortals(bsp->zoneportals, bsp->numZonePortals);
	// Q3 zones: simulate loading (create 1 zone)

/*
	map_zones = new (dataChain) czone_t;
	numZones  = 1;
	numzoneportals = 0;
*/

	cm.numAreas = 2;
	cm.areas = Hunk_Alloc( cm.numAreas * sizeof( *cm.areas ), h_high );
	cm.areaPortals = Hunk_Alloc( cm.numAreas * cm.numAreas * sizeof( *cm.areaPortals ), h_high );
}



/*
=================
CMod_LoadNodes

=================
*/
void CMod_LoadNodes2( lump_t *l ) {
	dBsp2Node_t	*in;
	int		child;
	cNode_t	*out;
	int		i, j, count;

	in = (void *)(cmod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size" );

	count = l->filelen / sizeof(*in);

	if (count < 1)
		Com_Error( ERR_DROP, "Map has no nodes" );
	cm.nodes = Hunk_Alloc( count * sizeof( *cm.nodes ), h_high );
	cm.numNodes = count;

	out = cm.nodes;

	for ( i = 0; i < count; i++, out++, in++ )
	{
		out->plane = cm.planes + LittleLong( in->planeNum );
		for ( j = 0; j < 2; j++ )
		{
			child = LittleLong( in->children[j] );
			out->children[j] = child;
		}
	}

}


/*
=================
CMod_LoadPlanes
=================
*/
void CMod_LoadPlanes2( const lump_t *l )
{
	int			i, j;
	cplane_t	*out;
	dBsp2Plane_t 	*in;
	int			count;
	int			bits;

	in = (void *)(cmod_base + l->fileofs);
	if ( l->filelen % sizeof(*in) )
		Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size" );

	count = l->filelen / sizeof(*in);

	if ( count < 1 )
		Com_Error( ERR_DROP, "Map with no planes" );

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
	/*
	
		// silly check for correct lightmaps
		for (i = 0; i < cm.numFaces; i++)
			if (cm.faces2[i].lightofs > cm.lightDataSize)
				cm.faces2[i].lightofs = -1;

*/
}


/*
=================
CMod_LoadLeafBrushes
=================
*/
void CMod_LoadLeafBrushes2( const lump_t *l )
{
	int i;
	int *out;
	short *in;
	int count;

	in = (void *)(cmod_base + l->fileofs);
	count = l->filelen / sizeof(*in);

	cm.leafbrushes = Hunk_Alloc( (count + BOX_BRUSHES) * sizeof( *cm.leafbrushes ), h_high );
	cm.numLeafBrushes = count;

	out = cm.leafbrushes;

	for ( i = 0; i < count; i++, in++, out++ ) {
		*out = LittleLong( *in );
	}
}


/*
=================
CMod_LoadLeafSurfaces
=================
*/
void CMod_LoadLeafSurfaces2( const lump_t *l )
{
	int i;
	int *out;
	short *in;
	int count;

	in = (void *)(cmod_base + l->fileofs);
	count = l->filelen / sizeof(*in);

	cm.leafsurfaces = Hunk_Alloc( count * sizeof( *cm.leafsurfaces ), h_high );
	cm.numLeafSurfaces = count;

	out = cm.leafsurfaces;

	for ( i = 0; i < count; i++, in++, out++ ) {
		*out = LittleLong( *in );
	}
}


/*
=================
CMod_LoadBrushSides
=================
*/
void CMod_LoadBrushSides2 (lump_t *l)
{
	int				i;
	cbrushside_t	*out;
	dBsp2Brushside_t	*in;
	int				count;
	int				num;

	in = (void *)(cmod_base + l->fileofs);
	if ( l->filelen % sizeof(*in) ) {
		Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size" );
	}
	count = l->filelen / sizeof(*in);

	cm.brushsides = Hunk_Alloc( ( BOX_SIDES + count ) * sizeof( *cm.brushsides ), h_high );
	cm.numBrushSides = count;

	out = cm.brushsides;

	for ( i= 0; i < count; i++, in++, out++ ) {
		num = LittleLong( in->planenum );
		out->plane = &cm.planes[num];
		out->shaderNum = LittleLong( in->texinfo );
		if ( out->shaderNum < 0 ) out->shaderNum = 0;
		if ( out->shaderNum < 0 || out->shaderNum >= cm.numShaders ) {
			Com_Error( ERR_DROP, "CMod_LoadBrushSides2: bad shaderNum: %i", out->shaderNum );
		}
		out->surfaceFlags = cm.shaders[out->shaderNum].surfaceFlags;
	}
}


/*
=================
CMod_LoadEntityString
=================
*/
void CMod_LoadEntityString2( lump_t *l, const char *name ) {
	CMod_LoadEntityString(l, name);

	// detect kingpin map entities
	if (strstr(cm.entityString, "\"classname\" \"junior\"") ||
		strstr(cm.entityString, "\"classname\" \"lightflare\"") ||
		strstr(cm.entityString, "\"fogdensity2\""))
	{
		//TODO: bspfile.type = map_kp;
		Com_DPrintf("Kingpin map detected\n");
	}

}


static void DecompressVis(byte *dst, void *vis, int pos, int rowSize)
{
	if (pos == -1)
	{
		memset(dst, 0xFF, rowSize);	// all visible
		dst += rowSize;
		return;
	}

	byte *src = (byte*)vis + pos;
	// decompress vis
	for (int j = rowSize; j; /*empty*/)
	{
		byte c = *src++;
		if (c)
		{	// non-zero byte
			*dst++ = c;
			j--;
		}
		else
		{	// zero byte -- decompress RLE data (with filler 0)
			c = *src++;				// count
			c = min(c, j);			// should not be, but ...
			j -= c;
			while (c--)
				*dst++ = 0;
		}
	}
}

/*
=================
CMod_LoadVisibility
=================
*/
#define	VIS_HEADER	8
void CMod_LoadVisibility2( lump_t *l ) {
	int		len;
	byte	*buf;
	int rowSize;
	dBsp2Vis_t *in;
	in = (void *)(cmod_base + l->fileofs);

	len = cm.numClusters;
	if ( !len ) {
		cm.numClusters = 1;
		cm.clusterBytes = ( cm.numClusters + 31 ) & ~31;
		cm.visibility = Hunk_Alloc( cm.clusterBytes, h_high );
		Com_Memset( cm.visibility, 255, cm.clusterBytes );
		return;
	}
	buf = cmod_base + l->fileofs;

	cm.clusterBytes = rowSize = (cm.numClusters + 7) >> 3;

	cm.vised = qtrue;
	cm.visibility = Hunk_Alloc( rowSize * cm.numClusters , h_high );
	//cm.numClusters = LittleLong( ((int *)buf)[0] );
	//cm.clusterBytes = LittleLong( ((int *)buf)[1] );
	for (int i = 0; i < in->numClusters; i++, cm.visibility += rowSize)
		DecompressVis(cm.visibility, in, in->bitOfs[i][PVS], rowSize);
}

//==================================================================


/*
=================
CMod_LoadPatches
=================
*/
#define	MAX_PATCH_VERTS		1024
void CMod_LoadPatches2( lump_t *surfs, lump_t *verts ) {
	vec3_t	*dv, *dv_p;
	dBsp2Face_t	*in;
	int			count;
	int			i, j;
	int			c;
	cPatch_t	*patch;
	vec3_t		points[MAX_PATCH_VERTS];
	int			shaderNum;

	in = (void *)(cmod_base + surfs->fileofs);

	cm.numSurfaces = count = surfs->filelen / sizeof(*in);
	cm.surfaces = Hunk_Alloc( cm.numSurfaces * sizeof( cm.surfaces[0] ), h_high );

	dv = (void *)(cmod_base + verts->fileofs);

	// scan through all the surfaces, but only load the bsp plane
	for ( i = 0 ; i < count ; i++, in++ ) {
		cm.surfaces[ i ] = patch = Hunk_Alloc( sizeof( *patch ), h_high );

		// load the full drawverts onto the stack
		c = LittleLong( in->numedges );
		if ( c > MAX_PATCH_VERTS ) {
			Com_Error( ERR_DROP, "ParseMesh: MAX_PATCH_VERTS" );
		}

		dv_p = dv + LittleLong( in->firstedge );
		for ( j = 0 ; j < c ; j++, dv_p++ ) {
			points[j][0] = LittleFloat( *dv_p[0] );
			points[j][1] = LittleFloat( *dv_p[1] );
			points[j][2] = LittleFloat( *dv_p[2] );
		}

		shaderNum = LittleLong( in->texinfo );
		patch->contents = cm.shaders[shaderNum].contentFlags;
		patch->surfaceFlags = cm.shaders[shaderNum].surfaceFlags;
		
		if(patch->surfaceFlags & SURF_SKY) {
			continue;
		}

/*

		if (sflags & (SHADER_TRANS33|SHADER_TRANS66|SHADER_ALPHA|SHADER_TURB|SHADER_SKY))
			numVerts = RemoveCollinearPoints(pverts, numVerts);

		* numTriangles = numVerts - 2 (3 verts = 1 tri, 4 verts = 2 tri etc.)
		 * numIndexes = numTriangles * 3
		 * Indexes: (0 1 2) (0 2 3) (0 3 4) ... (here: 5 verts, 3 triangles, 9 indexes)
		int numTris;
		if (shader->tessSize)
		{
			numTris  = SubdividePlane(pverts, numVerts, shader->tessSize);
			numVerts = subdivNumVerts;
		}
		else
			numTris = numVerts - 2;

		int numIndexes = numTris * 3;

		----- Prepare for vertex generation ----------------
		// alloc new surface
		surfacePlanar_t *s = new (map.dataChain) surfacePlanar_t;
		s->shader = shader;
		map.faces[i] = s;
		
		
		s->plane = bspfile.planes[surfs->planenum];
		if (surfs->side)
		{
			// backface (needed for backface culling)
			s->plane.normal.Negate();
			s->plane.dist = -s->plane.dist;
			s->plane.Setup();
		}
		s->numVerts   = numVerts;
		s->verts      = new (map.dataChain) vertex_t [numVerts];
		s->numIndexes = numIndexes;
		s->indexes    = new (map.dataChain) int [numIndexes];

		------------ Generate indexes ----------------------
		if (shader->tessSize)
			GetSubdivideIndexes(s->indexes);
		else
		{
			int *pindex = s->indexes;
			for (j = 0; j < numTris; j++)
			{
				*pindex++ = 0;
				*pindex++ = j+1;
				*pindex++ = j+2;
			}
		}

		--------- Create surface vertexes ------------------
		vertex_t *v = s->verts;
		// ClearBounds2D(mins, maxs)
		float mins[2], maxs[2];				// surface extents
		mins[0] = mins[1] =  BIG_NUMBER;
		maxs[0] = maxs[1] = -BIG_NUMBER;
		// Enumerate vertexes, prepare data for lightmap
		for (j = 0; j < numVerts; j++, v++)
		{
			v->xyz = *pverts[j];
			if (sflags & SHADER_SKY) continue;

			float v1 = dot(v->xyz, stex->vecs[0].vec) + stex->vecs[0].offset;
			float v2 = dot(v->xyz, stex->vecs[1].vec) + stex->vecs[1].offset;
			// Update bounds
			EXPAND_BOUNDS(v1, mins[0], maxs[0]);
			EXPAND_BOUNDS(v2, mins[1], maxs[1]);
			// Texture coordinates
			if (!(sflags & SHADER_TURB)) //?? (!shader->tessSize)
			{
				assert(shader->width > 0 && shader->height > 0);
				v->st[0] = v1 / shader->width;
				v->st[1] = v2 / shader->height;
			}
			else
			{
				v->st[0] = v1 - stex->vecs[0].offset;
				v->st[1] = v2 - stex->vecs[1].offset;
			}
			// save intermediate data for lightmap texcoords
			v->lm[0] = v1;
			v->lm[1] = v2;
			// Vertex color
			v->c.rgba = RGBA(1,1,1,1);
		}

		--------------- Lightmap -------------------
		s->lightmap = NULL;
		if (needLightmap)
			InitSurfaceLightmap2(surfs, s, mins, maxs);
		// special case for q1/hl lightmap without data: dark lightmap
		// (other map formats -- fullbright texture)
		if (darkLightmap)
		{
			dynamicLightmap_t *lm = s->lightmap;
			lm->w = 0;
			lm->h = 0;
			static byte dark[] = { 0, 0, 0 };
			lm->source[0] = dark;		// hack: offset from lighting start
			v = s->verts;
			for (j = 0; j < numVerts; j++, v++)
			{
				v->lm[0] = v->lm[1] = 0.5f;
				v->lm2[0] = v->lm2[1] = 0;
			}
			lm->externalSource = true;
		}

		BuildPlanarSurf(s);
		if (stex->flags & SURF_LIGHT)		//!! + sky when ambient <> 0
		{
			static const color_t defColor = {96, 96, 96, 255};// {255, 255, 255, 255};

			float area = GetSurfArea(s);
			image_t *img = FindImage(va("textures/%s", stex->texture), IMAGE_MIPMAP);
			BuildSurfLight(s, img ? &img->color : &defColor, area, stex->value, (stex->flags & SURF_SKY) != 0);
			if (stex->flags & SURF_AUTOFLARE && !(stex->flags & SURF_SKY))
				BuildSurfFlare(s, img ? &img->color : &defColor, area);
		}

		// free allocated poly
		if (shader->tessSize)
			FreeSubdividedPlane();
	}
*/



		// create the internal facet structure
		//patch->pc = CM_GeneratePatchCollide( width, height, points );
	}
}


static int CheckLump(lump_t *l, char *lumpName, int size)
{
	if (l->filelen % size)
		Com_Error(ERR_DROP, "%s: incorrect lump size (%s)", __func__, lumpName);

	//*ptr = (void *)(cmod_base + l->fileofs);

	return l->filelen / size;
}


void LoadQ2Map(const char *name) {
	dBsp2Hdr_t		header;

	Com_DPrintf("CM_Load: Loading Quake 2 map.\n");
	header = *(dBsp2Hdr_t *)cmod_base;

#if !LITTLE_ENDIAN
	// swap the header
	for (int i = 0; i < sizeof(dBsp2Hdr_t) / 4; i++)
		((int *)cmod_base)[i] = LittleLong(((int *)cmod_base)[i]);
#endif

#define C(num,field,count,type) \
	CheckLump(&header.lumps[LUMP_Q2_##num], XSTRING(num), sizeof(type))
	C(LIGHTING, lighting, lightDataSize, byte);
	C(VERTEXES, vertexes2, numVertexes, vec3_t);
	C(PLANES, planes2, numPlanes, dBsp2Plane_t);
	C(LEAFS, leafs2, numLeafs, dBsp2Leaf_t);
	C(NODES, nodes2, numNodes, dBsp2Node_t);
	C(TEXINFO, texinfo2, numTexinfo, dBsp2Texinfo_t);
	C(FACES, faces2, numFaces, dBsp2Face_t);
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
	CMod_LoadShaders2( &header.lumps[LUMP_Q2_TEXINFO] );
	CMod_LoadLeafs2 (&header.lumps[LUMP_Q2_LEAFS]);
	CMod_LoadLeafBrushes2 (&header.lumps[LUMP_Q2_LEAFBRUSHES]);
	CMod_LoadLeafSurfaces2 (&header.lumps[LUMP_Q2_LEAFFACES]);
	CMod_LoadPlanes2 (&header.lumps[LUMP_Q2_PLANES]);
	CMod_LoadBrushSides2 (&header.lumps[LUMP_Q2_BRUSHSIDES]);
	CMod_LoadBrushes2 (&header.lumps[LUMP_Q2_BRUSHES]);
	CMod_LoadSubmodels2 (&header.lumps[LUMP_Q2_MODELS]);
	CMod_LoadNodes2 (&header.lumps[LUMP_Q2_NODES]);
	CMod_LoadEntityString2 (&header.lumps[LUMP_Q2_ENTITIES], name);
	CMod_LoadVisibility2( &header.lumps[LUMP_Q2_VISIBILITY] );
	// TODO: area portals and area mask stuff
	CMod_LoadAreas( &header.lumps[LUMP_Q2_ZONES] );
	//CMod_LoadAreaPortals( &header.lumps[LUMP_Q2_ZONEPORTALS] );
	//
	//;
	CMod_LoadPatches2( &header.lumps[LUMP_Q2_FACES], &header.lumps[LUMP_Q2_VERTEXES] );

}

#endif
