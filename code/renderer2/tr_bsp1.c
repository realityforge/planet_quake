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
// tr_map.c

#ifdef USE_BSP1

#include "tr_local.h"
#include "../qcommon/cm_load_bsp1.h"

extern void R_LoadSubmodels( lump_t *l );


/*
=================
R_LoadShaders
=================
*/
void R_LoadShaders1( lump_t *l ) {	
	int		i, count;
  dBsp1Texinfo_t *in;
	dshader_t	*out;
  miptex_t *mt;
	
	in = (void *)(fileBase + l->fileofs);
  count = LittleLong ( in->nummiptex );
	out = ri.Hunk_Alloc ( count*sizeof(*out), h_low );

	s_worldData.shaders = out;
	s_worldData.numShaders = count;

  Com_Printf("shader: %i\n", l->fileofs);
	for ( i=0 ; i<count ; i++ ) {
    if (in->dataofs[i] == -1) {
      continue;
    }

    mt = (miptex_t *)((byte *)in + in->dataofs[i]);
    memcpy(out[i].shader, mt->name, 16);
    Com_Printf("text: %s\n", out[i].shader);
		//out[i].surfaceFlags = LittleLong( out[i].surfaceFlags );
		//out[i].contentFlags = LittleLong( out[i].contentFlags );

    /* TODO:
    if()
    #define	CONTENTS_Q1_SOLID		-2
    #define	CONTENTS_Q1_WATER		-3
    #define	CONTENTS_Q1_SLIME		-4
    #define	CONTENTS_Q1_LAVA		-5
    if(in->flags & Q2_SURF_SKY)
			out->surfaceFlags |= SURF_SKY;
		if(in->flags & Q2_SURF_NODRAW)
			out->surfaceFlags |= SURF_NODRAW;
		if (in->flags & MATERIAL_METAL)
			out->surfaceFlags |= SURF_METALSTEPS;
		if (in->flags & MATERIAL_SILENT)
			out->surfaceFlags |= SURF_NOSTEPS;
    */
	}
}


/*
=================
R_LoadPlanes
=================
*/
void R_LoadPlanes1( lump_t *l ) {
	int			i, j;
	cplane_t	*out;
	dBsp1Plane_t 	*in;
	int			count;
	int			bits;
	
	in = (void *)(fileBase + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Error (ERR_DROP, "%s: funny lump size in %s", __func__, s_worldData.name);
	count = l->filelen / sizeof(*in);
	out = ri.Hunk_Alloc ( count*2*sizeof(*out), h_low);	
	
	s_worldData.planes = out;
	s_worldData.numplanes = count;

	for ( i=0 ; i<count ; i++, in++, out++) {
		bits = 0;
		for (j=0 ; j<3 ; j++) {
			out->normal[j] = LittleFloat (in->normal[j]);
			if (out->normal[j] < 0) {
				bits |= 1<<j;
			}
		}

		out->dist = LittleFloat (in->dist);
		out->type = PlaneTypeForNormal( out->normal );
		out->signbits = bits;
	}
}


/*
===============
R_LoadSurfaces
===============
*/
void R_LoadSurfaces1( lump_t *surfs, lump_t *verts, lump_t *indexLump, lump_t *textures ) {
	dBsp1Face_t *in;
  texinfo_t *texinfo;
	msurface_t	*out;
	vec3_t	*dv, *dverts;
	int			*indexes;
	int			count;
	int			numFaces, badTriangles;
	int			i, j;
	float *hdrVertColors = NULL;
  srfBspSurface_t	*cv;
  glIndex_t  *tri;

	numFaces = 0;

	if (surfs->filelen % sizeof(*in))
		ri.Error (ERR_DROP, "%s: funny lump size in %s", __func__, s_worldData.name);
	count = surfs->filelen / sizeof(*in);

	dv = (void *)(fileBase + verts->fileofs);
	if (verts->filelen % sizeof(*dv))
		ri.Error (ERR_DROP, "%s: funny lump size in %s", __func__, s_worldData.name);

	indexes = (void *)(fileBase + indexLump->fileofs);
	if ( indexLump->filelen % sizeof(*indexes))
		ri.Error (ERR_DROP, "%s: funny lump size in %s", __func__, s_worldData.name);

	texinfo = (void *)(fileBase + textures->fileofs);
	if ( textures->filelen % sizeof(*texinfo))
		ri.Error (ERR_DROP, "%s: funny lump size in %s", __func__, s_worldData.name);

	out = ri.Hunk_Alloc ( count * sizeof(*out), h_low );	

	s_worldData.surfaces = out;
	s_worldData.numsurfaces = count;
	s_worldData.surfacesViewCount = ri.Hunk_Alloc ( count * sizeof(*s_worldData.surfacesViewCount), h_low );
	s_worldData.surfacesDlightBits = ri.Hunk_Alloc ( count * sizeof(*s_worldData.surfacesDlightBits), h_low );
	s_worldData.surfacesPshadowBits = ri.Hunk_Alloc ( count * sizeof(*s_worldData.surfacesPshadowBits), h_low );

	// Two passes, allocate surfaces first, then load them full of data
	// This ensures surfaces are close together to reduce L2 cache misses when using VAOs,
	// which don't actually use the verts and indexes
	in = (void *)(fileBase + surfs->fileofs);
	out = s_worldData.surfaces;
	for ( i = 0 ; i < count ; i++, in++, out++ ) {
    out->data = ri.Hunk_Alloc( sizeof(srfBspSurface_t), h_low);
	}

	in = (void *)(fileBase + surfs->fileofs);
	out = s_worldData.surfaces;
	for ( i = 0 ; i < count ; i++, in++, out++ ) {
    //int realLightmapNum = LittleLong( in->lightofs );
    //texinfo_t *texture = (void *)&texinfo[in->texinfo];
    out->shader = tr.defaultShader; //ShaderForShaderNum( texture.nummiptex, FatLightmap(realLightmapNum) );

    cv = (void *)out->data;
    cv->surfaceType = SF_FACE;

    cv->numIndexes = (in->numedges - 2) * 3;
    cv->indexes = ri.Hunk_Alloc(cv->numIndexes * sizeof(cv->indexes[0]), h_low);

    cv->numVerts = in->numedges - 2;
    cv->verts = ri.Hunk_Alloc(cv->numVerts * sizeof(cv->verts[0]), h_low);

    if (cv->numVerts > MAX_FACE_POINTS) {
  		ri.Printf( PRINT_WARNING, "WARNING: MAX_FACE_POINTS exceeded: %i\n", cv->numVerts);
  		cv->numVerts = MAX_FACE_POINTS;
    }

    out->cullinfo.type = CULLINFO_PLANE | CULLINFO_BOX;
    ClearBounds(out->cullinfo.bounds[0], out->cullinfo.bounds[1]);
    dverts = dv + LittleLong(in->firstedge);
    for(j = 0; j < cv->numVerts; j++) {
      vec4_t v;
      //LoadDrawVertToSrfVert(&cv->verts[i], &dverts[i], realLightmapNum, hdrVertColors ? hdrVertColors + (in->firstedge + i) * 3 : NULL, out->cullinfo.bounds);
      cv->verts[j].xyz[0] = LittleFloat(dverts[j][0]);
      cv->verts[j].xyz[1] = LittleFloat(dverts[j][1]);
      cv->verts[j].xyz[2] = LittleFloat(dverts[j][2]);
      AddPointToBounds(cv->verts[j].xyz, out->cullinfo.bounds[0], out->cullinfo.bounds[1]);
      v[0] = LittleFloat(s_worldData.planes[in->planenum].normal[0]);
      v[1] = LittleFloat(s_worldData.planes[in->planenum].normal[1]);
      v[2] = LittleFloat(s_worldData.planes[in->planenum].normal[2]);
      R_VaoPackNormal(cv->verts[j].normal, v);
    }

  	// copy triangles
  	badTriangles = 0;
  	//indexes += LittleLong(in->firstedge);
    printf("first: %i\n", LittleLong(in->firstedge));
  	for(i = 0, tri = cv->indexes; i < cv->numIndexes; i += 3, tri += 3)
  	{
  		for(j = 0; j < 3; j++)
  		{
  			tri[j] = LittleLong(indexes[i + j]);

  			if(tri[j] >= cv->numVerts)
  			{
  				ri.Printf(PRINT_WARNING, "Bad index in face surface");
  				tri[j] = 0;
  			}
  		}

  		if ((tri[0] == tri[1]) || (tri[1] == tri[2]) || (tri[0] == tri[2]))
  		{
  			tri -= 3;
  			badTriangles++;
  		}
  	}
  
  	if (badTriangles)
  	{
  		ri.Printf(PRINT_WARNING, "Face has bad triangles, originally shader %s %d tris %d verts, now %d tris\n", out->shader->name, cv->numIndexes / 3, cv->numVerts, cv->numIndexes / 3 - badTriangles);
  		cv->numIndexes -= badTriangles * 3;
  	}

    numFaces++;
	}

	if (hdrVertColors)
	{
		ri.FS_FreeFile(hdrVertColors);
	}

	R_FixSharedVertexLodError();

	ri.Printf( PRINT_ALL, "...loaded %d faces\n", numFaces );
}


/*
=================
R_LoadMarksurfaces
=================
*/
void R_LoadMarksurfaces1 (lump_t *l)
{	
	int		i, j, count;
	short		*in;
	int     *out;
	
	in = (void *)(fileBase + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Error (ERR_DROP, "%s: funny lump size in %s", __func__, s_worldData.name);
	count = l->filelen / sizeof(*in);
	out = ri.Hunk_Alloc ( count*sizeof(*out), h_low);	

	s_worldData.marksurfaces = out;
	s_worldData.nummarksurfaces = count;

	for ( i=0 ; i<count ; i++)
	{
		j = LittleLong(in[i]);
		out[i] = j;
	}
}


/*
=================
R_LoadNodesAndLeafs
=================
*/
void R_LoadNodesAndLeafs1 (lump_t *nodeLump, lump_t *leafLump) {
	int			i, j, p;
	dBsp1Node_t		*in;
	dBsp1Leaf_t		*inLeaf;
	mnode_t 	*out;
	int			numNodes, numLeafs;

	in = (void *)(fileBase + nodeLump->fileofs);
	if (nodeLump->filelen % sizeof(*in) ||
		leafLump->filelen % sizeof(*inLeaf) ) {
		ri.Error (ERR_DROP, "%s: funny lump size in %s", __func__, s_worldData.name);
	}
	numNodes = nodeLump->filelen / sizeof(dnode_t);
	numLeafs = leafLump->filelen / sizeof(dleaf_t);

	out = ri.Hunk_Alloc ( (numNodes + numLeafs) * sizeof(*out), h_low);	

	s_worldData.nodes = out;
	s_worldData.numnodes = numNodes + numLeafs;
	s_worldData.numDecisionNodes = numNodes;

	// load nodes
	for ( i=0 ; i<numNodes; i++, in++, out++)
	{
		for (j=0 ; j<3 ; j++)
		{
			out->mins[j] = LittleLong (in->mins[j]);
			out->maxs[j] = LittleLong (in->maxs[j]);
		}
	
		p = LittleLong(in->planenum);
		out->plane = s_worldData.planes + p;

		out->contents = CONTENTS_NODE;	// differentiate from leafs

		for (j=0 ; j<2 ; j++)
		{
			p = LittleLong (in->children[j]);
			if (p >= 0)
				out->children[j] = s_worldData.nodes + p;
			else
				out->children[j] = s_worldData.nodes + numNodes + (-1 - p);
		}
	}
	
	// load leafs
	inLeaf = (void *)(fileBase + leafLump->fileofs);
	for ( i=0 ; i<numLeafs ; i++, inLeaf++, out++)
	{
		for (j=0 ; j<3 ; j++)
		{
			out->mins[j] = LittleLong (inLeaf->mins[j]);
			out->maxs[j] = LittleLong (inLeaf->maxs[j]);
		}

		out->cluster = LittleLong(inLeaf->contents);
		out->area = -1; //LittleLong(inLeaf->visofs);

		if ( out->contents >= s_worldData.numClusters ) {
			s_worldData.numClusters = out->contents + 1;
		}

		out->firstmarksurface = LittleLong(inLeaf->firstmarksurface);
		out->nummarksurfaces = LittleLong(inLeaf->nummarksurfaces);
	}	

	// chain descendants
	R_SetParent (s_worldData.nodes, NULL);
}


/*
=================
R_LoadSubmodels
=================
*/
void R_LoadSubmodels1( lump_t *l ) {
	dBsp1Model_t	*in;
	bmodel_t	*out;
	int			i, j, count;

	in = (void *)(fileBase + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Error (ERR_DROP, "%s: funny lump size in %s", __func__, s_worldData.name);
	count = l->filelen / sizeof(*in);

	s_worldData.numBModels = count;
	s_worldData.bmodels = out = ri.Hunk_Alloc( count * sizeof(*out), h_low );

	for ( i=0 ; i<count ; i++, in++, out++ ) {
		model_t *model;

		model = R_AllocModel();

		assert( model != NULL );			// this should never happen
		if ( model == NULL ) {
			ri.Error(ERR_DROP, "R_LoadSubmodels: R_AllocModel() failed");
		}

		model->type = MOD_BRUSH;
		model->bmodel = out;
		Com_sprintf( model->name, sizeof( model->name ), "*%d", i );

		for (j=0 ; j<3 ; j++) {
			//out->bounds[0][j] = LittleFloat (in->origin[j]) + LittleFloat (in->mins[j]);
			//out->bounds[1][j] = LittleFloat (in->origin[j]) + LittleFloat (in->maxs[j]);
      out->bounds[0][j] = LittleFloat (in->mins[j]);
			out->bounds[1][j] = LittleFloat (in->maxs[j]);
		}

		out->firstSurface = LittleLong( in->firstface );
		out->numSurfaces = LittleLong( in->numfaces );

		if(i == 0)
		{
			// Add this for limiting VAO surface creation
			s_worldData.numWorldSurfaces = out->numSurfaces;
		}
	}
}


void LoadBsp1(const char *name) {
	int i;
	dBsp1Hdr_t	*header;
	header = (dBsp1Hdr_t *)fileBase;

	// swap all the lumps
	for (i=0; i < ARRAY_LEN(header->lumps); i++) {
		((int *)header)[i] = LittleLong ( ((int *)header)[i]);
	}
  /*
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
  */
  
	// load into heap
	R_LoadEntities( &header->lumps[LUMP_Q1_ENTITIES] );
	R_LoadShaders1( &header->lumps[LUMP_Q1_TEXTURES] );
	R_LoadPlanes1( &header->lumps[LUMP_Q1_PLANES] );
	R_LoadSurfaces1( &header->lumps[LUMP_Q1_FACES], &header->lumps[LUMP_Q1_VERTEXES], 
    &header->lumps[LUMP_Q1_SURFEDGES], &header->lumps[LUMP_Q1_TEXINFO] );
  R_LoadMarksurfaces1 (&header->lumps[LUMP_Q1_MARKSURFACES]);
  R_LoadNodesAndLeafs1 (&header->lumps[LUMP_Q1_NODES], &header->lumps[LUMP_Q1_LEAFS]);
	R_LoadSubmodels1 (&header->lumps[LUMP_Q1_MODELS]);
}

#endif
