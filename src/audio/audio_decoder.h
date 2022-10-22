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

// Audio decoders

#ifndef _AUDIO_DECODER_H
#define _AUDIO_DECODER_H

#include "twinkle_config.h"
#include "audio_codecs.h"
#include "user.h"

#ifdef HAVE_GSM
extern "C" {
#ifdef HAVE_GSM_GSM_H
#include <gsm/gsm.h>
#else
#include <gsm.h>
#endif
}
#else
#include "gsm/inc/gsm.h"
#endif

#ifdef HAVE_SPEEX
#include <speex/speex.h>
#include <speex/speex_preprocess.h> 
#endif

extern "C" {
#	include "g722.h"
#	include "g722_local.h"
}

#ifdef HAVE_BCG729
extern "C" {
#	include <bcg729/decoder.h>
}
#endif

#ifdef HAVE_ILBC
#ifndef HAVE_ILBC_CPP
extern "C" {
#endif
#include <ilbc/iLBC_define.h>
#ifndef HAVE_ILBC_CPP
}
#endif
#endif

// Abstract definition of an audio decoder
class t_audio_decoder {
protected:
	t_audio_codec	_codec;
	uint16		_default_ptime;
	bool		_plc; // packet loss concealment
	t_user		*_user_config;

	t_audio_decoder(uint16 default_ptime, bool plc, t_user *user_config);
	
public:
	virtual ~t_audio_decoder() {};
	
	t_audio_codec get_codec(void) const;
	uint16 get_default_ptime(void) const;
	virtual uint16 get_ptime(uint16 payload_size) const = 0;
	
	// Decode a buffer of encoded samples to 16-bit PCM.
	// Returns the number of pcm samples written into pcm_buf
	// Returns 0 if decoding failed
	virtual uint16 decode(uint8 *payload, uint16 payload_size,
			int16 *pcm_buf, uint16 pcm_buf_size) = 0;
			
	// Indicates if codec has PLC algorithm
	bool has_plc(void) const;
	
	// Create a payload to conceal a lost packet.
	// Returns the number of pcm samples written into pcm_buf
	// Returns 0 if decoding failed
	virtual uint16 conceal(int16 *pcm_buf, uint16 pcm_buf_size);
	
	// Determine if the payload size is valid for this decoder
	virtual bool valid_payload_size(uint16 payload_size, uint16 sample_buf_size) const = 0;
};

// G.711 A-law
class t_g711a_audio_decoder : public t_audio_decoder {
public:
	t_g711a_audio_decoder(uint16 default_ptime, t_user *user_config);
	
	virtual uint16 get_ptime(uint16 payload_size) const;
	virtual uint16 decode(uint8 *payload, uint16 payload_size,
			int16 *pcm_buf, uint16 pcm_buf_size);
	virtual bool valid_payload_size(uint16 payload_size, uint16 sample_buf_size) const;
};

// G.711 u-law
class t_g711u_audio_decoder : public t_audio_decoder {
public:
	t_g711u_audio_decoder(uint16 default_ptime, t_user *user_config);
	
	virtual uint16 get_ptime(uint16 payload_size) const;
	virtual uint16 decode(uint8 *payload, uint16 payload_size,
			int16 *pcm_buf, uint16 pcm_buf_size);
	virtual bool valid_payload_size(uint16 payload_size, uint16 sample_buf_size) const;
};

// GSM
class t_gsm_audio_decoder : public t_audio_decoder {
private:
	gsm		gsm_decoder;
	
public:
	t_gsm_audio_decoder(t_user *user_config);
	virtual ~t_gsm_audio_decoder();
	
	virtual uint16 get_ptime(uint16 payload_size) const;
	virtual uint16 decode(uint8 *payload, uint16 payload_size,
			int16 *pcm_buf, uint16 pcm_buf_size);
	virtual bool valid_payload_size(uint16 payload_size, uint16 sample_buf_size) const;
};

#ifdef HAVE_SPEEX
// Speex
class t_speex_audio_decoder : public t_audio_decoder {
public:
	enum t_mode {
		MODE_NB,	// Narrow band
		MODE_WB,	// Wide band
		MODE_UWB	// Ultra wide band
	};
	
private:
	SpeexBits	speex_bits;
	void		*speex_dec_state;
	t_mode		_mode;
	
public:
	t_speex_audio_decoder(t_mode mode, t_user *user_config);
	virtual ~t_speex_audio_decoder();
	
	virtual uint16 get_ptime(uint16 payload_size) const;
	virtual uint16 decode(uint8 *payload, uint16 payload_size,
			int16 *pcm_buf, uint16 pcm_buf_size);
	virtual uint16 conceal(int16 *pcm_buf, uint16 pcm_buf_size);
	virtual bool valid_payload_size(uint16 payload_size, uint16 sample_buf_size) const;
};
#endif

#ifdef HAVE_ILBC
// iLBC
class t_ilbc_audio_decoder : public t_audio_decoder {
private:
	iLBC_Dec_Inst_t	_ilbc_decoder_20; // decoder for 20ms frames
	iLBC_Dec_Inst_t	_ilbc_decoder_30; // decoder for 30ms frames
	
	// The number of ms received in the last frame, so the conceal function
	// can determine which decoder to use to conceal a lost frame.
	int		_last_received_ptime;
	
public:
	t_ilbc_audio_decoder(uint16 default_ptime, t_user *user_config);
	
	virtual uint16 get_ptime(uint16 payload_size) const;
	virtual uint16 decode(uint8 *payload, uint16 payload_size,
			int16 *pcm_buf, uint16 pcm_buf_size);
	virtual uint16 conceal(int16 *pcm_buf, uint16 pcm_buf_size);
	virtual bool valid_payload_size(uint16 payload_size, uint16 sample_buf_size) const;
};

#endif

// G.722
class t_g722_audio_decoder : public t_audio_decoder {
private:
	g722_decode_state_t *_state;

public:
	t_g722_audio_decoder(uint16 default_ptime, t_user *user_config);
	~t_g722_audio_decoder();

	virtual uint16 get_ptime(uint16 payload_size) const;
	virtual uint16 decode(uint8 *payload, uint16 payload_size,
			int16 *pcm_buf, uint16 pcm_buf_size);
	virtual bool valid_payload_size(uint16 payload_size, uint16 sample_buf_size) const;
};

// G.726
class t_g726_audio_decoder : public t_audio_decoder {
public:
	enum t_bit_rate {
		BIT_RATE_16,
		BIT_RATE_24,
		BIT_RATE_32,
		BIT_RATE_40
	};
	
private:
	uint16 decode_16(uint8 *payload, uint16 payload_size, int16 *pcm_buf, uint16 pcm_buf_size);
	uint16 decode_24(uint8 *payload, uint16 payload_size, int16 *pcm_buf, uint16 pcm_buf_size);
	uint16 decode_32(uint8 *payload, uint16 payload_size, int16 *pcm_buf, uint16 pcm_buf_size);
	uint16 decode_40(uint8 *payload, uint16 payload_size, int16 *pcm_buf, uint16 pcm_buf_size);

	struct g72x_state	_state;
	t_bit_rate		_bit_rate;
	uint8			_bits_per_sample;
	t_g726_packing		_packing;
	
public:
	t_g726_audio_decoder(t_bit_rate bit_rate, uint16 default_ptime, t_user *user_config);
	virtual uint16 get_ptime(uint16 payload_size) const;
	virtual uint16 decode(uint8 *payload, uint16 payload_size,
			int16 *pcm_buf, uint16 pcm_buf_size);
	virtual bool valid_payload_size(uint16 payload_size, uint16 sample_buf_size) const;
};

#ifdef HAVE_BCG729

// G.729A
class t_g729a_audio_decoder : public t_audio_decoder {
public:
	t_g729a_audio_decoder(uint16 default_ptime, t_user *user_config);
	~t_g729a_audio_decoder();

	virtual uint16 get_ptime(uint16 payload_size) const override;
	virtual uint16 decode(uint8 *payload, uint16 payload_size,
			int16 *pcm_buf, uint16 pcm_buf_size) override;
	virtual bool valid_payload_size(uint16 payload_size, uint16 sample_buf_size) const override;
	virtual uint16 conceal(int16 *pcm_buf, uint16 pcm_buf_size) override;
private:
	bcg729DecoderChannelContextStruct* _context;
};

#endif

#endif
