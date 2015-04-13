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
 * Subscription (RFC 3265)
 */

#ifndef _SUBSCRIPTION_H
#define _SUBSCRIPTION_H

#include <queue>
#include <string>
#include "abstract_dialog.h"

/** Subscription role */
enum t_subscription_role {
	SR_SUBSCRIBER,		/**< Subscriber */
	SR_NOTIFIER		/**< Notifier */
};

/** Subscription state */
enum t_subscription_state {
	SS_NULL,		/**< Initial state */
	SS_ESTABLISHED,		/**< Subscription is in place */
	SS_UNSUBSCRIBING,	/**< A request to unsubscribe has been sent */
	SS_UNSUBSCRIBED,	/**< An outoging unsubscribe was succesful. Waiting for the final NOTIFY. */
	SS_TERMINATED,		/**< Subscription ended */
};

/**
 * Convert a subscription state to string.
 * @param state [in] Subscription state.
 * @return String representation of state.
 */
string t_subscription_state2str(t_subscription_state state);

/**
 * RFC 3265
 * Generic subscription state for subscribers and notifiers
 * For each event type this class should be subclassed.
 */
class t_subscription {
protected:
	t_subscription_role	role;
	t_subscription_state	state;
	
	/**
	 *  When a subscriber subscription is terminated, this reason indicates
	 * the reason conveyed in the NOTIFY, if any.
	 */
	string			reason_termination;
	
	/**
	 * If the NOTIFY indicated that the subscriber may retry subscription at
	 * a later time, then resubscribe_after indicates the number of seconds to wait.
	 */
	unsigned long		resubscribe_after;
	
	/** Indicates if a re-subscribe may be done after a failure. */
	bool			may_resubscribe;
	
	t_abstract_dialog	*dialog; /**< Dialog owning the subscription */
	string			event_type;
	string			event_id;
	
	/**
	 * User profile of user using the line
	 * This is a pointer to the user_config owned by a phone user.
	 * So this pointer should never be deleted.
	 */
	t_user			*user_config;

	bool			pending; /**< Indicates if not active yet. */

	/** @name Timers */
	//@{
	/**
	 * For a subscriber the subscription_timeout timer indicates when
	 * the subscription must be refreshed.
	 * For a notifier it indicates when the subscription expires.
	 */
	unsigned short		id_subscription_timeout;

	/**
	 * Indicates if a subscriber automatically refreshes the subscritption
	 * when the subscription timer expires. If not, then the subscription
	 * terminates at expiry.
	 */
	bool			auto_refresh;
	
	/** Subcription expiry for a SUBSCRIBE request */
	unsigned long		subscription_expiry;
	
	/** Default duration for a subscription */
	unsigned long		default_duration;
	//@}

	/** Protect constructor from being used */
	t_subscription() {};
	
	/** Write event type and id to log file */
	void log_event() const;

	/**
	 * Remove a pending request. Pass one of the client request pointers.
	 * @param cr [in] Client request to remove.
	 */
	void remove_client_request(t_client_request **cr);

	/** @name Create requests based on the event type */
	//@{
	/**
	 * Create a SUBSCRIBE request.
	 * Creating a SUBSCRIBE is for subscription refreshment/unsubscribe.
	 * @param expires [in] Expiry time in seconds.
	 */
	virtual t_request *create_subscribe(unsigned long expires) const;
	
	/**
	 * Create a NOTIFY request.
	 * @param sub_state [in] Subscription state to be put in the Subscription-State header.
	 * @param reason [in] The reason parameter of the Subscription-State header.
	 */
	virtual t_request *create_notify(const string &sub_state,
		const string &reason = "") const;
	//@}

	/**
	 * Send request.
	 * @param user_config [in] User profile of user sending the request.
	 * @param r [in] Request to send.
	 * @param tuid [in] Transaction user id.
	 */
	void send_request(t_user *user_config, t_request *r, t_tuid tuid) const;
	
	/**
	 * Send response.
	 * @param user_config [in] User profile of user sending the response.
	 * @param r [in] Response to send.
	 * @param tuid [in] Transaction user id.
	 * @param tid [in] Transaction id.
	 */
	void send_response(t_user *user_config, t_response *r, t_tuid tuid, t_tid tid) const;

	/** 
	 * Start a subscription timer.
	 * @param timer [in] Type of subscription timer.
	 * @param duration [in] Duration of timer in ms
	 */
	void start_timer(t_subscribe_timer timer, long duration);
	
	/**
	 * Stop a subscription timer.
	 * @param timer [in] Type of subscription timer.
	 */
	void stop_timer(t_subscribe_timer timer);

public:
	/** Pending request */
	t_client_request	*req_out;

	/**
	 * Queue of pending outgoing NOTIFY requests. A next NOTIFY
	 * will only be sent after the previous NOTIFY has been
	 * answered.
	 */
	queue<t_request *>	queue_notify;

	/**
	 * Constructor
	 * @param _dialog [in] Dialog owning this subscription. SUBSCRIBE and NOTIFY
	 * requests are sent within this dialog.
	 * @param _role [in] Role
	 * @param _event_type [in] Event type of the subscription.
	 */
	t_subscription(t_abstract_dialog *_dialog, t_subscription_role _role,
			const string &_event_type);
			
	/**
	 * Constructor
	 * @param _dialog [in] Dialog owning this subscription. SUBSCRIBE and NOTIFY
	 * requests are sent within this dialog.
	 * @param _role [in] Role
	 * @param _event_type [in] Event type of the subscription.
	 * @param _event_id [in] Event id.
	 */
	t_subscription(t_abstract_dialog *_dialog, t_subscription_role _role,
			const string &_event_type, const string &_event_id);
			
	/** Destructor */
	virtual ~t_subscription();

	/** @name Getters */
	//@{
	t_subscription_role get_role(void) const;
	t_subscription_state get_state(void) const;
	string get_reason_termination(void) const;
	unsigned long get_resubscribe_after(void) const;
	bool get_may_resubscribe(void) const;
	string get_event_type(void) const;
	string get_event_id(void) const;
	unsigned long get_expiry(void) const;
	//@}

	/** @name Receive requests */
	//@{
	/**
	 * Reveive SUBSCRIBE request
	 * @param r [in] Received request.
	 * @param tuid [in] Transaction user id.
	 * @param tid [in] Transaction id.
	 * @return The return value indicates if processing is finished.
	 * This way a subclass can first call the parent class method.
	 * If the parent indicates that process is finished, then the child
	 * does not need to further process.
	 * Note that recv_subscribe returns false if the SUBSCRIBE is valid. The
	 * subscription timer will be started, but no response is sent. The subclass
	 * MUST further handle the SUBSCRIBE, i.e. send a response and a NOTIFY.
	 */
	virtual bool recv_subscribe(t_request *r, t_tuid tuid, t_tid tid);
	
	/**
	 * Receive NOTIFY request.
	 * @param r [in] Received request.
	 * @param tuid [in] Transaction user id.
	 * @param tid [in] Transaction id.
	 * @return When the NOTIFY is valid, false is returned. The subclass MUST further
	 * handle the NOTIFY, i.e. send a response.
	 */
	virtual bool recv_notify(t_request *r, t_tuid tuid, t_tid tid);
	//@}

	/** @name Receive responses */
	//@{
	/**
	 * Receive NOTIFY/SUBSCRIBE response.
	 * @param r [in] Received response.
	 * @param tuid [in] Transaction user id.
	 * @param tid [in] Transaction id.
	 * @return The return value indicates if processing is finished.
	 */
	virtual bool recv_response(t_response *r, t_tuid tuid, t_tid tid);
	
	/**
	 * Receive NOTIFY response.
	 * @param r [in] Received response.
	 * @param tuid [in] Transaction user id.
	 * @param tid [in] Transaction id.
	 * @return The return value indicates if processing is finished.
	 */
	virtual bool recv_notify_response(t_response *r, t_tuid tuid, t_tid tid);
	
	/**
	 * Receive SUBSCRIBE response.
	 * @param r [in] Received response.
	 * @param tuid [in] Transaction user id.
	 * @param tid [in] Transaction id.
	 * @return The return value indicates if processing is finished.
	 */
	virtual bool recv_subscribe_response(t_response *r, t_tuid tuid, t_tid tid);
	//@}

	/**
	 * Process timeouts
	 * @param timer [in] Type of subscription timer.
	 * @return The return value indicates if processing is finished.
	 */
	virtual bool timeout(t_subscribe_timer timer);
	
	/**
	 * Match timer id with a running timer.
	 * @param timer [in] Type of subscription timer.
	 * @return True, if id matches, otherwise false.
	 */
	virtual bool match_timer(t_subscribe_timer timer, t_object_id id_timer) const;
	
	/**
	 * Does incoming request match with event type and id?
	 * @param r [in] Request to match.
	 * @return True if request matches, otherwise false.
	 */
	virtual bool match(t_request *r) const;

	/**
	 * Check if subscription is pending.
	 * @return True if subscription is pending, otherwise false.
	 */
	bool is_pending(void) const;

	/**
	 * Subscribe to an event. 
	 * @param expires [in] Expiry in seconds. If expires == 0, then the default duration is used.
	 */
	virtual void subscribe(unsigned long expires);
	
	/** Unsubscribe from an event. */
	virtual void unsubscribe(void);
	
	/** Refresh subscription. */
	virtual void refresh_subscribe(void);
};

#endif
