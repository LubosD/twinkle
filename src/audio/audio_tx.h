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

#ifndef _AUDIO_TX_H
#define _AUDIO_TX_H

// Receive RTP and send audio to soundcard

#include <map>
#include <string>
#include "audio_codecs.h"
#include "audio_decoder.h"
#include "audio_rx.h"
#include "media_buffer.h"
#include "rtp_telephone_event.h"
#include "user.h"
#include "threads/mutex.h"
#include "gsm/inc/gsm.h"
#include "audio_device.h"
#include "twinkle_rtp_session.h"
#include "twinkle_config.h"

using namespace std;
using namespace ost;

// Forward declarations
class t_audio_session;
class t_line;

class t_audio_tx {
private:
	// audio_session owning this audio transmitter
	t_audio_session *audio_session;
	
	// User profile of user using the line
	// This is a pointer to the user_config owned by a phone user.
	// So this pointer should never be deleted.
	t_user		*user_config;

	// file descriptor audio capture device
	t_audio_io		*playback_device;
	t_twinkle_rtp_session	*rtp_session;

	// Indicates if this transmitter is part of a 3-way conference
	bool		is_3way;

	// Indicates if this transmitter is the mixer in a 3-way conference.
	// The mixer will mix this audio stream with the audio from the other
	// party and send it to the soundcard.
	// If the transmitter is part of a 3-way conference, but not the
	// mixer, then it simply has to send its audio payload to the mixer.
	bool		is_3way_mixer;

	// Media buffer for media from the other far-end. This buffer is
	// used by the mixer.
	t_media_buffer	*media_3way_peer_tx;

	// The peer audio transmitter in a 3-way conference
	t_audio_tx	*peer_tx_3way;

	// The audio receiver that needs input from this transmitter in
	// a 3-way conference.
	t_audio_rx	*peer_rx_3way;

	// Buffer for mixing purposes in 3-way conference.
	unsigned char	*mix_buf_3way;

	// Mutex to protect 3-way resources
	t_mutex		mtx_3way;

	// Codec information
	t_audio_codec	codec;
	map<unsigned short, t_audio_codec> payload2codec;
	unsigned short	ptime;	// in milliseconds
	
	// Sample rate of sound card.
	// The sample rate of the sound card is set to the sample rate
	// used for the initial codec. The far end may dynamically switch
	// to a codec with another sample rate. This will not change the
	// sample rate of the sound card! (capture and playback cannot
	// be done at different sampling rates).
	unsigned short	sc_sample_rate;
	
	// Mapping from codecs to decoders
	map<t_audio_codec, t_audio_decoder *> map_audio_decoder;

	// Buffer to store PCM samples of a received RTP packet
	unsigned char	*sample_buf;

	// Jitter buffer (PCM).
	// jitter_buf_len indicates the number of bytes in the jitter buffer.
	// At the start of playing the samples are stored in the jitter buffer.
	// Once the buffer is full, all samples are copied to the memory of
	// the soundcard. From that point the soundcard itself is the jitter
	// buffer.
	unsigned char	*jitter_buf;
	unsigned short	jitter_buf_len;
	bool		load_jitter_buf;

	// Buffer to keep last played packets for concealment.
	unsigned char	*conceal_buf[MAX_CONCEALMENT];
	unsigned short	conceal_buflen[MAX_CONCEALMENT]; // length of packet
	short		conceal_pos; // points to the oldest packet.
	short		conceal_num; // number of retained packets.

	unsigned short	soundcard_buf_size;

	// Payload type for telephone-event payload.
	// Some endpoints ignore the payload type that was sent in an
	// outgoing INVITE and simply sends it with the payload type,
	// they indicated in the 200 OK. Accept both payloads for
	// interoperability.
	int pt_telephone_event;
	int pt_telephone_event_alt;

	// Timestamp of previous DTMF tone
	unsigned long	dtmf_previous_timestamp;

	// Inidicates if the playing thread is running
	volatile bool is_running;

	// The thread exits when this indicator is set to true
	volatile bool stop_running;

	// Retain a packet (PCM encoded) for possible concealment.
	void retain_for_concealment(unsigned char *buf, unsigned short len);

	// Play last num packets again.
	void conceal(short num);

	// Erase concealment buffers.
	void clear_conceal_buf(void);

	// Play PCM encoded samples
	// - only_3rd_party indicates if there is only 3rd_party audio available,
	//   i.e. due to jitter, packet loss or silence suppression
	void play_pcm(unsigned char *buf, unsigned short len, bool only_3rd_party = false);

public:
	// Create the audio transmitter
	// _fd		file descriptor of capture device
	// _rtp_session	RTP socket tp send the RTP stream
	// _codec	audio codec to use
	// _ptime	length of the audio packets in ms
	// _ptime = 0 means use default ptime value for the codec
	t_audio_tx(t_audio_session *_audio_session, t_audio_io *_playback_device,
		   t_twinkle_rtp_session *_rtp_session,
	           t_audio_codec _codec, 
	           const map<unsigned short, t_audio_codec> &_payload2codec,
	           unsigned short _ptime = 0);

	~t_audio_tx();
	
	// Set the is running flag
	void set_running(bool running);

	void run(void);
	
	// Set the dynamic payload type for telephone events
	void set_pt_telephone_event(int pt, int pt_alt);

	// Get phone line belonging to this audio transmitter
	t_line *get_line(void) const;

	// Join this transmitter in a 3way conference.
	// mixer indicates if this transmitter must be the mixer.
	// - peer_tx is the peer transmitter in a 3-way
	// - audio_rx is the audio receiver needing the output from this
	//   transmitter for mixing.
	void join_3way(bool mixer, t_audio_tx *peer_tx, t_audio_rx *peer_rx);

	// Change the peer rx/tx (NULL to erase)
	void set_peer_tx_3way(t_audio_tx *peer_tx);
	void set_peer_rx_3way(t_audio_rx *peer_rx);

	// Change the mixer role
	void set_mixer_3way(bool mixer);

	// Stop the 3-way conference and make it a 1-on-1 call again.
	void stop_3way(void);

	// Post media from the peer transmitter for a 3-way mixer.
	void post_media_peer_tx_3way(unsigned char *media, int len, 
		unsigned short peer_sample_rate);

	// Returns if this transmitter is the mixer in a 3-way
	bool get_is_3way_mixer(void) const;
};

#endif
