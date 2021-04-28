
#ifndef __BSP_H__
#define __BSP_H__

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"
#include "q3map2.h"

/* -------------------------------------------------------------------------------

   bsp/general global variables

   ------------------------------------------------------------------------------- */
 char            *surfaceTypes[ NUM_SURFACE_TYPES ]
 	=
 	{
 	"SURFACE_BAD",
 	"SURFACE_FACE",
 	"SURFACE_PATCH",
 	"SURFACE_TRIANGLES",
 	"SURFACE_FLARE",
 	"SURFACE_FOLIAGE",
 	"SURFACE_FORCED_META",
 	"SURFACE_META",
 	"SURFACE_FOGHULL",
 	"SURFACE_DECAL",
 	"SURFACE_SHADER"
 	};

/* game support */
game_t games[]
	=
	{
								#include "game_quake3.h"
	,
								#include "game_quakelive.h" /* most be after game_quake3.h as they share defines! */
	,
								#include "game_nexuiz.h" /* most be after game_quake3.h as they share defines! */
	,
								#include "game_tremulous.h" /*LinuxManMikeC: must be after game_quake3.h, depends on #define's set in it */
	,
								#include "game_unvanquished.h" /* must be after game_tremulous.h as they share defines! */
	,
								#include "game_tenebrae.h"
	,
								#include "game_wolf.h"
	,
								#include "game_wolfet.h" /* most be after game_wolf.h as they share defines! */
	,
								#include "game_etut.h"
	,
								#include "game_ef.h"
	,
								#include "game_sof2.h"
	,
								#include "game_jk2.h"   /* most be after game_sof2.h as they share defines! */
	,
								#include "game_ja.h"    /* most be after game_jk2.h as they share defines! */
	,
								#include "game_qfusion.h"   /* qfusion game */
	,
								#include "game_reaction.h" /* must be after game_quake3.h */
	,
								#include "game__null.h" /* null game (must be last item) */
	};
game_t             *game = &games[ 0 ];


/* general */
int numImages = 0;
image_t images[ MAX_IMAGES ];

int numPicoModels = 0;
picoModel_t        *picoModels[ MAX_MODELS ];

shaderInfo_t       *shaderInfo = NULL;
int numShaderInfo = 0;
int numVertexRemaps = 0;

surfaceParm_t custSurfaceParms[ MAX_CUST_SURFACEPARMS ];
int numCustSurfaceParms = 0;

char mapName[ MAX_QPATH ];                 /* ydnar: per-map custom shaders for larger lightmaps */
char mapShaderFile[ 1024 ];
qboolean warnImage = qtrue;

/* ydnar: sinusoid samples */
float jitters[ MAX_JITTERS ];


/* commandline arguments */
qboolean verbose;
qboolean verboseEntities = qfalse;
qboolean force = qfalse;
qboolean infoMode = qfalse;
qboolean useCustomInfoParms = qfalse;
qboolean noprune = qfalse;
qboolean leaktest = qfalse;
qboolean nodetail = qfalse;
qboolean nosubdivide = qfalse;
qboolean notjunc = qfalse;
qboolean fulldetail = qfalse;
qboolean nowater = qfalse;
qboolean noCurveBrushes = qfalse;
qboolean fakemap = qfalse;
qboolean coplanar = qfalse;
qboolean nofog = qfalse;
qboolean noHint = qfalse;                        /* ydnar */
qboolean renameModelShaders = qfalse;            /* ydnar */
qboolean skyFixHack = qfalse;                    /* ydnar */
qboolean bspAlternateSplitWeights = qfalse;      /* 27 */
qboolean deepBSP = qfalse;                       /* div0 */

int patchSubdivisions = 8;                       /* ydnar: -patchmeta subdivisions */

int maxLMSurfaceVerts = 64;                      /* ydnar */
int maxSurfaceVerts = 999;                       /* ydnar */
int maxSurfaceIndexes = 6000;                    /* ydnar */
float npDegrees = 0.0f;                          /* ydnar: nonplanar degrees */
int bevelSnap = 0;                               /* ydnar: bevel plane snap */
int texRange = 0;
qboolean flat = qfalse;
qboolean meta = qfalse;
qboolean patchMeta = qfalse;
qboolean emitFlares = qfalse;
qboolean debugSurfaces = qfalse;
qboolean debugInset = qfalse;
qboolean debugPortals = qfalse;

#if Q3MAP2_EXPERIMENTAL_SNAP_NORMAL_FIX
// Increasing the normalEpsilon to compensate for new logic in SnapNormal(), where
// this epsilon is now used to compare against 0 components instead of the 1 or -1
// components.  Unfortunately, normalEpsilon is also used in PlaneEqual().  So changing
// this will affect anything that calls PlaneEqual() as well (which are, at the time
// of this writing, FindFloatPlane() and AddBrushBevels()).
double normalEpsilon = 0.00005;
#else
double normalEpsilon = 0.00001;
#endif

#if Q3MAP2_EXPERIMENTAL_HIGH_PRECISION_MATH_FIXES
// NOTE: This distanceEpsilon is too small if parts of the map are at maximum world
// extents (in the range of plus or minus 2^16).  The smallest epsilon at values
// close to 2^16 is about 0.007, which is greater than distanceEpsilon.  Therefore,
// maps should be constrained to about 2^15, otherwise slightly undesirable effects
// may result.  The 0.01 distanceEpsilon used previously is just too coarse in my
// opinion.  The real fix for this problem is to have 64 bit distances and then make
// this epsilon even smaller, or to constrain world coordinates to plus minus 2^15
// (or even 2^14).
double distanceEpsilon = 0.005;
#else
double distanceEpsilon = 0.01;
#endif


/* bsp */
int numMapEntities = 0;

int blockSize[ 3 ]                                 /* should be the same as in radiant */
= { 1024, 1024, 1024 };

char name[ 1024 ];
char source[ 1024 ];
char outbase[ 32 ];

int sampleSize;                                    /* lightmap sample size in units */

int mapEntityNum = 0;

int entitySourceBrushes;

plane_t mapplanes[ MAX_MAP_PLANES ];               /* mapplanes[ num ^ 1 ] will always be the mirror or mapplanes[ num ] */
int nummapplanes;                                  /* nummapplanes will always be even */
int numMapPatches;
vec3_t mapMins, mapMaxs;

int defaultFogNum = -1;                  /* ydnar: cleaner fog handling */
int numMapFogs = 0;
fog_t mapFogs[ MAX_MAP_FOGS ];

bspEntity_t           *mapEnt;
brush_t            *buildBrush;
int numActiveBrushes;
int g_bBrushPrimit;

int numStrippedLights = 0;


/* surface stuff */
mapDrawSurface_t   *mapDrawSurfs = NULL;
int numMapDrawSurfs;

int numSurfacesByType[ NUM_SURFACE_TYPES ];
int numClearedSurfaces;
int numStripSurfaces;
int numFanSurfaces;
int numMergedSurfaces;
int numMergedVerts;

int numRedundantIndexes;

int numSurfaceModels = 0;

byte debugColors[ 12 ][ 3 ]
	=
	{
	{ 255, 0, 0 },
	{ 192, 128, 128 },
	{ 255, 255, 0 },
	{ 192, 192, 128 },
	{ 0, 255, 255 },
	{ 128, 192, 192 },
	{ 0, 0, 255 },
	{ 128, 128, 192 },
	{ 255, 0, 255 },
	{ 192, 128, 192 },
	{ 0, 255, 0 },
	{ 128, 192, 128 }
	};

qboolean skyboxPresent = qfalse;
int skyboxArea = -1;
m4x4_t skyboxTransform;



/* -------------------------------------------------------------------------------

   vis global variables

   ------------------------------------------------------------------------------- */

/* commandline arguments */
qboolean			fastvis;
qboolean			noPassageVis;
qboolean			passageVisOnly;
qboolean			mergevis;
qboolean			mergevisportals;
qboolean			nosort;
qboolean			saveprt;
qboolean			hint;	/* ydnar */
char				inbase[ MAX_QPATH ];
char				globalCelShader[ MAX_QPATH ];

/* other bits */
int totalvis;

float farPlaneDist;                /* rr2do2, rf, mre, ydnar all contributed to this one... */

int numportals;
int portalclusters;

vportal_t          *portals;
leaf_t             *leafs;

vportal_t          *faces;
leaf_t             *faceleafs;

int numfaces;

int c_portaltest, c_portalpass, c_portalcheck;
int c_portalskip, c_leafskip;
int c_vistest, c_mighttest;
int c_chains;

byte               *vismap, *vismap_p, *vismap_end;

int testlevel;

byte               *uncompressed;

int leafbytes, leaflongs;
int portalbytes, portallongs;

vportal_t          *sorted_portals[ MAX_MAP_PORTALS * 2 ];



/* -------------------------------------------------------------------------------

   light global variables

   ------------------------------------------------------------------------------- */

/* commandline arguments */
qboolean wolfLight = qfalse;
qboolean loMem = qfalse;
qboolean noStyles = qfalse;

int sampleSize = DEFAULT_LIGHTMAP_SAMPLE_SIZE;
qboolean noVertexLighting = qfalse;
qboolean noGridLighting = qfalse;

qboolean noTrace = qfalse;
qboolean noSurfaces = qfalse;
qboolean patchShadows = qfalse;
qboolean cpmaHack = qfalse;

qboolean deluxemap = qfalse;
qboolean debugDeluxemap = qfalse;

qboolean fast = qfalse;
qboolean faster = qfalse;
qboolean fastgrid = qfalse;
qboolean fastbounce = qfalse;
qboolean cheap = qfalse;
qboolean cheapgrid = qfalse;
int bounce = 0;
qboolean bounceOnly = qfalse;
qboolean bouncing = qfalse;
qboolean bouncegrid = qfalse;
qboolean normalmap = qfalse;
qboolean trisoup = qfalse;
qboolean shade = qfalse;
float shadeAngleDegrees = 0.0f;
int superSample = 0;
int lightSamples = 1;
qboolean filter = qfalse;
qboolean dark = qfalse;
qboolean sunOnly = qfalse;
int approximateTolerance = 0;
qboolean noCollapse = qfalse;
qboolean exportLightmaps = qfalse;
qboolean externalLightmaps = qfalse;
int lmCustomSize = LIGHTMAP_WIDTH;

qboolean dirty = qfalse;
qboolean dirtDebug = qfalse;
int dirtMode = 0;
float dirtDepth = 128.0f;
float dirtScale = 1.0f;
float dirtGain = 1.0f;

qboolean debugnormals = qfalse;
qboolean floodlighty = qfalse;
qboolean floodlight_lowquality = qfalse;
vec3_t floodlightRGB;
float floodlightIntensity = 512;
float floodlightDistance = 1024;

qboolean dump = qfalse;
qboolean debug = qfalse;
qboolean debugUnused = qfalse;
qboolean debugAxis = qfalse;
qboolean debugCluster = qfalse;
qboolean debugOrigin = qfalse;
qboolean lightmapBorder = qfalse;

/* longest distance across the map */
float maxMapDistance = 0;

/* for run time tweaking of light sources */
float pointScale = 7500.0f;
float areaScale = 0.25f;
float skyScale = 1.0f;
float bounceScale = 0.25f;

/* ydnar: lightmap gamma/compensation */
float lightmapGamma = 1.0f;
float lightmapExposure = 0.0f;
float lightmapCompensate = 1.0f;

/* ydnar: for runtime tweaking of falloff tolerance */
float falloffTolerance = 1.0f;
qboolean exactPointToPolygon = qtrue;
float formFactorValueScale = 3.0f;
float linearScale = 1.0f / 8000.0f;

light_t            *lights;
int numPointLights;
int numSpotLights;
int numSunLights;
int numAreaLights;

/* ydnar: for luxel placement */
int numSurfaceClusters, maxSurfaceClusters;
int                *surfaceClusters;

/* ydnar: for radiosity */
int numDiffuseLights;
int numBrushDiffuseLights;
int numTriangleDiffuseLights;
int numPatchDiffuseLights;

/* ydnar: general purpose extra copy of drawvert list */
bspDrawVert_t      *yDrawVerts;

/* ydnar: for tracing statistics */
int minSurfacesTested;
int maxSurfacesTested;
int totalSurfacesTested;
int totalTraces;

FILE               *dumpFile;

int c_visible, c_occluded;
int c_subsampled;                  /* ydnar */

int defaultLightSubdivide = 999;

vec3_t ambientColor;
vec3_t minLight, minVertexLight, minGridLight;

int                *entitySurface;
vec3_t             *surfaceOrigin;

vec3_t sunDirection;
vec3_t sunLight;

/* tracing */
int c_totalTrace;
int c_cullTrace, c_testTrace;
int c_testFacets;

/* ydnar: light optimization */
float subdivideThreshold = DEFAULT_SUBDIVIDE_THRESHOLD;

int numOpaqueBrushes, maxOpaqueBrush;
byte               *opaqueBrushes;

int numLights;
int numCulledLights;

int gridBoundsCulled;
int gridEnvelopeCulled;

int lightsBoundsCulled;
int lightsEnvelopeCulled;
int lightsPlaneCulled;
int lightsClusterCulled;

/* ydnar: radiosity */
float diffuseSubdivide = 256.0f;
float minDiffuseSubdivide = 64.0f;
int numDiffuseSurfaces = 0;

/* ydnar: list of surface information necessary for lightmap calculation */
surfaceInfo_t      *surfaceInfos = NULL;

/* ydnar: sorted list of surfaces */
int                *sortSurfaces = NULL;

/* clumps of surfaces that share a raw lightmap */
int numLightSurfaces = 0;
int                *lightSurfaces = NULL;

/* raw lightmaps */
int numRawSuperLuxels = 0;
int numRawLightmaps = 0;
rawLightmap_t      *rawLightmaps = NULL;
int                *sortLightmaps = NULL;

/* vertex luxels */
float              *vertexLuxels[ MAX_LIGHTMAPS ];
float              *radVertexLuxels[ MAX_LIGHTMAPS ];

/* bsp lightmaps */
int numLightmapShaders = 0;
int numSolidLightmaps = 0;
int numOutLightmaps = 0;
int numBSPLightmaps = 0;
int numExtLightmaps = 0;
outLightmap_t      *outLightmaps = NULL;

/* grid points */
int numRawGridPoints = 0;
rawGridPoint_t     *rawGridPoints = NULL;

int numSurfsVertexLit = 0;
int numSurfsVertexForced = 0;
int numSurfsVertexApproximated = 0;
int numSurfsLightmapped = 0;
int numPlanarsLightmapped = 0;
int numNonPlanarsLightmapped = 0;
int numPatchesLightmapped = 0;
int numPlanarPatchesLightmapped = 0;

int numLuxels = 0;
int numLuxelsMapped = 0;
int numLuxelsOccluded = 0;
int numLuxelsIlluminated = 0;
int numVertsIlluminated = 0;

/* lightgrid */
vec3_t gridMins;
int gridBounds[ 3 ];
vec3_t gridSize	= { 64, 64, 128 };



/* -------------------------------------------------------------------------------

   abstracted bsp globals

   ------------------------------------------------------------------------------- */

int numEntities = 0;
int numBSPEntities = 0;
bspEntity_t entities[ MAX_MAP_ENTITIES ];

int numBSPModels = 0;
bspModel_t bspModels[ MAX_MAP_MODELS ];

int numBSPShaders = 0;
bspShader_t bspShaders[ MAX_MAP_MODELS ];

int bspEntDataSize = 0;
char bspEntData[ MAX_MAP_ENTSTRING ];

int numBSPLeafs = 0;
bspLeaf_t bspLeafs[ MAX_MAP_LEAFS ];

int numBSPPlanes = 0;
bspPlane_t bspPlanes[ MAX_MAP_PLANES ];

int numBSPNodes = 0;
bspNode_t bspNodes[ MAX_MAP_NODES ];

int numBSPLeafSurfaces = 0;
int bspLeafSurfaces[ MAX_MAP_LEAFFACES ];

int numBSPLeafBrushes = 0;
int bspLeafBrushes[ MAX_MAP_LEAFBRUSHES ];

int numBSPBrushes = 0;
bspBrush_t bspBrushes[ MAX_MAP_BRUSHES ];

int numBSPBrushSides = 0;
bspBrushSide_t bspBrushSides[ MAX_MAP_BRUSHSIDES ];

int numBSPLightBytes = 0;
byte *bspLightBytes = NULL;

//%	int				numBSPGridPoints = 0;
//%	byte				*bspGridPoints = NULL;

int numBSPGridPoints = 0;
bspGridPoint_t     *bspGridPoints = NULL;

int numBSPVisBytes = 0;
byte bspVisBytes[ MAX_MAP_VISIBILITY ];

int numBSPDrawVerts = 0;
bspDrawVert_t *bspDrawVerts = NULL;

int numBSPDrawIndexes = 0;
int bspDrawIndexes[ MAX_MAP_DRAW_INDEXES ];

int numBSPDrawSurfaces = 0;
bspDrawSurface_t   *bspDrawSurfaces = NULL;

int numBSPFogs = 0;
bspFog_t bspFogs[ MAX_MAP_FOGS ];

int numBSPAds = 0;
bspAdvertisement_t bspAds[ MAX_MAP_ADVERTISEMENTS ];

#endif
