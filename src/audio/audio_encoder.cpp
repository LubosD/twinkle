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

#include <cassert>
#include <iostream>
#include "audio_encoder.h"

#ifdef HAVE_ILBC
#ifndef HAVE_ILBC_CPP
extern "C" {
#endif
#include <ilbc/iLBC_encode.h>
#ifndef HAVE_ILBC_CPP
}
#endif
#endif

//////////////////////////////////////////
// class t_audio_encoder
//////////////////////////////////////////

t_audio_encoder::t_audio_encoder(uint16 payload_id, uint16 ptime, t_user *user_config) :
	_payload_id(payload_id),
	_ptime(ptime),
	_user_config(user_config)
{}

t_audio_codec t_audio_encoder::get_codec(void) const {
	return _codec;
}

uint16 t_audio_encoder::get_payload_id(void) const {
	return _payload_id;
}

uint16 t_audio_encoder::get_ptime(void) const {
	return _ptime;
}

uint16 t_audio_encoder::get_sample_rate(void) const {
	return audio_sample_rate(_codec);
}

uint16 t_audio_encoder::get_sample_rate_rtp(void) const {
	return audio_sample_rate_rtp(_codec);
}

uint16 t_audio_encoder::get_sample_rate_rtp_ratio(void) const {
	return audio_sample_rate_rtp_ratio(_codec);
}

uint16 t_audio_encoder::get_max_payload_size(void) const {
	return _max_payload_size;
}


//////////////////////////////////////////
// class t_g711a_audio_encoder
//////////////////////////////////////////

t_g711a_audio_encoder::t_g711a_audio_encoder(uint16 payload_id, uint16 ptime, 
		t_user *user_config) :
	t_audio_encoder(payload_id, ptime, user_config)
{
	_codec = CODEC_G711_ALAW;
	if (ptime == 0) _ptime = PTIME_G711_ALAW;
	_max_payload_size = audio_sample_rate(_codec)/1000 * _ptime;
}

uint16 t_g711a_audio_encoder::encode(int16 *sample_buf, uint16 nsamples, 
			uint8 *payload, uint16 payload_size, bool &silence)
{
	assert(nsamples <= payload_size);
	silence = false;
	
	for (int i = 0; i < nsamples; i++) {
		payload[i] = linear2alaw(sample_buf[i]);
	}
	
	return nsamples;	
}


//////////////////////////////////////////
// class t_g711u_audio_encoder
//////////////////////////////////////////

t_g711u_audio_encoder::t_g711u_audio_encoder(uint16 payload_id, uint16 ptime, 
		t_user *user_config) :
	t_audio_encoder(payload_id, ptime, user_config)
{
	_codec = CODEC_G711_ULAW;
	if (ptime == 0) _ptime = PTIME_G711_ULAW;
	_max_payload_size = audio_sample_rate(_codec)/1000 * _ptime;
}

uint16 t_g711u_audio_encoder::encode(int16 *sample_buf, uint16 nsamples, 
			uint8 *payload, uint16 payload_size, bool &silence)
{
	assert(nsamples <= payload_size);
	silence = false;

	for (int i = 0; i < nsamples; i++) {
		payload[i] = linear2ulaw(sample_buf[i]);
	}
	
	return nsamples;	
}


//////////////////////////////////////////
// class t_gsm_audio_encoder
//////////////////////////////////////////

t_gsm_audio_encoder::t_gsm_audio_encoder(uint16 payload_id, uint16 ptime, 
		t_user *user_config) :
	t_audio_encoder(payload_id, PTIME_GSM, user_config)
{
	_codec = CODEC_GSM;
	_max_payload_size = 33;
	gsm_encoder = gsm_create();
}

t_gsm_audio_encoder::~t_gsm_audio_encoder() {
	gsm_destroy(gsm_encoder);
}

uint16 t_gsm_audio_encoder::encode(int16 *sample_buf, uint16 nsamples, 
			uint8 *payload, uint16 payload_size, bool &silence)
{
	assert(payload_size >= _max_payload_size);
	silence = false;
	gsm_encode(gsm_encoder, sample_buf, payload);
	return _max_payload_size;
}


#ifdef HAVE_SPEEX
//////////////////////////////////////////
// class t_speex_audio_encoder
//////////////////////////////////////////

t_speex_audio_encoder::t_speex_audio_encoder(uint16 payload_id, uint16 ptime, 
		t_mode mode, t_user *user_config) :
	t_audio_encoder(payload_id, PTIME_SPEEX, user_config)
{
	speex_bits_init(&speex_bits);
	_mode = mode;
	
	switch (mode) {
	case MODE_NB:
		_codec = CODEC_SPEEX_NB;
		speex_enc_state = speex_encoder_init(&speex_nb_mode);
		break;
	case MODE_WB:
		_codec = CODEC_SPEEX_WB;
		speex_enc_state = speex_encoder_init(&speex_wb_mode);
		break;
	case MODE_UWB:
		_codec = CODEC_SPEEX_UWB;
		speex_enc_state = speex_encoder_init(&speex_uwb_mode);
		break;
	default:
		assert(false);
	}
	
	int arg;
	
	// Bit rate type
	switch (_user_config->get_speex_bit_rate_type()) {
	case BIT_RATE_CBR:
		arg = 0;
		speex_encoder_ctl(speex_enc_state, SPEEX_SET_VBR, &arg);
		break;
	case BIT_RATE_VBR:
		arg = 1;
		speex_encoder_ctl(speex_enc_state, SPEEX_SET_VBR, &arg);
		break;
	case BIT_RATE_ABR:
		if (_codec == CODEC_SPEEX_NB) {
			arg = user_config->get_speex_abr_nb();
			speex_encoder_ctl(speex_enc_state, SPEEX_SET_ABR, &arg);
		} else {
			arg = user_config->get_speex_abr_wb();
			speex_encoder_ctl(speex_enc_state, SPEEX_SET_ABR, &arg);
		}
		break;
	default:
		assert(false);
	}
	
	_max_payload_size = 1500;
	
	/*** ENCODER OPTIONS ***/
	
	// Discontinuos trasmission
	arg = (_user_config->get_speex_dtx() ? 1 : 0);
	speex_encoder_ctl(speex_enc_state, SPEEX_SET_DTX, &arg);
		
	// Quality
	arg = _user_config->get_speex_quality();
	if (_user_config->get_speex_bit_rate_type() == BIT_RATE_VBR) 
	    speex_encoder_ctl(speex_enc_state, SPEEX_SET_VBR_QUALITY, &arg);
	else
	    speex_encoder_ctl(speex_enc_state, SPEEX_SET_QUALITY, &arg);

	// Complexity
	arg = _user_config->get_speex_complexity();
	speex_encoder_ctl(speex_enc_state, SPEEX_SET_COMPLEXITY, &arg);
}

t_speex_audio_encoder::~t_speex_audio_encoder() {
	speex_bits_destroy(&speex_bits);
	speex_encoder_destroy(speex_enc_state);
}

uint16 t_speex_audio_encoder::encode(int16 *sample_buf, uint16 nsamples, 
			uint8 *payload, uint16 payload_size, bool &silence)
{
	assert(payload_size >= _max_payload_size);

	silence = false;
	speex_bits_reset(&speex_bits);
            
    if (speex_encode_int(speex_enc_state, sample_buf, &speex_bits) == 0) 
		silence = true;

	return speex_bits_write(&speex_bits, (char *)payload, payload_size);
}
#endif

#ifdef HAVE_OPUS
//////////////////////////////////////////
// class t_opus_audio_encoder
//////////////////////////////////////////

t_opus_audio_encoder::t_opus_audio_encoder(uint16 payload_id, uint16 ptime,
		t_user *user_config, const t_codec_sdp_params &sdp_params) :
	t_audio_encoder(payload_id, ptime, user_config)
{
	_codec = CODEC_OPUS;
	if (_ptime == 0) {
		_ptime = PTIME_OPUS;
	}
	// Value suggested by the Opus documentation (enough to hold 60 ms of
	// a 510 kbps stream)
	_max_payload_size = 4000;

	int error;
	enc = opus_encoder_create(audio_sample_rate(_codec), 1,  // (1 = mono)
			OPUS_APPLICATION_VOIP, &error);
	if (error != OPUS_OK) {
		log_opus_error("t_opus_audio_encoder::t_opus_audio_encoder",
				"Cannot create encoder", error);
		enc = NULL;
		return;
	}

	// Encoder parameters

	// Bandwidth
	log_file->write_header("t_opus_audio_encoder::t_opus_audio_encoder", LOG_NORMAL, LOG_DEBUG);
	log_file->write_raw("Determining bandwidth\n");
	log_file->write_raw("(Values are OPUS_BANDWIDTH_* constants)\n");
	opus_int32 bandwidth = _user_config->get_opus_bandwidth();
	log_file->write_raw("Initial bandwidth = ");
	log_file->write_raw(bandwidth);
	log_file->write_endl();
	// Apply maximum bandwidth according to "maxplaybackrate" (mandated by
	// RFC 7587, s. 7.1)
	unsigned int maxplaybackrate = sdp_params.opus_maxplaybackrate;
	if (maxplaybackrate == 0) {
		maxplaybackrate = 48000;  // Default value
	}
	if (maxplaybackrate < 48000) {
		opus_int32 max_bandwidth;
		if (maxplaybackrate <= OPUS_SAMPLE_RATE_NB) {
			max_bandwidth = OPUS_BANDWIDTH_NARROWBAND;
		} else if (maxplaybackrate <= OPUS_SAMPLE_RATE_MB) {
			max_bandwidth = OPUS_BANDWIDTH_MEDIUMBAND;
		} else if (maxplaybackrate <= OPUS_SAMPLE_RATE_WB) {
			max_bandwidth = OPUS_BANDWIDTH_WIDEBAND;
		} else if (maxplaybackrate <= OPUS_SAMPLE_RATE_SWB) {
			max_bandwidth = OPUS_BANDWIDTH_SUPERWIDEBAND;
		} else {
			max_bandwidth = OPUS_BANDWIDTH_FULLBAND;
		}
		log_file->write_raw("Maximum bandwidth (");
		log_file->write_raw(maxplaybackrate);
		log_file->write_raw(") = ");
		log_file->write_raw(max_bandwidth);
		log_file->write_endl();
		// FIXME: OPUS_BANDWIDTH_* are not *guaranteed* to be ordered
		bandwidth = std::min(bandwidth, max_bandwidth);
	}
	log_file->write_raw("Final bandwidth = ");
	log_file->write_raw(bandwidth);
	log_file->write_endl();
	log_file->write_footer();
	error = opus_encoder_ctl(enc, OPUS_SET_MAX_BANDWIDTH(bandwidth));
	if (error != OPUS_OK) {
		log_opus_error("t_opus_audio_encoder::t_opus_audio_encoder",
				"Cannot set maximum bandwidth", error);
	}

	// Bitrate
	log_file->write_header("t_opus_audio_encoder::t_opus_audio_encoder", LOG_NORMAL, LOG_DEBUG);
	log_file->write_raw("Determining bitrate\n");
	opus_int32 bitrate;
	log_file->write_raw("Initial bitrate = ");
	if (_user_config->get_opus_bitrate() == 0) {
		bitrate = OPUS_AUTO;
		log_file->write_raw("auto");
	} else {
		bitrate = _user_config->get_opus_bitrate();
		log_file->write_raw(bitrate);
	}
	log_file->write_endl();
	// maxaveragebitrate dictates the maximum avg. bitrate we can use
	unsigned int maxaveragebitrate = sdp_params.opus_maxaveragebitrate;
	log_file->write_raw("Initial maxaveragebitrate = ");
	log_file->write_raw(maxaveragebitrate);
	log_file->write_endl();
	// If not provided, its value defaults to "the maximum value specified
	// in Section 3.1.1 for the corresponding mode of Opus and
	// corresponding maxplaybackrate", per RFC 7587, s. 6.1
	if (maxaveragebitrate == 0) {
		log_file->write_raw("Default maxaveragebitrate (");

		switch (_user_config->get_opus_signal_type()) {
		case OPUS_AUTO:
		case OPUS_SIGNAL_VOICE:
			log_file->write_raw("voice, ");
			if (maxplaybackrate <= OPUS_SAMPLE_RATE_NB) {
				log_file->write_raw("NB");
				// 8-12 kbit/s for NB speech
				maxaveragebitrate = 12000;
			} else if (maxplaybackrate <= OPUS_SAMPLE_RATE_WB) {
				log_file->write_raw("WB");
				// 16-20 kbit/s for WB speech
				maxaveragebitrate = 20000;
			} else {
				log_file->write_raw("FB");
				// 28-40 kbit/s for FB speech
				maxaveragebitrate = 40000;
			}
			break;
		case OPUS_SIGNAL_MUSIC:
			log_file->write_raw("music, FB");
			// 48-64 kbit/s for FB mono music
			maxaveragebitrate = 64000;
			break;
		default:
			assert(false);
		}

		log_file->write_raw(") = ");
		log_file->write_raw(maxaveragebitrate);
		log_file->write_endl();
	}
	// When bitrate is "auto", we must take care not to raise its
	// value if maxaveragebitrate happens to be higher than the
	// actual bitrate, so we convert it to a real value
	if (bitrate == OPUS_AUTO) {
		// Formula copied from the libopus source code
		const unsigned short Fs = audio_sample_rate(_codec);
		const unsigned short frame_size = Fs * ptime / 1000;
		const unsigned channels = 1;
		bitrate = 60*Fs/frame_size + Fs*channels;

		log_file->write_raw("Auto bitrate = ");
		log_file->write_raw(bitrate);
		log_file->write_endl();
	}
	bitrate = std::min(bitrate, (int)maxaveragebitrate);
	log_file->write_raw("Final bitrate = ");
	log_file->write_raw(bitrate);
	log_file->write_endl();
	log_file->write_footer();
	error = opus_encoder_ctl(enc, OPUS_SET_BITRATE(bitrate));
	if (error != OPUS_OK) {
		log_opus_error("t_opus_audio_encoder::t_opus_audio_encoder",
				"Cannot set bitrate", error);
	}

	// Complexity
	error = opus_encoder_ctl(enc, OPUS_SET_COMPLEXITY(_user_config->get_opus_complexity()));
	if (error != OPUS_OK) {
		log_opus_error("t_opus_audio_encoder::t_opus_audio_encoder",
				"Cannot set complexity", error);
	}

	// DTX
	error = opus_encoder_ctl(enc, OPUS_SET_DTX(sdp_params.opus_usedtx ? 1 : 0));
	if (error != OPUS_OK) {
		log_opus_error("t_opus_audio_encoder::t_opus_audio_encoder",
				"Cannot set DTX", error);
	}

	// FEC
	error = opus_encoder_ctl(enc, OPUS_SET_INBAND_FEC(sdp_params.opus_useinbandfec ? 1 : 0));
	if (error != OPUS_OK) {
		log_opus_error("t_opus_audio_encoder::t_opus_audio_encoder",
				"Cannot set FEC", error);
	}

	// Packet loss
	error = opus_encoder_ctl(enc, OPUS_SET_PACKET_LOSS_PERC(_user_config->get_opus_packet_loss()));
	if (error != OPUS_OK) {
		log_opus_error("t_opus_audio_encoder::t_opus_audio_encoder",
				"Cannot set packet loss", error);
	}

	// Signal type
	error = opus_encoder_ctl(enc, OPUS_SET_SIGNAL(_user_config->get_opus_signal_type()));
	if (error != OPUS_OK) {
		log_opus_error("t_opus_audio_encoder::t_opus_audio_encoder",
				"Cannot set signal type", error);
	}

	// Constant/variable bitrate
	error = opus_encoder_ctl(enc, OPUS_SET_VBR(!sdp_params.opus_cbr ? 1 : 0));
	if (error != OPUS_OK) {
		log_opus_error("t_opus_audio_encoder::t_opus_audio_encoder",
				"Cannot set VBR", error);
	}
}

t_opus_audio_encoder::~t_opus_audio_encoder() {
	if (enc) {
		opus_encoder_destroy(enc);
	}
}

uint16 t_opus_audio_encoder::encode(int16 *sample_buf, uint16 nsamples,
			uint8 *payload, uint16 payload_size, bool &silence)
{
	if (!enc) return 0;

	assert(payload_size >= _max_payload_size);

	silence = false;

	int nbytes = opus_encode(enc, sample_buf, nsamples, payload, payload_size);

	if (nbytes < 0) {
		log_opus_error("t_opus_audio_encoder::encode",
				"Error while encoding", nbytes);
		return 0;
	} else if (nbytes <= 2) {
		silence = true;  // DTX
	}

	return nbytes;
}
#endif

#ifdef HAVE_ILBC
//////////////////////////////////////////
// class t_ilbc_audio_encoder
//////////////////////////////////////////

t_ilbc_audio_encoder::t_ilbc_audio_encoder(uint16 payload_id, uint16 ptime,
		t_user *user_config) :
	t_audio_encoder(payload_id, (ptime < 25 ? 20 : 30), user_config)
{
	_codec = CODEC_ILBC;
	_mode = _ptime;
	
	if (_mode == 20) {
		_max_payload_size = NO_OF_BYTES_20MS;
	} else {
		_max_payload_size = NO_OF_BYTES_30MS;
	}
	
	initEncode(&_ilbc_encoder, _mode);
}

uint16 t_ilbc_audio_encoder::encode(int16 *sample_buf, uint16 nsamples, 
			uint8 *payload, uint16 payload_size, bool &silence)
{
	assert(payload_size >= _max_payload_size);
	assert(nsamples == _ilbc_encoder.blockl);
	
	silence = false;
	float block[nsamples];
	
	for (int i = 0; i < nsamples; i++) {
		block[i] = static_cast<float>(sample_buf[i]);
	}
	
	iLBC_encode((unsigned char*)payload, block, &_ilbc_encoder);
	
	return _ilbc_encoder.no_of_bytes;
}
#endif

//////////////////////////////////////////
// class t_g722_audio_encoder
//////////////////////////////////////////

t_g722_audio_encoder::t_g722_audio_encoder(uint16 payload_id, uint16 ptime,
		t_user *user_config) :
	t_audio_encoder(payload_id, ptime, user_config)
{
	_codec = CODEC_G722;
	if (ptime == 0) _ptime = PTIME_G722;
	_max_payload_size = audio_sample_rate(_codec)/1000 * _ptime / G722_SAMPLES_PAYLOAD_RATIO;

	_state = g722_encode_init(NULL, 64000, 0);
}

t_g722_audio_encoder::~t_g722_audio_encoder()
{
	g722_encode_release(_state);
}

uint16 t_g722_audio_encoder::encode(int16 *sample_buf, uint16 nsamples,
			uint8 *payload, uint16 payload_size, bool &silence)
{
	assert((nsamples % G722_SAMPLES_PAYLOAD_RATIO) == 0);
	assert(payload_size >= (nsamples / G722_SAMPLES_PAYLOAD_RATIO));

	silence = false;

	int nbytes = g722_encode(_state, payload, sample_buf, nsamples);

	return nbytes;
}


//////////////////////////////////////////
// class t_g726_encoder
//////////////////////////////////////////

t_g726_audio_encoder::t_g726_audio_encoder(uint16 payload_id, uint16 ptime, 
		t_bit_rate bit_rate, t_user *user_config) :
	t_audio_encoder(payload_id, ptime, user_config)
{
	_bit_rate = bit_rate;
	
	switch (bit_rate) {
	case BIT_RATE_16:
		_codec = CODEC_G726_16;
		break;
	case BIT_RATE_24:
		_codec = CODEC_G726_24;
		break;
	case BIT_RATE_32:
		_codec = CODEC_G726_32;
		break;
	case BIT_RATE_40:
		_codec = CODEC_G726_40;
		break;
	default:
		assert(false);
	}
	
	if (ptime == 0) _ptime = PTIME_G726;
	_max_payload_size = audio_sample_rate(_codec)/1000 * _ptime;
	_packing = user_config->get_g726_packing();
	
	g72x_init_state(&_state);
}

uint16 t_g726_audio_encoder::encode_16(int16 *sample_buf, uint16 nsamples, 
			uint8 *payload, uint16 payload_size)
{
	assert(nsamples % 4 == 0);
	assert(nsamples / 4 <= payload_size);
	
	for (int i = 0; i < nsamples; i += 4) {
		payload[i >> 2] = 0;
		for (int j = 0; j < 4; j++) {
			uint8 v = static_cast<uint8>(g723_16_encoder(sample_buf[i+j],
				AUDIO_ENCODING_LINEAR, &_state));
				
			if (_packing == G726_PACK_RFC3551) {
				payload[i >> 2] |= v << (j * 2);
			} else {
				payload[i >> 2] |= v << ((3-j) * 2);
			}
		}
	}
	
	return nsamples >> 2;
}

uint16 t_g726_audio_encoder::encode_24(int16 *sample_buf, uint16 nsamples, 
			uint8 *payload, uint16 payload_size)
{
	assert(nsamples % 8 == 0);
	assert(nsamples / 8 * 3 <= payload_size);

	for (int i = 0; i < nsamples; i += 8) {
		uint32 v = 0;
		for (int j = 0; j < 8; j++) {
			if (_packing == G726_PACK_RFC3551) {
				v |= static_cast<uint32>(g723_24_encoder(sample_buf[i+j],
					AUDIO_ENCODING_LINEAR, &_state)) << (j * 3);
			} else {
				v |= static_cast<uint32>(g723_24_encoder(sample_buf[i+j],
					AUDIO_ENCODING_LINEAR, &_state)) << ((7-j) * 3);
			}
		}
		payload[(i >> 3) * 3] = static_cast<uint8>(v & 0xff);
		payload[(i >> 3) * 3 + 1] = static_cast<uint8>((v >> 8) & 0xff);
		payload[(i >> 3) * 3 + 2] = static_cast<uint8>((v >> 16) & 0xff);
	}
	
	return (nsamples >> 3) * 3;
}

uint16 t_g726_audio_encoder::encode_32(int16 *sample_buf, uint16 nsamples, 
			uint8 *payload, uint16 payload_size)
{
	assert(nsamples % 2 == 0);
	assert(nsamples / 2 <= payload_size);

	for (int i = 0; i < nsamples; i += 2) {
		payload[i >> 1] = 0;
		for (int j = 0; j < 2; j++) {
			uint8 v = static_cast<uint8>(g721_encoder(sample_buf[i+j],
				AUDIO_ENCODING_LINEAR, &_state));
				
			if (_packing == G726_PACK_RFC3551) {
				payload[i >> 1] |= v << (j * 4);
			} else {
				payload[i >> 1] |= v << ((1-j) * 4);
			}
		}
	}
	
	return nsamples >> 1;
}

uint16 t_g726_audio_encoder::encode_40(int16 *sample_buf, uint16 nsamples, 
			uint8 *payload, uint16 payload_size)
{
	assert(nsamples % 8 == 0);
	assert(nsamples / 8 * 5 <= payload_size);

	for (int i = 0; i < nsamples; i += 8) {
		uint64 v = 0;
		for (int j = 0; j < 8; j++) {
			if (_packing == G726_PACK_RFC3551) {
				v |= static_cast<uint64>(g723_40_encoder(sample_buf[i+j],
					AUDIO_ENCODING_LINEAR, &_state)) << (j * 5);
			} else {
				v |= static_cast<uint64>(g723_40_encoder(sample_buf[i+j],
					AUDIO_ENCODING_LINEAR, &_state)) << ((7-j) * 5);
			}
		}
		payload[(i >> 3) * 5] = static_cast<uint8>(v & 0xff);
		payload[(i >> 3) * 5 + 1] = static_cast<uint8>((v >> 8) & 0xff);
		payload[(i >> 3) * 5 + 2] = static_cast<uint8>((v >> 16) & 0xff);
		payload[(i >> 3) * 5 + 3] = static_cast<uint8>((v >> 24) & 0xff);
		payload[(i >> 3) * 5 + 4] = static_cast<uint8>((v >> 32) & 0xff);
	}
	
	return (nsamples >> 3) * 5;
}

uint16 t_g726_audio_encoder::encode(int16 *sample_buf, uint16 nsamples, 
			uint8 *payload, uint16 payload_size, bool &silence)
{
	silence = false;
	
	switch (_bit_rate) {
	case BIT_RATE_16:
		return encode_16(sample_buf, nsamples, payload, payload_size);
	case BIT_RATE_24:
		return encode_24(sample_buf, nsamples, payload, payload_size);
	case BIT_RATE_32:
		return encode_32(sample_buf, nsamples, payload, payload_size);
	case BIT_RATE_40:
		return encode_40(sample_buf, nsamples, payload, payload_size);
	default:
		assert(false);
	}
	
	return 0;
}

#ifdef HAVE_BCG729

t_g729a_audio_encoder::t_g729a_audio_encoder(uint16 payload_id, uint16 ptime, t_user *user_config)
	: t_audio_encoder(payload_id, ptime, user_config)
{
#ifdef HAVE_BCG729_ANNEX_B
	_context = initBcg729EncoderChannel(false);
#else
	_context = initBcg729EncoderChannel();
#endif
}

t_g729a_audio_encoder::~t_g729a_audio_encoder()
{
	closeBcg729EncoderChannel(_context);
}

uint16 t_g729a_audio_encoder::encode(int16 *sample_buf, uint16 nsamples,
		uint8 *payload, uint16 payload_size, bool &silence)
{
	assert ((nsamples % 80) == 0);
	assert (payload_size >= (nsamples/8));

	silence = false;

	for (uint16 done = 0; done < nsamples; done += 80)
	{
#ifdef HAVE_BCG729_ANNEX_B
		uint8 frame_size = 10;
		bcg729Encoder(_context, &sample_buf[done], &payload[done / 8], &frame_size);
		assert(frame_size == 10);
#else
		bcg729Encoder(_context, &sample_buf[done], &payload[done / 8]);
#endif
	}

	return nsamples / 8;
}

#endif
