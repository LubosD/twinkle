/*
    Copyright (C) 2005-2009  Michel de Boer <michel@twinklephone.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _AUDIO_CODECS_H
#define _AUDIO_CODECS_H

#include <string>
#include "twinkle_config.h"
#include "g711.h"
#include "g72x.h"
#include "log.h"

// Audio codecs
enum t_audio_codec {
	CODEC_NULL,
	CODEC_UNSUPPORTED,
	CODEC_G711_ALAW,
	CODEC_G711_ULAW,
	CODEC_GSM,
	CODEC_SPEEX_NB,
	CODEC_SPEEX_WB,
	CODEC_SPEEX_UWB,
	CODEC_OPUS,
	CODEC_ILBC,
	CODEC_G722,
	CODEC_G726_16,
	CODEC_G726_24,
	CODEC_G726_32,
	CODEC_G726_40,
	CODEC_TELEPHONE_EVENT,
	CODEC_G729A
};

enum t_opus_bandwidth_sample_rate {
	OPUS_SAMPLE_RATE_NB  =  8000,
	OPUS_SAMPLE_RATE_MB  = 12000,
	OPUS_SAMPLE_RATE_WB  = 16000,
	OPUS_SAMPLE_RATE_SWB = 24000,
	OPUS_SAMPLE_RATE_FB  = 48000,
};

// Format specific parameters, received on a "a=fmtp:" line during the SDP
// negotiation, to be passed to the corresponding encoder/decoder
struct t_codec_sdp_params {
#ifdef HAVE_OPUS
	bool			opus_cbr = 0;
	unsigned int		opus_maxplaybackrate = 0;
	unsigned int		opus_maxaveragebitrate = 0;
	bool			opus_useinbandfec = 0;
	bool			opus_usedtx = 0;
#endif
};

// Default ptime values (ms) for audio codecs
#define PTIME_G711_ALAW		20
#define PTIME_G711_ULAW		20
#define PTIME_G722		20
#define PTIME_G726		20
#define PTIME_GSM		20
#define PTIME_SPEEX		20
#define PTIME_OPUS		20
#define MIN_PTIME		10
#define MAX_PTIME		80

// Audio sample settings
#define AUDIO_SAMPLE_SIZE	16


// Maximum length (in packets) for concealment of lost packets
#define MAX_CONCEALMENT		2

// Size of jitter buffer in ms
// The jitter buffer is used to smooth playing out incoming RTP packets.
// The size of the buffer is also used as the expiry time in the ccRTP
// stack. Packets that have timestamp that is older than then size of
// the jitter buffer will not be sent out anymore.
#define JITTER_BUF_MS		80

// Duration of the expiry timer in the RTP stack.
// The ccRTP stack checks all data delivered to it against its clock.
// If the data is too old it will not send it out. Data can be old
// for several reasons:
// 
// 1) The thread reading the soundcard has been paused for a while
// 2) The audio card buffers sound before releasing it.
//
// Especially the latter seems to happen on some soundcards. Data
// not older than defined delay are still allowed to go out. It's up
// to the receiving and to deal with the jitter this may cause.
#define MAX_OUT_AUDIO_DELAY_MS	160

// Buffer sizes
#define JITTER_BUF_SIZE(sample_rate) (JITTER_BUF_MS * (sample_rate)/1000 * AUDIO_SAMPLE_SIZE/8)

// Log speex errors
#define LOG_SPEEX_ERROR(func, spxfunc, spxerr) {\
	log_file->write_header((func), LOG_NORMAL, LOG_DEBUG);\
	log_file->write_raw("Speex error: ");\
	log_file->write_raw((spxfunc));\
	log_file->write_raw(" returned ");\
	log_file->write_raw((spxerr));\
	log_file->write_footer(); }

// Return the sampling rate for a codec
unsigned short audio_sample_rate(t_audio_codec codec);
// Some codecs (namely G.722 as far as we are concerned) are required to have
// an RTP clock rate that differs from the actual sampling rate
unsigned short audio_sample_rate_rtp(t_audio_codec codec);
// Ratio of sampling clock rate to RTP clock rate
unsigned short audio_sample_rate_rtp_ratio(t_audio_codec codec);

// Returns true if the codec is a speex codec
bool is_speex_codec(t_audio_codec codec);

// Resample the input buffer to the output buffer
// Returns the number of samples put in the output buffer
// If the output buffer is too small, the number of samples will be
// truncated.
int resample(short *input_buf, int input_len, int input_sample_rate,
	short *output_buf, int output_len, int output_sample_rate);

// Mix 2 16 bits signed linear PCM values
short mix_linear_pcm(short pcm1, short pcm2);

#ifdef HAVE_OPUS
// Bump an arbitrary ptime to the next value allowed by the Opus codec
unsigned short opus_adjusted_ptime(unsigned short ptime);

// Log an Opus error
void log_opus_error(
		const std::string &func_name,
		const std::string &msg,
		int opus_error,
		bool display_msg = true);
#endif

#endif
