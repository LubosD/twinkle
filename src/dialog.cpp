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

#include <cstdlib>
#include <assert.h>
#include <iostream>
#include "call_history.h"
#include "call_script.h"
#include "dialog.h"
#include "exceptions.h"
#include "line.h"
#include "log.h"
#include "phone_user.h"
#include "sub_refer.h"
#include "util.h"
#include "userintf.h"
#include "audio/rtp_telephone_event.h"
#include "audits/memman.h"
#include "im/im_iscomposing_body.h"
#include "sdp/sdp.h"
#include "sockets/socket.h"
#include "stun/stun_transaction.h"

extern t_event_queue	*evq_sender;
extern t_event_queue	*evq_trans_mgr;
extern string		user_host;
extern string		local_hostname;
extern t_phone		*phone;

// Protected

// Create a request within a dialog
// RFC 3261 12.2.1.1
t_request *t_dialog::create_request(t_method m) {
	assert(state != DS_NULL);
	t_user *user_config = phone_user->get_user_profile();

	// RFC 3261 9.1
	if (m == CANCEL) {
		t_request *r = new t_request(m);
		MEMMAN_NEW(r);
		
		assert(req_out_invite);
		t_request *orig_req = req_out_invite->get_request();
		r->hdr_to = orig_req->hdr_to;
		r->hdr_from = orig_req->hdr_from;
		r->hdr_call_id = orig_req->hdr_call_id;
		r->hdr_cseq.set_seqnr(orig_req->hdr_cseq.seqnr);
		r->hdr_cseq.set_method(CANCEL);
		r->hdr_via = orig_req->hdr_via; // RFC 3261 8.1.1.7
		r->hdr_max_forwards.set_max_forwards(MAX_FORWARDS);
		r->hdr_route = orig_req->hdr_route;
		SET_HDR_USER_AGENT(r->hdr_user_agent);
		r->uri = orig_req->uri;
		
		// RFC 3263 4
		// CANCEL for a particular SIP request MUST be sent to the same SIP
		// server that the SIP request was delivered to.
		t_ip_port ip_port;
		orig_req->get_destination(ip_port, *user_config);
		r->set_destination(ip_port);
		return r;
	}
	
	t_request *r = t_abstract_dialog::create_request(m);

	// CSeq header
	if (m == ACK) {
		assert(req_out_invite);
		
		// Local sequence number was incremented by t_abstract_dialog.
		// Decrement as it ACK does not take a new sequence number.
		local_seqnr--;

		// ACK has the same sequence number
		// as the INVITE.
		r->hdr_cseq.set_seqnr(req_out_invite->get_request()->hdr_cseq.seqnr);

		// RFC 3261 22.1
		// Authorization and Proxy-Authorization headers in INVITE
		// must be repeated in ACK
		r->hdr_authorization = req_out_invite->get_request()->
					hdr_authorization;
		r->hdr_proxy_authorization = req_out_invite->get_request()->
					hdr_proxy_authorization;
	}

	// Contact header
	t_contact_param contact;
	switch (m) {
	case REFER:
	case SUBSCRIBE:
	case NOTIFY:
		// RFC 3265 7.1, RFC 3515 2.2
		// Contact header is mandatory
		contact.uri.set_url(line->create_user_contact(h_ip2str(r->get_local_ip())));
		r->hdr_contact.add_contact(contact);
		break;
	default:
		break;
	}
	
	// Privacy header
	if (line->get_hide_user()) {
		r->hdr_privacy.add_privacy(PRIVACY_ID);
	}

	return r;
}

// NULL state. Waiting for incoming INVITE
void t_dialog::state_null(t_request *r, t_tuid tuid, t_tid tid) {
	t_response *resp;
	t_user *user_config = phone_user->get_user_profile();

	if (r->method != INVITE) {
		state = DS_TERMINATED;
		return;
	}
	
	// Set local tag
	if (r->hdr_to.tag.size() == 0) {
		local_tag = NEW_TAG;
	} else {
		local_tag = r->hdr_to.tag;
	}
	
	// If STUN is enabled, then first send a STUN binding request to
	// discover the IP adderss and port for media.
	if (phone->use_stun(user_config)) {
		// The STUN transaction may take a while.
		// Send 100 Trying
		resp = r->create_response(R_100_TRYING);
		resp->hdr_to.set_tag("");
		line->send_response(resp, tuid, tid);
		MEMMAN_DELETE(resp);
		delete resp;
		
		if (!stun_bind_media()) {
				// STUN request failed. Send a 500 on the INVITE.
				resp = r->create_response(R_500_INTERNAL_SERVER_ERROR);
				resp->hdr_to.set_tag(local_tag);
				line->send_response(resp, tuid, tid);
				MEMMAN_DELETE(resp);
				delete resp;
				
				state = DS_TERMINATED;
				return;
		}
	}
	
	call_id = r->hdr_call_id.call_id;

	// Initialize local seqnr
	local_seqnr = NEW_SEQNR;
	local_resp_nr = NEW_SEQNR;

	remote_tag = r->hdr_from.tag;
	local_uri = r->hdr_to.uri;
	local_display = r->hdr_to.display;
	remote_uri = r->hdr_from.uri;
	remote_display = r->hdr_from.display;

	// Set remote target URI and display name
	remote_target_uri = r->hdr_contact.contact_list.front().uri;
	remote_target_display = r->
			hdr_contact.contact_list.front().display;

	// Set route set
	if (r->hdr_record_route.is_populated()) {
		route_set = r->hdr_record_route.route_list;
	}
	
	// RFC 3261 13.2.1
	// An initial INVITE should list all supported extensions.
	// Set supported extensions
	if (r->hdr_supported.is_populated()) {
		remote_extensions.insert(r->hdr_supported.features.begin(),
				r->hdr_supported.features.end());
	}

	// Media information
	int warn_code;
	string warn_text;
	if (r->body) {
		switch(r->body->get_type()) {
		case BODY_SDP:
			if (session->process_sdp_offer((t_sdp*)r->body,
					warn_code, warn_text)) {
				session->recvd_offer = true;
				break;
			}

			// Unsupported media
			resp = r->create_response(
					R_488_NOT_ACCEPTABLE_HERE);
			resp->hdr_to.set_tag(local_tag);
			resp->hdr_warning.add_warning(t_warning(LOCAL_HOSTNAME,
					0, warn_code, warn_text));
			line->send_response(resp, tuid, tid);
			
			// Create call history record
			line->call_hist_record.start_call(r, t_call_record::DIR_IN,
				user_config->get_profile_name());
			line->call_hist_record.fail_call(resp);
			
			MEMMAN_DELETE(resp);
			delete resp;
			state = DS_TERMINATED;
			return;
		default:
			// Unsupported body type. Reject call.
			resp = r->create_response(
					R_415_UNSUPPORTED_MEDIA_TYPE);
			resp->hdr_to.set_tag(local_tag);

			// RFC 3261 21.4.13
			SET_HDR_ACCEPT(resp->hdr_accept);
			
			// Create call history record
			line->call_hist_record.start_call(r, t_call_record::DIR_IN,
				user_config->get_profile_name());
			line->call_hist_record.fail_call(resp);

			line->send_response(resp, tuid, tid);
			MEMMAN_DELETE(resp);
			delete resp;
			state = DS_TERMINATED;
			return;
		}
	}
	
	resp = r->create_response(R_180_RINGING);
	resp->hdr_to.set_tag(local_tag);

	// RFC 3261 13.3.1.1
	// A provisional response creates an early dialog, so
	// copy the Record-Route header and add a Contact
	// header.

	// Copy the Record-Route header from request to response
	if (r->hdr_record_route.is_populated()) {
		resp->hdr_record_route = r->hdr_record_route;
	}

	// Set Contact header
	t_contact_param contact;
	contact.uri.set_url(line->create_user_contact(h_ip2str(resp->get_local_ip())));
	resp->hdr_contact.add_contact(contact);

	// RFC 3262 3
	// Send 180 response reliable if needed
	if (r->hdr_require.contains(EXT_100REL) ||
	    (r->hdr_supported.contains(EXT_100REL) &&
	     (user_config->get_ext_100rel() == EXT_PREFERRED ||
	      user_config->get_ext_100rel() == EXT_REQUIRED)))
	{
		resp->hdr_require.add_feature(EXT_100REL);
		resp->hdr_rseq.set_resp_nr(++local_resp_nr);

		// RFC 3262 5
		// Create SDP offer in first reliable response if no offer
		// was received in INVITE.
		// This implentation does not create an answer in an
		// reliable 1xx response if an offer was received.
		if (!session->recvd_offer) {
			session->create_sdp_offer(resp, SDP_O_USER);
		}

		// Keep a copy of the response for retransmission
		resp_1xx_invite = (t_response *)resp->copy();

		// Start 100rel timeout and guard timers
		line->start_timer(LTMR_100REL_GUARD, get_object_id());
		line->start_timer(LTMR_100REL_TIMEOUT, get_object_id());
	}

	line->send_response(resp, req_in_invite->get_tuid(),
			req_in_invite->get_tid());
	MEMMAN_DELETE(resp);
	delete resp;
	
	ui->cb_incoming_call(user_config, line->get_line_number(), r);
	line->call_hist_record.start_call(r, t_call_record::DIR_IN,
		user_config->get_profile_name());
	
	state = DS_W4ANSWER;
}

// A provisional answer has been sent. Waiting for user to answer.
void t_dialog::state_w4answer(t_request *r, t_tuid tuid, t_tid tid) {
	t_response *resp;
	t_user *user_config = phone_user->get_user_profile();
	bool tear_down = false;
	bool answer_call = false;
	
	t_call_script script_in_call_failed(user_config, t_call_script::TRIGGER_IN_CALL_FAILED,
			line->get_line_number() + 1);

	switch (r->method) {
	case CANCEL:
		// Cancel the request and terminate the dialog.
		// A response on the CANCEL is already given by dialog::recvd_cancel
		resp = req_in_invite->get_request()->
				create_response(R_487_REQUEST_TERMINATED);
		resp->hdr_to.set_tag(local_tag);
		line->send_response(resp, req_in_invite->get_tuid(),
				req_in_invite->get_tid());
		line->call_hist_record.fail_call(resp);
		
		// Trigger call script
		script_in_call_failed.exec_notify(resp);
		
		MEMMAN_DELETE(resp);
		delete resp;

		ui->cb_call_cancelled(line->get_line_number());
		state = DS_TERMINATED;
		break;
	case BYE:
		// Send 200 on the BYE request
		resp = r->create_response(R_200_OK);
		line->send_response(resp, tuid, tid);
		MEMMAN_DELETE(resp);
		delete resp;

		// Send a 487 response to terminate the pending request
		resp = req_in_invite->get_request()->create_response(
						R_487_REQUEST_TERMINATED);
		resp->hdr_to.set_tag(local_tag);
		line->send_response(resp, req_in_invite->get_tuid(),
						req_in_invite->get_tid());
		line->call_hist_record.fail_call(resp);
		
		// Trigger call script
		script_in_call_failed.exec_notify(resp);
		
		MEMMAN_DELETE(resp);
		delete resp;

		ui->cb_far_end_hung_up(line->get_line_number());
		state = DS_TERMINATED;
		break;
	case PRACK:
		// RFC 3262 3
		if (respond_prack(r, tuid, tid)) {
			answer_call = answer_after_prack;

			// RFC 3262 5
			// If an offer was sent in the 1xx response, then PRACK must
			// contain an answer
			if (session->sent_offer && r->body) {
				int warn_code;
				string warn_text;
				if (r->body->get_type() != BODY_SDP) {
					// Only SDP bodies are supported
					ui->cb_unsupported_content_type(
						line->get_line_number(), r);
					tear_down = true;
				} else if (session->process_sdp_answer((t_sdp *)r->body,
						warn_code, warn_text))
				{
					session->recvd_answer = true;
					session->start_rtp();
				} else {
					// SDP answer is not supported.
					// Tear down the call.
					ui->cb_sdp_answer_not_supported(
						line->get_line_number(), warn_text);
					tear_down = true;
				}
			}

			if (session->sent_offer && !r->body) {
				ui->cb_sdp_answer_missing(line->get_line_number());
				tear_down = true;
			}
		}

		if (tear_down) {
			resp = req_in_invite->get_request()->create_response(
				R_400_BAD_REQUEST,
				"SDP answer in PRACK missing or unsupported");
			resp->hdr_to.set_tag(local_tag);		
			line->send_response(resp, req_in_invite->get_tuid(),
						req_in_invite->get_tid());
			line->call_hist_record.fail_call(resp);
			
			// Trigger call script
			t_call_script script(user_config, t_call_script::TRIGGER_IN_CALL_FAILED,
					line->get_line_number() + 1);
			script.exec_notify(resp);
			
			MEMMAN_DELETE(resp);
			delete resp;
			state = DS_TERMINATED;
		} else if (answer_call) {
			answer();
		}

		break;
	default:
		// INVITE transaction has not been completed. Deny
		// other requests within the dialog.
		resp = r->create_response(R_500_INTERNAL_SERVER_ERROR,
			"Session not yet established");
		line->send_response(resp, tuid, tid);
		MEMMAN_DELETE(resp);
		delete resp;
		break;
	}
}

void t_dialog::state_w4answer(t_line_timer timer) {
	t_ip_port ip_port;
	t_response *resp;
	t_user *user_config = phone_user->get_user_profile();
	
	t_call_script script_in_call_failed(user_config, t_call_script::TRIGGER_IN_CALL_FAILED,
			line->get_line_number() + 1);

	// RFC 3262 3
	switch(timer) {
	case LTMR_100REL_TIMEOUT:
		// Retransmit 1xx response.
		// Send the response directly to the sender thread
		// bypassing the transaction layer. As this is a retransmission
		// from the TU, the transaction layer does not need to know.
		resp_1xx_invite->get_destination(ip_port);
		if (ip_port.ipaddr == 0) {
			// This should not happen. The response has been
			// sent before so it should be possible to sent
			// it again. Ignore the timeout. When the 100rel
			// guard timer expires, the dialog will be
			// cleaned up.
			break;
		}
		evq_sender->push_network(resp_1xx_invite, ip_port);
		line->start_timer(LTMR_100REL_TIMEOUT, get_object_id());
		break;
	case LTMR_100REL_GUARD:
		line->stop_timer(LTMR_100REL_TIMEOUT, get_object_id());

		// PRACK was not received in time. Tear down the call.
		resp = req_in_invite->get_request()->create_response(
			R_500_INTERNAL_SERVER_ERROR, "100rel timeout");
		resp->hdr_to.set_tag(local_tag);
		line->send_response(resp, req_in_invite->get_tuid(),
						req_in_invite->get_tid());
		line->call_hist_record.fail_call(resp);
		
		// Trigger call script
		script_in_call_failed.exec_notify(resp);
		
		MEMMAN_DELETE(resp);
		delete resp;

		remove_client_request(&req_in_invite);
		MEMMAN_DELETE(resp_1xx_invite);
		delete resp_1xx_invite;
		resp_1xx_invite = NULL;

		state = DS_TERMINATED;
		log_file->write_report("LTMR_100REL_GUARD expired.",
				"t_dialog::state_w4answer");

		ui->cb_100rel_timeout(line->get_line_number());
		break;
	default:
		// Other timeouts are not expected. Ignore.
		break;
	}
}

// 200 OK has been sent. Waiting for ACK
void t_dialog::state_w4ack(t_request *r, t_tuid tuid, t_tid tid) {
	t_response *resp;
	t_user *user_config = phone_user->get_user_profile();
	bool tear_down = false;
	t_client_request *cr;
	
	t_call_script script_out_call_failed(user_config, t_call_script::TRIGGER_OUT_CALL_FAILED,
			line->get_line_number() + 1);

	switch(r->method) {
	case ACK:
		// Dialog is established now.
		line->stop_timer(LTMR_ACK_TIMEOUT, get_object_id());
		line->stop_timer(LTMR_ACK_GUARD, get_object_id());
		remove_client_request(&req_in_invite);
		MEMMAN_DELETE(resp_invite);
		delete resp_invite;
		resp_invite = NULL;

		// If no offer was received in INVITE, then an offer
		// has been sent in 200 OK or reliable 1xx (RFC 3262).
		// Therefor an answer must be present in ACK
		if (!session->recvd_offer && r->body) {
			int warn_code;
			string warn_text;
			if (r->body->get_type() != BODY_SDP) {
				// Only SDP bodies are supported
				ui->cb_unsupported_content_type(
					line->get_line_number(), r);
				tear_down = true;
			} else if (session->process_sdp_answer((t_sdp *)r->body,
					warn_code, warn_text))
			{
				session->recvd_answer = true;
				session->start_rtp();
			} else {
				// SDP answer is not supported.
				// Tear down the call.
				ui->cb_sdp_answer_not_supported(
						line->get_line_number(), warn_text);
				tear_down = true;
			}
		}

		if (!session->recvd_offer && !r->body) {
			ui->cb_sdp_answer_missing(line->get_line_number());
			tear_down = true;
		}

		if (end_after_ack) {
			ui->cb_far_end_hung_up(line->get_line_number());
			state = DS_TERMINATED;
		} else {
			state = DS_CONFIRMED;
			if (tear_down) {
				send_bye();
			}
		}

		ui->cb_call_established(line->get_line_number());
		break;
	case BYE:
		// Send 200 on the BYE request
		resp = r->create_response(R_200_OK);
		line->send_response(resp, tuid, tid);
		
		// Trigger call script
		script_out_call_failed.exec_notify(resp);
		
		MEMMAN_DELETE(resp);
		delete resp;
		
		line->call_hist_record.end_call(true);

		// The session will be ended when an ACK has been
		// received.
		end_after_ack = true;
		break;
	case PRACK:
		// RFC 3262 3
		// This is a late PRACK as the call is answered already.
		// Respond in a normal way to the PRACK
		respond_prack(r, tuid, tid);
		break;
	default:
		// Queue the request as ACK needs to be received first.
		// Note that the tuid value is not stored in the queue.
		// For an incoming request tuid is always 0.
		cr = new t_client_request(user_config, r, tid);
		MEMMAN_NEW(cr);
		inc_req_queue.push_back(cr);
		log_file->write_header("t_dialog::state_w4ack", 
				LOG_NORMAL, LOG_INFO);
		log_file->write_raw("Waiting for ACK.\n");
		log_file->write_raw("Queue incoming ");
		log_file->write_raw(method2str(r->method, r->unknown_method));
		log_file->write_endl();
		log_file->write_footer();
		break;
	}
}

void t_dialog::state_w4ack_re_invite(t_request *r, t_tuid tuid, t_tid tid) {
	t_response *resp;
	t_user *user_config = phone_user->get_user_profile();
	bool tear_down = false;
	
	t_call_script script_out_call_failed(user_config, t_call_script::TRIGGER_OUT_CALL_FAILED,
			line->get_line_number() + 1);

	switch(r->method) {
	case ACK:
		// re_INVITE is finished now
		line->stop_timer(LTMR_ACK_TIMEOUT, get_object_id());
		line->stop_timer(LTMR_ACK_GUARD, get_object_id());
		remove_client_request(&req_in_invite);
		MEMMAN_DELETE(resp_invite);
		delete resp_invite;
		resp_invite = NULL;

		// If no offer was received in INVITE, then an offer
		// has been sent in 200 OK or reliable 1xx (RFC 3262).
		// Therefor an answer must be present in ACK
		if (!session_re_invite->recvd_offer && r->body) {
			int warn_code;
			string warn_text;

			if (r->body->get_type() != BODY_SDP) {
				// Only SDP bodies are supported
				ui->cb_unsupported_content_type(
					line->get_line_number(), r);
				tear_down = true;
			} else if (session_re_invite->process_sdp_answer(
				(t_sdp *)r->body, warn_code, warn_text))
			{
				session_re_invite->recvd_answer = true;
			} else {
				// SDP answer is not supported.
				// Tear down the call.
				ui->cb_sdp_answer_not_supported(
						line->get_line_number(), warn_text);
				tear_down = true;
			}
		}

		if (!session_re_invite->recvd_offer && !r->body) {
			ui->cb_sdp_answer_missing(line->get_line_number());
			tear_down = true;
		}

		if (end_after_ack) {
			ui->cb_far_end_hung_up(line->get_line_number());
			state = DS_TERMINATED;
		} else {
			state = DS_CONFIRMED;
			if (tear_down) {
				send_bye();
			} else {
				// Make the new session description current
				activate_new_session();
			}
		}
		break;
	case BYE:
		// Send 200 on the BYE request
		resp = r->create_response(R_200_OK);
		line->send_response(resp, tuid, tid);
		
		// Trigger call script
		script_out_call_failed.exec_notify(resp);
		
		MEMMAN_DELETE(resp);
		delete resp;
		
		line->call_hist_record.end_call(true);

		// The session will be ended when an ACK has been
		// received.
		end_after_ack = true;
		break;
	default:
		// ACK has not been received. Handle other incoming request
		// as if we are in the confirmed state. These incoming requests
		// should not change state.
		state_confirmed(r, tuid, tid);
		assert(state == DS_W4ACK_RE_INVITE);
		break;
	}
}

// RFC 3261 13.3.1.4
void t_dialog::state_w4ack(t_line_timer timer) {
	t_ip_port ip_port;

	// NOTE: this code is also executed for re-INVITE ACK time-outs
	//       timeout handling for INVITE/re-INVITE is the same

	switch(timer) {
	case LTMR_ACK_TIMEOUT:
		// Retransmit 2xx response.
		// Send the response directly to the sender thread
		// as the INVITE transaction completed already.
		// (see RFC 3261 17.2.1)
		if (!resp_invite) break; // there is no response to send
		resp_invite->get_destination(ip_port);
		if (ip_port.ipaddr == 0) {
			// This should not happen. The response has been
			// sent before so it should be possible to sent
			// it again. Ignore the timeout. When the ACK
			// guard timer expires, the dialog will be
			// cleaned up.
			break;
		}
		evq_sender->push_network(resp_invite, ip_port);
		line->start_timer(LTMR_ACK_TIMEOUT, get_object_id());
		break;
	case LTMR_ACK_GUARD:
		line->stop_timer(LTMR_ACK_TIMEOUT, get_object_id());
		// Consider dialog as established and tear down call
		remove_client_request(&req_in_invite);
		MEMMAN_DELETE(resp_invite);
		delete resp_invite;
		resp_invite = NULL;
		state = DS_CONFIRMED;
		log_file->write_report("LTMR_ACK_GUARD expired.",
				"t_dialog::state_w4ack");
		if (end_after_ack) {
			state = DS_TERMINATED;
		} else {
			send_bye();
		}
		ui->cb_ack_timeout(line->get_line_number());
		break;
	default:
		// Other timeouts are not expected. Ignore.
		break;
	}
}

void t_dialog::state_w4ack_re_invite(t_line_timer timer) {
	state_w4ack(timer);
}

void t_dialog::state_w4re_invite_resp(t_request *r, t_tuid tuid, t_tid tid) {
	t_response *resp;
	t_user *user_config = phone_user->get_user_profile();
	
	t_call_script script_remote_release(user_config, t_call_script::TRIGGER_REMOTE_RELEASE,
			line->get_line_number() + 1);

	switch(r->method) {
	case BYE:
		resp = r->create_response(R_200_OK);		
		line->send_response(resp, tuid, tid);
		
		// Trigger call script
		script_remote_release.exec_notify(r);
		
		MEMMAN_DELETE(resp);
		delete resp;
		ui->cb_far_end_hung_up(line->get_line_number());
		line->call_hist_record.end_call(true);

		if (!sub_refer) {
			state = DS_TERMINATED;
		} else {
			state = DS_CONFIRMED_SUB;
			if (sub_refer->get_role() == SR_SUBSCRIBER) {
				// End subscription
				sub_refer->unsubscribe();
			}
		}
		break;
	case ACK:
		// Ignore ACK
		break;
	case OPTIONS:
		resp = line->create_options_response(r, true);
		line->send_response(resp, tuid, tid);
		MEMMAN_DELETE(resp);
		delete resp;
	case PRACK:
		// RFC 3262 3
		// This is a late PRACK. Respond in a normal way.
		respond_prack(r, tuid, tid);
		break;
	case SUBSCRIBE:
		process_subscribe(r, tuid, tid);
		break;
	case NOTIFY:
		process_notify(r, tuid, tid);
		break;
	default:
		resp = r->create_response(R_500_INTERNAL_SERVER_ERROR,
			"Waiting for re-INVITE response");
		line->send_response(resp, tuid, tid);
		MEMMAN_DELETE(resp);
		delete resp;
		break;
	}
}

// In the confirmed state, requests will be responded.
void t_dialog::state_confirmed(t_request *r, t_tuid tuid, t_tid tid) {
	t_response *resp;
	t_user *user_config = phone_user->get_user_profile();
	
	t_call_script script_remote_release(user_config, t_call_script::TRIGGER_REMOTE_RELEASE,
			line->get_line_number() + 1);

	switch(r->method) {
	case INVITE:
		// re-INVITE
		process_re_invite(r, tuid, tid);
		break;
	case BYE:
		resp = r->create_response(R_200_OK);		
		line->send_response(resp, tuid, tid);
		
		// Trigger call script
		script_remote_release.exec_notify(r);
		
		MEMMAN_DELETE(resp);
		delete resp;
		ui->cb_far_end_hung_up(line->get_line_number());
		line->call_hist_record.end_call(true);

		if (!sub_refer) {
			state = DS_TERMINATED;
		} else {
			state = DS_CONFIRMED_SUB;
			if (sub_refer->get_role() == SR_SUBSCRIBER) {
				// End subscription
				sub_refer->unsubscribe();
			}
		}
		break;
	case ACK:
		// Ignore ACK
		break;
	case OPTIONS:
		resp = line->create_options_response(r, true);
		line->send_response(resp, tuid, tid);
		MEMMAN_DELETE(resp);
		delete resp;
	case PRACK:
		// RFC 3262 3
		// This is a late PRACK. Respond in a normal way.
		respond_prack(r, tuid, tid);
		break;
	case REFER:
		process_refer(r, tuid, tid);
		break;
	case SUBSCRIBE:
		process_subscribe(r, tuid, tid);
		break;
	case NOTIFY:
		process_notify(r, tuid, tid);
		break;
	case INFO:
		process_info(r, tuid, tid);
		break;
	case MESSAGE:
		process_message(r, tuid, tid);
		break;
	default:
		resp = r->create_response(R_500_INTERNAL_SERVER_ERROR);
		line->send_response(resp, tuid, tid);
		MEMMAN_DELETE(resp);
		delete resp;
		break;
	}
}

void t_dialog::state_confirmed(t_line_timer timer) {
	switch(timer) {
	case LTMR_GLARE_RETRY:
		switch(reinvite_purpose) {
		case REINVITE_HOLD:
			hold();
			break;
		case REINVITE_RETRIEVE:
			retrieve();
			line->retry_retrieve_succeeded();
			// Note that the re-INVITE is not completed here yet.
			// If re-INVITE fails then line->failed_retrieve will
			// be called later.
			break;
		default:
			assert(false);
		}

		break;
	default:
		// Other timeouts are not exepcted. Ignore.
		break;
	}
}

void t_dialog::state_confirmed_sub(t_request *r, t_tuid tuid, t_tid tid) {
	t_response *resp;

	switch(r->method) {
	case OPTIONS:
		resp = line->create_options_response(r, true);
		line->send_response(resp, tuid, tid);
		MEMMAN_DELETE(resp);
		delete resp;
	case SUBSCRIBE:
		process_subscribe(r, tuid, tid);
		break;
	case NOTIFY:
		process_notify(r, tuid, tid);
		break;
	default:
		resp = r->create_response(R_500_INTERNAL_SERVER_ERROR);
		line->send_response(resp, tuid, tid);
		MEMMAN_DELETE(resp);
		delete resp;
		break;
	}

	if (!sub_refer) {
		// The subscription has been terminated already.
		state = DS_TERMINATED;
	} else if (sub_refer->get_state() == SS_TERMINATED) {
		MEMMAN_DELETE(sub_refer);
		delete sub_refer;
		sub_refer = NULL;
		state = DS_TERMINATED;
	}
}

void t_dialog::process_re_invite(t_request *r, t_tuid tuid, t_tid tid) {
	t_response *resp;
	t_user *user_config = phone_user->get_user_profile();

	session_re_invite = session->create_clean_copy();

	// Media information
	int warn_code;
	string warn_text;
	if (r->body) {
		switch(r->body->get_type()) {
		case BODY_SDP:
			if (session_re_invite->
					process_sdp_offer((t_sdp*)r->body,
						warn_code, warn_text))
			{
				session_re_invite->recvd_offer = true;
				break;
			}

			// Unsupported media
			resp = r->create_response(
					R_488_NOT_ACCEPTABLE_HERE);
			resp->hdr_warning.add_warning(t_warning(LOCAL_HOSTNAME,
					0, warn_code, warn_text));
			line->send_response(resp, tuid, tid);
			MEMMAN_DELETE(resp);
			delete resp;
			
			MEMMAN_DELETE(session_re_invite);
			delete session_re_invite;
			session_re_invite = NULL;

			// Stay in the confirmed state. The sender of the
			// request has to determine if the dialog needs to
			// be torn down by sending a BYE.
			return;
		default:
			// Unsupported body type. Reject call.
			resp = r->create_response(
					R_415_UNSUPPORTED_MEDIA_TYPE);

			// RFC 3261 21.4.13
			SET_HDR_ACCEPT(resp->hdr_accept);

			line->send_response(resp, tuid, tid);
			MEMMAN_DELETE(resp);
			delete resp;
			
			MEMMAN_DELETE(session_re_invite);
			delete session_re_invite;
			session_re_invite = NULL;

			// Stay in the confirmed state.
			return;
		}
	}
	
	// If STUN is enabled, then first send a STUN binding request to
	// discover the IP adderss and port for media if no RTP stream
	// is currently active.
	if (phone->use_stun(user_config) && !session->is_rtp_active()) {
		// The STUN transaction may take a while.
		// Send 100 Trying
		resp = r->create_response(R_100_TRYING);
		resp->hdr_to.set_tag("");
		line->send_response(resp, tuid, tid);
		MEMMAN_DELETE(resp);
		delete resp;
		
		if (!stun_bind_media()) {
			// STUN request failed. Send a 500 on the INVITE.
			resp = r->create_response(R_500_INTERNAL_SERVER_ERROR);
			resp->hdr_to.set_tag(local_tag);
			line->send_response(resp, tuid, tid);
			MEMMAN_DELETE(resp);
			delete resp;
				
			state = DS_TERMINATED;
			return;
		}
	}

	// Refresh target
	if (r->hdr_contact.is_populated() &&
	    r->hdr_contact.contact_list.size() > 0)
	{
		remote_target_uri = r->hdr_contact.contact_list.front().uri;
		remote_target_display = r->
			hdr_contact.contact_list.front().display;
	}

	// Send 200 OK
	resp_invite = r->create_response(R_200_OK);
	resp_invite->hdr_to.set_tag(local_tag);

	// Set Contact header
	t_contact_param contact;
	contact.uri.set_url(line->create_user_contact(h_ip2str(resp_invite->get_local_ip())));
	resp_invite->hdr_contact.add_contact(contact);

	// Set Allow and Supported headers
	SET_HDR_ALLOW(resp_invite->hdr_allow, user_config);
	SET_HDR_SUPPORTED(resp_invite->hdr_supported, user_config);

	// RFC 3261 13.3.1.4
	// Create SDP offer if no offer was received in INVITE and no offer
	// was sent in a reliable 1xx response (RFC 3262 5).
	// Otherwise create an SDP answer.
	if (!session_re_invite->recvd_offer && !session_re_invite->sent_offer) {
		session_re_invite->create_sdp_offer(resp_invite, SDP_O_USER);
	} else {
		session_re_invite->create_sdp_answer(resp_invite, SDP_O_USER);
	}

	line->send_response(resp_invite, tuid, tid);
	line->start_timer(LTMR_ACK_GUARD, get_object_id());
	line->start_timer(LTMR_ACK_TIMEOUT, get_object_id());

	state = DS_W4ACK_RE_INVITE;
}

void t_dialog::process_refer(t_request *r, t_tuid tuid, t_tid tid) {
	t_response *resp;
	t_user *user_config = phone_user->get_user_profile();
	t_contact_param contact;
	
	refer_accepted = true;

	// RFC 3515
	if (sub_refer || !user_config->get_allow_refer()) {
		// A reference is already in progress or REFER is not
		// allowed.
		resp = r->create_response(R_603_DECLINE);
		line->send_response(resp, tuid, tid);
		MEMMAN_DELETE(resp);
		delete resp;
		refer_accepted = false;
		return;
	}

	// Check if the URI scheme is supported
	if (r->hdr_refer_to.uri.get_scheme() != "sip") {
		resp = r->create_response(R_416_UNSUPPORTED_URI_SCHEME);
		line->send_response(resp, tuid, tid);
		MEMMAN_DELETE(resp);
		delete resp;
		refer_accepted = false;
		return;
	}

	resp = r->create_response(R_202_ACCEPTED);

	// RFC 3515 2.2
	// Contact header is mandatory
	contact.uri.set_url(line->create_user_contact(h_ip2str(resp->get_local_ip())));
	resp->hdr_contact.add_contact(contact);

	if (r->hdr_refer_sub.is_populated() && !r->hdr_refer_sub.create_refer_sub) {
		// RFC 4488 4
		resp->hdr_refer_sub.set_create_refer_sub(false);
	}
	line->send_response(resp, tuid, tid);
	MEMMAN_DELETE(resp);
	delete resp;

	if (r->hdr_refer_sub.is_populated() && !r->hdr_refer_sub.create_refer_sub) {
		// RFC 4488
		// The REFER-issuer requested not to create an implicit refer
		// subscription.
		log_file->write_report(
			"REFER-issuer requested not to create a refer subscription.",
			"t_dialog::process_refer");
	} else {
		// RFC 3515
		// The event header of a NOTIFY to a first REFER MAY
		// include the id paramter. NOTIFY's to subsequent
		// REFERs MUST include the id parameter (CSeq from REFER).
		sub_refer = new t_sub_refer(this, SR_NOTIFIER,
			ulong2str(r->hdr_cseq.seqnr));
		MEMMAN_NEW(sub_refer);
	
		// Send immediate NOTIFY
		resp = new t_response(R_100_TRYING);
		MEMMAN_NEW(resp);
		if (user_config->get_ask_user_to_refer()) {
			// If the user has to grant permission, then the
			// subscription is pending.
			sub_refer->send_notify(resp, SUBSTATE_PENDING);
		} else {
			sub_refer->send_notify(resp, SUBSTATE_ACTIVE);
		}
		MEMMAN_DELETE(resp);
		delete resp;
	}

	// Ask permission to refer
	if (user_config->get_ask_user_to_refer()) {
		if (r->hdr_referred_by.is_populated()) {
			ui->cb_ask_user_to_refer(user_config,
				r->hdr_refer_to.uri,
				r->hdr_refer_to.display,
				r->hdr_referred_by.uri,
				r->hdr_referred_by.display);
		} else {
			ui->cb_ask_user_to_refer(user_config,
				r->hdr_refer_to.uri,
				r->hdr_refer_to.display,
				t_url(), "");
		}
	} else {
		ui->send_refer_permission(true);
	}
	
	// NOTE: refer_accepted = true, though the answer to permission
	//       is not given yet. So this means, that the refer is not
	//       rejected at this moment. It may be rejected by the user.
}

void t_dialog::recvd_refer_permission(bool permission, t_request *r) {
	t_response *resp;
	
	// NOTE: if the REFER-issuer requested not to create a refer
	// subscription (RFC 4488), then no NOTIFY can be sent to signal
	// the rejection.
	if (!permission && sub_refer) {
		// User denied REFER
		// RFC 3515 2.4.5
		resp = new t_response(R_603_DECLINE);
		MEMMAN_NEW(resp);
		sub_refer->send_notify(resp, SUBSTATE_TERMINATED,
				EV_REASON_REJECTED);
		MEMMAN_DELETE(resp);
		delete resp;
	}
	
	refer_accepted = permission;
}

void t_dialog::process_subscribe(t_request *r, t_tuid tuid, t_tid tid) {
	t_response *resp;

	if (sub_refer && sub_refer->match(r)) {
		sub_refer->recv_subscribe(r, tuid, tid);
		if (sub_refer->get_state() == SS_TERMINATED) {
			MEMMAN_DELETE(sub_refer);
			delete sub_refer;
			sub_refer = NULL;
		}

		return;
	}

	resp = r->create_response(R_481_TRANSACTION_NOT_EXIST,
			REASON_481_SUBSCRIPTION_NOT_EXIST);
	line->send_response(resp, tuid, tid);
	MEMMAN_DELETE(resp);
	delete resp;
}

void t_dialog::process_notify(t_request *r, t_tuid tuid, t_tid tid) {
	t_response *resp;

	if (!sub_refer &&
	    (refer_state == REFST_W4RESP || refer_state == REFST_W4NOTIFY))
	{
		// First NOTIFY after sending a REFER
		sub_refer = new t_sub_refer(this, SR_SUBSCRIBER, r->hdr_event.id);
		MEMMAN_NEW(sub_refer);
		refer_state = REFST_PENDING;
	}

	if (sub_refer && sub_refer->match(r)) {
		sub_refer->recv_notify(r, tuid, tid);
		if (sub_refer->get_state() == SS_TERMINATED) {
			// Set the refer state to NULL before calling the UI
			// call back functions as the user interface might use
			// the refer state to render the correct status to the
			// user.
			refer_state = REFST_NULL;
		
			// Determine outcome of the reference
			switch(sub_refer->get_sr_result()) {
			case SRR_INPROG:
				// The outcome of the reference is unknown.
				// Treat it as a success as no new info will
				// come to the referrer.
				refer_succeeded = true;
				ui->cb_refer_result_inprog(line->get_line_number());
				break;
			case SRR_FAILED:
				refer_succeeded = false;
				ui->cb_refer_result_failed(line->get_line_number());
				break;
			case SRR_SUCCEEDED:
				refer_succeeded = true;
				ui->cb_refer_result_success(line->get_line_number());
				break;
			default:
				assert(false);
			}

			MEMMAN_DELETE(sub_refer);
			delete sub_refer;
			sub_refer = NULL;
		} else if (!sub_refer->is_pending()) {
			refer_state = REFST_ACTIVE;
		}

		return;
	}

	// RFC 3265 3.2.4
	resp = r->create_response(R_481_TRANSACTION_NOT_EXIST,
			REASON_481_SUBSCRIPTION_NOT_EXIST);
	line->send_response(resp, tuid, tid);
	MEMMAN_DELETE(resp);
	delete resp;
}

void t_dialog::process_info(t_request *r, t_tuid tuid, t_tid tid) {
	t_response *resp;
	
	// RFC 2976 2.2
	// A 200 OK response MUST be sent by a UAS for an INFO request with
        // no message body if the INFO request was successfully received for
        // an existing call.
	if (!r->body) {
		resp = r->create_response(R_200_OK);
		line->send_response(resp, tuid, tid);
		MEMMAN_DELETE(resp);
		delete resp;
		
		return;
	}
	
	if (r->body->get_type() != BODY_DTMF_RELAY) {
		resp = r->create_response(R_415_UNSUPPORTED_MEDIA_TYPE);
		resp->hdr_accept.add_media(t_media("application", "dtmf-relay"));
		line->send_response(resp, tuid, tid);
		MEMMAN_DELETE(resp);
		delete resp;
		
		return;
	}
	
	char dtmf_signal = ((t_sip_body_dtmf_relay *)r->body)->signal;
	if (!VALID_DTMF_SYM(dtmf_signal)) {
		resp = r->create_response(R_400_BAD_REQUEST, "Invalid DTMF signal");
		line->send_response(resp, tuid, tid);
		MEMMAN_DELETE(resp);
		delete resp;
		
		return;
	}
	
	resp = r->create_response(R_200_OK);
	line->send_response(resp, tuid, tid);
	MEMMAN_DELETE(resp);
	delete resp;
	
	ui->cb_dtmf_detected(line->get_line_number(), char2dtmf_ev(dtmf_signal));
}

void t_dialog::process_message(t_request *r, t_tuid tuid, t_tid tid) {
	t_response *resp;
	t_user *user_config = phone_user->get_user_profile();
	
	log_file->write_report("Received in-dialog MESSAGE.",
		"t_dialog::process_message", LOG_NORMAL, LOG_DEBUG);
		
	if (!r->body || !MESSAGE_CONTENT_TYPE_SUPPORTED(*r)) {
		resp = r->create_response(R_415_UNSUPPORTED_MEDIA_TYPE);
		// RFC 3261 21.4.13
		SET_MESSAGE_HDR_ACCEPT(resp->hdr_accept);
		line->send_response(resp, tuid, tid);
		MEMMAN_DELETE(resp);
		delete resp;
		
		return;
	}
	
	if (r->body && r->body->get_type() == BODY_IM_ISCOMPOSING_XML) {
		// Message composing indication
		t_im_iscomposing_xml_body *sb = dynamic_cast<t_im_iscomposing_xml_body *>(r->body);
		im::t_composing_state state = im::string2composing_state(sb->get_state());
		time_t refresh = sb->get_refresh();
		
		ui->cb_im_iscomposing_request(line->get_user(), r, state, refresh);
		resp = r->create_response(R_200_OK);
	} else {
		// Instant message
		bool accepted = ui->cb_message_request(line->get_user(), r);
		if (accepted) {
			resp = r->create_response(R_200_OK);
		} else {
			if (user_config->get_im_max_sessions() == 0) {
				resp = r->create_response(R_603_DECLINE);
			} else {
				resp = r->create_response(R_486_BUSY_HERE);
			}
		}
	}
	
	line->send_response(resp, tuid, tid);
	MEMMAN_DELETE(resp);
	delete resp;
}

// INVITE sent. Waiting for a first non-100 response.
void t_dialog::state_w4invite_resp(t_response *r, t_tuid tuid, t_tid tid) {
	if (r->hdr_cseq.method != INVITE) return;
	t_user *user_config = phone_user->get_user_profile();

	// 1XX (except 100) and 2XX establish the dialog.
	// Update the state for dialog establishment.
	// RFC 3261 12.1.2
	switch (r->get_class()) {
	case R_1XX:
		if (r->code == R_100_TRYING) break;
		
		// RFC 3262 4
		// Discard retransmissions and out-of-sequence reliable
		// provisional responses.
		if (must_discard_100rel(r)) return;

		// fall thru
	case R_2XX:
		// Set remote tag
		remote_tag = r->hdr_to.tag;

		create_route_set(r);
		create_remote_target(r);

		// Set remote URI and display name
		remote_uri = r->hdr_to.uri;
		remote_display = r->hdr_to.display;

		process_1xx_2xx_invite_resp(r);
		break;
	default:
		break;
	}

	// RFC 3262
	// Send PRACK if required
	send_prack_if_required(r);
	
	t_call_script script_out_call_answered(user_config,
			t_call_script::TRIGGER_OUT_CALL_ANSWERED,
			line->get_line_number() + 1);
	t_call_script script_out_call_failed(user_config, 
			t_call_script::TRIGGER_OUT_CALL_FAILED,
			line->get_line_number() + 1);

	switch (r->get_class()) {
	case R_1XX:
		// Provisional response received.
		line->ci_set_last_provisional_reason(r->reason);
		ui->cb_provisional_resp_invite(line->get_line_number(), r);
		if (r->code > R_100_TRYING && r->hdr_to.tag.size() > 0) {
			state = DS_EARLY;
		} else {
			state = DS_W4INVITE_RESP2;
		}

		// User indicated that the request should be cancelled.
		// Now that the first provisional response has been received,
		// a CANCEL can be sent.
		if (request_cancelled) {
			send_cancel(true);
		}

		break;
	case R_2XX:
		// Stop cancel guard timer if it was running
		line->stop_timer(LTMR_CANCEL_GUARD, get_object_id());
	
		// Success received.
		ack_2xx_invite(r);

		// Check for REFER support
		// If the Allow header is not present then assume REFER
		// is supported.
		if (!r->hdr_allow.is_populated() ||
		    r->hdr_allow.contains_method(REFER))
		{
			line->ci_set_refer_supported(true);
		}
		
		// Trigger call script
		script_out_call_answered.exec_notify(r);

		ui->cb_call_answered(user_config, line->get_line_number(), r);
		line->call_hist_record.answer_call(r);
		state = DS_CONFIRMED;

		if (request_cancelled) {
			// User indicated that the request should be cancelled,
			// but no response was received yet. A final response
			// has been received. Instead of CANCEL a BYE will be
			// sent now.
			send_bye();
		} else if (end_after_2xx_invite) {
			// Or user cancelled the request already, but the 2XX
			// glared with CANCEL.
			log_file->write_report("CANCEL / 2XX INVITE glare.",
				"t_dialog::state_w4invite_resp");
			send_bye();
		}

		break;
	case R_3XX:
	case R_4XX:
	case R_5XX:
	case R_6XX:
	default:
		// Stop cancel guard timer if it was running
		line->stop_timer(LTMR_CANCEL_GUARD, get_object_id());
	
		// Final response (failure) received.
		// Treat unknown response classes as failure.
		
		// Trigger call script
		script_out_call_failed.exec_notify(r);
	
		ui->cb_stop_call_notification(line->get_line_number());
		ui->cb_call_failed(user_config, line->get_line_number(), r);
		line->call_hist_record.fail_call(r);
		remove_client_request(&req_out_invite);
		state = DS_TERMINATED;
		break;
	}

	// Notify progress to the referror if this is a referred call
	if (is_referred_call) {
		get_phone()->notify_refer_progress(r, line->get_line_number());
	}
}

void t_dialog::state_w4invite_resp(t_line_timer timer) {
	switch (timer) {
	case LTMR_CANCEL_GUARD:
		log_file->write_report("Timer LTMR_CANCEL_GUARD expired.",
				"t_dialog::state_w4invite_resp", LOG_NORMAL, LOG_WARNING);
	
		// CANCEL has been responded to, but 487 on INVITE was never
		// received. Abort the INVITE transaction.
		if (req_out_invite) {
			t_tid _tid = req_out_invite->get_tid();
			if (_tid > 0) {
				evq_trans_mgr->push_abort_trans(_tid);
			}
		}
		break;
	default:
		// Ignore other timeouts
		break;
	}
}

// INVITE response sent. At least 1 provisional response (not 100 Trying)
// received.
void t_dialog::state_early(t_response *r, t_tuid tuid, t_tid tid) {
	if (r->hdr_cseq.method != INVITE) return;
	t_user *user_config = phone_user->get_user_profile();

	switch (r->get_class()) {
	case R_1XX:
		// RFC 3262 4
		// Discard retransmissiona and out-of-sequence reliable
		// provisional responses.
		if (must_discard_100rel(r)) return;

		// fall thru
	case R_2XX:
		create_route_set(r);
		create_remote_target(r);
		process_1xx_2xx_invite_resp(r);
		break;
	default:
		break;
	}

	// RFC 3262
	// Send PRACK if required
	send_prack_if_required(r);
	
	t_call_script script_out_call_answered(user_config, 
			t_call_script::TRIGGER_OUT_CALL_ANSWERED,
			line->get_line_number() + 1);
	t_call_script script_out_call_failed(user_config, 
			t_call_script::TRIGGER_OUT_CALL_FAILED,
			line->get_line_number() + 1);

	switch (r->get_class()) {
	case R_1XX:
		// Provisional response received.
		line->ci_set_last_provisional_reason(r->reason);
		ui->cb_provisional_resp_invite(line->get_line_number(), r);

		if (request_cancelled) {
			send_cancel(true);
		}

		break;
	case R_2XX:
		// Stop cancel guard timer if it was running
		line->stop_timer(LTMR_CANCEL_GUARD, get_object_id());
		
		// Success received.
		ack_2xx_invite(r);

		// Check for REFER support
		// If the Allow header is not present then assume REFER
		// is supported.
		if (!r->hdr_allow.is_populated() ||
		    r->hdr_allow.contains_method(REFER))
		{
			line->ci_set_refer_supported(true);
		}
		
		// Trigger call script
		script_out_call_answered.exec_notify(r);

		ui->cb_call_answered(user_config, line->get_line_number(), r);
		line->call_hist_record.answer_call(r);
		state = DS_CONFIRMED;

		if (request_cancelled) {
			// User indicated that the request should be cancelled,
			// but no response was received yet. A final response
			// has been received. Instead of CANCEL a BYE will be
			// sent now.
			send_bye();
		} else if (end_after_2xx_invite) {
			// Or user cancelled the request already, but the 2XX
			// glared with CANCEL.
			log_file->write_report("CANCEL / 2XX INVITE glare.",
				"t_dialog::state_w4invite_resp");
			send_bye();
		}

		break;
	case R_3XX:
	case R_4XX:
	case R_5XX:
	case R_6XX:
	default:
		// Stop cancel guard timer if it was running
		line->stop_timer(LTMR_CANCEL_GUARD, get_object_id());
		
		// Final response (failure) received.
		// Treat unknown response classes as failure.

		// Trigger call script
		script_out_call_failed.exec_notify(r);
		
		ui->cb_stop_call_notification(line->get_line_number());
		ui->cb_call_failed(user_config, line->get_line_number(), r);
		line->call_hist_record.fail_call(r);
		remove_client_request(&req_out_invite);
		state = DS_TERMINATED;
		break;
	}

	// Notify progress to the referror if this is a referred call
	if (is_referred_call) {
		get_phone()->notify_refer_progress(r, line->get_line_number());
	}
}

void t_dialog::state_early(t_line_timer timer) {
	switch (timer) {
	case LTMR_CANCEL_GUARD:
		log_file->write_report("Timer LTMR_CANCEL_GUARD expired.",
				"t_dialog::state_early", LOG_NORMAL, LOG_WARNING);
	
		// CANCEL has been responded to, but 487 on INVITE was never
		// received. Abort the INVITE transaction.
		if (req_out_invite) {
			t_tid _tid = req_out_invite->get_tid();
			if (_tid > 0) {
				evq_trans_mgr->push_abort_trans(_tid);
			}
		}
		break;
	default:
		// Ignore other timeouts
		break;
	}
}

// BYE sent. Waiting for response.
void t_dialog::state_w4bye_resp(t_response *r, t_tuid tuid, t_tid tid) {
	if (r->hdr_cseq.method != BYE) return;

	switch (r->get_class()) {
	case R_1XX:
		// Provisional response received. Wait for final response.
		break;
	default:
		// All final responses terminate the dialog.
		remove_client_request(&req_out);
		if (!sub_refer) {
			state = DS_TERMINATED;
		} else {
			state = DS_CONFIRMED_SUB;
			if (sub_refer->get_role() == SR_SUBSCRIBER) {
				// End subscription
				sub_refer->unsubscribe();
			}
		}
		break;
	}
}

void t_dialog::state_w4bye_resp(t_request *r, t_tuid tuid, t_tid tid) {
	t_response *resp;

	switch(r->method) {
	case BYE:
		resp = r->create_response(R_200_OK);
		line->send_response(resp, tuid, tid);
		MEMMAN_DELETE(resp);
		delete resp;

		// A BYE glare situation. Keep waiting for the BYE
		// response.
		break;
	default:
		resp = r->create_response(R_500_INTERNAL_SERVER_ERROR);
		line->send_response(resp, tuid, tid);
		MEMMAN_DELETE(resp);
		delete resp;
		break;
	}
}

// Confirmed dialog. Responses are for mid-dialog requests.
void t_dialog::state_confirmed_resp(t_response *r, t_tuid tuid, t_tid tid) {
	// 1XX responses are not expected. If they are received
	// then simply ignore them.
	if (r->is_provisional()) return;

	switch (r->hdr_cseq.method) {
	case OPTIONS:
		ui->cb_options_response(r);
		remove_client_request(&req_out);
		break;
	case REFER:
		remove_client_request(&req_refer);

		if (refer_state != REFST_W4RESP) {
			// NOTIFY has already been received. No need to
			// process the REFER response anymore. Interesting
			// issue might be: what if NOTIFY has been received and
			// now a failure response comes in?
			break;
		}

		if (!r->is_success()) {
			// REFER failed
			refer_state = REFST_NULL;
			refer_succeeded = false;
			
			// KLUDGE: only signal REFER failure in case of
			//         non-408/481 responses. These responses
			//         clear the line, so the upper layers should not
			//         take action on the failed refer.
			if (r->code != R_408_REQUEST_TIMEOUT ||
	    		    r->code == R_481_TRANSACTION_NOT_EXIST)
	    		{
				out_refer_req_failed = true;
			}
			
			ui->cb_refer_failed(line->get_line_number(), r);
			break;
		}

		refer_state = REFST_W4NOTIFY;
		break;
	case INFO:
		remove_client_request(&req_info);
		
		if (!dtmf_queue.empty()) {
			char digit = dtmf_queue.front();
			dtmf_queue.pop();
			send_dtmf(digit, false, true);
		}
		
		break;
	default:
		// The received response should match the pending request.
		// So this point should never be reached.
		assert(false);
		break;
	}

	// RFC 3261 12.2.1.2
	// If a mid-dialog request is timed out, or the call/transaction
	// does not exist anymore at the server, then terminate the
	// dialog.
	if (r->code == R_408_REQUEST_TIMEOUT ||
	    r->code == R_481_TRANSACTION_NOT_EXIST)
	{
		send_bye();
	}
}

void t_dialog::state_w4re_invite_resp(t_response *r, t_tuid tuid, t_tid tid) {
	if (r->hdr_cseq.method != INVITE) return;

	switch (r->get_class()) {
	case R_1XX:
		if (r->code == R_100_TRYING) break;

		// RFC 3262 4
		// Discard retransmissiona and out-of-sequence reliable
		// provisional responses.
		if (must_discard_100rel(r)) return;

		if (state == DS_W4RE_INVITE_RESP2) {
			// RFC 3262
			// Discard retransmissions and out-of-order
			// reliable provisional responses.
			if (must_discard_100rel(r)) return;
		}

		// RFC 3262
		// Send PRACK if required
		send_prack_if_required(r);

		// fall thru
	case R_2XX:
		// Process SDP answer if answer is present and no
		// answer has been received yet.
		if (!session_re_invite->recvd_answer && r->body) {
			int warn_code;
			string warn_text;

			if (r->body->get_type() != BODY_SDP) {
				// Only SDP bodies are supported
				ui->cb_unsupported_content_type(
					line->get_line_number(), r);
				request_cancelled = true;
			} else if (session_re_invite->
				   process_sdp_answer((t_sdp *)r->body,
				   	warn_code, warn_text))
			{
				session_re_invite->recvd_answer = true;
			} else {
				// SDP answer is not supported. Cancel
				// the INVITE.
				request_cancelled = true;
				ui->cb_sdp_answer_not_supported(
						line->get_line_number(), warn_text);
				break;
			}
		}

		// This implementation always sends an offer in
		// INVITE. So an answer must be in a 2XX response
		// as PRACK is not supported.
		if (r->get_class() == R_2XX && !r->body) {
			request_cancelled = true;
			ui->cb_sdp_answer_missing(line->get_line_number());
			break;
		}

		// Refresh target URI and display name
		if (r->get_class() == R_2XX &&
		    r->hdr_contact.is_populated() &&
	    	    r->hdr_contact.contact_list.size() > 0)
		{
			remote_target_uri = r->
				hdr_contact.contact_list.front().uri;
			remote_target_display = r->
				hdr_contact.contact_list.front().display;
		}

		break;
	default:
		break;
	}

	switch (r->get_class()) {
	case R_1XX:
		// Provisional response received.
		state = DS_W4RE_INVITE_RESP2;

		// Start re-INVITE guard timer (no RFC requirement)
		line->start_timer(LTMR_RE_INVITE_GUARD, get_object_id());

		// User indicated that the request should be cancelled.
		// Now that the first provional response has been received,
		// a CANCEL can be sent.
		if (request_cancelled) {
			send_cancel(true);
		}

		break;
	case R_2XX:
		// Success received.
		line->stop_timer(LTMR_RE_INVITE_GUARD, get_object_id());

		ack_2xx_invite(r);
		ui->cb_reinvite_success(line->get_line_number(), r);
		state = DS_CONFIRMED;

		if (request_cancelled) {
			// User indicated that the request should be cancelled,
			// but no response was received yet. A final response
			// has been received. Instead of CANCEL a BYE will be
			// sent now.
			send_bye();
		} else if (end_after_2xx_invite) {
			// Or user cancelled the request already, but the 2XX
			// glared with CANCEL.
			log_file->write_report("CANCEL / 2XX INVITE glare.",
				"t_dialog::state_w4invite_resp");
			send_bye();
		} else {
			// Make the re-INIVTE session info the current info
			activate_new_session();
		}

		break;
	case R_3XX:
	case R_4XX:
	case R_5XX:
	case R_6XX:
	default:
		// Final response (failure) received.
		// Treat unknown response classes as failure.
		line->stop_timer(LTMR_RE_INVITE_GUARD, get_object_id());
		ui->cb_reinvite_failed(line->get_line_number(), r);
		remove_client_request(&req_out_invite);

		// RFC 3261 14.1
		// delete re-INVITE session info. Old session info
		// stays as re-INVITE failed.
		MEMMAN_DELETE(session_re_invite);
		delete session_re_invite;
		session_re_invite = NULL;

		state = DS_CONFIRMED;

		switch(reinvite_purpose) {
		case REINVITE_HOLD:
			// A call hold may not fail for the user as
			// this cause problems with soundcard access and
			// showing line status in the GUI. Even though re-INVITE
			// failed, the RTP still stopped. So simply indicated
			// that the hold failed, such that a subsequent retrieve
			// can simply restart the RTP.
			hold_failed = true;
			break;
		case REINVITE_RETRIEVE:
			line->failed_retrieve();
			if (r->code != R_491_REQUEST_PENDING) {
				ui->cb_retrieve_failed(line->get_line_number(), r);
			}
			break;
		default:
			assert(false);
		}

		// RFC 3261 14.1
		// Start wait timer before retrying a re-INVITE after a
		// glare.
		if (r->code == R_491_REQUEST_PENDING) {
			line->start_timer(LTMR_GLARE_RETRY, get_object_id());
		}

		// RFC 3261 14.1
		if (r->code == R_408_REQUEST_TIMEOUT ||
		    r->code == R_481_TRANSACTION_NOT_EXIST)
		{
			send_bye();
		}
		break;
	}
}

void t_dialog::state_w4re_invite_resp(t_line_timer timer) {
	switch(timer) {
	case LTMR_RE_INVITE_GUARD:
		// Abort the INVITE as the user cannot terminate
		// it in a normal way.
		if (req_out_invite) {
			t_tid _tid = req_out_invite->get_tid();
			if (_tid > 0) {
				evq_trans_mgr->push_abort_trans(_tid);
			}
		} else {
			// Consider this as if a 408 Timeout response has
			// been received. Terminate the dialog.
			send_bye();
		}
		break;
	default:
		break;
	}
}

void t_dialog::activate_new_session(void) {
	if (session->equal_audio(*session_re_invite)) {
		log_file->write_report("SDP in re-INVITE is a noop.",
			"t_dialog::activate_new_session");
	
		MEMMAN_DELETE(session_re_invite);
		delete session_re_invite;
		session_re_invite = NULL;
		return;
	}
	
	log_file->write_report("Renew session as specified by SDP in re-INVITE.",
		"t_dialog::activate_new_session");

	// Stop current session
	MEMMAN_DELETE(session);
	delete session;

	// Create new session
	session = session_re_invite;
	session_re_invite = NULL;
	session->start_rtp();
}

void t_dialog::process_1xx_2xx_invite_resp(t_response *r) {
	t_user *user_config = phone_user->get_user_profile();
	
	// Process SDP answer if answer is present and no
	// answer has been received yet.
	if (r->body) {
		int warn_code;
		string warn_text;

		if (r->body->get_type() != BODY_SDP) {
			// Only SDP bodies are supported
			ui->cb_unsupported_content_type(line->get_line_number(), r);
			request_cancelled = true;
		} else if (!session->recvd_answer || 
		           (user_config->get_allow_sdp_change() && 
		            ((t_sdp *)r->body)->origin.session_version !=
		            session->dst_sdp_version))
		{
			// Only process SDP if no SDP was received yet (RFC 3261
			// 13.3.1. Or process SDP if overridden by the
			// allow_sdp_change setting in the user profile.
			// A changed SDP must have a new version number (RFC 3264)
			if (session->process_sdp_answer((t_sdp *)r->body,
					warn_code, warn_text))
			{
				// If this is a changed SDP, then stop the
				// current RTP stream based on the previous SDP.
				if (session->recvd_answer) session->stop_rtp();
				
				session->recvd_answer = true;
	
				// The following code part handles the ugly interaction
				// between forking and early media (Vonage uses this).
				// In case of forking 1xx responses with SDP may com
				// from different destinations. Only the first 1xx will
				// create a media stream. Media streams on other legs cannot
				// be created as that would give sound conflicts.
				// When a 2xx response with SDP is received, an early media
				// stream on another leg must be killed.
				// Due to forking multiple 2xx repsonses from different
				// destinations may be received. Only the first 2xx response
				// will create a media session. The other dialogs receiving
				// a 2xx will be released immediately anyway (see line.cpp).
				bool start_media = true;
				t_dialog *d = line->get_dialog_with_active_session();
				if (d != NULL) {
					if (r->get_class() == R_2XX &&
					    d->get_state() != DS_CONFIRMED)
					{
						log_file->write_header(
							"t_dialog::process_1xx_2xx_invite_resp");
						log_file->write_raw(
							"Kill early media on another dialog, id=");
						log_file->write_raw(d->get_object_id());
						log_file->write_endl();
						log_file->write_footer();
						
						d->kill_rtp();
					} else {
						log_file->write_header(
							"t_dialog::process_1xx_2xx_invite_resp");
						log_file->write_raw(
							"Cannot start media as another dialog (id=");
						log_file->write_raw(d->get_object_id());
						log_file->write_raw(") already has media.\n");
						log_file->write_footer();
					
						start_media = false;
					}
				}
				
				if (start_media) {
					if (r->is_provisional()) {
						log_file->write_report("Starting early media.",
							"t_dialog::process_1xx_2xx_invite_resp");
					}
		
					// Stop locally played tones to free the soundcard
					// for the voice stream
					ui->cb_stop_call_notification(line->get_line_number());
		
					session->start_rtp();
				}
			} else {
				// SDP answer is not supported. Cancel
				// the INVITE.
				request_cancelled = true;
				ui->cb_sdp_answer_not_supported(
						line->get_line_number(), warn_text);
			}
		}
	} else if (r->code == R_180_RINGING &&
	           !ringing_received && !session->recvd_answer)
	{
		// There is no SDP and far-end indicated that it is ringing
		// so generate ring back tone locally.
		ui->cb_play_ringback(user_config);
		ringing_received = true;
	}

	// This implementation always sends an offer in
	// INVITE. So an answer must be in a 2XX response if
	// no answer has been received in a provisional response.
	if (!session->recvd_answer && r->get_class() == R_2XX && !r->body) {
		request_cancelled = true;
		ui->cb_sdp_answer_missing(line->get_line_number());
	}
	
	// RFC 3261 13.3.1.4
	// A 2XX response to an INVITE should contain a Supported header
	// listing all supported extensions.
	// Set extensions supported by remote party
	if (r->get_class() == R_2XX && r->hdr_supported.is_populated()) {
		remote_extensions.insert(r->hdr_supported.features.begin(),
				r->hdr_supported.features.end());
	}
}

void t_dialog::ack_2xx_invite(t_response *r) {
	t_ip_port ip_port;
	t_user *user_config = phone_user->get_user_profile();

	if (ack) {
		// delete previous cached ACK
		MEMMAN_DELETE(ack);
		delete ack;
	}
	ack = create_request(ACK);
	ack->get_destination(ip_port, *user_config);

	// If for some strange reason the destination could
	// not be computed then wait for a retransmission of
	// 2XX.
	if (ip_port.ipaddr != 0 && ip_port.port != 0) {
		evq_sender->push_network(ack, ip_port);
	} else {
		log_file->write_header("t_dialog::ack_2xx_invite", LOG_SIP, LOG_CRITICAL);
		log_file->write_raw("Cannot determine destination IP address for ACK.\n\n");
		log_file->write_raw(ack->encode());
		log_file->write_footer();
	}

	remove_client_request(&req_out_invite);
}

void t_dialog::send_prack_if_required(t_response *r) {
	t_user *user_config = phone_user->get_user_profile();
	
	// RFC 3262
	// Send PRACK if needed
	if (r->get_class() == R_1XX && r->code != R_100_TRYING) {
		// RFC 3262 4
		// Send PRACK if the 1xx response is sent reliable and 100rel
		// is enabled.
		if (r->hdr_to.tag.size() > 0 &&
		    r->hdr_require.contains(EXT_100REL) &&
		    r->hdr_rseq.is_populated() &&
		    remote_target_uri.is_valid() &&
		    user_config->get_ext_100rel() != EXT_DISABLED)
		{
			t_request *prack = create_request(PRACK);
			prack->hdr_rack.set_method(r->hdr_cseq.method);
			prack->hdr_rack.set_cseq_nr(r->hdr_cseq.seqnr);
			prack->hdr_rack.set_resp_nr(r->hdr_rseq.resp_nr);

			// Delete previous PRACK request if it is still pending
			if (req_prack) {
				log_file->write_report("Previous PRACK still pending.",
					"t_dialog::send_prack_if_needed");
				remove_client_request(&req_prack);
			}

			req_prack = new t_client_request(user_config, prack, 0);
			MEMMAN_NEW(req_prack);
			line->send_request(prack, req_prack->get_tuid());
			MEMMAN_DELETE(prack);
			delete prack;
		}
	}
}

bool t_dialog::must_discard_100rel(t_response *r) {
	t_user *user_config = phone_user->get_user_profile();
	
	// RFC 3262 4
	// Discard retransmissiona and out-of-sequence reliable
	// provisional responses.
	if (r->code > R_100_TRYING && r->hdr_to.tag.size() > 0 &&
	    r->hdr_require.contains(EXT_100REL) &&
	    r->hdr_rseq.is_populated() &&
	    user_config->get_ext_100rel() != EXT_DISABLED)
	{
		if (remote_resp_nr == 0) {
			// This is the first response with a repsonse nr.
			// Initialize the remote response nr
			remote_resp_nr = r->hdr_rseq.resp_nr;
			return false;
		}
	
		if (r->hdr_rseq.resp_nr <= remote_resp_nr) {
			// This is a retransmission.
			// PRACK has already been sent. The transaction
			// layer takes care of retransmitting PRACK
			// if PRACK got lost.
			log_file->write_report("Discard 1xx retransmission.",
				"t_dialog::must_discard_100rel");
			return true;
		}

		if (r->hdr_rseq.resp_nr != remote_resp_nr + 1) {
			// A provisional response has been lost.
			// Discard this response and wait for a retransmission
			// of the lost response.
			log_file->write_report("Discard out-of-order 1xx",
				"t_dialog::must_discard_100rel");
			return true;
		}
	}

	remote_resp_nr = r->hdr_rseq.resp_nr;
	return false;
}

bool t_dialog::respond_prack(t_request *r, t_tuid tuid, t_tid tid) {
	t_response *resp;

	// RFC 3262 3
	if (resp_1xx_invite &&
	    r->hdr_rack.method == resp_1xx_invite->hdr_cseq.method &&
	    r->hdr_rack.cseq_nr == resp_1xx_invite->hdr_cseq.seqnr &&
	    r->hdr_rack.resp_nr == resp_1xx_invite->hdr_rseq.resp_nr)
	{
		// The provisional response has been delivered now.
		line->stop_timer(LTMR_100REL_TIMEOUT, get_object_id());
		line->stop_timer(LTMR_100REL_GUARD, get_object_id());
		MEMMAN_DELETE(resp_1xx_invite);
		delete resp_1xx_invite;
		resp_1xx_invite = NULL;

		// Send 200 on the PRACK request
		resp = r->create_response(R_200_OK);
		line->send_response(resp, tuid, tid);
		MEMMAN_DELETE(resp);
		delete resp;

		return true;
	} else {
		// PRACK does not match pending 1xx response
		// Send a 481 on the PRACK request
		resp = r->create_response(R_481_TRANSACTION_NOT_EXIST);
		line->send_response(resp, tuid, tid);
		MEMMAN_DELETE(resp);
		delete resp;

		return false;
	}
}

void t_dialog::send_request(t_request *r, t_tuid tuid) {
	line->send_request(r, tuid);
}

////////////
// Public
////////////

t_dialog::t_dialog(t_line *_line) :
	t_abstract_dialog(_line->get_phone_user())
{
	line = _line;
	
	req_out = NULL;
	req_out_invite = NULL;
	req_in_invite = NULL;
	req_cancel = NULL;
	req_prack = NULL;
	req_refer = NULL;
	req_info = NULL;
	req_stun = NULL;

	request_cancelled = false;
	end_after_ack = false;
	end_after_2xx_invite = false;
	answer_after_prack = false;
	ringing_received = false;
	
	resp_invite = NULL;
	resp_1xx_invite = NULL;
	ack = NULL;

	state = DS_NULL;

	// Timers
	dur_ack_timeout = 0;
	id_ack_timeout = 0;
	id_ack_guard = 0;
	id_re_invite_guard = 0;
	id_glare_retry = 0;
	id_cancel_guard = 0;

	// RFC 3262
	// Timers
	dur_100rel_timeout = 0;
	id_100rel_timeout = 0;
	id_100rel_guard = 0;
	
	t_user *user_config = phone_user->get_user_profile();

	// Create session
	session = new t_session(this, USER_HOST(user_config, AUTO_IP4_ADDRESS), line->get_rtp_port());
	MEMMAN_NEW(session);
	session_re_invite = NULL;

	// Subscription
	sub_refer = NULL;
	is_referred_call = false;
	refer_state = REFST_NULL;
	refer_accepted = false;
	refer_succeeded = false;
	out_refer_req_failed = false;
}

t_dialog::~t_dialog() {
	if (req_out) remove_client_request(&req_out);
	if (req_out_invite) remove_client_request(&req_out_invite);
	if (req_in_invite) remove_client_request(&req_in_invite);
	if (req_cancel) remove_client_request(&req_cancel);
	if (req_prack) remove_client_request(&req_prack);
	if (req_refer) remove_client_request(&req_refer);
	if (req_info) remove_client_request(&req_info);
	if (req_stun) remove_client_request(&req_stun);
	if (resp_invite) { MEMMAN_DELETE(resp_invite); delete resp_invite; }
	if (resp_1xx_invite) {
		MEMMAN_DELETE(resp_1xx_invite);
		delete resp_1xx_invite;
	}
	if (ack) { MEMMAN_DELETE(ack); delete ack; }
	if (session) { MEMMAN_DELETE(session); delete session; }
	if (session_re_invite) {
		MEMMAN_DELETE(session_re_invite);
		delete session_re_invite;
	}
	if (sub_refer) { MEMMAN_DELETE(sub_refer); delete sub_refer; }
	
	for (list<t_client_request *>::iterator i = inc_req_queue.begin();
	     i != inc_req_queue.end(); i++)
	{
		MEMMAN_DELETE(*i);
		delete *i;
	}
}

// Copy will only be used on the open dialog.
t_dialog *t_dialog::copy(void) {
	t_dialog *d = new t_dialog(*this);
	MEMMAN_NEW(d);

	d->generate_new_id();

	// Increment reference count on client request
	if (req_out) d->req_out->inc_ref_count();
	if (req_out_invite) d->req_out_invite->inc_ref_count();
	if (req_in_invite) d->req_in_invite->inc_ref_count();
	if (req_prack) d->req_prack->inc_ref_count();
	if (req_refer) d->req_refer->inc_ref_count();
	if (req_stun) d->req_stun->inc_ref_count();

	// The open dialog will handle the CANCEL, so delete it
	// from the copy.
	if (req_cancel) d->req_cancel = NULL;

	if (resp_invite) d->resp_invite = (t_response *)resp_invite->copy();
	if (resp_1xx_invite) d->resp_1xx_invite = (t_response *)resp_1xx_invite->copy();
	if (ack) d->ack = (t_request *)ack->copy();
	dur_ack_timeout = 0;
	id_ack_timeout = 0;
	id_ack_guard = 0;
	dur_100rel_timeout = 0;
	id_100rel_timeout = 0;
	id_100rel_guard = 0;

	if (session) {
		d->session = new t_session(*session);
		MEMMAN_NEW(d->session);
		d->session->set_owner(d);

		// If an audio session was already created for early media
		// then the audio session will be moved to the copy of the
		// dialog. Only 1 dialog can have an audio session.
		// See process_1xx_2xx_invite_resp for more information on
		// early media problems.
		// Clear a possible audio session in the open dialog.
		t_audio_session *as = session->get_audio_session();
		if (as) {
			as->set_session(d->session);
			session->set_audio_session(NULL);
			log_file->write_report(
				"An audio session was created on an open dialog.",
				"t_dialog::copy",
				LOG_NORMAL, LOG_DEBUG);
		}
	}
	
	log_file->write_header("t_dialog::copy", LOG_NORMAL, LOG_DEBUG);
	log_file->write_raw("Created dialog through copy, id=");
	log_file->write_raw(d->get_object_id());
	log_file->write_endl();
	log_file->write_footer();

	return d;
}

void t_dialog::send_invite(const t_url &to_uri, const string &to_display,
		const string &subject, const t_hdr_referred_by &hdr_referred_by,
		const t_hdr_replaces &hdr_replaces, 
		const t_hdr_require &hdr_require, 
		const t_hdr_request_disposition &hdr_request_disposition,
		bool anonymous)
{
	t_user *user_config = phone_user->get_user_profile();
	
	if (state != DS_NULL) {
		throw X_DIALOG_ALREADY_ESTABLISHED;
	}

	// If STUN is enabled, then first send a STUN binding request to
	// discover the IP adderss and port for media.
	if (phone->use_stun(user_config)) {
		if (!stun_bind_media()) {
			ui->cb_stun_failed_call_ended(line->get_line_number());
			state = DS_TERMINATED;
			return;
		}
	}
	
	t_request invite(INVITE);
	
	// RFC 3261 12.2.1.1
	// Request URI and Route header
	invite.set_route(to_uri, phone_user->get_service_route());

	// Set Call-ID header
	call_id = NEW_CALL_ID(user_config);
	invite.hdr_call_id.set_call_id(call_id);
	call_id_owner = true;

	// Set To header
	invite.hdr_to.set_uri(to_uri);
	invite.hdr_to.set_display(to_display);

	// Set From header
	local_tag = NEW_TAG;
	local_uri.set_url(line->create_user_uri());
	local_display = user_config->get_display(anonymous);
	invite.hdr_from.set_uri(local_uri);
	invite.hdr_from.set_display(local_display);
	invite.hdr_from.set_tag(local_tag);
	
	// Privacy header
	if (line->get_hide_user()) {
		invite.hdr_privacy.add_privacy(PRIVACY_ID);
	}
	
	// Set P-Preferred-Identity header
	if (anonymous && user_config->get_send_p_preferred_id()) {
		t_identity identity;
		identity.set_uri(user_config->create_user_uri(false));
		identity.set_display(user_config->get_display(false));
		invite.hdr_p_preferred_identity.add_identity(identity);
	}

	// Set CSeq header
	local_seqnr = rand() % 1000 + 1;
	invite.hdr_cseq.set_method(INVITE);
	invite.hdr_cseq.set_seqnr(local_seqnr);

	// Set Max-Forwards header
	invite.hdr_max_forwards.set_max_forwards(MAX_FORWARDS);

	// User-Agent
	SET_HDR_USER_AGENT(invite.hdr_user_agent);

	// RFC 3261 13.2.1
	// Allow and Supported headers
	SET_HDR_ALLOW(invite.hdr_allow, user_config);
	SET_HDR_SUPPORTED(invite.hdr_supported, user_config);

	// Extensions specific for INVITE
	if (user_config->get_ext_100rel() != EXT_DISABLED) {
		invite.hdr_supported.add_feature(EXT_100REL);
	}

	// Require header
	switch (user_config->get_ext_100rel()) {
	case EXT_PREFERRED:
	case EXT_REQUIRED:
		invite.hdr_require.add_feature(EXT_100REL);
		break;
	default:
		break;
	}

	// Subject header
	if (subject != "") {
		invite.hdr_subject.set_subject(subject);
	}

	// Organization
	if (!anonymous) {
		SET_HDR_ORGANIZATION(invite.hdr_organization, user_config);
	}

	// RFC 3892 Referred-By header if a call is initated because
	// of an incoming REFER.
	invite.hdr_referred_by = hdr_referred_by;
	
	// RFC 3891 Replaces header
	invite.hdr_replaces = hdr_replaces;
	
	// Add required extension passed by the upper layer
	if (hdr_require.is_populated()) {
		invite.hdr_require.add_features(hdr_require.features);
	}
	
	// RFC 3841 Request-Disposition header
	invite.hdr_request_disposition = hdr_request_disposition;
	
	// Calculate destinations
	// See create_request() for more comments
	invite.calc_destinations(*user_config);
	
        // The Contatc, Via header and SDP can only be created after the destinations
        // are calculated, because the destination deterimines which
        // local IP address should be used.
        
	// Create SDP offer
	session->create_sdp_offer(&invite, SDP_O_USER);
	
	// Set Via header
	unsigned long local_ip = invite.get_local_ip();
	t_via via(USER_HOST(user_config, h_ip2str(local_ip)), PUBLIC_SIP_PORT(user_config));
	invite.hdr_via.add_via(via);
	
	// Set Contact header
	t_contact_param contact;
	contact.uri.set_url(line->create_user_contact(h_ip2str(local_ip)));
	invite.hdr_contact.add_contact(contact);
	
	// Send INVITE
	req_out_invite = new t_client_request(user_config, &invite, 0);
	MEMMAN_NEW(req_out_invite);
	
	// Trigger call script
	t_call_script script(user_config, t_call_script::TRIGGER_OUT_CALL,
			line->get_line_number() + 1);
	script.exec_notify(&invite);
	
	line->send_request(&invite, req_out_invite->get_tuid());
	line->call_hist_record.start_call(&invite, t_call_record::DIR_OUT, 
		user_config->get_profile_name());

	state = DS_W4INVITE_RESP;
}

bool t_dialog::resend_invite_auth(t_response *resp) {
	t_user *user_config = phone_user->get_user_profile();
	if (!req_out_invite) return false;

	assert(state == DS_W4INVITE_RESP || state == DS_W4INVITE_RESP2);

	t_request *req = req_out_invite->get_request();

	// Add authorization header, increment CSeq and create new branch id
	if (get_phone()->authorize(user_config, req, resp)) {
		resend_request(req_out_invite);

		// Reset state in case a 100 Trying was received
		state = DS_W4INVITE_RESP;

		return true;
	}

	return false;
}

bool t_dialog::resend_invite_unsupported(t_response *resp) {
	t_user *user_config = phone_user->get_user_profile();
	
	if (!req_out_invite) return false;
	if (resp->code != R_420_BAD_EXTENSION) return false;
	if (!resp->hdr_unsupported.is_populated()) return false;
	if (resp->hdr_unsupported.features.empty()) return false;

	t_request *req = req_out_invite->get_request();

	// If no extensions were required then return.
	if (!req->hdr_require.is_populated()) return false;
	if (req->hdr_require.features.empty()) return false;
	
	bool removed_ext = false;

	for (list<string>::iterator i = resp->hdr_unsupported.features.begin();
	     i != resp->hdr_unsupported.features.end(); i++)
	{
		if (req->hdr_require.contains(*i)) {
			if (*i == EXT_100REL) {
				if (user_config->get_ext_100rel() == EXT_PREFERRED) {
					req->hdr_require.del_feature(*i);
				} else {
					// The 100rel is required.
					return false;
				}
			} else {
				// There is no specific requirement for
				// this extension so do not remove it.
				return false;
			}

			removed_ext = true;
		}
	}

	// Return if none of the unsupported extensions was required.
	if (!removed_ext) return false;

	if (req->hdr_require.features.empty()) {
		// There are no required features anymore
		req->hdr_require.unpopulate();
	}

	resend_request(req_out_invite);

	// Reset state in case a 100 Trying was received
	state = DS_W4INVITE_RESP;

	return true;
}

bool t_dialog::redirect_invite(t_response *resp) {
	t_contact_param contact;
	t_user *user_config = phone_user->get_user_profile();

	if (!req_out_invite) return false;

	// If the response is a 3XX response then add redirection contacts
	if (resp->get_class() == R_3XX  && resp->hdr_contact.is_populated()) {
		req_out_invite->redirector.add_contacts(
					resp->hdr_contact.contact_list);
	}

	// Get next destination
	if (!req_out_invite->redirector.get_next_contact(contact)) {
		// There is no next destination
		return false;
	}

	assert(state == DS_W4INVITE_RESP || state == DS_W4INVITE_RESP2);

	t_request *req = req_out_invite->get_request();

	// Ask user for permission to redirect if indicated by user config
	if (user_config->get_ask_user_to_redirect()) {
		if(!ui->cb_ask_user_to_redirect_invite(user_config,
				contact.uri, contact.display)) 
		{
			// User did not permit to redirect
			return false;
		}
	}

	// Change the request URI to the new URI.
	// As the URI changes the destination set must be recalculated
	req->uri = contact.uri;
	req->calc_destinations(*user_config);

	ui->cb_redirecting_request(user_config, line->get_line_number(), contact);
	resend_request(req_out_invite);

	// Reset state in case a 100 Trying was received
	state = DS_W4INVITE_RESP;

	return true;
}

bool t_dialog::failover_invite(void) {
	if (!req_out_invite) return false;
	
	log_file->write_report("Failover to next destination.",
				"t_dialog::failover_invite");
	
	t_request *req = req_out_invite->get_request();
	
	// Get next destination
	if (!req->next_destination()) {
		log_file->write_report("No next destination for failover.",
				"t_dialog::failover_invite");
		return false;
	}
	
	assert(state == DS_W4INVITE_RESP || state == DS_W4INVITE_RESP2);
	resend_request(req_out_invite);

	// Reset state in case a 100 Trying was received
	state = DS_W4INVITE_RESP;

	return true;
}

void t_dialog::send_bye(void) {
	t_user *user_config = phone_user->get_user_profile();
	
	switch (state) {
	case DS_W4INVITE_RESP2:
	case DS_EARLY:
	case DS_CONFIRMED:
		break;
	case DS_W4RE_INVITE_RESP:
	case DS_W4RE_INVITE_RESP2:
		// send BYE after completion of re-INVITE
		request_cancelled = true;
		return;
	case DS_W4ACK:
	case DS_W4ACK_RE_INVITE:
		// send BYE after completion of re-INVITE
		request_cancelled = true;
		return;
	case DS_TERMINATED:
		// Dialog has already been terminated. Do not send BYE.
		return;
	default:
		log_file->write_header("t_dialog::failover_invite",
			LOG_NORMAL, LOG_WARNING);
		log_file->write_raw("Cannot send BYE on dialog in state ");
		log_file->write_raw(state);
		log_file->write_endl();
		log_file->write_footer();
		return;
	}

	// If a previous request is still pending then remove it.
	if (req_out) { MEMMAN_DELETE(req_out); delete (req_out); }

	t_request *bye = create_request(BYE);
	req_out = new t_client_request(user_config, bye, 0);
	MEMMAN_NEW(req_out);
	
	// Trigger call script
	t_call_script script(user_config, t_call_script::TRIGGER_LOCAL_RELEASE,
			line->get_line_number() + 1);
	script.exec_notify(bye);
	
	line->send_request(bye, req_out->get_tuid());
	line->call_hist_record.end_call(false);	
	MEMMAN_DELETE(bye);
	delete bye;

	state = DS_W4BYE_RESP;
	ui->cb_call_ended(line->get_line_number());
}

void t_dialog::send_options(void) {
	t_user *user_config = phone_user->get_user_profile();
	
	// Request can only be sent in a confirmed dialog.
	if (state != DS_CONFIRMED) return;

	// If a previous request is still pending then remove it.
	if (req_out) { MEMMAN_DELETE(req_out); delete (req_out); }

	t_request *r = create_request(OPTIONS);

	// Accept
	r->hdr_accept.add_media(t_media("application","sdp"));
	req_out = new t_client_request(user_config, r, 0);
	MEMMAN_NEW(req_out);
	line->send_request(r, req_out->get_tuid());
	MEMMAN_DELETE(r);
	delete r;
}

void t_dialog::send_cancel(bool early_dialog_exists) {
	t_request *cancel;
	t_user *user_config = phone_user->get_user_profile();

	switch (state) {
	case DS_W4INVITE_RESP:
	case DS_W4RE_INVITE_RESP:
		if (!early_dialog_exists) {
			// wait for first response then send CANCEL or BYE
			request_cancelled = true;
			break;
		}
		// Fall through
	case DS_W4INVITE_RESP2:
	case DS_W4RE_INVITE_RESP2:
	case DS_EARLY:
		if (req_cancel) {
			// CANCEL has been sent already
			break;
		}
		
		cancel = create_request(CANCEL);
		req_cancel = new t_client_request(user_config, cancel, 0);
		MEMMAN_NEW(req_cancel);
		line->send_request(cancel, req_cancel->get_tuid());
		MEMMAN_DELETE(cancel);
		delete cancel;
		
		// Make sure dialog is terminated if CANCEL glares with
		// 2XX on INVITE.
		set_end_after_2xx_invite(true);
		break;
	default:
		break;
	}
	
	ui->cb_call_ended(line->get_line_number());
}

void t_dialog::set_end_after_2xx_invite(bool on) {
	end_after_2xx_invite = on;
}

void t_dialog::send_re_invite(void) {
	assert(session_re_invite);
	t_user *user_config = phone_user->get_user_profile();

	// Request can only be sent in a confirmed dialog.
	if (state != DS_CONFIRMED) return;

	// Do nothing if a re-INVITE is already in progress
	if (req_out_invite) return;

	t_request *r = create_request(INVITE);

	// Set Contact header
	// INVITE must contain a contact header
	t_contact_param contact;
	contact.uri.set_url(line->create_user_contact(h_ip2str(r->get_local_ip())));
	r->hdr_contact.add_contact(contact);

	// RFC 3261 13.2.1
	// Allow and Supported headers
	SET_HDR_ALLOW(r->hdr_allow, user_config);
	SET_HDR_SUPPORTED(r->hdr_supported, user_config);

	// Extensions specific for INVITE
	if (user_config->get_ext_100rel() != EXT_DISABLED) {
		// If some weird far end implementation wants to send
		// a reliable provisional then support it.
		// As a provisional response not needed for a re-INVITE,
		// do not require the 100rel.
		r->hdr_supported.add_feature(EXT_100REL);
	}

	// Create SDP offer
	session_re_invite->create_sdp_offer(r, SDP_O_USER);

	// Send INVITE
	req_out_invite = new t_client_request(user_config, r, 0);
	MEMMAN_NEW(req_out_invite);
	line->send_request(r, req_out_invite->get_tuid());
	MEMMAN_DELETE(r);
	delete r;

	state = DS_W4RE_INVITE_RESP;
}

bool t_dialog::resend_request_auth(t_response *resp) {
	t_client_request **current_cr;

	switch (resp->hdr_cseq.method) {
	case INVITE:
		// re-INVITE
		if (!req_out_invite) return false;
		assert(state == DS_W4RE_INVITE_RESP ||
		       state == DS_W4RE_INVITE_RESP2);
		current_cr = &req_out_invite;
		break;
	case PRACK:
		if (!req_prack) return false;
		current_cr = &req_prack;
		break;
	case REFER:
		if (!req_refer) return false;
		current_cr = &req_refer;
		break;
	case INFO:
		if (!req_info) return false;
		current_cr = &req_info;
		break;
	case SUBSCRIBE:
	case NOTIFY:
		if (!sub_refer) return false;
		if (!sub_refer->req_out) return false;
		current_cr = &(sub_refer->req_out);
		break;
	default:
		// other requests
		if (!req_out) return false;
		current_cr = &req_out;
	}
	
	if (t_abstract_dialog::resend_request_auth(*current_cr, resp)) {
		if (resp->hdr_cseq.method == INVITE) {
			// Reset state in case a 100 Trying was received
			state = DS_W4RE_INVITE_RESP;
		}
		return true;
	}

	return false;
}

bool t_dialog::redirect_request(t_response *resp) {
	t_client_request **current_cr;
	t_user *user_config = phone_user->get_user_profile();
	
	if (resp->hdr_cseq.method == INVITE) {
		// re-INVITE
		if (!req_out_invite) return false;
		assert(state == DS_W4RE_INVITE_RESP ||
		       state == DS_W4RE_INVITE_RESP2);
		current_cr = &req_out_invite;
	} else {
		// non-INVITE
		if (!req_out) return false;
		current_cr = &req_out;
	}
	
	t_contact_param contact;
	if (!t_abstract_dialog::redirect_request(*current_cr, resp, contact)) return false;

	// Re-INVITE
	if (resp->hdr_cseq.method == INVITE) {
		// Reset state in case a 100 Trying was received
		state = DS_W4RE_INVITE_RESP;
	}
	
	ui->cb_redirecting_request(user_config, line->get_line_number(), contact);
	return true;
}

bool t_dialog::failover_request(t_response *resp) {
	t_client_request **current_cr;

	if (resp->hdr_cseq.method == INVITE) {
		// re-INVITE
		if (!req_out_invite) return false;
		assert(state == DS_W4RE_INVITE_RESP ||
		       state == DS_W4RE_INVITE_RESP2);
		current_cr = &req_out_invite;
	} else {
		// non-INVITE
		if (!req_out) return false;
		current_cr = &req_out;
	}
	
	if (!t_abstract_dialog::failover_request(*current_cr)) return false;

	// Re-INVITE
	if (resp->hdr_cseq.method == INVITE) {
		// Reset state in case a 100 Trying was received
		state = DS_W4RE_INVITE_RESP;
	}

	return true;	
}

void t_dialog::hold(bool rtponly) {
	assert(!session_re_invite);

	// Stop glare retry timer
	if (id_glare_retry) {
		line->stop_timer(LTMR_GLARE_RETRY, get_object_id());
	}

	reinvite_purpose = REINVITE_HOLD;

	if (rtponly) {
		session->stop_rtp();
		session->hold();

		// Stopping the RTP only is like a full call hold where
		// the re-INVITE failed. By setting the hold_failed flag,
		// a subsequent retrieve will only start RTP.
		hold_failed = true;
		return;
	}

	hold_failed = false;
	session_re_invite = session->create_call_hold();
	send_re_invite();

	// Stop the audio streams now. If we do not stop the stream now
	// the stream will be stopped when a 200 OK is received on the
	// re-INVITE. However, when the line is put on-hold because
	// the user switches to another line that already has a held call
	// a race condition might occur:
	//
	// 1. A re-INVITE on this line is sent to put it on-hold
	// 2. A re-INVITE on the other line is sent to retrieve the call
	// 3. If the 200 OK on the second re-INVITE comes in before the
	//    the 200 OK on the first re-INVITE, then the audio streams
	//    for the second line will be started already while the first
	//    line still has the audio device open. On some systems this
	//    causes a dead lock as the audio device may only be opened
	//    once.
	//
	// Also if the re-INVITE to put the line on-hold fails, the
	// audio might not be stopped at all. It must be stopped however
	// as the user has switched to the other line. So stopping the
	// audio now will make sure the audio device is idle when the
	// second call is retrieved.
	session->stop_rtp();

	// Prevent RTP stream from getting started even if the signaling 
	// for hold fails. After all the user has put the phone locally
	// on-hold, so RTP should never be started.
	session->hold();
}

void t_dialog::retrieve(void) {
	assert(!session_re_invite);
	t_user *user_config = phone_user->get_user_profile();

	// Stop glare retry timer
	if (id_glare_retry) {
		line->stop_timer(LTMR_GLARE_RETRY, get_object_id());
	}
	
	// Allow RTP stream to be started again.
	session->unhold();

	// If the previous call-hold failed, then only RTP needs to
	// be restarted. The session description did never change
	// because of the failure.
	if (hold_failed) {
		session->start_rtp();
		return;
	}
	
	// If STUN is enabled, then first send a STUN binding request to
	// discover the IP adderss and port for media.
	if (phone->use_stun(user_config)) {
		if (!stun_bind_media()) {	
			// No re-INVITE can be sent. Simply return.
			// User will decide if the call should be
			// torn down.
			return;
		}
	}

	reinvite_purpose = REINVITE_RETRIEVE;
	session_re_invite = session->create_call_retrieve();
	send_re_invite();
}

void t_dialog::kill_rtp(void){
	session->kill_rtp();
	if (session_re_invite) session_re_invite->kill_rtp();
}

void t_dialog::send_refer(const t_url &uri, const string &display) {
	t_user *user_config = phone_user->get_user_profile();
	
	if (state != DS_CONFIRMED) return;

	if (refer_state != REFST_NULL) return;
	
	// If a previous refer is still in progress, then do nothing
	if (req_refer) {
		log_file->write_report("A REFER request is already in progress.",
			"t_dialog::send_refer");
		return;
	}

	// If a refer subscription already exists, then do nothing
	if (sub_refer) {
		log_file->write_report("Refer subscription exists already.",
			"t_dialog::send_refer");
		return;
	}

	t_request *refer = create_request(REFER);

	// Refer-To header
	refer->hdr_refer_to.set_uri(uri);
	refer->hdr_refer_to.set_display(display);

	// Referred-By header
	refer->hdr_referred_by.set_uri(line->create_user_uri());
	refer->hdr_referred_by.set_display(user_config->get_display(line->get_hide_user()));

	req_refer = new t_client_request(user_config, refer, 0);
	MEMMAN_NEW(req_refer);
	line->send_request(refer, req_refer->get_tuid());
	MEMMAN_DELETE(refer);
	delete refer;

	refer_succeeded = false;
	out_refer_req_failed = false;
	refer_state = REFST_W4RESP;
}

void t_dialog::send_dtmf(char digit, bool inband, bool info) {
	t_user *user_config = phone_user->get_user_profile();
	
	if (info) {
		if (req_info) {
			// An INFO request is still in progress, put the
			// DTMF digit in the queue
			dtmf_queue.push(digit);
		} else {
			t_request *info_request = create_request(INFO);
			
			// Content-Type header
			info_request->hdr_content_type.set_media(t_media("application", "dtmf-relay"));
			
			// application/dtmf-relay body
			info_request->body = new t_sip_body_dtmf_relay(digit,
					user_config->get_dtmf_duration());
			MEMMAN_NEW(info_request->body);
			
			req_info = new t_client_request(user_config, info_request, 0);
			MEMMAN_NEW(req_info);
			line->send_request(info_request, req_info->get_tuid());
			MEMMAN_DELETE(info_request);
			delete info_request;
			
			ui->cb_send_dtmf(line->get_line_number(), char2dtmf_ev(digit));
		}
	} else {
		if (session) session->send_dtmf(digit, inband);
	}
}

bool t_dialog::stun_bind_media(void) {
	t_user *user_config = phone_user->get_user_profile();
	
	try {
		unsigned long mapped_ip;
		unsigned short mapped_port;
		int stun_err_code;
		string stun_err_reason;
		bool ret = get_stun_binding(user_config, line->get_rtp_port(),
			mapped_ip, mapped_port,
			stun_err_code, stun_err_reason);
			
		if (!ret) {
			// STUN request failed
			ui->cb_stun_failed(user_config, stun_err_code, stun_err_reason);
			
			log_file->write_header("t_dialog::stun_bind_media", 
				LOG_NORMAL, LOG_CRITICAL);
			log_file->write_raw("STUN bind request for media failed.\n");
			log_file->write_raw(stun_err_code);
			log_file->write_raw(" ");
			log_file->write_raw(stun_err_reason);
			log_file->write_endl();
			log_file->write_footer();
			return false;
		}
		
		// STUN binding request succeeded.
		session->receive_host = h_ip2str(mapped_ip);
		session->receive_port = mapped_port;
	} catch (int err) {
		// STUN request failed
		ui->cb_stun_failed(user_config);
		
		log_file->write_header("t_dialog::stun_bind_media", 
			LOG_NORMAL, LOG_CRITICAL);
		log_file->write_raw("STUN bind request for media failed.\n");
		log_file->write_raw(get_error_str(err));
		log_file->write_endl();
		log_file->write_footer();
		return false;
	}
	
	return true;
}

void t_dialog::recvd_response(t_response *r, t_tuid tuid, t_tid tid) {
	t_user *user_config = phone_user->get_user_profile();
	t_abstract_dialog::recvd_response(r, tuid, tid);

	if (r->hdr_cseq.method == INVITE &&
	    tuid == 0 && tid == 0 && !req_out_invite)
	{
		t_ip_port ip_port;

		// Only a retransmission of a 2XX INVITE is allowed.
		if (r->get_class() != R_2XX) return;
		if (!ack) return;
		if (r->hdr_cseq.seqnr != ack->hdr_cseq.seqnr)
		{
			// The 2XX response does not match the ACK
			return;
		}

		ack->get_destination(ip_port, *user_config);
		if (ip_port.ipaddr != 0 && ip_port.port != 0) {
			evq_sender->push_network(ack, ip_port);
		}

		return;
	}

	if (r->hdr_cseq.method == CANCEL) {
		if (!req_cancel) return;
		if (r->is_final()) {
			remove_client_request(&req_cancel);
			if (r->is_success()) {
				line->start_timer(LTMR_CANCEL_GUARD, get_object_id());
			} else {
				// CANCEL request failed.
				ui->cb_cancel_failed(line->get_line_number(), r);

				// Abort the INVITE as the user cannot terminate
				// it in a normal way.
				if (req_out_invite) {
					t_tid _tid = req_out_invite->get_tid();
					if (_tid > 0) {
						evq_trans_mgr->push_abort_trans(_tid);
					}
				}
			}
		}
		return;
	}

	// No processing done for PRACK responses.
	if (r->hdr_cseq.method == PRACK) {
		if (!req_prack) return;
		t_request *prack = req_prack->get_request();

		if (r->hdr_cseq.seqnr != prack->hdr_cseq.seqnr) {
			// The response does not match the latest sent PRACK.
			// It might match a previous sent PRACK. However, when
			// a previous PRACK fails, then the latest PRACK will also
			// fail, so the failure will be handled in the end without
			// the overhead to keep a list of all pending PRACKs which
			// should be a rare case.
			return;
		}

		if (r->is_final()) {
			// PRACK is finished, so remove request
			remove_client_request(&req_prack);

			// Tear down the call if PRACK failed and call is
			// not yet established.
			if (!r->is_success() && state == DS_EARLY) {
				log_file->write_header("t_dialog::recvd_response",
					LOG_NORMAL, LOG_WARNING);
				log_file->write_raw("PRACK failed: ");
				log_file->write_raw(r->code);
				log_file->write_raw(" ");
				log_file->write_raw(r->reason);
				log_file->write_endl();
				log_file->write_raw("Call will be cancelled.\n");
				log_file->write_footer();

				ui->cb_prack_failed(line->get_line_number(), r);
				send_cancel(true);

				// Ignore the failure in other states.
				// The call has been setup, so all seems fine.
			}
		}

		return;
	}

	// Determine if this is an INVITE or non-INVITE response
	t_client_request *req;
	bool send_to_sub_refer = false;

	switch(r->hdr_cseq.method) {
	case INVITE:
		req = req_out_invite;
		break;
	case SUBSCRIBE:
	case NOTIFY:
		if (!sub_refer) return;
		req = sub_refer->req_out;
		send_to_sub_refer = true;
		break;
	case REFER:
		req = req_refer;
		break;
	case INFO:
		req = req_info;
		break;
	default:
		req = req_out;
	}

	// Discard response if no request is pending
	if (!req) {
		return;
	}

	// Check cseq
	if (r->hdr_cseq.method != req->get_request()->method) {
		return;
	}
	if (r->hdr_cseq.seqnr != req->get_request()->hdr_cseq.seqnr) return;

	// Set the transaction identifier. This identifier is needed if the
	// transaction must be aborted at a later time.
	req->set_tid(tid);

	if (send_to_sub_refer) {
		sub_refer->recv_response(r, tuid, tid);
		if (sub_refer->get_state() == SS_TERMINATED) {
			MEMMAN_DELETE(sub_refer);
			delete sub_refer;
			sub_refer = NULL;
			if (state == DS_CONFIRMED_SUB) {
				state = DS_TERMINATED;
			}
		}
		return;
	}

	switch (state) {
	case DS_W4INVITE_RESP:
	case DS_W4INVITE_RESP2:
		state_w4invite_resp(r, tuid, tid);
		break;
	case DS_EARLY:
		state_early(r, tuid, tid);
		break;
	case DS_W4BYE_RESP:
		state_w4bye_resp(r, tuid, tid);
		break;
	case DS_CONFIRMED:
		state_confirmed_resp(r, tuid, tid);
		break;
	case DS_W4RE_INVITE_RESP:
	case DS_W4RE_INVITE_RESP2:
		state_w4re_invite_resp(r, tuid, tid);
		break;
	default:
		// No response expected in other states. Discard.
		break;
	}
}

void t_dialog::recvd_request(t_request *r, t_tuid tuid, t_tid tid) {
	t_response *resp;
	t_user *user_config = phone_user->get_user_profile();

	// CANCEL will be handled by recvd_cancel()
	
	t_abstract_dialog::recvd_request(r, tuid, tid);

	switch (r->method) {
	case ACK:
		// When ACK is received then the current incoming request
		// must be INVITE.
		if (!req_in_invite) return;
		if (req_in_invite->get_request()->hdr_cseq.seqnr !=
				r->hdr_cseq.seqnr)
		{
			log_file->write_header("t_dialog::recvd_request",
					LOG_NORMAL, LOG_WARNING);
			log_file->write_raw("ACK does not match a pending INVITE.\n");
			log_file->write_raw("Discard ACK.\n");
			log_file->write_footer();
			return;
		}
		break;
	case INVITE:
		if (remote_seqnr_set && r->hdr_cseq.seqnr <= remote_seqnr) {
			// Request received out of sequence. Discard.
			log_file->write_header("t_dialog::recvd_request",
					LOG_NORMAL, LOG_WARNING);
			log_file->write_raw("INVITE is received out of order.\n");
			log_file->write_raw("Remote seqnr = ");
			log_file->write_raw(remote_seqnr);
			log_file->write_endl();
			log_file->write_raw("Received seqnr = ");
			log_file->write_raw(r->hdr_cseq.seqnr);
			log_file->write_endl();
			log_file->write_raw("Discard INVITE.\n");
			log_file->write_footer();
			return;
		}
		
		remote_seqnr = r->hdr_cseq.seqnr;
		remote_seqnr_set = true;

		if (req_in_invite) {
			// RFC 3261 14.2
			// Another INVITE is received while the previous
			// one is not finished.
			resp = r->create_response(R_500_INTERNAL_SERVER_ERROR,
				"Previous INVITE still in progress");
			line->send_response(resp, tuid, tid);
			MEMMAN_DELETE(resp);
			delete resp;
			return;
		} else if (req_out_invite) {
			// RFC 3261 14.2
			// re-INVITE glare
			resp = r->create_response(R_491_REQUEST_PENDING);
			line->send_response(resp, tuid, tid);
			MEMMAN_DELETE(resp);
			delete resp;
			return;
		} else {
			req_in_invite = new t_client_request(user_config, r, tid);
			MEMMAN_NEW(req_in_invite);
		}
		break;
	case REFER:
		// Reset refer_accepted indication.
		refer_accepted = false;
		// fall thru
	default:
		// Check cseq
		// RFC 3261 12.2.2
		if (remote_seqnr_set && r->hdr_cseq.seqnr <= remote_seqnr) {
			// Request received out of order.		
			log_file->write_header("t_dialog::recvd_request",
				LOG_NORMAL, LOG_WARNING);
			log_file->write_raw("CSeq seqnr is out of sequence.\n");
			log_file->write_raw("Reveived seqnr: ");
			log_file->write_raw(r->hdr_cseq.seqnr);
			log_file->write_endl();
			log_file->write_raw("Remote seqnr: ");
			log_file->write_raw(remote_seqnr);
			log_file->write_endl();
			log_file->write_footer();
			
			resp = r->create_response(R_500_INTERNAL_SERVER_ERROR,
				"Request received out of order");
			line->send_response(resp, tuid, tid);
			MEMMAN_DELETE(resp);
			delete resp;

			return;
		}
		
		remote_seqnr = r->hdr_cseq.seqnr;
		remote_seqnr_set = true;
	}

	t_dialog_state old_state = state;
	
	switch (state) {
	case DS_NULL:
		state_null(r, tuid, tid);
		break;
	case DS_W4ACK:
		state_w4ack(r, tuid, tid);
		break;
	case DS_W4ACK_RE_INVITE:
		state_w4ack_re_invite(r, tuid, tid);
		break;
	case DS_W4ANSWER:
		state_w4answer(r, tuid, tid);
		break;
	case DS_W4RE_INVITE_RESP:
	case DS_W4RE_INVITE_RESP2:
		state_w4re_invite_resp(r, tuid, tid);
		break;
	case DS_W4BYE_RESP:
		state_w4bye_resp(r, tuid, tid);
		break;
	case DS_CONFIRMED:
		state_confirmed(r, tuid, tid);
		break;
	case DS_CONFIRMED_SUB:
		state_confirmed_sub(r, tuid, tid);
		break;
	default:
		// No request expected in other states. Discard.
		resp = r->create_response(R_500_INTERNAL_SERVER_ERROR);
		line->send_response(resp, tuid, tid);
		MEMMAN_DELETE(resp);
		delete resp;
		break;
	}
	
	// If the state has changed, then waiting requests needs to be
	// processed.
	if (state != old_state && !inc_req_queue.empty()) {
		t_client_request *queued_cr = inc_req_queue.front();
		inc_req_queue.pop_front();
		
		log_file->write_header("t_dialog::recvd_request", 
				LOG_NORMAL, LOG_INFO);
		log_file->write_raw("Process queued ");
		log_file->write_raw(method2str(r->method, r->unknown_method));
		log_file->write_endl();
		log_file->write_footer();

		recvd_request(queued_cr->get_request(), 0, queued_cr->get_tid());
		MEMMAN_DELETE(queued_cr);
		delete queued_cr;
	}
}

// RFC 3261 9.2
void t_dialog::recvd_cancel(t_request *r, t_tid cancel_tid,
		t_tid target_tid)
{
	t_response *resp;

	assert(r->method == CANCEL);

	// Send 200 as response to CANCEL
	resp = r->create_response(R_200_OK);
	// RFC 3261 9.2
	// The To-tag in the response to the CANCEL should be the same
	// as the To-tag in the original request.
	resp->hdr_to.set_tag(local_tag);
	line->send_response(resp, 0, cancel_tid);
	MEMMAN_DELETE(resp);
	delete resp;

	switch (state) {
	case DS_W4ANSWER:
		state_w4answer(r, 0, cancel_tid);
		break;
	default:
		// Ignore CANCEL in other states.
		break;
	}
}

void t_dialog::recvd_stun_resp(StunMessage *r, t_tuid tuid, t_tid tid) {
	// Not used anymore.
	// STUN requests are performed in a synchronous way.
}

// RFC 3261 13.3.1.4
void t_dialog::answer(void) {
	t_user *user_config = phone_user->get_user_profile();
	if (!req_in_invite) return;

	t_request *invite_req = req_in_invite->get_request();

	// RFC 3262 3
	// Delay the final response if we are still waiting for a PRACK
	// on a 1xx response containing SDP
	if (resp_1xx_invite && resp_1xx_invite->body) {
		answer_after_prack = true;
		return;
	}

	if (state != DS_W4ANSWER) {
		throw X_WRONG_STATE;
	}
	
	resp_invite = invite_req->create_response(R_200_OK);
	resp_invite->hdr_to.set_tag(local_tag);

	// Set Organization header
	SET_HDR_ORGANIZATION(resp_invite->hdr_organization, user_config);

	// RFC 3261 12.1.1
	// Copy the Record-Route header from request to response
	if (invite_req->hdr_record_route.is_populated()) {
		resp_invite->hdr_record_route = invite_req->hdr_record_route;
	}

	// Set Contact header
	t_contact_param contact;
	contact.uri.set_url(line->create_user_contact(h_ip2str(resp_invite->get_local_ip())));
	resp_invite->hdr_contact.add_contact(contact);

	// Set Allow and Supported headers
	SET_HDR_ALLOW(resp_invite->hdr_allow, user_config);
	SET_HDR_SUPPORTED(resp_invite->hdr_supported, user_config);

	// RFC 3261 13.3.1.4
	// Create SDP offer if no offer was received in INVITE and no offer
	// was sent in a reliable 1xx response (RFC 3262 5)
	// Otherwise if no offer was sent in a reliable 1xx, create an SDP answer.
	if (!session->sent_offer) {
		if (!session->recvd_offer && !session->sent_offer) {
			session->create_sdp_offer(resp_invite, SDP_O_USER);
		} else {
			session->create_sdp_answer(resp_invite, SDP_O_USER);
			session->start_rtp();
		}
	}
	
	// Trigger call script
	t_call_script script(user_config, t_call_script::TRIGGER_IN_CALL_ANSWERED,
			line->get_line_number() + 1);
	script.exec_notify(resp_invite);

	line->call_hist_record.answer_call(resp_invite);
	line->send_response(resp_invite, req_in_invite->get_tuid(),
					req_in_invite->get_tid());
	line->start_timer(LTMR_ACK_GUARD, get_object_id());
	line->start_timer(LTMR_ACK_TIMEOUT, get_object_id());

	// Stop 100rel timers if they are running.
	line->stop_timer(LTMR_100REL_GUARD, get_object_id());
	line->stop_timer(LTMR_100REL_TIMEOUT, get_object_id());

	state = DS_W4ACK;
}

void t_dialog::reject(int code, string reason) {
	t_response *resp;
	t_user *user_config = phone_user->get_user_profile();

	if (state != DS_W4ANSWER) {
		throw X_WRONG_STATE;
	}

	assert(req_in_invite);
	assert(code >= 400);

	resp = req_in_invite->get_request()->create_response(code, reason);
	resp->hdr_to.set_tag(local_tag);
	
	// Trigger call script
	t_call_script script(user_config, t_call_script::TRIGGER_IN_CALL_FAILED,
			line->get_line_number() + 1);
	script.exec_notify(resp);
		
	line->send_response(resp, req_in_invite->get_tuid(),
						req_in_invite->get_tid());
	line->call_hist_record.fail_call(resp);
	MEMMAN_DELETE(resp);
	delete resp;

	// Stop 100rel timers if they are running.
	line->stop_timer(LTMR_100REL_GUARD, get_object_id());
	line->stop_timer(LTMR_100REL_TIMEOUT, get_object_id());

	state = DS_TERMINATED;
}

void t_dialog::redirect(const list<t_display_url> &destinations, int code, string reason)
{
	t_response *resp;
	t_user *user_config = phone_user->get_user_profile();

	if (state != DS_W4ANSWER) {
		throw X_WRONG_STATE;
	}

	assert(req_in_invite);
	assert(code >= 300 && code <= 399);

	resp = req_in_invite->get_request()->create_response(code, reason);
	resp->hdr_to.set_tag(local_tag);

	t_contact_param *contact;
	float q = 0.9;
	for (list<t_display_url>::const_iterator i = destinations.begin();
	     i != destinations.end(); i++)
	{
		contact = new t_contact_param();
		MEMMAN_NEW(contact);
		contact->display = i->display;
		contact->uri = i->url;
		contact->set_qvalue(q);
		resp->hdr_contact.add_contact(*contact);
		MEMMAN_DELETE(contact);
		delete contact;
		q = q - 0.1;
		if (q < 0.1) q = 0.1;
	}
	
	// Trigger call script
	t_call_script script(user_config, t_call_script::TRIGGER_IN_CALL_FAILED,
			line->get_line_number() + 1);
	script.exec_notify(resp);

	line->send_response(resp, req_in_invite->get_tuid(),
						req_in_invite->get_tid());
	line->call_hist_record.fail_call(resp);
	MEMMAN_DELETE(resp);
	delete resp;

	// Stop 100rel timers if they are running.
	line->stop_timer(LTMR_100REL_GUARD, get_object_id());
	line->stop_timer(LTMR_100REL_TIMEOUT, get_object_id());

	state = DS_TERMINATED;
}

bool t_dialog::match_response(t_response *r, t_tuid tuid) {
	if (tuid != 0) {
  		if (req_out && req_out->get_tuid() == tuid) return true;
		if (req_out_invite && req_out_invite->get_tuid() == tuid) {
			return true;
		}
		if (req_cancel && req_cancel->get_tuid() == tuid) {
			return true;
		}
		return false;
	}
	
	// The implementation sends CANCEL on the open dialog.
	// The tags of a CANCEL response will be identical to the tags of
	// the INVITE, so it matches all pending dialogs as well.
	// So a CANCEL should only match if the dialog has a CANCEL request
	// pending.
	if (r->hdr_cseq.method == CANCEL && !req_cancel) return false;

	return t_abstract_dialog::match_response(r, tuid);
}

bool t_dialog::match_response(StunMessage *r, t_tuid tuid) {
	if (tuid == 0) return false;
	if (!req_stun) return false;

	return (req_stun->get_tuid() == tuid);
}

bool t_dialog::match_cancel(t_request *r, t_tid target_tid) {
	return (req_in_invite && req_in_invite->get_tid() == target_tid);
}

bool t_dialog::is_invite_retrans(t_request *r) {
	assert(r->method == INVITE);

	// An INVITE can only be a retransmission if an incoming INVITE is
	// still in progress.
	if (!req_in_invite) return false;
	t_request *request = req_in_invite->get_request();

	// RFC 3261 17.2.3
	t_via &orig_top_via = request->hdr_via.via_list.front();
	t_via &recv_top_via = r->hdr_via.via_list.front();

	if (recv_top_via.rfc3261_compliant()) {
		if (orig_top_via.branch != recv_top_via.branch) return false;
		if (orig_top_via.host != recv_top_via.host) return false;
		if (orig_top_via.port != recv_top_via.port) return false;
		return (request->hdr_cseq.method == r->hdr_cseq.method);
	}

	// Matching rules for backward compatibiliy with RFC 2543
	// TODO: verify rules for matching via headers
	return (request->uri.sip_match(r->uri) &&
		request->hdr_to.tag == r->hdr_to.tag &&
		request->hdr_from.tag == r->hdr_from.tag &&
		request->hdr_call_id.call_id == r->hdr_call_id.call_id &&
		request->hdr_cseq.seqnr == r->hdr_cseq.seqnr &&
		orig_top_via.host == recv_top_via.host &&
		orig_top_via.port == recv_top_via.port);
}

void t_dialog::process_invite_retrans(void) {
	t_ip_port ip_port;
	
	// Retransmit 2xx response.
	// Send the response directly to the sender thread
	// as the INVITE transaction completed already.
	// (see RFC 3261 17.2.1)
	if (!resp_invite) return; // there is no response to send
	resp_invite->get_destination(ip_port);
	if (ip_port.ipaddr == 0) {
		// This should not happen. The response has been
		// sent before so it should be possible to sent
		// it again. Ignore the timeout. When the ACK
		// guard timer expires, the dialog will be
		// cleaned up.
		return;
	}
	evq_sender->push_network(resp_invite, ip_port);
}

t_dialog_state t_dialog::get_state(void) const {
	return state;
}

void t_dialog::timeout(t_line_timer timer) {
	switch(state) {
	case DS_W4INVITE_RESP:
	case DS_W4INVITE_RESP2:
		state_w4invite_resp(timer);
		break;
	case DS_EARLY:
		state_early(timer);
		break;
	case DS_W4ACK:
		state_w4ack(timer);
		break;
	case DS_W4ACK_RE_INVITE:
		state_w4ack_re_invite(timer);
		break;
	case DS_W4RE_INVITE_RESP2:
		state_w4re_invite_resp(timer);
		break;
	case DS_W4ANSWER:
		state_w4answer(timer);
		break;
	case DS_CONFIRMED:
		state_confirmed(timer);
		break;
	default:
		// Timeout not expected in other states. Ignore.
		break;
	}
}

void t_dialog::timeout_sub(t_subscribe_timer timer, const string &event_type,
		const string &event_id)
{
	if (sub_refer &&
	    sub_refer->get_event_type() == event_type &&
	    sub_refer->get_event_id() == event_id)
	{
		sub_refer->timeout(timer);
	} else {
		// Timeout does not match with the current subscription.
		// Ignore.
		return;
	}

	if (sub_refer->get_state() == SS_TERMINATED && state == DS_CONFIRMED_SUB) {
		MEMMAN_DELETE(sub_refer);
		delete sub_refer;
		sub_refer = NULL;
		state = DS_TERMINATED;
	}
}

t_phone *t_dialog::get_phone(void) const {
	return line->get_phone();
}

t_line *t_dialog::get_line(void) const {
	return line;
}

t_session *t_dialog::get_session(void) const {
	return session;
}

t_audio_session *t_dialog::get_audio_session(void) const {
	if (!session) return NULL;

	return session->get_audio_session();
}

bool t_dialog::has_active_session(void) const {
	if (session) return session->is_rtp_active();
	
	return false;
}

// RFC 3515
// Send a NOTIFY with reference progress to the referror
void t_dialog::notify_refer_progress(t_response *r) {
	if (!sub_refer) return;

	if (r->is_final()) {
		sub_refer->send_notify(r, SUBSTATE_TERMINATED, EV_REASON_NORESOURCE);
	} else {
		sub_refer->send_notify(r, SUBSTATE_ACTIVE);
	}
}

bool t_dialog::will_release(void) const {
	return state == DS_W4BYE_RESP || request_cancelled || 
		end_after_2xx_invite || end_after_ack;
}
