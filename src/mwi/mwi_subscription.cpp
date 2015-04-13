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

#include "mwi_subscription.h"

#include <cassert>

#include "userintf.h"
#include "audits/memman.h"
#include "parser/hdr_event.h"

t_request *t_mwi_subscription::create_subscribe(unsigned long expires) const {
	t_request *r = t_subscription::create_subscribe(expires);
	SET_MWI_HDR_ACCEPT(r->hdr_accept);
	
	return r;
}

t_mwi_subscription::t_mwi_subscription(t_mwi_dialog *_dialog, t_mwi *_mwi) :
	t_subscription(_dialog, SR_SUBSCRIBER, SIP_EVENT_MSG_SUMMARY),
	mwi(_mwi)
{}

bool t_mwi_subscription::recv_notify(t_request *r, t_tuid tuid, t_tid tid) {
	if (t_subscription::recv_notify(r, tuid, tid)) return true;
	
	bool unsupported_body = false;
	
	// NOTE: if the subscription is still pending (RFC 3265 3.2.4), then the
	//       information in the body has no meaning.
	if (r->body && r->body->get_type() == BODY_SIMPLE_MSG_SUM &&
	    !is_pending()) 
	{
		t_simple_msg_sum_body *body = dynamic_cast<t_simple_msg_sum_body *>(r->body);
		assert(body);
		mwi->set_msg_waiting(body->get_msg_waiting());
		
		t_msg_summary summary;
		if (body->get_msg_summary(MSG_CONTEXT_VOICE, summary)) {
			mwi->set_voice_msg_summary(summary);
		} else {
			mwi->clear_voice_msg_summary();
		}
		
		mwi->set_status(t_mwi::MWI_KNOWN);
	}
	
	// Verify if there is an usupported body.
	if (r->body && r->body->get_type() != BODY_SIMPLE_MSG_SUM) {
		unsupported_body = true;
	}
	
	if (state == SS_TERMINATED && !may_resubscribe) {
		// The MWI server ended the subscription and indicated
		// that resubscription is not possible. So no MWI status
		// can be retrieved anymore. This should not happen, so
		// present it as a failure to the user.
		mwi->set_status(t_mwi::MWI_FAILED);
		ui->cb_mwi_terminated(user_config, get_reason_termination());
	}
	
	t_response *resp;
	if (unsupported_body) {
		resp = r->create_response(R_415_UNSUPPORTED_MEDIA_TYPE);
		SET_MWI_HDR_ACCEPT(r->hdr_accept);
	} else {
		resp = r->create_response(R_200_OK);
	}
	send_response(user_config, resp, 0, tid);
	MEMMAN_DELETE(resp);
	delete resp;
	
	ui->cb_update_mwi();
	return true;
}

bool t_mwi_subscription::recv_subscribe_response(t_response *r, t_tuid tuid, t_tid tid) {
	// Parent handles the SUBSCRIBE response
	(void)t_subscription::recv_subscribe_response(r, tuid, tid);
	
	// If the subscription is terminated after the SUBSCRIBE response, it means
	// that subscription failed.
	if (state == SS_TERMINATED) {
		ui->cb_mwi_subscribe_failed(user_config, r, mwi->get_status() != t_mwi::MWI_FAILED);
		mwi->set_status(t_mwi::MWI_FAILED);
	}
	
	ui->cb_update_mwi();
	return true;
}
