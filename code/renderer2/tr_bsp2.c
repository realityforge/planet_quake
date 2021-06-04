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

#ifdef USE_BSP2


#include "tr_local.h"
#include "../qcommon/cm_load_bsp2.h"

/*

Loads and prepares a map file for scene rendering.

A single entry point:

void RE_LoadWorldMap( const char *name );

*/


/*
===============
R_LoadLightmaps

===============
*/
#define	DEFAULT_LIGHTMAP_SIZE	128
static	void R_LoadLightmaps2( lump_t *l, lump_t *surfs ) {
	imgFlags_t  imgFlags = IMGFLAG_NOLIGHTSCALE | IMGFLAG_NO_COMPRESSION | IMGFLAG_CLAMPTOEDGE;
	byte		*buf, *buf_p;
	dBspFace_t  *surf;
	int			len;
	byte		*image;
	int			i, j, numLightmaps, textureInternalFormat = 0;
	int			numLightmapsPerPage = 16;
	float maxIntensity = 0;
	double sumIntensity = 0;

	// if we are in r_vertexLight mode, we don't need the lightmaps at all
	if ( ( r_vertexLight->integer && tr.vertexLightingAllowed ) || glConfig.hardwareType == GLHW_PERMEDIA2 ) {
		return;
	}

	len = l->filelen;
	if ( !len ) {
		return;
	}
	buf = fileBase + l->fileofs;

	// we are about to upload textures
	R_IssuePendingRenderCommands();

	s_worldData.lightmapSize = tr.lightmapSize = DEFAULT_LIGHTMAP_SIZE;
	numLightmaps = len / (tr.lightmapSize * tr.lightmapSize * 3);

	// check for deluxe mapping
	if (numLightmaps <= 1)
	{
		tr.worldDeluxeMapping = qfalse;
	}
	else
	{
		tr.worldDeluxeMapping = qtrue;
		for( i = 0, surf = (dBspFace_t *)(fileBase + surfs->fileofs);
			i < surfs->filelen / sizeof(dBspFace_t); i++, surf++ ) {
			int lightmapNum = LittleLong( surf->lightofs );

			if ( lightmapNum >= 0 && (lightmapNum & 1) != 0 ) {
				tr.worldDeluxeMapping = qfalse;
				break;
			}
		}
	}

	image = ri.Malloc(tr.lightmapSize * tr.lightmapSize * 4 * 2);

	if (tr.worldDeluxeMapping)
		numLightmaps >>= 1;

	// Use fat lightmaps of an appropriate size.
	if (r_mergeLightmaps->integer)
	{
		int maxLightmapsPerAxis = glConfig.maxTextureSize / tr.lightmapSize;
		int lightmapCols = 4, lightmapRows = 4;

		// Increase width at first, then height.
		while (lightmapCols * lightmapRows < numLightmaps && lightmapCols != maxLightmapsPerAxis)
			lightmapCols <<= 1;

		while (lightmapCols * lightmapRows < numLightmaps && lightmapRows != maxLightmapsPerAxis)
			lightmapRows <<= 1;

		tr.fatLightmapCols  = lightmapCols;
		tr.fatLightmapRows  = lightmapRows;
		numLightmapsPerPage = lightmapCols * lightmapRows;

		s_worldData.numLightmaps = tr.numLightmaps = (numLightmaps + (numLightmapsPerPage - 1)) / numLightmapsPerPage;
	}
	else
	{
		s_worldData.numLightmaps = tr.numLightmaps = numLightmaps;
	}

	s_worldData.lightmaps = tr.lightmaps = ri.Hunk_Alloc( tr.numLightmaps * sizeof(image_t *), h_low );

	if (tr.worldDeluxeMapping)
		tr.deluxemaps = ri.Hunk_Alloc( tr.numLightmaps * sizeof(image_t *), h_low );

	textureInternalFormat = GL_RGBA8;
	if (r_hdr->integer)
	{
		// Check for the first hdr lightmap, if it exists, use GL_RGBA16 for textures.
		char filename[MAX_QPATH];

		Com_sprintf(filename, sizeof(filename), "maps/%s/lm_0000.hdr", s_worldData.baseName);
		if (ri.FS_FileExists(filename))
			textureInternalFormat = GL_RGBA16;
	}

	if (r_mergeLightmaps->integer)
	{
		int width  = tr.fatLightmapCols * tr.lightmapSize;
		int height = tr.fatLightmapRows * tr.lightmapSize;

		for (i = 0; i < tr.numLightmaps; i++)
		{
			tr.lightmaps[i] = R_CreateImage(va("_fatlightmap%d", i), NULL, width, height, IMGTYPE_COLORALPHA, imgFlags, textureInternalFormat);

			if (tr.worldDeluxeMapping)
				tr.deluxemaps[i] = R_CreateImage(va("_fatdeluxemap%d", i), NULL, width, height, IMGTYPE_DELUXE, imgFlags, 0);
		}
	}

	for(i = 0; i < numLightmaps; i++)
	{
		int xoff = 0, yoff = 0;
		int lightmapnum = i;
		// expand the 24 bit on-disk to 32 bit

		if (r_mergeLightmaps->integer)
		{
			int lightmaponpage = i % numLightmapsPerPage;
			xoff = (lightmaponpage % tr.fatLightmapCols) * tr.lightmapSize;
			yoff = (lightmaponpage / tr.fatLightmapCols) * tr.lightmapSize;

			lightmapnum /= numLightmapsPerPage;
		}

		// if (tr.worldLightmapping)
		{
			char filename[MAX_QPATH];
			byte *hdrLightmap = NULL;
			int size = 0;

			// look for hdr lightmaps
			if (textureInternalFormat == GL_RGBA16)
			{
				Com_sprintf( filename, sizeof( filename ), "maps/%s/lm_%04d.hdr", s_worldData.baseName, i * (tr.worldDeluxeMapping ? 2 : 1) );
				//ri.Printf(PRINT_ALL, "looking for %s\n", filename);

				size = ri.FS_ReadFile(filename, (void **)&hdrLightmap);
			}

			if (hdrLightmap)
			{
				byte *p = hdrLightmap, *end = hdrLightmap + size;
				//ri.Printf(PRINT_ALL, "found!\n");
				
				/* FIXME: don't just skip over this header and actually parse it */
				while (p < end && !(*p == '\n' && *(p+1) == '\n'))
					p++;

				p += 2;
				
				while (p < end && !(*p == '\n'))
					p++;

				p++;

				if (p >= end)
					ri.Error(ERR_DROP, "Bad header for %s!", filename);

				buf_p = p;

#if 0 // HDRFILE_RGBE
				if ((int)(end - hdrLightmap) != tr.lightmapSize * tr.lightmapSize * 4)
					ri.Error(ERR_DROP, "Bad size for %s (%i)!", filename, size);
#else // HDRFILE_FLOAT
				if ((int)(end - hdrLightmap) != tr.lightmapSize * tr.lightmapSize * 12)
					ri.Error(ERR_DROP, "Bad size for %s (%i)!", filename, size);
#endif
			}
			else
			{
				int imgOffset = tr.worldDeluxeMapping ? i * 2 : i;
				buf_p = buf + imgOffset * tr.lightmapSize * tr.lightmapSize * 3;
			}

			for ( j = 0 ; j < tr.lightmapSize * tr.lightmapSize; j++ ) 
			{
				if (hdrLightmap)
				{
					vec4_t color;

#if 0 // HDRFILE_RGBE
					float exponent = exp2(buf_p[j*4+3] - 128);

					color[0] = buf_p[j*4+0] * exponent;
					color[1] = buf_p[j*4+1] * exponent;
					color[2] = buf_p[j*4+2] * exponent;
#else // HDRFILE_FLOAT
					memcpy(color, &buf_p[j*12], 12);

					color[0] = LittleFloat(color[0]);
					color[1] = LittleFloat(color[1]);
					color[2] = LittleFloat(color[2]);
#endif
					color[3] = 1.0f;

					R_ColorShiftLightingFloats(color, color);

					ColorToRGB16(color, (uint16_t *)(&image[j * 8]));
					((uint16_t *)(&image[j * 8]))[3] = 65535;
				}
				else if (textureInternalFormat == GL_RGBA16)
				{
					vec4_t color;

					//hack: convert LDR lightmap to HDR one
					color[0] = MAX(buf_p[j*3+0], 0.499f);
					color[1] = MAX(buf_p[j*3+1], 0.499f);
					color[2] = MAX(buf_p[j*3+2], 0.499f);

					// if under an arbitrary value (say 12) grey it out
					// this prevents weird splotches in dimly lit areas
					if (color[0] + color[1] + color[2] < 12.0f)
					{
						float avg = (color[0] + color[1] + color[2]) * 0.3333f;
						color[0] = avg;
						color[1] = avg;
						color[2] = avg;
					}
					color[3] = 1.0f;

					R_ColorShiftLightingFloats(color, color);

					ColorToRGB16(color, (uint16_t *)(&image[j * 8]));
					((uint16_t *)(&image[j * 8]))[3] = 65535;
				}
				else
				{
					if ( r_lightmap->integer == 2 )
					{	// color code by intensity as development tool	(FIXME: check range)
						float r = buf_p[j*3+0];
						float g = buf_p[j*3+1];
						float b = buf_p[j*3+2];
						float intensity;
						float out[3] = {0.0, 0.0, 0.0};

						intensity = 0.33f * r + 0.685f * g + 0.063f * b;

						if ( intensity > 255 )
							intensity = 1.0f;
						else
							intensity /= 255.0f;

						if ( intensity > maxIntensity )
							maxIntensity = intensity;

						HSVtoRGB( intensity, 1.00, 0.50, out );

						image[j*4+0] = out[0] * 255;
						image[j*4+1] = out[1] * 255;
						image[j*4+2] = out[2] * 255;
						image[j*4+3] = 255;

						sumIntensity += intensity;
					}
					else
					{
						R_ColorShiftLightingBytes( &buf_p[j*3], &image[j*4] );
						image[j*4+3] = 255;
					}
				}
			}

			if (r_mergeLightmaps->integer)
				R_UpdateSubImage(tr.lightmaps[lightmapnum], image, xoff, yoff, tr.lightmapSize, tr.lightmapSize, textureInternalFormat);
			else
#ifdef USE_MULTIVM_CLIENT
  			s_worldData.lightmaps[i] = tr.lightmaps[i] = R_CreateImage(va("*lightmap_%d_%d", rwi, i), image, tr.lightmapSize, tr.lightmapSize, IMGTYPE_COLORALPHA, imgFlags, textureInternalFormat );
#else
        s_worldData.lightmaps[i] = tr.lightmaps[i] = R_CreateImage(va("*lightmap_%d", i), image, tr.lightmapSize, tr.lightmapSize, IMGTYPE_COLORALPHA, imgFlags, textureInternalFormat );
#endif

			if (hdrLightmap)
				ri.FS_FreeFile(hdrLightmap);
		}

		if (tr.worldDeluxeMapping)
		{
			buf_p = buf + (i * 2 + 1) * tr.lightmapSize * tr.lightmapSize * 3;

			for ( j = 0 ; j < tr.lightmapSize * tr.lightmapSize; j++ ) {
				image[j*4+0] = buf_p[j*3+0];
				image[j*4+1] = buf_p[j*3+1];
				image[j*4+2] = buf_p[j*3+2];

				// make 0,0,0 into 127,127,127
				if ((image[j*4+0] == 0) && (image[j*4+1] == 0) && (image[j*4+2] == 0))
				{
					image[j*4+0] =
					image[j*4+1] =
					image[j*4+2] = 127;
				}

				image[j*4+3] = 255;
			}

			if (r_mergeLightmaps->integer)
				R_UpdateSubImage(tr.deluxemaps[lightmapnum], image, xoff, yoff, tr.lightmapSize, tr.lightmapSize, GL_RGBA8 );
			else
				tr.deluxemaps[i] = R_CreateImage(va("*deluxemap%d", i), image, tr.lightmapSize, tr.lightmapSize, IMGTYPE_DELUXE, imgFlags, 0 );
		}
	}

	if ( r_lightmap->integer == 2 )	{
		ri.Printf( PRINT_ALL, "Brightest lightmap value: %d\n", ( int ) ( maxIntensity * 255 ) );
	}

	ri.Free(image);
}


/*
=================
R_LoadVisibility
=================
*/
static	void R_LoadVisibility2( lump_t *l ) {
	int		len;
	byte	*buf;

	len = l->filelen;
	if ( !len ) {
		return;
	}
	buf = fileBase + l->fileofs;

	s_worldData.numClusters = LittleLong( ((int *)buf)[0] );
	s_worldData.clusterBytes = LittleLong( ((int *)buf)[1] );

	// CM_Load should have given us the vis data to share, so
	// we don't need to allocate another copy
	if ( tr.externalVisData ) {
		s_worldData.vis = tr.externalVisData;
	} else {
		byte	*dest;

		dest = ri.Hunk_Alloc( len - 8, h_low );
		Com_Memcpy( dest, buf + 8, len - 8 );
		s_worldData.vis = dest;
	}
}

//===============================================================================


void LoadVertToSrfVert(srfVert_t *s, vec3_t *d, float hdrVertColors[3], vec3_t *bounds)
{
	memcpy(&s->xyz, d, sizeof(s->xyz));

	if (bounds)
		AddPointToBounds(s->xyz, bounds[0], bounds[1]);
}



/*
===============
ParseMesh
===============
*/
static void ParseMesh ( dBspFace_t *ds, vec3_t *verts, float *hdrVertColors, msurface_t *surf ) {
	srfBspSurface_t	*grid = (srfBspSurface_t *)surf->data;
	int				i;
	int				numPoints;
	srfVert_t points[MAX_PATCH_SIZE*MAX_PATCH_SIZE];
	vec3_t			bounds[2];
	vec3_t			tmpVec;
	static surfaceType_t	skipData = SF_SKIP;

	// get shader value
	surf->shader = ShaderForShaderNum( ds->texinfo, FatLightmap(0) );
	if ( r_singleShader->integer && !surf->shader->isSky ) {
		surf->shader = tr.defaultShader;
	}

	// we may have a nodraw surface, because they might still need to
	// be around for movement clipping
	if ( s_worldData.shaders[ LittleLong( ds->texinfo ) ].surfaceFlags & SURF_NODRAW ) {
		surf->data = &skipData;
		return;
	}

	verts += LittleLong( ds->firstedge );
	numPoints = LittleLong( ds->numedges );
	for(i = 0; i < numPoints; i++)
		LoadVertToSrfVert(&points[i], &verts[i], hdrVertColors ? hdrVertColors + (ds->firstedge + i) * 3 : NULL, NULL);

	VectorAdd( bounds[0], bounds[1], bounds[1] );
	VectorScale( bounds[1], 0.5f, grid->lodOrigin );
	VectorSubtract( bounds[0], grid->lodOrigin, tmpVec );
	grid->lodRadius = VectorLength( tmpVec );

	surf->cullinfo.type = CULLINFO_BOX | CULLINFO_SPHERE;
	VectorCopy(grid->cullBounds[0], surf->cullinfo.bounds[0]);
	VectorCopy(grid->cullBounds[1], surf->cullinfo.bounds[1]);
	VectorCopy(grid->cullOrigin, surf->cullinfo.localOrigin);
	surf->cullinfo.radius = grid->cullRadius;
}


/*
===============

===============
*/
static	void R_LoadSurfaces2( lump_t *surfs, lump_t *verts ) {
	dBspFace_t	*in;
	msurface_t	*out;
	vec3_t	*dv;
	//int			*indexes;
	int			count;
	int			numFaces, numMeshes, numTriSurfs, numFlares;
	int			i;
	float *hdrVertColors = NULL;

	numFaces = 0;
	numMeshes = 0;
	numTriSurfs = 0;
	numFlares = 0;

	if (surfs->filelen % sizeof(*in))
		ri.Error (ERR_DROP, "R_LoadSurfaces2: funny lump size in %s",s_worldData.name);
	count = surfs->filelen / sizeof(*in);

	dv = (void *)(fileBase + verts->fileofs);
	if (verts->filelen % sizeof(*dv))
		ri.Error (ERR_DROP, "R_LoadSurfaces2: funny lump size in %s",s_worldData.name);

/*
	indexes = (void *)(fileBase + indexLump->fileofs);
	if ( indexLump->filelen % sizeof(*indexes))
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name);
*/

	out = ri.Hunk_Alloc ( count * sizeof(*out), h_low );	

	s_worldData.surfaces = out;
	s_worldData.numsurfaces = count;
	s_worldData.surfacesViewCount = ri.Hunk_Alloc ( count * sizeof(*s_worldData.surfacesViewCount), h_low );
	s_worldData.surfacesDlightBits = ri.Hunk_Alloc ( count * sizeof(*s_worldData.surfacesDlightBits), h_low );
	s_worldData.surfacesPshadowBits = ri.Hunk_Alloc ( count * sizeof(*s_worldData.surfacesPshadowBits), h_low );

	// load hdr vertex colors
	if (r_hdr->integer)
	{
		char filename[MAX_QPATH];
		int size;

		Com_sprintf( filename, sizeof( filename ), "maps/%s/vertlight.raw", s_worldData.baseName);
		//ri.Printf(PRINT_ALL, "looking for %s\n", filename);

		size = ri.FS_ReadFile(filename, (void **)&hdrVertColors);

		if (hdrVertColors)
		{
			//ri.Printf(PRINT_ALL, "Found!\n");
			if (size != sizeof(float) * 3 * (verts->filelen / sizeof(*dv)))
				ri.Error(ERR_DROP, "Bad size for %s (%i, expected %i)!", filename, size, (int)((sizeof(float)) * 3 * (verts->filelen / sizeof(*dv))));
		}
	}


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
		ParseMesh ( in, dv, hdrVertColors, out );
		numMeshes++;
	}

	if (hdrVertColors)
	{
		ri.FS_FreeFile(hdrVertColors);
	}

#ifdef PATCH_STITCHING
	R_StitchAllPatches();
#endif

	R_FixSharedVertexLodError();

#ifdef PATCH_STITCHING
	R_MovePatchSurfacesToHunk();
#endif

	ri.Printf( PRINT_ALL, "...loaded %d faces, %i meshes, %i trisurfs, %i flares\n", 
		numFaces, numMeshes, numTriSurfs, numFlares );
}



/*
=================
R_LoadSubmodels
=================
*/
static	void R_LoadSubmodels2( lump_t *l ) {
	dBsp2Model_t	*in;
	bmodel_t	*out;
	int			i, j, count;

	in = (void *)(fileBase + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Error (ERR_DROP, "R_LoadSubmodels2: funny lump size in %s",s_worldData.name);
	count = l->filelen / sizeof(*in);

	s_worldData.numBModels = count;
	s_worldData.bmodels = out = ri.Hunk_Alloc( count * sizeof(*out), h_low );

	for ( i=0 ; i<count ; i++, in++, out++ ) {
		model_t *model;

		model = R_AllocModel();

		assert( model != NULL );			// this should never happen
		if ( model == NULL ) {
			ri.Error(ERR_DROP, "R_LoadSubmodels2: R_AllocModel() failed");
		}

		model->type = MOD_BRUSH;
		model->bmodel = out;
		Com_sprintf( model->name, sizeof( model->name ), "*%d", i );

		for (j=0 ; j<3 ; j++) {
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



//==================================================================

/*
=================
R_LoadNodesAndLeafs
=================
*/
static	void R_LoadNodesAndLeafs2 (lump_t *nodeLump, lump_t *leafLump) {
	int			i, j, p;
	dBsp2Node_t		*in;
	dBsp2Leaf_t		*inLeaf;
	mnode_t 	*out;
	int			numNodes, numLeafs;

	in = (void *)(fileBase + nodeLump->fileofs);
	if (nodeLump->filelen % sizeof(dBsp2Node_t) ||
		leafLump->filelen % sizeof(dBsp2Leaf_t) ) {
		ri.Error (ERR_DROP, "R_LoadNodesAndLeafs: funny lump size in %s",s_worldData.name);
	}
	numNodes = nodeLump->filelen / sizeof(dBsp2Node_t);
	numLeafs = leafLump->filelen / sizeof(dBsp2Leaf_t);

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
	
		p = LittleLong(in->planeNum);
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

		out->cluster = LittleLong(inLeaf->cluster);
		out->area = LittleLong(inLeaf->zone);

		if ( out->cluster >= s_worldData.numClusters ) {
			s_worldData.numClusters = out->cluster + 1;
		}

		out->firstmarksurface = LittleLong(inLeaf->firstleafface);
		out->nummarksurfaces = LittleLong(inLeaf->numleaffaces);
	}	

	// chain descendants
	R_SetParent (s_worldData.nodes, NULL);
}

//=============================================================================


/*
=================
R_LoadShaders
=================
*/
static	void R_LoadShaders2( lump_t *l ) {	
	int		i, count;
	dBsp2Texinfo_t	*in;
	dshader_t *out;
	
	in = (void *)(fileBase + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Error (ERR_DROP, "R_LoadShaders2: funny lump size in %s",s_worldData.name);
	count = l->filelen / sizeof(*in);
	out = ri.Hunk_Alloc ( count*sizeof(*out), h_low );

	s_worldData.shaders = out;
	s_worldData.numShaders = count;

	for ( i=0 ; i<count ; i++ ) {
		Com_Memcpy(out[i].shader, va("textures/%s", in[i].texture), sizeof(out[i].shader));

		out[i].contentFlags |= CONTENTS_SOLID;
		out[i].surfaceFlags |= (in[i].flags & Q2_SURF_ALPHA ? (SURF_ALPHASHADOW | SURF_NOLIGHTMAP) : 0);
		out[i].contentFlags |= (in[i].flags & Q2_SURF_TRANS33 ? CONTENTS_TRANSLUCENT : 0);
		out[i].contentFlags |= (in[i].flags & Q2_SURF_TRANS66 ? CONTENTS_TRANSLUCENT : 0);
		out[i].surfaceFlags |= (in[i].flags & Q2_SURF_FLOWING ? SURF_SLICK : 0);
		out[i].surfaceFlags |= (in[i].flags & Q2_SURF_SKY ? SURF_SKY : 0);
		out[i].contentFlags |= (in[i].flags & Q2_SURF_WARP ? CONTENTS_TELEPORTER : 0);
		out[i].contentFlags |= (in[i].flags & Q2_SURF_SPECULAR ? SURF_POINTLIGHT : 0);
		out[i].surfaceFlags |= (in[i].flags & Q2_SURF_DIFFUSE ? SURF_LIGHTFILTER : 0);
	}
}


/*
=================
R_LoadMarksurfaces
=================
*/
static	void R_LoadMarksurfaces2 (lump_t *l)
{	
	int		i, j, count;
	short		*in;
	int     *out;
	
	in = (void *)(fileBase + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData.name);
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
R_LoadPlanes
=================
*/
static	void R_LoadPlanes2( lump_t *l ) {
	int			i, j;
	cplane_t	*out;
	dBsp2Plane_t 	*in;
	int			count;
	int			bits;
	
	in = (void *)(fileBase + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Error (ERR_DROP, "R_LoadPlanes: funny lump size in %s",s_worldData.name);
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
=================
R_LoadFogs

=================
*/
static	void R_LoadFogs2( lump_t *brushesLump, lump_t *sidesLump ) {
	int			count;
	count = 0;
	s_worldData.numfogs = count + 1;
	s_worldData.fogs = ri.Hunk_Alloc ( s_worldData.numfogs*sizeof(fog_t), h_low);
}

/*


static surfaceBase_t *LoadFakePlanarSurface3(const dBsp3Face_t *surf, dBsp3Vert_t *verts, unsigned *indexes,
	shader_t *shader)
{
	surfaceTrisurf_t *s = new (map.dataChain) surfaceTrisurf_t;
	s->shader = shader;
#if 1
	if (shader->type != SHADERTYPE_SKY && shader->lightValue)
		appWPrintf("Trisurf light: %s\n", *shader->Name);
#endif

	s->numVerts   = surf->numVerts;
	s->verts      = new (map.dataChain) vertexNormal_t [surf->numVerts];
	s->numIndexes = surf->numIndexes;
	s->indexes    = new (map.dataChain) int [surf->numIndexes];
	s->fogNum     = (surf->fogNum + 1) & 255;
	// copy verts
	vertexNormal_t *dst = s->verts;
	s->bounds.Clear();
	for (int j = 0; j < surf->numVerts; j++, verts++, dst++)
	{
		dst->xyz    = verts->v;
		dst->st[0]  = verts->st[0];
		dst->st[1]  = verts->st[1];
		dst->lm[0]  = verts->lm[0];
		dst->lm[1]  = verts->lm[1];
		dst->c.rgba = verts->c.rgba;		//!! saturate
		dst->normal = verts->normal;
		s->bounds.Expand(dst->xyz);
	}
	// copy indexes
	memcpy(s->indexes, indexes, surf->numIndexes * sizeof(int));
	return s;
}


static surfaceBase_t *LoadPlanarSurface3(const dBsp3Face_t *surf, dBsp3Vert_t *verts, unsigned *indexes,
	const dBsp3Shader_t *stex)
{
	unsigned sflags = SHADER_WALL;
	shader_t *shader = CreateSurfShader3(sflags, stex, surf->lightmapNum);

	if (surf->lightmapVecs[2][0] == 0 &&
		surf->lightmapVecs[2][1] == 0 &&
		surf->lightmapVecs[2][2] == 0)
	{
		// Third-party q3map2 compiler has special surface type: pre-tesselated patch.
		// It is stored in bsp as planar surface, but really it is not planar. Create
		// trisurf from it.
		return LoadFakePlanarSurface3(surf, verts, indexes, shader);
	}

*/

void LoadBsp2(const char *name) {
	int i;
	dheader_t	*header;
	header = (dheader_t *)fileBase;

	// swap all the lumps
	for (i=0 ; i<sizeof(dheader_t)/4 ; i++) {
		((int *)header)[i] = LittleLong ( ((int *)header)[i]);
	}

	// load into heap
	R_LoadEntities( &header->lumps[LUMP_Q2_ENTITIES] );
	R_LoadShaders2( &header->lumps[LUMP_Q2_TEXINFO] );
	R_LoadLightmaps2( &header->lumps[LUMP_Q2_LIGHTING], &header->lumps[LUMP_Q2_FACES] );
	R_LoadPlanes2 (&header->lumps[LUMP_Q2_PLANES]);
	R_LoadFogs2( &header->lumps[LUMP_Q2_BRUSHES], &header->lumps[LUMP_Q2_BRUSHSIDES] );
	R_LoadSurfaces2( &header->lumps[LUMP_Q2_FACES], &header->lumps[LUMP_Q2_VERTEXES] );
	R_LoadMarksurfaces2 (&header->lumps[LUMP_Q2_LEAFFACES]);
	R_LoadNodesAndLeafs2 (&header->lumps[LUMP_Q2_NODES], &header->lumps[LUMP_Q2_LEAFS]);
	R_LoadSubmodels2 (&header->lumps[LUMP_Q2_MODELS]);
	R_LoadVisibility2( &header->lumps[LUMP_Q2_VISIBILITY] );
	//R_LoadLightGrid( &header->lumps[LUMP_Q2_LIGHTING] );

}

#endif
