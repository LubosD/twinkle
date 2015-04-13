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

#include <cassert>
#include <iostream>
#include "audio_decoder.h"
#include "log.h"

#ifdef HAVE_ILBC
#ifndef HAVE_ILBC_CPP
extern "C" {
#endif
#include <ilbc/iLBC_decode.h>
#ifndef HAVE_ILBC_CPP
}
#endif
#endif

//////////////////////////////////////////
// class t_audio_decoder
//////////////////////////////////////////

t_audio_decoder::t_audio_decoder(uint16 default_ptime, bool plc, t_user *user_config) :
	_default_ptime(default_ptime),
	_plc(plc),
	_user_config(user_config)
{}

t_audio_codec t_audio_decoder::get_codec(void) const {
	return _codec;
}

uint16 t_audio_decoder::get_default_ptime(void) const {
	return _default_ptime;
}

bool t_audio_decoder::has_plc(void) const {
	return _plc;
}

uint16 t_audio_decoder::conceal(int16 *pcm_buf, uint16 pcm_buf_size) {
	return 0;
}

//////////////////////////////////////////
// class t_g711a_audio_decoder
//////////////////////////////////////////

t_g711a_audio_decoder::t_g711a_audio_decoder(uint16 default_ptime, t_user *user_config) :
	t_audio_decoder(default_ptime, false, user_config)
{
	_codec = CODEC_G711_ALAW;
	
	if (default_ptime == 0) {
		_default_ptime = PTIME_G711_ALAW;
	}
}

uint16 t_g711a_audio_decoder::get_ptime(uint16 payload_size) const {
	return payload_size / (audio_sample_rate(_codec) / 1000);
}

uint16 t_g711a_audio_decoder::decode(uint8 *payload, uint16 payload_size,
		int16 *pcm_buf, uint16 pcm_buf_size)
{
	assert(pcm_buf_size >= payload_size);

	for (int i = 0; i < payload_size; i++) {
		pcm_buf[i] = alaw2linear(payload[i]);
	}
	return payload_size;
}

bool t_g711a_audio_decoder::valid_payload_size(uint16 payload_size, uint16 sample_buf_size) const
{
	return payload_size <= sample_buf_size;
}

//////////////////////////////////////////
// class t_g711u_audio_decoder
//////////////////////////////////////////

t_g711u_audio_decoder::t_g711u_audio_decoder(uint16 default_ptime, t_user *user_config) :
	t_audio_decoder(default_ptime, false, user_config)
{
	_codec = CODEC_G711_ULAW;
	
	if (default_ptime == 0) {
		_default_ptime = PTIME_G711_ULAW;
	}
}

uint16 t_g711u_audio_decoder::get_ptime(uint16 payload_size) const {
	return payload_size / (audio_sample_rate(_codec) / 1000);
}

uint16 t_g711u_audio_decoder::decode(uint8 *payload, uint16 payload_size,
		int16 *pcm_buf, uint16 pcm_buf_size)
{
	assert(pcm_buf_size >= payload_size);
	
	for (int i = 0; i < payload_size; i++) {
		pcm_buf[i] = ulaw2linear(payload[i]);
	}
	return payload_size;
}

bool t_g711u_audio_decoder::valid_payload_size(uint16 payload_size, uint16 sample_buf_size) const
{
	return payload_size <= sample_buf_size;
}

//////////////////////////////////////////
// class t_gsm_audio_decoder
//////////////////////////////////////////

t_gsm_audio_decoder::t_gsm_audio_decoder(t_user *user_config) :
	t_audio_decoder(PTIME_GSM, false, user_config)
{
	_codec = CODEC_GSM;
	gsm_decoder = gsm_create();
}

t_gsm_audio_decoder::~t_gsm_audio_decoder() {
	gsm_destroy(gsm_decoder);
}

uint16 t_gsm_audio_decoder::get_ptime(uint16 payload_size) const {
	return get_default_ptime();
}

uint16 t_gsm_audio_decoder::decode(uint8 *payload, uint16 payload_size,
		int16 *pcm_buf, uint16 pcm_buf_size)
{
	assert(pcm_buf_size >= 160);
	
	gsm_decode(gsm_decoder, payload, pcm_buf);
	return 160;
}

bool t_gsm_audio_decoder::valid_payload_size(uint16 payload_size, uint16 sample_buf_size) const
{
	return payload_size == 33;
}

#ifdef HAVE_SPEEX
//////////////////////////////////////////
// class t_speex_audio_decoder
//////////////////////////////////////////

t_speex_audio_decoder::t_speex_audio_decoder(t_mode mode, t_user *user_config) :
	t_audio_decoder(0, true, user_config)
{
	speex_bits_init(&speex_bits);
	_mode = mode;
	
	switch (mode) {
	case MODE_NB:
		_codec = CODEC_SPEEX_NB;
		speex_dec_state = speex_decoder_init(&speex_nb_mode);
		break;
	case MODE_WB:
		_codec = CODEC_SPEEX_WB;
		speex_dec_state = speex_decoder_init(&speex_wb_mode);
		break;
	case MODE_UWB:
		_codec = CODEC_SPEEX_UWB;
		speex_dec_state = speex_decoder_init(&speex_uwb_mode);
		break;
	default:
		assert(false);
	}
	
    int frame_size = 0;
    speex_decoder_ctl(speex_dec_state, SPEEX_GET_FRAME_SIZE, &frame_size);

	// Initialize decoder with user settings
	int arg = (user_config->get_speex_penh() ? 1 : 0);
	speex_decoder_ctl(speex_dec_state, SPEEX_SET_ENH, &arg);

    _default_ptime = frame_size / (audio_sample_rate(_codec) / 1000);
}

t_speex_audio_decoder::~t_speex_audio_decoder() {
	speex_bits_destroy(&speex_bits);
	speex_decoder_destroy(speex_dec_state);
}

uint16 t_speex_audio_decoder::get_ptime(uint16 payload_size) const {
	return get_default_ptime();
}

uint16 t_speex_audio_decoder::decode(uint8 *payload, uint16 payload_size,
		int16 *pcm_buf, uint16 pcm_buf_size)
{
	int retval;
	int speex_frame_size;
	
	speex_decoder_ctl(speex_dec_state, SPEEX_GET_FRAME_SIZE, 
			&speex_frame_size);
			
	assert(pcm_buf_size >= speex_frame_size);
	
	speex_bits_read_from(&speex_bits, reinterpret_cast<char *>(payload), payload_size);
	retval = speex_decode_int(speex_dec_state, &speex_bits, pcm_buf);
	
	if (retval < 0) {
		LOG_SPEEX_ERROR("t_speex_audio_decoder::decode", 
			"speex_decode_int", retval);
		return 0;
	}
	
	return speex_frame_size;
}

uint16 t_speex_audio_decoder::conceal(int16 *pcm_buf, uint16 pcm_buf_size) {
	int retval;
	int speex_frame_size;
	
	speex_decoder_ctl(speex_dec_state, SPEEX_GET_FRAME_SIZE, 
		&speex_frame_size);
		
	assert(pcm_buf_size >= speex_frame_size);
		
	retval = speex_decode_int(speex_dec_state, NULL, pcm_buf);
	
	if (retval < 0) {
		LOG_SPEEX_ERROR("t_speex_audio_decoder::conceal", 
			"speex_decode_int", retval);
		return 0;
	}
	
	return speex_frame_size;
}

bool t_speex_audio_decoder::valid_payload_size(uint16 payload_size, uint16 sample_buf_size) const
{
	return true;
}
#endif

#ifdef HAVE_ILBC
//////////////////////////////////////////
// class t_ilbc_audio_decoder
//////////////////////////////////////////
t_ilbc_audio_decoder::t_ilbc_audio_decoder(uint16 default_ptime, t_user *user_config) :
	t_audio_decoder(default_ptime, true, user_config)
{
	_codec = CODEC_ILBC;
	_last_received_ptime = 0;
	initDecode(&_ilbc_decoder_20, 20, 1);
	initDecode(&_ilbc_decoder_30, 30, 1);
}

uint16 t_ilbc_audio_decoder::get_ptime(uint16 payload_size) const {
	if (payload_size == NO_OF_BYTES_20MS) {
		return 20;
	} else {
		return 30;
	}
}

uint16 t_ilbc_audio_decoder::decode(uint8 *payload, uint16 payload_size,
		int16 *pcm_buf, uint16 pcm_buf_size)
{
	float sample;
	float block[BLOCKL_MAX];
	int block_len;
	
	if (get_ptime(payload_size) == 20) {
		block_len = BLOCKL_20MS;
		assert(pcm_buf_size >= block_len);
		iLBC_decode(block, (unsigned char*)payload, &_ilbc_decoder_20, 1);
		_last_received_ptime = 20;
	} else {
		block_len = BLOCKL_30MS;
		assert(pcm_buf_size >= block_len);
		iLBC_decode(block, (unsigned char*)payload, &_ilbc_decoder_30, 1);
		_last_received_ptime = 30;
	}
	
	for (int i = 0; i < block_len; i++) {
		sample = block[i];
		
		if (sample < MIN_SAMPLE) sample = MIN_SAMPLE;
		if (sample > MAX_SAMPLE) sample = MAX_SAMPLE;
		
		pcm_buf[i] = static_cast<int16>(sample);
	}

	return block_len;
}

uint16 t_ilbc_audio_decoder::conceal(int16 *pcm_buf, uint16 pcm_buf_size) {
	float sample;
	float block[BLOCKL_MAX];
	int block_len;
	
	if (_last_received_ptime == 0) return 0;
	
	if (_last_received_ptime == 20) {
		block_len = BLOCKL_20MS;
		assert(pcm_buf_size >= block_len);
		iLBC_decode(block, NULL, &_ilbc_decoder_20, 0);
	} else {
		block_len = BLOCKL_30MS;
		assert(pcm_buf_size >= block_len);
		iLBC_decode(block, NULL, &_ilbc_decoder_30, 0);
	}
	
	for (int i = 0; i < block_len; i++) {
		sample = block[i];
		
		if (sample < MIN_SAMPLE) sample = MIN_SAMPLE;
		if (sample > MAX_SAMPLE) sample = MAX_SAMPLE;
		
		pcm_buf[i] = static_cast<int16>(sample);
	}
	
	return block_len;
}

bool t_ilbc_audio_decoder::valid_payload_size(uint16 payload_size, uint16 sample_buf_size) const
{
	return payload_size == NO_OF_BYTES_20MS || payload_size == NO_OF_BYTES_30MS;
}
#endif

//////////////////////////////////////////
// class t_g726_audio_decoder
//////////////////////////////////////////
t_g726_audio_decoder::t_g726_audio_decoder(t_bit_rate bit_rate, uint16 default_ptime, 
		t_user *user_config) :
	t_audio_decoder(default_ptime, false, user_config)
{
	_bit_rate = bit_rate;
	
	if (default_ptime == 0) {
		_default_ptime = PTIME_G726;
	}
	
	switch (_bit_rate) {
	case BIT_RATE_16:
		_codec = CODEC_G726_16;
		_bits_per_sample = 2;
		break;
	case BIT_RATE_24:
		_codec = CODEC_G726_24;
		_bits_per_sample = 3;
		break;
	case BIT_RATE_32:
		_codec = CODEC_G726_32;
		_bits_per_sample = 4;
		break;
	case BIT_RATE_40:
		_codec = CODEC_G726_40;
		_bits_per_sample = 5;
		break;
	default:
		assert(false);
	}
	
	_packing = user_config->get_g726_packing();
	
	g72x_init_state(&_state);
}

uint16 t_g726_audio_decoder::get_ptime(uint16 payload_size) const {
	return (payload_size * 8 / _bits_per_sample) / (audio_sample_rate(_codec) / 1000);
}

uint16 t_g726_audio_decoder::decode_16(uint8 *payload, uint16 payload_size,
			int16 *pcm_buf, uint16 pcm_buf_size)
{
	assert(payload_size * 4 <= pcm_buf_size);

	for (int i = 0; i < payload_size; i++) {
		for (int j = 0; j < 4; j++) {
			uint8 w;
			if (_packing == G726_PACK_RFC3551) {
				w = (payload[i] >> (j*2)) & 0x3;
			} else {
				w = (payload[i] >> ((3-j)*2)) & 0x3;
			}
			pcm_buf[4*i+j] = g723_16_decoder(
				w, AUDIO_ENCODING_LINEAR, &_state);
		}
	}
	
	return payload_size * 4;
}

uint16 t_g726_audio_decoder::decode_24(uint8 *payload, uint16 payload_size,
			int16 *pcm_buf, uint16 pcm_buf_size)
{
	assert(payload_size % 3 == 0);
	assert(payload_size * 8 / 3 <= pcm_buf_size);

	for (int i = 0; i < payload_size; i += 3) {
		uint32 v = (static_cast<uint32>(payload[i+2]) << 16) |
			   (static_cast<uint32>(payload[i+1]) << 8) |
			    static_cast<uint32>(payload[i]);
			     
		for (int j = 0; j < 8; j++) {
			uint8 w;
			if (_packing == G726_PACK_RFC3551) {
				w = (v >> (j*3)) & 0x7;
			} else {
				w = (v >> ((7-j)*3)) & 0x7;
			}
			pcm_buf[8*(i/3)+j] = g723_24_decoder(
				w, AUDIO_ENCODING_LINEAR, &_state);
		}
	}

	return payload_size * 8 / 3;
}

uint16 t_g726_audio_decoder::decode_32(uint8 *payload, uint16 payload_size,
			int16 *pcm_buf, uint16 pcm_buf_size)
{
	assert(payload_size * 2 <= pcm_buf_size);

	for (int i = 0; i < payload_size; i++) {
		for (int j = 0; j < 2; j++) {
			uint8 w;
			if (_packing == G726_PACK_RFC3551) {
				w = (payload[i] >> (j*4)) & 0xf;
			} else {
				w = (payload[i] >> ((1-j)*4)) & 0xf;
			}
			pcm_buf[2*i+j] = g721_decoder(
				w, AUDIO_ENCODING_LINEAR, &_state);
		}
	}
	
	return payload_size * 2;
}

uint16 t_g726_audio_decoder::decode_40(uint8 *payload, uint16 payload_size,
			int16 *pcm_buf, uint16 pcm_buf_size)
{
	assert(payload_size % 5 == 0);
	assert(payload_size * 8 / 5 <= pcm_buf_size);

	for (int i = 0; i < payload_size; i += 5) {
		uint64 v = (static_cast<uint64>(payload[i+4]) << 32) |
			   (static_cast<uint64>(payload[i+3]) << 24) |
		           (static_cast<uint64>(payload[i+2]) << 16) |
			   (static_cast<uint64>(payload[i+1]) << 8) |
			    static_cast<uint64>(payload[i]);
			     
		for (int j = 0; j < 8; j++) {
			uint8 w;
			if (_packing == G726_PACK_RFC3551) {
				w = (v >> (j*5)) & 0x1f;
			} else {
				w = (v >> ((7-j)*5)) & 0x1f;
			}
			pcm_buf[8*(i/5)+j] = g723_40_decoder(
				w, AUDIO_ENCODING_LINEAR, &_state);
		}
	}

	return payload_size * 8 / 5;
}

uint16 t_g726_audio_decoder::decode(uint8 *payload, uint16 payload_size,
			int16 *pcm_buf, uint16 pcm_buf_size)
{
	switch (_bit_rate) {
	case BIT_RATE_16:
		return decode_16(payload, payload_size, pcm_buf, pcm_buf_size);
		break;
	case BIT_RATE_24:
		return decode_24(payload, payload_size, pcm_buf, pcm_buf_size);
		break;
	case BIT_RATE_32:
		return decode_32(payload, payload_size, pcm_buf, pcm_buf_size);
		break;
	case BIT_RATE_40:
		return decode_40(payload, payload_size, pcm_buf, pcm_buf_size);
		break;
	default:
		assert(false);
	}
	
	return 0;
}

bool t_g726_audio_decoder::valid_payload_size(uint16 payload_size, 
		uint16 sample_buf_size) const
{
	switch (_bit_rate) {
	case BIT_RATE_24:
		// Payload size must be multiple of 3
		if (payload_size % 3 != 0) return false;
		break;
	case BIT_RATE_40:
		// Payload size must be multiple of 5
		if (payload_size % 5 != 0) return false;
		break;
	default:
		break;
	}
	
	return (payload_size * 8 / _bits_per_sample ) <= sample_buf_size;
}
