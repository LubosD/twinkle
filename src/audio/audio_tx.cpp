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

#include <assert.h>
#include <iostream>
#include <cstdio>
#include <ctime>
#include <sys/types.h>
#include <sys/time.h>
#include "audio_tx.h"
#include "log.h"
#include "phone.h"
#include "userintf.h"
#include "util.h"
#include "line.h"
#include "sequence_number.h"
#include "audits/memman.h"

extern t_phone *phone;

#define SAMPLE_BUF_SIZE (MAX_PTIME * sc_sample_rate/1000 * AUDIO_SAMPLE_SIZE/8)

// Debug macro to print timestamp
#define DEBUG_TS(s)	{ gettimeofday(&debug_timer, NULL);\
			  cout << "DEBUG: ";\
			  cout << debug_timer.tv_sec * 1000 +\
			          debug_timer.tv_usec / 1000;\
			  cout << ":" << debug_timer.tv_sec * 1000 + debug_timer.tv_usec / 1000 - (debug_timer_prev.tv_sec * 1000 + debug_timer_prev.tv_usec / 1000);\
			  cout << " " << (s) << endl;\
			  debug_timer_prev = debug_timer;\
			}

//////////
// PUBLIC
//////////

t_audio_tx::t_audio_tx(t_audio_session *_audio_session,
		   t_audio_io *_playback_device, t_twinkle_rtp_session *_rtp_session,
	           t_audio_codec _codec, 
	           const map<unsigned short, t_audio_codec> &_payload2codec,
	           unsigned short _ptime)
{
	audio_session = _audio_session;
	
	user_config = audio_session->get_line()->get_user();
	assert(user_config);
	
	playback_device = _playback_device;
	rtp_session = _rtp_session;
	codec = _codec;
	sc_sample_rate = audio_sample_rate(_codec);
	payload2codec = _payload2codec;
	is_running = false;
	stop_running = false;
	
	// Create audio decoders
	map_audio_decoder[CODEC_G711_ALAW] = new t_g711a_audio_decoder(_ptime, user_config);
	MEMMAN_NEW(map_audio_decoder[CODEC_G711_ALAW]);
	
	map_audio_decoder[CODEC_G711_ULAW] = new t_g711u_audio_decoder(_ptime, user_config);
	MEMMAN_NEW(map_audio_decoder[CODEC_G711_ULAW]);
	
	map_audio_decoder[CODEC_GSM] = new t_gsm_audio_decoder(user_config);
	MEMMAN_NEW(map_audio_decoder[CODEC_GSM]);
	
#ifdef HAVE_SPEEX
	map_audio_decoder[CODEC_SPEEX_NB] = new t_speex_audio_decoder(
			t_speex_audio_decoder::MODE_NB, user_config);
	MEMMAN_NEW(map_audio_decoder[CODEC_SPEEX_NB]);

	map_audio_decoder[CODEC_SPEEX_WB] = new t_speex_audio_decoder(
			t_speex_audio_decoder::MODE_WB, user_config);
	MEMMAN_NEW(map_audio_decoder[CODEC_SPEEX_WB]);
	
	map_audio_decoder[CODEC_SPEEX_UWB] = new t_speex_audio_decoder(
			t_speex_audio_decoder::MODE_UWB, user_config);
	MEMMAN_NEW(map_audio_decoder[CODEC_SPEEX_UWB]);
#endif
#ifdef HAVE_OPUS
	map_audio_decoder[CODEC_OPUS] = new t_opus_audio_decoder(_ptime, user_config);
	MEMMAN_NEW(map_audio_decoder[CODEC_OPUS]);
#endif
#ifdef HAVE_ILBC
	map_audio_decoder[CODEC_ILBC] = new t_ilbc_audio_decoder(_ptime, user_config);
	MEMMAN_NEW(map_audio_decoder[CODEC_ILBC]);
#endif

	map_audio_decoder[CODEC_G722] = new t_g722_audio_decoder(_ptime, user_config);
	MEMMAN_NEW(map_audio_decoder[CODEC_G722]);

	map_audio_decoder[CODEC_G726_16] = new t_g726_audio_decoder(
			t_g726_audio_decoder::BIT_RATE_16, _ptime, user_config);
	MEMMAN_NEW(map_audio_decoder[CODEC_G726_16]);
	
	map_audio_decoder[CODEC_G726_24] = new t_g726_audio_decoder(
			t_g726_audio_decoder::BIT_RATE_24, _ptime, user_config);
	MEMMAN_NEW(map_audio_decoder[CODEC_G726_24]);
	
	map_audio_decoder[CODEC_G726_32] = new t_g726_audio_decoder(
			t_g726_audio_decoder::BIT_RATE_32, _ptime, user_config);
	MEMMAN_NEW(map_audio_decoder[CODEC_G726_32]);
	
	map_audio_decoder[CODEC_G726_40] = new t_g726_audio_decoder(
			t_g726_audio_decoder::BIT_RATE_40, _ptime, user_config);
	MEMMAN_NEW(map_audio_decoder[CODEC_G726_40]);
#ifdef HAVE_BCG729
	map_audio_decoder[CODEC_G729A] = new t_g729a_audio_decoder(
			_ptime, user_config);
	MEMMAN_NEW(map_audio_decoder[CODEC_G729A]);
#endif

	ptime = map_audio_decoder[codec]->get_default_ptime();

	sample_buf = new unsigned char[SAMPLE_BUF_SIZE];
	MEMMAN_NEW_ARRAY(sample_buf);

	// Create concealment buffers
	for (int i = 0; i < MAX_CONCEALMENT; i++) {
		conceal_buf[i] = new unsigned char[SAMPLE_BUF_SIZE];
		MEMMAN_NEW_ARRAY(conceal_buf[i]);
		conceal_buflen[i] = 0;
	}
	conceal_num = 0;
	conceal_pos = 0;

	// Initialize jitter buffer
	jitter_buf = new unsigned char[JITTER_BUF_SIZE(sc_sample_rate)];
	MEMMAN_NEW_ARRAY(jitter_buf);
	jitter_buf_len = 0;
	load_jitter_buf = true;
	soundcard_buf_size = playback_device->get_buffer_size(false);

	// Initialize 3-way settings
	is_3way = false;
	is_3way_mixer = false;
	media_3way_peer_tx = NULL;
	peer_tx_3way = NULL;
	peer_rx_3way = NULL;
	mix_buf_3way = NULL;
	
	// Initialize telephone event settings
	pt_telephone_event = -1;
	pt_telephone_event_alt = 1;
}

t_audio_tx::~t_audio_tx() {
	struct timespec sleeptimer;
	
	if (is_running) {
		stop_running = true;
		do {
			sleeptimer.tv_sec = 0;
			sleeptimer.tv_nsec = 10000000;
			nanosleep(&sleeptimer, NULL);
			continue;
		} while (is_running);
	}

	MEMMAN_DELETE_ARRAY(sample_buf);
	delete [] sample_buf;
	MEMMAN_DELETE_ARRAY(jitter_buf);
	delete [] jitter_buf;

	for (int i = 0; i < MAX_CONCEALMENT; i++) {
		MEMMAN_DELETE_ARRAY(conceal_buf[i]);
		delete [] conceal_buf[i];
	}
	
	// Destroy audio decoders
	for (map<t_audio_codec, t_audio_decoder *>::iterator i = map_audio_decoder.begin();
	     i != map_audio_decoder.end(); i++)
	{
		MEMMAN_DELETE(i->second);
		delete i->second;
	}

	// Cleanup 3-way resources
	if (media_3way_peer_tx) {
		MEMMAN_DELETE(media_3way_peer_tx);
		delete media_3way_peer_tx;
	}
	if (mix_buf_3way) {
		MEMMAN_DELETE_ARRAY(mix_buf_3way);
		delete [] mix_buf_3way;
	}
}

void t_audio_tx::retain_for_concealment(unsigned char *buf, unsigned short len) {
	if (conceal_num == 0) {
		memcpy(conceal_buf[0], buf, len);
		conceal_buflen[0] = len;
		conceal_num = 1;
		conceal_pos = 0;
		return;
	}

	if (conceal_num < MAX_CONCEALMENT) {
		memcpy(conceal_buf[conceal_num], buf, len);
		conceal_buflen[conceal_num] = len;
		conceal_num++;
		return;
	}

	memcpy(conceal_buf[conceal_pos], buf, len);
	conceal_buflen[conceal_pos] = len;
	conceal_pos = (conceal_pos + 1) % MAX_CONCEALMENT;
}

void t_audio_tx::conceal(short num) {
	// Some codecs have a PLC.
	// Only use this PLC is the sound card sample rate equals the codec
	// sample rate. If they differ, then we should resample the codec
	// samples. As this should be a rare case, we are lazy here. In
	// this rare case, use Twinkle's low-tech PLC.
	if (map_audio_decoder[codec]->has_plc() && audio_sample_rate(codec) == sc_sample_rate) {
		short *sb = (short *)sample_buf;
		for (int i = 0; i < num; i++) {
			int nsamples;
			nsamples = map_audio_decoder[codec]->conceal(sb, SAMPLE_BUF_SIZE);
			if (nsamples > 0) {
				play_pcm(sample_buf, nsamples * 2);
			}
		}
		
		return;
	}

	// Replay previous packets for other codecs
	short i = (conceal_pos + (MAX_CONCEALMENT - num)) % MAX_CONCEALMENT;

	if (i >= conceal_pos) {
		for (int j = i; j < MAX_CONCEALMENT; j++) {
			play_pcm(conceal_buf[j], conceal_buflen[j]);
		}

		for (int j = 0; j < conceal_pos; j++) {
			play_pcm(conceal_buf[j], conceal_buflen[j]);
		}
	} else {
		for (int j = i; j < conceal_pos; j++) {
			play_pcm(conceal_buf[j], conceal_buflen[j]);
		}
	}
}

void t_audio_tx::clear_conceal_buf(void) {
	conceal_pos = 0;
	conceal_num = 0;
}

void t_audio_tx::play_pcm(unsigned char *buf, unsigned short len, bool only_3rd_party) {
	int status;
	//struct timeval debug_timer, debug_timer_prev;

	unsigned char *playbuf = buf;

	// If there is only sound from the 3rd party in a 3-way, then check
	// if there is still enough sound in the buffer of the DSP to be
	// played. If not, then play out the sound from the 3rd party only.
	if (only_3rd_party) {
		/* Does not work on all ALSA implementations.
		if (playback_device->get_buffer_space(false) < soundcard_buf_size - len) {
		*/
		if (!playback_device->play_buffer_underrun()) {
			// There is still sound in the DSP buffers to be
			// played, so let's wait. Maybe in the next cycle
			// an RTP packet from the far-end will be received.
			return;
		}
	}

	// If we are in a 3-way then send the samples to the peer audio
	// receiver for mixing
	if (!only_3rd_party && is_3way && peer_rx_3way) {
		peer_rx_3way->post_media_peer_tx_3way(buf, len, sc_sample_rate);
	}

	// If we are in a 3-way conference and we are not the mixer then
	// send the sound samples to the mixer
	if (is_3way && !is_3way_mixer) {
		if (peer_tx_3way) {
			peer_tx_3way->post_media_peer_tx_3way(buf, len, sc_sample_rate);
			return;
		} else {
			// There is no peer.
			return;
		}
	}

	// Mix audio for 3-way conference
	if (is_3way && is_3way_mixer) {
		if (media_3way_peer_tx->get(mix_buf_3way, len)) {
			short *mix_sb = (short *)mix_buf_3way;
			short *sb = (short *)buf;
			for (int i = 0; i < len / 2; i++) {
				mix_sb[i] = mix_linear_pcm(sb[i], mix_sb[i]);
			}

			playbuf = mix_buf_3way;
		}
	}

	// Fill jitter buffer before playing
	if (load_jitter_buf) {
		if (jitter_buf_len + len < JITTER_BUF_SIZE(sc_sample_rate)) {
			memcpy(jitter_buf + jitter_buf_len, playbuf, len);
			jitter_buf_len += len;
		} else {
			// Write the contents of the jitter buffer to the DSP.
			// The buffers in the DSP will now function as jitter
			// buffer.
			status = playback_device->write(jitter_buf, jitter_buf_len);
			if (status != jitter_buf_len) {
				string msg("Writing to dsp failed: ");
				msg += get_error_str(errno);
				log_file->write_report(msg, "t_audio_tx::play_pcm",
					LOG_NORMAL, LOG_CRITICAL);
			}

			// Write passed sound samples to DSP.
			status = playback_device->write(playbuf, len);
			if (status != len) {
				string msg("Writing to dsp failed: ");
				msg += get_error_str(errno);
				log_file->write_report(msg, "t_audio_tx::play_pcm",
					LOG_NORMAL, LOG_CRITICAL);
			}

			load_jitter_buf = false;
		}

		return;
	}

	// If buffer on soundcard is empty, then the jitter buffer needs
	// to be refilled. This should only occur when no RTP packets
	// have been received for a while (silence suppression or packet loss)
	/*
	 * This code does not work on all ALSA implementations, e.g. ALSA via pulse audio
	int bufferspace = playback_device->get_buffer_space(false);
	if (bufferspace == soundcard_buf_size && len <= JITTER_BUF_SIZE(sc_sample_rate)) {
	*/
	if (playback_device->play_buffer_underrun()) {
		memcpy(jitter_buf, playbuf, len);
		jitter_buf_len = len;
		load_jitter_buf = true;
		log_file->write_header("t_audio_tx::play_pcm", LOG_NORMAL, LOG_DEBUG);
		log_file->write_raw("Audio tx line ");
		log_file->write_raw(get_line()->get_line_number()+1);
		log_file->write_raw(": jitter buffer empty.\n");
		log_file->write_footer();
		return;
	}

	// If the play-out buffer contains the maximum number of
	// packets then start skipping packets to prevent
	// unacceptable delay.
	// This can only happen if the thread did not get
	// processing time for a while and RTP packets start to
	// pile up.
	// Or if a soundcard plays out the samples at just less then
	// the requested sample rate.
	/* Not needed anymore, the ::run loop already discards incoming RTP packets
	   with a late timestamp. This seems to solve the slow soundcard problem
	   better. The solution below caused annoying ticks in the playout.
	   
	if (soundcard_buf_size - bufferspace > JITTER_BUF_SIZE + len) {
		log_file->write_header("t_audio_tx::play_pcm", LOG_NORMAL, LOG_DEBUG);
		log_file->write_raw("Audio tx line ");
		log_file->write_raw(get_line()->get_line_number()+1);
		log_file->write_raw(": jitter buffer overflow: ");
		log_file->write_raw(bufferspace);
		log_file->write_raw(" bytes.\n");
		log_file->write_footer();
		return;
	}
	*/

	// Write passed sound samples to DSP.
	status = playback_device->write(playbuf, len);
	
	if (status != len) {
		string msg("Writing to dsp failed: ");
		msg += get_error_str(errno);
		log_file->write_report(msg, "t_audio_tx::play_pcm",
			LOG_NORMAL, LOG_CRITICAL);
		return;
	}
}

void t_audio_tx::set_running(bool running) {
	is_running = running;
}

void t_audio_tx::run(void) {
	const AppDataUnit* adu;
	struct timespec sleeptimer;
	//struct timeval debug_timer, debug_timer_prev;
	int last_seqnum = -1; // seqnum of last received RTP packet
	
	// RTP packets with multiple SSRCs may be received. Each SSRC
	// represents an audio stream. Twinkle will only play 1 audio stream.
	// On a reception of a new SSRC, Twinkle will switch over to play the
	// new stream. This supports devices that change SSRC during a call.
	uint32 ssrc_current = 0;
	
	bool recvd_dtmf = false; // indicates if last RTP packets is a DTMF event

	// The running flag is set already in t_audio_session::run to prevent
	// a crash when the thread gets destroyed before it starts running.
	// is_running = true;

	uint32 rtp_timestamp = 0;
	
	// This thread may not take the lock on the transaction layer to
	// prevent dead locks
	phone->add_prohibited_thread();
	ui->add_prohibited_thread();
	
	while (true) {
		do {
			adu = NULL;
			if (stop_running) break;
			rtp_timestamp = rtp_session->getFirstTimestamp();
			adu = rtp_session->getData(
					rtp_session->getFirstTimestamp());
			if (adu == NULL || adu->getSize() <= 0) {
				// There is no packet available. This may have
				// several reasons:
				// - the thread scheduling granularity does
				//   not match ptime
				// - packet lost
				// - packet delayed
				// Wait another cycle for a packet. The
				// jitter buffer will cope with this variation.
				if (adu) {
					delete adu;
					adu = NULL;
				}

				// If we are the mixer in a 3-way call and there
				// is enough media from the other far-end then
				// this must be sent to the dsp.
				if (is_3way && is_3way_mixer &&
				    media_3way_peer_tx->size_content() >=
				    	ptime * (audio_sample_rate(codec) / 1000) * 2)
				{
					// Fill the sample buffer with silence
					int len = ptime * (audio_sample_rate(codec) / 1000) * 2;
					memset(sample_buf, 0, len);
					play_pcm(sample_buf, len, true);
				}

				// Sleep ptime ms
				sleeptimer.tv_sec = 0;

				if (ptime >= 20) {
					sleeptimer.tv_nsec =
						ptime * 1000000 - 10000000;
				} else {
					// With a thread schedule of 10ms
					// granularity, this will schedule the
					// thread every 10ms.
					sleeptimer.tv_nsec = 5000000;
				}
				nanosleep(&sleeptimer, NULL);
			}
		} while (adu == NULL || (adu->getSize() <= 0));
		
		if (stop_running) {
			if (adu) delete adu;
			break;
		}

		if (adu) {
			// adu is created by ccRTP, but we have to delete it,
			// so report it to MEMMAN
			MEMMAN_NEW(const_cast<ost::AppDataUnit*>(adu));
		}

		// Check for a codec change
		map<unsigned short, t_audio_codec>::const_iterator it_codec;
		it_codec = payload2codec.find(adu->getType());
		t_audio_codec recvd_codec = CODEC_NULL;
		if (it_codec != payload2codec.end()) {
			recvd_codec = it_codec->second;
		}
		
		// Switch over to new SSRC
		if (last_seqnum == -1 || ssrc_current != adu->getSource().getID()) {
			if (recvd_codec != CODEC_NULL) {
				ssrc_current = adu->getSource().getID();
				
				// An SSRC defines a sequence number space. So a new
				// SSRC starts with a new random sequence number
				last_seqnum = -1;
				
				log_file->write_header("t_audio_tx::run", 
					LOG_NORMAL);
				log_file->write_raw("Audio tx line ");
				log_file->write_raw(get_line()->get_line_number()+1);
				log_file->write_raw(": play SSRC ");
				log_file->write_raw(ssrc_current);
				log_file->write_endl();
				log_file->write_footer();
			} else {
				// SSRC received had an unsupported codec
				// Discard.
				// KLUDGE: for now this supports a scenario where a
				// far-end starts ZRTP negotiation by sending CN
				// packets with a separate SSRC while ZRTP is disabled
				// in Twinkle. Twinkle will then receive the CN packets
				// and discard them here as CN is an unsupported codec.
				log_file->write_header("t_audio_tx::run", 
					LOG_NORMAL, LOG_DEBUG);
				log_file->write_raw("Audio tx line ");
				log_file->write_raw(get_line()->get_line_number()+1);
				log_file->write_raw(": SSRC received (");
				log_file->write_raw(adu->getSource().getID());
				log_file->write_raw(") has unsupported codec ");
				log_file->write_raw(adu->getType());
				log_file->write_endl();
				log_file->write_footer();
				
				MEMMAN_DELETE(const_cast<ost::AppDataUnit*>(adu));
				delete adu;
				continue;
			}
		}
		
		map<t_audio_codec, t_audio_decoder *>::const_iterator it_decoder;
		it_decoder = map_audio_decoder.find(recvd_codec);
		if (it_decoder != map_audio_decoder.end()) {
			if (codec != recvd_codec) {
				codec = recvd_codec;
				get_line()->ci_set_recv_codec(codec);
				ui->cb_async_recv_codec_changed(get_line()->get_line_number(),
					codec);

				log_file->write_header("t_audio_tx::run", 
					LOG_NORMAL, LOG_DEBUG);
				log_file->write_raw("Audio tx line ");
				log_file->write_raw(get_line()->get_line_number()+1);
				log_file->write_raw(": codec change to ");
				log_file->write_raw(ui->format_codec(codec));
				log_file->write_endl();
				log_file->write_footer();
			}
		} else {
			if (adu->getType() == pt_telephone_event ||
			    adu->getType() == pt_telephone_event_alt) 
			{
				recvd_dtmf = true;
			} else {
				if (codec != CODEC_UNSUPPORTED) {
					codec = CODEC_UNSUPPORTED;
					get_line()->ci_set_recv_codec(codec);
					ui->cb_async_recv_codec_changed(
						get_line()->get_line_number(), codec);
	
					log_file->write_header("t_audio_tx::run", 
						LOG_NORMAL, LOG_DEBUG);
					log_file->write_raw("Audio tx line ");
					log_file->write_raw(get_line()->get_line_number()+1);
					log_file->write_raw(": payload type ");
					log_file->write_raw(adu->getType());
					log_file->write_raw(" not supported\n");
					log_file->write_footer();
				}
	
				last_seqnum = adu->getSeqNum();
				MEMMAN_DELETE(const_cast<ost::AppDataUnit*>(adu));
				delete adu;
				continue;
			}
		}

		// DTMF event
		if (recvd_dtmf) {
			// NOTE: the DTMF tone will be detected here
			// while there might still be data in the jitter
			// buffer. If the jitter buffer was already sent
			// to the DSP, then the DSP will continue to play
			// out the buffer sound samples.

			if (dtmf_previous_timestamp != rtp_timestamp) {
				// A new DTMF tone has been received.
				dtmf_previous_timestamp = rtp_timestamp;
				t_rtp_telephone_event *e =
					(t_rtp_telephone_event *)adu->getData();
				ui->cb_async_dtmf_detected(get_line()->get_line_number(),
					e->get_event());

				// Log DTMF event
				log_file->write_header("t_audio_tx::run");
				log_file->write_raw("Audio tx line ");
				log_file->write_raw(get_line()->get_line_number()+1);
				log_file->write_raw(": detected DTMF event - ");
				log_file->write_raw(e->get_event());
				log_file->write_endl();
				log_file->write_footer();
			}

			recvd_dtmf = false;
			last_seqnum = adu->getSeqNum();
			MEMMAN_DELETE(const_cast<ost::AppDataUnit*>(adu));
			delete adu;
			continue;
		}

		// Discard invalide payload sizes
		if (!map_audio_decoder[codec]->valid_payload_size(
				adu->getSize(), SAMPLE_BUF_SIZE / 2))
		{
			log_file->write_header("t_audio_tx::run", LOG_NORMAL, LOG_DEBUG);
			log_file->write_raw("Audio tx line ");
			log_file->write_raw(get_line()->get_line_number()+1);
			log_file->write_raw(": RTP payload size (");
			log_file->write_raw((unsigned long)(adu->getSize()));
			log_file->write_raw(" bytes) invalid for \n");
			log_file->write_raw(ui->format_codec(codec));
			log_file->write_footer();
			last_seqnum = adu->getSeqNum();
			MEMMAN_DELETE(const_cast<ost::AppDataUnit*>(adu));
			delete adu;
			continue;
		}
		
		unsigned short recvd_ptime;
		recvd_ptime = map_audio_decoder[codec]->get_ptime(adu->getSize());

		// Log a change of ptime
		if (ptime != recvd_ptime) {
			log_file->write_header("t_audio_tx::run", LOG_NORMAL, LOG_DEBUG);
			log_file->write_raw("Audio tx line ");
			log_file->write_raw(get_line()->get_line_number()+1);
			log_file->write_raw(": ptime changed from ");
			log_file->write_raw(ptime);
			log_file->write_raw(" ms to ");
			log_file->write_raw(recvd_ptime);
			log_file->write_raw(" ms\n");
			log_file->write_footer();
			ptime = recvd_ptime;
		}
		
		// Check for lost packets
		// This must be done before decoding the received samples as the
		// speex decoder has its own PLC algorithm for which it needs the decoding
		// state before decoding the new samples.
		seq16_t seq_recvd(adu->getSeqNum());
		seq16_t seq_last(static_cast<uint16>(last_seqnum));
		if (last_seqnum != -1 && seq_recvd - seq_last > 1) {
			// Packets have been lost
			uint16 num_lost = (seq_recvd - seq_last) - 1;
			log_file->write_header("t_audio_tx::run", LOG_NORMAL, LOG_DEBUG);
			log_file->write_raw("Audio tx line ");
			log_file->write_raw(get_line()->get_line_number()+1);
			log_file->write_raw(": ");
			log_file->write_raw(num_lost);
			log_file->write_raw(" RTP packets lost.\n");
			log_file->write_footer();

			if (num_lost <= conceal_num) {
				// Conceal packet loss
				conceal(num_lost);
			}
			clear_conceal_buf();
		}
		
		// Determine if resampling is needed due to dynamic change to
		// codec with other sample rate.
		short downsample_factor = 1;
		short upsample_factor = 1;
		if (audio_sample_rate(codec) > sc_sample_rate) {
			downsample_factor = audio_sample_rate(codec) / sc_sample_rate;
		} else if (audio_sample_rate(codec) < sc_sample_rate) {
			upsample_factor = sc_sample_rate / audio_sample_rate(codec);
		}
		
		// Create sample buffer. If no resampling is needed, the sample
		// buffer from the audio_tx object can be used directly.
		// Otherwise a temporary sample buffers is created that will
		// be resampled to the object's sample buffer later.
		short *sb;
		int sb_size;
		if (downsample_factor > 1) {
			sb_size = SAMPLE_BUF_SIZE / 2 * downsample_factor;
			sb = new short[sb_size];
			MEMMAN_NEW_ARRAY(sb);
		} else if (upsample_factor > 1) {
			sb_size = SAMPLE_BUF_SIZE / 2;
			sb = new short[SAMPLE_BUF_SIZE / 2];
			MEMMAN_NEW_ARRAY(sb);
		} else {
			sb_size = SAMPLE_BUF_SIZE / 2;
			sb = (short *)sample_buf;
		}
				
				
		// Decode the audio
		unsigned char *payload = const_cast<uint8 *>(adu->getData());
		short sample_size; // size in bytes
		
		sample_size = 2 * map_audio_decoder[codec]->decode(payload, adu->getSize(), sb, sb_size);
				
		// Resample if needed
		if (downsample_factor > 1) {
			short *p = sb;
			sb = (short *)sample_buf;
			for (int i = 0; i < sample_size / 2; i += downsample_factor) {
				sb[i / downsample_factor] = p[i];
			}
			MEMMAN_DELETE_ARRAY(p);
			delete [] p;
			sample_size /= downsample_factor;
		} else if (upsample_factor > 1) {
			short *p = sb;
			sb = (short *)sample_buf;
			for (int i = 0; i < sample_size / 2; i++) {
				for (int j = 0; j < upsample_factor; j++) {
					sb[i * upsample_factor + j] = p[i];
				}
			}
			MEMMAN_DELETE_ARRAY(p);
			delete [] p;
			sample_size *= upsample_factor;
		}
		
		// If the decoder deliverd 0 bytes, then it failed
		if (sample_size == 0) {
			last_seqnum = adu->getSeqNum();
			MEMMAN_DELETE(const_cast<ost::AppDataUnit*>(adu));
			delete adu;	
			continue;
		}
		
		// Discard packet if we are lacking behind. This happens if the
		// soundcard plays at a rate less than the requested sample rate.
		if (rtp_session->isWaiting(&(adu->getSource()))) {

			uint32 last_ts = rtp_session->getLastTimestamp(&(adu->getSource()));
			uint32 diff;
			
			diff = last_ts - rtp_timestamp;
			
			if (diff > (uint32_t)(JITTER_BUF_SIZE(sc_sample_rate) / AUDIO_SAMPLE_SIZE) * 8)
			{
				log_file->write_header("t_audio_tx::run", LOG_NORMAL, LOG_DEBUG);
				log_file->write_raw("Audio tx line ");
				log_file->write_raw(get_line()->get_line_number()+1);
				log_file->write_raw(": discard delayed packet.\n");
				log_file->write_raw("Timestamp: ");
				log_file->write_raw(rtp_timestamp);
				log_file->write_raw(", Last timestamp: ");
				log_file->write_raw((long unsigned int)last_ts);
				log_file->write_endl();
				log_file->write_footer();
					
				last_seqnum = adu->getSeqNum();
				MEMMAN_DELETE(const_cast<ost::AppDataUnit*>(adu));
				delete adu;
				continue;
			}
		}

		play_pcm(sample_buf, sample_size);
		retain_for_concealment(sample_buf, sample_size);
		last_seqnum = adu->getSeqNum();
		MEMMAN_DELETE(const_cast<ost::AppDataUnit*>(adu));
		delete adu;

		// No sleep is done here but in the loop waiting
		// for a new packet. If a packet is already available
		// it can be send to the sound card immediately so
		// the play-out buffer keeps filled.
		// If the play-out buffer gets empty you hear a
		// crack in the sound.


#ifdef HAVE_SPEEX		
		// store decoded output for (optional) echo cancellation 
		if (audio_session->get_do_echo_cancellation()) {
		    if (audio_session->get_echo_captured_last()) {
			speex_echo_playback(audio_session->get_speex_echo_state(), (spx_int16_t *) sb);
			audio_session->set_echo_captured_last(false);;
		    }
		}
#endif

	}

	phone->remove_prohibited_thread();
	ui->remove_prohibited_thread();
	is_running = false;
}

void t_audio_tx::set_pt_telephone_event(int pt, int pt_alt) {
	pt_telephone_event = pt;
	pt_telephone_event_alt = pt_alt;
}

t_line *t_audio_tx::get_line(void) const {
	return audio_session->get_line();
}

void t_audio_tx::join_3way(bool mixer, t_audio_tx *peer_tx, t_audio_rx *peer_rx) {
	mtx_3way.lock();

	if (is_3way) {
		log_file->write_header("t_audio_tx::join_3way");
		log_file->write_raw("ERROR: audio tx line ");
		log_file->write_raw(get_line()->get_line_number()+1);
		log_file->write_raw(" - 3way is already active.\n");
		log_file->write_footer();
		mtx_3way.unlock();
		return;
	}

	// Logging
	log_file->write_header("t_audio_tx::join_3way");
	log_file->write_raw("Audio tx line ");
	log_file->write_raw(get_line()->get_line_number()+1);
	log_file->write_raw(": join 3-way.\n");
	if (mixer) {
		log_file->write_raw("Role is: mixer.\n");
	} else {
		log_file->write_raw("Role is: non-mixing.\n");
	}
	if (peer_tx) {
		log_file->write_raw("A peer transmitter already exists.\n");
	} else {
		log_file->write_raw("A peer transmitter does not exist.\n");
	}
	if (peer_rx) {
		log_file->write_raw("A peer receiver already exists.\n");
	} else {
		log_file->write_raw("A peer receiver does not exist.\n");
	}
	log_file->write_footer();

	peer_tx_3way = peer_tx;
	peer_rx_3way = peer_rx;
	is_3way_mixer = mixer;
	is_3way = true;

	// Create buffers for mixing
	mix_buf_3way = new unsigned char[SAMPLE_BUF_SIZE];
	MEMMAN_NEW_ARRAY(mix_buf_3way);

	// See comments in audio_rx.cpp for the size of this buffer.
	media_3way_peer_tx = new t_media_buffer(JITTER_BUF_SIZE(sc_sample_rate));
	MEMMAN_NEW(media_3way_peer_tx);

	mtx_3way.unlock();
}

void t_audio_tx::set_peer_tx_3way(t_audio_tx *peer_tx) {
	mtx_3way.lock();

	if (!is_3way) {
		mtx_3way.unlock();
		return;
	}

	// Logging
	log_file->write_header("t_audio_tx::set_peer_tx_3way");
	log_file->write_raw("Audio tx line ");
	log_file->write_raw(get_line()->get_line_number()+1);
	if (peer_tx) {
		log_file->write_raw(": set peer transmitter.\n");
	} else {
		log_file->write_raw(": erase peer transmitter.\n");
	}
	if (is_3way_mixer) {
		log_file->write_raw("Role is: mixer.\n");
	} else {
		log_file->write_raw("Role is: non-mixing.\n");
	}
	log_file->write_footer();


	peer_tx_3way = peer_tx;

	mtx_3way.unlock();
}

void t_audio_tx::set_peer_rx_3way(t_audio_rx *peer_rx) {
	mtx_3way.lock();

	if (!is_3way) {
		mtx_3way.unlock();
		return;
	}

	// Logging
	log_file->write_header("t_audio_tx::set_peer_rx_3way");
	log_file->write_raw("Audio tx line ");
	log_file->write_raw(get_line()->get_line_number()+1);
	if (peer_rx) {
		log_file->write_raw(": set peer receiver.\n");
	} else {
		log_file->write_raw(": erase peer receiver.\n");
	}
	if (is_3way_mixer) {
		log_file->write_raw("Role is: mixer.\n");
	} else {
		log_file->write_raw("Role is: non-mixing.\n");
	}
	log_file->write_footer();

	peer_rx_3way = peer_rx;

	mtx_3way.unlock();
}

void t_audio_tx::set_mixer_3way(bool mixer) {
	mtx_3way.lock();

	if (!is_3way) {
		mtx_3way.unlock();
		return;
	}

	// Logging
	log_file->write_header("t_audio_tx::set_mixer_3way");
	log_file->write_raw("Audio tx line ");
	log_file->write_raw(get_line()->get_line_number()+1);
	if (mixer) {
		log_file->write_raw(": change role to: mixer.\n");
	} else {
		log_file->write_raw(": change role to: non-mixing.\n");
	}
	log_file->write_footer();

	is_3way_mixer = mixer;
	
	mtx_3way.unlock();
}

void t_audio_tx::stop_3way(void) {
	mtx_3way.lock();

	if (!is_3way) {
		log_file->write_header("t_audio_tx::stop_3way");
		log_file->write_raw("ERROR: audio tx line ");
		log_file->write_raw(get_line()->get_line_number()+1);
		log_file->write_raw(" - 3way is not active.\n");
		log_file->write_footer();
		mtx_3way.unlock();
		return;
	}

	// Logging
	log_file->write_header("t_audio_tx::stop_3way");
	log_file->write_raw("Audio tx line ");
	log_file->write_raw(get_line()->get_line_number()+1);
	log_file->write_raw(": stop 3-way.\n");
	log_file->write_footer();

	is_3way = false;
	is_3way_mixer = false;

	if (media_3way_peer_tx) {
		MEMMAN_DELETE(media_3way_peer_tx);
		delete media_3way_peer_tx;
		media_3way_peer_tx = NULL;
	}

	if (mix_buf_3way) {
		MEMMAN_DELETE_ARRAY(mix_buf_3way);
		delete [] mix_buf_3way;
		mix_buf_3way = NULL;
	}

	mtx_3way.unlock();
}

void t_audio_tx::post_media_peer_tx_3way(unsigned char *media, int len,
		unsigned short peer_sample_rate) 
{
	mtx_3way.lock();

	if (!is_3way || !is_3way_mixer) {
		mtx_3way.unlock();
		return;
	}
	
	if (peer_sample_rate != sc_sample_rate) {
		// Resample media from peer to sample rate of this transmitter
		int output_len = (len / 2) * sc_sample_rate / peer_sample_rate;
		short *output_buf = new short[output_len];
		MEMMAN_NEW_ARRAY(output_buf);
		int resample_len = resample((short *)media, len / 2, peer_sample_rate,
					output_buf, output_len, sc_sample_rate);
		media_3way_peer_tx->add((unsigned char *)output_buf, resample_len * 2);
		MEMMAN_DELETE_ARRAY(output_buf);
		delete [] output_buf;
	} else {
		media_3way_peer_tx->add(media, len);
	}

	mtx_3way.unlock();
}

bool t_audio_tx::get_is_3way_mixer(void) const {
	return is_3way_mixer;
}
