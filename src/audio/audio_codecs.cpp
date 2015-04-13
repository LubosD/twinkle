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
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <cstdlib>
#include "audio_codecs.h"

unsigned short audio_sample_rate(t_audio_codec codec) {
	switch(codec) {
	case CODEC_G711_ALAW:
	case CODEC_G711_ULAW:
	case CODEC_GSM:
	case CODEC_SPEEX_NB:
	case CODEC_ILBC:
	case CODEC_G726_16:
	case CODEC_G726_24:
	case CODEC_G726_32:
	case CODEC_G726_40:
	case CODEC_TELEPHONE_EVENT:
		return 8000;
	case CODEC_SPEEX_WB:
		return 16000;
	case CODEC_SPEEX_UWB:
		return 32000;
	default:
		// Use 8000 as default rate
		return 8000;
	}
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
