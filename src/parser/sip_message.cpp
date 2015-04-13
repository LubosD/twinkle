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

#include <fstream>
#include <iostream>
#include <sstream>

#include "sip_message.h"
#include "util.h"
#include "parse_ctrl.h"
#include "sdp/sdp.h"
#include "audits/memman.h"

////////////////////////////////////
// class t_sip_message
////////////////////////////////////

t_sip_message::t_sip_message() :
	local_ip_(0)
{
	src_ip_port.clear();
	version = SIP_VERSION;
	body = NULL;
}

t_sip_message::t_sip_message(const t_sip_message& m) :
		local_ip_(m.local_ip_),
		src_ip_port(m.src_ip_port),
		version(m.version),
		hdr_accept(m.hdr_accept),
		hdr_accept_encoding(m.hdr_accept_encoding),
		hdr_accept_language(m.hdr_accept_language),
		hdr_alert_info(m.hdr_alert_info),
		hdr_allow(m.hdr_allow),
		hdr_allow_events(m.hdr_allow_events),
		hdr_auth_info(m.hdr_auth_info),
		hdr_authorization(m.hdr_authorization),
		hdr_call_id(m.hdr_call_id),
		hdr_call_info(m.hdr_call_info),
		hdr_contact(m.hdr_contact),
		hdr_content_disp(m.hdr_content_disp),
		hdr_content_encoding(m.hdr_content_encoding),
		hdr_content_language(m.hdr_content_language),
		hdr_content_length(m.hdr_content_length),
		hdr_content_type(m.hdr_content_type),
		hdr_cseq(m.hdr_cseq),
		hdr_date(m.hdr_date),
		hdr_error_info(m.hdr_error_info),
		hdr_event(m.hdr_event),
		hdr_expires(m.hdr_expires),
		hdr_from(m.hdr_from),
		hdr_in_reply_to(m.hdr_in_reply_to),
		hdr_max_forwards(m.hdr_max_forwards),
		hdr_min_expires(m.hdr_min_expires),
		hdr_mime_version(m.hdr_mime_version),
		hdr_organization(m.hdr_organization),
		hdr_p_asserted_identity(m.hdr_p_asserted_identity),
		hdr_p_preferred_identity(m.hdr_p_preferred_identity),
		hdr_priority(m.hdr_priority),
		hdr_privacy(m.hdr_privacy),
		hdr_proxy_authenticate(m.hdr_proxy_authenticate),
		hdr_proxy_authorization(m.hdr_proxy_authorization),
		hdr_proxy_require(m.hdr_proxy_require),
		hdr_rack(m.hdr_rack),
		hdr_record_route(m.hdr_record_route),
		hdr_refer_sub(m.hdr_refer_sub),
		hdr_refer_to(m.hdr_refer_to),
		hdr_referred_by(m.hdr_referred_by),
		hdr_replaces(m.hdr_replaces),
		hdr_reply_to(m.hdr_reply_to),
		hdr_require(m.hdr_require),
		hdr_request_disposition(m.hdr_request_disposition),
		hdr_retry_after(m.hdr_retry_after),
		hdr_route(m.hdr_route),
		hdr_rseq(m.hdr_rseq),
		hdr_server(m.hdr_server),
		hdr_service_route(m.hdr_service_route),
		hdr_sip_etag(m.hdr_sip_etag),
		hdr_sip_if_match(m.hdr_sip_if_match),
		hdr_subject(m.hdr_subject),
		hdr_subscription_state(m.hdr_subscription_state),
		hdr_supported(m.hdr_supported),
		hdr_timestamp(m.hdr_timestamp),
		hdr_to(m.hdr_to),
		hdr_unsupported(m.hdr_unsupported),
		hdr_user_agent(m.hdr_user_agent),
		hdr_via(m.hdr_via),
		hdr_warning(m.hdr_warning),
		hdr_www_authenticate(m.hdr_www_authenticate),
		unknown_headers(m.unknown_headers)
{
	if (m.body) {
		body = m.body->copy();
	} else {
		body = NULL;
	}
}

t_sip_message::~t_sip_message() {
	if (body) {
		MEMMAN_DELETE(body);
		delete body;
	}
}

t_msg_type t_sip_message::get_type(void) const {
	return MSG_SIPFRAG;
}

void t_sip_message::add_unknown_header(const string &name,
				       const string &value) {
	t_parameter h(name, value);
	unknown_headers.push_back(h);
}

bool t_sip_message::is_valid(bool &fatal, string &reason) const {

	// RFC 3261 8.1.1
	// Mandatory headers
	if (!hdr_to.is_populated()) {
		fatal = true;
		reason = "To-header missing";
		return false;
	}

	if (!hdr_from.is_populated()) {
		fatal = true;
		reason = "From header missing";
		return false;
	}

	if (!hdr_cseq.is_populated()) {
		fatal = true;
		reason = "CSeq header missing";
		return false;
	}

	if (!hdr_call_id.is_populated()) {
		fatal = true;
		reason = "Call-ID header missing";
		return false;
	}

	if (!hdr_via.is_populated()) {
		fatal = true;
		reason = "Via header missing";
		return false;
	}

	// RFC 3261 20.15
	// Content-Type MUST be present if body is not empty
	if (body && !hdr_content_type.is_populated()) {
		fatal = false;
		reason = "Content-Type header missing";
		return false;
	}
	
	// RFC 3261 18.4
	// The Content-Length header field MUST be used with stream oriented transports.
	if (cmp_nocase(hdr_via.via_list.front().transport, "tcp") == 0 &&
	    !hdr_content_length.is_populated())
	{
		fatal = false;
		reason = "Content-Length header missing";
		return false;
	}

	return true;
}

string t_sip_message::encode(bool add_content_length) {
	string s;
	string encoded_body;

	// RFC 3261 7.3.1
	// Headers needed by a proxy should be on top
	s += hdr_via.encode();
	s += hdr_route.encode();
	s += hdr_record_route.encode();
	s += hdr_service_route.encode();
	s += hdr_proxy_require.encode();
	s += hdr_max_forwards.encode();
	s += hdr_proxy_authenticate.encode();
	s += hdr_proxy_authorization.encode();

	// Order as in many examples
	s += hdr_to.encode();
	s += hdr_from.encode();
	s += hdr_call_id.encode();
	s += hdr_cseq.encode();
	s += hdr_contact.encode();
	s += hdr_content_type.encode();
	
	// Privacy related headers
	s += hdr_privacy.encode();
	s += hdr_p_asserted_identity.encode();
	s += hdr_p_preferred_identity.encode();

	// Authentication headers
	s += hdr_auth_info.encode();
	s += hdr_authorization.encode();
	s += hdr_www_authenticate.encode();

	// Remaining headers in alphabetical order
	s += hdr_accept.encode();
	s += hdr_accept_encoding.encode();
	s += hdr_accept_language.encode();
	s += hdr_alert_info.encode();
	s += hdr_allow.encode();
	s += hdr_allow_events.encode();
	s += hdr_call_info.encode();
	s += hdr_content_disp.encode();
	s += hdr_content_encoding.encode();
	s += hdr_content_language.encode();
	s += hdr_date.encode();
	s += hdr_error_info.encode();
	s += hdr_event.encode();
	s += hdr_expires.encode();
	s += hdr_in_reply_to.encode();
	s += hdr_min_expires.encode();
	s += hdr_mime_version.encode();
	s += hdr_organization.encode();
	s += hdr_priority.encode();
	s += hdr_rack.encode();
	s += hdr_refer_sub.encode();
	s += hdr_refer_to.encode();
	s += hdr_referred_by.encode();
	s += hdr_replaces.encode();
	s += hdr_reply_to.encode();
	s += hdr_require.encode();
	s += hdr_request_disposition.encode();
	s += hdr_retry_after.encode();
	s += hdr_rseq.encode();
	s += hdr_server.encode();
	s += hdr_sip_etag.encode();
	s += hdr_sip_if_match.encode();
	s += hdr_subject.encode();
	s += hdr_subscription_state.encode();
	s += hdr_supported.encode();
	s += hdr_timestamp.encode();
	s += hdr_unsupported.encode();
	s += hdr_user_agent.encode();
	s += hdr_warning.encode();

	// Unknown headers
	for (list<t_parameter>::const_iterator i = unknown_headers.begin();
	     i != unknown_headers.end(); i++)
	{
		s += i->name;
		s += ": ";
		s += i->value;
		s += CRLF;
	}

	// Encode body if present. Set the content length.
	if (body) {
		encoded_body = body->encode();
		hdr_content_length.set_length(encoded_body.size());
	} else {
		// RFC 3261 20.14
		// If no body is present then Content-Length MUST be 0
		hdr_content_length.set_length(0);
	}

	// Content-Length appears last in examples
	if (add_content_length) {
		s += hdr_content_length.encode();
	}

	// Blank line between headers and body
	s += CRLF;

	// Add body
	if (body) s += encoded_body;

	return s;
}

list<string> t_sip_message::encode_env(void) {
	list<string> l;

	// RFC 3261 7.3.1
	// Headers needed by a proxy should be on top
	l.push_back(hdr_via.encode_env());
	l.push_back(hdr_route.encode_env());
	l.push_back(hdr_record_route.encode_env());
	l.push_back(hdr_service_route.encode_env());
	l.push_back(hdr_proxy_require.encode_env());
	l.push_back(hdr_max_forwards.encode_env());
	l.push_back(hdr_proxy_authenticate.encode_env());
	l.push_back(hdr_proxy_authorization.encode_env());

	// Order as in many examples
	l.push_back(hdr_to.encode_env());
	l.push_back(hdr_from.encode_env());
	l.push_back(hdr_call_id.encode_env());
	l.push_back(hdr_cseq.encode_env());
	l.push_back(hdr_contact.encode_env());
	l.push_back(hdr_content_type.encode_env());
	
	// Authentication headers
	l.push_back(hdr_auth_info.encode_env());
	l.push_back(hdr_authorization.encode_env());
	l.push_back(hdr_www_authenticate.encode_env());

	// Authentication headers
	l.push_back(hdr_auth_info.encode_env());
	l.push_back(hdr_authorization.encode_env());
	l.push_back(hdr_www_authenticate.encode_env());

	// Remaining headers in alphabetical order
	l.push_back(hdr_accept.encode_env());
	l.push_back(hdr_accept_encoding.encode_env());
	l.push_back(hdr_accept_language.encode_env());
	l.push_back(hdr_alert_info.encode_env());
	l.push_back(hdr_allow.encode_env());
	l.push_back(hdr_allow_events.encode_env());
	l.push_back(hdr_call_info.encode_env());
	l.push_back(hdr_content_disp.encode_env());
	l.push_back(hdr_content_encoding.encode_env());
	l.push_back(hdr_content_language.encode_env());
	l.push_back(hdr_date.encode_env());
	l.push_back(hdr_error_info.encode_env());
	l.push_back(hdr_event.encode_env());
	l.push_back(hdr_expires.encode_env());
	l.push_back(hdr_in_reply_to.encode_env());
	l.push_back(hdr_min_expires.encode_env());
	l.push_back(hdr_mime_version.encode_env());
	l.push_back(hdr_organization.encode_env());
	l.push_back(hdr_priority.encode_env());
	l.push_back(hdr_rack.encode_env());
	l.push_back(hdr_refer_sub.encode_env());
	l.push_back(hdr_refer_to.encode_env());
	l.push_back(hdr_referred_by.encode_env());
	l.push_back(hdr_replaces.encode_env());
	l.push_back(hdr_reply_to.encode_env());
	l.push_back(hdr_require.encode_env());
	l.push_back(hdr_request_disposition.encode_env());
	l.push_back(hdr_retry_after.encode_env());
	l.push_back(hdr_rseq.encode_env());
	l.push_back(hdr_server.encode_env());
	l.push_back(hdr_sip_etag.encode_env());
	l.push_back(hdr_sip_if_match.encode_env());
	l.push_back(hdr_subject.encode_env());
	l.push_back(hdr_subscription_state.encode_env());
	l.push_back(hdr_supported.encode_env());
	l.push_back(hdr_timestamp.encode_env());
	l.push_back(hdr_unsupported.encode_env());
	l.push_back(hdr_user_agent.encode_env());
	l.push_back(hdr_warning.encode_env());

	// Unknown headers
	for (list<t_parameter>::const_iterator i = unknown_headers.begin();
	     i != unknown_headers.end(); i++)
	{
		string s = "SIP_";
		s += toupper(replace_char(i->name, '-', '_'));
		s += '=';
		s += i->value;
		l.push_back(s);
	}
	
	l.push_back(hdr_content_length.encode_env());

	return l;
}

t_sip_message *t_sip_message::copy(void) const {
	t_sip_message *m = new t_sip_message(*this);
	MEMMAN_NEW(m);
	return m;
}

void t_sip_message::set_body_plain_text(const string &text, const string &charset) {
	// Content-Type header
	t_media mime_type("text", "plain");
	mime_type.charset = charset;
	hdr_content_type.set_media(mime_type);
	
	if (body) {
		MEMMAN_DELETE(body);
		delete body;
	}
	
	body = new t_sip_body_plain_text(text);
	MEMMAN_NEW(body);
}

bool t_sip_message::set_body_from_file(const string &filename, const t_media &media) {
	// Open file and set read pointer at end so we know the size.
	ifstream f(filename.c_str(), ios::binary);
	if (!f) return false;
	
	ostringstream body_stream(ios::binary);
	
	// Copy file into body
	body_stream << f.rdbuf();
	
	if (!f.good() || !body_stream.good()) {
		return false;
	}
	
	// Create body of correct type
	t_sip_body *new_body = NULL;
	if (media.type == "text" && media.subtype == "plain") {
		t_sip_body_plain_text *text_body = new t_sip_body_plain_text(body_stream.str());
		MEMMAN_NEW(text_body);

		new_body = text_body;
	} else {
		t_sip_body_opaque *opaque_body = new t_sip_body_opaque(body_stream.str());
		MEMMAN_NEW(opaque_body);
		
		new_body = opaque_body;
	}
	
	if (body) {
		MEMMAN_DELETE(body);
		delete body;
	}
	body = new_body;
	
	// Content-Type header
	hdr_content_type.set_media(media);
	
	return true;
}

size_t t_sip_message::get_encoded_size(void) {
	string s = encode();
	return s.size();
}

bool t_sip_message::local_ip_check(void) const {
	if (get_type() == MSG_REQUEST && hdr_via.is_populated()) {
		const t_via &v = hdr_via.via_list.front();
		if (v.host == "0.0.0.0") return false;
	}
	
	if (hdr_contact.is_populated()) {
		if (!hdr_contact.any_flag && !hdr_contact.contact_list.empty()) {
			const t_contact_param &c = hdr_contact.contact_list.front();
			if (c.uri.get_host() == "0.0.0.0") return false;
		}
	}
	
	if (body) {
		return body->local_ip_check();
	}
	
	return true;
}

void t_sip_message::calc_local_ip(void) {
	// Do nothing
}

unsigned long t_sip_message::get_local_ip(void) {
	if (local_ip_ == 0) calc_local_ip();
	return local_ip_;
}
