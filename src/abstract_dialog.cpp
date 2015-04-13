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
#include "abstract_dialog.h"
#include "log.h"
#include "phone.h"
#include "phone_user.h"
#include "util.h"
#include "userintf.h"
#include "audits/memman.h"

extern string		user_host;
extern t_phone		*phone;

// Private
void t_abstract_dialog::remove_client_request(t_client_request **cr) {
	if ((*cr)->dec_ref_count() == 0) {
		MEMMAN_DELETE(*cr);
		delete *cr;
	}

	*cr = NULL;
}

// Create a request within a dialog
// RFC 3261 12.2.1.1
t_request *t_abstract_dialog::create_request(t_method m) {
	t_user *user_config = phone_user->get_user_profile();
	
	t_request *r = new t_request(m);
	MEMMAN_NEW(r);

	// To header
	r->hdr_to.set_uri(remote_uri);
	r->hdr_to.set_display(remote_display);
	r->hdr_to.set_tag(remote_tag);

	// From header
	r->hdr_from.set_uri(local_uri);
	r->hdr_from.set_display(local_display);
	r->hdr_from.set_tag(local_tag);

	// Call-ID header
	r->hdr_call_id.set_call_id(call_id);

	// CSeq header
	r->hdr_cseq.set_method(m);
	r->hdr_cseq.set_seqnr(++local_seqnr);

	// Set Max-Forwards header
	r->hdr_max_forwards.set_max_forwards(MAX_FORWARDS);

	// User-Agent
	SET_HDR_USER_AGENT(r->hdr_user_agent);

	// RFC 3261 12.2.1.1
	// Request URI and Route header
	r->set_route(remote_target_uri, route_set);
        
        // Caculate destination set. A DNS request can result in multiple
        // IP address. In failover scenario's the request must be sent to
        // the next IP address in the list. As the request will be copied
        // in various places, the destination set must be calculated now.
        // In previous version the DNS request was done by the transaction
        // manager. This is too late as the transaction manager gets a copy
        // of the request. The destination set should be set in the copy
        // kept by the dialog.
        r->calc_destinations(*user_config);
        
        // The Via header can only be created after the destinations
        // are calculated, because the destination deterimines which
        // local IP address should be used.
        
        // Via header
        unsigned long local_ip = r->get_local_ip();
	t_via via(USER_HOST(user_config, h_ip2str(local_ip)), PUBLIC_SIP_PORT(user_config));
	r->hdr_via.add_via(via);

	return r;
}

void t_abstract_dialog::create_route_set(t_response *r) {
	// Originally the check was this:
	// if (route_set.empty() && r->hdr_record_route.is_populated())
	// This prevented the route set from being altered between a 18X response
	// and a 2XX response. This is allowed per RFC 3261 13.2.2.4
	if (r->hdr_record_route.is_populated())
	{
		route_set = r->hdr_record_route.route_list;
		route_set.reverse();
	} else {
		route_set.clear();
	}
}

void t_abstract_dialog::create_remote_target(t_response *r) {
	if (r->hdr_contact.is_populated()) {
		remote_target_uri = r->hdr_contact.contact_list.front().uri;
		remote_target_display = r->hdr_contact.contact_list.front().display;
	}
}

void t_abstract_dialog::resend_request(t_client_request *cr) {
	t_user *user_config = phone_user->get_user_profile();
	t_request *req = cr->get_request();

	// A new sequence number must be assigned
	req->hdr_cseq.set_seqnr(++local_seqnr);

	// Create a new via-header. Otherwise the
	// request will be seen as a retransmission
	unsigned long local_ip = req->get_local_ip();
	req->hdr_via.via_list.clear();
	t_via via(USER_HOST(user_config, h_ip2str(local_ip)), PUBLIC_SIP_PORT(user_config));
	req->hdr_via.add_via(via);

	cr->renew(0);
	send_request(req, cr->get_tuid());
}

bool t_abstract_dialog::resend_request_auth(t_client_request *cr, t_response *resp) {
	t_user *user_config = phone_user->get_user_profile();
	t_request *req = cr->get_request();

	// Add authorization header, increment CSeq and create new branch id
	if (phone->authorize(user_config, req, resp)) {
		resend_request(cr);
		return true;
	}

	return false;
}

bool t_abstract_dialog::redirect_request(t_client_request *cr, t_response *resp,
		t_contact_param &contact) 
{
	t_user *user_config = phone_user->get_user_profile();
	
	// If the response is a 3XX response then add redirection contacts
	if (resp->get_class() == R_3XX  && resp->hdr_contact.is_populated()) {
		cr->redirector.add_contacts(
					resp->hdr_contact.contact_list);
	}

	// Get next destination
	if (!cr->redirector.get_next_contact(contact)) {
		// There is no next destination
		return false;
	}

	t_request *req = cr->get_request();

	// Ask user for permission to redirect if indicated by user config
	if (user_config->get_ask_user_to_redirect()) {
		if(!ui->cb_ask_user_to_redirect_request(user_config,
				contact.uri, contact.display, resp->hdr_cseq.method)) 
		{
			// User did not permit to redirect
			return false;
		}
	}

	// Change the request URI to the new URI.
	// As the URI changes the destination set must be recalculated
	req->uri = contact.uri;
	req->calc_destinations(*user_config);

	resend_request(cr);

	return true;
}

bool t_abstract_dialog::failover_request(t_client_request *cr) {
	log_file->write_report("Failover to next destination.",
				"t_abstract_dialog::failover_request");
	
	t_request *req = cr->get_request();
	
	// Get next destination
	if (!req->next_destination()) {
		log_file->write_report("No next destination for failover.",
				"t_abstract_dialog::failover_request");
		return false;
	}
	
	resend_request(cr);
	return true;
}


////////////
// Public
////////////

t_abstract_dialog::t_abstract_dialog(t_phone_user *pu) :
	t_id_object()
{
	assert(pu);
	phone_user = pu;
	call_id_owner = false;
	
	local_seqnr = 0;
	remote_seqnr = 0;
	remote_seqnr_set = false;
	
	local_resp_nr = 0;
	remote_resp_nr = 0;
	
	remote_ip_port.clear();
	
	log_file->write_header("t_abstract_dialog::t_abstract_dialog", LOG_NORMAL, LOG_DEBUG);
	log_file->write_raw("Created dialog, id=");
	log_file->write_raw(get_object_id());
	log_file->write_endl();
	log_file->write_footer();
}

t_abstract_dialog::~t_abstract_dialog() {
	log_file->write_header("t_abstract_dialog::~t_abstract_dialog", LOG_NORMAL, LOG_DEBUG);
	log_file->write_raw("Destroy dialog, id=");
	log_file->write_raw(get_object_id());
	log_file->write_endl();
	log_file->write_footer();
}

t_user *t_abstract_dialog::get_user(void) const {
	return phone_user->get_user_profile();
}

void t_abstract_dialog::recvd_response(t_response *r, t_tuid tuid, t_tid tid) {
	// The source address and port of a message may be 0 when the
	// message was sent internally.
	if (!r->src_ip_port.is_null()) {
		remote_ip_port = r->src_ip_port;
	}
}

void t_abstract_dialog::recvd_request(t_request *r, t_tuid tuid, t_tid tid) {
	// The source address and port of a message may be 0 when the
	// message was sent internally.
	if (!r->src_ip_port.is_null()) {
		remote_ip_port = r->src_ip_port;
	}
}

bool t_abstract_dialog::match_response(t_response *r, t_tuid tuid) {
	return (call_id == r->hdr_call_id.call_id &&
		local_tag == r->hdr_from.tag &&
		(remote_tag.size() == 0 || remote_tag == r->hdr_to.tag));
}

bool t_abstract_dialog::match_request(t_request *r) {
	return match(r->hdr_call_id.call_id, r->hdr_to.tag, r->hdr_from.tag);
}

bool t_abstract_dialog::match_partial_request(t_request *r) {
	return (r->hdr_call_id.call_id == call_id &&
	        r->hdr_to.tag == local_tag);
}

bool t_abstract_dialog::match(const string &_call_id, const string &to_tag, 
		const string &from_tag) const
{
	return (call_id == _call_id &&
		local_tag == to_tag &&
		remote_tag == from_tag);
}

t_url t_abstract_dialog::get_remote_target_uri(void) const {
	return remote_target_uri;
}

string t_abstract_dialog::get_remote_target_display(void) const {
	return remote_target_display;
}

t_url t_abstract_dialog::get_remote_uri(void) const {
	return remote_uri;
}

string t_abstract_dialog::get_remote_display(void) const {
	return remote_display;
}

t_ip_port t_abstract_dialog::get_remote_ip_port(void) const {
	return remote_ip_port;
}

string t_abstract_dialog::get_call_id(void) const {
	return call_id;
}

string t_abstract_dialog::get_local_tag(void) const {
	return local_tag;
}

string t_abstract_dialog::get_remote_tag(void) const {
	return remote_tag;
}

bool t_abstract_dialog::remote_extension_supported(const string &extension) const {
	return (remote_extensions.find(extension) != remote_extensions.end());
}

bool t_abstract_dialog::is_call_id_owner(void) const {
	return call_id_owner;
}
