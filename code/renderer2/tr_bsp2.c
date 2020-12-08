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
static	void R_LoadLightmaps( lump_t *l, lump_t *surfs ) {
	imgFlags_t  imgFlags = IMGFLAG_NOLIGHTSCALE | IMGFLAG_NO_COMPRESSION | IMGFLAG_CLAMPTOEDGE;
	byte		*buf, *buf_p;
	dsurface_t  *surf;
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

	s_worldData[rw].lightmapSize = tr.lightmapSize = DEFAULT_LIGHTMAP_SIZE;
	numLightmaps = len / (tr.lightmapSize * tr.lightmapSize * 3);

	// check for deluxe mapping
	if (numLightmaps <= 1)
	{
		tr.worldDeluxeMapping = qfalse;
	}
	else
	{
		tr.worldDeluxeMapping = qtrue;
		for( i = 0, surf = (dsurface_t *)(fileBase + surfs->fileofs);
			i < surfs->filelen / sizeof(dsurface_t); i++, surf++ ) {
			int lightmapNum = LittleLong( surf->lightmapNum );

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

		s_worldData[rw].numLightmaps = tr.numLightmaps = (numLightmaps + (numLightmapsPerPage - 1)) / numLightmapsPerPage;
	}
	else
	{
		s_worldData[rw].numLightmaps = tr.numLightmaps = numLightmaps;
	}

	s_worldData[rw].lightmaps = tr.lightmaps = ri.Hunk_Alloc( tr.numLightmaps * sizeof(image_t *), h_low );

	if (tr.worldDeluxeMapping)
		tr.deluxemaps = ri.Hunk_Alloc( tr.numLightmaps * sizeof(image_t *), h_low );

	textureInternalFormat = GL_RGBA8;
	if (r_hdr->integer)
	{
		// Check for the first hdr lightmap, if it exists, use GL_RGBA16 for textures.
		char filename[MAX_QPATH];

		Com_sprintf(filename, sizeof(filename), "maps/%s/lm_0000.hdr", s_worldData[rw].baseName);
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
				Com_sprintf( filename, sizeof( filename ), "maps/%s/lm_%04d.hdr", s_worldData[rw].baseName, i * (tr.worldDeluxeMapping ? 2 : 1) );
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
				s_worldData[rw].lightmaps[i] = tr.lightmaps[i] = R_CreateImage(va("*lightmap_%d_%d", rw, i), image, tr.lightmapSize, tr.lightmapSize, IMGTYPE_COLORALPHA, imgFlags, textureInternalFormat );

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


static int FatLightmap(int lightmapnum)
{
	if (lightmapnum < 0)
		return lightmapnum;

	if (tr.worldDeluxeMapping)
		lightmapnum >>= 1;

	if (tr.fatLightmapCols > 0)
		return lightmapnum / (tr.fatLightmapCols * tr.fatLightmapRows);
	
	return lightmapnum;
}


/*
=================
R_LoadVisibility
=================
*/
static	void R_LoadVisibility( lump_t *l ) {
	int		len;
	byte	*buf;

	len = l->filelen;
	if ( !len ) {
		return;
	}
	buf = fileBase + l->fileofs;

	s_worldData[rw].numClusters = LittleLong( ((int *)buf)[0] );
	s_worldData[rw].clusterBytes = LittleLong( ((int *)buf)[1] );

	// CM_Load should have given us the vis data to share, so
	// we don't need to allocate another copy
	if ( tr.externalVisData ) {
		s_worldData[rw].vis = tr.externalVisData;
	} else {
		byte	*dest;

		dest = ri.Hunk_Alloc( len - 8, h_low );
		Com_Memcpy( dest, buf + 8, len - 8 );
		s_worldData[rw].vis = dest;
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
ParseFace
===============
*/
static void ParseFace( dBspFace_t *ds, vec3_t *verts, float *hdrVertColors, msurface_t *surf, int *indexes  ) {
	int			i;
	srfBspSurface_t	*cv;
	int			numVerts;

	// get shader value
	if ( r_singleShader->integer && !surf->shader->isSky ) {
		surf->shader = tr.defaultShader;
	}

	numVerts = LittleLong(ds->numedges);
	if (numVerts > MAX_FACE_POINTS) {
		ri.Printf( PRINT_WARNING, "WARNING: MAX_FACE_POINTS exceeded: %i\n", numVerts);
		numVerts = MAX_FACE_POINTS;
		surf->shader = tr.defaultShader;
	}

	//cv = ri.Hunk_Alloc(sizeof(*cv), h_low);
	cv = (void *)surf->data;
	cv->surfaceType = SF_FACE;

	cv->numVerts = numVerts;
	cv->verts = ri.Hunk_Alloc(numVerts * sizeof(cv->verts[0]), h_low);

	// copy vertexes
	surf->cullinfo.type = CULLINFO_PLANE | CULLINFO_BOX;
	ClearBounds(surf->cullinfo.bounds[0], surf->cullinfo.bounds[1]);
	verts += LittleLong(ds->firstedge);
	for(i = 0; i < numVerts; i++)
		LoadVertToSrfVert(&cv->verts[i], &verts[i], hdrVertColors ? hdrVertColors + (ds->firstedge + i) * 3 : NULL, surf->cullinfo.bounds);

	surf->data = (surfaceType_t *)cv;
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
	//surf->shader = ShaderForShaderNum( ds->texinfo, FatLightmap(realLightmapNum) );
	if ( r_singleShader->integer && !surf->shader->isSky ) {
		surf->shader = tr.defaultShader;
	}

	// we may have a nodraw surface, because they might still need to
	// be around for movement clipping
	if ( s_worldData[rw].shaders[ LittleLong( ds->texinfo ) ].surfaceFlags & SURF_NODRAW ) {
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
ParseTriSurf
===============
*/
static void ParseTriSurf( dBspFace_t *ds, vec3_t *verts, float *hdrVertColors, msurface_t *surf, int *indexes ) {
	srfBspSurface_t *cv;
	int             i;
	int             numVerts;

	// get shader
	surf->shader = ShaderForShaderNum( ds->texinfo, LIGHTMAP_BY_VERTEX );
	if ( r_singleShader->integer && !surf->shader->isSky ) {
		surf->shader = tr.defaultShader;
	}

	numVerts = LittleLong(ds->numedges);

	//cv = ri.Hunk_Alloc(sizeof(*cv), h_low);
	cv = (void *)surf->data;
	cv->surfaceType = SF_TRIANGLES;

	cv->numVerts = numVerts;
	cv->verts = ri.Hunk_Alloc(numVerts * sizeof(cv->verts[0]), h_low);

	surf->data = (surfaceType_t *) cv;

	// copy vertexes
	surf->cullinfo.type = CULLINFO_BOX;
	ClearBounds(surf->cullinfo.bounds[0], surf->cullinfo.bounds[1]);
	verts += LittleLong(ds->firstedge);
	for(i = 0; i < numVerts; i++)
		LoadVertToSrfVert(&cv->verts[i], &verts[i], hdrVertColors ? hdrVertColors + (ds->firstedge + i) * 3 : NULL, surf->cullinfo.bounds);

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
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData[rw].name);
	count = surfs->filelen / sizeof(*in);

	dv = (void *)(fileBase + verts->fileofs);
	if (verts->filelen % sizeof(*dv))
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData[rw].name);

/*
	indexes = (void *)(fileBase + indexLump->fileofs);
	if ( indexLump->filelen % sizeof(*indexes))
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData[rw].name);
*/

	out = ri.Hunk_Alloc ( count * sizeof(*out), h_low );	

	s_worldData[rw].surfaces = out;
	s_worldData[rw].numsurfaces = count;
	s_worldData[rw].surfacesViewCount = ri.Hunk_Alloc ( count * sizeof(*s_worldData[rw].surfacesViewCount), h_low );
	s_worldData[rw].surfacesDlightBits = ri.Hunk_Alloc ( count * sizeof(*s_worldData[rw].surfacesDlightBits), h_low );
	s_worldData[rw].surfacesPshadowBits = ri.Hunk_Alloc ( count * sizeof(*s_worldData[rw].surfacesPshadowBits), h_low );

	// load hdr vertex colors
	if (r_hdr->integer)
	{
		char filename[MAX_QPATH];
		int size;

		Com_sprintf( filename, sizeof( filename ), "maps/%s/vertlight.raw", s_worldData[rw].baseName);
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
	out = s_worldData[rw].surfaces;
	for ( i = 0 ; i < count ; i++, in++, out++ ) {
		out->data = ri.Hunk_Alloc( sizeof(srfBspSurface_t), h_low);
	}

	in = (void *)(fileBase + surfs->fileofs);
	out = s_worldData[rw].surfaces;
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
	dmodel_t	*in;
	bmodel_t	*out;
	int			i, j, count;

	in = (void *)(fileBase + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData[rw].name);
	count = l->filelen / sizeof(*in);

	s_worldData[rw].numBModels = count;
	s_worldData[rw].bmodels = out = ri.Hunk_Alloc( count * sizeof(*out), h_low );

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
			out->bounds[0][j] = LittleFloat (in->mins[j]);
			out->bounds[1][j] = LittleFloat (in->maxs[j]);
		}

		out->firstSurface = LittleLong( in->firstSurface );
		out->numSurfaces = LittleLong( in->numSurfaces );

		if(i == 0)
		{
			// Add this for limiting VAO surface creation
			s_worldData[rw].numWorldSurfaces = out->numSurfaces;
		}
	}
}



//==================================================================

/*
=================
R_LoadNodesAndLeafs
=================
*/
static	void R_LoadNodesAndLeafs (lump_t *nodeLump, lump_t *leafLump) {
	int			i, j, p;
	dnode_t		*in;
	dleaf_t		*inLeaf;
	mnode_t 	*out;
	int			numNodes, numLeafs;

	in = (void *)(fileBase + nodeLump->fileofs);
	if (nodeLump->filelen % sizeof(dnode_t) ||
		leafLump->filelen % sizeof(dleaf_t) ) {
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData[rw].name);
	}
	numNodes = nodeLump->filelen / sizeof(dnode_t);
	numLeafs = leafLump->filelen / sizeof(dleaf_t);

	out = ri.Hunk_Alloc ( (numNodes + numLeafs) * sizeof(*out), h_low);	

	s_worldData[rw].nodes = out;
	s_worldData[rw].numnodes = numNodes + numLeafs;
	s_worldData[rw].numDecisionNodes = numNodes;

	// load nodes
	for ( i=0 ; i<numNodes; i++, in++, out++)
	{
		for (j=0 ; j<3 ; j++)
		{
			out->mins[j] = LittleLong (in->mins[j]);
			out->maxs[j] = LittleLong (in->maxs[j]);
		}
	
		p = LittleLong(in->planeNum);
		out->plane = s_worldData[rw].planes + p;

		out->contents = CONTENTS_NODE;	// differentiate from leafs

		for (j=0 ; j<2 ; j++)
		{
			p = LittleLong (in->children[j]);
			if (p >= 0)
				out->children[j] = s_worldData[rw].nodes + p;
			else
				out->children[j] = s_worldData[rw].nodes + numNodes + (-1 - p);
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
		out->area = LittleLong(inLeaf->area);

		if ( out->cluster >= s_worldData[rw].numClusters ) {
			s_worldData[rw].numClusters = out->cluster + 1;
		}

		out->firstmarksurface = LittleLong(inLeaf->firstLeafSurface);
		out->nummarksurfaces = LittleLong(inLeaf->numLeafSurfaces);
	}	

	// chain descendants
	R_SetParent (s_worldData[rw].nodes, NULL);
}

//=============================================================================

/*
=================
R_LoadShaders
=================
*/
static	void R_LoadShaders2( lump_t *l ) {	
	int		i, count;
	dshader_t	*in, *out;
	
	in = (void *)(fileBase + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData[rw].name);
	count = l->filelen / sizeof(*in);
	out = ri.Hunk_Alloc ( count*sizeof(*out), h_low );

	s_worldData[rw].shaders = out;
	s_worldData[rw].numShaders = count;

	Com_Memcpy( out, in, count*sizeof(*out) );

	for ( i=0 ; i<count ; i++ ) {
		out[i].surfaceFlags = LittleLong( out[i].surfaceFlags );
		out[i].contentFlags = LittleLong( out[i].contentFlags );
	}
}


/*
=================
R_LoadPlanes
=================
*/
static	void R_LoadPlanes( lump_t *l ) {
	int			i, j;
	cplane_t	*out;
	dplane_t 	*in;
	int			count;
	int			bits;
	
	in = (void *)(fileBase + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData[rw].name);
	count = l->filelen / sizeof(*in);
	out = ri.Hunk_Alloc ( count*2*sizeof(*out), h_low);	
	
	s_worldData[rw].planes = out;
	s_worldData[rw].numplanes = count;

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
	int			i;
	fog_t		*out;
	dfog_t		*fogs;
	dbrush_t 	*brushes, *brush;
	dbrushside_t	*sides;
	int			count, brushesCount, sidesCount;
	int			sideNum;
	int			planeNum;
	shader_t	*shader;
	float		d;
	int			firstSide;

	fogs = NULL;
	return;

	// create fog strucutres for them
	s_worldData[rw].numfogs = count + 1;
	s_worldData[rw].fogs = ri.Hunk_Alloc ( s_worldData[rw].numfogs*sizeof(*out), h_low);
	out = s_worldData[rw].fogs + 1;

	if ( !count ) {
		return;
	}

	brushes = (void *)(fileBase + brushesLump->fileofs);
	if (brushesLump->filelen % sizeof(*brushes)) {
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData[rw].name);
	}
	brushesCount = brushesLump->filelen / sizeof(*brushes);

	sides = (void *)(fileBase + sidesLump->fileofs);
	if (sidesLump->filelen % sizeof(*sides)) {
		ri.Error (ERR_DROP, "LoadMap: funny lump size in %s",s_worldData[rw].name);
	}
	sidesCount = sidesLump->filelen / sizeof(*sides);

	for ( i=0 ; i<count ; i++, fogs++) {
		out->originalBrushNumber = LittleLong( fogs->brushNum );

		if ( (unsigned)out->originalBrushNumber >= brushesCount ) {
			ri.Printf( PRINT_WARNING, "fog brushNumber out of range" );
			continue;
		}
		brush = brushes + out->originalBrushNumber;

		firstSide = LittleLong( brush->firstSide );

			if ( (unsigned)firstSide > sidesCount - 6 ) {
			ri.Error( ERR_DROP, "fog brush sideNumber out of range" );
		}

		// brushes are always sorted with the axial sides first
		sideNum = firstSide + 0;
		planeNum = LittleLong( sides[ sideNum ].planeNum );
		out->bounds[0][0] = -s_worldData[rw].planes[ planeNum ].dist;

		sideNum = firstSide + 1;
		planeNum = LittleLong( sides[ sideNum ].planeNum );
		out->bounds[1][0] = s_worldData[rw].planes[ planeNum ].dist;

		sideNum = firstSide + 2;
		planeNum = LittleLong( sides[ sideNum ].planeNum );
		out->bounds[0][1] = -s_worldData[rw].planes[ planeNum ].dist;

		sideNum = firstSide + 3;
		planeNum = LittleLong( sides[ sideNum ].planeNum );
		out->bounds[1][1] = s_worldData[rw].planes[ planeNum ].dist;

		sideNum = firstSide + 4;
		planeNum = LittleLong( sides[ sideNum ].planeNum );
		out->bounds[0][2] = -s_worldData[rw].planes[ planeNum ].dist;

		sideNum = firstSide + 5;
		planeNum = LittleLong( sides[ sideNum ].planeNum );
		out->bounds[1][2] = s_worldData[rw].planes[ planeNum ].dist;

		// get information from the shader for fog parameters
		shader = R_FindShader( fogs->shader, LIGHTMAP_NONE, qtrue );

		out->parms = shader->fogParms;

		out->colorInt = ColorBytes4 ( shader->fogParms.color[0],
			                          shader->fogParms.color[1],
			                          shader->fogParms.color[2], 1.0 );

		d = shader->fogParms.depthForOpaque < 1 ? 1 : shader->fogParms.depthForOpaque;
		out->tcScale = 1.0f / ( d * 8 );

		// set the gradient vector
		sideNum = LittleLong( fogs->visibleSide );

		if ( sideNum == -1 ) {
			out->hasSurface = qfalse;
		} else {
			out->hasSurface = qtrue;
			planeNum = LittleLong( sides[ firstSide + sideNum ].planeNum );
			VectorSubtract( vec3_origin, s_worldData[rw].planes[ planeNum ].normal, out->surface );
			out->surface[3] = -s_worldData[rw].planes[ planeNum ].dist;
		}

		out++;
	}

}



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
	R_LoadLightmaps( &header->lumps[LUMP_Q2_LIGHTING], &header->lumps[LUMP_Q2_FACES] );
	R_LoadPlanes (&header->lumps[LUMP_Q2_PLANES]);
	R_LoadFogs2( &header->lumps[LUMP_Q2_BRUSHES], &header->lumps[LUMP_Q2_BRUSHSIDES] );
	R_LoadSurfaces2( &header->lumps[LUMP_Q2_FACES], &header->lumps[LUMP_Q2_VERTEXES] );
	//R_LoadMarksurfaces (&header->lumps[LUMP_Q2_LEAFSURFACES]);
	R_LoadNodesAndLeafs (&header->lumps[LUMP_Q2_NODES], &header->lumps[LUMP_Q2_LEAFS]);
	R_LoadSubmodels2 (&header->lumps[LUMP_Q2_MODELS]);
	R_LoadVisibility( &header->lumps[LUMP_Q2_VISIBILITY] );
	//R_LoadLightGrid( &header->lumps[LUMP_Q2_LIGHTGRID] );

}
