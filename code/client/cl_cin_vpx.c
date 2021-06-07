/*
Brian Cullinan (2021) 100% free, no obligations, no guaruntees, it works on my machine
loosely based on https://github.com/zaps166/libsimplewebm
inspiration from cl_cin_ogm.c and Zach "ZTM" Middleton with Spearmint engine
inspiration from ZaRR and persistent cattle prodding to get me to write this

*/

#ifdef USE_CIN_VPX
#include "cl_cin.h"
#include <vpx/vpx_decoder.h>
#include <vpx/vp8dx.h>
#include <mkvparser/mkvparser.h>

#ifdef USE_CODEC_VORBIS
#include <vorbis/codec.h>
#endif
#ifdef USE_CODEC_OPUS
#include <opus/opus.h>
#endif

#include "client.h"
#include "snd_local.h"

#define VPX_BUFFER_SIZE	8*1024	//4096

typedef struct
{
  struct WebmInputContext *webm_ctx;
  struct VpxInputContext *vpx_ctx;
  struct VorbisDecoder *m_vorbis;
  struct OpusDecoder *m_opus;

} cin_vpx_t

static cin_vpx_t g_vpx;

extern cinematics_t cin;
extern cin_cache		cinTable[MAX_VIDEO_HANDLES];
extern int          currentHandle;
extern int CL_ScaledMilliseconds( void );

enum IMAGE_ERROR
{
  UNSUPPORTED_FRAME = -1,
  NO_ERROR,
  NO_FRAME
};


qboolean OpusVorbisDecoder_getPCMS16(short *outBuffer, int *numOutSamples, short *inBuffer, int bufferSize)
{
	if (m_vorbis)
	{
		m_vorbis->op.packet = inBuffer;
		m_vorbis->op.bytes = bufferSize;

		if (vorbis_synthesis(&m_vorbis->block, &m_vorbis->op))
			return qfalse;
		if (vorbis_synthesis_blockin(&m_vorbis->dspState, &m_vorbis->block))
			return qfalse;

		const int maxSamples = m_numSamples;
		int samplesCount, count = 0;
		float **pcm;
		while ((samplesCount = vorbis_synthesis_pcmout(&m_vorbis->dspState, &pcm)))
		{
			const int toConvert = samplesCount <= maxSamples ? samplesCount : maxSamples;
			for (int c = 0; c < m_channels; ++c)
			{
				float *samples = pcm[c];
				for (int i = 0, j = c; i < toConvert; ++i, j += m_channels)
				{
					int sample = samples[i] * 32767.0f;
					if (sample > 32767)
						sample = 32767;
					else if (sample < -32768)
						sample = -32768;
					outBuffer[count + j] = sample;
				}
			}
			vorbis_synthesis_read(&m_vorbis->dspState, toConvert);
			count += toConvert;
		}

		numOutSamples = count;
		return qtrue;
	}
	else if (m_opus)
	{
		const int samples = opus_decode(m_opus, inBuffer, bufferSize, outBuffer, m_numSamples, 0);
		if (samples >= 0)
		{
			numOutSamples = samples;
			return qtrue;
		}
	}
	return qfalse;
}


//The data is NOT copied! Only 3-plane, 8-bit images are supported.
IMAGE_ERROR VPXDecoder_getImage(Image &image)
{
  IMAGE_ERROR err = NO_FRAME;
  if (vpx_image_t *img = vpx_codec_get_frame(m_ctx, &m_iter))
  {
    // It seems to be a common problem that UNKNOWN comes up a lot, yet FFMPEG is somehow getting accurate colour-space information.
    // After checking FFMPEG code, *they're* getting colour-space information, so I'm assuming something like this is going on.
    // It appears to work, at least.
    if (img->cs != VPX_CS_UNKNOWN)
      m_last_space = img->cs;
    if ((img->fmt & VPX_IMG_FMT_PLANAR) && !(img->fmt & (VPX_IMG_FMT_HAS_ALPHA | VPX_IMG_FMT_HIGHBITDEPTH)))
    {
      if (img->stride[0] && img->stride[1] && img->stride[2])
      {
        const int uPlane = !!(img->fmt & VPX_IMG_FMT_UV_FLIP) + 1;
        const int vPlane =  !(img->fmt & VPX_IMG_FMT_UV_FLIP) + 1;

        image.w = img->d_w;
        image.h = img->d_h;
        image.cs = m_last_space;
        image.chromaShiftW = img->x_chroma_shift;
        image.chromaShiftH = img->y_chroma_shift;

        image.planes[0] = img->planes[0];
        image.planes[1] = img->planes[uPlane];
        image.planes[2] = img->planes[vPlane];

        image.linesize[0] = img->stride[0];
        image.linesize[1] = img->stride[uPlane];
        image.linesize[2] = img->stride[vPlane];

        err = NO_ERROR;
      }
    }
    else
    {
      err = UNSUPPORTED_FRAME;
    }
  }
  return err;
}


int Cin_VPX_Init(const char *filename)
{
  if (m_file)
    fclose(m_file);

  if (file_is_webm(g_vpx.webm_ctx, g_vpx.vpx_ctx,
    g_vpx.m_vorbis, g_vpx.m_opus) {
    return 1;
  }

	return 0;
}


int Cin_VPX_Run(int time)
{
	g_vpx.currentTime = time;

  int bufferSize;
  int track = webm_read_frame(g_vpx.webm_ctx, uint8_t **buffer,
                              &bufferSize);
  if(track == g_vpx.webm_ctx.video_track_index)
  {
    if (VPXDecoder_getImage(buffer) == NO_ERROR)
    {
      return 1;
    }
  }
  else if (track == g_vpx.webm_ctx.audio_track_index)
  {
    if (OpusVorbisDecoder_getPCMS16(buffer, bufferSize, pcm, numOutSamples))
    {
      return 1;
    }
  }
	return 0;
}


void Cin_VPX_Shutdown(void)
{
  OpusVorbisDecoder_close();
  free(pcm);
  pcm = NULL;
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
    qboolean	resolutionChange = qfalse;

    if (newW != cinTable[currentHandle].CIN_WIDTH)
    {
      cinTable[currentHandle].CIN_WIDTH = g_vpx.vpx_ctx.width;
      resolutionChange = qtrue;
    }
    if (newH != cinTable[currentHandle].CIN_HEIGHT)
    {
      cinTable[currentHandle].CIN_HEIGHT = g_vpx.vpx_ctx.height;
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
      Cin_VPX_Shutdown();
      Cin_VPX_Init(cinTable[currentHandle].fileName);
      cinTable[currentHandle].buf = NULL;
      cinTable[currentHandle].startTime = 0;
      cinTable[currentHandle].status = FMV_PLAY;
    }
    else
    {
      CIN_Shutdown();
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

  if (Cin_VPX_Init(cinTable[currentHandle].fileName))
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
