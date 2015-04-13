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

#include "presence_subscription.h"

#include <cassert>

#include "pidf_body.h"

#include "log.h"
#include "util.h"
#include "parser/hdr_event.h"
#include "audits/memman.h"

t_request *t_presence_subscription::create_subscribe(unsigned long expires) const {
	t_request *r = t_subscription::create_subscribe(expires);
	SET_PRESENCE_HDR_ACCEPT(r->hdr_accept);
	
	return r;
}

t_presence_subscription::t_presence_subscription(t_presence_dialog *_dialog, t_presence_state *_state) :
	t_subscription(_dialog, SR_SUBSCRIBER, SIP_EVENT_PRESENCE),
	presence_state(_state)
{
}

bool t_presence_subscription::recv_notify(t_request *r, t_tuid tuid, t_tid tid) {
	if (t_subscription::recv_notify(r, tuid, tid)) return true;

	bool unsupported_body = false;
	
	// NOTE: if the subscription is still pending (RFC 3265 3.2.4), then the
	//       information in the body has no meaning.
	// NOTE: a NOTIFY request may have no body (RFC 3856 6.6.2)
	if (r->body && r->body->get_type() == BODY_PIDF_XML &&
	    !is_pending()) 
	{
		t_pidf_xml_body *body = dynamic_cast<t_pidf_xml_body *>(r->body);
		assert(body);
		
		string basic = body->get_basic_status();
		if (basic == PIDF_STATUS_BASIC_OPEN) {
			presence_state->set_basic_state(t_presence_state::ST_BASIC_OPEN);
		} else if (basic == PIDF_STATUS_BASIC_CLOSED) {
			presence_state->set_basic_state(t_presence_state::ST_BASIC_CLOSED);
		} else {
			log_file->write_header("t_presence_subscription::recv_notify",
				LOG_NORMAL, LOG_WARNING);
			log_file->write_raw("Unknown basic status in pidf: ");
			log_file->write_raw(basic);
			log_file->write_endl();
			log_file->write_footer();
			
			presence_state->set_basic_state(t_presence_state::ST_BASIC_UNKNOWN);
		}
	}
	
	// Verify if there is an usupported body.
	if (r->body && r->body->get_type() != BODY_PIDF_XML) {
		unsupported_body = true;
	}

	if (state == SS_TERMINATED) {
		if (may_resubscribe) {
			presence_state->set_basic_state(t_presence_state::ST_BASIC_UNKNOWN);
			log_file->write_report("Presence subscription terminated.",
				"t_presence_subscription::recv_notify");
		} else {
			if (reason_termination == EV_REASON_REJECTED) {
				presence_state->set_basic_state(t_presence_state::ST_BASIC_REJECTED);
				
				log_file->write_report("Presence agent rejected the subscription.",
					"t_presence_subscription::recv_notify");
			} else {
				// The PA ended the subscription and indicated
				// that resubscription is not possible. So no presence status
				// can be retrieved anymore. This should not happen.
				// Show it as a failure to the user.
				presence_state->set_failure_msg(reason_termination);
				presence_state->set_basic_state(t_presence_state::ST_BASIC_FAILED);
				
				log_file->write_report(
					"Presence agent permanently terminated the subscription.",
					"t_presence_subscription::recv_notify");
			}
		}
	}
	
	t_response *resp;
	if (unsupported_body) {
		resp = r->create_response(R_415_UNSUPPORTED_MEDIA_TYPE);
		SET_PRESENCE_HDR_ACCEPT(r->hdr_accept);
	} else {
		resp = r->create_response(R_200_OK);
	}
	send_response(user_config, resp, 0, tid);
	MEMMAN_DELETE(resp);
	delete resp;
	
	return true;
}

bool t_presence_subscription::recv_subscribe_response(t_response *r, t_tuid tuid, t_tid tid) {
	// Parent handles the SUBSCRIBE response
	(void)t_subscription::recv_subscribe_response(r, tuid, tid);

	// If the subscription is terminated after the SUBSCRIBE response, it means
	// that subscription failed.
	if (state == SS_TERMINATED) {
		if (r->code == R_403_FORBIDDEN || r->code == R_603_DECLINE) {
			presence_state->set_basic_state(t_presence_state::ST_BASIC_REJECTED);
			
			log_file->write_report("Presence subscription rejected.",
				"t_presence_subscription::recv_subscribe_response");
		} else {
			string failure = int2str(r->code);
			failure += ' ';
			failure += r->reason;
			presence_state->set_failure_msg(failure);
			presence_state->set_basic_state(t_presence_state::ST_BASIC_FAILED);
			
			log_file->write_report("Presence subscription failed.",
				"t_presence_subscription::recv_subscribe_response");
		}
	}
	
	return true;
}
