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

/**
 * @file
 * Subscription dialog (RFC 3265)
 */

#ifndef _SUBSCRIPTION_DIALOG_H
#define _SUBSCRIPTION_DIALOG_H

#include "abstract_dialog.h"
#include "subscription.h"

// Forward declaration
class t_phone_user;

/**
 * RFC 3265
 * Generic subscription dialog state for subscribers and notifiers.
 * For each event type this class should be subclassed.
 */
class t_subscription_dialog : public t_abstract_dialog {
protected:
	/**
	 * The subscription belonging to this dialog. Subclasses must
	 * create the proper subscription.
	 */
	t_subscription		*subscription;
	
	/**
	 * Constructor. This class must be subclassed. The subclass must provide
	 * a public constructor.
	 */
	t_subscription_dialog(t_phone_user *_phone_user);

	virtual void send_request(t_request *r, t_tuid tuid);
	
	/**
	 * Process a received SUBSCRIBE request.
	 * @param r [in] The request.
	 * @param tuid [in] Transaction user id.
	 * @param tid [in] Transaction id.
	 */
	virtual void process_subscribe(t_request *r, t_tuid tuid, t_tid tid);
	
	/**
	 * Process a received NOTIFY request.
	 * @param r [in] The request.
	 * @param tuid [in] Transaction user id.
	 * @param tid [in] Transaction id.
	 */
	virtual void process_notify(t_request *r, t_tuid tuid, t_tid tid);
	
	/**
	 * Process the response to the initial SUBSCRIBE.
	 * @param r [in] The response.
	 * @param tuid [in] Transaction user id.
	 * @param tid [in] Transaction id.
	 * @return true, if no further processing is needed. This happens, when a
	 * 423 Interval too brief response is received. Then this method sends a
	 * new SUBSCRIBE.
	 * @return false, subcalss must do further processing.
	 */
	virtual bool process_initial_subscribe_response(t_response *r, t_tuid tuid, t_tid tid);

public:
	/** Destructor. */
	virtual ~t_subscription_dialog();
	
	virtual t_request *create_request(t_method m);
	
	virtual t_subscription_dialog *copy(void) = 0;
	
	virtual bool resend_request_auth(t_response *resp);

	virtual bool redirect_request(t_response *resp);
	
	virtual bool failover_request(t_response *resp);

	virtual void recvd_response(t_response *r, t_tuid tuid, t_tid tid);
	
	virtual void recvd_request(t_request *r, t_tuid tuid, t_tid tid);
	
	/**
	 * Match request with dialog and subscription.
	 * @param r [in] The request.
	 * @param partial [out] Indicates if there is a partial match on return.
	 * @return true, if the request matches.
	 * @return false, if the request does not match. In this case the request
	 * may match partially, i.e. the from-tag matches, but the to-tag does not.
	 * In case of a partial match, partial is set to true.
	 */
	virtual bool match_request(t_request *r, bool &partial);
	
	/**
	 * Get the state of the subscription.
	 * @return The subscription state.
	 */
	t_subscription_state get_subscription_state(void) const;
	
	/**
	 * Get the reason for termination of the subscription.
	 * @return The termination reason.
	 */
	string get_reason_termination(void) const;
	
	/**
	 * Get the time after which a resubscription may be tried.
	 * @return The time in seconds.
	 */
	unsigned long get_resubscribe_after(void) const;
	
	/**
	 * Check if a resubscription may be tried.
	 * @return true, if a resubscription may be tried.
	 * @return false, otherwise.
	 */
	bool get_may_resubscribe(void) const;
	
	/**
	 * Process timeout.
	 * @param timer [in] The timer that expired.
	 * @return true, if processing is finished.
	 * @return false, if subsclass needs to do further processing.
	 */
	virtual bool timeout(t_subscribe_timer timer);
	
	/**
	 * Match a timer id with a running timer.
	 * @param timer [in] The running timer.
	 * @param id_timer [in] The timer id.
	 * @return true, if timer id matches with timer.
	 * @return false, otherwise.
	 */
	virtual bool match_timer(t_subscribe_timer timer, t_object_id id_timer) const;
	
	/**
	 * Subscribe to an event (send SUBSCRIBE).
	 * @param epxires [in] The subscription interval in seconds.
	 * @param req_uri [in] The request-URI for the SUBSCRIBE.
	 * @param to_uri [in] The URI for the To header in the SUBSCRIBE.
	 * @param to_display [in] The display name for the To header in the SUBSCRIBE.
	 */
	virtual void subscribe(unsigned long expires, const t_url &req_uri, 
			const t_url &to_uri, const string &to_display);
			
	/** Unsubscribe to an event (send SUBSCRIBE). */
	virtual void unsubscribe(void);
	
	/** Refresh subscription. */
	virtual void refresh_subscribe(void);
};

#endif
