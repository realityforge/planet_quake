#include "server.h"

#ifdef USE_MEMORY_MAPS

#include "../qcommon/cm_local.h"

/*
   ================
   WriteTGA
   ================
 */
void WriteTGA( const char *filename, byte *data, int width, int height ) {
	byte    *buffer;
	int i;
	int c;
	FILE    *f;

	buffer = Z_Malloc( width * height * 4 + 18 );
	memset( buffer, 0, 18 );

	buffer[2] = 2;      // uncompressed type
	buffer[12] = width & 255;
	buffer[13] = width >> 8;
	buffer[14] = height & 255;
	buffer[15] = height >> 8;
	buffer[16] = 32;    // pixel size

	// swap rgb to bgr
	c = 18 + width * height * 4;
	for ( i = 18 ; i < c ; i += 4 )
	{
		buffer[i + 0] = 0; //data[i - 18 + 2];       // blue
		buffer[i + 1] = 0; //data[i - 18 + 1];     // green
		buffer[i + 2] = data[i - 18 + 0];     // red
		buffer[i + 3] = data[i - 18 + 3];     // alpha
	}

	f = fopen( va("%s-r.tga", filename), "wb" );
	fwrite( buffer, 1, c, f );
	fclose( f );

	// swap rgb to bgr
	for ( i = 18 ; i < c ; i += 4 )
	{
		buffer[i + 0] = 0; //data[i - 18 + 2];       // blue
		buffer[i + 1] = data[i - 18 + 1];     // green
		buffer[i + 2] = 0; //data[i - 18 + 0];     // red
		buffer[i + 3] = data[i - 18 + 3];     // alpha
	}

	f = fopen( va("%s-g.tga", filename), "wb" );
	fwrite( buffer, 1, c, f );
	fclose( f );

	// swap rgb to bgr
	for ( i = 18 ; i < c ; i += 4 )
	{
		buffer[i + 0] = data[i - 18 + 2];       // blue
		buffer[i + 1] = data[i - 18 + 1];     // green
		buffer[i + 2] = data[i - 18 + 0];     // red
		buffer[i + 3] = data[i - 18 + 3];     // alpha
	}

	f = fopen( va("%s.tga", filename), "wb" );
	fwrite( buffer, 1, c, f );
	fclose( f );

	// swap rgb to bgr
	for ( i = 18 ; i < c ; i += 4 )
	{
		buffer[i + 0] = data[i - 18 + 2];       // blue
		buffer[i + 1] = 0; //data[i - 18 + 1];     // green
		buffer[i + 2] = 0; //data[i - 18 + 0];     // red
		buffer[i + 3] = data[i - 18 + 3];     // alpha
	}

	f = fopen( va("%s-b.tga", filename), "wb" );
	fwrite( buffer, 1, c, f );
	fclose( f );

	Z_Free( buffer );
}


static void SV_TraceArea(vec3_t angle, vec3_t scale, float *data1f) {
	trace_t		trace;
	vec3_t start, end, forward, right, up;
	//vec3_t dist;
	AngleVectors( angle, forward, right, up );

	for ( int y = 0; y < sv_bspMiniSize->integer; ++y ) {
		for ( int x = 0; x < sv_bspMiniSize->integer; ++x ) {
			if(data1f[y * sv_bspMiniSize->integer + x] == MAX_MAP_BOUNDS) {
				continue;
			}

			vec3_t newStart = {
				cm.cmodels[0].mins[0] + scale[0] * x, 
				cm.cmodels[0].mins[1] + scale[1] * y, 
				data1f[y * sv_bspMiniSize->integer + x] // bottom of sky brush
			};
			VectorMA( newStart, 8192, forward, end );
			VectorMA ( newStart, 1, forward, start );
			while (1) {
				CM_BoxTrace( &trace, start, end, vec3_origin, vec3_origin, 0, MASK_SOLID|MASK_SHOT, qfalse );
				//trap_Trace (&trace, tracefrom, NULL, NULL, end, passent, MASK_SHOT );

				// Hypo: break if we traversed length of vector tracefrom
				if (trace.fraction == 1.0 
					|| trace.entityNum == ENTITYNUM_NONE
					//
					|| isnan(trace.endpos[2])
				) {
					data1f[y * sv_bspMiniSize->integer + x] = MAX_MAP_BOUNDS; //VectorLength(dist);
					break;
				}

				// otherwise continue tracing thru walls
				if ((trace.surfaceFlags & SURF_NODRAW)
					|| (trace.surfaceFlags & SURF_NONSOLID)
					// this is a bit odd, normally we'd stop, but this helps trace through the tops of skyboxes for outlines
					|| (trace.surfaceFlags & SURF_SKY) 
				) {
					VectorMA (trace.endpos,1,forward,start);
					continue;
				}

				//VectorSubtract(newStart, trace.endpos, dist);
				data1f[y * sv_bspMiniSize->integer + x] = trace.endpos[2]; //VectorLength(dist);
				break;
			}
		}
	}

}


static void SV_FindFacets(int surfaceFlags, vec3_t scale, float *data1f) {
	for(int i = 0; i < cm.numBrushes; i++) {
		for(int j = 0; j < cm.brushes[i].numsides; j++) {
			//vec3_t cover;
			if (cm.brushes[i].sides[j].surfaceFlags & surfaceFlags) {
				//VectorScale(cm.brushes[i].sides[j].plane->normal, cm.brushes[i].sides[j].plane->dist, cover);
				if(!cm.brushes[i].sides[j].plane->normal[2]) {
					continue;
				}
				int startX = (cm.brushes[i].bounds[0][0] - cm.cmodels[0].mins[0]) / scale[0];
				int startY = (cm.brushes[i].bounds[0][1] - cm.cmodels[0].mins[1]) / scale[1];
				int endX = (cm.brushes[i].bounds[1][0] - cm.cmodels[0].mins[0]) / scale[0];
				int endY = (cm.brushes[i].bounds[1][1] - cm.cmodels[0].mins[1]) / scale[1];
				for ( int y = startY; y < endY; ++y ) {
					for ( int x = startX; x < endX; ++x ) {
						// mark the spot as being covered by the surface requested
						// MAX of min bounds
						if(data1f[y * sv_bspMiniSize->integer + x] == MAX_MAP_BOUNDS
							|| cm.brushes[i].bounds[0][2] > data1f[y * sv_bspMiniSize->integer + x]
						) {
							data1f[y * sv_bspMiniSize->integer + x] = cm.brushes[i].bounds[0][2];
						}
					}
				}
			}
		}
	}

}

void SV_XRay(float *d1, float *d2, float *d3, vec3_t scale) {
	memcpy(d2, d1, sv_bspMiniSize->integer * sv_bspMiniSize->integer * sizeof( float ));
	SV_TraceArea((vec3_t){89, 0, 0}, scale, d2);
	memcpy(d3, d2, sv_bspMiniSize->integer * sv_bspMiniSize->integer * sizeof( float ));
	SV_TraceArea((vec3_t){-89, 0, 0}, scale, d3);
}



static float *data1f, *data1f1, *data1f2, *data1f3;
static byte *data4b;

// store every facet in one color channel normalized to 255
// Or up to 3 traces / per XY coordinate can be summarized for things like caves, if we wanted
//   automatically simulate a cave in or rocks falling during Ground Zero opening maps
//  Or an avalanche we could detect all of the bumps or roofs or sides that stick out 
//    when it's over using RGB as XY with the color value being Z, or even XZ, YZ

void SV_MakeMinimap() {
	char minimapFilename[MAX_QPATH];
	float *q;
	byte *p;
	int i;
	vec3_t scale, size;

	VectorSubtract(cm.cmodels[0].maxs, cm.cmodels[0].mins, size);
	for(int i = 0; i < 3; i++) {
		// could use height from above to add a bunch of stupid black space around it
		//   but I like the extra dexterity - Brian Cullinan
		scale[i] = size[i] / sv_bspMiniSize->value;
	}

	if(!data1f)
		data1f = (float *)Z_Malloc( sv_bspMiniSize->integer * sv_bspMiniSize->integer * sizeof( float ) );
	if(!data1f1)
		data1f1 = (float *)Z_Malloc( sv_bspMiniSize->integer * sv_bspMiniSize->integer * sizeof( float ) );
	if(!data1f2)
		data1f2 = (float *)Z_Malloc( sv_bspMiniSize->integer * sv_bspMiniSize->integer * sizeof( float ) );
	if(!data1f3)
		data1f3 = (float *)Z_Malloc( sv_bspMiniSize->integer * sv_bspMiniSize->integer * sizeof( float ) );
	if(!data4b)
		data4b = (byte *)Z_Malloc( sv_bspMiniSize->integer * sv_bspMiniSize->integer * 4 );
	memset(data4b, 0, sv_bspMiniSize->integer * sv_bspMiniSize->integer * 4);
	for (size_t i = 0; i < sv_bspMiniSize->integer * sv_bspMiniSize->integer; ++i) {
		data1f[i] = MAX_MAP_BOUNDS;
		data1f1[i] = MAX_MAP_BOUNDS;
		data1f2[i] = MAX_MAP_BOUNDS;
		data1f3[i] = MAX_MAP_BOUNDS;
	}

	// this is where a combination solution comes in handy
	//   if trace from infinity, it hits the top of the skybox
	//   trace again and again and again and get some sort of solution
	// combo-solution, trace downward from all sky shaders, then trace
	//   upwards to make sure we hit sky again, if not, then we trace through a ceiling
	// thats only a 2 pass trace solution
	SV_FindFacets(SURF_SKY, scale, data1f);

	q = data1f;
	p = data4b;
	for ( int y = 0; y < sv_bspMiniSize->integer; ++y ) {
		for ( int x = 0; x < sv_bspMiniSize->integer; ++x ) {
			// sky, probably dealing with high numbers (i.e. close to top of map), 
			//   so subtract it from 128 to get very low numbers, this will have a less 
			//   boring effect on the redness of the image
			// this has a neat side effect of showing full bright where there is no rain,
			//   i.e. start AND stop in the sky, don't come down at all
			if(*q == MAX_MAP_BOUNDS) {
				p[0] = 0;
				p[1] = 0;
				p[2] = 255;
				*q = cm.cmodels[0].maxs[2]; // to get an outline of the map?
			} else {
				p[0] = 255;
				p[1] = 0;
				// initial sky value is small because it's subtracted from second floor ceiling
				p[2] = Com_Clamp( 0.f, 255.f / 256.f, (cm.cmodels[0].maxs[2] - *q) / size[2] ) * 256;
				p[3] = 255;
			}
			p += 4;
			q++;
		}
	}

	// subtract sky

	// Backup this value because this is the one we start with for the ground depths
	//   we will use it below to start with ground and the measure the next depth if there 
	//   are multiple floors, we will see at least 2
	SV_TraceArea((vec3_t){89, 0, 0}, scale, data1f);

	q = data1f;
	p = data4b;
	for ( int y = 0; y < sv_bspMiniSize->integer; ++y ) {
		for ( int x = 0; x < sv_bspMiniSize->integer; ++x ) {
			if(*q == MAX_MAP_BOUNDS) {
				p[0] = 255;
				p[1] = 0;
				p[2] = 0;
			} else {
				// always guarantee this is somewhat red to indicate it should rain
				p[0] = (int)( Com_Clamp( 0.f, 255.f / 256.f, (cm.cmodels[0].maxs[2] - *q) / size[2] ) * 256 ) | 1;
				p[1] = Com_Clamp( 0.f, 255.f / 256.f, (cm.cmodels[0].maxs[2] - *q) / size[2] ) * 256;
				p[2] = 0;
				p[3] = 255;
			}
			p += 4;
			q++;
		}
	}


	// display distance between ground and sub surfaces, i.e. thickness on green channel
	//  trace down to find a starting position for measuring the ceiling of the floor

	float *d1 = data1f1;
	float *d2 = data1f2;
	float *d3 = data1f3;

	memcpy(d1, data1f, sv_bspMiniSize->integer * sv_bspMiniSize->integer * sizeof( float ));

	i = 0;
	while(i < 5) {
		SV_XRay(d1, d2, d3, scale);

		p = data4b;
		for ( int y = 0; y < sv_bspMiniSize->integer; ++y ) {
			for ( int x = 0; x < sv_bspMiniSize->integer; ++x ) {
				if(d1[y * sv_bspMiniSize->integer + x] == MAX_MAP_BOUNDS) {
					//p += 4;
					//continue;
				}
				// if it hit from the first trace, look underneath
				if(d3[y * sv_bspMiniSize->integer + x] != MAX_MAP_BOUNDS 
					&& d2[y * sv_bspMiniSize->integer + x] != MAX_MAP_BOUNDS
					&& d2[y * sv_bspMiniSize->integer + x] < d3[y * sv_bspMiniSize->integer + x]
					&& d3[y * sv_bspMiniSize->integer + x] < d1[y * sv_bspMiniSize->integer + x]
				) {
					p[1] += Com_Clamp( 0.f, 255.f / 256.f, (d3[y * sv_bspMiniSize->integer + x] - d2[y * sv_bspMiniSize->integer + x]) / size[2] ) * 256;
				} else if (d3[y * sv_bspMiniSize->integer + x] == MAX_MAP_BOUNDS) {
					//p[1] += Com_Clamp( 0.f, 255.f / 256.f, (data1f[y * sv_bspMiniSize->integer + x] - cm.cmodels[0].mins[2]) / size[2] ) * 256;
				} else if (d2[y * sv_bspMiniSize->integer + x] == MAX_MAP_BOUNDS) {
					printf("hit\n");
					//p[1] += Com_Clamp( 0.f, 255.f / 256.f, (cm.cmodels[0].maxs[2] - d1[y * sv_bspMiniSize->integer + x]) / size[2] ) * 256;
				}
				p += 4;
			}
		}

		// switch the floor starting point, d2 will be overwritten
		memcpy(d1, d2, sv_bspMiniSize->integer * sv_bspMiniSize->integer * sizeof( float ));
		//d2 = data1f;
		i++;
	}
	



	// finally reverse all the greens so it doesn't look inside out
	p = data4b;
	for ( int y = 0; y < sv_bspMiniSize->integer; ++y ) {
		for ( int x = 0; x < sv_bspMiniSize->integer; ++x ) {
			p[1] = 256 - p[1];
			p += 4;
		}
	}


	COM_StripExtension(cm.name, minimapFilename, sizeof(minimapFilename));
	char *mapPath = FS_BuildOSPath( Cvar_VariableString("fs_homepath"), Cvar_VariableString("fs_game"), minimapFilename );
	Com_Printf( " writing to %s...", mapPath );
	WriteTGA( mapPath, data4b, sv_bspMiniSize->integer, sv_bspMiniSize->integer );

	Com_Printf( " done.\n" );

	if(data4b)
		Z_Free(data4b);
	if(data1f)
		Z_Free(data1f);
	if(data1f1)
		Z_Free(data1f1);
	if(data1f2)
		Z_Free(data1f2);
	if(data1f3)
		Z_Free(data1f3);
}

#endif
