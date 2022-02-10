#include "server.h"

#ifdef USE_MEMORY_MAPS

#include "../qcommon/cm_local.h"

#if 0
// TODO: try to copy the minimap code in here

typedef enum
{
	EMMM_Gray,
	EMMM_Black,
	EMMM_White,
	EMMM_Color
} EMiniMapMode;

typedef struct 
{
	dbrush_t *brushes;
	int numBrushes;
	dbrushside_t *brushsides;
	int numBrushSides;
	dplane_t *planes;
	int numPlanes;
	dmodel_t *bmodels;
	int numBModels;
	dshader_t *shaders;
	int numShaders;
	int width;
	int height;
	int samples;
	float *sample_offsets;
	float sharpen_boxmult;
	float sharpen_centermult;
	float boost, brightness, contrast;
	float *data1f;
	float *sharpendata1f;
	vec3_t mins, size;
} minimap_t;

static minimap_t minimap;
int numOpaqueBrushes;
byte *opaqueBrushes;

#define	SURFACE_CLIP_EPSILON	(0.125)

qboolean BrushIntersectionWithLine( dbrush_t *brush, const vec3_t start, 
	const vec3_t dir, float *t_in, float *t_out )
{
	int i;
	qboolean in = qfalse, out = qfalse;
	dbrushside_t *sides = &minimap.brushsides[brush->firstSide];

	for ( i = 0; i < brush->numSides; ++i )
	{
		const dplane_t *p = &minimap.planes[sides[i].planeNum];
		float sn = DotProduct( start, p->normal );
		float dn = DotProduct( dir, p->normal );
		if ( dn == 0 ) {
			if ( sn > p->dist ) {
				return qfalse; // outside!
			}
		}
		else
		{
			float t = ( p->dist - sn ) / dn;
			if ( dn < 0 ) {
				if ( !in || t > *t_in ) {
					*t_in = t;
					in = qtrue;
					// as t_in can only increase, and t_out can only decrease, early out
					if ( out && *t_in >= *t_out ) {
						return qfalse;
					}
				}
			}
			else
			{
				if ( !out || t < *t_out ) {
					*t_out = t;
					out = qtrue;
					// as t_in can only increase, and t_out can only decrease, early out
					if ( in && *t_in >= *t_out ) {
						return qfalse;
					}
				}
			}
		}
	}
	return in && out;
}

static float MiniMapSample( float x, float y ){
	int i, bi;
	float t0, t1;
	float samp;
	dbrush_t *b;
	dbrushside_t *s;
	int cnt;

	const vec3_t org = { x, y, 0 };
	const vec3_t dir = { 0, 0, 1 };

	cnt = 0;
	samp = 0;
	for ( i = 0; i < minimap.bmodels[0].numBrushes; ++i )
	{
		bi = minimap.bmodels[0].firstBrush + i;
		if ( opaqueBrushes[bi >> 3] & ( 1 << ( bi & 7 ) ) ) {
			b = &minimap.brushes[bi];

			// sort out mins/maxs of the brush
			s = &minimap.brushsides[b->firstSide];
			if ( x < -minimap.planes[s[0].planeNum].dist ) {
				continue;
			}
			if ( x > +minimap.planes[s[1].planeNum].dist ) {
				continue;
			}
			if ( y < -minimap.planes[s[2].planeNum].dist ) {
				continue;
			}
			if ( y > +minimap.planes[s[3].planeNum].dist ) {
				continue;
			}

			if ( BrushIntersectionWithLine( b, org, dir, &t0, &t1 ) ) {
				samp += t1 - t0;
				++cnt;
			}
		}
	}

	return samp;
}

void RandomVector2f( float v[2] ){
	do
	{
		v[0] = 2 * random() - 1;
		v[1] = 2 * random() - 1;
	}
	while ( v[0] * v[0] + v[1] * v[1] > 1 );
}

static void MiniMapRandomlySupersampled( int y ){
	int x, i;
	float *p = &minimap.data1f[y * minimap.width];
	float ymin = minimap.mins[1] + minimap.size[1] * ( y / (float) minimap.height );
	float dx   =                   minimap.size[0]      / (float) minimap.width;
	float dy   =                   minimap.size[1]      / (float) minimap.height;
	float uv[2];
	float thisval;

	for ( x = 0; x < minimap.width; ++x )
	{
		float xmin = minimap.mins[0] + minimap.size[0] * ( x / (float) minimap.width );
		float val = 0;

		for ( i = 0; i < minimap.samples; ++i )
		{
			RandomVector2f( uv );
			thisval = MiniMapSample(
			              xmin + ( uv[0] + 0.5 ) * dx, /* exaggerated random pattern for better results */
			              ymin + ( uv[1] + 0.5 ) * dy  /* exaggerated random pattern for better results */
			          );
			val += thisval;
		}
		val /= minimap.samples * minimap.size[2];
		*p++ = val;
	}
}

static void MiniMapSupersampled( int y ){
	int x, i;
	float *p = &minimap.data1f[y * minimap.width];
	float ymin = minimap.mins[1] + minimap.size[1] * ( y / (float) minimap.height );
	float dx   =                   minimap.size[0]      / (float) minimap.width;
	float dy   =                   minimap.size[1]      / (float) minimap.height;

	for ( x = 0; x < minimap.width; ++x )
	{
		float xmin = minimap.mins[0] + minimap.size[0] * ( x / (float) minimap.width );
		float val = 0;

		for ( i = 0; i < minimap.samples; ++i )
		{
			float thisval = MiniMapSample(
			                    xmin + minimap.sample_offsets[2 * i + 0] * dx,
			                    ymin + minimap.sample_offsets[2 * i + 1] * dy
			                );
			val += thisval;
		}
		val /= minimap.samples * minimap.size[2];
		*p++ = val;
	}
}

static void MiniMapNoSupersampling( int y ){
	int x;
	float *p = &minimap.data1f[y * minimap.width];
	float ymin = minimap.mins[1] + minimap.size[1] * ( ( y + 0.5 ) / (float) minimap.height );

	for ( x = 0; x < minimap.width; ++x )
	{
		float xmin = minimap.mins[0] + minimap.size[0] * ( ( x + 0.5 ) / (float) minimap.width );
		*p = MiniMapSample( xmin, ymin ) / minimap.size[2];
		p++;
	}
}

static void MiniMapSharpen( int y ){
	int x;
	const qboolean up = ( y > 0 );
	const qboolean down = ( y < minimap.height - 1 );
	float *p = &minimap.data1f[y * minimap.width];
	float *q = &minimap.sharpendata1f[y * minimap.width];

	for ( x = 0; x < minimap.width; ++x )
	{
		const qboolean left = ( x > 0 );
		const qboolean right = ( x < minimap.width - 1 );
		float val = p[0] * minimap.sharpen_centermult;

		if ( left && up ) {
			val += p[-1 - minimap.width] * minimap.sharpen_boxmult;
		}
		if ( left && down ) {
			val += p[-1 + minimap.width] * minimap.sharpen_boxmult;
		}
		if ( right && up ) {
			val += p[+1 - minimap.width] * minimap.sharpen_boxmult;
		}
		if ( right && down ) {
			val += p[+1 + minimap.width] * minimap.sharpen_boxmult;
		}

		if ( left ) {
			val += p[-1] * minimap.sharpen_boxmult;
		}
		if ( right ) {
			val += p[+1] * minimap.sharpen_boxmult;
		}
		if ( up ) {
			val += p[-minimap.width] * minimap.sharpen_boxmult;
		}
		if ( down ) {
			val += p[+minimap.width] * minimap.sharpen_boxmult;
		}

		++p;
		*q++ = val;
	}
}

static void MiniMapContrastBoost( int y ){
	int x;
	float *q = &minimap.data1f[y * minimap.width];
	for ( x = 0; x < minimap.width; ++x )
	{
		*q = *q * minimap.boost / ( ( minimap.boost - 1 ) * *q + 1 );
		++q;
	}
}

static void MiniMapBrightnessContrast( int y ){
	int x;
	float *q = &minimap.data1f[y * minimap.width];
	for ( x = 0; x < minimap.width; ++x )
	{
		*q = *q * minimap.contrast + minimap.brightness;
		++q;
	}
}

static void MiniMapMakeMinsMaxs( const vec3_t mins_in, const vec3_t maxs_in, 
	float border, qboolean keepaspect )
{
	vec3_t mins, maxs;
	vec3_t extend;
	VectorCopy(mins_in, mins);
	VectorCopy(maxs_in, maxs);

	// line compatible to nexuiz mapinfo
	Com_Printf( "size %f %f %f %f %f %f\n", mins[0], mins[1], mins[2], maxs[0], maxs[1], maxs[2] );

	if ( keepaspect ) {
		VectorSubtract(maxs, mins, extend);
		if ( extend[1] > extend[0] ) {
			mins[0] -= ( extend[1] - extend[0] ) * 0.5;
			maxs[0] += ( extend[1] - extend[0] ) * 0.5;
		}
		else
		{
			mins[1] -= ( extend[0] - extend[1] ) * 0.5;
			maxs[1] += ( extend[0] - extend[1] ) * 0.5;
		}
	}

	/* border: amount of black area around the image */
	/* input: border, 1-2*border, border but we need border/(1-2*border) */
	VectorSubtract(maxs, mins, extend);
	VectorScale(extend, border / ( 1 - 2 * border), extend);
	VectorSubtract(mins, extend, mins);
	VectorAdd(maxs, extend, maxs);

	VectorCopy(mins, minimap.mins);
	VectorSubtract(maxs, mins, minimap.size);

	// line compatible to nexuiz mapinfo
	Com_Printf( "size_texcoords %f %f %f %f %f %f\n", mins[0], mins[1], mins[2], maxs[0], maxs[1], maxs[2] );
}

/*
   MiniMapSetupBrushes()
   determines solid non-sky brushes in the world
 */
#define C_SOLID                 0x00000001
#define C_SKY                   0x00000800

void SetupBrushesFlags( int mask_any, int test_any, int mask_all, int test_all ){
	//char		strippedName[MAX_QPATH];
	int i, j, b;
	int compileFlags, allCompileFlags;
	dbrush_t      *brush;
	dbrushside_t  *side;
	dshader_t     *shader;
	//const char	*shaderText;

	/* note it */
	Com_DPrintf( "--- SetupBrushes ---\n" );

	/* allocate */
	if ( opaqueBrushes == NULL ) {
		opaqueBrushes = Z_Malloc( minimap.numBrushes / 8 + 1 );
	}

	/* clear */
	memset( opaqueBrushes, 0, minimap.numBrushes / 8 + 1 );
	numOpaqueBrushes = 0;

	/* walk the list of worldspawn brushes */
	for ( i = 0; i < minimap.bmodels[ 0 ].numBrushes; i++ )
	{
		/* get brush */
		b = minimap.bmodels[ 0 ].firstBrush + i;
		brush = &minimap.brushes[ b ];
		shader = &minimap.shaders[ brush->shaderNum ];
		//printf("%i, %i, %i - %s\n", brush->shaderNum, shader->contentFlags, shader->surfaceFlags, shader->shader);

		/* check all sides */
		compileFlags = 0;
		compileFlags |= (shader->contentFlags & CONTENTS_SOLID) ? C_SOLID : 0;
		compileFlags |= (shader->surfaceFlags & SURF_SKY) ? C_SKY : 0;
		allCompileFlags = ~( 0 );
		allCompileFlags &= compileFlags;
		for ( j = 0; j < brush->numSides; j++ )
		{
			/* do bsp shader calculations */
			side = &minimap.brushsides[ brush->firstSide + j ];
			shader = &minimap.shaders[ side->shaderNum ];

			/* get shader info */
			//COM_StripExtension(shader->shader, strippedName, sizeof(strippedName));
			//shaderText = FindShaderInShaderText( strippedName );
			//if ( !shaderText ) {
			//	continue;
			//}
			//ParseShader( &shaderText, &compileFlags );

			/* or together compile flags */
			//allCompileFlags &= compileFlags;
			compileFlags |= (shader->contentFlags & CONTENTS_SOLID) ? C_SOLID : 0;
			compileFlags |= (shader->surfaceFlags & SURF_SKY) ? C_SKY : 0;
			allCompileFlags &= compileFlags;
			//printf("%i, %i, %i - %s\n", side->shaderNum, shader->contentFlags, shader->surfaceFlags, shader->shader);
		}

		/* determine if this brush is opaque to light */
		if ( ( compileFlags & mask_any ) == test_any && ( allCompileFlags & mask_all ) == test_all ) {
			opaqueBrushes[ b >> 3 ] |= ( 1 << ( b & 7 ) );
			numOpaqueBrushes++;
			//maxOpaqueBrush = i;
		}
	}

	/* emit some statistics */
	Com_DPrintf( "%9d opaque brushes\n", numOpaqueBrushes );
}

void MiniMapSetupBrushes( void ){
	SetupBrushesFlags( C_SOLID | C_SKY, C_SOLID, 0, 0 );
	// at least one must be solid
	// none may be sky
	// not all may be nodraw
}

qboolean MiniMapEvaluateSampleOffsets( int *bestj, int *bestk, float *bestval ){
	float val, dx, dy;
	int j, k;

	*bestj = *bestk = -1;
	*bestval = 3; /* max possible val is 2 */

	for ( j = 0; j < minimap.samples; ++j )
		for ( k = j + 1; k < minimap.samples; ++k )
		{
			dx = minimap.sample_offsets[2 * j + 0] - minimap.sample_offsets[2 * k + 0];
			dy = minimap.sample_offsets[2 * j + 1] - minimap.sample_offsets[2 * k + 1];
			if ( dx > +0.5 ) {
				dx -= 1;
			}
			if ( dx < -0.5 ) {
				dx += 1;
			}
			if ( dy > +0.5 ) {
				dy -= 1;
			}
			if ( dy < -0.5 ) {
				dy += 1;
			}
			val = dx * dx + dy * dy;
			if ( val < *bestval ) {
				*bestj = j;
				*bestk = k;
				*bestval = val;
			}
		}

	return *bestval < 3;
}

void MiniMapMakeSampleOffsets( void )
{
	int i, j, k, jj, kk;
	float val, valj, valk, sx, sy, rx, ry;

	Com_Printf( "Generating good sample offsets (this may take a while)...\n" );

	/* start with entirely random samples */
	for ( i = 0; i < minimap.samples; ++i )
	{
		minimap.sample_offsets[2 * i + 0] = random();
		minimap.sample_offsets[2 * i + 1] = random();
	}

	for ( i = 0; i < 1000; ++i )
	{
		if ( MiniMapEvaluateSampleOffsets( &j, &k, &val ) ) {
			sx = minimap.sample_offsets[2 * j + 0];
			sy = minimap.sample_offsets[2 * j + 1];
			minimap.sample_offsets[2 * j + 0] = rx = random();
			minimap.sample_offsets[2 * j + 1] = ry = random();
			if ( !MiniMapEvaluateSampleOffsets( &jj, &kk, &valj ) ) {
				valj = -1;
			}
			minimap.sample_offsets[2 * j + 0] = sx;
			minimap.sample_offsets[2 * j + 1] = sy;

			sx = minimap.sample_offsets[2 * k + 0];
			sy = minimap.sample_offsets[2 * k + 1];
			minimap.sample_offsets[2 * k + 0] = rx;
			minimap.sample_offsets[2 * k + 1] = ry;
			if ( !MiniMapEvaluateSampleOffsets( &jj, &kk, &valk ) ) {
				valk = -1;
			}
			minimap.sample_offsets[2 * k + 0] = sx;
			minimap.sample_offsets[2 * k + 1] = sy;

			if ( valj > valk ) {
				if ( valj > val ) {
					/* valj is the greatest */
					minimap.sample_offsets[2 * j + 0] = rx;
					minimap.sample_offsets[2 * j + 1] = ry;
					i = -1;
				}
				else
				{
					/* valj is the greater and it is useless - forget it */
				}
			}
			else
			{
				if ( valk > val ) {
					/* valk is the greatest */
					minimap.sample_offsets[2 * k + 0] = rx;
					minimap.sample_offsets[2 * k + 1] = ry;
					i = -1;
				}
				else
				{
					/* valk is the greater and it is useless - forget it */
				}
			}
		}
		else{
			break;
		}
	}
}

void SV_MakeMinimap(void) {
	char minimapFilename[MAX_QPATH];
	float *data1f;
	byte *data4b, *p;
	float *q;
	int sampleSize = 128;
	trace_t		trace;
	vec3_t scale, mins, maxs;
	cmodel_t *map = CM_ClipHandleToModel(0);
	vec3_t start, end, forward, right, up, size, midpoint, dist, median;

	for(int i = 0; i < cm.numSubModels; i++) {
		VectorAdd(mins, cm.cmodels[i].mins, mins);
		VectorAdd(maxs, cm.cmodels[i].maxs, maxs);
	}
	VectorScale(mins, 1.0f/cm.numSubModels, mins);
	VectorScale(maxs, 1.0f/cm.numSubModels, maxs);
	VectorSubtract(map->maxs, map->mins, size);
	VectorSubtract(maxs, mins, median);
	VectorScale(median, 0.5, midpoint);
	VectorSubtract(maxs, midpoint, midpoint);
	AngleVectors( (vec3_t){89, 0, 0}, forward, right, up );
	float height = MAX(size[0], size[1]);
	midpoint[2] = map->maxs[2] + height;

	data1f = (float *)Z_Malloc( sampleSize * sampleSize * sizeof( float ) );
	memset(data1f, 0, sampleSize * sampleSize * sizeof( float ));
	data4b = (byte *)Z_Malloc( sampleSize * sampleSize * 4 );
	memset(data4b, 0, sampleSize * sampleSize * 4);

	for(int i = 0; i < 3; i++) {
		scale[i] = size[i] / sampleSize;
	}

	q = data1f;
	for ( int y = 0; y < sampleSize; ++y ) {
		for ( int x = 0; x < sampleSize; ++x ) {
			vec3_t newStart = {map->mins[0] + scale[0] * x, map->mins[1] + scale[1] * y, midpoint[2]};
			VectorMA( newStart, fabsf(size[2]) * 4, forward, end );
			VectorCopy(midpoint, start);
			while (1) {
				CM_BoxTrace( &trace, start, end, vec3_origin, vec3_origin, 0, MASK_PLAYERSOLID, qfalse );
				//trap_Trace (&trace, tracefrom, NULL, NULL, end, passent, MASK_SHOT );
				//if (trace.surfaceFlags & SURF_NOIMPACT)
				//	break;

				if (trace.surfaceFlags & SURF_SKY)
					break;

				// Hypo: break if we traversed length of vector tracefrom
				if (trace.fraction == 1.0)
					break;

				VectorSubtract(start, trace.endpos, dist);
				*q += fabsf(midpoint[2] - trace.endpos[2]) - height; //VectorLength(dist);
				//printf("working? x:%i, y:%i, %f\n", x, y, *q);
				
				// otherwise continue tracing thru walls
				VectorMA (trace.endpos,1,forward,start);
			}
			q++;
		}
	}

	q = data1f;
	p = data4b;
	for ( int y = 0; y < sampleSize; ++y ) {
		for ( int x = 0; x < sampleSize; ++x ) {
			printf("working: %f\n", *q);
			*p = Com_Clamp( 0.f, 255.f / 256.f, *q / size[2] / 5 ) * 256;
			q++;
			p++;
		}
	}
	COM_StripExtension(cm.name, minimapFilename, sizeof(minimapFilename));
	Q_strcat(minimapFilename, sizeof(minimapFilename), ".tga");
	char *mapPath = FS_BuildOSPath( Cvar_VariableString("fs_homepath"), Cvar_VariableString("fs_game"), minimapFilename );
	Com_Printf( " writing to %s...", mapPath );
	WriteTGAGray( mapPath, data4b, sampleSize, sampleSize );

	Z_Free(data4b);
}


void MiniMapBSPMain( const char *source ){
	char minimapFilename[MAX_QPATH];
	void *buf;
	qboolean autolevel;
	float minimapSharpen;
	float border;
	byte *data4b, *p;
	float *q;
	int x, y;
	EMiniMapMode mode = EMMM_Color;
	qboolean keepaspect;
	dheader_t		header;
	vec3_t mins, maxs;

	/* load the BSP first */
	Com_Printf( "Loading %s\n", source );
	//BeginMapShaderFile( source ); //do not delete q3map2_*.shader on minimap generation
	//LoadShaderInfo();
	FS_ReadFile( source, &buf );
	if(!buf) {
		Com_Error(ERR_DROP, "Couldn't load %s", source);
		return;
	}
	header = *(dheader_t *)buf;
	if(header.version != BSP3_VERSION) {
		Com_Error(ERR_DROP, "Couldn't load %s", source);
		return;
	}

	minimap.brushes = (void *)(buf + header.lumps[LUMP_BRUSHES].fileofs);
	minimap.numBrushes = header.lumps[LUMP_BRUSHES].filelen / sizeof(dbrush_t);
	minimap.brushsides = (void *)(buf + header.lumps[LUMP_BRUSHSIDES].fileofs);
	minimap.numBrushSides = header.lumps[LUMP_BRUSHSIDES].filelen / sizeof(dbrushside_t);
	minimap.planes = (void *)(buf + header.lumps[LUMP_PLANES].fileofs);
	minimap.numPlanes = header.lumps[LUMP_PLANES].filelen / sizeof(dplane_t);
	minimap.bmodels = (void *)(buf + header.lumps[LUMP_MODELS].fileofs);
	minimap.numBModels = header.lumps[LUMP_MODELS].filelen / sizeof(dmodel_t);
	minimap.shaders = (void *)(buf + header.lumps[LUMP_SHADERS].fileofs);
	minimap.numShaders = header.lumps[LUMP_SHADERS].filelen / sizeof(dshader_t);

	VectorCopy(minimap.bmodels[0].mins, mins);
	VectorCopy(minimap.bmodels[0].maxs, maxs);

	minimapSharpen = 1.0f;
	minimap.width = minimap.height = 512;
	border = 0.0f;
	keepaspect = qtrue;
	autolevel = qfalse;
	minimap.samples = 1;
	minimap.sample_offsets = NULL;
	minimap.boost = 1.0;
	minimap.brightness = 0.0;
	minimap.contrast = 1.0;

	/* process arguments */
#if 0
	for ( i = 1; i < ( argc - 1 ); i++ )
	{
		if ( striEqual( argv[ i ],  "-size" ) ) {
			minimap.width = minimap.height = atoi( argv[i + 1] );
			i++;
			Com_Printf( "Image size set to %i\n", minimap.width );
		}
		else if ( striEqual( argv[ i ],  "-sharpen" ) ) {
			minimapSharpen = atof( argv[i + 1] );
			i++;
			Com_Printf( "Sharpening coefficient set to %f\n", minimapSharpen );
		}
		else if ( striEqual( argv[ i ],  "-samples" ) ) {
			minimap.samples = atoi( argv[i + 1] );
			i++;
			Com_Printf( "Samples set to %i\n", minimap.samples );
			Z_Free( minimap.sample_offsets );
			minimap.sample_offsets = Z_Malloc( 2 * sizeof( *minimap.sample_offsets ) * minimap.samples );
			MiniMapMakeSampleOffsets();
		}
		else if ( striEqual( argv[ i ],  "-random" ) ) {
			minimap.samples = atoi( argv[i + 1] );
			i++;
			Com_Printf( "Random samples set to %i\n", minimap.samples );
			Z_Free( minimap.sample_offsets );
			minimap.sample_offsets = NULL;
		}
		else if ( striEqual( argv[ i ],  "-border" ) ) {
			border = atof( argv[i + 1] );
			i++;
			Com_Printf( "Border set to %f\n", border );
		}
		else if ( striEqual( argv[ i ],  "-keepaspect" ) ) {
			keepaspect = true;
			Com_Printf( "Keeping aspect ratio by letterboxing\n", border );
		}
		else if ( striEqual( argv[ i ],  "-nokeepaspect" ) ) {
			keepaspect = false;
			Com_Printf( "Not keeping aspect ratio\n", border );
		}
		else if ( striEqual( argv[ i ],  "-o" ) ) {
			strcpy( minimapFilename, argv[i + 1] );
			i++;
			Com_Printf( "Output file name set to %s\n", minimapFilename );
		}
		else if ( striEqual( argv[ i ],  "-minmax" ) && i < ( argc - 7 ) ) {
			mins[0] = atof( argv[i + 1] );
			mins[1] = atof( argv[i + 2] );
			mins[2] = atof( argv[i + 3] );
			maxs[0] = atof( argv[i + 4] );
			maxs[1] = atof( argv[i + 5] );
			maxs[2] = atof( argv[i + 6] );
			i += 6;
			Com_Printf( "Map mins/maxs overridden\n" );
		}
		else if ( striEqual( argv[ i ],  "-gray" ) ) {
			mode = EMMM_Gray;
			Com_Printf( "Writing as white-on-black image\n" );
		}
		else if ( striEqual( argv[ i ],  "-black" ) ) {
			mode = EMMM_Black;
			Com_Printf( "Writing as black alpha image\n" );
		}
		else if ( striEqual( argv[ i ],  "-white" ) ) {
			mode = EMMM_White;
			Com_Printf( "Writing as white alpha image\n" );
		}
		else if ( striEqual( argv[ i ],  "-boost" ) && i < ( argc - 2 ) ) {
			minimap.boost = atof( argv[i + 1] );
			i++;
			Com_Printf( "Contrast boost set to %f\n", minimap.boost );
		}
		else if ( striEqual( argv[ i ],  "-brightness" ) && i < ( argc - 2 ) ) {
			minimap.brightness = atof( argv[i + 1] );
			i++;
			Com_Printf( "Brightness set to %f\n", minimap.brightness );
		}
		else if ( striEqual( argv[ i ],  "-contrast" ) && i < ( argc - 2 ) ) {
			minimap.contrast = atof( argv[i + 1] );
			i++;
			Com_Printf( "Contrast set to %f\n", minimap.contrast );
		}
		else if ( striEqual( argv[ i ],  "-autolevel" ) ) {
			autolevel = true;
			Com_Printf( "Auto level enabled\n", border );
		}
		else if ( striEqual( argv[ i ],  "-noautolevel" ) ) {
			autolevel = false;
			Com_Printf( "Auto level disabled\n", border );
		}
	}
#endif


	MiniMapMakeMinsMaxs( mins, maxs, border, keepaspect );

	char *mapPath = FS_BuildOSPath( Cvar_VariableString("fs_homepath"), Cvar_VariableString("fs_game"), source );
	COM_StripExtension(mapPath, minimapFilename, sizeof(minimapFilename));
	Q_strcat(minimapFilename, sizeof(minimapFilename), ".tga");
	Com_Printf( "Output file name automatically set to %s\n", minimapFilename );

	if ( minimapSharpen >= 0 ) {
		minimap.sharpen_centermult = 8 * minimapSharpen + 1;
		minimap.sharpen_boxmult    =    -minimapSharpen;
	}

	minimap.data1f = (float *)Z_Malloc( minimap.width * minimap.height * sizeof( *minimap.data1f ) );
	if ( minimapSharpen >= 0 ) {
		minimap.sharpendata1f = (float *)Z_Malloc( minimap.width * minimap.height * sizeof( *minimap.data1f ) );
	}
	data4b = (byte *)Z_Malloc( minimap.width * minimap.height * 4 );

	MiniMapSetupBrushes();

	if ( minimap.samples <= 1 ) {
		Com_Printf( "\n--- MiniMapNoSupersampling (%d) ---\n", minimap.height );
		for(int y = 0; y < minimap.height; y++) {
			MiniMapNoSupersampling(y);
		}
	}
	else
	{
		if ( minimap.sample_offsets ) {
			Com_Printf( "\n--- MiniMapSupersampled (%d) ---\n", minimap.height );
			for(int y = 0; y < minimap.height; y++) {
				MiniMapSupersampled(y);
			}
		}
		else
		{
			Com_Printf( "\n--- MiniMapRandomlySupersampled (%d) ---\n", minimap.height );
			for(int y = 0; y < minimap.height; y++) {
				MiniMapRandomlySupersampled(y);
			}
		}
	}

	if ( minimap.boost != 1.0 ) {
		Com_Printf( "\n--- MiniMapContrastBoost (%d) ---\n", minimap.height );
		for(int y = 0; y < minimap.height; y++) {
			MiniMapContrastBoost(y);
		}
	}

	if ( autolevel ) {
		Com_Printf( "\n--- MiniMapAutoLevel (%d) ---\n", minimap.height );
		float mi = 1, ma = 0;
		float s, o;

		// TODO threads!
		q = minimap.data1f;
		for ( y = 0; y < minimap.height; ++y )
			for ( x = 0; x < minimap.width; ++x )
			{
				float v = *q++;
				mi = MIN(mi, v);
				ma = MAX(ma, v);
			}
		if ( ma > mi ) {
			s = 1 / ( ma - mi );
			o = mi / ( ma - mi );

			// equations:
			//   brightness + contrast * v
			// after autolevel:
			//   brightness + contrast * (v * s - o)
			// =
			//   (brightness - contrast * o) + (contrast * s) * v
			minimap.brightness = minimap.brightness - minimap.contrast * o;
			minimap.contrast *= s;

			Com_Printf( "Auto level: Brightness changed to %f\n", minimap.brightness );
			Com_Printf( "Auto level: Contrast changed to %f\n", minimap.contrast );
		}
		else{
			Com_Printf( "Auto level: failed because all pixels are the same value\n" );
		}
	}


	if ( minimap.brightness != 0 || minimap.contrast != 1 ) {
		Com_Printf( "\n--- MiniMapBrightnessContrast (%d) ---\n", minimap.height );
		for(int y = 0; y < minimap.height; y++) {
			MiniMapBrightnessContrast(y);
		}
	}

	if ( minimap.sharpendata1f ) {
		Com_Printf( "\n--- MiniMapSharpen (%d) ---\n", minimap.height );
		for(int y = 0; y < minimap.height; y++) {
			MiniMapSharpen(y);
		}
		q = minimap.sharpendata1f;
	} else
	{
		q = minimap.data1f;
	}

	Com_Printf( "\nConverting...\n" );

	switch ( mode )
	{
	default:
	case EMMM_Gray:
		p = data4b;
		for ( y = 0; y < minimap.height; ++y )
			for ( x = 0; x < minimap.width; ++x )
			{
				*p = Com_Clamp( 0.f, 255.f / 256.f, *q++ ) * 256;
				p++;
			}
		Com_Printf( " writing to %s...", minimapFilename );
		WriteTGAGray( minimapFilename, data4b, minimap.width, minimap.height );
		break;
	case EMMM_Black:
		p = data4b;
		for ( y = 0; y < minimap.height; ++y )
			for ( x = 0; x < minimap.width; ++x )
			{
				*p++ = 0;
				*p++ = 0;
				*p++ = 0;
				*p++ = Com_Clamp( 0.f, 255.f / 256.f, *q++ ) * 256;
			}
		Com_Printf( " writing to %s...", minimapFilename );
		WriteTGA( minimapFilename, data4b, minimap.width, minimap.height );
		break;
	case EMMM_White:
		p = data4b;
		for ( y = 0; y < minimap.height; ++y )
			for ( x = 0; x < minimap.width; ++x )
			{
				*p++ = 255;
				*p++ = 255;
				*p++ = 255;
				*p++ = Com_Clamp( 0.f, 255.f / 256.f, *q++ ) * 256;
			}
		Com_Printf( " writing to %s...", minimapFilename );
		WriteTGA( minimapFilename, data4b, minimap.width, minimap.height );
		break;
	}

	Com_Printf( " done.\n" );

	Z_Free(minimap.data1f);
	if(minimap.sharpendata1f)
		Z_Free(minimap.sharpendata1f);
	if(opaqueBrushes)
		Z_Free(opaqueBrushes);
	Z_Free(data4b);

	/* return to sender */
	return;
}
#endif

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
		buffer[i] = data[i - 18 + 2];       // blue
		buffer[i + 1] = data[i - 18 + 1];     // green
		buffer[i + 2] = data[i - 18 + 0];     // red
		buffer[i + 3] = data[i - 18 + 3];     // alpha
	}

	f = fopen( filename, "wb" );
	fwrite( buffer, 1, c, f );
	fclose( f );

	Z_Free( buffer );
}


static void SV_TraceArea(vec3_t angle, vec3_t scale, float *data1f) {
	trace_t		trace;
	vec3_t start, end;
	//vec3_t dist;

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
			VectorMA( newStart, 8192, angle, end );
			VectorMA (newStart,16,angle,start);
			while (1) {
				CM_BoxTrace( &trace, start, end, vec3_origin, vec3_origin, 0, MASK_PLAYERSOLID, qfalse );
				//trap_Trace (&trace, tracefrom, NULL, NULL, end, passent, MASK_SHOT );

				// Hypo: break if we traversed length of vector tracefrom
				if (trace.fraction == 1.0 || trace.entityNum == ENTITYNUM_NONE) {
					data1f[y * sv_bspMiniSize->integer + x] = MAX_MAP_BOUNDS; //VectorLength(dist);
					break;
				}

				// otherwise continue tracing thru walls
				if ((trace.surfaceFlags & SURF_NODRAW)
					|| (trace.surfaceFlags & SURF_NONSOLID)
					|| (trace.surfaceFlags & SURF_SKY)) {
					VectorMA (trace.endpos,16,angle,start);
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
				//if(cm.brushes[i].sides[j].plane->normal[2]) {
				int startX = (cm.brushes[i].bounds[0][0] - cm.cmodels[0].mins[0]) / scale[0];
				int startY = (cm.brushes[i].bounds[0][1] - cm.cmodels[0].mins[1]) / scale[1];
				int endX = (cm.brushes[i].bounds[1][0] - cm.cmodels[0].mins[0]) / scale[0];
				int endY = (cm.brushes[i].bounds[1][1] - cm.cmodels[0].mins[1]) / scale[1];
				for ( int y = startY; y < endY; ++y ) {
					for ( int x = startX; x < endX; ++x ) {
						// mark the spot as being covered by the surface requested
						data1f[y * sv_bspMiniSize->integer + x] = cm.brushes[i].bounds[0][2]; //VectorLength(dist);
					}
				}
			}
		}
	}

}


void SV_MakeMinimap() {
	char minimapFilename[MAX_QPATH];
	float *q;
	float *data1f;
	byte *data4b, *p;
	vec3_t scale, size, angle, right, up;

	VectorSubtract(cm.cmodels[0].maxs, cm.cmodels[0].mins, size);
	for(int i = 0; i < 3; i++) {
		// could use height from above to add a bunch of stupid black space around it
		//   but I like the extra dexterity - Brian Cullinan
		scale[i] = size[i] / sv_bspMiniSize->value;
	}

	data1f = (float *)Z_Malloc( sv_bspMiniSize->integer * sv_bspMiniSize->integer * sizeof( float ) );
	memset(data1f, (float)MAX_MAP_BOUNDS, sv_bspMiniSize->integer * sv_bspMiniSize->integer * sizeof( float ));
	data4b = (byte *)Z_Malloc( sv_bspMiniSize->integer * sv_bspMiniSize->integer * 4 );
	memset(data4b, 0, sv_bspMiniSize->integer * sv_bspMiniSize->integer * 4);

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
			byte val = Com_Clamp( 0.f, 127.f / 128.f, *q / cm.cmodels[0].maxs[2] ) * 128;
			// sky, probably dealing with high numbers (i.e. close to top of map), 
			//   so subtract it from 128 to get very low numbers, this will have a less 
			//   boring effect on the redness of the image
			// this has a neat side effect of showing full bright where there is no rain,
			//   i.e. start AND stop in the sky, don't come down at all
			*p++ = *q == MAX_MAP_BOUNDS ? 0 : (128 - val + 1);
			p++;
			p++;
			*p++ = 255;
			q++;
		}
	}

	// subtract sky
	AngleVectors( (vec3_t){89, 0, 0}, angle, right, up );
	SV_TraceArea(angle, scale, data1f);

	// store every facet in half (aka 128 grid, vertical in case of rain/ground)
	//   so that a trace from the bottom can be -128, and trace from top can be 128.
	// Or up to 6 traces can be summarized for things like caves, if we wanted
	//   automatically simulate a cave in or rocks falling during Ground Zero opening maps

	q = data1f;
	p = data4b;
	for ( int y = 0; y < sv_bspMiniSize->integer; ++y ) {
		for ( int x = 0; x < sv_bspMiniSize->integer; ++x ) {
			byte val = Com_Clamp( 0.f, 127.f / 128.f, *q / cm.cmodels[0].maxs[2] ) * 128;
			// subtracting from 128 here is descriptive of the distance that the rain
			//   falls in the image, but in the RGB values we can 2 offsets and distance,
			//   i.e. the brighter the red the further the rain falls
			if(*q == MAX_MAP_BOUNDS) {
				*p++ = 0;
				//p++;
			} else {
				*p++ |= val << 4;
			}
			p++;
			p++;
			*p++ = 255;
			q++;
		}
	}

	// subtract the area that passes through a ceiling starting from the floor we just calculated
	AngleVectors( (vec3_t){-89, 0, 0}, angle, right, up );
	SV_TraceArea(scale, angle, data1f);

	q = data1f;
	p = data4b;
	for ( int y = 0; y < sv_bspMiniSize->integer; ++y ) {
		for ( int x = 0; x < sv_bspMiniSize->integer; ++x ) {
			if(*q != MAX_MAP_BOUNDS) {
				//*p++ = 0;
				p++;
			} else {
				p++;
			}
			p++;
			p++;
			*p++ = 255;
			q++;
		}
	}

	COM_StripExtension(cm.name, minimapFilename, sizeof(minimapFilename));
	Q_strcat(minimapFilename, sizeof(minimapFilename), ".tga");
	char *mapPath = FS_BuildOSPath( Cvar_VariableString("fs_homepath"), Cvar_VariableString("fs_game"), minimapFilename );
	Com_Printf( " writing to %s...", mapPath );
	WriteTGA( mapPath, data4b, sv_bspMiniSize->integer, sv_bspMiniSize->integer );

	Com_Printf( " done.\n" );

	Z_Free(data4b);
}

#endif
