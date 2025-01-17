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
#include "tr_local.h"

/*
=====================
R_PerformanceCounters
=====================
*/
static void R_PerformanceCounters( void ) {
	if ( !r_speeds->integer ) {
		// clear the counters even if we aren't printing
		Com_Memset( &tr.pc, 0, sizeof( tr.pc ) );
		Com_Memset( &backEnd.pc, 0, sizeof( backEnd.pc ) );
		return;
	}

	if (r_speeds->integer == 1) {
		ri.Printf (PRINT_ALL, "%i/%i shaders/surfs %i leafs %i verts %i/%i tris %.2f mtex %.2f dc\n",
			backEnd.pc.c_shaders, backEnd.pc.c_surfaces, tr.pc.c_leafs, backEnd.pc.c_vertexes, 
			backEnd.pc.c_indexes/3, backEnd.pc.c_totalIndexes/3, 
			R_SumOfUsedImages()/(1000000.0f), backEnd.pc.c_overDraw / (float)(glConfig.vidWidth * glConfig.vidHeight) ); 
	} else if (r_speeds->integer == 2) {
		ri.Printf (PRINT_ALL, "(patch) %i sin %i sclip  %i sout %i bin %i bclip %i bout\n",
			tr.pc.c_sphere_cull_patch_in, tr.pc.c_sphere_cull_patch_clip, tr.pc.c_sphere_cull_patch_out, 
			tr.pc.c_box_cull_patch_in, tr.pc.c_box_cull_patch_clip, tr.pc.c_box_cull_patch_out );
		ri.Printf (PRINT_ALL, "(md3) %i sin %i sclip  %i sout %i bin %i bclip %i bout\n",
			tr.pc.c_sphere_cull_md3_in, tr.pc.c_sphere_cull_md3_clip, tr.pc.c_sphere_cull_md3_out, 
			tr.pc.c_box_cull_md3_in, tr.pc.c_box_cull_md3_clip, tr.pc.c_box_cull_md3_out );
	} else if (r_speeds->integer == 3) {
		ri.Printf (PRINT_ALL, "viewcluster: %i\n", tr.viewCluster );
	} else if (r_speeds->integer == 4) {
		if ( backEnd.pc.c_dlightVertexes ) {
			ri.Printf (PRINT_ALL, "dlight srf:%i  culled:%i  verts:%i  tris:%i\n", 
				tr.pc.c_dlightSurfaces, tr.pc.c_dlightSurfacesCulled,
				backEnd.pc.c_dlightVertexes, backEnd.pc.c_dlightIndexes / 3 );
		}
	} 
	else if (r_speeds->integer == 5 )
	{
		ri.Printf( PRINT_ALL, "zFar: %.0f\n", tr.viewParms.zFar );
	}
	else if (r_speeds->integer == 6 )
	{
		ri.Printf( PRINT_ALL, "flare adds:%i tests:%i renders:%i\n", 
			backEnd.pc.c_flareAdds, backEnd.pc.c_flareTests, backEnd.pc.c_flareRenders );
	}

	Com_Memset( &tr.pc, 0, sizeof( tr.pc ) );
	Com_Memset( &backEnd.pc, 0, sizeof( backEnd.pc ) );
}


/*
====================
R_IssueRenderCommands
====================
*/
static void R_IssueRenderCommands( void ) {
	renderCommandList_t	*cmdList;

	cmdList = &backEndData->commands;

	// add an end-of-list command
#ifdef USE_UNLOCKED_CVARS
	int cmdUsed = cmdList->used % MAX_RENDER_DIVISOR;
	int cmdSubList = (cmdList->used - cmdUsed) / MAX_RENDER_DIVISOR;
	*(int *)(cmdList->cmds[ cmdSubList ] + cmdUsed) = RC_END_OF_LIST;
#else
	*(int *)(cmdList->cmds + cmdList->used) = RC_END_OF_LIST;
#endif

	// clear it out, in case this is a sync and not a buffer flip
	cmdList->used = 0;

	if ( backEnd.screenshotMask == 0 ) {
		if ( ri.CL_IsMinimized() )
			return; // skip backend when minimized
		if ( backEnd.throttle )
			return; // or throttled on demand
	}

	// actually start the commands going
	if ( !r_skipBackEnd->integer ) {
		// let it start on the new batch
#ifdef USE_UNLOCKED_CVARS
		for(int i = 0; i <= cmdSubList; i++) {
			RB_ExecuteRenderCommands( cmdList->cmds[i] );
		}
#else
		RB_ExecuteRenderCommands( cmdList->cmds );
#endif
	}
}


/*
====================
R_IssuePendingRenderCommands

Issue any pending commands and wait for them to complete.
====================
*/
void R_IssuePendingRenderCommands( void ) {
	if ( !tr.registered ) {
		return;
	}
	R_IssueRenderCommands();
}


/*
============
R_GetCommandBufferReserved

make sure there is enough command space
============
*/
static void *R_GetCommandBufferReserved( int bytes, int reservedBytes ) {
	renderCommandList_t	*cmdList;

	cmdList = &backEndData->commands;
	bytes = PAD(bytes, sizeof(void *));

	// always leave room for the end of list command
#ifdef USE_UNLOCKED_CVARS
	if ( cmdList->used + bytes + sizeof( int ) + reservedBytes > r_maxcmds->integer ) {
		if ( bytes > r_maxcmds->integer - sizeof( int ) ) {
			ri.Error( ERR_FATAL, "R_GetCommandBuffer: bad size %i", bytes );
		}
		// if we run out of room, just start dropping commands
		return NULL;
	}

	// expand the commands list when necessary
	int cmdUsed = cmdList->used % MAX_RENDER_DIVISOR;
	int cmdSubList = (cmdList->used - cmdUsed) / MAX_RENDER_DIVISOR;
	if(cmdUsed + bytes >= MAX_RENDER_DIVISOR) {
		if(!cmdList->cmds[cmdSubList + 1]) {
			Com_Printf("Expanding the command list one time.\n");
			cmdList->cmds[cmdSubList + 1] = ri.Hunk_Alloc(sizeof(byte) * MAX_RENDER_DIVISOR, h_low);
		}

		*(int *)(cmdList->cmds[cmdSubList] + cmdUsed) = RC_END_OF_LIST;
		cmdList->used = (cmdSubList + 1) * MAX_RENDER_DIVISOR + bytes;

		return cmdList->cmds[cmdSubList + 1];
	} else {
		cmdList->used += bytes;

		return cmdList->cmds[cmdSubList] + cmdUsed;
	}
#else
	if ( cmdList->used + bytes + sizeof( int ) + reservedBytes > MAX_RENDER_COMMANDS ) {
		if ( bytes > MAX_RENDER_COMMANDS - sizeof( int ) ) {
			ri.Error( ERR_FATAL, "R_GetCommandBuffer: bad size %i", bytes );
		}
		// if we run out of room, just start dropping commands
		return NULL;
	}

	cmdList->used += bytes;

	return cmdList->cmds + cmdList->used - bytes;
#endif
}


/*
=============
R_GetCommandBuffer
returns NULL if there is not enough space for important commands
=============
*/
static void *R_GetCommandBuffer( int bytes ) {
	return R_GetCommandBufferReserved( bytes, PAD( sizeof( swapBuffersCommand_t ), sizeof(void *) ) );
}


/*
=============
R_AddDrawSurfCmd
=============
*/
void R_AddDrawSurfCmd( drawSurf_t *drawSurfs, int numDrawSurfs ) {
	drawSurfsCommand_t	*cmd;

	cmd = R_GetCommandBuffer( sizeof( *cmd ) );
	if ( !cmd ) {
		return;
	}
	cmd->commandId = RC_DRAW_SURFS;

	cmd->drawSurfs = drawSurfs;
	cmd->numDrawSurfs = numDrawSurfs;

	cmd->refdef = tr.refdef;
	cmd->viewParms = tr.viewParms;
}


#ifdef USE_MULTIVM_CLIENT
void R_SetWorld(viewParms_t *oldParms, viewParms_t *newParms) {
	// first, add a world command to this world to switch command buffers
	setWorldCommand_t	*cmd1;
	cmd1 = R_GetCommandBuffer( sizeof( *cmd1 ) );
	cmd1->commandId = RC_SET_WORLD;
	cmd1->world = ri.worldMaps[newParms->newWorld];

	// then add a command to the new world to skip the render sequence 
	//   it switches to at the end of frame so it doesn't render twice
	rwi = ri.worldMaps[newParms->newWorld];
	setWorldCommand_t	*cmd2;
	cmd2 = R_GetCommandBuffer( sizeof( *cmd2 ) );
	cmd2->commandId = RC_SET_WORLD;
	cmd2->world = rwi; // same world
	// update the first setWorldCommand to where to pick up after this skip command
	cmd1->next = (const void *)(cmd2 + 1);

	// render commands on newWorld command buffer
	R_RenderView( newParms );
	// TODO: fix oldParms should come from newWorld?
	tr.viewParms = *oldParms; // happens in new world, so reset in new world before `rwi` changes

	// then add a command to switch back to original world
	setWorldCommand_t	*cmd3;
	cmd3 = R_GetCommandBuffer( sizeof( *cmd3 ) );
	cmd3->commandId = RC_SET_WORLD;
	cmd3->world = ri.worldMaps[oldParms->newWorld];
	cmd3->next = (const void *)(cmd1 + 1);
	// update the skip command (cmd2) to skip the number of commands this render added
	cmd2->next = (const void *)(cmd3 + 1);

	rwi = ri.worldMaps[oldParms->newWorld];
}
#endif


/*
=============
RE_SetColor

Passing NULL will set the color to white
=============
*/
void RE_SetColor( const float *rgba ) {
	setColorCommand_t	*cmd;

	if ( !tr.registered ) {
		return;
	}
	cmd = R_GetCommandBuffer( sizeof( *cmd ) );
	if ( !cmd ) {
		return;
	}
	cmd->commandId = RC_SET_COLOR;
	if ( !rgba ) {
		rgba = colorWhite;
	}

	cmd->color[0] = rgba[0];
	cmd->color[1] = rgba[1];
	cmd->color[2] = rgba[2];
	cmd->color[3] = rgba[3];
}


#ifdef USE_RMLUI
extern int r_numpolyverts;
extern int r_numindexes;
typedef struct {
  vec2_t		xy;
	struct {
    byte red;
    byte green;
    byte blue;
    byte alpha;
  } colour;
  vec2_t		tex_coord;
} rocketVertex_t;

void RE_RenderGeometry(void *vertices, int num_vertices, int* indices, 
                        int num_indices, qhandle_t texture, const vec2_t translation) {
  polyIndexedCommand_t	*cmd;
  rocketVertex_t *rmlVs = (rocketVertex_t *)vertices;

  if ( !tr.registered ) {
    return;
  }
  if ( r_numpolyverts + num_vertices > r_maxpolyverts->integer || r_numindexes + num_indices > r_maxpolyverts->integer ) {
    ri.Printf( PRINT_DEVELOPER, "WARNING: RE_AddPolyToScene: r_maxpolyverts reached\n");
    return;
  }
	cmd = R_GetCommandBuffer( sizeof( *cmd ) );
	if ( !cmd ) {
		return;
	}
  cmd->commandId = RC_POLY2D_INDEXED;
  cmd->shader = R_GetShaderByHandle( texture );
  cmd->numVerts = num_vertices;
#ifdef USE_UNLOCKED_CVARS
		int vertsUsed = r_numpolyverts % MAX_POLYVERTS_DIVISOR;
		int vertsList = (r_numpolyverts - vertsUsed) / MAX_POLYVERTS_DIVISOR;
		if(vertsUsed + 1 >= MAX_POLYVERTS_DIVISOR) {
			Com_Printf("Expanding the polyverts list one time.\n");
			backEndData->polyVerts[vertsList + 1] = ri.Hunk_Alloc(sizeof(polyVert_t) * MAX_POLYVERTS_DIVISOR, h_low);
			r_numpolyverts = (vertsList + 1) * MAX_POLYVERTS_DIVISOR;
			cmd->verts = &backEndData->polyVerts[vertsList + 1][0];
		} else {
			cmd->verts = &backEndData->polyVerts[vertsList][vertsUsed];
		}
#else
  cmd->verts = &backEndData->polyVerts[ r_numpolyverts ];
#endif
	for(int i = 0; i < num_vertices; i++) {
    cmd->verts[i].xyz[0] = rmlVs[i].xy[0];
    cmd->verts[i].xyz[1] = rmlVs[i].xy[1];
    cmd->verts[i].xyz[2] = 0;
    cmd->verts[i].st[0] = rmlVs[i].tex_coord[0];
    cmd->verts[i].st[1] = rmlVs[i].tex_coord[1];
    cmd->verts[i].modulate.rgba[0] = rmlVs[i].colour.red;
    cmd->verts[i].modulate.rgba[1] = rmlVs[i].colour.green;
    cmd->verts[i].modulate.rgba[2] = rmlVs[i].colour.blue;
    cmd->verts[i].modulate.rgba[3] = rmlVs[i].colour.alpha;
  }
  cmd->numIndexes = num_indices;
#ifdef USE_UNLOCKED_CVARS
	int indexUsed = r_numindexes % MAX_POLYVERTS_DIVISOR;
	int indexList = (r_numindexes - indexUsed) / MAX_POLYVERTS_DIVISOR;
	if(indexUsed + num_indices >= MAX_POLYVERTS_DIVISOR) {
		Com_Printf("Expanding the index list one time.\n");
		backEndData->indexes[indexList + 1] = ri.Hunk_Alloc(sizeof(srfPoly_t) * MAX_POLYVERTS_DIVISOR, h_low);
		cmd->indexes = &backEndData->indexes[indexList + 1][0];
		r_numindexes = (indexList + 1) * MAX_POLYVERTS_DIVISOR;
	} else {
		cmd->indexes = &backEndData->indexes[indexList][indexUsed];
	}
#else
  cmd->indexes = &backEndData->indexes[ r_numindexes ];
#endif
	memcpy( cmd->indexes, indices, sizeof( int ) * num_indices );
  cmd->translation[0] = translation[0];
  cmd->translation[1] = translation[1];

  r_numpolyverts += num_vertices;
	r_numindexes += num_indices;
}
#endif


/*
=============
RE_StretchPic
=============
*/
void RE_StretchPic( float x, float y, float w, float h,
					float s1, float t1, float s2, float t2, qhandle_t hShader ) {
	stretchPicCommand_t	*cmd;

	if ( !tr.registered ) {
		return;
	}
	cmd = R_GetCommandBuffer( sizeof( *cmd ) );
	if ( !cmd ) {
		return;
	}
	cmd->commandId = RC_STRETCH_PIC;
	cmd->shader = R_GetShaderByHandle( hShader );
#ifdef USE_MULTIVM_CLIENT
	cmd->x = x * dvrXScale + (dvrXOffset * glConfig.vidWidth);
	cmd->y = y * dvrYScale + (dvrYOffset * glConfig.vidHeight);
	cmd->w = w * dvrXScale;
	cmd->h = h * dvrYScale;
#else
	cmd->x = x;
	cmd->y = y;
	cmd->w = w;
	cmd->h = h;
#endif
	cmd->s1 = s1;
	cmd->t1 = t1;
	cmd->s2 = s2;
	cmd->t2 = t2;
}

#define MODE_RED_CYAN	1
#define MODE_RED_BLUE	2
#define MODE_RED_GREEN	3
#define MODE_GREEN_MAGENTA 4
#define MODE_MAX	MODE_GREEN_MAGENTA

static void R_SetColorMode(GLboolean *rgba, stereoFrame_t stereoFrame, int colormode)
{
	rgba[0] = rgba[1] = rgba[2] = rgba[3] = GL_TRUE;

	if(colormode > MODE_MAX)
	{
		if(stereoFrame == STEREO_LEFT)
			stereoFrame = STEREO_RIGHT;
		else if(stereoFrame == STEREO_RIGHT)
			stereoFrame = STEREO_LEFT;

		colormode -= MODE_MAX;
	}

	if(colormode == MODE_GREEN_MAGENTA)
	{
		if(stereoFrame == STEREO_LEFT)
			rgba[0] = rgba[2] = GL_FALSE;
		else if(stereoFrame == STEREO_RIGHT)
			rgba[1] = GL_FALSE;
	}
	else
	{
		if(stereoFrame == STEREO_LEFT)
			rgba[1] = rgba[2] = GL_FALSE;
		else if(stereoFrame == STEREO_RIGHT)
		{
			rgba[0] = GL_FALSE;

			if(colormode == MODE_RED_BLUE)
				rgba[1] = GL_FALSE;
			else if(colormode == MODE_RED_GREEN)
				rgba[2] = GL_FALSE;
		}
	}
}


/*
====================
RE_BeginFrame

If running in stereo, RE_BeginFrame will be called twice
for each RE_EndFrame
====================
*/
void RE_BeginFrame( stereoFrame_t stereoFrame ) {
	drawBufferCommand_t	*cmd = NULL;
	colorMaskCommand_t *colcmd = NULL;
	clearColorCommand_t *clrcmd = NULL;

	if ( !tr.registered ) {
		return;
	}

	glState.finishCalled = qfalse;

	tr.frameCount++;
	tr.frameSceneNum = 0;

	backEnd.doneBloom = qfalse;

	// check for errors
	GL_CheckErrors();

	if ( ( cmd = R_GetCommandBuffer( sizeof( *cmd ) ) ) == NULL )
		return;
	cmd->commandId = RC_DRAW_BUFFER;

	if ( glConfig.stereoEnabled ) {
		if ( stereoFrame == STEREO_LEFT ) {
			cmd->buffer = (int)GL_BACK_LEFT;
		} else if ( stereoFrame == STEREO_RIGHT ) {
			cmd->buffer = (int)GL_BACK_RIGHT;
		} else {
			ri.Error( ERR_FATAL, "RE_BeginFrame: Stereo is enabled, but stereoFrame was %i", stereoFrame );
		}
	}
	else
	{
		if ( !Q_stricmp( r_drawBuffer->string, "GL_FRONT" ) )
			cmd->buffer = (int)GL_FRONT;
		else
			cmd->buffer = (int)GL_BACK;

		if ( r_anaglyphMode->integer )
		{
			if ( r_anaglyphMode->modified )
			{
				clrcmd = R_GetCommandBuffer( sizeof( *clrcmd ) );
				if ( clrcmd ) {
					Com_Memset( clrcmd, 0, sizeof( *clrcmd ) );
					clrcmd->commandId = RC_CLEARCOLOR;
				} else {
					return;
				}
				clrcmd->colorMask = qtrue;
				if ( !fboEnabled ) {
					// clear both, front and backbuffer.
					clrcmd->frontAndBack = qtrue;
				}
			}

			if ( stereoFrame == STEREO_LEFT )
			{
				// first frame
			}
			else if ( stereoFrame == STEREO_RIGHT )
			{
				clearDepthCommand_t *cldcmd;
				
				if ( (cldcmd = R_GetCommandBuffer(sizeof(*cldcmd))) == NULL )
					return;

				cldcmd->commandId = RC_CLEARDEPTH;
			}
			else
				ri.Error( ERR_FATAL, "RE_BeginFrame: Stereo is enabled, but stereoFrame was %i", stereoFrame );

			if ( (colcmd = R_GetCommandBuffer(sizeof(*colcmd))) == NULL )
				return;

			R_SetColorMode( colcmd->rgba, stereoFrame, r_anaglyphMode->integer );
			colcmd->commandId = RC_COLORMASK;
		}
		else // !r_anaglyphMode->integer
		{
			if ( stereoFrame != STEREO_CENTER )
				ri.Error( ERR_FATAL, "RE_BeginFrame: Stereo is disabled, but stereoFrame was %i", stereoFrame );

			// reset color mask
			if ( r_anaglyphMode->modified )	{
				if ( ( colcmd = R_GetCommandBuffer( sizeof( *colcmd ) ) ) == NULL )
					return;

				R_SetColorMode( colcmd->rgba, stereoFrame, r_anaglyphMode->integer );
				colcmd->commandId = RC_COLORMASK;
			}
		}
	}

	if ( r_fastsky->integer ) {
		if ( stereoFrame != STEREO_RIGHT ) {
			if ( !clrcmd ) {
				clrcmd = R_GetCommandBuffer( sizeof( *clrcmd ) );
				if ( clrcmd ) {
					Com_Memset( clrcmd, 0, sizeof( *clrcmd ) );
					clrcmd->commandId = RC_CLEARCOLOR;
				} else {
					return;
				}
			}
			clrcmd->fullscreen = qtrue;
			if ( r_anaglyphMode->integer ) {
				clrcmd->colorMask = qtrue;
			}
		}
	}

	tr.refdef.stereoFrame = stereoFrame;
}


/*
=============
RE_EndFrame

Returns the number of msec spent in the back end
=============
*/
void RE_EndFrame( int *frontEndMsec, int *backEndMsec ) {

	swapBuffersCommand_t *cmd;

	if ( !tr.registered ) {
		return;
	}

	cmd = R_GetCommandBufferReserved( sizeof( *cmd ), 0 );
	if ( !cmd ) {
		return;
	}
	cmd->commandId = RC_SWAP_BUFFERS;

	R_PerformanceCounters();

#ifdef USE_MULTIVM_CLIENT
	for(int i = 0; i < MAX_NUM_VMS; i++) {
		rwi = i;
//ri.Printf(PRINT_ALL, "cmds: %i -> %i\n", rwi, backEndData->commands.used);
		R_IssueRenderCommands();

		R_InitNextFrame();
	}
	rwi = 0;
#else
//ri.Printf(PRINT_ALL, "cmds: %i\n", backEndData->commands.used);
	R_IssueRenderCommands();

	R_InitNextFrame();
#endif

	if ( frontEndMsec ) {
		*frontEndMsec = tr.frontEndMsec;
	}
	tr.frontEndMsec = 0;
	if ( backEndMsec ) {
		*backEndMsec = backEnd.pc.msec;
	}
	backEnd.pc.msec = 0;
	backEnd.throttle = qfalse;

	// recompile GPU shaders if needed
	if ( ri.Cvar_CheckGroup( CVG_RENDERER ) )
	{
		ARB_UpdatePrograms();
		if ( r_ext_multisample->modified || r_hdr->modified )
			QGL_InitFBO();

		if ( r_textureMode->modified )
			GL_TextureMode( r_textureMode->string );

		if ( r_gamma->modified )
			R_SetColorMappings();

		ri.Cvar_ResetGroup( CVG_RENDERER, qtrue );
	}
}


/*
=============
RE_TakeVideoFrame
=============
*/
void RE_TakeVideoFrame( int width, int height,
		byte *captureBuffer, byte *encodeBuffer, qboolean motionJpeg )
{
	videoFrameCommand_t	*cmd;

	if( !tr.registered ) {
		return;
	}

	backEnd.screenshotMask |= SCREENSHOT_AVI;

	cmd = &backEnd.vcmd;

	//cmd->commandId = RC_VIDEOFRAME;

	cmd->width = width;
	cmd->height = height;
	cmd->captureBuffer = captureBuffer;
	cmd->encodeBuffer = encodeBuffer;
	cmd->motionJpeg = motionJpeg;
}


void RE_ThrottleBackend( void )
{
	backEnd.throttle = qtrue;
}


void RE_FinishBloom( void )
{
	finishBloomCommand_t *cmd;

	if ( !tr.registered ) {
		return;
	}

	cmd = R_GetCommandBuffer( sizeof( *cmd ) );
	if ( !cmd ) {
		return;
	}

	cmd->commandId = RC_FINISHBLOOM;
}


qboolean RE_CanMinimize( void )
{
	return fboEnabled;
}


const glconfig_t *RE_GetConfig( void )
{
	return &glConfig;
}


void RE_VertexLighting( qboolean allowed )
{
	tr.vertexLightingAllowed = allowed;
}
