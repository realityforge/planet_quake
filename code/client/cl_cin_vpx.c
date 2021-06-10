/*
Brian Cullinan (2021) 100% free, no obligations, no guaruntees, it works on my machine
loosely based on https://github.com/zaps166/libsimplewebm
inspiration from cl_cin_ogm.c and Zach "ZTM" Middleton with Spearmint engine
inspiration from ZaRR and persistent cattle prodding to get me to write this

*/

#ifdef USE_CIN_VPX
#include "cl_cin.h"
#include <webmdec.h>
#include <vpx/vpx_decoder.h>
#include <tools_common.h>

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
  MkvReaderInterface *reader;
  int m_last_space;
  short *pcm;
  int currentTime;
} cin_vpx_t;

typedef struct
{
	int w, h;
	int cs;
	int chromaShiftW, chromaShiftH;
	unsigned char *planes[3];
	int linesize[3];
} Image;


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


qboolean OpusVorbisDecoder_getPCMS16(uint8_t *outBuffer, int *numOutSamples, short *inBuffer, size_t bufferSize)
{
#ifdef USE_CODEC_VORBIS
	if (g_vpx.m_vorbis)
	{
		g_vpx.m_vorbis->op.packet = inBuffer;
		g_vpx.m_vorbis->op.bytes = bufferSize;

		if (vorbis_synthesis(g_vpx.m_vorbis->block, g_vpx.m_vorbis->op))
			return qfalse;
		if (vorbis_synthesis_blockin(g_vpx.m_vorbis->dspState, g_vpx.m_vorbis->block))
			return qfalse;

		int samplesCount, count = 0;
		float **pcm;
		while ((samplesCount = vorbis_synthesis_pcmout(g_vpx.m_vorbis->dspState, &pcm)))
		{
			const int toConvert = samplesCount <= g_vpx.webm_ctx->m_numSamples ? samplesCount : g_vpx.webm_ctx->m_numSamples;
			for (int c = 0; c < g_vpx.m_vorbis->info.channels; ++c)
			{
				float *samples = pcm[c];
				for (int i = 0, j = c; i < toConvert; ++i, j += g_vpx.m_vorbis->info.channels)
				{
					int sample = samples[i] * 32767.0f;
					if (sample > 32767)
						sample = 32767;
					else if (sample < -32768)
						sample = -32768;
					outBuffer[count + j] = sample;
				}
			}
			vorbis_synthesis_read(g_vpx.m_vorbis->dspState, toConvert);
			count += toConvert;
		}

		*numOutSamples = count;
		return qtrue;
	}
#endif
#ifdef USE_CODEC_OPUS
  if (m_opus)
	{
		const int samples = opus_decode(g_vpx.m_opus, inBuffer, bufferSize, outBuffer, webm_ctx.m_numSamples, 0);
		if (samples >= 0)
		{
			*numOutSamples = samples;
			return qtrue;
		}
	}
#endif
	return qfalse;
}


//The data is NOT copied! Only 3-plane, 8-bit images are supported.
enum IMAGE_ERROR VPXDecoder_getImage(Image *image)
{
  const void *m_iter;
  vpx_image_t *img;
  enum IMAGE_ERROR err = NO_FRAME;
  if ((img = vpx_codec_get_frame(g_vpx.vpx_ctx->dcodec, &m_iter)))
  {
    // It seems to be a common problem that UNKNOWN comes up a lot, yet FFMPEG is somehow getting accurate colour-space information.
    // After checking FFMPEG code, *they're* getting colour-space information, so I'm assuming something like this is going on.
    // It appears to work, at least.
    if (img->cs != VPX_CS_UNKNOWN)
      g_vpx.m_last_space = img->cs;
    if ((img->fmt & VPX_IMG_FMT_PLANAR) && !(img->fmt & (VPX_IMG_FMT_HAS_ALPHA | VPX_IMG_FMT_HIGHBITDEPTH)))
    {
      if (img->stride[0] && img->stride[1] && img->stride[2])
      {
        const int uPlane = !!(img->fmt & VPX_IMG_FMT_UV_FLIP) + 1;
        const int vPlane =  !(img->fmt & VPX_IMG_FMT_UV_FLIP) + 1;

        image->w = img->d_w;
        image->h = img->d_h;
        image->cs = g_vpx.m_last_space;
        image->chromaShiftW = img->x_chroma_shift;
        image->chromaShiftH = img->y_chroma_shift;

        image->planes[0] = img->planes[0];
        image->planes[1] = img->planes[uPlane];
        image->planes[2] = img->planes[vPlane];

        image->linesize[0] = img->stride[0];
        image->linesize[1] = img->stride[uPlane];
        image->linesize[2] = img->stride[vPlane];

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

int CL_CIN_FileLength(fileHandle_t file) {
  int		pos;
  int		end;

  pos = FS_FTell( file );
  FS_Seek( file, 0, FS_SEEK_END );
  end = FS_FTell( file );
  FS_Seek( file, pos, FS_SEEK_SET );

  return end;
}

int CL_CIN_FileRead( void *buffer, int len, fileHandle_t f ) {
  int result = FS_Read(buffer, len, f);
  return result;
}

int Cin_VPX_Init(const char *filename)
{
  memset(&g_vpx, 0, sizeof(cin_vpx_t));
  g_vpx.webm_ctx = malloc(sizeof(struct WebmInputContext));
  g_vpx.vpx_ctx = malloc(sizeof(struct VpxInputContext));
  g_vpx.reader = malloc(sizeof(MkvReaderInterface));
  g_vpx.reader->Tell = FS_FTell;
  g_vpx.reader->Read = CL_CIN_FileRead;
  g_vpx.reader->Seek = FS_Seek;
  g_vpx.reader->Length = CL_CIN_FileLength;
  g_vpx.webm_ctx->reader = webm_new_reader(g_vpx.reader);
  cinTable[currentHandle].ROQSize = FS_FOpenFileRead(filename, &cinTable[currentHandle].iFile, qtrue);
  g_vpx.reader->fp = cinTable[currentHandle].iFile;

	if (cinTable[currentHandle].ROQSize<=0) {
		Com_DPrintf("play(%s), VPXSize<=0\n", filename);
		cinTable[currentHandle].fileName[0] = '\0';
		if ( cinTable[currentHandle].iFile != FS_INVALID_HANDLE ) {
			FS_FCloseFile( cinTable[currentHandle].iFile );
			cinTable[currentHandle].iFile = FS_INVALID_HANDLE;
		}
		return 0;
	}

  if (file_is_webm(g_vpx.webm_ctx, g_vpx.vpx_ctx, g_vpx.m_vorbis, g_vpx.m_opus))
  {
    g_vpx.pcm = g_vpx.m_vorbis || g_vpx.m_opus
      ? malloc(sizeof(short) * g_vpx.webm_ctx->m_numSamples * (4096 / g_vpx.webm_ctx->m_numSamples))
      : NULL;
    return 1;
  }

	return 0;
}


int Cin_VPX_Run(int time)
{
	g_vpx.currentTime = time;

  size_t bufferSize;
  uint8_t *buffer;
  int track = webm_read_frame(g_vpx.webm_ctx, &buffer, &bufferSize);
  if(track == g_vpx.webm_ctx->video_track_index)
  {
    if (vpx_codec_decode(g_vpx.vpx_ctx->dcodec, buffer, bufferSize, NULL, 0))
      Com_Error(ERR_DROP, "Failed to decode frame.");

    Image image;
    if (VPXDecoder_getImage(&image) == NO_ERROR)
    {
      
      return 1;
    }
  }
  else if (track == g_vpx.webm_ctx->audio_track_index)
  {
    int outSize;
    if (OpusVorbisDecoder_getPCMS16(buffer, &outSize, g_vpx.pcm, bufferSize))
    {
      
      return 1;
    }
  }
	return 0;
}


void Cin_VPX_Shutdown(void)
{
  webm_free(g_vpx.webm_ctx, g_vpx.vpx_ctx, g_vpx.m_vorbis, g_vpx.m_opus);
  if(g_vpx.pcm)
    free(g_vpx.pcm);
  g_vpx.pcm = NULL;
  if(g_vpx.webm_ctx) {
    free(g_vpx.webm_ctx);
  }
  if(g_vpx.vpx_ctx) {
    free(g_vpx.vpx_ctx);
  }
}

/*
==================
CIN_RunCinematic

Fetch and decompress the pending frame
==================
*/


e_status CIN_RunVPX(int handle) 
{
  if (Cin_VPX_Run(cinTable[currentHandle].startTime == 0 ? 0 : CL_ScaledMilliseconds() - cinTable[currentHandle].startTime))
    cinTable[currentHandle].status = FMV_EOF;
  else
  {
    qboolean	resolutionChange = qfalse;

    if (g_vpx.vpx_ctx->width != cinTable[currentHandle].CIN_WIDTH)
    {
      cinTable[currentHandle].CIN_WIDTH = g_vpx.vpx_ctx->width;
      resolutionChange = qtrue;
    }
    if (g_vpx.vpx_ctx->height != cinTable[currentHandle].CIN_HEIGHT)
    {
      cinTable[currentHandle].CIN_HEIGHT = g_vpx.vpx_ctx->height;
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
  if (!Cin_VPX_Init(cinTable[currentHandle].fileName))
  {
    Com_DPrintf("starting vpx-playback failed(%s)\n", name);
    cinTable[currentHandle].fileName[0] = 0;
    Cin_VPX_Shutdown();
    return 0;
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
      VM_Call(uivm, 1, UI_SET_ACTIVE_MENU, UIMENU_NONE);
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
