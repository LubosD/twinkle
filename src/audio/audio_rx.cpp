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

#include <iostream>
#include <cstdio>
#include <ctime>
#include <cstdlib>
#include <sys/types.h>
#include <sys/time.h>
#include <cc++/config.h>

#include "audio_rx.h"
#include "log.h"
#include "phone.h"
#include "rtp_telephone_event.h"
#include "userintf.h"
#include "line.h"
#include "sys_settings.h"
#include "sequence_number.h"
#include "audits/memman.h"

extern t_phone *phone;

#define SAMPLE_BUF_SIZE (audio_encoder->get_ptime() * audio_encoder->get_sample_rate()/1000 *\
				AUDIO_SAMPLE_SIZE/8)

// Debug macro to print timestamp
#define DEBUG_TS(s)	{ gettimeofday(&debug_timer, NULL);\
			  cout << "DEBUG: ";\
			  cout << debug_timer.tv_sec * 1000 +\
			          debug_timer.tv_usec / 1000;\
			  cout << " " << (s) << endl;\
			}

//////////
// PRIVATE
//////////

bool t_audio_rx::get_sound_samples(unsigned short &sound_payload_size, bool &silence) {
	int status;
	struct timespec sleeptimer;
	//struct timeval debug_timer;
	
	silence = false;

	mtx_3way.lock();

	if (is_3way && !is_main_rx_3way) {
		// We are not the main receiver in a 3-way call, so
		// get the sound samples from the local media buffer.
		// This buffer will be filled by the main receiver.
		if (!media_3way_peer_rx->get(input_sample_buf, SAMPLE_BUF_SIZE)) {
			// The mutex is unlocked before going to sleep.
			// First I had the mutex unlock after the sleep.
			// That worked fine with LinuxThreading, but it does
			// not work with NPTL. It causes a deadlock when
			// the main receiver calls post_media_peer_rx_3way
			// as NPTL does not fair scheduling. This thread
			// simly gets the lock again and the main receiver
			// dies from starvation.
			mtx_3way.unlock();

			// There is not enough data yet. Sleep for 1 ms.
			sleeptimer.tv_sec = 0;
			sleeptimer.tv_nsec = 1000000;
			nanosleep(&sleeptimer, NULL);
			return false;
		}
		
		mtx_3way.unlock();
	} else {
		// Don't keep the 3way mutex locked while waiting for the DSP.
		mtx_3way.unlock();
		
		// Get the sound samples from the DSP
		status = input_device->read(input_sample_buf, SAMPLE_BUF_SIZE);

		if (status != SAMPLE_BUF_SIZE) {
			if (!logged_capture_failure) {
				// Log this failure only once
				log_file->write_header("t_audio_rx::get_sound_samples",
					LOG_NORMAL, LOG_WARNING);
				log_file->write_raw("Audio rx line ");
				log_file->write_raw(get_line()->get_line_number()+1);
				log_file->write_raw(": sound capture failed.\n");
				log_file->write_raw("Status: ");
				log_file->write_raw(status);
				log_file->write_endl();
				log_file->write_footer();
				logged_capture_failure = true;
			}

			stop_running = true;
			return false;
		}

		// If line is muted, then fill sample buffer with silence.
		// Note that we keep reading the dsp, to prevent the DSP buffers
		// from filling up.
		if (get_line()->get_is_muted()) {
			memset(input_sample_buf, 0, SAMPLE_BUF_SIZE);
		}
	}

	// Convert buffer to a buffer of shorts as the samples are 16 bits
	short *sb = (short *)input_sample_buf;

	mtx_3way.lock();
	if (is_3way) {
		// Send the sound samples to the other receiver if we
		// are the main receiver.
		// There may be no other receiver when one of the far-ends
		// has put the call on-hold.
		if (is_main_rx_3way && peer_rx_3way) {
			peer_rx_3way->post_media_peer_rx_3way(input_sample_buf, SAMPLE_BUF_SIZE,
				audio_encoder->get_sample_rate());
		}

		// Mix the sound samples with the 3rd party
		if (media_3way_peer_tx->get(mix_buf_3way, SAMPLE_BUF_SIZE)) {
			short *mix_sb = (short *)mix_buf_3way;
			for (int i = 0; i < SAMPLE_BUF_SIZE / 2; i++) {
				sb[i] = mix_linear_pcm(sb[i], mix_sb[i]);
			}
		}
	}

	mtx_3way.unlock();

	/*** PREPROCESSING & ENCODING ***/

	bool preprocessing_silence = false;

#ifdef HAVE_SPEEX
	// speex acoustic echo cancellation
	if (audio_session->get_do_echo_cancellation() && !audio_session->get_echo_captured_last()) {

	    spx_int16_t *input_buf = new spx_int16_t[SAMPLE_BUF_SIZE/2];
	    MEMMAN_NEW_ARRAY(input_buf);

	    for (int i = 0; i < SAMPLE_BUF_SIZE / 2; i++) {
		input_buf[i] = sb[i];
	    }

	    speex_echo_capture(audio_session->get_speex_echo_state(), input_buf, sb);
	    audio_session->set_echo_captured_last(true);

	    MEMMAN_DELETE_ARRAY(input_buf);
	    delete [] input_buf;
	}

	// preprocessing
	preprocessing_silence = !speex_preprocess_run(speex_preprocess_state, sb);
	
	// According to the speex API documentation the return value
	// from speex_preprocess_run() is only defined when VAD is
	// enabled. So to be safe, reset the return value, if VAD is
	// disabled.
	if (!speex_dsp_vad) preprocessing_silence = false;
#endif

	// encoding
	sound_payload_size = audio_encoder->encode(sb, nsamples, payload, payload_size, silence);	

	// recognizing silence (both from preprocessing and encoding)
	silence = silence || preprocessing_silence;

	return true;
}

bool t_audio_rx::get_dtmf_event(void) {
	// DTMF events are not supported in a 3-way conference
	if (is_3way) return false;

	if (!sema_dtmf_q.try_down()) {
		// No DTMF event available
		return false;
	}
	
	// Get next DTMF event
	mtx_dtmf_q.lock();
	t_dtmf_event dtmf_event = dtmf_queue.front();
	dtmf_queue.pop();
	mtx_dtmf_q.unlock();
	
	ui->cb_async_send_dtmf(get_line()->get_line_number(), dtmf_event.dtmf_tone);
	
	// Create DTMF player
	if (dtmf_event.inband) {
		dtmf_player = new t_inband_dtmf_player(this, audio_encoder, user_config,
				dtmf_event.dtmf_tone, timestamp, nsamples);
		MEMMAN_NEW(dtmf_player);
		
		// Log DTMF event
		log_file->write_header("t_audio_rx::get_dtmf_event", LOG_NORMAL);
		log_file->write_raw("Audio rx line ");
		log_file->write_raw(get_line()->get_line_number()+1);
		log_file->write_raw(": start inband DTMF tone - ");
		log_file->write_raw(dtmf_event.dtmf_tone);
		log_file->write_endl();
		log_file->write_footer();
	} else {
		// The telephone events may have a different sampling rate than
		// the audio codec. Change nsamples accordingly.
		nsamples = audio_sample_rate(CODEC_TELEPHONE_EVENT)/1000 *
				audio_encoder->get_ptime();
		
		dtmf_player = new t_rtp_event_dtmf_player(this, audio_encoder, user_config,
				dtmf_event.dtmf_tone, timestamp, nsamples);
		MEMMAN_NEW(dtmf_player);

		// Log DTMF event
		log_file->write_header("t_audio_rx::get_dtmf_event", LOG_NORMAL);
		log_file->write_raw("Audio rx line ");
		log_file->write_raw(get_line()->get_line_number()+1);
		log_file->write_raw(": start DTMF event - ");
		log_file->write_raw(dtmf_event.dtmf_tone);
		log_file->write_endl();
		log_file->write_raw("Payload type: ");
		log_file->write_raw(pt_telephone_event);
		log_file->write_endl();
		log_file->write_footer();

		// Set RTP payload format
		// HACK: the sample rate for telephone events is 8000, but the
		//       ccRTP stack does not handle it well when the sample rate
		//       changes. When the sample rate of the audio codec is kept
		//       on the ccRTP session settings, then all works fine.
		rtp_session->setPayloadFormat(DynamicPayloadFormat(pt_telephone_event,
				audio_encoder->get_sample_rate()));
				// should be this: audio_sample_rate(CODEC_TELEPHONE_EVENT)
	
		// As all RTP event contain the same timestamp, the ccRTP stack will
		// discard packets when the timestamp gets to old.
		// Increase the expire timeout value to prevent this.
		rtp_session->setExpireTimeout((JITTER_BUF_MS +
			user_config->get_dtmf_duration() + user_config->get_dtmf_pause()) * 1000);
	}

	return true;
}

void t_audio_rx::set_sound_payload_format(void) {
	nsamples = audio_encoder->get_sample_rate()/1000 * audio_encoder->get_ptime();
	rtp_session->setPayloadFormat(DynamicPayloadFormat(audio_encoder->get_payload_id(),
			audio_encoder->get_sample_rate()));
}

//////////
// PUBLIC
//////////

t_audio_rx::t_audio_rx(t_audio_session *_audio_session,
		   t_audio_io *_input_device, t_twinkle_rtp_session *_rtp_session,
	           t_audio_codec _codec, unsigned short _payload_id,
	           unsigned short _ptime) : sema_dtmf_q(0)
{
	audio_session = _audio_session;
	
	user_config = audio_session->get_line()->get_user();
	assert(user_config);
	
	input_device = _input_device;
	rtp_session = _rtp_session;
	dtmf_player = NULL;
	is_running = false;
	stop_running = false;
	logged_capture_failure = false;
	use_nat_keepalive = phone->use_nat_keepalive(user_config);

	pt_telephone_event = -1;
	
	// Create audio encoder
	switch (_codec) {
	case CODEC_G711_ALAW:
		audio_encoder = new t_g711a_audio_encoder(_payload_id, _ptime, user_config);
		MEMMAN_NEW(audio_encoder);
		break;
	case CODEC_G711_ULAW:
		audio_encoder = new t_g711u_audio_encoder(_payload_id, _ptime, user_config);
		MEMMAN_NEW(audio_encoder);
		break;
	case CODEC_GSM:
		audio_encoder = new t_gsm_audio_encoder(_payload_id, _ptime, user_config);
		MEMMAN_NEW(audio_encoder);
		break;
#ifdef HAVE_SPEEX
	case CODEC_SPEEX_NB:
		audio_encoder = new t_speex_audio_encoder(_payload_id, _ptime,
				t_speex_audio_encoder::MODE_NB, user_config);
		MEMMAN_NEW(audio_encoder);
		break;
	case CODEC_SPEEX_WB:
		audio_encoder = new t_speex_audio_encoder(_payload_id, _ptime,
				t_speex_audio_encoder::MODE_WB, user_config);
		MEMMAN_NEW(audio_encoder);
		break;
	case CODEC_SPEEX_UWB:
		audio_encoder = new t_speex_audio_encoder(_payload_id, _ptime,
				t_speex_audio_encoder::MODE_UWB, user_config);
		MEMMAN_NEW(audio_encoder);
		break;
#endif
#ifdef HAVE_ILBC
	case CODEC_ILBC:
		audio_encoder = new t_ilbc_audio_encoder(_payload_id, _ptime, user_config);
		MEMMAN_NEW(audio_encoder);
		break;
#endif
	case CODEC_G726_16:
		audio_encoder = new t_g726_audio_encoder(_payload_id, _ptime,
				t_g726_audio_encoder::BIT_RATE_16, user_config);
		MEMMAN_NEW(audio_encoder);
		break;
	case CODEC_G726_24:
		audio_encoder = new t_g726_audio_encoder(_payload_id, _ptime,
				t_g726_audio_encoder::BIT_RATE_24, user_config);
		MEMMAN_NEW(audio_encoder);
		break;
	case CODEC_G726_32:
		audio_encoder = new t_g726_audio_encoder(_payload_id, _ptime,
				t_g726_audio_encoder::BIT_RATE_32, user_config);
		MEMMAN_NEW(audio_encoder);
		break;
	case CODEC_G726_40:
		audio_encoder = new t_g726_audio_encoder(_payload_id, _ptime,
				t_g726_audio_encoder::BIT_RATE_40, user_config);
		MEMMAN_NEW(audio_encoder);
		break;
	default:
		assert(false);
	}
	
	payload_size = audio_encoder->get_max_payload_size();

	input_sample_buf = new unsigned char[SAMPLE_BUF_SIZE];
	MEMMAN_NEW_ARRAY(input_sample_buf);

	payload = new unsigned char[payload_size];
	MEMMAN_NEW_ARRAY(payload);
	nsamples = audio_encoder->get_sample_rate()/1000 * audio_encoder->get_ptime();

	// Initialize 3-way settings to 'null'
	media_3way_peer_tx = NULL;
	media_3way_peer_rx = NULL;
	peer_rx_3way = NULL;
	mix_buf_3way = NULL;
	is_3way = false;
	is_main_rx_3way = false;

#ifdef HAVE_SPEEX
	// initializing speex preprocessing state
	speex_preprocess_state = speex_preprocess_state_init(nsamples, audio_encoder->get_sample_rate());

	int arg;
	float farg;

	// Noise reduction
	arg = (user_config->get_speex_dsp_nrd() ? 1 : 0);
	speex_preprocess_ctl(speex_preprocess_state, SPEEX_PREPROCESS_SET_DENOISE, &arg);
	arg = -30;	
	speex_preprocess_ctl(speex_preprocess_state, SPEEX_PREPROCESS_SET_NOISE_SUPPRESS, &arg);

	// Automatic gain control
	arg = (user_config->get_speex_dsp_agc() ?  1 : 0);	
	speex_preprocess_ctl(speex_preprocess_state, SPEEX_PREPROCESS_SET_AGC, &arg);
	farg = (float) (user_config->get_speex_dsp_agc_level()) * 327.68f;	
	speex_preprocess_ctl(speex_preprocess_state, SPEEX_PREPROCESS_SET_AGC_LEVEL, &farg);
	arg = 30;	
	speex_preprocess_ctl(speex_preprocess_state, SPEEX_PREPROCESS_SET_AGC_MAX_GAIN, &arg);

	// Voice activity detection
	arg = (user_config->get_speex_dsp_vad() ? 1 : 0);
	speex_dsp_vad = (bool)arg;
	speex_preprocess_ctl(speex_preprocess_state, SPEEX_PREPROCESS_SET_VAD, &arg);

	// Acoustic echo cancellation 
	if (audio_session->get_do_echo_cancellation()) {
	    speex_preprocess_ctl(speex_preprocess_state, SPEEX_PREPROCESS_SET_ECHO_STATE, 
				 audio_session->get_speex_echo_state());
	}
#endif
}

t_audio_rx::~t_audio_rx() {
	struct timespec sleeptimer;

	if (is_running) {
		stop_running = true;
		do {
			sleeptimer.tv_sec = 0;
			sleeptimer.tv_nsec = 10000000;
			nanosleep(&sleeptimer, NULL);
		} while (is_running);
	}

#ifdef HAVE_SPEEX
	// cleaning speex preprocessing
	if (audio_session->get_do_echo_cancellation()) {
	    speex_echo_state_reset(audio_session->get_speex_echo_state());
	}
	speex_preprocess_state_destroy(speex_preprocess_state);
#endif
       
	MEMMAN_DELETE_ARRAY(input_sample_buf);
	delete [] input_sample_buf;
	
	MEMMAN_DELETE_ARRAY(payload);
	delete [] payload;

	MEMMAN_DELETE(audio_encoder);
	delete audio_encoder;

	// Clean up resources for 3-way conference calls
	if (media_3way_peer_tx) {
		MEMMAN_DELETE(media_3way_peer_tx);
		delete media_3way_peer_tx;
	}
	if (media_3way_peer_rx) {
		MEMMAN_DELETE(media_3way_peer_rx);
		delete media_3way_peer_rx;
	}
	if (mix_buf_3way) {
		MEMMAN_DELETE_ARRAY(mix_buf_3way);
		delete [] mix_buf_3way;
	}
	
	if (dtmf_player) {
		MEMMAN_DELETE(dtmf_player);
		delete dtmf_player;
	}
}

void t_audio_rx::set_running(bool running) {
	is_running = running;
}

// NOTE: no operations on the phone object are allowed inside the run() method.
//       Such an operation needs a lock on the transaction layer. The destructor
//       on audio_rx is called while this lock is locked. The destructor waits
//	 in a busy loop for the run() method to finish. If the run() method would
//       need the phone lock, this would lead to a dead lock (and a long trip
//	 in debug hell!)
void t_audio_rx::run(void) {
	//struct timeval debug_timer;
	unsigned short sound_payload_size;
	uint32 dtmf_rtp_timestamp;
	
	phone->add_prohibited_thread();
	ui->add_prohibited_thread();
	
	// This flag indicates if we are currently in a silence period.
	// The start of a new stream is assumed to start in silence, such
	// that the very first RTP packet will be marked.
	bool silence_period = true;
	uint64 silence_nsamples = 0; // duration in samples
	
	// This flag indicates if a sound frame can be suppressed
	bool suppress_samples = false;

	// The running flag is set already in t_audio_session::run to prevent
	// a crash when the thread gets destroyed before it starts running.
	// is_running = true;

	// For a 3-way conference only the main receiver has access
	// to the dsp.
	if (!is_3way || is_main_rx_3way) {
		// Enable recording
		if (sys_config->equal_audio_dev(sys_config->get_dev_speaker(),
				sys_config->get_dev_mic())) 
		{
			input_device->enable(true, true);
		} else {
			input_device->enable(false, true);
		}

		// If the stream is stopped for call-hold, then the buffer might
		// be filled with old sound samples.
		input_device->flush(false, true);
	}

	// Synchronize the timestamp driven by the sampling rate
	// of the recording with the timestamp of the RTP session.
	// As the RTP session is already created in advance, the
	// RTP clock is a bit ahead already.
	timestamp = rtp_session->getCurrentTimestamp() + nsamples;

	// This loop keeps running until the stop_running flag is set to true.
	// When a call is being released the stop_running flag is set to true.
	// At that moment the lock on the transaction layer (phone) is taken.
	// So do not use operations that take the phone lock, otherwise a
	// dead lock may occur during call release.
	while (true) {
		if (stop_running) break;

		if (dtmf_player) {
			rtp_session->setMark(false);
			// Skip samples from sound card
			input_device->read(input_sample_buf, SAMPLE_BUF_SIZE);
			sound_payload_size = dtmf_player->get_payload(
				payload, payload_size, timestamp, dtmf_rtp_timestamp);
			silence_period = false;
		} else if (get_dtmf_event()) {
			// RFC 2833
			// Set marker in first RTP packet of a DTMF event
			rtp_session->setMark(true);
			// Skip samples from sound card
			input_device->read(input_sample_buf, SAMPLE_BUF_SIZE);
			assert(dtmf_player);
			sound_payload_size = dtmf_player->get_payload(
				payload, payload_size, timestamp, dtmf_rtp_timestamp);
			silence_period = false;
		} else if (get_sound_samples(sound_payload_size, suppress_samples)) {
			if (suppress_samples && use_nat_keepalive) {
				if (!silence_period) silence_nsamples = 0;
				
				// Send a silence packet at the NAT keep alive interval
				// to keep the NAT bindings for RTP fresh.
				silence_nsamples += SAMPLE_BUF_SIZE / 2;
				if (silence_nsamples > 
					(uint64_t)user_config->get_timer_nat_keepalive() * 1000 *
					audio_encoder->get_sample_rate())
				{
					suppress_samples = false;
				}
			}
		
			if (silence_period && !suppress_samples) {
				// RFC 3551 4.1
				// Set marker bit in first RTP packet after silence
				rtp_session->setMark(true);
			} else {
				rtp_session->setMark(false);
			}
			silence_period = suppress_samples;
		} else {
			continue;
		}

		// If timestamp is more than 1 payload size ahead of the clock of
		// the ccRTP stack, then drop the current payload and do not advance
		// the timestamp. This will happen if the DSP delivers more
		// sound samples than the set sample rate. To compensate for this
		// samples must be dropped.
		uint32 current_timestamp = rtp_session->getCurrentTimestamp();
		if (seq32_t(timestamp) <= seq32_t(current_timestamp + nsamples)) {
			if (dtmf_player) {
				// Send DTMF payload
				rtp_session->putData(dtmf_rtp_timestamp, payload,
							sound_payload_size);

				// If DTMF has ended then set payload back to sound
				if (dtmf_player->finished()) {
					set_sound_payload_format();
					MEMMAN_DELETE(dtmf_player);
					delete dtmf_player;
					dtmf_player = NULL;
				}
			} else if (!suppress_samples) {
				// Send sound samples
				// Set the expire timeout to the jitter buffer size.
				// This allows for old packets still to be sent out.
				rtp_session->setExpireTimeout(MAX_OUT_AUDIO_DELAY_MS * 1000);
				rtp_session->putData(timestamp, payload, sound_payload_size);
			}

			timestamp += nsamples;
		} else {
			log_file->write_header("t_audio_rx::run", LOG_NORMAL, LOG_DEBUG);
			log_file->write_raw("Audio rx line ");
			log_file->write_raw(get_line()->get_line_number()+1);
			log_file->write_raw(": discarded surplus of sound samples.\n");
			log_file->write_raw("Timestamp: ");
			log_file->write_raw(timestamp);
			log_file->write_endl();
			log_file->write_raw("Current timestamp: ");
			log_file->write_raw(current_timestamp);
			log_file->write_endl();
			log_file->write_raw("nsamples: ");
			log_file->write_raw(nsamples);
			log_file->write_endl();
			log_file->write_footer();
		}

		// If there is enough data in the DSP buffers to fill another
		// RTP packet then do not sleep, but immediately go to the
		// next cycle to play out the data. Probably this thread did
		// not get enough time, so the buffer filled up. The far end
		// jitter buffer has to cope with the jitter caused by this.
		if (is_3way && !is_main_rx_3way) {
			if (media_3way_peer_rx->size_content() >= SAMPLE_BUF_SIZE) {
				continue;
			}
		} else {
			if (input_device->get_buffer_space(true) >= SAMPLE_BUF_SIZE) continue;
		}

		// There is no data left in the DSP buffers to play out anymore.
		// So the timestamp must be in sync with the clock of the ccRTP
		// stack. It might get behind if the sound cards samples a bit
		// slower than the set sample rate. Advance the timestamp to get
		// in sync again.
		current_timestamp = rtp_session->getCurrentTimestamp();
		if (seq32_t(timestamp) <= seq32_t(current_timestamp - 
			(JITTER_BUF_MS / audio_encoder->get_ptime()) * nsamples))
		{
			timestamp += nsamples * (JITTER_BUF_MS / audio_encoder->get_ptime());
			log_file->write_header("t_audio_rx::run", LOG_NORMAL, LOG_DEBUG);
			log_file->write_raw("Audio rx line ");
			log_file->write_raw(get_line()->get_line_number()+1);
			log_file->write_raw(": timestamp forwarded by ");
			log_file->write_raw(nsamples * (JITTER_BUF_MS /
					audio_encoder->get_ptime()));
			log_file->write_endl();
			log_file->write_raw("Timestamp: ");
			log_file->write_raw(timestamp);
			log_file->write_endl();
			log_file->write_raw("Current timestamp: ");
			log_file->write_raw(current_timestamp);
			log_file->write_endl();
			log_file->write_raw("nsamples: ");
			log_file->write_raw(nsamples);
			log_file->write_endl();
			log_file->write_footer();
		}			
	}

	phone->remove_prohibited_thread();
	ui->remove_prohibited_thread();
	is_running = false;
}

void t_audio_rx::set_pt_telephone_event(int pt) {
	pt_telephone_event = pt;
}

void t_audio_rx::push_dtmf(char digit, bool inband) {
	// Ignore invalid DTMF digits
	if (!VALID_DTMF_SYM(digit)) return;

	// Ignore DTMF tones in a 3-way conference
	if (is_3way) return;

	t_dtmf_event dtmf_event;
	dtmf_event.dtmf_tone = char2dtmf_ev(digit);
	dtmf_event.inband = inband;

	mtx_dtmf_q.lock();
	dtmf_queue.push(dtmf_event);
	mtx_dtmf_q.unlock();
	sema_dtmf_q.up();
}

t_line *t_audio_rx::get_line(void) const {
	return audio_session->get_line();
}

void t_audio_rx::join_3way(bool main_rx, t_audio_rx *peer_rx) {
	mtx_3way.lock();

	if (is_3way) {
		log_file->write_header("t_audio_rx::join_3way", LOG_NORMAL);
		log_file->write_raw("ERROR: audio rx line ");
		log_file->write_raw(get_line()->get_line_number()+1);
		log_file->write_raw(" - 3way is already active.\n");
		log_file->write_footer();
		mtx_3way.unlock();
		return;
	}

	// Logging
	log_file->write_header("t_audio_rx::join_3way");
	log_file->write_raw("Audio rx line ");
	log_file->write_raw(get_line()->get_line_number()+1);
	log_file->write_raw(": join 3-way.\n");
	if (main_rx) {
		log_file->write_raw("Role is: mixer.\n");
	} else {
		log_file->write_raw("Role is: non-mixing.\n");
	}
	if (peer_rx) {
		log_file->write_raw("A peer receiver already exists.\n");
	} else {
		log_file->write_raw("A peer receiver does not exist.\n");
	}
	log_file->write_footer();

	// Create media buffers for the 2 far-ends of a 3-way call.
	// The size of the media buffer is the size of the jitter buffer.
	// This allows for jitter in the RTP streams and also for
	// incompatible payload sizes. Eg. 1 far-end may send 20ms paylaods,
	// while the other sends 30ms payloads. The outgoing RTP stream might
	// even have another payload size.
	// When the data has been captured from the soundcard, it will be
	// checked if there is enough data available in the media buffers, i.e.
	// the same amount of data as captured from the soundcard for mixing.
	// If there is it will be retrieved and mixed.
	// If there isn't the captured sound will simply be sent on its own
	// to the far-end. Meanwhile the buffer will fill up with data such
	// that from the next captured sample there will be sufficient data
	// for mixing.
	media_3way_peer_tx = new t_media_buffer(
			JITTER_BUF_SIZE(audio_encoder->get_sample_rate()));
	MEMMAN_NEW(media_3way_peer_tx);
	media_3way_peer_rx = new t_media_buffer(
			JITTER_BUF_SIZE(audio_encoder->get_sample_rate()));
	MEMMAN_NEW(media_3way_peer_rx);

	// Create a mix buffer for one sample frame.
	mix_buf_3way = new unsigned char[SAMPLE_BUF_SIZE];
	MEMMAN_NEW_ARRAY(mix_buf_3way);

	peer_rx_3way = peer_rx;

	is_3way = true;
	is_main_rx_3way = main_rx;

	// Stop DTMF tones as these are not supported in a 3way
	if (dtmf_player) {
		MEMMAN_DELETE(dtmf_player);
		delete dtmf_player;
		dtmf_player = NULL;
	}

	mtx_3way.unlock();
}

void t_audio_rx::set_peer_rx_3way(t_audio_rx *peer_rx) {
	mtx_3way.lock();

	if (!is_3way) {
		mtx_3way.unlock();
		return;
	}

	// Logging
	log_file->write_header("t_audio_rx::set_peer_rx_3way");
	log_file->write_raw("Audio rx line ");
	log_file->write_raw(get_line()->get_line_number()+1);
	if (peer_rx) {
		log_file->write_raw(": set peer receiver.\n");
	} else {
		log_file->write_raw(": erase peer receiver.\n");
	}
	if (is_main_rx_3way) {
		log_file->write_raw("Role is: mixer.\n");
	} else {
		log_file->write_raw("Role is: non-mixing.\n");
	}
	log_file->write_footer();

	peer_rx_3way = peer_rx;

	mtx_3way.unlock();
}

void t_audio_rx::set_main_rx_3way(bool main_rx) {
	mtx_3way.lock();

	if (!is_3way) {
		mtx_3way.unlock();
		return;
	}

	// Logging
	log_file->write_header("t_audio_rx::set_main_rx_3way");
	log_file->write_raw("Audio rx line ");
	log_file->write_raw(get_line()->get_line_number()+1);
	if (main_rx) {
		log_file->write_raw(": change role to: mixer.\n");
	} else {
		log_file->write_raw(": change role to: non-mixing.\n");
	}
	log_file->write_footer();


	// Initialize the DSP if we become the mixer and we were not before
	if (main_rx && !is_main_rx_3way) {		
		// Enable recording
		if (sys_config->equal_audio_dev(sys_config->get_dev_speaker(),
				sys_config->get_dev_mic())) 
		{
			input_device->enable(true, true);
		} else {
			input_device->enable(false, true);
		}

		// If the stream is stopped for call-hold, then the buffer might
		// be filled with old sound samples.
		input_device->flush(false, true);
	}

	is_main_rx_3way = main_rx;

	mtx_3way.unlock();
}

void t_audio_rx::stop_3way(void) {
	mtx_3way.lock();

	if (!is_3way) {
		log_file->write_header("t_audio_rx::stop_3way");
		log_file->write_raw("ERROR: audio rx line ");
		log_file->write_raw(get_line()->get_line_number()+1);
		log_file->write_raw(" - 3way is not active.\n");
		log_file->write_footer();
		mtx_3way.unlock();
		return;
	}

	// Logging
	log_file->write_header("t_audio_rx::stop_3way");
	log_file->write_raw("Audio rx line ");
	log_file->write_raw(get_line()->get_line_number()+1);
	log_file->write_raw(": stop 3-way.\n");
	log_file->write_footer();

	is_3way = false;
	is_main_rx_3way = false;

	peer_rx_3way = NULL;

	MEMMAN_DELETE(media_3way_peer_tx);
	delete media_3way_peer_tx;
	media_3way_peer_tx = NULL;
	MEMMAN_DELETE(media_3way_peer_rx);
	delete media_3way_peer_rx;
	media_3way_peer_rx = NULL;
	MEMMAN_DELETE_ARRAY(mix_buf_3way);
	delete [] mix_buf_3way;
	mix_buf_3way = NULL;

	mtx_3way.unlock();
}

void t_audio_rx::post_media_peer_tx_3way(unsigned char *media, int len,
		unsigned short peer_sample_rate) 
{
	mtx_3way.lock();

	if (!is_3way) {
		// This is not a 3-way call. This is not necessarily an
		// error condition. The 3rd party may be in the process of
		// leaving the conference.
		// Simply discard the posted media
		mtx_3way.unlock();
		return;
	}
	
	if (peer_sample_rate != audio_encoder->get_sample_rate()) {
		// Resample media from peer to sample rate of this receiver
		int output_len = (len / 2) * audio_encoder->get_sample_rate() / peer_sample_rate;
		short *output_buf = new short[output_len];
		MEMMAN_NEW_ARRAY(output_buf);
		int resample_len = resample((short *)media, len / 2, peer_sample_rate,
					output_buf, output_len, audio_encoder->get_sample_rate());
		media_3way_peer_tx->add((unsigned char *)output_buf, resample_len * 2);
		MEMMAN_DELETE_ARRAY(output_buf);
		delete [] output_buf;
	} else {
		media_3way_peer_tx->add(media, len);
	}

	mtx_3way.unlock();
}

void t_audio_rx::post_media_peer_rx_3way(unsigned char *media, int len,
		unsigned short peer_sample_rate) 
{
	mtx_3way.lock();

	if (!is_3way) {
		// This is not a 3-way call. This is not necessarily an
		// error condition. The 3rd party may be in the process of
		// leaving the conference.
		// Simply discard the posted media
		mtx_3way.unlock();
		return;
	}
	
	if (peer_sample_rate != audio_encoder->get_sample_rate()) {
		// Resample media from peer to sample rate of this receiver
		int output_len = (len / 2) * audio_encoder->get_sample_rate() / peer_sample_rate;
		short *output_buf = new short[output_len];
		MEMMAN_NEW_ARRAY(output_buf);
		int resample_len = resample((short *)media, len / 2, peer_sample_rate,
					output_buf, output_len, audio_encoder->get_sample_rate());
		media_3way_peer_rx->add((unsigned char *)output_buf, resample_len * 2);
		MEMMAN_DELETE_ARRAY(output_buf);
		delete [] output_buf;
	} else {
		media_3way_peer_rx->add(media, len);
	}

	mtx_3way.unlock();
}

bool t_audio_rx::get_is_main_rx_3way(void) const {
	return is_main_rx_3way;
}
