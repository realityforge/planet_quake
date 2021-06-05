/*
===========================================================================
Copyright (C) 2008 Stefan Langer <raute@users.sourceforge.net>

This file is part of Turtle Arena source code.

Turtle Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Turtle Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Turtle Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

/*

  This is a "ogm"-decoder to use a "better"(smaller files,higher resolutions) Cinematic-Format than roq

  In this code "ogm" is only: ogg wrapper, vorbis audio, xvid video (or theora video)
    (ogm(Ogg Media) in general is ogg wrapper with all kind of audio/video/subtitle/...)

... infos used for this src:
xvid:
 * examples/xvid_decraw.c
 * xvid.h
ogg/vobis:
 * decoder_example.c (libvorbis src)
 * libogg Documentation ( http://www.xiph.org/ogg/doc/libogg/ )
 * VLC ogg demux ( http://trac.videolan.org/vlc/browser/trunk/modules/demux/ogg.c )
theora:
 * theora doxygen docs (1.0beta1)
*/

#ifdef USE_CIN_VPX
#include "cl_cin.h"
#include <vpx/vpx_decoder.h>

#ifdef USE_CODEC_VORBIS
#include <ogg/ogg.h>
#include <vorbis/codec.h>
#endif

#include "client.h"
#include "snd_local.h"

#define VPX_BUFFER_SIZE	8*1024	//4096

typedef struct
{
  
	int             currentTime;	// input from Run-function
} cin_ogm_t;


extern cinematics_t		cin;
extern cin_cache		cinTable[MAX_VIDEO_HANDLES];
extern int currentHandle;

static cin_ogm_t g_ogm;

int             nextNeededVFrame(void);
extern int		  CL_ScaledMilliseconds( void );
void            Cin_VPX_Shutdown(void);


/* ####################### #######################

  XVID

*/
#ifdef USE_CIN_VPX

static int init_xvid(void)
{

	return (ret);
}

static int dec_xvid(unsigned char *input, int input_size)
{

	return (ret);
}

static int shutdown_xvid(void)
{

	return (ret);
}
#endif

static int loadBlockToSync(void)
{
	int             r = -1;
	char           *buffer;
	int             bytes;

	if(g_ogm.ogmFile)
	{
		buffer = ogg_sync_buffer(&g_ogm.oy, VPX_BUFFER_SIZE);
		bytes = FS_Read(buffer, VPX_BUFFER_SIZE, g_ogm.ogmFile);
		ogg_sync_wrote(&g_ogm.oy, bytes);

		r = (bytes == 0);
	}

	return r;
}

/*
  loadPagesToStreams

  return:
  !0 -> no data transferred (or not for all Streams)
*/
static int loadPagesToStreams(void)
{

	return r;
}

#define SIZEOF_RAWBUFF 4*1024
static byte     rawBuffer[SIZEOF_RAWBUFF];

#define MIN_AUDIO_PRELOAD 400	// in ms
#define MAX_AUDIO_PRELOAD 500	// in ms


/*

  return: audio wants more packets
*/
static qboolean loadAudio(void)
{

	while(anyDataTransferred && g_ogm.currentTime + MAX_AUDIO_PRELOAD > (int)(g_ogm.vd.granulepos * 1000 / g_ogm.vi.rate))
	{

			if(i > 0)
			{
//              S_RawSamples(ssize, 22050, 2, 2, (byte *)sbuf, 1.0f, -1);
				S_RawSamples(i, g_ogm.vi.rate, 2, 2, rawBuffer, s_volume->value);

				anyDataTransferred = qtrue;
			}
		}
	}

	if(g_ogm.currentTime + MIN_AUDIO_PRELOAD > (int)(g_ogm.vd.granulepos * 1000 / g_ogm.vi.rate))
		return qtrue;
	else
		return qfalse;
}


/*

  return: qtrue => noDataTransferred
*/
static qboolean loadFrame(void)
{
	qboolean        anyDataTransferred = qtrue;
	qboolean        needVOutputData = qtrue;

//  qboolean audioSDone = qfalse;
//  qboolean videoSDone = qfalse;
	qboolean        audioWantsMoreData = qfalse;
	int             status;

	while(anyDataTransferred && (needVOutputData || audioWantsMoreData))
	{
		anyDataTransferred = qfalse;

//      xvid -> "gl" ? videoDone : needPacket
//      vorbis -> raw sound ? audioDone : needPacket
//      anyDataTransferred = videoDone && audioDone;
//      needVOutputData = videoDone && audioDone;
//      if needPacket
		{
//          videoStream -> xvid ? videoStreamDone : needPage
//          audioSteam -> vorbis ? audioStreamDone : needPage
//          anyDataTransferred = audioStreamDone && audioStreamDone;

			if(needVOutputData && (status = loadVideoFrame()))
			{
				needVOutputData = qfalse;
				if(status > 0)
					anyDataTransferred = qtrue;
				else
					anyDataTransferred = qfalse;	// error (we don't need any videodata and we had no transferred)
			}

//          if needPage
			if(needVOutputData || audioWantsMoreData)
			{
				// try to transfer Pages to the audio- and video-Stream
				if(loadPagesToStreams())
				{
					// try to load a datablock from file
					anyDataTransferred |= !loadBlockToSync();
				}
				else
					anyDataTransferred = qtrue;	// successful loadPagesToStreams()
			}

			// load all Audio after loading new pages ...
			if(g_ogm.VFrameCount > 1)	// wait some videoframes (it's better to have some delay, than a lagy sound)
				audioWantsMoreData = loadAudio();
		}
	}

//  ogg_packet_clear(&op);

	return !anyDataTransferred;
}

//from VLC ogg.c ( http://trac.videolan.org/vlc/browser/trunk/modules/demux/ogg.c )

/*

  return: 0 -> no problem
*/
//TODO: vorbis/theora-header&init in sub-functions
//TODO: "clean" error-returns ...
int Cin_VPX_Init(const char *filename)
{
	int             status;
	ogg_page        og;
	ogg_packet      op;
	int             i;

	if(g_ogm.ogmFile)
	{
		Com_Printf(S_COLOR_YELLOW "WARNING: it seams there was already a ogm running, it will be killed to start %s\n", filename);
		Cin_OGM_Shutdown();
	}

	memset(&g_ogm, 0, sizeof(cin_ogm_t));

	FS_FOpenFileRead(filename, &g_ogm.ogmFile, qtrue);
	if(!g_ogm.ogmFile)
	{
		Com_Printf(S_COLOR_YELLOW "WARNING: Can't open ogm-file for reading (%s)\n", filename);
		return -1;
	}

	return 0;
}

int nextNeededVFrame(void)
{
	return (int)(g_ogm.currentTime * (ogg_int64_t) 10000 / g_ogm.Vtime_unit);
}

/*

  time ~> time in ms to which the movie should run
  return:	0 => nothing special
			1 => eof
*/
int Cin_VPX_Run(int time)
{

	g_ogm.currentTime = time;

	while(!g_ogm.VFrameCount || time + 20 >= (int)(g_ogm.VFrameCount * g_ogm.Vtime_unit / 10000))
	{
		if(loadFrame())
			return 1;
	}

	return 0;
}

/*
  Gives a Pointer to the current Output-Buffer
  and the Resolution
*/
unsigned char  *Cin_VPX_GetOutput(int *outWidth, int *outHeight)
{
}

void Cin_VPX_Shutdown(void)
{

}

/*
==================
CIN_RunCinematic

Fetch and decompress the pending frame
==================
*/


e_status CIN_RunVPX(int handle) 
{
  int	start = 0;
	int     thisTime = 0;

  if (Cin_VPX_Run(cinTable[currentHandle].startTime == 0 ? 0 : CL_ScaledMilliseconds() - cinTable[currentHandle].startTime))
    cinTable[currentHandle].status = FMV_EOF;
  else
  {
    int			newW, newH;
    qboolean	resolutionChange = qfalse;

    //cinTable[currentHandle].buf = Cin_OGM_GetOutput(&newW, &newH);

    if (newW != cinTable[currentHandle].CIN_WIDTH)
    {
      cinTable[currentHandle].CIN_WIDTH = newW;
      resolutionChange = qtrue;
    }
    if (newH != cinTable[currentHandle].CIN_HEIGHT)
    {
      cinTable[currentHandle].CIN_HEIGHT = newH;
      resolutionChange = qtrue;
    }

    if (resolutionChange)
    {
      cinTable[currentHandle].drawX = cinTable[currentHandle].CIN_WIDTH;
      cinTable[currentHandle].drawY = cinTable[currentHandle].CIN_HEIGHT;

      // rage pro is very slow at 512 wide textures, voodoo can't do it at all
      if ( cls.glconfig.hardwareType == GLHW_RAGEPRO || cls.glconfig.maxTextureSize <= 256) {
        if (cinTable[currentHandle].drawX>256) {
            cinTable[currentHandle].drawX = 256;
        }
        if (cinTable[currentHandle].drawY>256) {
            cinTable[currentHandle].drawY = 256;
        }
        if (cinTable[currentHandle].CIN_WIDTH != 256 || cinTable[currentHandle].CIN_HEIGHT != 256) {
          Com_DPrintf("HACK: approxmimating cinematic for Rage Pro or Voodoo\n");
        }
      }
    }

    cinTable[currentHandle].status = FMV_PLAY;
    cinTable[currentHandle].dirty = qtrue;
  }

  if (!cinTable[currentHandle].startTime)
    cinTable[currentHandle].startTime = CL_ScaledMilliseconds();

  if (cinTable[currentHandle].status == FMV_EOF)
  {
    if (cinTable[currentHandle].holdAtEnd)
    {
      cinTable[currentHandle].status = FMV_IDLE;
    }
    else if (cinTable[currentHandle].looping)
    {
      //Cin_OGM_Shutdown();
      //Cin_OGM_Init(cinTable[currentHandle].fileName);
      cinTable[currentHandle].buf = NULL;
      cinTable[currentHandle].startTime = 0;
      cinTable[currentHandle].status = FMV_PLAY;
    }
    else
    {
      CIN_Shutdown();
//              Cin_OGM_Shutdown();
    }
  }

  return cinTable[currentHandle].status;
}


/*
==================
CIN_PlayCinematic
==================
*/

int CIN_PlayVPX( const char *name, int x, int y, int w, int h, int systemBits ) 
{
  Q_strncpyz( cinTable[currentHandle].fileName, name, sizeof( cinTable[currentHandle].fileName ) );

  if (Cin_OGM_Init(cinTable[currentHandle].fileName))
  {
    Com_DPrintf("starting vpx-playback failed(%s)\n", name);
    cinTable[currentHandle].fileName[0] = 0;
    Cin_VPX_Shutdown();
    return -1;
  }

  cinTable[currentHandle].fileType = FT_VPX;

  CIN_SetExtents(currentHandle, x, y, w, h);
  CIN_SetLooping(currentHandle, (systemBits & CIN_loop) != 0);

  cinTable[currentHandle].holdAtEnd = (systemBits & CIN_hold) != 0;
  cinTable[currentHandle].alterGameState = (systemBits & CIN_system) != 0;
  cinTable[currentHandle].playonwalls = 1;
  cinTable[currentHandle].silent = (systemBits & CIN_silent) != 0;
  cinTable[currentHandle].shader = (systemBits & CIN_shader) != 0;

/* we will set this info after the first xvid-frame
  cinTable[currentHandle].CIN_HEIGHT = DEFAULT_CIN_HEIGHT;
  cinTable[currentHandle].CIN_WIDTH  =  DEFAULT_CIN_WIDTH;
*/

  if (cinTable[currentHandle].alterGameState)
  {
    // close the menu
    if (uivm)
    {
      VM_Call(uivm, UI_SET_ACTIVE_MENU, UIMENU_NONE);
    }
  }
  else
  {
    cinTable[currentHandle].playonwalls = cl_inGameVideo->integer;
  }

  if (cinTable[currentHandle].alterGameState)
  {
    cls.state = CA_CINEMATIC;
  }

  cinTable[currentHandle].status = FMV_PLAY;

  return currentHandle;
}

#endif
