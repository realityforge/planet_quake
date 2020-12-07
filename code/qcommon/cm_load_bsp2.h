
// to allow boxes to be treated as brush models, we allocate
// some extra indexes along with those needed by the map
#define	BOX_BRUSHES		1
#define	BOX_SIDES		6
#define	BOX_LEAFS		2
#define	BOX_PLANES		12
#define HULL_LEAFS			1
#define HULL_BRUSHES		1

#define	LL(x) x=LittleLong(x)

#ifndef min
#  define min(x,y)  ((x)>(y)?(y):(x))
#endif

#ifndef max
#  define max(x,y)  ((x)<(y)?(y):(x))
#endif

typedef enum {
	LUMP_Q2_ENTITIES = 0,				// char[]
	LUMP_Q2_PLANES,				// dBsp2Plane_t[]
	LUMP_Q2_VERTEXES,				// vec3_t[]
	LUMP_Q2_VISIBILITY,			// dBsp2Vis_t[] + byte[]
	LUMP_Q2_NODES,					// dBsp2Node_t[]
	LUMP_Q2_TEXINFO,				// dBsp2Texinfo_t[]
	LUMP_Q2_FACES,					// dBspFace_t[]
	LUMP_Q2_LIGHTING,				// byte[]
	LUMP_Q2_LEAFS,					// dBsp2Leaf_t[]
	LUMP_Q2_LEAFFACES,				// short[]
	LUMP_Q2_LEAFBRUSHES,			// short[]
	LUMP_Q2_EDGES,					// dEdge_t[]
	LUMP_Q2_SURFEDGES,				// int[]
	LUMP_Q2_MODELS,				// dBsp2Model_t[]
	LUMP_Q2_BRUSHES,				// dBsp2Brush_t[]
	LUMP_Q2_BRUSHSIDES,			// dBsp2Brushside_t[]
	LUMP_Q2_POP,					// ?
	LUMP_Q2_ZONES,					// dZone_t[] (original: LUMP_Q2_AREAS)
	LUMP_Q2_ZONEPORTALS,			// dZonePortal_t[] (original: LUMP_Q2_AREAPORTALS)

	LUMP_Q2_COUNT					// should be last	
} dBsp2Lump_t;

typedef struct dBsp2Hdr_s
{
	unsigned ident;					// BSP_IDENT
	unsigned version;				// BSP2_VERSION
	lump_t	lumps[LUMP_Q2_COUNT];
} dBsp2Hdr_t;

typedef struct dBsp2Model_s
{
	vec3_t	mins, maxs;
	vec3_t	origin;					// unused
	int		headnode;
	int		firstface, numfaces;	// submodels just draw faces without walking the bsp tree
} dBsp2Model_t;


// planes (x&~1) and (x&~1)+1 are always opposites
// the same as CPlane, but "byte type,signbits,pad[2]") -> "int type"
typedef struct dBsp2Plane_s
{
	vec3_t	normal;
	float	dist;
	int		type;					// useless - will be recomputed on map loading
} dBsp2Plane_t;


#define MAX_TREE_DEPTH	512

typedef struct dBsp2Node_s
{
	int		planeNum;
	int		children[2];			// negative numbers are -(leafs+1), not nodes
	short	mins[3], maxs[3];		// for frustom culling
	short	firstFace, numFaces;	// unused
} dBsp2Node_t;

typedef struct dBsp2Leaf_s
{
	unsigned contents;				// OR of all brushes (not needed?)

	short	cluster;
	short	zone;

	short	mins[3], maxs[3];		// for frustum culling

	unsigned short firstleafface, numleaffaces;
	unsigned short firstleafbrush, numleafbrushes;
} dBsp2Leaf_t;

typedef struct dBsp2Texinfo_s
{
	struct {						// axis for s/t computation
		vec3_t	vec;
		float	offset;
	} vecs[2];
	unsigned flags;					// miptex flags + overrides
	int		value;					// light emission
	char	texture[32];			// texture name (textures/*.wal)
	int		nexttexinfo;			// for animations, -1 = end of chain
} dBsp2Texinfo_t;


// note that edge 0 is never used, because negative edge nums are used for
// counterclockwise use of the edge in a face
struct dEdge_t
{
	unsigned short v[2];			// vertex numbers
};

#define	MAXLIGHTMAPS	4
typedef struct dBspFace_s
{
	unsigned short planenum;
	short	side;

	int		firstedge;				// we must support > 64k edges
	short	numedges;
	short	texinfo;

	// lighting info
	byte	styles[MAXLIGHTMAPS];
	int		lightofs;				// start of [numstyles*surfsize] samples
} dBspFace_t;

typedef struct dBsp2Brush_s
{
	int		firstside;
	int		numsides;
	unsigned contents;
} dBsp2Brush_t;

typedef struct dBsp2Brushside_s
{
	unsigned short planenum;		// facing out of the leaf
	short	texinfo;				// may be -1 (no texinfo)
} dBsp2Brushside_t;

// each zone has a list of portals that lead into other zones
// when portals are closed, other zones may not be visible or
// hearable even if the vis info says that it should be
struct dZonePortal_t
{
	int		portalNum;
	int		otherZone;
};

struct dZone_t
{
	int		numZonePortals;
	int		firstZonePortal;
};


// the visibility lump consists of a header with a count, then
// byte offsets for the PVS and PHS of each cluster, then the raw
// compressed bit vectors
typedef struct dBsp2Vis_s
{
	enum
	{
		PVS,
		PHS							// unused
	};

	int		numClusters;
	int		bitOfs[8][2];			// bitOfs[numClusters][2]
} dBsp2Vis_t;

/*-----------------------------------------------------------------------------
	Kingpin extensions for Q2 BSP
-----------------------------------------------------------------------------*/
// materials

#define SURF_WATER			0x00080000
#define SURF_CONCRETE		0x00100000
#define SURF_FABRIC			0x00200000
#define SURF_GRAVEL			0x00400000
#define SURF_METAL			0x00800000
#define SURF_METAL_L		0x01000000
#define SURF_SNOW			0x02000000
#define SURF_TILE			0x04000000
#define SURF_WOOD			0x08000000

#define SURF_KP_MATERIAL	0x0FF80000
