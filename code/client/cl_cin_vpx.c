/*
Brian Cullinan (2021) 100% free, no obligations, no guaruntees, it works on my machine
loosely based on https://github.com/zaps166/libsimplewebm
and https://www.philhassey.com/blog/2012/02/02/how-to-create-and-play-ivf-vp8-webm-libvpx-video-in-opengl/
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
  byte *buffer;
} cin_vpx_t;


static cin_vpx_t g_vpx;

extern cinematics_t cin;
extern cin_cache		cinTable[MAX_VIDEO_HANDLES];
extern int          currentHandle;
extern int CL_ScaledMilliseconds( void );


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
  memset(g_vpx.webm_ctx, 0, sizeof(struct WebmInputContext));
  g_vpx.vpx_ctx = malloc(sizeof(struct VpxInputContext));
  memset(g_vpx.vpx_ctx, 0, sizeof(struct VpxInputContext));
  g_vpx.reader = malloc(sizeof(MkvReaderInterface));
  memset(g_vpx.reader, 0, sizeof(MkvReaderInterface));
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

  size_t bufferSize = 0;
  uint8_t *buffer = NULL;
  int track = webm_read_frame(g_vpx.webm_ctx, &buffer, &bufferSize);
  if(track == g_vpx.webm_ctx->video_track_index)
  {
    if (vpx_codec_decode(g_vpx.vpx_ctx->dcodec, buffer, bufferSize, NULL, 0))
      Com_Error(ERR_DROP, "Failed to decode frame.");

    vpx_codec_iter_t iter = NULL;
    vpx_image_t *img = vpx_codec_get_frame(g_vpx.vpx_ctx->dcodec, &iter);
    if (img)
    {
      /*
			yWShift = findSizeShift(g_ogm.th_yuvbuffer.y_width, g_ogm.th_info.width);
			uvWShift = findSizeShift(g_ogm.th_yuvbuffer.uv_width, g_ogm.th_info.width);
			yHShift = findSizeShift(g_ogm.th_yuvbuffer.y_height, g_ogm.th_info.height);
			uvHShift = findSizeShift(g_ogm.th_yuvbuffer.uv_height, g_ogm.th_info.height);

			if(yWShift < 0 || uvWShift < 0 || yHShift < 0 || uvHShift < 0)
			{
				Com_Printf("[Theora] unexpected resolution in a yuv-Frame\n");
				r = -1;
			}
			else
			{

				Frame_yuv_to_rgb24(g_ogm.th_yuvbuffer.y, g_ogm.th_yuvbuffer.u, g_ogm.th_yuvbuffer.v,
								   g_ogm.th_info.width, g_ogm.th_info.height, 
                   img->stride[0], img->stride[1], 
                   yWShift, uvWShift, yHShift, uvHShift,
								   (unsigned int *)g_vpx.buffer);
      }
      */
      //webm_yuv_to_rgb(&g_vpx.buffer, img);
      return 0;
    }
  }
  else if (track == g_vpx.webm_ctx->audio_track_index)
  {
    int outSize;
    if (OpusVorbisDecoder_getPCMS16(buffer, &outSize, g_vpx.pcm, bufferSize))
    {
      
      return 0;
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
  if (Cin_VPX_Run(cinTable[currentHandle].startTime == 0 ? 0 : CL_ScaledMilliseconds() - cinTable[currentHandle].startTime)) {
    cinTable[currentHandle].status = FMV_EOF;
  }
  else
  {
    qboolean	resolutionChange = qfalse;

    cinTable[currentHandle].buf = g_vpx.buffer;

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
    
    /*

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
    */

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
  Com_Printf("%s\n", __func__);
  Q_strncpyz( cinTable[currentHandle].fileName, name, sizeof( cinTable[currentHandle].fileName ) );
  if (!Cin_VPX_Init(cinTable[currentHandle].fileName))
  {
    Com_DPrintf("starting vpx-playback failed(%s)\n", name);
    cinTable[currentHandle].fileName[0] = 0;
    Cin_VPX_Shutdown();
    return -1;
  }

  cinTable[currentHandle].fileType = FT_VPX;

  CIN_SetExtents(currentHandle, x, y, w, h);
  CIN_SetLooping(currentHandle, (systemBits & CIN_loop) != 0);

  cinTable[currentHandle].CIN_HEIGHT = DEFAULT_CIN_HEIGHT;
	cinTable[currentHandle].CIN_WIDTH  =  DEFAULT_CIN_WIDTH;
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
