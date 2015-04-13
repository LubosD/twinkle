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

#include <assert.h>
#include "line.h"
#include "log.h"
#include "phone.h"
#include "phone_user.h"
#include "session.h"
#include "util.h"
#include "userintf.h"
#include "audits/memman.h"

extern string user_host;
extern string local_hostname;
extern t_phone *phone;

///////////
// PRIVATE
///////////

void t_session::set_recvd_codecs(t_sdp *sdp) {
	recvd_codecs.clear();
	send_ac2payload.clear();
	send_payload2ac.clear();
	list<unsigned short> payloads = sdp->get_codecs(SDP_AUDIO);
	for (list<unsigned short>::iterator i = payloads.begin(); 
	     i != payloads.end(); i++)
	{
		t_audio_codec ac = sdp->get_codec(SDP_AUDIO, *i);
		if (ac > CODEC_UNSUPPORTED) {
			recvd_codecs.push_back(ac);
			send_ac2payload[ac] = *i;
			send_payload2ac[*i] = ac;
		}
	}
}

bool t_session::is_3way(void) const {
	t_line *l = get_line();
	t_phone *p = l->get_phone();

	return p->part_of_3way(l->get_line_number());
}

t_session *t_session::get_peer_3way(void) const {
	t_line *l = get_line();
	t_phone *p = l->get_phone();

	t_line *peer_line = p->get_3way_peer_line(l->get_line_number());

	return peer_line->get_session();
}

///////////
// PUBLIC
///////////

t_session::t_session(t_dialog *_dialog, string _receive_host,
		  unsigned short _receive_port)
{
	dialog = _dialog;
	
	user_config = dialog->get_line()->get_user();
	assert(user_config);

	receive_host = _receive_host;
	retrieve_host = _receive_host;
	receive_port = _receive_port;
	src_sdp_version = int2str(rand());
	src_sdp_id = int2str(rand());
	use_codec = CODEC_NULL;
	
	switch (user_config->get_dtmf_transport()) {
	case DTMF_RFC2833:
	case DTMF_AUTO:
		recv_dtmf_pt = user_config->get_dtmf_payload_type();
		break;
	default:
		recv_dtmf_pt = 0;
	}
	
	send_dtmf_pt = 0;

	offer_codecs = user_config->get_codecs();
	ptime = user_config->get_ptime();
	ilbc_mode = user_config->get_ilbc_mode();

	recvd_offer = false;
	recvd_answer = false;
	sent_offer = false;
	direction = SDP_SENDRECV;

	audio_rtp_session = NULL;
	is_on_hold = false;
	is_killed = false;
	
	// Initialize audio codec to payload mappings
	recv_ac2payload[CODEC_G711_ULAW] = SDP_FORMAT_G711_ULAW;
	recv_ac2payload[CODEC_G711_ALAW] = SDP_FORMAT_G711_ALAW;
	recv_ac2payload[CODEC_GSM] = SDP_FORMAT_GSM;
	recv_ac2payload[CODEC_SPEEX_NB] = user_config->get_speex_nb_payload_type();
	recv_ac2payload[CODEC_SPEEX_WB] = user_config->get_speex_wb_payload_type();
	recv_ac2payload[CODEC_SPEEX_UWB] = user_config->get_speex_uwb_payload_type();
	recv_ac2payload[CODEC_ILBC] = user_config->get_ilbc_payload_type();
	recv_ac2payload[CODEC_G726_16] = user_config->get_g726_16_payload_type();
	recv_ac2payload[CODEC_G726_24] = user_config->get_g726_24_payload_type();
	recv_ac2payload[CODEC_G726_32] = user_config->get_g726_32_payload_type();
	recv_ac2payload[CODEC_G726_40] = user_config->get_g726_40_payload_type();
	recv_ac2payload[CODEC_TELEPHONE_EVENT] = user_config->get_dtmf_payload_type();
	send_ac2payload.clear();
	
	// Initialize pauload to audio codec mappings
	recv_payload2ac[SDP_FORMAT_G711_ULAW] = CODEC_G711_ULAW;
	recv_payload2ac[SDP_FORMAT_G711_ALAW] = CODEC_G711_ALAW;
	recv_payload2ac[SDP_FORMAT_GSM] = CODEC_GSM;
	recv_payload2ac[user_config->get_speex_nb_payload_type()] = CODEC_SPEEX_NB;
	recv_payload2ac[user_config->get_speex_wb_payload_type()] = CODEC_SPEEX_WB;
	recv_payload2ac[user_config->get_speex_uwb_payload_type()] = CODEC_SPEEX_UWB;
	recv_payload2ac[user_config->get_ilbc_payload_type()] = CODEC_ILBC;
	recv_payload2ac[user_config->get_g726_16_payload_type()] = CODEC_G726_16;
	recv_payload2ac[user_config->get_g726_24_payload_type()] = CODEC_G726_24;
	recv_payload2ac[user_config->get_g726_32_payload_type()] = CODEC_G726_32;
	recv_payload2ac[user_config->get_g726_40_payload_type()] = CODEC_G726_40;
	recv_payload2ac[user_config->get_dtmf_payload_type()] = CODEC_TELEPHONE_EVENT;
	send_payload2ac.clear();
}

t_session::~t_session() {
	stop_rtp();
}

t_session *t_session::create_new_version(void) const {
	t_session *s = new t_session(*this);
	MEMMAN_NEW(s);
	s->src_sdp_version = int2str(atoi(src_sdp_version.c_str()) + 1);
	s->recvd_codecs.clear();
	s->recvd_offer = false;
	s->recvd_answer = false;
	s->sent_offer = false;

	// Do not copy the RTP session
	s->set_audio_session(NULL);
	
	// Clear the codec to payload mappings as a new response must
	// be received from the far end
	s->send_ac2payload.clear();
	s->send_payload2ac.clear();

	return s;
}

t_session *t_session::create_call_hold(void) const {
	t_session *s = create_new_version();

	if (user_config->get_hold_variant() == HOLD_RFC2543) {
		s->receive_host = "0.0.0.0";
	} else if (user_config->get_hold_variant() == HOLD_RFC3264) {
		// RFC 3264 8.4
		if (direction == SDP_SENDRECV) {
			s->direction = SDP_SENDONLY;
		}
		else if (direction == SDP_RECVONLY) {
			s->direction = SDP_INACTIVE;
		}
	} else {
		assert(false);
	}
	
	// Prevent RTP from being started for this session as long
	// as the call is put on hold. Without this, the RTP sessions
	// will get started when a re-INVITE is received from the far-end
	// while the call is still locally on-hold.
	s->hold();

	return s;
}

t_session *t_session::create_call_retrieve(void) const {
	t_session *s = create_new_version();

	if (user_config->get_hold_variant() == HOLD_RFC2543) {
		s->receive_host = retrieve_host;
	} else if (user_config->get_hold_variant() == HOLD_RFC3264) {
		// RFC 3264 8.4
		if (direction == SDP_SENDONLY) {
			s->direction = SDP_SENDRECV;
		}
		else if (direction == SDP_INACTIVE) {
			s->direction = SDP_RECVONLY;
		}
	} else {
		assert(false);
	}

	return s;
}

t_session *t_session::create_clean_copy(void) const {
	t_session *s = new t_session(*this);
	MEMMAN_NEW(s);
	s->src_sdp_version = int2str(atoi(src_sdp_version.c_str()) + 1);
	s->dst_sdp_version = "";
	s->dst_sdp_id = "";
	s->dst_rtp_host = "";
	s->dst_rtp_port = 0;
	s->recvd_codecs.clear();
	s->recvd_offer = false;
	s->recvd_answer = false;
	s->sent_offer = false;
	s->direction = SDP_SENDRECV;

	// Do not copy the RTP session
	s->set_audio_session(NULL);
	
	// Clear the codec to payload mappings as a new response must
	// be received from the far end
	s->send_ac2payload.clear();
	s->send_payload2ac.clear();

	return s;
}

bool t_session::process_sdp_offer(t_sdp *sdp, int &warn_code,
		string &warn_text)
{
	if (!sdp->is_supported(warn_code, warn_text)) return false;

	dst_sdp_version = sdp->origin.session_version;
	dst_sdp_id = sdp->origin.session_id;
	recvd_sdp_offer = *sdp;
	
	// RFC 3264 5
	// SDP may contain 0 m= lines
	if (sdp->media.empty()) return true;
	
	dst_rtp_host = sdp->get_rtp_host(SDP_AUDIO);
	dst_rtp_port = sdp->get_rtp_port(SDP_AUDIO);
	set_recvd_codecs(sdp);
	dst_zrtp_support = sdp->get_zrtp_support(SDP_AUDIO);

	// The direction in the SDP is from the point of view of the
	// far end. Swap the direction to store it as the point of view
	// from the near end.
	switch(sdp->get_direction(SDP_AUDIO)) {
	case SDP_INACTIVE:
		direction = SDP_INACTIVE;
		break;
	case SDP_SENDONLY:
		if (is_on_hold && user_config->get_hold_variant() == HOLD_RFC3264) {
			// The phone is put on-hold. We don't want to
			// receive media.
			direction = SDP_INACTIVE;
		} else {
			direction = SDP_RECVONLY;
		}
		break;
	case SDP_RECVONLY:
		direction = SDP_SENDONLY;
		break;
	case SDP_SENDRECV:
		if (is_on_hold && user_config->get_hold_variant() == HOLD_RFC3264) {
			// The phone is put on-hold. We don't want to
			// receive media.
			direction = SDP_SENDONLY;
		} else {
			direction = SDP_SENDRECV;
		}
		break;
	default:
		assert(false);
	}

	// Check if the list of received codecs has at least 1 codec
	// in common with the list of codecs we can offer. If there
	// is no common codec, then no call can be established.
	list<t_audio_codec>::iterator supported_codec_it = offer_codecs.end();
	for (list<t_audio_codec>::const_iterator i = recvd_codecs.begin();
	     i != recvd_codecs.end(); i++)
	{
		list<t_audio_codec>::iterator tmp_it;
		if ((supported_codec_it == offer_codecs.end() ||
		     !user_config->get_in_obey_far_end_codec_pref()) &&
		    (tmp_it = std::find(offer_codecs.begin(), supported_codec_it, *i)) !=
					supported_codec_it)
		{
			// Codec supported
			supported_codec_it = tmp_it;
			use_codec = *i; // this codec goes into answer
			
			// Use the payload to codec bindings as signalled in the
			// offer by the far end.
			recv_payload2ac[send_ac2payload[use_codec]] = use_codec;
			recv_ac2payload[use_codec] = send_ac2payload[use_codec];
		} else if (*i == CODEC_TELEPHONE_EVENT) {
			// telephone-event payload is supported
			send_dtmf_pt = send_ac2payload[*i];
			
			// When we support RFC 2833 events, then take the payload
			// type from the far end.
			if (recv_dtmf_pt > 0) {
				recv_dtmf_pt = send_dtmf_pt; // this goes into answer as well
			}
		}
	}

	if (supported_codec_it == offer_codecs.end()) {
		warn_code = W_305_INCOMPATIBLE_MEDIA_FORMAT;
		warn_text = "None of the audio codecs is supported";
		return false;
	}

	// Overwrite ptime value with ptime from SDP
	unsigned short p = sdp->get_ptime(SDP_AUDIO);
	if (p > 0) ptime = p;
	
	// RFC 3952 5
	// Select the iLBC mode that needs the lowest bandwidth
	if (use_codec == CODEC_ILBC) {
		int recvd_mode = sdp->get_fmtp_int_param(SDP_AUDIO, 
				send_ac2payload[use_codec], "mode");
		if (recvd_mode == -1) recvd_mode = 30;
		if (VALID_ILBC_MODE(recvd_mode) && recvd_mode > ilbc_mode) {
			ilbc_mode = static_cast<unsigned short>(recvd_mode);
		}
	}

	return true;
}

bool t_session::process_sdp_answer(t_sdp *sdp, int &warn_code,
		string &warn_text)
{
	if (!sdp->is_supported(warn_code, warn_text)) return false;
	
	// As our offer always contains an audio m= line, the answer
	// should contain one as well. If there are media lines, then
	// the sdp->is_supported already verified there is audio.
	if (sdp->media.empty()) {
		warn_code = W_304_MEDIA_TYPE_NOT_AVAILABLE;
		warn_text = "Valid media stream for audio is missing";
		return false;
	}

	dst_sdp_version = sdp->origin.session_version;
	dst_sdp_id = sdp->origin.session_id;
	dst_rtp_host = sdp->get_rtp_host(SDP_AUDIO);
	dst_rtp_port = sdp->get_rtp_port(SDP_AUDIO);
	dst_zrtp_support = sdp->get_zrtp_support(SDP_AUDIO);
	set_recvd_codecs(sdp);

	// Find the first codec in the received codecs list that
	// is supported.
	// Per the offer/answer model all received codecs should be
	// supported! It seems that some applications put more codecs
	// in the answer though.
	list<t_audio_codec>::iterator codec_found_it = offer_codecs.end();

	for (list<t_audio_codec>::const_iterator i = recvd_codecs.begin();
	     i != recvd_codecs.end(); i++)
	{
		list<t_audio_codec>::iterator tmp_it;
		if ((codec_found_it == offer_codecs.end() ||
		     !user_config->get_out_obey_far_end_codec_pref()) &&
		    (tmp_it = std::find(offer_codecs.begin(), codec_found_it, *i)) !=
				codec_found_it)
		{
			codec_found_it = tmp_it;
			use_codec = *i;
		} else if (*i == CODEC_TELEPHONE_EVENT) {
			// telephone-event payload is supported
			send_dtmf_pt = send_ac2payload[*i];
		}
	}

	if (codec_found_it == offer_codecs.end()) {
		// None of the answered codecs is supported
		warn_code = W_305_INCOMPATIBLE_MEDIA_FORMAT;
		warn_text = "None of the codecs is supported";
		return false;
	}

	// Overwrite ptime value with ptime from SDP
	unsigned short p = sdp->get_ptime(SDP_AUDIO);
	if (p > 0) ptime = p;
	
	// RFC 3952 5
	// Select the iLBC mode that needs the lowest bandwidth
	if (use_codec == CODEC_ILBC) {
		int recvd_mode = sdp->get_fmtp_int_param(SDP_AUDIO, 
				send_ac2payload[use_codec], "mode");
		if (recvd_mode == -1) recvd_mode = 30;
		if (VALID_ILBC_MODE(recvd_mode) && recvd_mode > ilbc_mode) {
			ilbc_mode = static_cast<unsigned short>(recvd_mode);
		}
	}

	return true;
}

void t_session::create_sdp_offer(t_sip_message *m, const string &user) {
	// Delete old body if present
	if (m->body) {
		MEMMAN_DELETE(m->body);
		delete m->body;
	}
	
	// Determine the IP address to receive the media streams
	if (receive_host == AUTO_IP4_ADDRESS) {
		unsigned local_ip = m->get_local_ip();
		if (local_ip == 0) {
			log_file->write_report("Cannot determine local IP address.",
				"t_session::create_sdp_offer", LOG_NORMAL, LOG_CRITICAL);
		} else {
			receive_host = USER_HOST(user_config, h_ip2str(local_ip));
			retrieve_host = receive_host;
		}
	}

	m->body = new t_sdp(user, src_sdp_id, src_sdp_version, receive_host, 
			receive_host, receive_port, offer_codecs, recv_dtmf_pt,
			recv_ac2payload);
	MEMMAN_NEW(m->body);


	// Set ptime for G711/G726 codecs
	list<t_audio_codec>::iterator it_g7xx;
	it_g7xx = find(offer_codecs.begin(), offer_codecs.end(), CODEC_G711_ALAW);
	if (it_g7xx == offer_codecs.end()) {
		it_g7xx = find(offer_codecs.begin(), offer_codecs.end(), CODEC_G711_ULAW);
	}
	if (it_g7xx == offer_codecs.end()) {
		it_g7xx = find(offer_codecs.begin(), offer_codecs.end(), CODEC_G726_16);
	}
	if (it_g7xx == offer_codecs.end()) {
		it_g7xx = find(offer_codecs.begin(), offer_codecs.end(), CODEC_G726_24);
	}
	if (it_g7xx == offer_codecs.end()) {
		it_g7xx = find(offer_codecs.begin(), offer_codecs.end(), CODEC_G726_32);
	}
	if (it_g7xx == offer_codecs.end()) {
		it_g7xx = find(offer_codecs.begin(), offer_codecs.end(), CODEC_G726_40);
	}
	if (it_g7xx != offer_codecs.end()) {
		((t_sdp *)m->body)->set_ptime(SDP_AUDIO, ptime);
	}
	
	// Set mode for iLBC codecs
	list<t_audio_codec>::iterator it_ilbc;
	it_ilbc = find(offer_codecs.begin(), offer_codecs.end(), CODEC_ILBC);
	if (it_ilbc != offer_codecs.end() && ilbc_mode != 30) {
		((t_sdp *)m->body)->set_fmtp_int_param(SDP_AUDIO, recv_ac2payload[CODEC_ILBC],
				"mode", ilbc_mode);
	}

	// Set direction
	if (direction != SDP_SENDRECV) {
		((t_sdp *)m->body)->set_direction(SDP_AUDIO, direction);
	}
	
	// Set zrtp support
	if (user_config->get_zrtp_enabled() && user_config->get_zrtp_sdp()) {
		((t_sdp *)m->body)->set_zrtp_support(SDP_AUDIO);
	}

	m->hdr_content_type.set_media(t_media("application", "sdp"));

	sent_offer = true;
}

void t_session::create_sdp_answer(t_sip_message *m, const string &user) {
	// Delete old body if present
	if (m->body) {
		MEMMAN_DELETE(m->body);
		delete m->body;
	}
	
	// Determine the IP address to receive the media streams
	if (receive_host == AUTO_IP4_ADDRESS) {
		unsigned long local_ip = 0;
		unsigned long dst_ip = gethostbyname(dst_rtp_host);
		
		if (dst_ip != 0)
		{
			// Determine source IP address for RTP from the
			// destination RTP IP address.
			log_file->write_report("Cannot determine local IP address from RTP destination.",
				"t_session::create_sdp_answer", LOG_NORMAL, LOG_WARNING);
				
			local_ip = get_src_ip4_address_for_dst(dst_ip);
		}
		else
		{
			string log_msg = "Cannot determine IP address for: ";
			log_msg += dst_rtp_host;
			log_file->write_report(log_msg,
				"t_session::create_sdp_answer", LOG_NORMAL, LOG_WARNING);
		}
		
		if (local_ip == 0)
		{
			// Somehow the source IP address could not be determined
			// from the destination RTP address. Try to determine it
			// from the destination of the SIP message.
			local_ip = m->get_local_ip();
		}
			
		if (local_ip == 0) {
			log_file->write_report("Cannot determine local IP address.",
				"t_session::create_sdp_answer", LOG_NORMAL, LOG_CRITICAL);
		} else {
			receive_host = USER_HOST(user_config, h_ip2str(local_ip));
			retrieve_host = receive_host;
		}
	}

	list<t_audio_codec> answer_codecs;
	answer_codecs.push_back(use_codec);

	// RFC 3264 6
	// The answer must contain an m-line for each m-line in the offer in
	// the same order. Media can be rejected by setting the port to 0.
	// Only the first audio stream is accepted, all other media streams
	// will be rejected.
	m->body = new t_sdp(user, src_sdp_id, src_sdp_version, receive_host,
				receive_host);
	MEMMAN_NEW(m->body);
	bool audio_answered = false;
	for (list<t_sdp_media>::const_iterator i = recvd_sdp_offer.media.begin();
	     i != recvd_sdp_offer.media.end(); i++)
	{
		if (!audio_answered && i->get_media_type() == SDP_AUDIO &&
		    i->port != 0)
		{
			// Accept the first audio stream
			((t_sdp *)m->body)->add_media(t_sdp_media(
				SDP_AUDIO, receive_port, answer_codecs, recv_dtmf_pt,
				send_ac2payload));
			audio_answered = true;
		}
		else
		{
			// Reject media stream by setting port to zero
			t_sdp_media reject_media(*i);
			reject_media.port = 0;
			((t_sdp *)m->body)->add_media(reject_media);
		}
	}
	
	m->hdr_content_type.set_media(t_media("application", "sdp"));
	
	// If there were no media lines in the offer, we sent no media
	// lines in the answer
	if (recvd_sdp_offer.media.empty()) return;

	// Set audio attributes
	
	// Set ptime for G711 codecs
	if (use_codec == CODEC_G711_ALAW ||
	    use_codec == CODEC_G711_ULAW)
	{
		((t_sdp *)m->body)->set_ptime(SDP_AUDIO, ptime);
	}

	// Set mode for iLBC codecs
	if (use_codec == CODEC_ILBC && ilbc_mode != 30) {
		unsigned short ilbc_payload = const_cast<t_session *>(this)->
				recv_ac2payload[CODEC_ILBC];
		((t_sdp *)m->body)->set_fmtp_int_param(SDP_AUDIO, ilbc_payload,
				"mode", ilbc_mode);
	}
	
	// Set direction
	if (direction != SDP_SENDRECV) {
		((t_sdp *)m->body)->set_direction(SDP_AUDIO, direction);
	}
	
	// Set zrtp support
	if (user_config->get_zrtp_enabled() && user_config->get_zrtp_sdp()) {
		((t_sdp *)m->body)->set_zrtp_support(SDP_AUDIO);
	}
}

void t_session::start_rtp(void) {
	// If a session is killed, it may not be started again.
	if (is_killed) {
		log_file->write_report("Cannot start. The session is killed already.",
			"t_session::start_rtp", LOG_NORMAL, LOG_DEBUG);
		return;
	}
	
	// If a session is on-hold then do not start RTP.
	if (is_on_hold) {
		log_file->write_report("Cannot start. The session is on hold.",
			"t_session::start_rtp", LOG_NORMAL, LOG_DEBUG);
		return;
	}

	if (receive_host.empty()) {
		log_file->write_report("Cannot start. receive_host is empty.",
			"t_session::start_rtp", LOG_NORMAL, LOG_DEBUG);
		return;
	}
	
	if (dst_rtp_host.empty()) {
		log_file->write_report("Cannot start. dst_rtp_host is empty.",
			"t_session::start_rtp", LOG_NORMAL, LOG_DEBUG);
		return;
	}

	// Local and remote hold
	if (((receive_host == "0.0.0.0" || receive_port == 0) &&
	     (dst_rtp_host == "0.0.0.0" || dst_rtp_port == 0)) ||
 	    direction == SDP_INACTIVE)
	{
		log_file->write_report("Cannot start. Local and remote on hold.",
			"t_session::start_rtp", LOG_NORMAL, LOG_DEBUG);
		return;
	}

	// Inform user about the codecs
	get_line()->ci_set_send_codec(use_codec);
	get_line()->ci_set_recv_codec(use_codec);
	ui->cb_send_codec_changed(get_line()->get_line_number(), use_codec);
	ui->cb_recv_codec_changed(get_line()->get_line_number(), use_codec);
	
	// Determine ptime
	unsigned short audio_ptime;
	if (use_codec == CODEC_ILBC) {
		audio_ptime = ilbc_mode;
	} else {
		audio_ptime = ptime;
	}
	
	// Determine if audio must be encrypted
	bool encrypt_audio = get_line()->get_try_to_encrypt();
	if (user_config->get_zrtp_send_if_supported()) {
		encrypt_audio = encrypt_audio && dst_zrtp_support;
	}

	// Start the RTP streams
	if (dst_rtp_host == "0.0.0.0" || dst_rtp_port == 0 ||
	    direction == SDP_RECVONLY)
	{
		// Local hold -> do not send RTP
		log_file->write_report("Local hold. Do not send RTP.",
			"t_session::start_rtp", LOG_NORMAL, LOG_DEBUG);
		audio_rtp_session = new t_audio_session(this,
				"0.0.0.0", get_line()->get_rtp_port(), "", 0, use_codec, 
				audio_ptime, recv_payload2ac, send_ac2payload,
				encrypt_audio);
		MEMMAN_NEW(audio_rtp_session);
	}
	else if (receive_host == "0.0.0.0" || receive_port == 0 ||
	         direction == SDP_SENDONLY)
	{
		// Remote hold
		// For music on-hold music should be played here.
		// Without music on-hold do not send out RTP
		/*
		audio_rtp_session = new t_audio_session(this,
				"", 0, dst_rtp_host, dst_rtp_port, codec, ptime);
		*/
		log_file->write_report("Do not start. Remote hold.",
			"t_session::start_rtp", LOG_NORMAL, LOG_DEBUG);
		return;
	} else {
		// Bi-directional audio
		audio_rtp_session = new t_audio_session(this,
				"0.0.0.0", get_line()->get_rtp_port(),
				dst_rtp_host, dst_rtp_port, use_codec, audio_ptime,
				recv_payload2ac, send_ac2payload,
				encrypt_audio);
		MEMMAN_NEW(audio_rtp_session);
	}

	// Check if the created audio session is valid.
	if (!audio_rtp_session->is_valid()) {
		log_file->write_report("Audio session is invalid.",
			"t_session::start_rtp", LOG_NORMAL, LOG_CRITICAL);
		MEMMAN_DELETE(audio_rtp_session);
		delete audio_rtp_session;
		audio_rtp_session = NULL;
		return;
	}

	// Set dynamic payload type for DTMF events
	if (recv_dtmf_pt > 0) {
		unsigned short alt_dtmf_pt;
		if (recv_payload2ac.find(send_dtmf_pt) == recv_payload2ac.end()) {
			// Allow the payload type as signalled by the far end
			// as an alternative to the payload as signalled by Twinkle.
			alt_dtmf_pt = send_dtmf_pt;
		} else {
			// The payload type as signalled by the far end for DTMF
			// is already in use by Twinkle for another codec, so it
			// cannot be used as an alternative.
			alt_dtmf_pt = recv_dtmf_pt;
		}
		audio_rtp_session->set_pt_in_dtmf(recv_dtmf_pt, alt_dtmf_pt);
	}

	if (send_dtmf_pt > 0) {
		audio_rtp_session->set_pt_out_dtmf(send_dtmf_pt);
		
		switch (user_config->get_dtmf_transport()) {
		case DTMF_AUTO:
		case DTMF_RFC2833:
			get_line()->ci_set_dtmf_supported(true, false);
			break;
		case DTMF_INBAND:
			get_line()->ci_set_dtmf_supported(true, true);
			break;
		case DTMF_INFO:
			get_line()->ci_set_dtmf_supported(true, false, true);
			break;
		default:
			assert(false);
		}
		
		ui->cb_dtmf_supported(get_line()->get_line_number());
	} else {
		switch (user_config->get_dtmf_transport()) {
		case DTMF_AUTO:
		case DTMF_INBAND:
			get_line()->ci_set_dtmf_supported(true, true);
			ui->cb_dtmf_supported(get_line()->get_line_number());
			break;
		case DTMF_RFC2833:
			get_line()->ci_set_dtmf_supported(false);
			ui->cb_dtmf_not_supported(get_line()->get_line_number());
			break;
		case DTMF_INFO:
			get_line()->ci_set_dtmf_supported(true, false, true);
			ui->cb_dtmf_supported(get_line()->get_line_number());
			break;
		default:
			assert(false);
		}
	}

	audio_rtp_session->run();
}

void t_session::stop_rtp(void) {
	if (audio_rtp_session) {
		MEMMAN_DELETE(audio_rtp_session);
		delete audio_rtp_session;
		audio_rtp_session = NULL;
	
		get_line()->ci_set_dtmf_supported(false);
		ui->cb_line_state_changed();
	}
}

void t_session::kill_rtp(void) {
	stop_rtp();
	is_killed = true;
}

t_audio_session *t_session::get_audio_session(void) const {
	return audio_rtp_session;
}

void t_session::set_audio_session(t_audio_session *as) {
	audio_rtp_session = as;
}

bool t_session::equal_audio(const t_session &s) const {
	// According to RFC 3264 6, the SDP version in the o= line
	// must be updated when the SDP is changed.
	// We check for more changes to interoperate with SIP
	// devices that do not adhere fully to RFC 3264
	return (receive_host == s.receive_host &&
		receive_port == s.receive_port &&
		dst_rtp_host == s.dst_rtp_host &&
		dst_rtp_port == s.dst_rtp_port &&
		direction == s.direction &&
		src_sdp_version == s.src_sdp_version &&
		dst_sdp_version == s.dst_sdp_version &&
		src_sdp_id == s.src_sdp_id &&
		dst_sdp_id == s.dst_sdp_id);
}

void t_session::send_dtmf(char digit, bool inband) {
	if (audio_rtp_session) audio_rtp_session->send_dtmf(digit, inband);
}

t_line *t_session::get_line(void) const {
	return dialog->get_line();
}

void t_session::set_owner(t_dialog *d) {
	dialog = d;
}

void t_session::hold(void) {
	is_on_hold = true;
}

void t_session::unhold(void) {
	is_on_hold = false;
}

bool t_session::is_rtp_active(void) const {
	return (audio_rtp_session != NULL);
}
