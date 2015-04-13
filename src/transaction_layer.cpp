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

#include <assert.h>
#include <iostream>
#include "events.h"
#include "transaction_layer.h"
#include "userintf.h"
#include "util.h"
#include "audits/memman.h"

extern t_event_queue	*evq_trans_mgr;
extern t_event_queue	*evq_trans_layer;
extern bool		end_app;

void t_transaction_layer::recvd_response(t_response *r, t_tuid tuid,
		t_tid tid)
{
	lock();

	switch(r->get_class()) {
	case R_1XX:
		recvd_provisional(r, tuid, tid);
		break;
	case R_2XX:
		recvd_success(r, tuid, tid);
		break;
	case R_3XX:
		recvd_redirect(r, tuid, tid);
		break;
	case R_4XX:
		recvd_client_error(r, tuid, tid);
		break;
	case R_5XX:
		recvd_server_error(r, tuid, tid);
		break;
	case R_6XX:
		recvd_global_error(r, tuid, tid);
		break;
	default:
		assert(false);
		break;
	}
	
	post_process_response(r, tuid, tid);

	unlock();
}

void t_transaction_layer::recvd_request(t_request *r, t_tid tid,
		t_tid tid_cancel_target)
{
	bool fatal;
	string reason;
	t_response *resp;

	lock();

	// Return a 400 response if the SIP headers are wrong
	if (!r->is_valid(fatal, reason)) {
		resp = r->create_response(R_400_BAD_REQUEST, reason);
		send_response(resp, 0, tid);
		MEMMAN_DELETE(resp);
		delete resp;
		unlock();
		return;
	}

	// Return a 400 response if the SIP body contained a parse error
	if (r->body && r->body->invalid) {
		resp = r->create_response(R_400_BAD_REQUEST, "Invalid SIP body.");
		send_response(resp, 0, tid);
		MEMMAN_DELETE(resp);
		delete resp;
		unlock();
		return;
	}
	
	// If a message exceeded the maximum message size, than the body
	// is not parsed by the listener.
	if (r->hdr_content_length.is_populated() &&
	    r->hdr_content_length.length > 0 &&
	    !r->body)
	{
		resp = r->create_response(R_513_MESSAGE_TOO_LARGE);
		send_response(resp, 0, tid);
		MEMMAN_DELETE(resp);
		delete resp;
		unlock();
		return;
	}
	
	// RFC 3261 8.2.3
	// Return a 415 response if content encoding is not supported
	if (r->body && r->hdr_content_encoding.is_populated()) {
		for (list<t_coding>::iterator it = r->hdr_content_encoding.coding_list.begin();
		     it != r->hdr_content_encoding.coding_list.end(); ++it)
		{
			if (!CONTENT_ENCODING_SUPPORTED(it->content_coding)) {
				resp = r->create_response(R_415_UNSUPPORTED_MEDIA_TYPE);
				SET_HDR_ACCEPT_ENCODING(resp->hdr_accept_encoding);
				send_response(resp, 0, tid);
				MEMMAN_DELETE(resp);
				delete resp;
				unlock();
				return;
			}		
		}
	}

	// Check if URI scheme is supported
	if (r->uri.get_scheme() != "sip") {
		resp = r->create_response(R_416_UNSUPPORTED_URI_SCHEME);
		send_response(resp, 0, tid);
		MEMMAN_DELETE(resp);
		delete resp;
		unlock();
		return;
	}

	switch(r->method) {
	case INVITE:
		recvd_invite(r, tid);
		break;
	case ACK:
		recvd_ack(r, tid);
		break;
	case CANCEL:
		recvd_cancel(r, tid, tid_cancel_target);
		break;
	case BYE:
		recvd_bye(r, tid);
		break;
	case OPTIONS:
		recvd_options(r, tid);
		break;
	case REGISTER:
		recvd_register(r, tid);
		break;
	case PRACK:
		recvd_prack(r, tid);
		break;
	case SUBSCRIBE:
		recvd_subscribe(r, tid);
		break;
	case NOTIFY:
		recvd_notify(r, tid);
		break;
	case REFER:
		recvd_refer(r, tid);
		break;
	case INFO:
		recvd_info(r, tid);
		break;
	case MESSAGE:
		recvd_message(r, tid);
		break;
	default:
		resp = r->create_response(R_501_NOT_IMPLEMENTED);
		send_response(resp, 0, tid);
		MEMMAN_DELETE(resp);
		delete resp;
		break;
	}
	
	post_process_request(r, tid, tid_cancel_target);

	unlock();
}

void t_transaction_layer::recvd_async_response(t_event_async_response *event) {
	lock();
	
	switch (event->get_response_type()) {
	case t_event_async_response::RESP_REFER_PERMISSION:
		recvd_refer_permission(event->get_bool_response());
		break;
	default:
		// Ignore other responses
		break;
	}
	
	unlock();
}

void t_transaction_layer::send_request(t_user *user_config, t_request *r, t_tuid tuid) {
	evq_trans_mgr->push_user(user_config, (t_sip_message *)r, tuid, 0);
}

void t_transaction_layer::send_request(t_user *user_config, StunMessage *r, t_tuid tuid) {
	// The transaction manager will determine the destination IP and port,
	// so they can be left to zero in the event.
	evq_trans_mgr->push_stun_request(user_config, r, TYPE_STUN_SIP, tuid, 0, 0, 0);
}

void t_transaction_layer::send_response(t_response *r, t_tuid tuid,
		t_tid tid)
{
	evq_trans_mgr->push_user((t_sip_message *)r, tuid, tid);
}

void t_transaction_layer::run(void) {
	t_event			*event;
	t_event_user		*ev_user;
	t_event_timeout		*ev_timeout;
	t_event_failure		*ev_failure;
	t_event_stun_response	*ev_stun_resp;
	t_event_async_response	*ev_async_resp;
	t_event_broken_connection *ev_broken_connection;
	t_sip_message		*msg;
	StunMessage		*stun_msg;
	t_tid			tid;
	t_tid			tid_cancel;
	t_tuid			tuid;

	bool quit = false;
	while (!quit) {
		event = evq_trans_layer->pop();

		switch (event->get_type()) {
		case EV_USER:
			ev_user = (t_event_user *)event;
			tid = ev_user->get_tid();
			tuid = ev_user->get_tuid();
			tid_cancel = ev_user->get_tid_cancel_target();
			msg = ev_user->get_msg();

			switch(msg->get_type()) {
			case MSG_REQUEST:
				recvd_request((t_request *)msg, tid,
						tid_cancel);
				break;
			case MSG_RESPONSE:
				recvd_response((t_response *)msg, tuid, tid);
				break;
			default:
				assert(false);
				break;
			}

			break;
		case EV_TIMEOUT:
			ev_timeout = dynamic_cast<t_event_timeout *>(event);
			handle_event_timeout(ev_timeout);
			break;
		case EV_FAILURE:
			ev_failure = (t_event_failure *)event;
			tid = ev_failure->get_tid();
			lock();
			failure(ev_failure->get_failure(), tid);
			unlock();
			break;
		case EV_STUN_RESPONSE:
			ev_stun_resp = (t_event_stun_response *)event;
			tid = ev_stun_resp->get_tid();
			tuid = ev_stun_resp->get_tuid();
			stun_msg = ev_stun_resp->get_msg();
			recvd_stun_resp(stun_msg, tuid, tid);
			break;
		case EV_ASYNC_RESPONSE:
			ev_async_resp = dynamic_cast<t_event_async_response *>(event);
			recvd_async_response(ev_async_resp);
			break;
		case EV_BROKEN_CONNECTION:
			ev_broken_connection = dynamic_cast<t_event_broken_connection *>(event);
			handle_broken_connection(ev_broken_connection);
			break;
		case EV_QUIT:
			quit = true;
			break;
		default:
			// other types of event are not expected
			assert(false);
			break;
		}

		MEMMAN_DELETE(event);
		delete event;
	}
}

void t_transaction_layer::lock(void) const {
	// Prohibited threads may not lock the transaction layer
	assert(!is_prohibited_thread());

	// The user interface and transaction layer threads both call
	// functions on the transaction layer. By locking the UI mutex
	// first, a deadlock can never occur as the UI also takes the
	// UI lock first and then the transaction layer lock.
	// During shutdown of Twinkle the GUI has exited already and
	// a lock on an exited QApplication causes a segmentation fault.
	// Therefore the lock on the UI should not be taken during shutdown.
	if (!end_app) ui->lock();
	tl_mutex.lock();
}

void t_transaction_layer::unlock(void) const {
	tl_mutex.unlock();
	if (!end_app) ui->unlock();
}
