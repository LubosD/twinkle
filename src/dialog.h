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

/**
 * @file
 * SIP dialog established by an INVITE transaction.
 */

#ifndef _DIALOG_H
#define _DIALOG_H

#include <string>
#include <list>
#include <set>
#include <queue>
#include "abstract_dialog.h"
#include "client_request.h"
#include "phone.h"
#include "transaction_layer.h"
#include "protocol.h"
#include "redirect.h"
#include "session.h"
#include "user.h"
#include "sockets/url.h"
#include "threads/mutex.h"
#include "parser/request.h"
#include "sdp/sdp.h"
#include "stun/stun.h"

using namespace std;

// Forward declarations
class t_phone;
class t_line;
class t_session;
class t_sub_refer;

/** Dialog state */
enum t_dialog_state {
	DS_NULL,		/**< Initial state */

	// UAC states
	DS_W4INVITE_RESP,	/**< INVITE sent, waiting for response */
	DS_W4INVITE_RESP2,	/**< Provisional response received */
	DS_EARLY,		/**< Provisional response with to-tag received */
	DS_W4BYE_RESP,		/**< BYE sent, waiting for response */

	// UAS states
	DS_W4ACK,		/**< Waiting for ACK on 2XX INVITE */
	DS_W4ANSWER,		/**< INVITE received, waiting for user to answer */

	// UAS and UAC states
	DS_CONFIRMED,		/**< Success received/sent */
	DS_W4ACK_RE_INVITE,	/**< Waiting for ACK on re-INVITE */
	DS_W4RE_INVITE_RESP,	/**< re-INVITE sent, waiting for response */
	DS_W4RE_INVITE_RESP2,	/**< re-INVITE sent, provisional response recvd */
	DS_TERMINATED,		/**< Dialog terminated */

	// Subscription states
	DS_CONFIRMED_SUB,	/**< Confirmed refer-subscription dialog */
};

/** Purpose for sending a re-INVITE request */
enum t_reinvite_purpose {
	REINVITE_HOLD,		/**< Re-invite for call hold */
	REINVITE_RETRIEVE,	/**< Re-invite for call retrieve */
};

/**
 * SIP dialog established by an INVITE transaction.
 *
 * Dialog state diagrams:
 * @dot
 * digraph call {
 *   label="Call setup and tear down state transitions"
 *   node [shape=ellipse, fontname=Helvetica, fontsize=10, style=filled, fillcolor=yellow];
 *   edge [fontname=Helvetica, fontsize=9];
 *
 *   null [label="DS_NULL" URL="\ref DS_NULL"];
 *   w4invite_resp [label="DS_W4INVITE_RESP" URL="\ref DS_W4INVITE_RESP"]
 *   w4invite_resp2 [label="DS_W4INVITE_RESP2" URL="\ref DS_W4INVITE_RESP2"]
 *   early [label="DS_EARLY" URL="\ref DS_EARLY"]
 *   w4bye_resp [label="DS_W4BYE_RESP" URL="\ref DS_W4BYE_RESP"]
 *   w4ack [label="DS_W4ACK" URL="\ref DS_W4ACK"]
 *   w4answer [label="DS_W4ANSWER" URL="\ref DS_W4ANSWER"]
 *   confirmed [label="DS_CONFIRMED" URL="\ref DS_CONFIRMED"]
 *   terminated [label="DS_TERMINATED" URL="\ref DS_TERMINATED"]
 *
 *   null -> w4invite_resp [label="send INVITE"]
 *   null -> w4answer [label="receive INVITE"]
 *   null -> terminated [label="receive INVITE\nSTUN media\nbind fails"]
 *   null -> terminated [label="receive INVITE\nunsupported\nmedia or body"]
 *   w4invite_resp -> w4invite_resp2 [label="receive 1XX without to-tag"]
 *   w4invite_resp -> early [label="receive 1XX with to-tag"]
 *   w4invite_resp -> confirmed [label="receive 2XX"]
 *   w4invite_resp -> terminated [label="receive failure\nresponse"]
 *   w4invite_resp2 -> confirmed [label="receive 2XX"]
 *   w4invite_resp2 -> early [label="receive 1XX with to-tag"]
 *   w4invite_resp2 -> terminated [label="receive failure\nresponse"]
 *   early -> confirmed [label="receive 2XX"]
 *   early -> terminated [label="receive failure\nresponse"]
 *   w4answer -> w4ack [label="user answered, send 2XX"]
 *   w4answer -> terminated [label="user rejected\nsend 4XX/6XX"]
 *   w4answer -> terminated [label="receive CANCEL/BYE\nsend 487"]
 *   w4ack -> confirmed [label="receive ACK"]
 *   confirmed -> w4bye_resp [label="send BYE"]
 *   confirmed -> terminated [label="receive BYE"]
 *   w4bye_resp -> terminated [label="receive BYE response"]
 * }
 * @enddot
 *
 * @dot
 * digraph reinvite {
 *   label="re-INVITE state transitions"
 *   node [shape=ellipse, fontname=Helvetica, fontsize=10, style=filled, fillcolor=yellow];
 *   edge [fontname=Helvetica, fontsize=9];
 *
 *   confirmed [label="DS_CONFIRMED" URL="\ref DS_CONFIRMED"]
 *   w4ack_re_invite [label="DS_W4ACK_RE_INVITE" URL="\ref DS_W4ACK_RE_INVITE"]
 *   w4re_invite_resp [label="DS_W4RE_INVITE_RESP" URL="\ref DS_W4RE_INVITE_RESP"]
 *   w4re_invite_resp2 [label="DS_W4RE_INVITE_RESP2" URL="\ref DS_W4RE_INVITE_RESP2"]
 *   terminated [label="DS_TERMINATED" URL="\ref DS_TERMINATED"]
 *
 *   confirmed -> w4ack_re_invite [label="receive re-INVITE, send 2XX"]
 *   confirmed -> terminated [label="receive re-INVITE\nSTUN fails"]
 *   confirmed -> confirmed [label="receive re-INVITE, send failure response"]
 *   confirmed -> w4re_invite_resp [label="send re-INVITE"]
 *   w4ack_re_invite -> confirmed [label="reveive ACK"]
 *   w4re_invite_resp -> w4re_invite_resp2 [label="receive 1XX"]
 *   w4re_invite_resp -> confirmed [label="receive final response"]
 *   w4re_invite_resp2 -> confirmed [label="receive final response"]
 *   w4re_invite_resp -> terminated [label="receive BYE"]
 *   w4re_invite_resp2 -> terminated [label="receive BYE"]
 * }
 * @enddot
 *
 * @dot
 * digraph refer {
 *   label="State transitions when REFER subscription is active"
 *   node [shape=ellipse, fontname=Helvetica, fontsize=10, style=filled, fillcolor=yellow];
 *   edge [fontname=Helvetica, fontsize=9];
 *
 *   confirmed [label="DS_CONFIRMED" URL="\ref DS_CONFIRMED"]
 *   w4bye_resp [label="DS_W4BYE_RESP" URL="\ref DS_W4BYE_RESP"]
 *   w4re_invite_resp [label="DS_W4RE_INVITE_RESP" URL="\ref DS_W4RE_INVITE_RESP"]
 *   w4re_invite_resp2 [label="DS_W4RE_INVITE_RESP2" URL="\ref DS_W4RE_INVITE_RESP2"]
 *   terminated [label="DS_TERMINATED" URL="\ref DS_TERMINATED"]
 *   confirmed_sub [label="DS_CONFIRMED_SUB" URL="DS_CONFIRMED_SUB"]
 *
 *   confirmed -> confirmed_sub [label="receive BYE"]
 *   w4re_invite_resp -> confirmed_sub [label="receive BYE"]
 *   w4re_invite_resp2 -> confirmed_sub [label="receive BYE"]
 *   w4bye_resp -> confirmed_sub [label="receive BYE response"]
 *   confirmed_sub -> terminated [label="terminate subscription"]
 * }
 * @enddot
 *
 * @dot
 * digraph timeout {
 *   label="State transitions due to timeouts"
 *   node [shape=ellipse, fontname=Helvetica, fontsize=10, style=filled, fillcolor=yellow];
 *   edge [fontname=Helvetica, fontsize=9];
 *
 *   w4answer [label="DS_W4ANSWER" URL="\ref DS_W4ANSWER"]
 *   w4ack [label="DS_W4ACK" URL="\ref DS_W4ACK"]
 *   confirmed [label="DS_CONFIRMED" URL="\ref DS_CONFIRMED"]
 *   terminated [label="DS_TERMINATED" URL="\ref DS_TERMINATED"]
 *   confirmed_sub [label="DS_CONFIRMED_SUB" URL="DS_CONFIRMED_SUB"]
 *
 *   w4answer -> w4answer [label="LTMR_100REL_TIMEOUT\nretransmit 1XX" URL="LTMR_100REL_TIMEOUT"]
 *   w4answer -> terminated [label="LTMR_100REL_GUARD\nsend 500" URL="LTMR_100REL_GUARD"]
 *   w4ack -> w4ack [label="LTMR_ACK_TIMEOUT\nretransmit 2XX" URL="LTMR_ACK_TIMEOUT"]
 *   w4ack -> confirmed [label="LTMR_ACK_GUARD\ntear down call" URL="LTMR_ACK_GUARD"]
 *   confirmed_sub -> terminated [label="REFER subscription\ntimeout"]
 * }
 * @enddot
 */
class t_dialog : public t_abstract_dialog {
	friend class t_phone;
	
protected:
	t_line			*line; /**< Phone line owning this dialog. */
	t_dialog_state		state; /**< Dialog state. */

	/** Session established by this dialog. */
	t_session	*session;

	/**
	 * New session being established during re-invite.
	 * When the re-INVITE transaction finishes successfully, then
	 * this session information will override the general session.
	 */
	t_session	*session_re_invite;

	/** The purpose of an outgoing re-INVITE request */
	t_reinvite_purpose	reinvite_purpose;

	/** Indicates if the last call hold action failed. */
	bool			hold_failed;

	t_client_request	*req_out;	  /**< Pending outgoing non-INVITE request */
	t_client_request	*req_out_invite;  /**< Pending outgoing INVITE */
	t_client_request	*req_in_invite;   /**< Pending incoming INVITE */
	t_client_request	*req_cancel;      /**< Pending outgoing CANCEL */
	t_client_request	*req_refer;	  /**< Pending outgoing REFER */
	t_client_request	*req_info;	  /**< Pending outgoing INFO */

	/**
	 * Last outgoing PRACK. While a PRACK is still pending a new 1xx
	 * response might come in. A PRACK will be sent for this 1xx without
	 * waiting for the response for the previous PRACK.
	 */
	t_client_request	*req_prack;
	
	/** Pending STUN request */
	t_client_request	*req_stun;
	
	/**
	 * Incoming request queue. A request may come in when it cannot be
	 * served yet. Such a request is stored in the queue to be served
	 * later.
	 */
	list<t_client_request *>	inc_req_queue;
	
	/** Indication if request must be cancelled */
	bool request_cancelled;
	
	/**
	 * Indication that the dialog must be terminated after a 2XX
	 * on an INVITE is received (e.g. when 2XX glares with CANCEL).
	 */
	bool end_after_2xx_invite;

	/** Indication that the dialog must be terminated after ACK. */
	bool end_after_ack;

	/**
	 * Indication that the user wants to answer the call.
	 * Sending the answer must be delayed as we are still waiting for
	 * a PRACK to acknowledge a 1xx containing SDP from the
	 * far end (RFC 3262 3).
	 */
	bool answer_after_prack;
	
	/** Indication if 180 ringing has already been received */
	bool ringing_received;

	/** Cached success response to INVITE needed for retransmission */
	t_response		*resp_invite;

	/**
	 * Cached provisional response to INVITE needed for retransmission
	 * when provisional responses are sent reliable (100rel)
	 */
	t_response		*resp_1xx_invite;

	/** Cached ack needed for retransmission */
	t_request		*ack;

	/** Subscription created by REFER (RFC 3515) */
	t_sub_refer		*sub_refer;
	
	/** Queue of DTMF digits to be sent via INFO requests */
	queue<char>		dtmf_queue;

	/** @name Process incoming responses */
	//@{
	/**
	 * Process an incoming response in the @ref DS_W4INVITE_RESP state.
	 * @param r The response
	 * @param tuid Transaction user id
	 * @param tid Transaction id
	 */
	void state_w4invite_resp(t_response *r, t_tuid tuid, t_tid tid);
	
	/**
	 * Process an incoming response in the @ref DS_EARLY state.
	 * @param r The response
	 * @param tuid Transaction user id
	 * @param tid Transaction id
	 */
	void state_early(t_response *r, t_tuid tuid, t_tid tid);
	
	/**
	 * Process an incoming response in the @ref DS_W4BYE_RESP state.
	 * @param r The response
	 * @param tuid Transaction user id
	 * @param tid Transaction id
	 */
	void state_w4bye_resp(t_response *r, t_tuid tuid, t_tid tid);
	
	/**
	 * Process an incoming response in the @ref DS_CONFIRMED state.
	 * @param r The response
	 * @param tuid Transaction user id
	 * @param tid Transaction id
	 */
	void state_confirmed_resp(t_response *r, t_tuid tuid, t_tid tid);
	
	/**
	 * Process an incoming response in the @ref DS_W4RE_INVITE_RESP state.
	 * @param r The response
	 * @param tuid Transaction user id
	 * @param tid Transaction id
	 */
	void state_w4re_invite_resp(t_response *r, t_tuid tuid, t_tid tid);
	//@}

	/** @name Process incoming requests */
	//@{
	/**
	 * Process an incoming request in the @ref DS_NULL state.
	 * @param r The request
	 * @param tuid Transaction user id
	 * @param tid Transaction id
	 */
	void state_null(t_request *r, t_tuid tuid, t_tid tid);
	
	/**
	 * Process an incoming request in the @ref DS_W4ANSWER state.
	 * @param r The request
	 * @param tuid Transaction user id
	 * @param tid Transaction id
	 */
	void state_w4answer(t_request *r, t_tuid tuid, t_tid tid);
	
	/**
	 * Process an incoming request in the @ref DS_W4ACK state.
	 * @param r The request
	 * @param tuid Transaction user id
	 * @param tid Transaction id
	 */
	void state_w4ack(t_request *r, t_tuid tuid, t_tid tid);
	
	/**
	 * Process an incoming request in the @ref DS_W4ACK_RE_INVITE state.
	 * @param r The request
	 * @param tuid Transaction user id
	 * @param tid Transaction id
	 */
	void state_w4ack_re_invite(t_request *r, t_tuid tuid, t_tid tid);
	
	/**
	 * Process an incoming request in the @ref DS_W4RE_INVITE_RESP state.
	 * @param r The request
	 * @param tuid Transaction user id
	 * @param tid Transaction id
	 */
	void state_w4re_invite_resp(t_request *r, t_tuid tuid, t_tid tid);
	
	/**
	 * Process an incoming request in the @ref DS_W4BYE_RESP state.
	 * @param r The request
	 * @param tuid Transaction user id
	 * @param tid Transaction id
	 */
	void state_w4bye_resp(t_request *r, t_tuid tuid, t_tid tid);
	
	/**
	 * Process an incoming request in the @ref DS_CONFIRMED state.
	 * @param r The request
	 * @param tuid Transaction user id
	 * @param tid Transaction id
	 */
	void state_confirmed(t_request *r, t_tuid tuid, t_tid tid);
	
	/**
	 * Process an incoming request in the @ref DS_CONFIRMED_SUB state.
	 * @param r The request
	 * @param tuid Transaction user id
	 * @param tid Transaction id
	 */
	void state_confirmed_sub(t_request *r, t_tuid tuid, t_tid tid);
	//@}
	
	/** @name Process requests in the confirmed state */
	//@{
	/** Proces incoming re-INVITE. */
	void process_re_invite(t_request *r, t_tuid tuid, t_tid tid);
	
	/** Proces incoming REFER. */
	void process_refer(t_request *r, t_tuid tuid, t_tid tid);
	
	/** Process incoming SUBSCRIBE (refer subscription). */
	void process_subscribe(t_request *r, t_tuid tuid, t_tid tid);
	
	/** Process incoming NOTIFY (refer subscription). */
	void process_notify(t_request *r, t_tuid tuid, t_tid tid);
	
	/** Process incoming INFO. */
	void process_info(t_request *r, t_tuid tuid, t_tid tid);
	
	/** Process incoming MESSAGE. */
	void process_message(t_request *r, t_tuid tuid, t_tid tid);
	//@}

	/** @name Process timeouts */
	//@{
	/**
	 * Process timeout in @ref DS_W4INVITE_RESP state.
	 * @param timer The expired timer.
	 */
	void state_w4invite_resp(t_line_timer timer);
	
	/**
	 * Process timeout in @ref DS_EARLY state.
	 * @param timer The expired timer.
	 */
	void state_early(t_line_timer timer);
	
	/**
	 * Process timeout in @ref DS_W4ACK state.
	 * @param timer The expired timer.
	 */
	void state_w4ack(t_line_timer timer);
	
	/**
	 * Process timeout in @ref DS_W4ACK_RE_INVITE state.
	 * @param timer The expired timer.
	 */
	void state_w4ack_re_invite(t_line_timer timer);
	
	/**
	 * Process timeout in @ref DS_W4RE_INVITE_RESP state.
	 * @param timer The expired timer.
	 */
	void state_w4re_invite_resp(t_line_timer timer);
	
	/**
	 * Process timeout in @ref DS_W4ANSWER state.
	 * @param timer The expired timer.
	 */
	void state_w4answer(t_line_timer timer);
	
	/**
	 * Process timeout in @ref DS_CONFIRMED state.
	 * @param timer The expired timer.
	 */
	void state_confirmed(t_line_timer timer);
	//@}

	/** Make the re-INVITE session the current session. */
	void activate_new_session(void);

	/**
	 * Process SDP answer in 1xx and 2xx responses if present.
	 * Apply ringing tone for a 180 response.
	 * Determine if call should be canceled due to unsupported
	 * or missing SDP.
	 * @param r The 1XX/2XX response.
	 */
	void process_1xx_2xx_invite_resp(t_response *r);

	/**
	 * Acknowledge a reveived 2xx response on an INVITE.
	 * @param r The 2XX response.
	 */
	void ack_2xx_invite(t_response *r);

	/**
	 * Send PRACK if the response requires it.
	 * @param r The response.
	 */
	void send_prack_if_required(t_response *r);

	/**
	 * Determine if a reliable provisional repsonse must be discarded.
	 * A provisional response must be discarded because it is a retransmission 
	 * or received out of order.
	 * Initializes the remote response nr if the response is the
	 * first response.
	 * @param r The provisional response.
	 * @return true, discard response.
	 * @return false, otherwise
	 */
	bool must_discard_100rel(t_response *r);

	/**
	 * Respond to an incoming PRACK.
	 * @param r The incoming PRACK.
	 * @param tuid Transaction user id
	 * @param tid Transaction id
	 * @return true, if a success response was given.
	 * @return false if an error response was given.
	 */
	bool respond_prack(t_request *r, t_tuid tuid, t_tid tid);

	virtual void send_request(t_request *r, t_tuid tuid);
public:
	/** @name Timer durations and timer id's */
	//@{
	unsigned long		dur_ack_timeout;	/**< @ref LTMR_ACK_TIMEOUT duration (ms) */
	t_object_id		id_ack_timeout;		/**< @ref LTMR_ACK_TIMEOUT timer id */
	t_object_id		id_ack_guard;		/**< @ref LTMR_ACK_GUARD timer id */
	t_object_id		id_re_invite_guard;	/**< @ref LTMR_RE_INVITE_GUARD timer id */
	t_object_id		id_glare_retry;		/**< @ref LTMR_GLARE_RETRY timer id */
	t_object_id		id_cancel_guard;	/**< @ref LTMR_CANCEL_GUARD timer id */
	//@}

	/** @name RFC 3262 100rel timers */
	//@{
	unsigned long		dur_100rel_timeout;	/**< @ref LTMR_100REL_TIMEOUT duration (ms) */
	t_object_id		id_100rel_timeout;	/**< @ref LTMR_100REL_TIMEOUT timer id */
	t_object_id		id_100rel_guard;	/**< @ref LTMR_100REL_GUARD timer id */
	//@}

	/** Indicates if last incoming REFER was accepted. */
	bool			refer_accepted;
	
	/** Indicates if the call transfer triggered by the last outgoing REFER succeeded. */
	bool			refer_succeeded;
	
	/** Indicates if the last outgoing REFER request failed. */
	bool			out_refer_req_failed;

	/**
	 * Indicates if this dialog is setup because the user told to do
	 * so by a REFER.
	 */
	bool			is_referred_call;

	/** State of an outgoing REFER. */
	t_refer_state		refer_state;

	/**
	 * Constructor.
	 * @param _line The line owning this dialog.
	 */
	t_dialog(t_line *_line);
	
	/** Destructor. */
	virtual ~t_dialog();

	virtual t_request *create_request(t_method m);

	virtual t_dialog *copy(void);

	/**
	 * Send INIVTE request.
	 * @param to_uri The URI to be used a request-URI and To header URI
	 * @param to_display Display name for To header.
	 * @param subject If not empty, this string will go into the Subject header.
	 * @param hdr_referred_by The Reffered-By header to be put in the INVITE.
	 * @param hdr_replaces The Replaces header to be put in the INVITE.
	 * @param hdr_require Required extensions to be put in the Require header.
	 * @param hdr_request_disposition Request-Disposition header to be put in the INVITE.
	 * @param anonymous Inidicates if the INVITE should be sent anonymous.
	 *
	 * @pre Dialog is in @ref DS_NULL state.
	 */
	void send_invite(const t_url &to_uri, const string &to_display,
		const string &subject, const t_hdr_referred_by &hdr_referred_by,
		const t_hdr_replaces &hdr_replaces, 
		const t_hdr_require &hdr_require, 
		const t_hdr_request_disposition &hdr_request_disposition,
		bool anonymous);

	/**
	 * Resend the INVITE with an authorization header containing credentials
	 * for the challenge in the response.
	 * @param resp, the response on the INVITE.
	 * @return true, if resending succeeded. 
	 * @return false, if credentials could not be determined.
	 *
	 * @pre The response must be a 401 or 407.
	 */
	bool resend_invite_auth(t_response *resp);

	/**
	 * Resend the INVITE because the far-end did not support all required
	 * extensions. Extensions that could not be supported will not be
	 * required this time.
	 * @param resp The response in the INVITE.
	 * @return true, if resending succeeded.
	 * @return false, if far-end did not indicate which extensions are
	 * unsupported. Or if a required extension could not be disabled.
	 *
	 * @pre The response must be a 420.
	 */
	bool resend_invite_unsupported(t_response *resp);

	/**
	 * Redirect INVITE to the next destination.
	 * @param resp The response on the INVITE.
	 * @return true, if INIVTE was redirected succesfully.
	 * @return false, if there is no next destination.
	 *
	 * @pre The response must be a 3XX.
	 */
	bool redirect_invite(t_response *resp);
	
	/**
	 * Failover INVITE to the next destination from DNS lookup.
	 * @return true, if INIVTE was succesfully sent.
	 * @return false, if there is no next destination.
	 */
	bool failover_invite(void);

	/** Send BYE request. */
	void send_bye(void);
	
	/** Send OPTIONS request. */
	void send_options(void);

	/**
	 * Send CANCEL request.
	 * If an early dialog exists, then the CANCEL can be sent
	 * right away as a response has been received for the INVITE.
	 * Otherwise, the CANCEL will be sent as soon as an early dialog
	 * is created.
	 * @param early_dialog_exists Indicates if an early dialog exists.
	 */
	void send_cancel(bool early_dialog_exists);
	
	/**
	 * Indicate that the dialog must be ended if a 2XX is received
	 * on an INVITE.
	 * @param on Set/clear indication.
	 */
	void set_end_after_2xx_invite(bool on);

	/**
	 * Send re-INVITE.
	 * @pre session_re_invite attribute contains the session
	 * information for the re-INVITE.
	 */
	void send_re_invite(void);

	virtual bool resend_request_auth(t_response *resp);

	virtual bool redirect_request(t_response *resp);
	
	virtual bool failover_request(t_response *resp);

	/**
	 * Hold call.
	 * If rtp_only is false, then a re-INVITE will be sent.
	 * @param rtponly Indicates if only the RTP streams should be stopped and
	 * the soundcard freed without any SIP signaling.
	 */
	void hold(bool rtponly = false);
	
	/** Retrieve call (send re-INVITE if needed). */
	void retrieve(void);
	
	/** Kill all RTP stream associated with this dialog. */
	void kill_rtp(void);

	/**
	 * Refer a call (send REFER).
	 * @param uri URI of the refer target.
	 * @param display Display name of the refer target.
	 */
	void send_refer(const t_url &uri, const string &display);

	/**
	 * Send DTMF digit.
	 * @param digit The digit.
	 * @param inband Indicates if digit must be sent inband.
	 * @param info Indicates if digit must be sent in a SIP INFO.
	 *
	 * @pre Either inband or info or none of the indicators is true.
	 * @post If none of the indicators is true, then RFC 2833 is used.
	 */
	void send_dtmf(char digit, bool inband, bool info);
	
	/**
	 * Create a binding for the media port via STUN.
	 * @return true, if binding is created.
	 * @return false, if binding cannot be created.
	 */
	bool stun_bind_media(void);

	/** @name Handle received events */
	//@{
	void recvd_response(t_response *r, t_tuid tuid, t_tid tid);
	
	void recvd_request(t_request *r, t_tuid tuid, t_tid tid);
	
	/**
	 * Handle incoming CANCEL request.
	 * @param r The CANCEL request.
	 * @param cancel_tid Transaction id of the CANCEL transaction.
	 * @param target_tid Transaction id of the transaction to be cancellerd.
	 */
	void recvd_cancel(t_request *r, t_tid cancel_tid, t_tid target_tid);
	
	/**
	 * Handle incoming STUN response.
	 * @param r The STUN response.
	 * @param tuid Transaction user id
	 * @param tid Transaction id
	 */
	void recvd_stun_resp(StunMessage *r, t_tuid tuid, t_tid tid);
	
	/**
	 * Handle the response from the user on the question for refer
	 * permission. This response is received on the dialog that received
	 * the REFER before.
	 * @param permission Permission response from the user.
	 * @param r The REFER request that was received.
	 */
	void recvd_refer_permission(bool permission, t_request *r);
	//@}

	/** Answer a call (send 200 OK). */
	void answer(void);

	/**
	 * Reject a call.
	 * @param code The response code to reject the call with.
	 * @param reason A specific reason may be given to the error code. If no
	 * reason is specified the default reason is used.
	 *
	 * @pre code >= 400
	 */
	void reject(int code, string reason = "");

	/**
	 * Redirect a call.
	 * @param code The response code to redirect the call with.
	 * @param A specific reason may be give to the error code. If no
	 * reason is specified the default reason is used.
	 * @param destinations The list of redirect destinations in order of
	 * preference.
	 *
	 * @pre code is 3XX.
	 */
	void redirect(const list<t_display_url> &destinations, int code, string reason = "");

	virtual bool match_response(t_response *r, t_tuid tuid);
	
	/**
	 * Match STUN response with dialog.
	 * @param r STUN response.
	 * @param tuid Transaction user id
	 * @return true, if response matches.
	 * @return false, otherwise.
	 */
	bool match_response(StunMessage *r, t_tuid tuid);

	/**
	 * Match CANCEL request with dialog.
	 * @param r CANCEL request.
	 * @param target_tid Transaction id of transaction to be cancelled.
	 * @return true, if request matches.
	 * @return false, otherwise.
	 */
	bool match_cancel(t_request *r, t_tid target_tid);

	/**
	 * Check if an incoming INVITE is a retransmission.
	 * @param r The INVITE request.
	 * @return true, if INVITE is a retransmission.
	 * @return false, otherwise.
	 */
	bool is_invite_retrans(t_request *r);

	/** Process a retransmission of an incoming INVITE. */
	void process_invite_retrans(void);

	/**
	 * Get the state of the dialog.
	 * @return The dialog state.
	 */
	t_dialog_state get_state(void) const;

	/**
	 * Process dialog timer timeout.
	 * @param timer The timer that expired.
	 */
	void timeout(t_line_timer timer);

	/**
	 * Process subcribe timer timeout (REFER subscription).
	 * @param timer The timer that expired.
	 * @param event_type Event type of the subscription.
	 * @param event_id Event id of the subscription.
	 */
	void timeout_sub(t_subscribe_timer timer, const string &event_type,
		const string &event_id);

	/**
	 * Get the phone that belongs to this dialog.
	 * @return The phone object.
	 */
	t_phone *get_phone(void) const;

	/**
	 * Get the line that belongs to this dialog.
	 * @return The line object.
	 */
	t_line *get_line(void) const;

	/**
	 * Get the session belonging to this dialog.
	 * @return The session belonging to this dialog.
	 * @return NULL if there is no session.
	 */
	t_session * get_session(void) const;
	
	/**
	 * Get the audio session belonging to this dialog.
	 * @return The audio session belonging to this dialog.
	 * @return NULL if there is no audio session.
	 */	
	t_audio_session *get_audio_session(void) const;
	
	/**
	 * Check if the dialog has an acitve session.
	 * @return true, if the dialog has an active session.
	 * @return false, otherwise.
	 */
	bool has_active_session(void) const;

	/**
	 * Notify the dialog of the progress of a reference.
	 * @param r The response sent by the refer target.
	 */
	void notify_refer_progress(t_response *r);
	
	/**
	 * Check if a dialog will be released.
	 * @return true, if the dialog will be released.
	 * @return false, otherwise.
	 */
	bool will_release(void) const;
};

#endif
