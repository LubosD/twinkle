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

// Classes to generate RTP payloads for DTMF tones.

#ifndef _DTMF_PLAYER_H
#define _DTMF_PLAYER_H

#include <cc++/config.h>
#include "twinkle_config.h"
#include "audio_encoder.h"
#include "freq_gen.h"
#include "user.h"

// Forward declarations
class t_audio_rx;

// Abstract class defintion for DTMF player
class t_dtmf_player {
protected:
	// Audio receiver owning the DTMF player.
	t_audio_rx	*_audio_rx;
	
	t_user		*_user_config;
	t_audio_encoder	*_audio_encoder;

	// Settings for current DTMF tone
	bool		_dtmf_pause;      // indicates if playing is in the pause phase
	bool		_dtmf_stop;	  // indicates if DTMF should be stopped
	uint8		_dtmf_current;    // Currently played DTMF tone
	uint32		_dtmf_timestamp;  // RTP timestamp (start of tone)
	uint16		_dtmf_duration;   // Duration of the tone currently played
	uint16		_nsamples; 	  // number of samples taken per packet
	
public:
	t_dtmf_player(t_audio_rx *audio_rx, t_audio_encoder *audio_encoder,
		t_user *user_config, uint8 dtmf_tone, uint32 dtmf_timestamp,
		uint16 nsamples);
		
	virtual ~t_dtmf_player() {};
	
	// Get payload for the DTMF tone
	// rtp_timestamp will be set with the timestamp value to be put in the
	// RTP header
	// Returns the payload size
	virtual uint16 get_payload(uint8 *payload, uint16 payload_size,
		uint32 timestamp, uint32 &rtp_timestamp) = 0;
	
	uint32 get_timestamp(void);
	
	// Returns true when last payload has been delivered.
	bool finished(void);
};


// DTMF player for RFC 2833 RTP telephone events
class t_rtp_event_dtmf_player : public t_dtmf_player {
public:
	t_rtp_event_dtmf_player(t_audio_rx *audio_rx, t_audio_encoder *audio_encoder,
		t_user *user_config, uint8 dtmf_tone, uint32 dtmf_timestamp,
		uint16 nsamples);
	
	virtual uint16 get_payload(uint8 *payload, uint16 payload_size,
		uint32 timestamp, uint32 &rtp_timestamp);
};


// DTMF player for inband tones
class t_inband_dtmf_player : public t_dtmf_player {
private:
	// Frequency generator to generate the inband tones.
	t_freq_gen	_freq_gen;
	
public:
	t_inband_dtmf_player(t_audio_rx *audio_rx, t_audio_encoder *audio_encoder,
		t_user *user_config, uint8 dtmf_tone, uint32 dtmf_timestamp,
		uint16 nsamples);
	
	virtual uint16 get_payload(uint8 *payload, uint16 payload_size,
		uint32 timestamp, uint32 &rtp_timestamp);
};

#endif
