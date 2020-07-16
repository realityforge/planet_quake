#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"

#define DEFAULT_CIN_WIDTH	512
#define DEFAULT_CIN_HEIGHT	512

typedef enum
{
	FT_ROQ = 0,				// normal roq (vq3 stuff)
	FT_OGM					// ogm(ogg wrapper, vorbis audio, xvid/theora video) for WoP
} filetype_t;

typedef struct {
	byte				linbuf[DEFAULT_CIN_WIDTH*DEFAULT_CIN_HEIGHT*4*2];
	byte				file[65536];
	short				sqrTable[256];

	int		mcomp[256];
	byte				*qStatus[2][32768];

	long				oldXOff, oldYOff;
	unsigned long		oldysize, oldxsize;

	int					currentHandle;
} cinematics_t;

typedef struct {
	char				fileName[MAX_OSPATH];
	int					CIN_WIDTH, CIN_HEIGHT;
	int					xpos, ypos, width, height;
	qboolean			looping, holdAtEnd, dirty, alterGameState, silent, shader;
	fileHandle_t		iFile;
	e_status			status;
	unsigned int		startTime;
	unsigned int		lastTime;
	long				tfps;
	long				RoQPlayed;
	long				ROQSize;
	unsigned int		RoQFrameSize;
	long				onQuad;
	long				numQuads;
	long				samplesPerLine;
	unsigned int		roq_id;
	long				screenDelta;

	void ( *VQ0)(byte *status, void *qdata );
	void ( *VQ1)(byte *status, void *qdata );
	void ( *VQNormal)(byte *status, void *qdata );
	void ( *VQBuffer)(byte *status, void *qdata );

	long				samplesPerPixel;				// defaults to 2
	byte*				gray;
	unsigned int		xsize, ysize, maxsize, minsize;

	qboolean			half, smootheddouble;
	long				inMemory;
	long				normalBuffer0;
	long				roq_flags;
	long				roqF0;
	long				roqF1;
	long				t[2];
	long				roqFPS;
	int					playonwalls;
	byte*				buf;
	long				drawX, drawY;
  filetype_t			fileType;
} cin_cache;

void CIN_Shutdown( void );
void CIN_SetLooping (int handle, qboolean loop);
