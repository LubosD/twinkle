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

#ifndef _AUDIO_SESSION_H
#define _AUDIO_SESSION_H

#include <map>
#include <string>
#include "audio_rx.h"
#include "audio_tx.h"
#include "session.h"
#include "twinkle_rtp_session.h"
#include "threads/thread.h"
#include "threads/mutex.h"

#ifdef HAVE_SPEEX
#include <speex/speex_echo.h> 
#endif

using namespace std;
using namespace ost;

// Forward declarations
class t_session;
class t_line;

class t_audio_session {
private:
	// SIP session owning this audio session
	t_session	*session;
	
	/** Mutex for concurrent access to the session. */
	mutable t_mutex	mtx_session;

	// This flag indicates if the created audio session is valid.
	// It might be invalid because, the RTP session could not be created
	// or the soundcard could not be opened.
	bool		valid;

	// file descriptor audio device
	t_audio_io		*speaker;
	t_audio_io		*mic;
	t_twinkle_rtp_session   *rtp_session;

	t_audio_codec	codec;
	unsigned short	ptime;	// in milliseconds

	t_thread	*thr_audio_rx; // recording thread
	t_thread	*thr_audio_tx; // playing thread
	
	// ZRTP info
	mutable t_mutex	mtx_zrtp_data;
	bool		is_encrypted;
	string		zrtp_sas;
	bool		zrtp_sas_confirmed;
	string		srtp_cipher_mode;

#ifdef HAVE_SPEEX
	// Indicator whether to use (Speex) AEC 
	bool do_echo_cancellation;

	// Indicator whether the last operation of (Speex) AEC,
	// speex_echo_capture or speex_echo_playback, was the speex_echo_capture
	bool echo_captured_last;

	// speex AEC state 
	SpeexEchoState *speex_echo_state;
#endif

	// 3-way conference data
	// Returns if this audio session is part of a 3-way conference
	bool is_3way(void) const;

	// Returns the peer audio session of a 3-way conference
	t_audio_session *get_peer_3way(void) const;

	// Open the sound card
	bool open_dsp(void);
	bool open_dsp_full_duplex(void);
	bool open_dsp_speaker(void);
	bool open_dsp_mic(void);

public:

	t_audio_rx	*audio_rx;
	t_audio_tx	*audio_tx;


	t_audio_session(t_session *_session,
			const string &_recv_host, unsigned short _recv_port,
		        const string &_dst_host, unsigned short _dst_port,
			t_audio_codec _codec, unsigned short _ptime,
			const map<unsigned short, t_audio_codec> &recv_payload2ac,
			const map<t_audio_codec, unsigned short> &send_ac2payload,
			bool encrypt);

	~t_audio_session();

	void run(void);
	
	/**
	 * Change the owning session.
	 * @param _session New session owning this audio session.
	 */
	void set_session(t_session *_session);

	// Set outgoing/incoming DTMF dynamic payload types
	void set_pt_out_dtmf(unsigned short pt);
	void set_pt_in_dtmf(unsigned short pt, unsigned short pt_alt);

	// Send DTMF digit
	void send_dtmf(char digit, bool inband);

	// Get the line that belongs to this audio session
	t_line *get_line(void) const;

	// Become the first session in a 3-way conference
	void start_3way(void);

	// Leave a 3-way conference
	void stop_3way(void);

	// Check if audio session is valid
	bool is_valid(void) const;

	// Get pointer for soundcard I/O object
	t_audio_io* get_dsp_speaker(void) const;
	t_audio_io* get_dsp_mic(void) const;
	
	// Check if sample rate from speaker and mic match with sample rate
	// from codec. The sample rates might not match due to 3-way conference
	// calls with mixed sample rate
	bool matching_sample_rates(void) const;
	
	// ZRTP actions
	void confirm_zrtp_sas(void);
	void reset_zrtp_sas_confirmation(void);
	void enable_zrtp(void);
	void zrtp_request_go_clear(void);
	void zrtp_go_clear_ok(void);
	
	// ZRTP data manipulations
	bool get_is_encrypted(void) const;
	string get_zrtp_sas(void) const;
	bool get_zrtp_sas_confirmed(void) const;
	string get_srtp_cipher_mode(void) const;
	
	void set_is_encrypted(bool on);
	void set_zrtp_sas(const string &sas);
	void set_zrtp_sas_confirmed(bool confirmed);
	void set_srtp_cipher_mode(const string &cipher_mode);

#ifdef HAVE_SPEEX
	// speex acoustic echo cancellation (AEC) manipulations
	bool get_do_echo_cancellation(void) const;
	bool get_echo_captured_last(void);
	void set_echo_captured_last(bool value);
	SpeexEchoState *get_speex_echo_state(void);
#endif
	
};

// Main functions for rx and tx threads
void *main_audio_rx(void *arg);
void *main_audio_tx(void *arg);

#endif
