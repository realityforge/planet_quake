

typedef enum {
  LUMP_Q1_ENTITIES = 0,
  LUMP_Q1_PLANES,
  LUMP_Q1_TEXTURES,
  LUMP_Q1_VERTEXES,
  LUMP_Q1_VISIBILITY,
  LUMP_Q1_NODES,
  LUMP_Q1_TEXINFO,
  LUMP_Q1_FACES,
  LUMP_Q1_LIGHTING,
  LUMP_Q1_CLIPNODES,
  LUMP_Q1_LEAFS,
  LUMP_Q1_MARKSURFACES,
  LUMP_Q1_EDGES,
  LUMP_Q1_SURFEDGES,
  LUMP_Q1_MODELS,

	LUMP_Q1_COUNT					// should be last	
} dBsp1Lump_t;

typedef struct dBsp1Hdr_s
{
	unsigned version;				// BSP2_VERSION
	lump_t	lumps[LUMP_Q1_COUNT];
} dBsp1Hdr_t;

#define	CONTENTS_Q1_EMPTY		-1
#define	CONTENTS_Q1_SOLID		-2
#define	CONTENTS_Q1_WATER		-3
#define	CONTENTS_Q1_SLIME		-4
#define	CONTENTS_Q1_LAVA		-5
#define	CONTENTS_Q1_SKY		  -6

typedef struct
{
	int         nummiptex;
	int         dataofs[4];                  // [nummiptex]
} dBsp1Texinfo_t;

#define	MIPLEVELS	4
typedef struct miptex_s
{
	char        name[16];
	unsigned    width, height;
	unsigned    offsets[MIPLEVELS];		// four mip maps stored
} miptex_t;

typedef struct texinfo_s
{
	float		vecs[2][4];         // [s/t][xyz offset]
	int			miptex;
	int			flags;
} texinfo_t;

typedef struct
{
	float       normal[3];
	float       dist;
	int         type;             // PLANE_X - PLANE_ANYZ ?remove? trivial to regenerate
} dBsp1Plane_t;

typedef struct {
	unsigned short	v[2];           // vertex numbers
} dedge_t;

#define	MAXLIGHTMAPS	4
typedef struct dBsp1Face_s
{
	unsigned short planenum;
	short	side;

	int		firstedge;				// we must support > 64k edges
	short	numedges;
	short	texinfo;

	// lighting info
	byte	styles[MAXLIGHTMAPS];
	int		lightofs;				// start of [numstyles*surfsize] samples
} dBsp1Face_t;

typedef struct
{
	int             planenum;
	short           children[2];	// negative numbers are -(leafs+1), not nodes
	short           mins[3];        // for sphere culling
	short           maxs[3];
	unsigned short  firstface;
	unsigned short  numfaces;       // counting both sides
} dBsp1Node_t;

#define	NUM_AMBIENTS    4       // automatic ambient sounds
typedef struct {
	int            contents;
	int            visofs;          // -1 = no visibility info

	short          mins[3];         // for frustum culling
	short          maxs[3];

	unsigned short firstmarksurface;
	unsigned short nummarksurfaces;

	byte           ambient_level[NUM_AMBIENTS];
} dBsp1Leaf_t;

#define	MAX_MAP_HULLS		4
typedef struct
{
	float       mins[3], maxs[3];
	float       origin[3];
	int	        headnode[MAX_MAP_HULLS];
	int         visleafs;                    // not including the solid leaf 0
	int         firstface, numfaces;
} dBsp1Model_t;
