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
#include <cstring>
#include <sys/time.h>
#include "dtmf_player.h"
#include "audio_rx.h"
#include "line.h"
#include "rtp_telephone_event.h"
#include "log.h"

/////////////////////////////////////////
// class t_dtmf_player
/////////////////////////////////////////

t_dtmf_player::t_dtmf_player(t_audio_rx *audio_rx, t_audio_encoder *audio_encoder, 
			t_user *user_config, uint8 dtmf_tone, uint32 dtmf_timestamp,
			uint16 nsamples) :
	_audio_rx(audio_rx),
	_user_config(user_config),
	_audio_encoder(audio_encoder),
	_dtmf_pause(false),
	_dtmf_stop(false),
	_dtmf_current(dtmf_tone),
	_dtmf_timestamp(dtmf_timestamp),
	_dtmf_duration(0),
	_nsamples(nsamples)
{}

uint32 t_dtmf_player::get_timestamp(void) {
	return _dtmf_timestamp;
}

bool t_dtmf_player::finished(void) {
	return _dtmf_stop;
}

/////////////////////////////////////////
// class t_rtp_event_dtmf_player
/////////////////////////////////////////

t_rtp_event_dtmf_player::t_rtp_event_dtmf_player(t_audio_rx *audio_rx, 
			t_audio_encoder *audio_encoder, t_user *user_config,
			uint8 dtmf_tone, uint32 dtmf_timestamp,
			uint16 nsamples) : 
	t_dtmf_player(audio_rx, audio_encoder, user_config, dtmf_tone, dtmf_timestamp, 
			nsamples)
{
}

uint16 t_rtp_event_dtmf_player::get_payload(uint8 *payload, 
		uint16 payload_size, uint32 timestamp, uint32 &rtp_timestamp)
{
	t_rtp_telephone_event *dtmf_payload = (t_rtp_telephone_event *)payload;
	assert(sizeof(t_rtp_telephone_event) <= payload_size);

	// RFC 2833 3.5, 3.6
	dtmf_payload->set_event(_dtmf_current);
	dtmf_payload->set_reserved(false);
	dtmf_payload->set_volume(_user_config->get_dtmf_volume());

	if (_dtmf_pause) {
		// Trailing pause phase of a DTMF tone
		// Repeat the last packet
		dtmf_payload->set_end(true);

		int pause_duration = timestamp - _dtmf_timestamp - _dtmf_duration +
				     _nsamples;
		if (pause_duration / _nsamples * _audio_encoder->get_ptime() >=
					_user_config->get_dtmf_pause())
		{
			// This is the last packet to be sent for the
			// current DTMF tone.
			_dtmf_stop = true;
			log_file->write_header("t_rtp_event_dtmf_player::get_payload",
					LOG_NORMAL);
			log_file->write_raw("Audio rx line ");
			log_file->write_raw(_audio_rx->get_line()->get_line_number()+1);
			log_file->write_raw(": finish DTMF event - ");
			log_file->write_raw(_dtmf_current);
			log_file->write_endl();
			log_file->write_footer();
		}
	} else {
		// Play phase of a DTMF tone
		// The duration counts from the start of the tone.
		_dtmf_duration = timestamp - _dtmf_timestamp + _nsamples;

		// Check if the tone must end
		if (_dtmf_duration / _nsamples * _audio_encoder->get_ptime() >=
						_user_config->get_dtmf_duration())
		{
			dtmf_payload->set_end(true);
			_dtmf_pause = true;
		} else {
			dtmf_payload->set_end(false);
		}
	}

	dtmf_payload->set_duration(_dtmf_duration);
	rtp_timestamp = _dtmf_timestamp;
	return sizeof(t_rtp_telephone_event);
}


/////////////////////////////////////////
// class t_inband_dtmf_player
/////////////////////////////////////////

t_inband_dtmf_player::t_inband_dtmf_player(t_audio_rx *audio_rx, 
		t_audio_encoder *audio_encoder, t_user *user_config,
		uint8 dtmf_tone, uint32 dtmf_timestamp,
		uint16 nsamples) :
	t_dtmf_player(audio_rx, audio_encoder, user_config, dtmf_tone, dtmf_timestamp, 
			nsamples),
	_freq_gen(dtmf_tone, -(user_config->get_dtmf_volume()))
{
}

uint16 t_inband_dtmf_player::get_payload(uint8 *payload, 
		uint16 payload_size, uint32 timestamp, uint32 &rtp_timestamp)
{
	int16 sample_buf[_nsamples];

	if (_dtmf_pause) {
		int pause_duration = timestamp - _dtmf_timestamp - _dtmf_duration +
				     _nsamples;
				     
		memset(sample_buf, 0, _nsamples * 2);
				     
		if (pause_duration / _nsamples * _audio_encoder->get_ptime() >=
					_user_config->get_dtmf_pause())
		{
			// This is the last packet to be sent for the
			// current DTMF tone.
			_dtmf_stop = true;
			log_file->write_header("t_inband_dtmf_player::get_payload", LOG_NORMAL);
			log_file->write_raw("Audio rx line ");
			log_file->write_raw(_audio_rx->get_line()->get_line_number()+1);
			log_file->write_raw(": finish DTMF event - ");
			log_file->write_raw(_dtmf_current);
			log_file->write_endl();
			log_file->write_footer();
		}
	} else {
		// Timestamp and interval for _freq_gen must be in usec
		uint32 ts_start = (timestamp - _dtmf_timestamp) * 1000000 /
					_audio_encoder->get_sample_rate();
		_freq_gen.get_samples(sample_buf, _nsamples, ts_start, 
			1000000.0 / _audio_encoder->get_sample_rate());
		
		// The duration counts from the start of the tone.
		_dtmf_duration = timestamp - _dtmf_timestamp + _nsamples;

		// Check if the tone must end
		if (_dtmf_duration / _nsamples * _audio_encoder->get_ptime() >=
						_user_config->get_dtmf_duration())
		{
			_dtmf_pause = true;
		}
	}
	
	// Encode audio samples
	bool silence;
	rtp_timestamp = timestamp;
	return _audio_encoder->encode(sample_buf, _nsamples, payload, payload_size, silence);
}
