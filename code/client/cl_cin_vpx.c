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

struct VorbisDecoder
{
	vorbis_info info;
	vorbis_dsp_state dspState;
	vorbis_block block;
	ogg_packet op;

	bool hasDSPState, hasBlock;
};

typedef struct
{
  // demuxer
  qboolean m_open;
  VorbisDecoder *m_vorbis;
  OpusDecoder *m_opus;
  qboolean m_isOpen;
  qboolean m_eos;
  fileHandle_t    m_file;

  mkvparser_IMkvReader *m_reader;
  mkvparser_Segment *m_segment;
  const mkvparser_Cluster *m_cluster;
  const mkvparser_Block *m_block;
  const mkvparser_BlockEntry *m_blockEntry;
  int m_blockFrameIndex;
  const mkvparser_VideoTrack *m_videoTrack;
  VIDEO_CODEC m_vCodec;
  const mkvparser_AudioTrack *m_audioTrack;
  AUDIO_CODEC m_aCodec;

  int             currentTime;	// input from Run-function
  
  // audio frame
  long audioBufferSize, audioBufferCapacity;
  unsigned char *audioBuffer;
  double audioTime;
  bool audioKey;
  int m_numSamples;
  int m_channels;


  // video frame
  long videoBufferSize, videoBufferCapacity;
  unsigned char *videoBuffer;
  double videoTime;
  bool videoKey;
  
  int w, h;
  int cs;
  int chromaShiftW, chromaShiftH;
  unsigned char *planes[3];
  int linesize[3];
  int threads;


  // vpx decoder
  vpx_codec_ctx *m_ctx;
  const void *m_iter;
  int m_delay;
  int m_last_space;
  
  
} cin_vpx_t

static cin_vpx_t g_vpx;

extern cinematics_t cin;
extern cin_cache		cinTable[MAX_VIDEO_HANDLES];
extern int          currentHandle;
extern int CL_ScaledMilliseconds( void );


enum VIDEO_CODEC
{
  NO_VIDEO,
  VIDEO_VP8,
  VIDEO_VP9
};

enum AUDIO_CODEC
{
  NO_AUDIO,
  AUDIO_VORBIS,
  AUDIO_OPUS
};


enum IMAGE_ERROR
{
  UNSUPPORTED_FRAME = -1,
  NO_ERROR,
  NO_FRAME
};

bool OpusVorbisDecoder_getPCMS16(WebMFrame &frame, short *buffer, int &numOutSamples)
{
	if (m_vorbis)
	{
		m_vorbis->op.packet = frame.buffer;
		m_vorbis->op.bytes = frame.bufferSize;

		if (vorbis_synthesis(&m_vorbis->block, &m_vorbis->op))
			return false;
		if (vorbis_synthesis_blockin(&m_vorbis->dspState, &m_vorbis->block))
			return false;

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
					buffer[count + j] = sample;
				}
			}
			vorbis_synthesis_read(&m_vorbis->dspState, toConvert);
			count += toConvert;
		}

		numOutSamples = count;
		return true;
	}
	else if (m_opus)
	{
		const int samples = opus_decode(m_opus, frame.buffer, frame.bufferSize, buffer, m_numSamples, 0);
		if (samples >= 0)
		{
			numOutSamples = samples;
			return true;
		}
	}
	return false;
}


bool OpusVorbisDecoder_openVorbis(const WebMDemuxer &demuxer)
{
	size_t extradataSize = 0;
	const unsigned char *extradata = m_audioTrack->GetCodecPrivate(extradataSize);

	if (extradataSize < 3 || !extradata || extradata[0] != 2)
		return false;

	size_t headerSize[3] = {0};
	size_t offset = 1;

	/* Calculate three headers sizes */
	for (int i = 0; i < 2; ++i)
	{
		for (;;)
		{
			if (offset >= extradataSize)
				return false;
			headerSize[i] += extradata[offset];
			if (extradata[offset++] < 0xFF)
				break;
		}
	}
	headerSize[2] = extradataSize - (headerSize[0] + headerSize[1] + offset);

	if (headerSize[0] + headerSize[1] + headerSize[2] + offset != extradataSize)
		return false;

	ogg_packet op[3];
	memset(op, 0, sizeof op);

	op[0].packet = (unsigned char *)extradata + offset;
	op[0].bytes = headerSize[0];
	op[0].b_o_s = 1;

	op[1].packet = (unsigned char *)extradata + offset + headerSize[0];
	op[1].bytes = headerSize[1];

	op[2].packet = (unsigned char *)extradata + offset + headerSize[0] + headerSize[1];
	op[2].bytes = headerSize[2];

	m_vorbis = (VorbisDecoder)malloc(sizeof(VorbisDecoder));
	m_vorbis->hasDSPState = m_vorbis->hasBlock = false;
	vorbis_info_init(&m_vorbis->info);

	/* Upload three Vorbis headers into libvorbis */
	vorbis_comment vc;
	vorbis_comment_init(&vc);
	for (int i = 0; i < 3; ++i)
	{
		if (vorbis_synthesis_headerin(&m_vorbis->info, &vc, &op[i]))
		{
			vorbis_comment_clear(&vc);
			return false;
		}
	}
	vorbis_comment_clear(&vc);

	if (vorbis_synthesis_init(&m_vorbis->dspState, &m_vorbis->info))
		return false;
	m_vorbis->hasDSPState = true;

	if (m_vorbis->info.channels != m_channels || m_vorbis->info.rate != m_audioTrack->GetSamplingRate())
		return false;

	if (vorbis_block_init(&m_vorbis->dspState, &m_vorbis->block))
		return false;
	m_vorbis->hasBlock = true;

	memset(&m_vorbis->op, 0, sizeof m_vorbis->op);

	m_numSamples = 4096 / m_channels;

	return true;
}


bool OpusVorbisDecoder_openOpus(const WebMDemuxer &demuxer)
{
	int opusErr = 0;
	m_opus = opus_decoder_create(m_audioTrack->GetSamplingRate(), m_channels, &opusErr);
	if (!opusErr)
	{
		m_numSamples = m_audioTrack->GetSamplingRate() * 0.06 + 0.5; //Maximum frame size (for 60 ms frame)
		return true;
	}
	return false;
}


void OpusVorbisDecoder_close()
{
	if (m_vorbis)
	{
		if (m_vorbis->hasBlock)
			vorbis_block_clear(&m_vorbis->block);
		if (m_vorbis->hasDSPState)
			vorbis_dsp_clear(&m_vorbis->dspState);
		vorbis_info_clear(&m_vorbis->info);
		delete m_vorbis;
	}
	if (m_opus)
		opus_decoder_destroy(m_opus);
}


/**/

void OpusVorbisDecoder(const WebMDemuxer &demuxer) {
  OpusVorbisDecoder_close();
  m_vorbis = NULL;
  m_opus = NULL;
  m_numSamples = 0;
	switch (m_aCodec)
	{
		case WebMDemuxer_AUDIO_VORBIS:
			m_channels = m_audioTrack->GetChannels();
			if (OpusVorbisDecoder_openVorbis(demuxer))
				return;
			break;
		case WebMDemuxer_AUDIO_OPUS:
			m_channels = m_audioTrack->GetChannels();
			if (OpusVorbisDecoder_openOpus(demuxer))
				return;
			break;
		default:
			return;
	}
	OpusVorbisDecoder_close();
}


void notSupportedTrackNumber(long videoTrackNumber, long audioTrackNumber) const
{
	const long trackNumber = m_block->GetTrackNumber();
	return (trackNumber != videoTrackNumber && trackNumber != audioTrackNumber);
}


void VPXDecoder(const WebMDemuxer &demuxer, unsigned threads) {
  if (m_ctx)
  {
    vpx_codec_destroy(m_ctx);
  }
  m_ctx = NULL;
  m_iter = NULL;
  m_delay = 0;
  m_last_space = VPX_CS_UNKNOWN;

  if (threads > 8)
    threads = 8;
  else if (threads < 1)
    threads = 1;

  const vpx_codec_dec_cfg_t codecCfg = {
    threads,
    0,
    0
  };
  vpx_codec_iface_t *codecIface = NULL;

  switch (m_vCodec)
  {
    case WebMDemuxer_VIDEO_VP8:
      codecIface = vpx_codec_vp8_dx();
      break;
    case WebMDemuxer_VIDEO_VP9:
      codecIface = vpx_codec_vp9_dx();
      m_delay = threads - 1;
      break;
    default:
      return;
  }

  m_ctx = (vpx_codec_ctx_t)malloc(sizeof(vpx_codec_ctx_t));
  if (vpx_codec_dec_init(m_ctx, codecIface, &codecCfg, m_delay > 0 ? VPX_CODEC_USE_FRAME_THREADING : 0))
  {
    free(m_ctx);
    m_ctx = NULL;
  }
}

bool VPXDecoder_decode(const WebMFrame &frame)
{
  m_iter = NULL;
  return !vpx_codec_decode(m_ctx, frame.buffer, frame.bufferSize, NULL, 0);
}


//The data is NOT copied! Only 3-plane, 8-bit images are supported.
VPXDecoder_IMAGE_ERROR VPXDecoder_getImage(Image &image)
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

/**/

static int ceilRshift(int val, int shift)
{
  return (val + (1 << shift) - 1) >> shift;
}

int VPXDecoder_Image_getWidth(int plane) const
{
  if (!plane)
    return m_vpx.w;
  return ceilRshift(m_vpx.w, m_vpx.chromaShiftW);
}

int VPXDecoder_Image_getHeight(int plane) const
{
  if (!plane)
    return m_vpx.h;
  return ceilRshift(m_vpx.h, m_vpx.chromaShiftH);
}


static qboolean loadFrame(void)
{
  const long videoTrackNumber = (videoFrame && m_videoTrack) ? m_videoTrack->GetNumber() : 0;
	const long audioTrackNumber = (audioFrame && m_audioTrack) ? m_audioTrack->GetNumber() : 0;
	bool blockEntryEOS = false;

	if (videoFrame)
		videoFrame->bufferSize = 0;
	if (audioFrame)
		audioFrame->bufferSize = 0;

	if (videoTrackNumber == 0 && audioTrackNumber == 0)
		return false;

	if (g_vpx.m_eos)
		return false;

	if (!m_cluster)
		m_cluster = m_segment->GetFirst();

	do
	{
		bool getNewBlock = false;
		long status = 0;
		if (!m_blockEntry && !blockEntryEOS)
		{
			status = m_cluster->GetFirst(m_blockEntry);
			getNewBlock = true;
		}
		else if (blockEntryEOS || m_blockEntry->EOS())
		{
			m_cluster = m_segment->GetNext(m_cluster);
			if (!m_cluster || m_cluster->EOS())
			{
				m_eos = true;
				return false;
			}
			status = m_cluster->GetFirst(m_blockEntry);
			blockEntryEOS = false;
			getNewBlock = true;
		}
		else if (!m_block || m_blockFrameIndex == m_block->GetFrameCount() || notSupportedTrackNumber(videoTrackNumber, audioTrackNumber))
		{
			status = m_cluster->GetNext(m_blockEntry, m_blockEntry);
			if (!m_blockEntry  || m_blockEntry->EOS())
			{
				blockEntryEOS = true;
				continue;
			}
			getNewBlock = true;
		}
		if (status || !m_blockEntry)
			return false;
		if (getNewBlock)
		{
			m_block = m_blockEntry->GetBlock();
			m_blockFrameIndex = 0;
		}
	} while (blockEntryEOS || notSupportedTrackNumber(videoTrackNumber, audioTrackNumber));

	WebMFrame *frame = NULL;

	const long trackNumber = m_block->GetTrackNumber();
	if (trackNumber == videoTrackNumber)
		frame = videoFrame;
	else if (trackNumber == audioTrackNumber)
		frame = audioFrame;
	else
	{
		//Should not be possible
		assert(trackNumber == videoTrackNumber || trackNumber == audioTrackNumber);
		return false;
	}

	const mkvparser_Block_Frame &blockFrame = m_block->GetFrame(m_blockFrameIndex++);
	if (blockFrame.len > frame->bufferCapacity)
	{
		unsigned char *newBuff = (unsigned char *)realloc(frame->buffer, frame->bufferCapacity = blockFrame.len);
		if (newBuff)
			frame->buffer = newBuff;
		else // Out of memory
			return false;
	}
	frame->bufferSize = blockFrame.len;

	frame->time = m_block->GetTime(m_cluster) / 1e9;
	frame->key  = m_block->IsKey();

	return !blockFrame.Read(m_reader, frame->buffer);
}


void WebMDemuxer_Init() {
	m_segment = NULL;
	m_cluster = NULL;
  m_block = NULL;
  m_blockEntry = NULL;
	m_blockFrameIndex = 0;
	m_videoTrack = NULL;
  m_vCodec = NO_VIDEO;
  m_audioTrack = NULL;
  m_aCodec = NO_AUDIO;
  
  m_isOpen = qfalse;
  m_eos = qfakse;
	long long pos = 0;
	if (mkvparser_EBMLHeader().Parse(m_reader, pos))
		return;

	if (mkvparser_Segment_CreateInstance(m_reader, pos, m_segment))
		return;

	if (m_segment->Load() < 0)
		return;

	const mkvparser_Tracks *tracks = m_segment->GetTracks();
	const unsigned long tracksCount = tracks->GetTracksCount();
	int currVideoTrack = -1, currAudioTrack = -1;
	for (unsigned long i = 0; i < tracksCount; ++i)
	{
		const mkvparser_Track *track = tracks->GetTrackByIndex(i);
		if (const char *codecId = track->GetCodecId())
		{
			if ((!m_videoTrack || currVideoTrack != videoTrack) && track->GetType() == mkvparser_Track_kVideo)
			{
				if (!strcmp(codecId, "V_VP8"))
					m_vCodec = VIDEO_VP8;
				else if (!strcmp(codecId, "V_VP9"))
					m_vCodec = VIDEO_VP9;
				if (m_vCodec != NO_VIDEO)
					m_videoTrack = static_cast<const mkvparser_VideoTrack *>(track);
				++currVideoTrack;
			}
			if ((!m_audioTrack || currAudioTrack != audioTrack) && track->GetType() == mkvparser_Track_kAudio)
			{
				if (!strcmp(codecId, "A_VORBIS")) {
					m_aCodec = AUDIO_VORBIS;
#ifndef USE_CODEC_VORBIS
          Com_Printf("WARNING: WebM has vorbis encoded audio but, but Vorbis support was not compiled, set USE_CODEC_VORBIS during compile.")
          m_aCodec = NO_AUDIO;
#endif
				} else if (!strcmp(codecId, "A_OPUS")) {
					m_aCodec = AUDIO_OPUS;
#ifndef USE_CODEC_VORBIS
          Com_Printf("WARNING: WebM has Opus encoded audio but, but Opus support was not compiled, set USE_CODEC_OPUS during compile.")
          m_aCodec = NO_AUDIO;
#endif
				}
        if (m_aCodec != NO_AUDIO)
					m_audioTrack = static_cast<const mkvparser_AudioTrack *>(track);
				++currAudioTrack;
			}
		}
	}
	if (!m_videoTrack && !m_audioTrack)
		return;

	m_isOpen = qtrue;
}


int Cin_VPX_Init(const char *filename)
{
  int             status;
	ogg_page        og;
	ogg_packet      op;
	int             i;

  if (m_file)
    fclose(m_file);

  WebMDemuxer_Init();
	if (m_isOpen)
	{
    VPXDecoder();
		OpusVorbisDecoder();

		WebMFrame videoFrame, audioFrame;

		VPXDecoder_Image image;
		short *pcm = m_vorbis || m_opus ? new short[m_numSamples * m_audioTrack->GetChannels()] : NULL;

		fprintf(stderr, "Length: %f\n", m_segment->GetDuration() / 1e9);
  }

	return 0;
}


int Cin_VPX_Run(int time)
{
	g_vpx.currentTime = time;

	//while(!g_ogm.VFrameCount || time + 20 >= (int)(g_ogm.VFrameCount * g_ogm.Vtime_unit / 10000))
	//{
		
	//}
  if(loadFrame(&videoFrame, &audioFrame))
  {
    if (m_ctx && videoBufferSize > 0)
    {
      if (!VPXDecoder_decode(videoFrame))
      {
        fprintf(stderr, "Video decode error\n");
        break;
      }
      while (VPXDecoder_getImage(image) == VPXDecoder_NO_ERROR)
      {
  // 					for (int p = 0; p < 3; ++p)
  // 					{
  // 						const int w = image.getWidth(p);
  // 						const int h = image.getHeight(p);
  // 						int offset = 0;
  // 						for (int y = 0; y < h; ++y)
  // 						{
  // 							fwrite(image.planes[p] + offset, 1, w, stdout);
  // 							offset += image.linesize[p];
  // 						}
  // 					}
      }
    }
    if ((m_vorbis || m_opus) && audioBufferSize > 0)
    {
      int numOutSamples;
      if (!OpusVorbisDecoder_getPCMS16(audioFrame, pcm, numOutSamples))
      {
        fprintf(stderr, "Audio decode error\n");
        break;
      }
  // 				fwrite(pcm, 1, numOutSamples * demuxer.getChannels() * sizeof(short), stdout);
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
      cinTable[currentHandle].CIN_WIDTH = VPXDecoder_Image_getWidth(1);
      resolutionChange = qtrue;
    }
    if (newH != cinTable[currentHandle].CIN_HEIGHT)
    {
      cinTable[currentHandle].CIN_HEIGHT = VPXDecoder_Image_getHeight(1);
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
