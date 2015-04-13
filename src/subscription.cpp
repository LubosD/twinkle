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

#include "subscription.h"

#include "dialog.h"
#include "line.h"
#include "log.h"
#include "phone_user.h"
#include "audits/memman.h"
#include "parser/hdr_event.h"

extern t_event_queue	*evq_trans_mgr;
extern t_event_queue	*evq_timekeeper;
extern t_timekeeper	*timekeeper;

string t_subscription_state2str(t_subscription_state state) {
	switch (state) {
	case SS_NULL:		return "SS_NULL";
	case SS_ESTABLISHED:	return "SS_ESTABLISHED";
	case SS_UNSUBSCRIBING:	return "SS_UNSUBSCRIBING";
	case SS_UNSUBSCRIBED:	return "SS_UNSUBSCRIBED";
	case SS_TERMINATED:	return "SS_TERMINATED";
	}
	
	return "UNKNOWN";
}

/////////////
// PROTECTED
/////////////

void t_subscription::log_event() const {
	log_file->write_raw("Event:    ");
	log_file->write_raw(event_type);
	log_file->write_endl();
	log_file->write_raw("Event id: ");
	log_file->write_raw(event_id);
	log_file->write_endl();
}

void t_subscription::remove_client_request(t_client_request **cr) {
	if ((*cr)->dec_ref_count() == 0) {
		MEMMAN_DELETE(*cr);
		delete *cr;
	}

	*cr = NULL;
}

t_request *t_subscription::create_subscribe(unsigned long expires) const {
	// RFC 3265 3.1.4
	t_request *r = dialog->create_request(SUBSCRIBE);
	r->hdr_expires.set_time(expires);
	r->hdr_event.set_event_type(event_type);
	if (event_id.size() > 0) r->hdr_event.set_id(event_id);
	
	// Re-calculate the destination as the event type may
	// influence the route to be taken.
	// The destination has been calculated already at the
	// dialog level.
	r->calc_destinations(*user_config);

	return r;
}

t_request *t_subscription::create_notify(const string &sub_state,
		const string &reason) const
{
	// RFC 3265 3.2.2
	t_request *r = dialog->create_request(NOTIFY);
	r->hdr_event.set_event_type(event_type);
	if (event_id.size() > 0) r->hdr_event.set_id(event_id);
	r->hdr_subscription_state.set_substate(sub_state);

	// Subscription state specific parameters
	if (sub_state == SUBSTATE_ACTIVE || sub_state == SUBSTATE_PENDING) {
		// Add expires parameter with remaining time
		if (id_subscription_timeout) {
			long remaining = timekeeper->
				get_remaining_time(id_subscription_timeout);
			r->hdr_subscription_state.set_expires(remaining / 1000);
		}
	} else if (sub_state == SUBSTATE_TERMINATED) {
		// Add reason parameter
		if (reason.size() > 0) {
			r->hdr_subscription_state.set_reason(reason);
		}
	}

	return r;
}

void t_subscription::send_request(t_user *user_config, t_request *r, t_tuid tuid) const {
	evq_trans_mgr->push_user(user_config, (t_sip_message *)r, tuid, 0);
}

void t_subscription::send_response(t_user *user_config, t_response *r, 
		t_tuid tuid, t_tid tid) const 
{
	evq_trans_mgr->push_user(user_config, (t_sip_message *)r, tuid, tid);
}

void t_subscription::start_timer(t_subscribe_timer timer, long duration) {
	t_tmr_subscribe		*t;
	t_object_id		oid_line = 0;

	switch(timer) {
	case STMR_SUBSCRIPTION:
		if (dynamic_cast<t_dialog *>(dialog) != NULL) {
			oid_line = dynamic_cast<t_dialog *>(dialog)->get_line()->get_object_id();
		}
		t = new t_tmr_subscribe(duration, timer, oid_line,
				dialog->get_object_id(), event_type, event_id);
		MEMMAN_NEW(t);
		id_subscription_timeout = t->get_object_id();
		break;
	default:
		assert(false);
	}

	evq_timekeeper->push_start_timer(t);
	MEMMAN_DELETE(t);
	delete t;
}

void t_subscription::stop_timer(t_subscribe_timer timer) {
	unsigned short	*id;

	switch(timer) {
	case STMR_SUBSCRIPTION:
		id = &id_subscription_timeout;
		break;
	default:
		assert(false);
	}

	if (*id != 0) evq_timekeeper->push_stop_timer(*id);
	*id = 0;
}

//////////
// PUBLIC
//////////

t_subscription::t_subscription(t_abstract_dialog *_dialog, t_subscription_role _role,
			const string &_event_type) 
{
	dialog = _dialog;
	
	user_config = dialog->get_user();
	assert(user_config);
	
	role = _role;
	state = SS_NULL;
	resubscribe_after = 0;
	may_resubscribe = false;
	pending = true;
	id_subscription_timeout = 0;
	req_out = NULL;
	event_type = _event_type;
	auto_refresh = true;
	subscription_expiry = 3600;
	default_duration = 3600;
}

t_subscription::t_subscription(t_abstract_dialog *_dialog, t_subscription_role _role,
			const string &_event_type, const string &_event_id)
{
	dialog = _dialog;
	
	user_config = dialog->get_user();
	assert(user_config);
	
	role = _role;
	state = SS_NULL;
	resubscribe_after = 0;
	may_resubscribe = false;
	pending = true;
	id_subscription_timeout = 0;
	req_out = NULL;
	event_type = _event_type;
	event_id = _event_id;
	auto_refresh = true;
	subscription_expiry = 3600;
	default_duration = 3600;
}

t_subscription::~t_subscription() {
	if (req_out) remove_client_request(&req_out);
	if (id_subscription_timeout) stop_timer(STMR_SUBSCRIPTION);

	// Cleanup list of unsent NOTIFY messages
	while (!queue_notify.empty()) {
		t_request *r = queue_notify.front();
		queue_notify.pop();
		MEMMAN_DELETE(r);
		delete r;
	}
}

t_subscription_role t_subscription::get_role(void) const {
	return role;
}

t_subscription_state t_subscription::get_state(void) const {
	return state;
}

string t_subscription::get_reason_termination(void) const {
	return reason_termination;
}

unsigned long t_subscription::get_resubscribe_after(void) const {
	return resubscribe_after;
}

bool t_subscription::get_may_resubscribe(void) const {
	return may_resubscribe;
}

string t_subscription::get_event_type(void) const {
	return event_type;
}

string t_subscription::get_event_id(void) const {
	return event_id;
}

unsigned long t_subscription::get_expiry(void) const {
	return subscription_expiry;
}

bool t_subscription::recv_subscribe(t_request *r, t_tuid tuid, t_tid tid) {
	if (role != SR_NOTIFIER) {
		// Reject a SUBSCRIBE coming in for a SUBSCRIBER
		// TODO: is this ok??
		log_file->write_header("t_subscription::recv_subscribe", LOG_NORMAL, LOG_DEBUG);
		log_file->write_raw("SUBSCRIBER receives SUBSCRIBE.\n");
		log_event();
		log_file->write_endl();
		log_file->write_raw(r->encode());
		log_file->write_endl();
		log_file->write_footer();
		
		t_response *resp = r->create_response(R_603_DECLINE);
		send_response(user_config, resp, 0, tid);
		MEMMAN_DELETE(resp);
		delete resp;
		return true;
	}

	// If the subscription is already in the terminated state
	// then a SUBSCRIBE is not allowed anymore.
	if (state == SS_TERMINATED) {
		t_response *resp = r->create_response(R_481_TRANSACTION_NOT_EXIST,
			REASON_481_SUBSCRIPTION_NOT_EXIST);
		send_response(user_config, resp, 0, tid);
		MEMMAN_DELETE(resp);
		delete resp;
		return true;
	}

	if (state == SS_NULL) state = SS_ESTABLISHED;

	// If there is no expires header, then the implementation of
	// the event specific package must deal with this subscribe
	if (!r->hdr_expires.is_populated()) {
		return false;
	}

	// An expiry time of 0 is an unsubscribe
	if (r->hdr_expires.time == 0) {
		stop_timer(STMR_SUBSCRIPTION);
		state = SS_TERMINATED;
		return false;
	}

	// Check if the requested expiry is not too small
	if (r->hdr_expires.time < MIN_DUR_SUBSCRIPTION) {
		t_response *resp = r->create_response(
					R_423_INTERVAL_TOO_BRIEF);
		resp->hdr_min_expires.set_time(MIN_DUR_SUBSCRIPTION);
		send_response(user_config, resp, 0, tid);
		MEMMAN_DELETE(resp);
		delete resp;
		return true;
	}

	// Restart subscription timer
	stop_timer(STMR_SUBSCRIPTION);
	start_timer(STMR_SUBSCRIPTION, r->hdr_expires.time * 1000);
	return false;
}

bool t_subscription::recv_notify(t_request *r, t_tuid tuid, t_tid tid) {
	if (role != SR_SUBSCRIBER) {
		// Reject a NOTIFY coming in for a NOTIFIER
		// TODO: is this ok??
		log_file->write_header("t_subscription::recv_notify", LOG_NORMAL, LOG_DEBUG);
		log_file->write_raw("NOTIFIER receives NOTIFY.\n");
		log_event();
		log_file->write_endl();
		log_file->write_raw(r->encode());
		log_file->write_endl();
		log_file->write_footer();
		
		t_response *resp = r->create_response(R_603_DECLINE);
		send_response(user_config, resp, 0, tid);
		MEMMAN_DELETE(resp);
		delete resp;
		return true;
	}
	
	if (state == SS_NULL) {
		log_file->write_header("t_subscription::recv_notify", LOG_NORMAL, LOG_DEBUG);
		log_file->write_raw("NOTIFY establishes subscription.\n");
		log_event();
		log_file->write_footer();
		
		state = SS_ESTABLISHED;
	}

	if (r->hdr_subscription_state.substate == SUBSTATE_ACTIVE && pending) {
		log_file->write_header("t_subscription::recv_notify", LOG_NORMAL, LOG_DEBUG);
		log_file->write_raw("NOTIFY ends pending state.\n");
		log_event();
		log_file->write_footer();
		
		pending = false;
	}

	if (r->hdr_subscription_state.substate == SUBSTATE_TERMINATED) {
		stop_timer(STMR_SUBSCRIPTION);
		state = SS_TERMINATED;
		reason_termination = r->hdr_subscription_state.reason;
		resubscribe_after = r->hdr_subscription_state.retry_after;
		
		// RFC 3264 3.2.4
		if (resubscribe_after > 0) {
			may_resubscribe = true;
		} else {
			if (reason_termination == EV_REASON_DEACTIVATED ||
			    reason_termination == EV_REASON_TIMEOUT)
			{
				may_resubscribe = true;
			} else if (reason_termination == EV_REASON_PROBATION ||
			           reason_termination == EV_REASON_GIVEUP)
			{
				may_resubscribe = true;
				resubscribe_after = DUR_RESUBSCRIBE;
			}
		}
		
		log_file->write_header("t_subscription::recv_notify", LOG_NORMAL, LOG_DEBUG);
		log_file->write_raw("NOTIFY terminates subscription.\n");
		log_event();
		log_file->write_raw("Termination reason: ");
		log_file->write_raw(reason_termination);
		log_file->write_endl();
		log_file->write_raw("May resubscribe:    ");
		log_file->write_bool(may_resubscribe);
		log_file->write_endl();
		log_file->write_raw("Resubscribe after:  ");
		log_file->write_raw(resubscribe_after);
		log_file->write_endl();
		log_file->write_footer();
	}

	if (r->hdr_subscription_state.expires > 0 && state == SS_ESTABLISHED) {
		log_file->write_header("t_subscription::recv_notify", LOG_NORMAL, LOG_DEBUG);
		log_file->write_raw("Received NOTIFY on established subscription.\n");
		log_event();
		log_file->write_footer();
		
		unsigned long dur = r->hdr_subscription_state.expires;
		if (auto_refresh) {
			if (!id_subscription_timeout ||
			    timekeeper->get_remaining_time(id_subscription_timeout) >=
			    	dur * 1000)
			{
				// Adjust timer to expiry duration indicated
				// in NOTIFY.
				dur -= dur / 10;
				stop_timer(STMR_SUBSCRIPTION);
				start_timer(STMR_SUBSCRIPTION, dur * 1000);
			}
		} else {
			if (!id_subscription_timeout ||
			    timekeeper->get_remaining_time(id_subscription_timeout) <
			    	dur * 1000)
			{
				// Adjust timer to expiry duration indicated
				// in NOTIFY.
				dur += dur / 10;
				stop_timer(STMR_SUBSCRIPTION);
				start_timer(STMR_SUBSCRIPTION, dur * 1000);
			}
		}
	}

	return false;
}

bool t_subscription::recv_response(t_response *r, t_tuid tuid, t_tid tid) {
	switch (r->hdr_cseq.method) {
	case NOTIFY:
		return recv_notify_response(r, tuid, tid);
		break;
	case SUBSCRIBE:
		return recv_subscribe_response(r, tuid, tid);
		break;
	default:
		break;
	}
	
	return false;
}

bool t_subscription::recv_notify_response(t_response *r, t_tuid tuid, t_tid tid) {
	// Discard response if it does not match a pending request
	if (!req_out) return true;
	if (r->hdr_cseq.method != req_out->get_request()->method) return true;

	// Ignore provisional responses
	if (r->is_provisional()) return true;
	
	// Successful response
	if (r->is_success()) {
		if (req_out->get_request()->hdr_subscription_state.substate ==
				SUBSTATE_TERMINATED)
		{
			// This is a 2XX respsone on a NOTIFY that terminates the
			// subscription.
			state = SS_TERMINATED;
			
			log_file->write_header("t_subscription::recv_notify_response", 
				LOG_NORMAL, LOG_DEBUG);
			log_file->write_raw("Subscription terminated by 2XX NOTIFY.\n");
			log_file->write_raw(r->code);
			log_file->write_raw(" " + r->reason + "\n");
			log_event();
			log_file->write_footer();
		}
		remove_client_request(&req_out);
	} else {
		// RFC 3265 3.2.2
		// NOTIFY failed, terminate subscription
		remove_client_request(&req_out);
		state = SS_TERMINATED;
		
		log_file->write_header("t_subscription::recv_notify_response", 
			LOG_NORMAL, LOG_DEBUG);
		log_file->write_raw("Subscription terminated by NOTIFY failure response.\n");
		log_file->write_raw(r->code);
		log_file->write_raw(" " + r->reason + "\n");
		log_event();
		log_file->write_footer();
		return true;
	}

	// If there is a NOTIFY in the queue, then send it
	if (!queue_notify.empty()) {
		log_file->write_header("t_subscription::recv_notify_response", 
			LOG_NORMAL, LOG_DEBUG);
		log_file->write_raw("Get NOTIFY from queue.");
		log_event();
		log_file->write_footer();
	
		t_request *notify = queue_notify.front();
		queue_notify.pop();
		req_out = new t_client_request(user_config, notify,0);
		MEMMAN_NEW(req_out);
		send_request(user_config, notify, req_out->get_tuid());
		MEMMAN_DELETE(notify);
		delete notify;
	}

	return true;
}

bool t_subscription::recv_subscribe_response(t_response *r, t_tuid tuid, t_tid tid) {
	// Discard response if it does not match a pending request
	if (!req_out) return true;
	if (r->hdr_cseq.method != req_out->get_request()->method) return true;

	// Ignore provisional responses
	if (r->is_provisional()) return true;
	
	// Successful response
	if (r->is_success()) {
		if (state == SS_NULL) {
			log_file->write_header("t_subscription::recv_subscribe_response", 
				LOG_NORMAL, LOG_DEBUG);
			log_file->write_raw("Subscription established by 2XX SUBSCRIBE.\n");
			log_file->write_raw(r->code);
			log_file->write_raw(" " + r->reason + "\n");
			log_event();
			log_file->write_footer();
			
			state = SS_ESTABLISHED;
		}
		
		// RFC 3265 7.1, 7.2 says that the Expires header is mandatory
		// in a 2XX response. Some SIP servers do not include this
		// however. To interoperate with such servers, assume that
		// the granted expiry time equals the requested expiry time.
		if (!r->hdr_expires.is_populated()) {
			r->hdr_expires.set_time(
				req_out->get_request()->hdr_expires.time);
				
			log_file->write_header(
				"t_subscription::recv_subscribe_response",
				LOG_NORMAL, LOG_WARNING);
			log_file->write_raw("Mandatory Expires header missing.\n");
			log_file->write_raw("Assuming expires = ");
			log_file->write_raw(r->hdr_expires.time);
			log_file->write_endl();
			log_event();
			log_file->write_footer();
		}

		// If some faulty server sends a non-zero expiry time in
		// a response on an unsubscribe request, then ignore
		// the expiry time.
		if (r->hdr_expires.time == 0 || state == SS_UNSUBSCRIBING) {
			// Unsubscription succeeded.
			stop_timer(STMR_SUBSCRIPTION);
			
			// Start the unsubscribe guard. If the triggered
			// NOTIFY is never received, this guard assures, that
			// the subscription will be cleaned up at timeout.
			start_timer(STMR_SUBSCRIPTION, DUR_UNSUBSCRIBE_GUARD);

			// The subscription will only
			// terminate after a NOTIFY triggered by the unsubscribe
			// has been received or this guard timer expires.
			state = SS_UNSUBSCRIBED;
			auto_refresh = false;
			
			log_file->write_header("t_subscription::recv_subscribe_response", 
				LOG_NORMAL, LOG_DEBUG);
			log_file->write_raw("Unsubcription succesful.\n");
			log_event();
			log_file->write_footer();
		} else {
			// Start/refresh subscribe timer
			stop_timer(STMR_SUBSCRIPTION);
			unsigned long dur = r->hdr_expires.time;
			if (auto_refresh) {
				dur -= dur / 10;
			} else {
				dur += dur / 10;
			}
			start_timer(STMR_SUBSCRIPTION, dur * 1000);
		}

		remove_client_request(&req_out);
		return true;
	}

	// RFC 3265 3.1.4.1
	// SUBSCRIBE failed, terminate subscription
	// NOTE: redirection and authentication responses should have
	// been handled already (eg. on line or phone user level).
	remove_client_request(&req_out);
	state = SS_TERMINATED;
	
	log_file->write_header("t_subscription::recv_subscribe_response", 
		LOG_NORMAL, LOG_DEBUG);
	log_file->write_raw("Subscription terminated by SUBSCRIBE failure response.\n");
	log_file->write_raw(r->code);
	log_file->write_raw(" " + r->reason + "\n");
	log_event();
	log_file->write_footer();

	return true;
}

bool t_subscription::timeout(t_subscribe_timer timer) {
	switch (timer) {
	case STMR_SUBSCRIPTION:
		id_subscription_timeout = 0;
		
		if (role == SR_SUBSCRIBER) {
			log_file->write_header("t_subcription::timeout");
			log_file->write_raw("Subscriber timed out.\n");
			log_event();
			log_file->write_footer();

			if (auto_refresh) {
				// Refresh subscription
				refresh_subscribe();
			} else {
				// The cause for timeout may be temporary.
				// Allow resubscription to overcome a transient problem.
				may_resubscribe = true;
				state = SS_TERMINATED;
			}

			return true;
		}
		break;
	default:
		assert(false);
	}

	return false;
}

bool t_subscription::match_timer(t_subscribe_timer timer, t_object_id id_timer) const {
	return id_timer == id_subscription_timeout;
}

bool t_subscription::match(t_request *r) const {
	if (!r->hdr_event.is_populated()) return false;
	
	// If the subscription has been terminated, then do not match
	// any request.
	if (state == SS_TERMINATED) return false;

	return (r->hdr_event.event_type == event_type &&
	        r->hdr_event.id == event_id);
}

bool t_subscription::is_pending(void) const {
	return pending;
}

void t_subscription::subscribe(unsigned long expires) {
	if (req_out) {
		// Delete previous outgoing request
		MEMMAN_DELETE(req_out);
		delete req_out;
	}
	
	if (expires > 0) {
		subscription_expiry = expires;
	} else {
		subscription_expiry = default_duration;
	}
	
	t_request *r = create_subscribe(subscription_expiry);
	req_out = new t_client_request(user_config, r ,0);
	MEMMAN_NEW(req_out);
	send_request(user_config, r, req_out->get_tuid());
	MEMMAN_DELETE(r);
	delete r;
}

void t_subscription::unsubscribe(void) {
	if (state != SS_ESTABLISHED) {
		state = SS_TERMINATED;
		return;
	}
	
	if (req_out) {
		// Delete previous outgoing request
		MEMMAN_DELETE(req_out);
		delete req_out;
	}

	stop_timer(STMR_SUBSCRIPTION);
	
	t_request *r = create_subscribe(0);
	req_out = new t_client_request(user_config, r ,0);
	MEMMAN_NEW(req_out);
	send_request(user_config, r, req_out->get_tuid());
	MEMMAN_DELETE(r);
	delete r;
	
	// NOTE: the subscription is only ended when the response is received.
	state = SS_UNSUBSCRIBING;
}

void t_subscription::refresh_subscribe(void) {
	if (state != SS_ESTABLISHED) return;
	stop_timer(STMR_SUBSCRIPTION);
	subscribe(subscription_expiry);
}
