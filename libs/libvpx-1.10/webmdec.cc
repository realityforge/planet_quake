/*
 *  Copyright (c) 2013 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <stdint.h>
#include <vorbis/codec.h>
#include <opus.h>
#include "./webmdec.h"

#include <cstring>
#include <cstdio>

#include "third_party/libwebm/mkvparser/mkvparser.h"
#include "third_party/libwebm/mkvparser/mkvreader.h"

namespace {

void reset(struct WebmInputContext *const webm_ctx,
            struct VorbisDecoder *const m_vorbis,
            struct OpusDecoder *const m_opus) {
  if (webm_ctx->reader != nullptr) {
    mkvparser::MkvReader *const reader =
        reinterpret_cast<mkvparser::MkvReader *>(webm_ctx->reader);
    delete reader;
  }
  if (webm_ctx->segment != nullptr) {
    mkvparser::Segment *const segment =
        reinterpret_cast<mkvparser::Segment *>(webm_ctx->segment);
    delete segment;
  }
  if (webm_ctx->buffer != nullptr) {
    delete[] webm_ctx->buffer;
  }
  webm_ctx->reader = nullptr;
  webm_ctx->segment = nullptr;
  webm_ctx->buffer = nullptr;
  webm_ctx->cluster = nullptr;
  webm_ctx->block_entry = nullptr;
  webm_ctx->block = nullptr;
  webm_ctx->block_frame_index = 0;
  webm_ctx->video_track_index = 0;
  webm_ctx->timestamp_ns = 0;
  webm_ctx->is_key_frame = false;
  
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

void get_first_cluster(struct WebmInputContext *const webm_ctx) {
  mkvparser::Segment *const segment =
      reinterpret_cast<mkvparser::Segment *>(webm_ctx->segment);
  const mkvparser::Cluster *const cluster = segment->GetFirst();
  webm_ctx->cluster = cluster;
}

void rewind_and_reset(struct WebmInputContext *const webm_ctx,
                      struct VpxInputContext *const vpx_ctx,
                      struct VorbisDecoder *const m_vorbis,
                      struct OpusDecoder *const m_opus) {
  rewind(vpx_ctx->file);
  reset(webm_ctx, m_vorbis, m_opus);
}

}  // namespace

int file_is_webm(struct WebmInputContext *webm_ctx,
                 struct VpxInputContext *vpx_ctx,
                 struct VorbisDecoder *m_vorbis,
                 struct OpusDecoder *m_opus) {
  m_vorbis = NULL;
  m_opus = NULL;
  mkvparser::MkvReader *const reader = new mkvparser::MkvReader(vpx_ctx->file);
  webm_ctx->reader = reader;
  webm_ctx->reached_eos = 0;

  mkvparser::EBMLHeader header;
  long long pos = 0;
  if (header.Parse(reader, pos) < 0) {
    rewind_and_reset(webm_ctx, vpx_ctx);
    return 0;
  }

  mkvparser::Segment *segment;
  if (mkvparser::Segment::CreateInstance(reader, pos, segment)) {
    rewind_and_reset(webm_ctx, vpx_ctx);
    return 0;
  }
  webm_ctx->segment = segment;
  if (segment->Load() < 0) {
    rewind_and_reset(webm_ctx, vpx_ctx, m_vorbis, m_opus);
    return 0;
  }

  const mkvparser::Tracks *const tracks = segment->GetTracks();
  const mkvparser::VideoTrack *video_track = nullptr;
  const mkvparser::AudioTrack *audio_track = nullptr;

  for (unsigned long i = 0; i < tracks->GetTracksCount(); ++i) {
    const mkvparser::Track *const track = tracks->GetTrackByIndex(i);
    if (track->GetType() == mkvparser::Track::kVideo) {
      video_track = static_cast<const mkvparser::VideoTrack *>(track);
      webm_ctx->video_track_index = static_cast<int>(track->GetNumber());
      break;
    }
    else if (track->GetType() == mkvparser::Track::kAudio) {
      audio_track = static_cast<const mkvparser::AudioTrack *>(track);
      webm_ctx->audio_track_index = static_cast<int>(track->GetNumber());
      break;
    }
  }

  if ((video_track == nullptr || video_track->GetCodecId() == nullptr)
    && (audio_track == nullptr || audio_track->GetCodecId() == nullptr)) {
    rewind_and_reset(webm_ctx, vpx_ctx, m_vorbis, m_opus);
    return 0; // nothing to do
  }

  if (!strncmp(video_track->GetCodecId(), "V_VP8", 5)) {
    vpx_ctx->fourcc = VP8_FOURCC;
  } else if (!strncmp(video_track->GetCodecId(), "V_VP9", 5)) {
    vpx_ctx->fourcc = VP9_FOURCC;
  }
  
  if (!strncmp(audio_track->GetCodecId(), "A_VORBIS", 8)) {
    size_t extradataSize = 0;
  	const unsigned char *extradata = audio_track->GetCodecPrivate(extradataSize);

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

  	m_vorbis = new VorbisDecoder;
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

  } else if (!strncmp(video_track->GetCodecId(), "A_OPUS", 6)) {
    m_opus = opus_decoder_create(m_audioTrack->GetSamplingRate(), m_channels, &opusErr);
    if (!opusErr)
    {
      m_numSamples = m_audioTrack->GetSamplingRate() * 0.06 + 0.5; //Maximum frame size (for 60 ms frame)
    }    
  }
  
  
  if(!vpx_ctx->fourcc && !(m_vorbis || m_opus)) {
    rewind_and_reset(webm_ctx, vpx_ctx, m_vorbis, m_opus);
    return 0;
  }

  vpx_ctx->framerate.denominator = 0;
  vpx_ctx->framerate.numerator = 0;
  vpx_ctx->width = static_cast<uint32_t>(video_track->GetWidth());
  vpx_ctx->height = static_cast<uint32_t>(video_track->GetHeight());

  get_first_cluster(webm_ctx);

  return 1;
}

int webm_read_frame(struct WebmInputContext *webm_ctx, uint8_t **buffer,
                    size_t *buffer_size) {
  // This check is needed for frame parallel decoding, in which case this
  // function could be called even after it has reached end of input stream.
  if (webm_ctx->reached_eos) {
    return 1;
  }
  mkvparser::Segment *const segment =
      reinterpret_cast<mkvparser::Segment *>(webm_ctx->segment);
  const mkvparser::Cluster *cluster =
      reinterpret_cast<const mkvparser::Cluster *>(webm_ctx->cluster);
  const mkvparser::Block *block =
      reinterpret_cast<const mkvparser::Block *>(webm_ctx->block);
  const mkvparser::BlockEntry *block_entry =
      reinterpret_cast<const mkvparser::BlockEntry *>(webm_ctx->block_entry);
  bool block_entry_eos = false;
  do {
    long status = 0;
    bool get_new_block = false;
    if (block_entry == nullptr && !block_entry_eos) {
      status = cluster->GetFirst(block_entry);
      get_new_block = true;
    } else if (block_entry_eos || block_entry->EOS()) {
      cluster = segment->GetNext(cluster);
      if (cluster == nullptr || cluster->EOS()) {
        *buffer_size = 0;
        webm_ctx->reached_eos = 1;
        return 1;
      }
      status = cluster->GetFirst(block_entry);
      block_entry_eos = false;
      get_new_block = true;
    } else if (block == nullptr ||
               webm_ctx->block_frame_index == block->GetFrameCount() ||
               (block->GetTrackNumber() != webm_ctx->video_track_index
                  && block->GetTrackNumber() != webm_ctx->audio_track_index)) {
      status = cluster->GetNext(block_entry, block_entry);
      if (block_entry == nullptr || block_entry->EOS()) {
        block_entry_eos = true;
        continue;
      }
      get_new_block = true;
    }
    if (status || block_entry == nullptr) {
      return -1;
    }
    if (get_new_block) {
      block = block_entry->GetBlock();
      if (block == nullptr) return -1;
      webm_ctx->block_frame_index = 0;
    }
  } while (block_entry_eos ||
           (block->GetTrackNumber() != webm_ctx->video_track_index
            && block->GetTrackNumber() != webm_ctx->audio_track_index));

  webm_ctx->cluster = cluster;
  webm_ctx->block_entry = block_entry;
  webm_ctx->block = block;

  const mkvparser::Block::Frame &frame =
      block->GetFrame(webm_ctx->block_frame_index);
  ++webm_ctx->block_frame_index;
  if (frame.len > static_cast<long>(*buffer_size)) {
    delete[] * buffer;
    *buffer = new uint8_t[frame.len];
    if (*buffer == nullptr) {
      return -1;
    }
    webm_ctx->buffer = *buffer;
  }
  *buffer_size = frame.len;
  webm_ctx->timestamp_ns = block->GetTime(cluster);
  webm_ctx->is_key_frame = block->IsKey();

  mkvparser::MkvReader *const reader =
      reinterpret_cast<mkvparser::MkvReader *>(webm_ctx->reader);
  return frame.Read(reader, *buffer) ? block->GetTrackNumber() : 0;
}

int webm_guess_framerate(struct WebmInputContext *webm_ctx,
                         struct VpxInputContext *vpx_ctx) {
  uint32_t i = 0;
  uint8_t *buffer = nullptr;
  size_t buffer_size = 0;
  while (webm_ctx->timestamp_ns < 1000000000 && i < 50) {
    if (webm_read_frame(webm_ctx, &buffer, &buffer_size)) {
      break;
    }
    ++i;
  }
  vpx_ctx->framerate.numerator = (i - 1) * 1000000;
  vpx_ctx->framerate.denominator =
      static_cast<int>(webm_ctx->timestamp_ns / 1000);
  delete[] buffer;

  get_first_cluster(webm_ctx);
  webm_ctx->block = nullptr;
  webm_ctx->block_entry = nullptr;
  webm_ctx->block_frame_index = 0;
  webm_ctx->timestamp_ns = 0;
  webm_ctx->reached_eos = 0;

  return 0;
}

void webm_free(struct WebmInputContext *webm_ctx) { reset(webm_ctx); }
