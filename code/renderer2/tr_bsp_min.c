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
extern void R_LoadShaders( lump_t *l );
extern void R_LoadPlanes( lump_t *l );
extern void R_LoadSurfaces( lump_t *surfs, lump_t *verts, lump_t *indexLump );
extern void R_LoadNodesAndLeafs (lump_t *nodeLump, lump_t *leafLump);
extern void R_LoadSubmodels( lump_t *l );
extern void R_LoadMarksurfaces (lump_t *l);

// load as few things as possible
void LoadBspMin(const char *name) {
	int i;
	dheader_t	*header;
	header = (dheader_t *)fileBase;

	// swap all the lumps
	for (i=0 ; i<sizeof(dheader_t)/4 ; i++) {
		((int *)header)[i] = LittleLong ( ((int *)header)[i]);
	}

	// load into heap
	R_LoadEntities( &header->lumps[LUMP_ENTITIES] );
	R_LoadShaders( &header->lumps[LUMP_SHADERS] );
	R_LoadPlanes (&header->lumps[LUMP_PLANES]);
	R_LoadSurfaces( &header->lumps[LUMP_SURFACES], &header->lumps[LUMP_DRAWVERTS], &header->lumps[LUMP_DRAWINDEXES] );
  R_LoadMarksurfaces (&header->lumps[LUMP_LEAFSURFACES]);
  R_LoadNodesAndLeafs (&header->lumps[LUMP_NODES], &header->lumps[LUMP_LEAFS]);
	R_LoadSubmodels (&header->lumps[LUMP_MODELS]);
}
