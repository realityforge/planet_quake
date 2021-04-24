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

/*****************************************************************************
 * name:		be_aas_file.c
 *
 * desc:		AAS file loading/writing
 *
 * $Archive: /MissionPack/code/botlib/be_aas_file.c $
 *
 *****************************************************************************/

#include "../qcommon/q_shared.h"
#include "l_memory.h"
#include "l_script.h"
#include "l_precomp.h"
#include "l_struct.h"
#include "l_libvar.h"
#include "l_utils.h"
#include "aasfile.h"
#include "botlib.h"
#include "be_aas.h"
#include "be_aas_funcs.h"
#include "be_interface.h"
#include "be_aas_def.h"

//#define AASFILEDEBUG

//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_SwapAASData(void)
{
	int i, j;
	//bounding boxes
	for (i = 0; i < aasworld[aasgvm].numbboxes; i++)
	{
		aasworld[aasgvm].bboxes[i].presencetype = LittleLong(aasworld[aasgvm].bboxes[i].presencetype);
		aasworld[aasgvm].bboxes[i].flags = LittleLong(aasworld[aasgvm].bboxes[i].flags);
		for (j = 0; j < 3; j++)
		{
			aasworld[aasgvm].bboxes[i].mins[j] = LittleFloat(aasworld[aasgvm].bboxes[i].mins[j]);
			aasworld[aasgvm].bboxes[i].maxs[j] = LittleFloat(aasworld[aasgvm].bboxes[i].maxs[j]);
		} //end for
	} //end for
	//vertexes
	for (i = 0; i < aasworld[aasgvm].numvertexes; i++)
	{
		for (j = 0; j < 3; j++)
			aasworld[aasgvm].vertexes[i][j] = LittleFloat(aasworld[aasgvm].vertexes[i][j]);
	} //end for
	//planes
	for (i = 0; i < aasworld[aasgvm].numplanes; i++)
	{
		for (j = 0; j < 3; j++)
			aasworld[aasgvm].planes[i].normal[j] = LittleFloat(aasworld[aasgvm].planes[i].normal[j]);
		aasworld[aasgvm].planes[i].dist = LittleFloat(aasworld[aasgvm].planes[i].dist);
		aasworld[aasgvm].planes[i].type = LittleLong(aasworld[aasgvm].planes[i].type);
	} //end for
	//edges
	for (i = 0; i < aasworld[aasgvm].numedges; i++)
	{
		aasworld[aasgvm].edges[i].v[0] = LittleLong(aasworld[aasgvm].edges[i].v[0]);
		aasworld[aasgvm].edges[i].v[1] = LittleLong(aasworld[aasgvm].edges[i].v[1]);
	} //end for
	//edgeindex
	for (i = 0; i < aasworld[aasgvm].edgeindexsize; i++)
	{
		aasworld[aasgvm].edgeindex[i] = LittleLong(aasworld[aasgvm].edgeindex[i]);
	} //end for
	//faces
	for (i = 0; i < aasworld[aasgvm].numfaces; i++)
	{
		aasworld[aasgvm].faces[i].planenum = LittleLong(aasworld[aasgvm].faces[i].planenum);
		aasworld[aasgvm].faces[i].faceflags = LittleLong(aasworld[aasgvm].faces[i].faceflags);
		aasworld[aasgvm].faces[i].numedges = LittleLong(aasworld[aasgvm].faces[i].numedges);
		aasworld[aasgvm].faces[i].firstedge = LittleLong(aasworld[aasgvm].faces[i].firstedge);
		aasworld[aasgvm].faces[i].frontarea = LittleLong(aasworld[aasgvm].faces[i].frontarea);
		aasworld[aasgvm].faces[i].backarea = LittleLong(aasworld[aasgvm].faces[i].backarea);
	} //end for
	//face index
	for (i = 0; i < aasworld[aasgvm].faceindexsize; i++)
	{
		aasworld[aasgvm].faceindex[i] = LittleLong(aasworld[aasgvm].faceindex[i]);
	} //end for
	//convex areas
	for (i = 0; i < aasworld[aasgvm].numareas; i++)
	{
		aasworld[aasgvm].areas[i].areanum = LittleLong(aasworld[aasgvm].areas[i].areanum);
		aasworld[aasgvm].areas[i].numfaces = LittleLong(aasworld[aasgvm].areas[i].numfaces);
		aasworld[aasgvm].areas[i].firstface = LittleLong(aasworld[aasgvm].areas[i].firstface);
		for (j = 0; j < 3; j++)
		{
			aasworld[aasgvm].areas[i].mins[j] = LittleFloat(aasworld[aasgvm].areas[i].mins[j]);
			aasworld[aasgvm].areas[i].maxs[j] = LittleFloat(aasworld[aasgvm].areas[i].maxs[j]);
			aasworld[aasgvm].areas[i].center[j] = LittleFloat(aasworld[aasgvm].areas[i].center[j]);
		} //end for
	} //end for
	//area settings
	for (i = 0; i < aasworld[aasgvm].numareasettings; i++)
	{
		aasworld[aasgvm].areasettings[i].contents = LittleLong(aasworld[aasgvm].areasettings[i].contents);
		aasworld[aasgvm].areasettings[i].areaflags = LittleLong(aasworld[aasgvm].areasettings[i].areaflags);
		aasworld[aasgvm].areasettings[i].presencetype = LittleLong(aasworld[aasgvm].areasettings[i].presencetype);
		aasworld[aasgvm].areasettings[i].cluster = LittleLong(aasworld[aasgvm].areasettings[i].cluster);
		aasworld[aasgvm].areasettings[i].clusterareanum = LittleLong(aasworld[aasgvm].areasettings[i].clusterareanum);
		aasworld[aasgvm].areasettings[i].numreachableareas = LittleLong(aasworld[aasgvm].areasettings[i].numreachableareas);
		aasworld[aasgvm].areasettings[i].firstreachablearea = LittleLong(aasworld[aasgvm].areasettings[i].firstreachablearea);
	} //end for
	//area reachability
	for (i = 0; i < aasworld[aasgvm].reachabilitysize; i++)
	{
		aasworld[aasgvm].reachability[i].areanum = LittleLong(aasworld[aasgvm].reachability[i].areanum);
		aasworld[aasgvm].reachability[i].facenum = LittleLong(aasworld[aasgvm].reachability[i].facenum);
		aasworld[aasgvm].reachability[i].edgenum = LittleLong(aasworld[aasgvm].reachability[i].edgenum);
		for (j = 0; j < 3; j++)
		{
			aasworld[aasgvm].reachability[i].start[j] = LittleFloat(aasworld[aasgvm].reachability[i].start[j]);
			aasworld[aasgvm].reachability[i].end[j] = LittleFloat(aasworld[aasgvm].reachability[i].end[j]);
		} //end for
		aasworld[aasgvm].reachability[i].traveltype = LittleLong(aasworld[aasgvm].reachability[i].traveltype);
		aasworld[aasgvm].reachability[i].traveltime = LittleShort(aasworld[aasgvm].reachability[i].traveltime);
	} //end for
	//nodes
	for (i = 0; i < aasworld[aasgvm].numnodes; i++)
	{
		aasworld[aasgvm].nodes[i].planenum = LittleLong(aasworld[aasgvm].nodes[i].planenum);
		aasworld[aasgvm].nodes[i].children[0] = LittleLong(aasworld[aasgvm].nodes[i].children[0]);
		aasworld[aasgvm].nodes[i].children[1] = LittleLong(aasworld[aasgvm].nodes[i].children[1]);
	} //end for
	//cluster portals
	for (i = 0; i < aasworld[aasgvm].numportals; i++)
	{
		aasworld[aasgvm].portals[i].areanum = LittleLong(aasworld[aasgvm].portals[i].areanum);
		aasworld[aasgvm].portals[i].frontcluster = LittleLong(aasworld[aasgvm].portals[i].frontcluster);
		aasworld[aasgvm].portals[i].backcluster = LittleLong(aasworld[aasgvm].portals[i].backcluster);
		aasworld[aasgvm].portals[i].clusterareanum[0] = LittleLong(aasworld[aasgvm].portals[i].clusterareanum[0]);
		aasworld[aasgvm].portals[i].clusterareanum[1] = LittleLong(aasworld[aasgvm].portals[i].clusterareanum[1]);
	} //end for
	//cluster portal index
	for (i = 0; i < aasworld[aasgvm].portalindexsize; i++)
	{
		aasworld[aasgvm].portalindex[i] = LittleLong(aasworld[aasgvm].portalindex[i]);
	} //end for
	//cluster
	for (i = 0; i < aasworld[aasgvm].numclusters; i++)
	{
		aasworld[aasgvm].clusters[i].numareas = LittleLong(aasworld[aasgvm].clusters[i].numareas);
		aasworld[aasgvm].clusters[i].numreachabilityareas = LittleLong(aasworld[aasgvm].clusters[i].numreachabilityareas);
		aasworld[aasgvm].clusters[i].numportals = LittleLong(aasworld[aasgvm].clusters[i].numportals);
		aasworld[aasgvm].clusters[i].firstportal = LittleLong(aasworld[aasgvm].clusters[i].firstportal);
	} //end for
} //end of the function AAS_SwapAASData
//===========================================================================
// dump the current loaded aas file
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_DumpAASData(void)
{
	aasworld[aasgvm].numbboxes = 0;
	if (aasworld[aasgvm].bboxes) FreeMemory(aasworld[aasgvm].bboxes);
	aasworld[aasgvm].bboxes = NULL;
	aasworld[aasgvm].numvertexes = 0;
	if (aasworld[aasgvm].vertexes) FreeMemory(aasworld[aasgvm].vertexes);
	aasworld[aasgvm].vertexes = NULL;
	aasworld[aasgvm].numplanes = 0;
	if (aasworld[aasgvm].planes) FreeMemory(aasworld[aasgvm].planes);
	aasworld[aasgvm].planes = NULL;
	aasworld[aasgvm].numedges = 0;
	if (aasworld[aasgvm].edges) FreeMemory(aasworld[aasgvm].edges);
	aasworld[aasgvm].edges = NULL;
	aasworld[aasgvm].edgeindexsize = 0;
	if (aasworld[aasgvm].edgeindex) FreeMemory(aasworld[aasgvm].edgeindex);
	aasworld[aasgvm].edgeindex = NULL;
	aasworld[aasgvm].numfaces = 0;
	if (aasworld[aasgvm].faces) FreeMemory(aasworld[aasgvm].faces);
	aasworld[aasgvm].faces = NULL;
	aasworld[aasgvm].faceindexsize = 0;
	if (aasworld[aasgvm].faceindex) FreeMemory(aasworld[aasgvm].faceindex);
	aasworld[aasgvm].faceindex = NULL;
	aasworld[aasgvm].numareas = 0;
	if (aasworld[aasgvm].areas) FreeMemory(aasworld[aasgvm].areas);
	aasworld[aasgvm].areas = NULL;
	aasworld[aasgvm].numareasettings = 0;
	if (aasworld[aasgvm].areasettings) FreeMemory(aasworld[aasgvm].areasettings);
	aasworld[aasgvm].areasettings = NULL;
	aasworld[aasgvm].reachabilitysize = 0;
	if (aasworld[aasgvm].reachability) FreeMemory(aasworld[aasgvm].reachability);
	aasworld[aasgvm].reachability = NULL;
	aasworld[aasgvm].numnodes = 0;
	if (aasworld[aasgvm].nodes) FreeMemory(aasworld[aasgvm].nodes);
	aasworld[aasgvm].nodes = NULL;
	aasworld[aasgvm].numportals = 0;
	if (aasworld[aasgvm].portals) FreeMemory(aasworld[aasgvm].portals);
	aasworld[aasgvm].portals = NULL;
	aasworld[aasgvm].numportals = 0;
	if (aasworld[aasgvm].portalindex) FreeMemory(aasworld[aasgvm].portalindex);
	aasworld[aasgvm].portalindex = NULL;
	aasworld[aasgvm].portalindexsize = 0;
	if (aasworld[aasgvm].clusters) FreeMemory(aasworld[aasgvm].clusters);
	aasworld[aasgvm].clusters = NULL;
	aasworld[aasgvm].numclusters = 0;
	//
	aasworld[aasgvm].loaded = qfalse;
	aasworld[aasgvm].initialized = qfalse;
	aasworld[aasgvm].savefile = qfalse;
} //end of the function AAS_DumpAASData
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
#ifdef AASFILEDEBUG
void AAS_FileInfo(void)
{
	int i, n, optimized;

	botimport.Print(PRT_MESSAGE, "version = %d\n", AASVERSION);
	botimport.Print(PRT_MESSAGE, "numvertexes = %d\n", aasworld[aasgvm].numvertexes);
	botimport.Print(PRT_MESSAGE, "numplanes = %d\n", aasworld[aasgvm].numplanes);
	botimport.Print(PRT_MESSAGE, "numedges = %d\n", aasworld[aasgvm].numedges);
	botimport.Print(PRT_MESSAGE, "edgeindexsize = %d\n", aasworld[aasgvm].edgeindexsize);
	botimport.Print(PRT_MESSAGE, "numfaces = %d\n", aasworld[aasgvm].numfaces);
	botimport.Print(PRT_MESSAGE, "faceindexsize = %d\n", aasworld[aasgvm].faceindexsize);
	botimport.Print(PRT_MESSAGE, "numareas = %d\n", aasworld[aasgvm].numareas);
	botimport.Print(PRT_MESSAGE, "numareasettings = %d\n", aasworld[aasgvm].numareasettings);
	botimport.Print(PRT_MESSAGE, "reachabilitysize = %d\n", aasworld[aasgvm].reachabilitysize);
	botimport.Print(PRT_MESSAGE, "numnodes = %d\n", aasworld[aasgvm].numnodes);
	botimport.Print(PRT_MESSAGE, "numportals = %d\n", aasworld[aasgvm].numportals);
	botimport.Print(PRT_MESSAGE, "portalindexsize = %d\n", aasworld[aasgvm].portalindexsize);
	botimport.Print(PRT_MESSAGE, "numclusters = %d\n", aasworld[aasgvm].numclusters);
	//
	for (n = 0, i = 0; i < aasworld[aasgvm].numareasettings; i++)
	{
		if (aasworld[aasgvm].areasettings[i].areaflags & AREA_GROUNDED) n++;
	} //end for
	botimport.Print(PRT_MESSAGE, "num grounded areas = %d\n", n);
	//
	botimport.Print(PRT_MESSAGE, "planes size %d bytes\n", aasworld[aasgvm].numplanes * sizeof(aas_plane_t));
	botimport.Print(PRT_MESSAGE, "areas size %d bytes\n", aasworld[aasgvm].numareas * sizeof(aas_area_t));
	botimport.Print(PRT_MESSAGE, "areasettings size %d bytes\n", aasworld[aasgvm].numareasettings * sizeof(aas_areasettings_t));
	botimport.Print(PRT_MESSAGE, "nodes size %d bytes\n", aasworld[aasgvm].numnodes * sizeof(aas_node_t));
	botimport.Print(PRT_MESSAGE, "reachability size %d bytes\n", aasworld[aasgvm].reachabilitysize * sizeof(aas_reachability_t));
	botimport.Print(PRT_MESSAGE, "portals size %d bytes\n", aasworld[aasgvm].numportals * sizeof(aas_portal_t));
	botimport.Print(PRT_MESSAGE, "clusters size %d bytes\n", aasworld[aasgvm].numclusters * sizeof(aas_cluster_t));

	optimized = aasworld[aasgvm].numplanes * sizeof(aas_plane_t) +
					aasworld[aasgvm].numareas * sizeof(aas_area_t) +
					aasworld[aasgvm].numareasettings * sizeof(aas_areasettings_t) +
					aasworld[aasgvm].numnodes * sizeof(aas_node_t) +
					aasworld[aasgvm].reachabilitysize * sizeof(aas_reachability_t) +
					aasworld[aasgvm].numportals * sizeof(aas_portal_t) +
					aasworld[aasgvm].numclusters * sizeof(aas_cluster_t);
	botimport.Print(PRT_MESSAGE, "optimzed size %d KB\n", optimized >> 10);
} //end of the function AAS_FileInfo
#endif //AASFILEDEBUG
//===========================================================================
// allocate memory and read a lump of an AAS file
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
char *AAS_LoadAASLump(fileHandle_t fp, int offset, int length, int *lastoffset, int size)
{
	char *buf;
	//
	if (!length)
	{
		//just alloc a dummy
		return (char *) GetClearedHunkMemory(size+1);
	} //end if
	//seek to the data
	if (offset != *lastoffset)
	{
		botimport.Print(PRT_WARNING, "AAS file not sequentially read\n");
		if (botimport.FS_Seek(fp, offset, FS_SEEK_SET))
		{
			AAS_Error("can't seek to aas lump\n");
			AAS_DumpAASData();
			botimport.FS_FCloseFile(fp);
			return NULL;
		} //end if
	} //end if
	//allocate memory
	buf = (char *) GetClearedHunkMemory(length+1);
	//read the data
	//if (length)
	{
		botimport.FS_Read(buf, length, fp );
		*lastoffset += length;
	} //end if
	return buf;
} //end of the function AAS_LoadAASLump
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void AAS_DData(unsigned char *data, int size)
{
	int i;

	for (i = 0; i < size; i++)
	{
		data[i] ^= (unsigned char) i * 119;
	} //end for
} //end of the function AAS_DData
//===========================================================================
// load an aas file
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int AAS_LoadAASFile(char *filename)
{
	fileHandle_t fp;
	aas_header_t header;
	int offset, length, lastoffset;

	botimport.Print(PRT_MESSAGE, "trying to load %s (%i)\n", filename, aasgvm);
	//dump current loaded aas file
	AAS_DumpAASData();
	//open the file
	botimport.FS_FOpenFile( filename, &fp, FS_READ );
	if (!fp)
	{
		AAS_Error("can't open %s\n", filename);
		return BLERR_CANNOTOPENAASFILE;
	} //end if
	//read the header
	botimport.FS_Read(&header, sizeof(aas_header_t), fp );
	lastoffset = sizeof(aas_header_t);
	//check header identification
	header.ident = LittleLong(header.ident);
	if (header.ident != AASID)
	{
		AAS_Error("%s is not an AAS file\n", filename);
		botimport.FS_FCloseFile(fp);
		return BLERR_WRONGAASFILEID;
	} //end if
	//check the version
	header.version = LittleLong(header.version);
	//
	if (header.version != AASVERSION_OLD && header.version != AASVERSION)
	{
		AAS_Error("aas file %s is version %i, not %i\n", filename, header.version, AASVERSION);
		botimport.FS_FCloseFile(fp);
		return BLERR_WRONGAASFILEVERSION;
	} //end if
	//
	if (header.version == AASVERSION)
	{
		AAS_DData((unsigned char *) &header + 8, sizeof(aas_header_t) - 8);
	} //end if
	//
	aasworld[aasgvm].bspchecksum = atoi(LibVarGetString( "sv_mapChecksum"));
	if (LittleLong(header.bspchecksum) != aasworld[aasgvm].bspchecksum)
	{
		AAS_Error("aas file %s is out of date\n", filename);
		botimport.FS_FCloseFile(fp);
		return BLERR_WRONGAASFILEVERSION;
	} //end if
	//load the lumps:
	//bounding boxes
	offset = LittleLong(header.lumps[AASLUMP_BBOXES].fileofs);
	length = LittleLong(header.lumps[AASLUMP_BBOXES].filelen);
	aasworld[aasgvm].bboxes = (aas_bbox_t *) AAS_LoadAASLump(fp, offset, length, &lastoffset, sizeof(aas_bbox_t));
	aasworld[aasgvm].numbboxes = length / sizeof(aas_bbox_t);
	if (aasworld[aasgvm].numbboxes && !aasworld[aasgvm].bboxes) return BLERR_CANNOTREADAASLUMP;
	//vertexes
	offset = LittleLong(header.lumps[AASLUMP_VERTEXES].fileofs);
	length = LittleLong(header.lumps[AASLUMP_VERTEXES].filelen);
	aasworld[aasgvm].vertexes = (aas_vertex_t *) AAS_LoadAASLump(fp, offset, length, &lastoffset, sizeof(aas_vertex_t));
	aasworld[aasgvm].numvertexes = length / sizeof(aas_vertex_t);
	if (aasworld[aasgvm].numvertexes && !aasworld[aasgvm].vertexes) return BLERR_CANNOTREADAASLUMP;
	//planes
	offset = LittleLong(header.lumps[AASLUMP_PLANES].fileofs);
	length = LittleLong(header.lumps[AASLUMP_PLANES].filelen);
	aasworld[aasgvm].planes = (aas_plane_t *) AAS_LoadAASLump(fp, offset, length, &lastoffset, sizeof(aas_plane_t));
	aasworld[aasgvm].numplanes = length / sizeof(aas_plane_t);
	if (aasworld[aasgvm].numplanes && !aasworld[aasgvm].planes) return BLERR_CANNOTREADAASLUMP;
	//edges
	offset = LittleLong(header.lumps[AASLUMP_EDGES].fileofs);
	length = LittleLong(header.lumps[AASLUMP_EDGES].filelen);
	aasworld[aasgvm].edges = (aas_edge_t *) AAS_LoadAASLump(fp, offset, length, &lastoffset, sizeof(aas_edge_t));
	aasworld[aasgvm].numedges = length / sizeof(aas_edge_t);
	if (aasworld[aasgvm].numedges && !aasworld[aasgvm].edges) return BLERR_CANNOTREADAASLUMP;
	//edgeindex
	offset = LittleLong(header.lumps[AASLUMP_EDGEINDEX].fileofs);
	length = LittleLong(header.lumps[AASLUMP_EDGEINDEX].filelen);
	aasworld[aasgvm].edgeindex = (aas_edgeindex_t *) AAS_LoadAASLump(fp, offset, length, &lastoffset, sizeof(aas_edgeindex_t));
	aasworld[aasgvm].edgeindexsize = length / sizeof(aas_edgeindex_t);
	if (aasworld[aasgvm].edgeindexsize && !aasworld[aasgvm].edgeindex) return BLERR_CANNOTREADAASLUMP;
	//faces
	offset = LittleLong(header.lumps[AASLUMP_FACES].fileofs);
	length = LittleLong(header.lumps[AASLUMP_FACES].filelen);
	aasworld[aasgvm].faces = (aas_face_t *) AAS_LoadAASLump(fp, offset, length, &lastoffset, sizeof(aas_face_t));
	aasworld[aasgvm].numfaces = length / sizeof(aas_face_t);
	if (aasworld[aasgvm].numfaces && !aasworld[aasgvm].faces) return BLERR_CANNOTREADAASLUMP;
	//faceindex
	offset = LittleLong(header.lumps[AASLUMP_FACEINDEX].fileofs);
	length = LittleLong(header.lumps[AASLUMP_FACEINDEX].filelen);
	aasworld[aasgvm].faceindex = (aas_faceindex_t *) AAS_LoadAASLump(fp, offset, length, &lastoffset, sizeof(aas_faceindex_t));
	aasworld[aasgvm].faceindexsize = length / sizeof(aas_faceindex_t);
	if (aasworld[aasgvm].faceindexsize && !aasworld[aasgvm].faceindex) return BLERR_CANNOTREADAASLUMP;
	//convex areas
	offset = LittleLong(header.lumps[AASLUMP_AREAS].fileofs);
	length = LittleLong(header.lumps[AASLUMP_AREAS].filelen);
	aasworld[aasgvm].areas = (aas_area_t *) AAS_LoadAASLump(fp, offset, length, &lastoffset, sizeof(aas_area_t));
	aasworld[aasgvm].numareas = length / sizeof(aas_area_t);
	if (aasworld[aasgvm].numareas && !aasworld[aasgvm].areas) return BLERR_CANNOTREADAASLUMP;
	//area settings
	offset = LittleLong(header.lumps[AASLUMP_AREASETTINGS].fileofs);
	length = LittleLong(header.lumps[AASLUMP_AREASETTINGS].filelen);
	aasworld[aasgvm].areasettings = (aas_areasettings_t *) AAS_LoadAASLump(fp, offset, length, &lastoffset, sizeof(aas_areasettings_t));
	aasworld[aasgvm].numareasettings = length / sizeof(aas_areasettings_t);
	if (aasworld[aasgvm].numareasettings && !aasworld[aasgvm].areasettings) return BLERR_CANNOTREADAASLUMP;
	//reachability list
	offset = LittleLong(header.lumps[AASLUMP_REACHABILITY].fileofs);
	length = LittleLong(header.lumps[AASLUMP_REACHABILITY].filelen);
	aasworld[aasgvm].reachability = (aas_reachability_t *) AAS_LoadAASLump(fp, offset, length, &lastoffset, sizeof(aas_reachability_t));
	aasworld[aasgvm].reachabilitysize = length / sizeof(aas_reachability_t);
	if (aasworld[aasgvm].reachabilitysize && !aasworld[aasgvm].reachability) return BLERR_CANNOTREADAASLUMP;
	//nodes
	offset = LittleLong(header.lumps[AASLUMP_NODES].fileofs);
	length = LittleLong(header.lumps[AASLUMP_NODES].filelen);
	aasworld[aasgvm].nodes = (aas_node_t *) AAS_LoadAASLump(fp, offset, length, &lastoffset, sizeof(aas_node_t));
	aasworld[aasgvm].numnodes = length / sizeof(aas_node_t);
	if (aasworld[aasgvm].numnodes && !aasworld[aasgvm].nodes) return BLERR_CANNOTREADAASLUMP;
	//cluster portals
	offset = LittleLong(header.lumps[AASLUMP_PORTALS].fileofs);
	length = LittleLong(header.lumps[AASLUMP_PORTALS].filelen);
	aasworld[aasgvm].portals = (aas_portal_t *) AAS_LoadAASLump(fp, offset, length, &lastoffset, sizeof(aas_portal_t));
	aasworld[aasgvm].numportals = length / sizeof(aas_portal_t);
	if (aasworld[aasgvm].numportals && !aasworld[aasgvm].portals) return BLERR_CANNOTREADAASLUMP;
	//cluster portal index
	offset = LittleLong(header.lumps[AASLUMP_PORTALINDEX].fileofs);
	length = LittleLong(header.lumps[AASLUMP_PORTALINDEX].filelen);
	aasworld[aasgvm].portalindex = (aas_portalindex_t *) AAS_LoadAASLump(fp, offset, length, &lastoffset, sizeof(aas_portalindex_t));
	aasworld[aasgvm].portalindexsize = length / sizeof(aas_portalindex_t);
	if (aasworld[aasgvm].portalindexsize && !aasworld[aasgvm].portalindex) return BLERR_CANNOTREADAASLUMP;
	//clusters
	offset = LittleLong(header.lumps[AASLUMP_CLUSTERS].fileofs);
	length = LittleLong(header.lumps[AASLUMP_CLUSTERS].filelen);
	aasworld[aasgvm].clusters = (aas_cluster_t *) AAS_LoadAASLump(fp, offset, length, &lastoffset, sizeof(aas_cluster_t));
	aasworld[aasgvm].numclusters = length / sizeof(aas_cluster_t);
	if (aasworld[aasgvm].numclusters && !aasworld[aasgvm].clusters) return BLERR_CANNOTREADAASLUMP;
	//swap everything
	AAS_SwapAASData();
	//aas file is loaded
	aasworld[aasgvm].loaded = qtrue;
	//close the file
	botimport.FS_FCloseFile(fp);
	//
#ifdef AASFILEDEBUG
	AAS_FileInfo();
#endif //AASFILEDEBUG
	//
	return BLERR_NOERROR;
} //end of the function AAS_LoadAASFile
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
static int AAS_WriteAASLump_offset;

int AAS_WriteAASLump(fileHandle_t fp, aas_header_t *h, int lumpnum, void *data, int length)
{
	aas_lump_t *lump;

	lump = &h->lumps[lumpnum];
	
	lump->fileofs = LittleLong(AAS_WriteAASLump_offset);	//LittleLong(ftell(fp));
	lump->filelen = LittleLong(length);

	if (length > 0)
	{
		botimport.FS_Write(data, length, fp );
	} //end if

	AAS_WriteAASLump_offset += length;

	return qtrue;
} //end of the function AAS_WriteAASLump
//===========================================================================
// aas data is useless after writing to file because it is byte swapped
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
qboolean AAS_WriteAASFile(char *filename)
{
	aas_header_t header;
	fileHandle_t fp;

	botimport.Print(PRT_MESSAGE, "writing %s\n", filename);
	//swap the aas data
	AAS_SwapAASData();
	//initialize the file header
	Com_Memset(&header, 0, sizeof(aas_header_t));
	header.ident = LittleLong(AASID);
	header.version = LittleLong(AASVERSION);
	header.bspchecksum = LittleLong(aasworld[aasgvm].bspchecksum);
	//open a new file
	botimport.FS_FOpenFile( filename, &fp, FS_WRITE );
	if (!fp)
	{
		botimport.Print(PRT_ERROR, "error opening %s\n", filename);
		return qfalse;
	} //end if
	//write the header
	botimport.FS_Write(&header, sizeof(aas_header_t), fp);
	AAS_WriteAASLump_offset = sizeof(aas_header_t);
	//add the data lumps to the file
	if (!AAS_WriteAASLump(fp, &header, AASLUMP_BBOXES, aasworld[aasgvm].bboxes,
		aasworld[aasgvm].numbboxes * sizeof(aas_bbox_t))) return qfalse;
	if (!AAS_WriteAASLump(fp, &header, AASLUMP_VERTEXES, aasworld[aasgvm].vertexes,
		aasworld[aasgvm].numvertexes * sizeof(aas_vertex_t))) return qfalse;
	if (!AAS_WriteAASLump(fp, &header, AASLUMP_PLANES, aasworld[aasgvm].planes,
		aasworld[aasgvm].numplanes * sizeof(aas_plane_t))) return qfalse;
	if (!AAS_WriteAASLump(fp, &header, AASLUMP_EDGES, aasworld[aasgvm].edges,
		aasworld[aasgvm].numedges * sizeof(aas_edge_t))) return qfalse;
	if (!AAS_WriteAASLump(fp, &header, AASLUMP_EDGEINDEX, aasworld[aasgvm].edgeindex,
		aasworld[aasgvm].edgeindexsize * sizeof(aas_edgeindex_t))) return qfalse;
	if (!AAS_WriteAASLump(fp, &header, AASLUMP_FACES, aasworld[aasgvm].faces,
		aasworld[aasgvm].numfaces * sizeof(aas_face_t))) return qfalse;
	if (!AAS_WriteAASLump(fp, &header, AASLUMP_FACEINDEX, aasworld[aasgvm].faceindex,
		aasworld[aasgvm].faceindexsize * sizeof(aas_faceindex_t))) return qfalse;
	if (!AAS_WriteAASLump(fp, &header, AASLUMP_AREAS, aasworld[aasgvm].areas,
		aasworld[aasgvm].numareas * sizeof(aas_area_t))) return qfalse;
	if (!AAS_WriteAASLump(fp, &header, AASLUMP_AREASETTINGS, aasworld[aasgvm].areasettings,
		aasworld[aasgvm].numareasettings * sizeof(aas_areasettings_t))) return qfalse;
	if (!AAS_WriteAASLump(fp, &header, AASLUMP_REACHABILITY, aasworld[aasgvm].reachability,
		aasworld[aasgvm].reachabilitysize * sizeof(aas_reachability_t))) return qfalse;
	if (!AAS_WriteAASLump(fp, &header, AASLUMP_NODES, aasworld[aasgvm].nodes,
		aasworld[aasgvm].numnodes * sizeof(aas_node_t))) return qfalse;
	if (!AAS_WriteAASLump(fp, &header, AASLUMP_PORTALS, aasworld[aasgvm].portals,
		aasworld[aasgvm].numportals * sizeof(aas_portal_t))) return qfalse;
	if (!AAS_WriteAASLump(fp, &header, AASLUMP_PORTALINDEX, aasworld[aasgvm].portalindex,
		aasworld[aasgvm].portalindexsize * sizeof(aas_portalindex_t))) return qfalse;
	if (!AAS_WriteAASLump(fp, &header, AASLUMP_CLUSTERS, aasworld[aasgvm].clusters,
		aasworld[aasgvm].numclusters * sizeof(aas_cluster_t))) return qfalse;
	//rewrite the header with the added lumps
	botimport.FS_Seek(fp, 0, FS_SEEK_SET);
	AAS_DData((unsigned char *) &header + 8, sizeof(aas_header_t) - 8);
	botimport.FS_Write(&header, sizeof(aas_header_t), fp);
	//close the file
	botimport.FS_FCloseFile(fp);
	return qtrue;
} //end of the function AAS_WriteAASFile
