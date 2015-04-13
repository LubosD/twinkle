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

#include "epa.h"

#include "log.h"
#include "phone.h"
#include "timekeeper.h"
#include "util.h"
#include "audits/memman.h"

extern t_phone *phone;
extern t_event_queue	*evq_timekeeper;
extern string local_hostname;

/////////////
// PRIVATE
/////////////

void t_epa::enqueue_request(t_request *r) {
	log_file->write_header("t_epa::enqueue_request", LOG_NORMAL, LOG_DEBUG);
	log_file->write_raw("Enqueue request\n");
	log_publication();
	log_file->write_footer();
	
	queue_publish.push(r);
}


/////////////
// PROTECTED
/////////////

void t_epa::log_publication() const {
	log_file->write_raw("Event: ");
	log_file->write_raw(event_type);
	log_file->write_raw(", URI: ");
	log_file->write_raw(request_uri.encode());
	log_file->write_raw(", SIP-ETag: ");
	log_file->write_raw(etag);
	log_file->write_endl();
}

void t_epa::remove_client_request(t_client_request **cr) {
	if ((*cr)->dec_ref_count() == 0) {
		MEMMAN_DELETE(*cr);
		delete *cr;
	}

	*cr = NULL;
}

t_request *t_epa::create_publish(unsigned long expires, t_sip_body *body) const {
	t_user *user_config = phone_user->get_user_profile();
	t_request *r = phone_user->create_request(PUBLISH, request_uri);
	
	// Call-ID
	r->hdr_call_id.set_call_id(NEW_CALL_ID(user_config));
	
	// CSeq
	r->hdr_cseq.set_method(PUBLISH);
	r->hdr_cseq.set_seqnr(NEW_SEQNR);
	
	// To
	r->hdr_to.set_uri(user_config->create_user_uri(false));
	r->hdr_to.set_display(user_config->get_display(false));
	
	// RFC 3903 4 Expires
	r->hdr_expires.set_time(expires);
	
	// RFC 3903 4 Event
	r->hdr_event.set_event_type(event_type);
	
	// SIP-If-Match
	if (!etag.empty()) {
		r->hdr_sip_if_match.set_etag(etag);
	}
	
	// Body
	if (body) {
		r->body = body;
		r->hdr_content_type.set_media(body->get_media());
	}

	return r;
}

void t_epa::send_request(t_request *r, t_tuid tuid) const {
	phone->send_request(phone_user->get_user_profile(), r, tuid);
}

void t_epa::send_publish_from_queue(void) {
	// If there is a PUBLISH in the queue, then send it
	while (!queue_publish.empty()) {
		log_file->write_header("t_epa::send_publish_from_queue", LOG_NORMAL, LOG_DEBUG);
		log_file->write_raw("Get PUBLISH from queue.\n");
		log_publication();
		log_file->write_footer();
	
		t_request *req = queue_publish.front();
		queue_publish.pop();
		
		// Update the SIP-If-Match header to the current entity tag
		if (!etag.empty()) {
			req->hdr_sip_if_match.set_etag(etag);
		} else {
			req->hdr_sip_if_match.clear();
		}
		
		if (req->hdr_expires.time == 0) {
			if (epa_state != EPA_PUBLISHED) {
				log_file->write_header("t_epa::send_publish_from_queue", LOG_NORMAL, LOG_DEBUG);
				log_file->write_raw("Nothing published, discard unpublish\n");
				log_publication();
				log_file->write_footer();
				
				MEMMAN_DELETE(req);
				delete req;
				continue;
			}
			is_unpublishing = true;
		} else {
			is_unpublishing = false;
		}
		
		stop_timer(PUBLISH_TMR_PUBLICATION);
		
		req_out = new t_client_request(phone_user->get_user_profile(), req, 0);
		MEMMAN_NEW(req_out);
		send_request(req, req_out->get_tuid());
		MEMMAN_DELETE(req);
		delete req;
		
		break;
	}
}

void t_epa::start_timer(t_publish_timer timer, long duration) {
	t_tmr_publish *t = NULL;

	switch(timer) {
	case PUBLISH_TMR_PUBLICATION:
		t = new t_tmr_publish(duration, timer, event_type);
		MEMMAN_NEW(t);
		id_publication_timeout = t->get_object_id();
		break;
	default:
		assert(false);
	}

	evq_timekeeper->push_start_timer(t);
	MEMMAN_DELETE(t);
	delete t;
}

void t_epa::stop_timer(t_publish_timer timer) {
	unsigned short	*id;

	switch(timer) {
	case PUBLISH_TMR_PUBLICATION:
		id = &id_publication_timeout;
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

t_epa::t_epa(t_phone_user *pu, const string &_event_type, const t_url _request_uri) :
	phone_user(pu),
	epa_state(EPA_UNPUBLISHED),
	event_type(_event_type),
	request_uri(_request_uri),
	id_publication_timeout(0),
	publication_expiry(3600),
	default_duration(3600),
	is_unpublishing(false),
	cached_body(NULL),
	req_out(NULL)
{}

t_epa::~t_epa() {
	clear();
}

t_epa::t_epa_state t_epa::get_epa_state(void) const {
	return epa_state;
}

string t_epa::get_failure_msg(void) const {
	return failure_msg;
}

t_phone_user *t_epa::get_phone_user(void) const {
	return phone_user;
}

t_user *t_epa::get_user_profile(void) const {
	return phone_user->get_user_profile();
}

bool t_epa::recv_response(t_response *r, t_tuid tuid, t_tid tid) {
	// Discard response if it does not match a pending request
	if (!req_out) return true;
	t_request *req = req_out->get_request();
	if (r->hdr_cseq.method != req->method) return true;

	// Ignore provisional responses
	if (r->is_provisional()) return true;
	
	if (r->is_success()) {
		// RFC 3903 11.3
		// A 2XX response must contain a SIP-ETag header
		if (r->hdr_sip_etag.is_populated()) {
			etag = r->hdr_sip_etag.etag;
		} else {
			log_file->write_report("SIP-ETag header missing from PUBLISH 2XX response.",
				"t_epa::recv_response", LOG_NORMAL, LOG_WARNING);
			etag.clear();
		}
		
		// RFC 3903 1.1.1 says that the Expires header is mandatory
		// in a 2XX response. Some SIP servers do not include this
		// however. To interoperate with such servers, assume that
		// the granted expiry time equals the requested expiry time.
		if (!r->hdr_expires.is_populated()) {
			r->hdr_expires.set_time(
				req->hdr_expires.time);
				
			log_file->write_header("t_epa::recv_response",
				LOG_NORMAL, LOG_WARNING);
			log_file->write_raw("Mandatory Expires header missing.\n");
			log_file->write_raw("Assuming expires = ");
			log_file->write_raw(r->hdr_expires.time);
			log_file->write_endl();
			log_publication();
			log_file->write_footer();
		}
		
		// If some faulty server sends a non-zero expiry time in
		// a response on an unsubscribe request, then ignore
		// the expiry time.
		if (r->hdr_expires.time == 0 || is_unpublishing) {
			// Unpublish succeeded.
			stop_timer(PUBLISH_TMR_PUBLICATION);
			etag.clear();
			is_unpublishing = false;
			epa_state = EPA_UNPUBLISHED;
			
			log_file->write_header("t_epa::recv_response", 
				LOG_NORMAL, LOG_WARNING);
			log_file->write_raw("Unpublish successful.\n");
			log_publication();
			log_file->write_footer();
		} else {
			log_file->write_header("t_epa::recv_response");
			log_file->write_raw("Publication sucessful.\n");
			log_publication();
			log_file->write_footer();
		
			// Start/refresh publish timer
			stop_timer(PUBLISH_TMR_PUBLICATION);
			unsigned long dur = r->hdr_expires.time;
			dur -= dur / 10;
			start_timer(PUBLISH_TMR_PUBLICATION, dur * 1000);
			epa_state = EPA_PUBLISHED;
		}
	
		remove_client_request(&req_out);
		send_publish_from_queue();
		return true;
	}
	
	// Authentication
	if (r->must_authenticate()) {
		if (phone_user->authorize(req, r)) {
			phone_user->resend_request(req, req_out);
			return true;
		}

		// Authentication failed
		// Handle the 401/407 as a normal failure response
	}
	
	// PUBLISH failed
	
	if (is_unpublishing) {
		// Unpublish failed.
		// There is nothing we can do about that. Just clear
		// the internal publication.
		stop_timer(PUBLISH_TMR_PUBLICATION);
		etag.clear();
		is_unpublishing = false;
		epa_state = EPA_UNPUBLISHED;
		
		log_file->write_header("t_epa::recv_response", 
			LOG_NORMAL, LOG_WARNING);
		log_file->write_raw("Unpublish failed.\n");
		log_publication();
		log_file->write_footer();
		
		remove_client_request(&req_out);
		return true;
	}
	
	if (r->code == R_423_INTERVAL_TOO_BRIEF) {
		if (!r->hdr_min_expires.is_populated()) {
			// Violation of RFC 3261 10.3 item 7		
			log_file->write_report("Min-Expires header missing from 423 response.",
				"t_epa::recv_response",
				LOG_NORMAL, LOG_WARNING);
		} else if (r->hdr_min_expires.time <= publication_expiry) {
			// Wrong Min-Expires time
			string s = "Min-Expires (";
			s += ulong2str(r->hdr_min_expires.time);
			s += ") is smaller than the requested ";
			s += "time (";
			s += ulong2str(publication_expiry);
			s += ")";
			log_file->write_report(s, "t_epa::recv_response",
				LOG_NORMAL, LOG_WARNING);
		} else {
			// Publish with the advised interval
			remove_client_request(&req_out);
			if (etag.empty()) {
				// Initial publication.
				publish(r->hdr_min_expires.time, cached_body);
			} else {
				publication_expiry = r->hdr_min_expires.time;
				refresh_publication();
			}
			
			return true;
		}
	} else if (r->code == R_412_CONDITIONAL_REQUEST_FAILED) {
		log_file->write_header("t_epa::recv_response");
		log_file->write_raw("SIP-ETag mismatch, retry with initial publication.\n");
		log_publication();
		log_file->write_endl();
		log_file->write_footer();
		
		// The state seems to be gone from the presence agent. Clear
		// the internal pubication state.
		remove_client_request(&req_out);
		etag.clear();
		epa_state = EPA_UNPUBLISHED;
		
		// Retry to publish state
		publish(publication_expiry, cached_body);
		return true;
	}
	
	remove_client_request(&req_out);
	epa_state = EPA_FAILED;
	failure_msg = int2str(r->code);
	failure_msg += ' ';
	failure_msg += r->reason;
	
	log_file->write_header("t_epa::recv_response", 
		LOG_NORMAL, LOG_WARNING);
	log_file->write_raw("PUBLISH failure response.\n");
	log_file->write_raw(r->code);
	log_file->write_raw(" " + r->reason + "\n");
	log_publication();
	log_file->write_footer();

	send_publish_from_queue();
	return true;
}

bool t_epa::match_response(t_response *r, t_tuid tuid) const {
	return (req_out && req_out->get_tuid() == tuid);
}

bool t_epa::timeout(t_publish_timer timer) {
	switch (timer) {
	case PUBLISH_TMR_PUBLICATION:
		id_publication_timeout = 0;
			
		log_file->write_header("t_epa::timeout");
		log_file->write_raw("Publication timed out.\n");
		log_publication();
		log_file->write_footer();
		
		refresh_publication();
		return true;
	default:
		assert(false);
	}

	return false;
}

bool t_epa::match_timer(t_publish_timer timer, t_object_id id_timer) const {
	return id_timer == id_publication_timeout;
}

void t_epa::publish(unsigned long expires, t_sip_body *body) {
	t_request *r = create_publish(expires, body);
	
	if (req_out) {
		// A PUBLISH request is pending, queue this one.
		// Only 1 PUBLISH at a time may be sent.
		// RFC 3903 4
		enqueue_request(r);
		return;
	}
	
	// If the body equals the cached body, then do not
	// delete the cached_body as that will delete the body!
	if (cached_body && body && body != cached_body) {
		MEMMAN_DELETE(cached_body);
		delete cached_body;
		cached_body = NULL;
	}
	
	if (body) {
		cached_body = body->copy();
	}
	
	if (expires > 0) {
		publication_expiry = expires;
	} else {
		publication_expiry = default_duration;
	}
	
	is_unpublishing = false;
	
	stop_timer(PUBLISH_TMR_PUBLICATION);
	
	req_out = new t_client_request(phone_user->get_user_profile(), r, 0);
	MEMMAN_NEW(req_out);
	send_request(r, req_out->get_tuid());
	MEMMAN_DELETE(r);
	delete r;
}

void t_epa::unpublish(void) {
	if (!req_out && epa_state != EPA_PUBLISHED) {
		log_file->write_header("t_epa::unpublish", LOG_NORMAL, LOG_DEBUG);
		log_file->write_raw("Nothing published, discard unpublish\n");
		log_publication();
		log_file->write_footer();
		return;
	}

	t_request *r = create_publish(0, NULL);
	
	if (req_out) {
		// A PUBLISH request is pending, queue this one.
		// Only 1 PUBLISH at a time may be sent.
		// RFC 3903 4
		enqueue_request(r);
		return;
	}
	
	if (cached_body) {
		MEMMAN_DELETE(cached_body);
		delete cached_body;
		cached_body = NULL;
	}
		
	is_unpublishing = true;
	
	stop_timer(PUBLISH_TMR_PUBLICATION);
	
	req_out = new t_client_request(phone_user->get_user_profile(), r, 0);
	MEMMAN_NEW(req_out);
	send_request(r, req_out->get_tuid());
	MEMMAN_DELETE(r);
	delete r;	
}

void t_epa::refresh_publication(void) {
	publish(publication_expiry, NULL);
}

void t_epa::clear(void) {
	if (req_out) remove_client_request(&req_out);
	if (id_publication_timeout) stop_timer(PUBLISH_TMR_PUBLICATION);
	
	if (cached_body) {
		MEMMAN_DELETE(cached_body);
		delete cached_body;
		cached_body = NULL;
	}

	// Cleanup list of unsent PUBLISH messages
	while (!queue_publish.empty()) {
		t_request *r = queue_publish.front();
		queue_publish.pop();
		MEMMAN_DELETE(r);
		delete r;
	}
}
