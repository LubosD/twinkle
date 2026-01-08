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

#include <cstdlib>
#include <sstream>
#include "audio_codecs.h"
#include "userintf.h"

unsigned short audio_sample_rate(t_audio_codec codec) {
	switch(codec) {
	case CODEC_G711_ALAW:
	case CODEC_G711_ULAW:
	case CODEC_GSM:
	case CODEC_SPEEX_NB:
	case CODEC_ILBC:
	case CODEC_G729A:
	case CODEC_G726_16:
	case CODEC_G726_24:
	case CODEC_G726_32:
	case CODEC_G726_40:
	case CODEC_TELEPHONE_EVENT:
		return 8000;
	case CODEC_G722:
	case CODEC_SPEEX_WB:
		return 16000;
	case CODEC_SPEEX_UWB:
		return 32000;
	case CODEC_OPUS:
		return 48000;
	default:
		// Use 8000 as default rate
		return 8000;
	}
}

unsigned short audio_sample_rate_rtp(t_audio_codec codec) {
	switch(codec) {
	case CODEC_G722:
		// RFC 3551 (s. 4.5.2) requires the RTP clock rate to be 8 kHz
		return 8000;
	default:
		return audio_sample_rate(codec);
	}
}

unsigned short audio_sample_rate_rtp_ratio(t_audio_codec codec) {
	return audio_sample_rate(codec) / audio_sample_rate_rtp(codec);
}

bool is_speex_codec(t_audio_codec codec) {
	return (codec == CODEC_SPEEX_NB ||
		codec == CODEC_SPEEX_WB ||
		codec == CODEC_SPEEX_UWB);
}

int resample(short *input_buf, int input_len, int input_sample_rate,
	short *output_buf, int output_len, int output_sample_rate)
{
	if (input_sample_rate > output_sample_rate) {
		int downsample_factor = input_sample_rate / output_sample_rate;
		int output_idx = -1;
		for (int i = 0; i < input_len; i += downsample_factor) {
			output_idx = i / downsample_factor;
			if (output_idx >= output_len) {
				// Output buffer is full
				return output_len;
			}
			output_buf[output_idx] = input_buf[i];
		}
		return output_idx + 1;
	} else {
		int upsample_factor = output_sample_rate / input_sample_rate;
		int output_idx = -1;
		for (int i = 0; i < input_len; i++) {
			for (int j = 0; j < upsample_factor; j++) {
				output_idx = i * upsample_factor + j;
				if (output_idx >= output_len) {
					// Output buffer is full
					return output_len;
				}
				output_buf[output_idx] = input_buf[i];
			}
		}
		return output_idx + 1;
	}
}

short mix_linear_pcm(short pcm1, short pcm2) {
	long mixed_sample = long(pcm1) + long(pcm2);

	// Compress a 17 bit PCM value into a 16-bit value.
	// The easy way is to divide the value by 2, but this lowers
	// the volume.
	// Only lower the volume for the loud values. As for a normal
	// voice call the values are not that loud, this gives better
	// quality.
	if (mixed_sample > 16384) {
		mixed_sample = 16384 + (mixed_sample - 16384) / 3;
	} else if (mixed_sample < -16384) {
		mixed_sample = -16384 - (-16384 - mixed_sample) / 3;
	}

	return short(mixed_sample);
}

#ifdef HAVE_OPUS
unsigned short opus_adjusted_ptime(unsigned short ptime) {
	if (ptime <= 10) {
		return 10;
	} else if (ptime <= 20) {
		return 20;
	} else if (ptime <= 40) {
		return 40;
	} else if (ptime <= 60) {
		return 60;
	} else {
		// Maximum duration of an Opus frame
		// (Although libopus v1.2+ allows for values up to 120 ms, it
		// merely encodes multiple frames into a single Opus packet)
		return 60;
	}
}

void log_opus_error(
		const std::string &func_name,
		const std::string &msg,
		int opus_error,
		bool display_msg)
{
	std::stringstream ss;
	ss << "Opus error: " << msg << ": " << opus_strerror(opus_error);
	std::string s = ss.str();

	log_file->write_report(s, func_name, LOG_NORMAL, LOG_CRITICAL);
	if (display_msg) {
		ui->cb_display_msg(s, MSG_CRITICAL);
	}
}
#endif
