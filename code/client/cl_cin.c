
#include "cl_cin.h"
#include "client.h"
#include "snd_local.h"

cinematics_t		cin;
cin_cache		cinTable[MAX_VIDEO_HANDLES];

int				currentHandle = -1;
int				CL_handle = -1;

typedef struct {
	filetype_t fileType;
	char ext[10];
	int (* Play)(const char *arg, int x, int y, int w, int h, int systemBits);
	e_status (* Run)(int handle);
	void (* Shutdown)(void);
} videoapi_t;

extern void RoQReset( void );
extern e_status CIN_RunROQ(int handle);
extern int CIN_PlayROQ(const char *arg, int x, int y, int w, int h, int systemBits );

#ifdef USE_CIN_VPX
extern void Cin_VPX_Shutdown(void);
extern int CIN_PlayVPX(const char *arg, int x, int y, int w, int h, int systemBits );
extern e_status CIN_RunVPX(int handle);
#endif

#if defined(USE_CODEC_VORBIS) && (defined(USE_CIN_XVID) || defined(USE_CIN_THEORA))
extern void Cin_OGM_Shutdown(void);
extern int CIN_PlayOGM(const char *arg, int x, int y, int w, int h, int systemBits );
extern e_status CIN_RunOGM(int handle);
#endif


static videoapi_t cinematicLoaders[ ] = {
#ifdef USE_CIN_VPX
	{FT_VPX, "webm", CIN_PlayVPX, CIN_RunVPX, Cin_VPX_Shutdown},
#endif
#if defined(USE_CODEC_VORBIS) && (defined(USE_CIN_XVID) || defined(USE_CIN_THEORA))
	{FT_OGM, "ogm", CIN_PlayOGM, CIN_RunOGM, Cin_OGM_Shutdown},
	{FT_OGM, "ogv", CIN_PlayOGM, CIN_RunOGM, Cin_OGM_Shutdown},
#endif
	{FT_ROQ, "roq", CIN_PlayROQ, CIN_RunROQ, }
};

static int numCinematicLoaders = ARRAY_LEN(cinematicLoaders);

/******************************************************************************
*
* Function:		
*
* Description:	
*
******************************************************************************/

static int CIN_HandleForVideo( void ) {
	int		i;

	for ( i = 0 ; i < MAX_VIDEO_HANDLES ; i++ ) {
		if ( cinTable[i].fileName[0] == '\0' ) {
			return i;
		}
	}
	Com_Error( ERR_DROP, "CIN_HandleForVideo: none free" );
	return -1;
}


void CIN_Shutdown( void ) {
	const char *s;

	if (!cinTable[currentHandle].buf) {
		return;
	}

	if ( cinTable[currentHandle].status == FMV_IDLE ) {
		return;
	}
	Com_DPrintf("finished cinematic\n");
	cinTable[currentHandle].status = FMV_IDLE;

	if ( cinTable[currentHandle].iFile != FS_INVALID_HANDLE ) {
		FS_FCloseFile( cinTable[currentHandle].iFile );
		cinTable[currentHandle].iFile = FS_INVALID_HANDLE;
	}

	if (cinTable[currentHandle].alterGameState) {
		cls.state = CA_DISCONNECTED;
		// we can't just do a vstr nextmap, because
		// if we are aborting the intro cinematic with
		// a devmap command, nextmap would be valid by
		// the time it was referenced
		s = Cvar_VariableString( "nextmap" );
		if ( s[0] ) {
			Cbuf_ExecuteText( EXEC_APPEND, va("%s\n", s) );
			Cvar_Set( "nextmap", "" );
		}
		CL_handle = -1;
	}
	cinTable[currentHandle].fileName[0] = '\0';
	for(int i = 0; i < numCinematicLoaders; i++) {
		if(cinTable[currentHandle].fileType == cinematicLoaders[i].fileType) {
			if(cinematicLoaders[i].Shutdown) {
				cinematicLoaders[i].Shutdown();
			}
		}
	}
	cinTable[currentHandle].buf = NULL;
	currentHandle = -1;
}

/*
==================
CIN_StopCinematic
==================
*/
e_status CIN_StopCinematic( int handle ) {
	
	if (handle < 0 || handle>= MAX_VIDEO_HANDLES || cinTable[handle].status == FMV_EOF) return FMV_EOF;
	currentHandle = handle;

	Com_DPrintf("trFMV::stop(), closing %s\n", cinTable[currentHandle].fileName);

	if (!cinTable[currentHandle].buf) {
		return FMV_EOF;
	}

	if (cinTable[currentHandle].alterGameState) {
		if ( cls.state != CA_CINEMATIC ) {
			return cinTable[currentHandle].status;
		}
	}
	cinTable[currentHandle].status = FMV_EOF;
	CIN_Shutdown();

	return FMV_EOF;
}

/*
==================
CIN_RunCinematic

Fetch and decompress the pending frame
==================
*/
e_status CIN_RunCinematic_Fake (int handle)
{
  if (handle < 0 || handle>= MAX_VIDEO_HANDLES || cinTable[handle].status == FMV_EOF) return FMV_EOF;
  return cinTable[currentHandle].status;
}



e_status CIN_RunCinematic (int handle)
{
	if (handle < 0 || handle>= MAX_VIDEO_HANDLES || cinTable[handle].status == FMV_EOF) return FMV_EOF;

	if (cin.currentHandle != handle) {
		currentHandle = handle;
		cin.currentHandle = currentHandle;
		cinTable[currentHandle].status = FMV_EOF;
		RoQReset();
	}

	if (cinTable[handle].playonwalls < -1)
	{
		return cinTable[handle].status;
	}

	currentHandle = handle;

	if (cinTable[currentHandle].alterGameState) {
		if ( cls.state != CA_CINEMATIC ) {
			return cinTable[currentHandle].status;
		}
	}

	if (cinTable[currentHandle].status == FMV_IDLE) {
		return cinTable[currentHandle].status;
	}
	
	for(int i = 0; i < numCinematicLoaders; i++) {
		if(cinTable[currentHandle].fileType == cinematicLoaders[i].fileType) {
			return cinematicLoaders[i].Run(handle);
		}
	}
  
  return FMV_EOF;
}

/*
==================
CIN_PlayCinematic
==================
*/

int CIN_PlayCinematic( const char *arg, int x, int y, int w, int h, int systemBits ) {
	char	name[MAX_OSPATH];
  char altName[MAX_QPATH];
	int		i;
  const char	*ext;
  int result;

	if (strchr(arg, '/') == NULL && strchr(arg, '\\') == NULL) {
		Com_sprintf (name, sizeof(name), "video/%s", arg);
	} else {
		Com_sprintf (name, sizeof(name), "%s", arg);
	}

	if (!(systemBits & CIN_system)) {
		for ( i = 0 ; i < MAX_VIDEO_HANDLES ; i++ ) {
			if (!strcmp(cinTable[i].fileName, name) ) {
				return i;
			}
		}
	}

	Com_DPrintf("CIN_PlayCinematic( %s )\n", arg);

	Com_Memset(&cin, 0, sizeof(cinematics_t) );
	currentHandle = CIN_HandleForVideo();
  Com_Memset(&cinTable[currentHandle], 0, sizeof(cin_cache));

	cin.currentHandle = currentHandle;

  ext = COM_GetExtension(name);
  if(ext[0] != '\0') {
  	for(i = 0; i < numCinematicLoaders; i++) {
  		if(!Q_stricmp(ext, cinematicLoaders[i].ext)) {
  			if((result = cinematicLoaders[i].Play(name, x, y, w, h, systemBits)) > -1)
  				return result;
  		}
  	}
  }

  if((result = CIN_PlayROQ(name, x, y, w, h, systemBits)) > -1)
    return result;
  
  COM_StripExtension(name, altName, sizeof(altName));
	for(i = 0; i < numCinematicLoaders; i++) {
    Com_DPrintf("%s: Searching for cinematic %s.%s\n", __func__, altName, cinematicLoaders[i].ext);
		if((result = cinematicLoaders[i].Play(va("%s.%s", altName, cinematicLoaders[i].ext), x, y, w, h, systemBits)) > -1) {
			Com_DPrintf( "WARNING: %s not present, using %s.%s instead\n",
					name, altName, cinematicLoaders[i].ext );
		  return result;
		}
	}

  return -1;
}

void CIN_SetExtents( int handle, int x, int y, int w, int h ) {
	if (handle < 0 || handle>= MAX_VIDEO_HANDLES || cinTable[handle].status == FMV_EOF) return;
	cinTable[handle].xpos = x;
	cinTable[handle].ypos = y;
	cinTable[handle].width = w;
	cinTable[handle].height = h;
	cinTable[handle].dirty = qtrue;
}


void CIN_SetLooping( int handle, qboolean loop ) {
	if (handle < 0 || handle>= MAX_VIDEO_HANDLES || cinTable[handle].status == FMV_EOF) return;
	cinTable[handle].looping = loop;
}


/*
==================
CIN_ResampleCinematic

Resample cinematic to 256x256 and store in buf2
==================
*/
static void CIN_ResampleCinematic( int handle, int *buf2 ) {
	int ix, iy, *buf3, xm, ym, ll;
	byte	*buf;

	buf = cinTable[handle].buf;

	xm = cinTable[handle].CIN_WIDTH/256;
	ym = cinTable[handle].CIN_HEIGHT/256;
	ll = 8;
	if (cinTable[handle].CIN_WIDTH==512) {
		ll = 9;
	}

	buf3 = (int*)buf;
	if (xm==2 && ym==2) {
		byte *bc2, *bc3;
		int	ic, iiy;

		bc2 = (byte *)buf2;
		bc3 = (byte *)buf3;
		for (iy = 0; iy<256; iy++) {
			iiy = iy<<12;
			for (ix = 0; ix<2048; ix+=8) {
				for(ic = ix;ic<(ix+4);ic++) {
					*bc2=(bc3[iiy+ic]+bc3[iiy+4+ic]+bc3[iiy+2048+ic]+bc3[iiy+2048+4+ic])>>2;
					bc2++;
				}
			}
		}
	} else if (xm==2 && ym==1) {
		byte *bc2, *bc3;
		int	ic, iiy;

		bc2 = (byte *)buf2;
		bc3 = (byte *)buf3;
		for (iy = 0; iy<256; iy++) {
			iiy = iy<<11;
			for (ix = 0; ix<2048; ix+=8) {
				for(ic = ix;ic<(ix+4);ic++) {
					*bc2=(bc3[iiy+ic]+bc3[iiy+4+ic])>>1;
					bc2++;
				}
			}
		}
	} else {
		for (iy = 0; iy<256; iy++) {
			for (ix = 0; ix<256; ix++) {
					buf2[(iy<<8)+ix] = buf3[((iy*ym)<<ll) + (ix*xm)];
			}
		}
	}
}


/*
==================
CIN_DrawCinematic
==================
*/
void CIN_DrawCinematic( int handle ) {
	float	x, y, w, h;
	byte	*buf;

	if (handle < 0 || handle>= MAX_VIDEO_HANDLES || cinTable[handle].status == FMV_EOF) return;

	if (!cinTable[handle].buf) {
		return;
	}

	x = cinTable[handle].xpos;
	y = cinTable[handle].ypos;
	w = cinTable[handle].width;
	h = cinTable[handle].height;
	buf = cinTable[handle].buf;

#if 0 // keep aspect ratio for cinematics
	if ( cls.biasX || cls.biasY ) {
		// clear side areas
		re.SetColor( colorBlack );
		re.DrawStretchPic( 0, 0, cls.glconfig.vidWidth, cls.glconfig.vidHeight, 0, 0, 1, 1, cls.whiteShader );
	}
	x = x * cls.scale + cls.biasX;
	y = y * cls.scale + cls.biasY;
	w = w * cls.scale;
	h = h * cls.scale;
#else
	SCR_AdjustFrom640( &x, &y, &w, &h );
#endif

	if (cinTable[handle].dirty && (cinTable[handle].CIN_WIDTH != cinTable[handle].drawX || cinTable[handle].CIN_HEIGHT != cinTable[handle].drawY)) {
		int *buf2;

		buf2 = Hunk_AllocateTempMemory( 256*256*4 );

		CIN_ResampleCinematic(handle, buf2);

		re.DrawStretchRaw( x, y, w, h, 256, 256, (byte *)buf2, handle, qtrue);
		cinTable[handle].dirty = qfalse;
		Hunk_FreeTempMemory(buf2);
		return;
	}

	re.DrawStretchRaw( x, y, w, h, cinTable[handle].drawX, cinTable[handle].drawY, buf, handle, cinTable[handle].dirty);
	cinTable[handle].dirty = qfalse;
}


void CL_PlayCinematic_f( void ) {
	char	*arg, *s;
	int bits = CIN_system;

	if (cls.state == CA_CINEMATIC) {
		SCR_StopCinematic();
	}

	arg = Cmd_Argv( 1 );
  Com_DPrintf("%s: %s\n", __func__, arg);
	s = Cmd_Argv(2);

	if ((s && s[0] == '1') || Q_stricmp(arg,"demoend.roq")==0 || Q_stricmp(arg,"end.roq")==0) {
		bits |= CIN_hold;
	}
	if (s && s[0] == '2') {
		bits |= CIN_loop;
	}

	S_StopAllSounds ();

	CL_handle = CIN_PlayCinematic( arg, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, bits );
	if (CL_handle >= 0) {
		do {
			SCR_RunCinematic();
		} while (cinTable[currentHandle].buf == NULL && cinTable[currentHandle].status == FMV_PLAY); // wait for first frame (load codebook and sound)
	}
}


void SCR_DrawCinematic( void ) {
	if (CL_handle >= 0 && CL_handle < MAX_VIDEO_HANDLES) {
		CIN_DrawCinematic(CL_handle);
	}
}


void SCR_RunCinematic( void ) {
	//if (CL_handle >= 0 && CL_handle < MAX_VIDEO_HANDLES) {
	//	CIN_RunCinematic(CL_handle);
	//}
  for ( int i = 0 ; i < MAX_VIDEO_HANDLES ; i++ ) {
    if ( cinTable[i].fileName[0] != '\0' ) {
      CIN_RunCinematic(i);
    }
  }
}


void SCR_StopCinematic( void ) {
	if (CL_handle >= 0 && CL_handle < MAX_VIDEO_HANDLES) {
		CIN_StopCinematic(CL_handle);
		S_StopAllSounds ();
		CL_handle = -1;
	}
}


void CIN_UploadCinematic( int handle ) {
	if (handle >= 0 && handle < MAX_VIDEO_HANDLES) {
		if (!cinTable[handle].buf) {
			return;
		}
		if (cinTable[handle].playonwalls <= 0 && cinTable[handle].dirty) {
			if (cinTable[handle].playonwalls == 0) {
				cinTable[handle].playonwalls = -1;
			} else {
				if (cinTable[handle].playonwalls == -1) {
					cinTable[handle].playonwalls = -2;
				} else {
					cinTable[handle].dirty = qfalse;
				}
			}
		}

		// Resample the video if needed
		if (cinTable[handle].dirty && (cinTable[handle].CIN_WIDTH != cinTable[handle].drawX || cinTable[handle].CIN_HEIGHT != cinTable[handle].drawY))  {
			int *buf2;

			buf2 = Hunk_AllocateTempMemory( 256*256*4 );

			CIN_ResampleCinematic(handle, buf2);

			re.UploadCinematic( cinTable[handle].CIN_WIDTH, cinTable[handle].CIN_HEIGHT, 256, 256, (byte *)buf2, handle, qtrue);
			cinTable[handle].dirty = qfalse;
			Hunk_FreeTempMemory(buf2);
		} else {
			// Upload video at normal resolution
			re.UploadCinematic( cinTable[handle].CIN_WIDTH, cinTable[handle].CIN_HEIGHT, cinTable[handle].drawX, cinTable[handle].drawY,
					cinTable[handle].buf, handle, cinTable[handle].dirty);
			cinTable[handle].dirty = qfalse;
		}

		if (cl_inGameVideo->integer == 0 && cinTable[handle].playonwalls == 1) {
			cinTable[handle].playonwalls--;
		}
		else if (cl_inGameVideo->integer != 0 && cinTable[handle].playonwalls != 1) {
			cinTable[handle].playonwalls = 1;
		}
	}
}
