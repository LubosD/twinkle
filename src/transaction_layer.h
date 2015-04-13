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

#ifndef _TRANSACTION_LAYER_H
#define _TRANSACTION_LAYER_H

#include "events.h"
#include "prohibit_thread.h"
#include "transaction.h"
#include "parser/request.h"
#include "parser/response.h"
#include "stun/stun.h"
#include "threads/mutex.h"

typedef unsigned short	t_tuid;

class t_transaction_layer : public i_prohibit_thread {
private:
	// Mutex to guarantee that only 1 thread at a time is
	// accessing the transaction layer.
	mutable t_recursive_mutex	tl_mutex;

	void recvd_response(t_response *r, t_tuid tuid, t_tid tid);
	void recvd_request(t_request *r, t_tid tid, t_tid tid_cancel_target);
	void recvd_async_response(t_event_async_response *event);

protected:
	// Client event handlers
	// After returning from this function, the response pointer
	// will be deleted.
	virtual void recvd_provisional(t_response *r, t_tuid tuid,
			t_tid tid) = 0;
	virtual void recvd_success(t_response *r, t_tuid tuid, t_tid tid) = 0;
	virtual void recvd_redirect(t_response *r, t_tuid tuid,
			t_tid tid) = 0;
	virtual void recvd_client_error(t_response *r, t_tuid tuid,
			t_tid tid) = 0;
	virtual void recvd_server_error(t_response *r, t_tuid tuid,
			t_tid tid) = 0;
	virtual void recvd_global_error(t_response *r, t_tuid tuid,
			t_tid tid) = 0;
			
	// General post processing for all responses
	virtual void post_process_response(t_response *r, t_tuid tuid,
			t_tid tid) = 0;

	// Server event handlers
	// After returning from this function, the request pointer
	// will be deleted.
	virtual void recvd_invite(t_request *r, t_tid tid) = 0;
	virtual void recvd_ack(t_request *r, t_tid tid) = 0;
	virtual void recvd_cancel(t_request *r, t_tid cancel_tid,
				t_tid target_tid) = 0;
	virtual void recvd_bye(t_request *r, t_tid tid) = 0;
	virtual void recvd_options(t_request *r, t_tid tid) = 0;
	virtual void recvd_register(t_request *r, t_tid tid) = 0;
	virtual void recvd_prack(t_request *r, t_tid tid) = 0;
	virtual void recvd_subscribe(t_request *r, t_tid tid) = 0;
	virtual void recvd_notify(t_request *r, t_tid tid) = 0;
	virtual void recvd_refer(t_request *r, t_tid tid) = 0;
	virtual void recvd_info(t_request *r, t_tid tid) = 0;
	virtual void recvd_message(t_request *r, t_tid tid) = 0;
	
	// General post processing for all requests
	virtual void post_process_request(t_request *r, t_tid cancel_tid,
				t_tid target_tid) = 0;

	// The transaction failed and is aborted
	virtual void failure(t_failure failure, t_tid tid) = 0;
	
	// STUN event handler
	virtual void recvd_stun_resp(StunMessage *r, t_tuid tuid, t_tid tid) = 0;
	
	// The user has granted or rejected an incoming REFER request.
	virtual void recvd_refer_permission(bool permission) = 0;
	
	/**
	 * Handle timeout event.
	 * @param e [in] Timeout event.
	 */
	virtual void handle_event_timeout(t_event_timeout *e) = 0;
	
	/**
	 * Handle broken connection event.
	 * @param e [in] Broken connection event.
	 */
	virtual void handle_broken_connection(t_event_broken_connection *e) = 0;

public:
	virtual ~t_transaction_layer() {};

	// Client primitives
	void send_request(t_user *user_config, t_request *r, t_tuid tuid);
	void send_request(t_user *user_config, StunMessage *r, t_tuid tuid);

	// Server primitives
	void send_response(t_response *r, t_tuid tuid, t_tid tid);

	// Main loop
	void run(void);

	// Lock and unlocking methods for dedicated access to the
	// transaction layer.
	void lock(void) const;
	void unlock(void) const;
};

#endif
