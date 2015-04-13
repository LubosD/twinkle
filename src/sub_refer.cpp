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

#include "sub_refer.h"
#include "line.h"
#include "log.h"
#include "phone_user.h"
#include "user.h"
#include "userintf.h"
#include "audits/memman.h"

t_dialog *t_sub_refer::get_dialog(void) const {
	return dynamic_cast<t_dialog *>(dialog);
}

t_sub_refer::t_sub_refer(t_dialog *_dialog, t_subscription_role _role) :
		t_subscription(_dialog, _role, SIP_EVENT_REFER)
{
	// A refer subscription is implicitly defined by the REFER
	// transaction
	state = SS_ESTABLISHED;

	// Start the subscription timer only for a notifier.
	// The subscriber will start a timer when it receives NOTIFY.
	if (role == SR_NOTIFIER) {
		unsigned long dur;
		if (user_config->get_ask_user_to_refer()) {
			dur = DUR_REFER_SUB_INTERACT * 1000;
		} else {
			dur = DUR_REFER_SUBSCRIPTION * 1000;
		}
		start_timer(STMR_SUBSCRIPTION, dur);
	}

	auto_refresh = user_config->get_auto_refresh_refer_sub();
	subscription_expiry = DUR_REFER_SUBSCRIPTION;
	sr_result = SRR_INPROG;
	
	last_response = NULL;

	log_file->write_header("t_sub_refer::t_sub_refer");
	log_file->write_raw("Refer ");
	if (role == SR_SUBSCRIBER) {
		log_file->write_raw("subscriber");
	} else {
		log_file->write_raw("notifier");
	}
	log_file->write_raw(" created: event = ");
	log_file->write_raw(event_type);
	log_file->write_endl();
	log_file->write_footer();
}

t_sub_refer::t_sub_refer(t_dialog *_dialog, t_subscription_role _role,
		const string &_event_id) :
			t_subscription(_dialog, _role, SIP_EVENT_REFER, _event_id)
{
	state = SS_ESTABLISHED;

	if (role == SR_NOTIFIER) {
		unsigned long dur;
		if (user_config->get_ask_user_to_refer()) {
			dur = DUR_REFER_SUB_INTERACT * 1000;
		} else {
			dur = DUR_REFER_SUBSCRIPTION * 1000;
		}
		start_timer(STMR_SUBSCRIPTION, dur);
	}

	auto_refresh = user_config->get_auto_refresh_refer_sub();
	subscription_expiry = DUR_REFER_SUBSCRIPTION;
	sr_result = SRR_INPROG;

	last_response = NULL;

	log_file->write_header("t_sub_refer::t_sub_refer");
	log_file->write_raw("Refer ");
	if (role == SR_SUBSCRIBER) {
		log_file->write_raw("subscriber");
	} else {
		log_file->write_raw("notifier");
	}
	log_file->write_raw(" created: event = ");
	log_file->write_raw(event_type);
	log_file->write_raw(";id=");
	log_file->write_raw(event_id);
	log_file->write_endl();
	log_file->write_footer();
}

t_sub_refer::~t_sub_refer() {
	if (last_response) {
		MEMMAN_DELETE(last_response);
		delete last_response;
	}

	log_file->write_header("t_sub_refer::~t_sub_refer");
	log_file->write_raw("Refer ");
	if (role == SR_SUBSCRIBER) {
		log_file->write_raw("subscriber");
	} else {
		log_file->write_raw("notifier");
	}
	log_file->write_raw(" destroyed: event = ");
	log_file->write_raw(event_type);
	if (!event_id.empty()) {
		log_file->write_raw(";id=");
		log_file->write_raw(event_id);
	}
	log_file->write_endl();
	log_file->write_footer();
}

void t_sub_refer::send_notify(t_response *r, const string &substate,
		const string reason)
{
	t_request *notify;
	
	if (substate == SUBSTATE_TERMINATED) {
		// RFC 3515 2.4.7
		notify = create_notify(substate, reason);
		stop_timer(STMR_SUBSCRIPTION);
	} else {
		notify = create_notify(substate);
	}

	// RFC 3515 2.4.4
	// Create message/sipfrag body containing only the status line
	// of the response.
	t_response sipfrag(r->code, r->reason);
	notify->body = new t_sip_body_sipfrag(&sipfrag);
	MEMMAN_NEW(notify->body);
	notify->hdr_content_type.set_media(t_media("message", "sipfrag"));

	// If an outgoing NOTIFY is still pending, then store this
	// NOTIFY in the queue
	if (req_out) {
		queue_notify.push(notify);
	} else {
		// Send NOTIFY
		req_out = new t_client_request(user_config, notify,0);
		MEMMAN_NEW(req_out);
		send_request(user_config, notify, req_out->get_tuid());
		MEMMAN_DELETE(notify);
		delete notify;
	}

	// Keep response and state such that it can be resend when
	// a SUBSCRIBE is received.
	if (last_response && last_response != r) {
		MEMMAN_DELETE(last_response);
		delete last_response;
		last_response = NULL;
	}
	
	if (!last_response) last_response = (t_response *)r->copy();
	current_substate = substate;
}

bool t_sub_refer::recv_notify(t_request *r, t_tuid tuid, t_tid tid) {
	if (t_subscription::recv_notify(r, tuid, tid)) return true;
	
	// RFC 3515 2.4.5.
	// NOTIFY must have a sipfrag body
	if (!r->body || r->body->get_type() != BODY_SIPFRAG) {
		t_response *resp = r->create_response(R_400_BAD_REQUEST,
				"message/sipfrag body missing");
		send_response(user_config, resp, 0, tid);
		MEMMAN_DELETE(resp);
		delete resp;
		return true;
	}

	// RFC 3515 2.4.5
	// The sipfrag body must start with a Status-Line
	if (((t_sip_body_sipfrag *)r->body)->sipfrag->get_type() != MSG_RESPONSE) {
		t_response *resp = r->create_response(R_400_BAD_REQUEST,
				"sipfrag body does not begin with Status-Line");
		send_response(user_config, resp, 0, tid);
		MEMMAN_DELETE(resp);
		delete resp;
		return true;
	}

	t_response *sipfrag = (t_response *)((t_sip_body_sipfrag *)r->body)->sipfrag;

	// Determine state of reference
	if (r->hdr_subscription_state.substate == SUBSTATE_TERMINATED) {
		if (r->hdr_subscription_state.reason == EV_REASON_REJECTED) {
			// Referee rejected to refer
			sr_result = SRR_FAILED;
		} else if (r->hdr_subscription_state.reason == EV_REASON_NORESOURCE) {
			// Reference is finished. The sipfrag body indicates
			// success or failure.
			if (sipfrag->is_success()) {
				sr_result = SRR_SUCCEEDED;
			} else {
				sr_result = SRR_FAILED;
			}
		}
	}


	// Inform user about progress
	ui->cb_notify_recvd(get_dialog()->get_line()->get_line_number(), r);

	t_response *resp = r->create_response(R_200_OK);
	send_response(user_config, resp, 0, tid);
	MEMMAN_DELETE(resp);
	delete resp;

	return true;
}

bool t_sub_refer::recv_subscribe(t_request *r, t_tuid tuid, t_tid tid) {
	unsigned long expires;

	if (t_subscription::recv_subscribe(r, tuid, tid)) return true;
	
	// Determine value for Expires header
	if (!r->hdr_expires.is_populated() ||
	    r->hdr_expires.time > 2 * DUR_REFER_SUBSCRIPTION)
	{
		// User did not indicate an expiry time for subscription
		// refresh or a time larger then 2 times the default.
		// Just use the Twinkle default.
		stop_timer(STMR_SUBSCRIPTION);
		start_timer(STMR_SUBSCRIPTION, DUR_REFER_SUBSCRIPTION * 1000);
		expires = DUR_REFER_SUBSCRIPTION;
	} else {
		expires = r->hdr_expires.time;
	}

	t_response *resp = r->create_response(R_200_OK);

	// RFC 3265 7.1
	// Contact header is mandatory
	t_contact_param contact;
	contact.uri.set_url(get_dialog()->get_line()->create_user_contact(
			h_ip2str(resp->get_local_ip())));
	resp->hdr_contact.add_contact(contact);

	// Expires header is mandatory
	resp->hdr_expires.set_time(expires);

	send_response(user_config, resp, 0, tid);
	MEMMAN_DELETE(resp);
	delete resp;

	// RFC 3265 3.2.2
	// After a succesful SUBSCRIBE the notifier must immediately
	// send a NOTIFY.
	// If no last response has been kept then this is probably a
	// first SUBSCRIBE. The dialog has to send the initial NOTIFY.
	if (last_response) {
		if (expires == 0) {
			send_notify(last_response, SUBSTATE_TERMINATED,
				EV_REASON_TIMEOUT);
		} else {
			send_notify(last_response, current_substate);
		}
	}

	return true;
}

bool t_sub_refer::timeout(t_subscribe_timer timer) {
	if (t_subscription::timeout(timer)) return true;

	switch (timer) {
	case STMR_SUBSCRIPTION:
		switch (role) {
		case SR_NOTIFIER:
			// RFC 3265 3.1.6.4
			// The subscription has expired
			// RFC 2.4.5
			// Each NOTIFY MUST contain a body
			if (last_response) {
				// Repeat last response as body
				send_notify(last_response, SUBSTATE_TERMINATED,
					EV_REASON_TIMEOUT);
			} else {
				// This should never happen. Create a timeout
				// response for the body.
				t_response resp(R_408_REQUEST_TIMEOUT);
				send_notify(&resp, SUBSTATE_TERMINATED,
					EV_REASON_TIMEOUT);
			}

			log_file->write_report("Refer notifier timed out.",
				"t_sub_refer::timeout");

			return true;
		case SR_SUBSCRIBER:
			// Should have been handled by parent class
		default:
			assert(false);
		}
		break;
	default:
		assert(false);
	}

	return false;
}

t_sub_refer_result t_sub_refer::get_sr_result(void) const {
	return sr_result;
}
