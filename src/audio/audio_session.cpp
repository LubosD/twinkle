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
#include "twinkle_config.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <cstdlib>
#include <cstdio>
#include "audio_session.h"
#include "line.h"
#include "log.h"
#include "sys_settings.h"
#include "translator.h"
#include "user.h"
#include "userintf.h"
#include "util.h"
#include "audits/memman.h"

#ifdef HAVE_ZRTP
#include "twinkle_zrtp_ui.h"
#endif

static t_audio_session *_audio_session;

///////////
// PRIVATE
///////////

bool t_audio_session::is_3way(void) const {
	t_line *l = get_line();
	t_phone *p = l->get_phone();

	return p->part_of_3way(l->get_line_number());
}

t_audio_session *t_audio_session::get_peer_3way(void) const {
	t_line *l = get_line();
	t_phone *p = l->get_phone();

	t_line *peer_line = p->get_3way_peer_line(l->get_line_number());

	return peer_line->get_audio_session();
}

bool t_audio_session::open_dsp(void) {
	if (sys_config->equal_audio_dev(sys_config->get_dev_speaker(), 
			sys_config->get_dev_mic())) 
	{
		return open_dsp_full_duplex();
	}
	
	return open_dsp_speaker() && open_dsp_mic();
}

bool t_audio_session::open_dsp_full_duplex(void) {

	// Open audio device
	speaker = t_audio_io::open(sys_config->get_dev_speaker(), true, true, true, 1, 
		SAMPLEFORMAT_S16, 44100, true);
	if (!speaker) {
		string msg(TRANSLATE2("CoreAudio", "Failed to open sound card"));
		log_file->write_report(msg, "t_audio_session::open_dsp_full_duplex",
			LOG_NORMAL, LOG_CRITICAL);
		ui->cb_display_msg(msg, MSG_CRITICAL);
		return false;
	}

	// Disable recording
	// If recording is not disabled, then the capture buffers will
	// already fill with data. Then when the audio_rx thread starts
	// to read blocks of 160 samples, it gets all these initial blocks
	// very quickly 1 per 12 ms I have seen. And hence the timestamps
	// for these blocks get out of sync with the RTP stack.
	// Also a large delay is introduced by this. So recording should
	// be enabled just before the data is read from the device.
	speaker->enable(true, false);
	
	mic = speaker;
	return true;
}

bool t_audio_session::open_dsp_speaker(void) {
	
	speaker = t_audio_io::open(sys_config->get_dev_speaker(), true, false, true, 1, 
		SAMPLEFORMAT_S16, audio_sample_rate(codec), true);
	if (!speaker) {
		string msg(TRANSLATE2("CoreAudio", "Failed to open sound card"));
		log_file->write_report(msg, "t_audio_session::open_dsp_speaker",
			LOG_NORMAL, LOG_CRITICAL);
		ui->cb_display_msg(msg, MSG_CRITICAL);
		return false;
	}
	
	return true;
}

bool t_audio_session::open_dsp_mic(void) {
	mic = t_audio_io::open(sys_config->get_dev_mic(), false, true, true, 1, 
		SAMPLEFORMAT_S16, audio_sample_rate(codec), true);
	if (!mic) {
		string msg(TRANSLATE2("CoreAudio", "Failed to open sound card"));
		log_file->write_report(msg, "t_audio_session::open_dsp_mic",
			LOG_NORMAL, LOG_CRITICAL);
		ui->cb_display_msg(msg, MSG_CRITICAL);
		return false;
	}

	// Disable recording
	// If recording is not disabled, then the capture buffers will
	// already fill with data. Then when the audio_rx thread starts
	// to read blocks of 160 samples, it gets all these initial blocks
	// very quickly 1 per 12 ms I have seen. And hence the timestamps
	// for these blocks get out of sync with the RTP stack.
	// Also a large delay is introduced by this. So recording should
	// be enabled just before the data is read from the device.
	speaker->enable(true, false);

	return true;
}

///////////
// PUBLIC
///////////

t_audio_session::t_audio_session(t_session *_session,
		const string &_recv_host, unsigned short _recv_port,
	        const string &_dst_host, unsigned short _dst_port,
		t_audio_codec _codec, unsigned short _ptime,
		const map<unsigned short, t_audio_codec> &recv_payload2ac,
		const map<t_audio_codec, unsigned short> &send_ac2payload,
		bool encrypt)
{
	valid = false;

	session = _session;
	audio_rx = NULL;
	audio_tx = NULL;
	thr_audio_rx = NULL;
	thr_audio_tx = NULL;
	speaker = NULL;
	mic = NULL;

	codec = _codec;
	ptime = _ptime;
	
	is_encrypted = false;
	zrtp_sas.clear();
	
	// Assume the SAS is confirmed. When a SAS is received from the ZRTP
	// stack, the confirmed flag will be cleared.
	zrtp_sas_confirmed = true;
	
	srtp_cipher_mode.clear();

	log_file->write_header("t_audio_session::t_audio_session");
	log_file->write_raw("Receive RTP from: ");
	log_file->write_raw(_recv_host);
	log_file->write_raw(":");
	log_file->write_raw(_recv_port);
	log_file->write_endl();
	log_file->write_raw("Send RTP to: ");
	log_file->write_raw(_dst_host);
	log_file->write_raw(":");
	log_file->write_raw(_dst_port);
	log_file->write_endl();
	log_file->write_footer();
	
	t_user *user_config = get_line()->get_user();

	// Create RTP session
	try {
		if (_recv_host.empty() || _recv_port == 0) {
			rtp_session = new t_twinkle_rtp_session(
				InetHostAddress("0.0.0.0"));
			MEMMAN_NEW(rtp_session);
		} else {
			rtp_session = new t_twinkle_rtp_session(
				InetHostAddress(_recv_host.c_str()), _recv_port);
			MEMMAN_NEW(rtp_session);
		}
#ifdef HAVE_ZRTP
		ZrtpQueue* zque = dynamic_cast<ZrtpQueue*>(rtp_session);
		if (zque && rtp_session->is_zrtp_initialized()) {
			zque->setEnableZrtp(encrypt);
		
			if (user_config->get_zrtp_enabled()) {
				// Create the ZRTP call back interface
				TwinkleZrtpUI* twui = new TwinkleZrtpUI(this);
				
				// The ZrtpQueue keeps track of the twui - the destructor of 
				// ZrtpQueue (aka t_twinkle_rtp_session) deletes this object, 
				// thus no other management is required.
				zque->setUserCallback(twui);
			}
		}
#endif
	} catch(...) {
		// If the RTPSession constructor throws an exception, no
		// object is created, so clear the pointer.
		rtp_session = NULL;
		string msg(TRANSLATE2("CoreAudio", "Failed to create a UDP socket (RTP) on port %1"));
		msg = replace_first(msg, "%1", int2str(_recv_port));
		log_file->write_report(msg, "t_audio_session::t_audio_session",
			LOG_NORMAL, LOG_CRITICAL);
		ui->cb_show_msg(msg, MSG_CRITICAL);
		return;
	}

	if (!_dst_host.empty() && _dst_port != 0) {
		rtp_session->addDestination(
			InetHostAddress(_dst_host.c_str()), _dst_port);
	}

	// Set payload format for outgoing RTP packets
	map<t_audio_codec, unsigned short>::const_iterator it;
	it = send_ac2payload.find(codec);
	assert(it != send_ac2payload.end());
	unsigned short payload_id = it->second;
	rtp_session->setPayloadFormat(DynamicPayloadFormat(
			payload_id, audio_sample_rate(codec)));

	// Open and initialize sound card
	t_audio_session *as_peer;
	if (is_3way() && (as_peer = get_peer_3way())) {
		speaker = as_peer->get_dsp_speaker();
		mic = as_peer->get_dsp_mic();
		if (!speaker || !mic) return;
	} else {
		if (!open_dsp()) return;
	}

#ifdef HAVE_SPEEX	
	// Speex AEC auxiliary data initialization 
	do_echo_cancellation = false;

	if (user_config->get_speex_dsp_aec()) {
	    int nsamples = audio_sample_rate(codec) / 1000 * ptime;
 	    speex_echo_state = speex_echo_state_init(nsamples, 5*nsamples);
	    do_echo_cancellation = true;
	    echo_captured_last = true;
	}
#endif

	// Create recorder
	if (!_recv_host.empty() && _recv_port != 0) {
		audio_rx = new t_audio_rx(this, mic, rtp_session, codec, 
				payload_id, ptime);
		MEMMAN_NEW(audio_rx);

		// Setup 3-way configuration if this audio session is part of
		// a 3-way.
		if (is_3way()) {
			t_audio_session *peer = get_peer_3way();
			if (!peer || !peer->audio_rx) {
				// There is no peer rx yet, so become the main rx
				audio_rx->join_3way(true, NULL);

				if (peer && peer->audio_tx) {
					peer->audio_tx->set_peer_rx_3way(audio_rx);
				}
			} else {
				// There is a peer rx already so that must be the
				// main rx.
				audio_rx->join_3way(false, peer->audio_rx);
				peer->audio_rx->set_peer_rx_3way(audio_rx);

				if (peer->audio_tx) {
					peer->audio_tx->set_peer_rx_3way(audio_rx);
				}
			}
		}
	}

	// Create player
	if (!_dst_host.empty() && _dst_port != 0) {
		audio_tx = new t_audio_tx(this, speaker, rtp_session, codec,
				recv_payload2ac, ptime);
		MEMMAN_NEW(audio_tx);

		// Setup 3-way configuration if this audio session is part of
		// a 3-way.
		if (is_3way()) {
			t_audio_session *peer = get_peer_3way();
			if (!peer) {
				// There is no peer tx yet, so become the mixer tx
				audio_tx->join_3way(true, NULL, NULL);
			} else if (!peer->audio_tx) {
				// There is a peer audio session, but no peer tx,
				// so become the mixer tx
				audio_tx->join_3way(true, NULL, peer->audio_rx);
			} else {
				// There is a peer tx already. That must be the
				// mixer.
				audio_tx->join_3way(
					false, peer->audio_tx, peer->audio_rx);
			}
		}
	}
	valid = true;
}

t_audio_session::~t_audio_session() {
	// Delete of the audio_rx and audio_tx objects will terminate
	// thread execution.
	if (audio_rx) {
		// Reconfigure 3-way configuration if this audio session is
		// part of a 3-way.
		if (is_3way()) {
			t_audio_session *peer = get_peer_3way();
			if (peer) {
				// Make the peer audio rx the main rx and remove
				// reference to this audio rx
				if (peer->audio_rx) {
					peer->audio_rx->set_peer_rx_3way(NULL);
					peer->audio_rx->set_main_rx_3way(true);
				}

				// Remove reference to this audio rx
				if (peer->audio_tx) {
					peer->audio_tx->set_peer_rx_3way(NULL);
				}
			}
		}
		MEMMAN_DELETE(audio_rx);
		delete audio_rx;
	}

	if (audio_tx) {
		// Reconfigure 3-way configuration if this audio session is
		// part of a 3-way.
		if (is_3way()) {
			t_audio_session *peer = get_peer_3way();
			if (peer) {
				// Make the peer audio tx the mixer and remove
				// reference to this audio tx
				if (peer->audio_tx) {
					peer->audio_tx->set_peer_tx_3way(NULL);
					peer->audio_tx->set_mixer_3way(true);
				}
			}
		}
		MEMMAN_DELETE(audio_tx);
		delete audio_tx;
	}

	if (thr_audio_rx) {
		MEMMAN_DELETE(thr_audio_rx);
		delete thr_audio_rx;
	}

	if (thr_audio_tx) {
		MEMMAN_DELETE(thr_audio_tx);
		delete thr_audio_tx;
	}

	if (rtp_session) {
		log_file->write_header("t_audio_session::~t_audio_session");
		log_file->write_raw("Line ");
		log_file->write_raw(get_line()->get_line_number()+1);
		log_file->write_raw(": stopping RTP session.\n");
		log_file->write_footer();
		
		MEMMAN_DELETE(rtp_session);
		delete rtp_session;

		log_file->write_header("t_audio_session::~t_audio_session");
		log_file->write_raw("Line ");
		log_file->write_raw(get_line()->get_line_number()+1);
		log_file->write_raw(": RTP session stopped.\n");
		log_file->write_footer();
	}

	if (speaker && (!is_3way() || !get_peer_3way())) {
		if (mic == speaker) mic = 0;
		MEMMAN_DELETE(speaker);
		delete speaker;
		speaker = 0;
	}
	
	if (mic && (!is_3way() || !get_peer_3way())) {
		MEMMAN_DELETE(mic);
		delete mic;
		mic = 0;
	}

#ifdef HAVE_SPEEX
	// cleaning speech AEC
	if (do_echo_cancellation) {
	    speex_echo_state_destroy(speex_echo_state); 
	}
#endif
}

void t_audio_session::set_session(t_session *_session) {
	mtx_session.lock();
	session = _session;
	mtx_session.unlock();
}

void t_audio_session::run(void) {
	_audio_session = this;

	log_file->write_header("t_audio_session::run");
	log_file->write_raw("Line ");
	log_file->write_raw(get_line()->get_line_number()+1);
	log_file->write_raw(": starting RTP session.\n");
	log_file->write_footer();

	rtp_session->startRunning();

	log_file->write_header("t_audio_session::run");
	log_file->write_raw("Line ");
	log_file->write_raw(get_line()->get_line_number()+1);
	log_file->write_raw(": RTP session started.\n");
	log_file->write_footer();

	if (audio_rx) {
		try {
			// Set the running flag now instead of at the start of
			// t_audio_tx::run as due to race conditions the thread might
			// get destroyed before the run method starts running. The
			// destructor still has to wait on the thread to finish.
			audio_rx->set_running(true);
			
			thr_audio_rx = new t_thread(main_audio_rx, NULL);
			MEMMAN_NEW(thr_audio_rx);
			// thr_audio_rx->set_sched_fifo(90);
			thr_audio_rx->detach();
		} catch (int) {
			audio_rx->set_running(false);
			string msg(TRANSLATE2("CoreAudio", "Failed to create audio receiver thread."));
			log_file->write_report(msg, "t_audio_session::run",
				LOG_NORMAL, LOG_CRITICAL);
			ui->cb_show_msg(msg, MSG_CRITICAL);
			exit(1);
		}
	}


	if (audio_tx) {
		try {
			// See comment above for audio_rx
			audio_tx->set_running(true);
			
			thr_audio_tx = new t_thread(main_audio_tx, NULL);
			MEMMAN_NEW(thr_audio_tx);
			// thr_audio_tx->set_sched_fifo(90);
			thr_audio_tx->detach();
		} catch (int) {
			audio_tx->set_running(false);
			string msg(TRANSLATE2("CoreAudio", "Failed to create audio transmitter thread."));
			log_file->write_report(msg, "t_audio_session::run",
				LOG_NORMAL, LOG_CRITICAL);
			ui->cb_show_msg(msg, MSG_CRITICAL);
			exit(1);
		}
	}
}

void t_audio_session::set_pt_out_dtmf(unsigned short pt) {
	if (audio_rx) audio_rx->set_pt_telephone_event(pt);
}

void t_audio_session::set_pt_in_dtmf(unsigned short pt, unsigned short pt_alt) {
	if (audio_tx) audio_tx->set_pt_telephone_event(pt, pt_alt);
}

void t_audio_session::send_dtmf(char digit, bool inband) {
	if (audio_rx) audio_rx->push_dtmf(digit, inband);
}

t_line *t_audio_session::get_line(void) const {
	t_line *line;
	mtx_session.lock();
	line = session->get_line();
	mtx_session.unlock();
	
	return line;
}

void t_audio_session::start_3way(void) {
	if (audio_rx) {
		audio_rx->join_3way(true, NULL);
	}

	if (audio_tx) {
		audio_tx->join_3way(true, NULL, NULL);
	}
}

void t_audio_session::stop_3way(void) {
	if (audio_rx) {
		t_audio_session *peer = get_peer_3way();
		if (peer) {
			if (peer->audio_rx) {
				peer->audio_rx->set_peer_rx_3way(NULL);
			}

			if (peer->audio_tx) {
				peer->audio_tx->set_peer_rx_3way(NULL);
			}
		}
		audio_rx->stop_3way();
	}

	if (audio_tx) {
		t_audio_session *peer = get_peer_3way();
		if (peer) {
			if (peer->audio_tx) {
				peer->audio_tx->set_peer_tx_3way(NULL);
			}
		}
		audio_tx->stop_3way();
	}
}

bool t_audio_session::is_valid(void) const {
	return valid;
}

t_audio_io* t_audio_session::get_dsp_speaker(void) const {
	return speaker;
}

t_audio_io* t_audio_session::get_dsp_mic(void) const {
	return mic;
}

bool t_audio_session::matching_sample_rates(void) const {
	int codec_sample_rate = audio_sample_rate(codec);
	return (speaker->get_sample_rate() == codec_sample_rate &&
		mic->get_sample_rate() == codec_sample_rate);
}

void t_audio_session::confirm_zrtp_sas(void) {
#ifdef HAVE_ZRTP
	ZrtpQueue* zque = dynamic_cast<ZrtpQueue*>(rtp_session);
	if (zque) {
		zque->SASVerified();
		set_zrtp_sas_confirmed(true);
	}
#endif
}

void t_audio_session::reset_zrtp_sas_confirmation(void) {
#ifdef HAVE_ZRTP
	ZrtpQueue* zque = dynamic_cast<ZrtpQueue*>(rtp_session);
	if (zque) {
		zque->resetSASVerified();
		set_zrtp_sas_confirmed(false);
	}
#endif
}

void t_audio_session::enable_zrtp(void) {
#ifdef HAVE_ZRTP
	ZrtpQueue* zque = dynamic_cast<ZrtpQueue*>(rtp_session);
	if (zque) {
		zque->setEnableZrtp(true);
	}
#endif
}

void t_audio_session::zrtp_request_go_clear(void) {
#ifdef HAVE_ZRTP
	ZrtpQueue* zque = dynamic_cast<ZrtpQueue*>(rtp_session);
	if (zque) {
		zque->requestGoClear();
	}
#endif
}

void t_audio_session::zrtp_go_clear_ok(void) {
#ifdef HAVE_ZRTP
	ZrtpQueue* zque = dynamic_cast<ZrtpQueue*>(rtp_session);
	if (zque) {
		zque->goClearOk();
	}
#endif
}

bool t_audio_session::get_is_encrypted(void) const {
	mtx_zrtp_data.lock();
	bool b = is_encrypted;
	mtx_zrtp_data.unlock();
	return b;
}

string t_audio_session::get_zrtp_sas(void) const {
	mtx_zrtp_data.lock();
	string s = zrtp_sas;
	mtx_zrtp_data.unlock();
	return s;
}

bool t_audio_session::get_zrtp_sas_confirmed(void) const {
	mtx_zrtp_data.lock();
	bool b = zrtp_sas_confirmed;
	mtx_zrtp_data.unlock();
	return b;
}

string t_audio_session::get_srtp_cipher_mode(void) const {
	mtx_zrtp_data.lock();
	string s = srtp_cipher_mode;
	mtx_zrtp_data.unlock();
	return s;
}

void t_audio_session::set_is_encrypted(bool on) {
	mtx_zrtp_data.lock();
	is_encrypted = on;
	mtx_zrtp_data.unlock();
}

void t_audio_session::set_zrtp_sas(const string &sas) {
	mtx_zrtp_data.lock();
	zrtp_sas = sas;
	mtx_zrtp_data.unlock();
}

void t_audio_session::set_zrtp_sas_confirmed(bool confirmed) {
	mtx_zrtp_data.lock();
	zrtp_sas_confirmed = confirmed;
	mtx_zrtp_data.unlock();
}

void t_audio_session::set_srtp_cipher_mode(const string &cipher_mode) {
	mtx_zrtp_data.lock();
	srtp_cipher_mode = cipher_mode;
	mtx_zrtp_data.unlock();
}


#ifdef HAVE_SPEEX
bool t_audio_session::get_do_echo_cancellation(void) const {
    return do_echo_cancellation;
}

bool t_audio_session::get_echo_captured_last(void) {
    return echo_captured_last;
}

void t_audio_session::set_echo_captured_last(bool value) {
    echo_captured_last = value;
}

SpeexEchoState *t_audio_session::get_speex_echo_state(void) {
    return speex_echo_state;
}
#endif

void *main_audio_rx(void *arg) {
	_audio_session->audio_rx->run();
	return NULL;
}

void *main_audio_tx(void *arg) {
	_audio_session->audio_tx->run();
	return NULL;
}
