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

#include "subscription_dialog.h"

#include <cassert>

#include "log.h"
#include "phone.h"
#include "phone_user.h"
#include "protocol.h"
#include "userintf.h"
#include "util.h"
#include "audits/memman.h"

extern t_phone *phone;
extern string local_hostname;

t_subscription_dialog::t_subscription_dialog(t_phone_user *_phone_user) :
		t_abstract_dialog(_phone_user),
		subscription(NULL)
{}

void t_subscription_dialog::send_request(t_request *r, t_tuid tuid) {
	t_user *user_config = phone_user->get_user_profile();
	phone->send_request(user_config, r, tuid);
}

void t_subscription_dialog::process_subscribe(t_request *r, t_tuid tuid, t_tid tid) {
	if (get_subscription_state() == SS_NULL) {
		// Process initial incoming SUBSCRIBE. Create dialog state.
		// Set local tag
		if (r->hdr_to.tag.size() == 0) {
			local_tag = NEW_TAG;
		} else {
			local_tag = r->hdr_to.tag;
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
	}
	
	(void)subscription->recv_subscribe(r, tuid, tid);
}

void t_subscription_dialog::process_notify(t_request *r, t_tuid tuid, t_tid tid) {
	// RFC 3265 3.1.4.4
	// A NOTIFY may be received before a 2XX response on the SUBSCRIBE.
	// If this happens, the remote information must be set using the
	// NOTIFY from header.
	if (remote_tag.empty()) {
		remote_tag = r->hdr_from.tag;
		remote_uri = r->hdr_from.uri;
		remote_display = r->hdr_from.display;
	}
	
	(void)subscription->recv_notify(r, tuid, tid);
}

bool t_subscription_dialog::process_initial_subscribe_response(t_response *r, t_tuid tuid, t_tid tid) {
	switch (r->get_class()) {
	case R_2XX:
		remote_tag = r->hdr_to.tag;
		remote_uri = r->hdr_to.uri;
		remote_display = r->hdr_to.display;
		create_route_set(r);
		create_remote_target(r);
		break;
	case R_4XX:
		if (r->code == R_423_INTERVAL_TOO_BRIEF) {
			if (!r->hdr_min_expires.is_populated()) {
				// Violation of RFC 3261 10.3 item 7		
				log_file->write_report("Expires header missing from 423 response.",
					"t_subscription_dialog::process_initial_subscribe_response",
					LOG_NORMAL, LOG_WARNING);
				break;
			}

			if (r->hdr_min_expires.time <= subscription->get_expiry()) {
				// Wrong Min-Expires time
				string s = "Min-Expires (";
				s += ulong2str(r->hdr_min_expires.time);
				s += ") is smaller than the requested ";
				s += "time (";
				s += ulong2str(subscription->get_expiry());
				s += ")";
                                log_file->write_report(s,
                                	"t_subscription_dialog::process_initial_subscribe_response",
                                	LOG_NORMAL, LOG_WARNING);
				break;
			}
			
			// Subscribe with the advised interval
			subscribe(r->hdr_min_expires.time, remote_target_uri, 
				remote_uri, remote_display);
			return true;
		}
		
		break;
	default:
		break;
	}
	
	return false;
}

t_subscription_dialog::~t_subscription_dialog() {
	if (subscription) {
		MEMMAN_DELETE(subscription);
		delete subscription;
	}
}

t_request *t_subscription_dialog::create_request(t_method m) {
	t_user *user_config = phone_user->get_user_profile();
	t_request *r = t_abstract_dialog::create_request(m);
	
	// Contact header
	t_contact_param contact;
	switch (m) {
	case REFER:
	case SUBSCRIBE:
	case NOTIFY:
		// RFC 3265 7.1, RFC 3515 2.2
		// Contact header is mandatory
		contact.uri.set_url(user_config->create_user_contact(false,
				h_ip2str(r->get_local_ip())));
		r->hdr_contact.add_contact(contact);
		break;
	default:
		break;
	}
	
	return r;
}

bool t_subscription_dialog::resend_request_auth(t_response *resp) {
	t_client_request **current_cr = &(subscription->req_out);
	if (!*current_cr) return false;
	t_request *req = (*current_cr)->get_request();
	
	if (phone_user->authorize(req, resp)) {
		resend_request(*current_cr);
		return true;
	}
	
	return false;
}

bool t_subscription_dialog::redirect_request(t_response *resp) {
	t_user *user_config = phone_user->get_user_profile();
	
	t_client_request **current_cr = &(subscription->req_out);
	if (!*current_cr) return false;
	t_request *req = (*current_cr)->get_request();

	// If the response is a 3XX response then add redirection
	// contacts
	if (resp->get_class() == R_3XX  &&
		resp->hdr_contact.is_populated())
	{
		(*current_cr)->redirector.add_contacts(
				resp->hdr_contact.contact_list);
	}

	// Get next destination
	t_contact_param contact;
	if ((*current_cr)->redirector.get_next_contact(contact)) {
		// Ask user for permission to redirect if indicated
		// by user config
		bool permission = true;
		if (user_config->get_ask_user_to_redirect()) {
			permission = ui->cb_ask_user_to_redirect_request(
						user_config,
						contact.uri, contact.display,
						resp->hdr_cseq.method);
		}

		if (permission) {
			req->uri = contact.uri;
			req->calc_destinations(*user_config);
			ui->cb_redirecting_request(user_config, contact);
			resend_request(*current_cr);
			return true;
		}
	}
	
	return false;
}

bool t_subscription_dialog::failover_request(t_response *resp) {
	t_client_request **current_cr = &(subscription->req_out);
	if (!*current_cr) return false;
	t_request *req = (*current_cr)->get_request();
	
	if (req->next_destination()) {
		log_file->write_report("Failover to next destination.",
			"t_subscription_dialog::handle_response_out_of_dialog");
		resend_request(*current_cr);
		return true;
	}
	
	return false;
}

void t_subscription_dialog::recvd_response(t_response *r, t_tuid tuid, t_tid tid) {
	t_user *user_config = phone_user->get_user_profile();
	t_abstract_dialog::recvd_response(r, tuid ,tid);

	t_client_request *cr = subscription->req_out;
	if (!cr) return;
	
	// Check cseq
	if (r->hdr_cseq.method != cr->get_request()->method) {
		return;
	}
	if (r->hdr_cseq.seqnr != cr->get_request()->hdr_cseq.seqnr) return;
	
	// Authentication
	if (r->must_authenticate()) {
		if (resend_request_auth(r)) {
			return;
		}

		// Authentication failed
		// Handle the 401/407 as a normal failure response
	}
	
	// RFC 3263 4.3
	// Failover
	if (r->code == R_503_SERVICE_UNAVAILABLE) {
		if (failover_request(r)) {
			return;
		}			
	}
	
	// Redirect failed request if there is another destination
	if (r->get_class() > R_2XX && user_config->get_allow_redirection()) {
		if (redirect_request(r)) {
			return;
		}
	}

	// Set the transaction identifier. This identifier is needed if the
	// transaction must be aborted at a later time.
	cr->set_tid(tid);
	
	switch (r->hdr_cseq.method) {
	case SUBSCRIBE:
		// Process response to initial SUBSCRIBE
		if (get_subscription_state() == SS_NULL) {
			if (process_initial_subscribe_response(r, tuid, tid)) {
				return;
			}
		}
		break;
	default:
		break;
	}
	
	(void)subscription->recv_response(r, tuid, tid);
}

void t_subscription_dialog::recvd_request(t_request *r, t_tuid tuid, t_tid tid) {
	t_response *resp;
	
	t_abstract_dialog::recvd_request(r, tuid, tid);
	
	// Check cseq
	// RFC 3261 12.2.2
	if (remote_seqnr_set && r->hdr_cseq.seqnr <= remote_seqnr) {
		// Request received out of order.		
		log_file->write_header("t_subscription_dialog::recvd_request",
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
		phone->send_response(resp, tuid, tid);
		MEMMAN_DELETE(resp);
		delete resp;

		return;
	}
	
	remote_seqnr = r->hdr_cseq.seqnr;
	remote_seqnr_set = true;

	switch (r->method) {
	case SUBSCRIBE:	
		process_subscribe(r, tuid, tid);
		break;
	case NOTIFY:
		process_notify(r, tuid, tid);
		break;
	default:
		// Other requests are not supported in a subscription dialog.
		resp = r->create_response(R_500_INTERNAL_SERVER_ERROR);
		phone->send_response(resp, tuid, tid);
		MEMMAN_DELETE(resp);
		delete resp;
		break;
	}
}

bool t_subscription_dialog::match_request(t_request *r, bool &partial) {
	if (!subscription->match(r)) return false;

	if (t_abstract_dialog::match_request(r)) {
		return true;
	}
	
	partial = t_abstract_dialog::match_partial_request(r);
	return false; 
}

t_subscription_state t_subscription_dialog::get_subscription_state(void) const {
	return subscription->get_state();
}

string t_subscription_dialog::get_reason_termination(void) const {
	return subscription->get_reason_termination();
}

unsigned long t_subscription_dialog::get_resubscribe_after(void) const {
	return subscription->get_resubscribe_after();
}

bool t_subscription_dialog::get_may_resubscribe(void) const {
	return subscription->get_may_resubscribe();
}

bool t_subscription_dialog::timeout(t_subscribe_timer timer) {
	return subscription->timeout(timer);
}

bool t_subscription_dialog::match_timer(t_subscribe_timer timer, t_object_id id_timer) const {
	return subscription->match_timer(timer, id_timer);
}

void t_subscription_dialog::subscribe(unsigned long expires, const t_url &req_uri,
		const t_url &to_uri, const string &to_display) 
{
	t_user *user_config = phone_user->get_user_profile();
	
	assert (get_subscription_state() == SS_NULL);
	call_id = NEW_CALL_ID(user_config);
	call_id_owner = true;
	local_tag = NEW_TAG;
	local_display = user_config->get_display(false);
	local_uri = user_config->create_user_uri(false);
	local_seqnr = rand() % 1000 + 1;
	remote_uri = to_uri;
	remote_display = to_display;
	remote_tag.clear();
	remote_target_uri = req_uri;
	route_set = phone_user->get_service_route();
	
	subscription->subscribe(expires);
}

void t_subscription_dialog::unsubscribe(void) {
	subscription->unsubscribe();
}

void t_subscription_dialog::refresh_subscribe(void) {
	subscription->refresh_subscribe();
}
