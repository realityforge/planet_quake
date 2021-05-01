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

#ifndef __CM_PUBLIC_H__
#define __CM_PUBLIC_H__

#include "qfiles.h"


int		CM_LoadMap( const char *name, qboolean clientload, int *checksum);
int		CM_SwitchMap( int world );
void		CM_ClearMap( void );
clipHandle_t CM_InlineModel( int index, int client, int world );		// 0 = world, 1 + are bmodels
clipHandle_t CM_TempBoxModel( const vec3_t mins, const vec3_t maxs, int capsule );

void		CM_ModelBounds( clipHandle_t model, vec3_t mins, vec3_t maxs );

int			CM_NumClusters (void);
int			CM_NumInlineModels( void );
char		*CM_EntityString (void);

void CMod_LoadShaders( lump_t *l );
void CMod_LoadLeafs( lump_t *l );
void CMod_LoadLeafBrushes( const lump_t *l );
void CMod_LoadLeafSurfaces( const lump_t *l );
void CMod_LoadPlanes( const lump_t *l );
void CMod_LoadBrushSides( lump_t *l );
void CMod_LoadBrushes( lump_t *l );
void CMod_LoadSubmodels( lump_t *l );
void CMod_LoadNodes( lump_t *l );
void CMod_LoadEntityString( lump_t *l, const char *name );
void CMod_LoadVisibility( lump_t *l );
void CMod_LoadPatches( lump_t *surfs, lump_t *verts );
extern dmodel_t *dModels;
extern dshader_t *dShaders;
extern char *dEntData;
extern dleaf_t *dLeafs;
extern dplane_t *dPlanes;
extern dnode_t *dNodes;
extern int *dLeafSurfaces;
extern int *dLeafBrushes;
extern dbrush_t *dBrushes;
extern dbrushside_t *dBrushSides;
extern byte *dLightBytes;
extern byte *dGridPoints;
extern byte *dVisBytes;
extern drawVert_t *dDrawVerts;
extern int *dDrawIndexes;
extern dsurface_t   *dDrawSurfaces;
extern dfog_t *dFogs;
//bspAdvertisement_t *dAds;

int CM_LoadMapFromMemory( void );

// returns an ORed contents mask
int			CM_PointContents( const vec3_t p, clipHandle_t model );
int			CM_TransformedPointContents( const vec3_t p, clipHandle_t model, const vec3_t origin, const vec3_t angles );

void		CM_BoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
						const vec3_t mins, const vec3_t maxs,
						clipHandle_t model, int brushmask, qboolean capsule );
void		CM_TransformedBoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
						const vec3_t mins, const vec3_t maxs,
						clipHandle_t model, int brushmask,
						const vec3_t origin, const vec3_t angles, qboolean capsule );

byte		*CM_ClusterPVS (int cluster);

int			CM_PointLeafnum( const vec3_t p );

// only returns non-solid leafs
// overflow if return listsize and if *lastLeaf != list[listsize-1]
int			CM_BoxLeafnums( const vec3_t mins, const vec3_t maxs, int *list,
		 					int listsize, int *lastLeaf );

int			CM_LeafCluster (int leafnum);
int			CM_LeafArea (int leafnum);

void		CM_AdjustAreaPortalState( int area1, int area2, qboolean open );
qboolean	CM_AreasConnected( int area1, int area2 );

int			CM_WriteAreaBits( byte *buffer, int area );

// cm_patch.c
void CM_DrawDebugSurface( void (*drawPoly)(int color, int numPoints, float *points) );

#endif
