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

// Session description of an established session.
// A session is the media part of a dialog.

#ifndef _SESSION_H
#define _SESSION_H

#include <list>
#include <map>
#include <string>
#include "dialog.h"
#include "user.h"
#include "sdp/sdp.h"
#include "parser/sip_message.h"
#include "audio/audio_codecs.h"
#include "audio/audio_session.h"

// Forward declarations
class t_dialog;
class t_line;

using namespace std;

class t_session {
private:
	// The owning dialog
	t_dialog		*dialog;
	
	// User profile of user for which the session is created.
	// This is a pointer to the user_config owned by a phone user.
	// So this pointer should never be deleted.
	t_user			*user_config;

	// Copy of host needed for call-retrieve after call-hold
	string			retrieve_host;

	// Audio RTP session
	t_audio_session		*audio_rtp_session;

	// Indicates if session is put on-hold, i.e. no RTP should be sent
	// or received for this session.
	bool			is_on_hold;
	
	// Indicates if a session is killed, i.e. RTP will never be
	// sent or received anymore.
	bool			is_killed;
	
	// Mapping from audio codecs to RTP payload numbers for receiving
	// and sending directions.
	map<t_audio_codec, unsigned short>	recv_ac2payload;
	map<t_audio_codec, unsigned short>	send_ac2payload;
	
	// Mapping from RTP payload numbers to audio codecs for receiving
	// and sending directions.
	map<unsigned short, t_audio_codec>	recv_payload2ac;
	map<unsigned short, t_audio_codec>	send_payload2ac;
	
	// Set the list of received codecs from the SDP.
	// Create the send_ac2paylaod and send_payload2ac mappings.
	void set_recvd_codecs(t_sdp *sdp);
	
	// Returns if this session is part of a 3-way conference
	bool is_3way(void) const;
	
	// Returns the peer session of a 3-way conference
	t_session *get_peer_3way (void) const;

public:
	// Audio session information

	// Near end information
	string			src_sdp_version;
	string			src_sdp_id;
	string			receive_host; // RTP receive host address
	unsigned short		receive_port; // RTP receive port

	// Far end information
	string			dst_sdp_version;
	string			dst_sdp_id;
	string			dst_rtp_host;
	unsigned short		dst_rtp_port;
	bool			dst_zrtp_support;

	// Direction of the audio stream from this phone's point of view
	t_sdp_media_direction	direction;

	list<t_audio_codec>	offer_codecs;	// codecs to offer in outgoing INVITE
	list<t_audio_codec>	recvd_codecs;	// codecs received from far-end
	t_audio_codec		use_codec;	// codec to be used
	unsigned short		ptime;		// payload size (ms)
	unsigned short		ilbc_mode;	// 20 or 30 ms
	bool			recvd_offer;  	// offer received?
	bool			recvd_answer; 	// answer received?
	bool			sent_offer;	// offer sent?
	unsigned short		recv_dtmf_pt;	// payload type for DTMF receiving
	unsigned short		send_dtmf_pt;	// payload type for DTMF sending
	t_sdp			recvd_sdp_offer;

	t_session(t_dialog *_dialog, string _receive_host,
		  unsigned short _receive_port);

	// The destructor will destroy the RTP session and stop the
	// RTP streams
	~t_session();

	/** @name Clone a new session from an existing session. */
	//@{
	/** @note copies of a session do not copy the audio RTP session! */

	/**
	 * Create a session based on an existing session, i.e.
	 * same receive user and host. The source SDP version of the
	 * new session will be increased by 1.
	 * @return The new session.
	 */
	t_session *create_new_version(void) const;

	/**
	 * Create a copy of the session. The destination paramters
	 * and recvd/offer and answer are erased in the copy.
	 * The source SDP version of the new session will be increased by 1.
	 * @return The new session.
	 */
	t_session *create_clean_copy(void) const;

	/**
	 * Create a session for call-hold.
	 * @return The call-hold session.
	 */
	t_session *create_call_hold(void) const;

	/**
	 * Create a session for call-retrieve.
	 * @return The call-retrieve session.
	 */
	t_session *create_call_retrieve(void) const;
	//@}

	// Process incoming SDP offer. Return false if SDP is not
	// supported. If SDP is supported then use_codec will be
	// set to the first codec in the received offer that is
	// supported by this phone, i.e. this is the codec that should
	// be put in the answer.
	bool process_sdp_offer(t_sdp *sdp, int &warn_code, string &warn_text);

	// Process incoming SDP answer. Return false if SDP is not
	// supported. It is expected that the answer contains 1 codec
	// only. If more codecs are answered, then only the first supported
	// codec is considered.
	bool process_sdp_answer(t_sdp *sdp, int &warn_code, string &warn_text);

	// Create an SDP offer body for a SIP message
	void create_sdp_offer(t_sip_message *m, const string &user);

	// Create an SDP answer body for a SIP message
	void create_sdp_answer(t_sip_message *m, const string &user);

	// Start/stop the RTP streams
	// When a session is on-hold then start_rtp simply returns.
	void start_rtp(void);
	void stop_rtp(void);
	
	// Kill RTP streams. The difference with stopping an RTP stream
	// is that it cannot be started after being killed.
	void kill_rtp(void);

	t_audio_session *get_audio_session(void) const;
	void set_audio_session(t_audio_session *as);

	// Check if two session are equal wrt the audio parameters
	bool equal_audio(const t_session &s) const;

	// Send DTMF digit
	void send_dtmf(char digit, bool inband);

	// Get the line that belongs to this session
	t_line *get_line(void) const;

	// Transfer ownership of this session to a new dialog
	void set_owner(t_dialog *d);

	// Hold/un-hold a session
	// These methods only toggle the hold indicator. If you hold
	// a session, you must make sure that any running RTP is stopped.
	// If you unhold a session you have to call start_rtp to start the
	// RTP.
	void hold(void);
	void unhold(void);
	
	// Check if RTP session is acitve
	bool is_rtp_active(void) const;
};

#endif
