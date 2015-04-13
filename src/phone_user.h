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

// State of active phone user

#ifndef _PHONE_USER_H
#define _PHONE_USER_H

#include <string>
#include <list>
#include "auth.h"
#include "protocol.h"
#include "service.h"
#include "transaction_layer.h"
#include "user.h"
#include "im/msg_session.h"
#include "mwi/mwi.h"
#include "mwi/mwi_dialog.h"
#include "parser/request.h"
#include "parser/response.h"
#include "stun/stun.h"
#include "presence/buddy.h"

using namespace std;
using namespace im;

// Forward declarations
class t_client_request;
class t_presence_epa;

class t_phone_user {
private:
	t_user			*user_config;

	// State
	// Indicates that this user is active. A non-active user
	// is not removed from the user list as some transactions may
	// still need the user info after the user got de-activated.
	bool			active;

	// Requests outside a dialog
	t_client_request	*r_options;
	t_client_request	*r_register;
	t_client_request	*r_deregister;
	t_client_request	*r_query_register;
	t_client_request	*r_message;
	
	// STUN request
	t_client_request	*r_stun;
	
	/**
	 * Pending MESSAGE requests.
	 * Twinkle will only send one message at a time per user.
	 * This satisfies the requirements in RFC 3428 8.
	 * NOTE: as an optimization a message queue per destination
	 * could be kept. This way a pending message for one destination
	 * will not block a message for another destination.
	 */
	list<t_request *> pending_messages;

	/** @name Registration data */
	//@{
	string			register_call_id;  /**< Call-ID for REGISTER requests. */
	unsigned long		register_seqnr;    /**< Last seqnr issued. */
	bool			is_registered;     /**< Indicates if user is registered. */
	unsigned long		registration_time; /**< Expiration in seconds */
	bool			last_reg_failed;   /** Indicates if last registration failed. */
	
	/** Destination of last REGISTER */
	t_ip_port		register_ip_port;
	
	/** Service Route, collected from REGISTER responses */
	list<t_route>		service_route;
	//@}
	
	// A STUN request can be triggered by the following events:
	//
	// * Registration
	// * MWI subscription.
	//
	// These events should take place after the STUN transaction has
	// finished. The following indicators indicate which events should
	// take place.
	bool		register_after_stun;
	bool		mwi_subscribe_after_stun;
	bool		stun_binding_inuse_registration;
	bool		stun_binding_inuse_mwi;
	
	// Authorizor
	t_auth			authorizor;
	
	// MWI dialog
	t_mwi_dialog		*mwi_dialog;
	
	/**
	 * Indicates if MWI must be automatically resubscribed to, if the
	 * subscription terminates with a reason telling that resubscription
	 * is possible.
	 */
	bool			mwi_auto_resubscribe;
	
	/** Buddy list for presence susbcriptions. */
	t_buddy_list		*buddy_list;
	
	/** Event publication agent for presence */
	t_presence_epa		*presence_epa;
	
	/**
	 * Resend the request: a new sequence number will be assigned and a new via
	 * header created (new transaction).
	 * @param req [in] The request to resend.
	 * @param is_register [in] indicates if this request is a register
	 * @param cr [in] is the current client request wrapper for this request.
	 * @note In case of a REGISTER, the internal register seqnr will be increased.
	 */
	void resend_request(t_request *req, bool is_register, t_client_request *cr);

	/** @name Handle responses. */
	//@{
	/**
	 * Process a repsonse on a registration request. 
	 * @param r [in] The response.
	 * @param re_register [out] Indicates if an automatic re-registration needs
	 * to be done.
	 */
	void handle_response_register(t_response *r, bool &re_register);
	
	/**
	 * Process a response on a de-registration request.
	 * @param r [in] The response.
	 */
	void handle_response_deregister(t_response *r);
	
	/**
	 * Process a response on a registration query request.
	 * @param r [in] The response.
	 */
	void handle_response_query_register(t_response *r);

	/**
	 * Process a response on an OPTIONS request.
	 * @param r [in] The response.
	 */
	void handle_response_options(t_response *r);
	
	/**
	 * Process a response on a MESSAGE request.
	 * @param r [in] The response.
	 */
	void handle_response_message(t_response *r);
	//@}
	
	/** Send a NAT keep alive packet. */
	void send_nat_keepalive(void);
	
	/** Send a TCP ping packet. */
	void send_tcp_ping(void);
	
	/** Handle MWI dialog termination. */
	void cleanup_mwi_dialog(void);
	
	/** Cleanup registration data for STUN and NAT keep alive. */
	void cleanup_registration_data(void);
	
public:
	/** @name Timers */
	//@{
	unsigned short		id_registration;	/**< Registration timeout */
	unsigned short		id_nat_keepalive;	/**< NAT keepalive interval */
	unsigned short		id_tcp_ping;		/**< TCP ping timer */
	unsigned short		id_resubscribe_mwi; 	/**< MWI re-subscribe after failure */
	//@}
	
	/** Supplementary services */
	t_service	*service;
	
	/** Message Waiting Indication data. */
	t_mwi		mwi;
	
	/** @name STUN data */
	//@{
	bool		use_stun; 		/**< Indicates if STUN must be used. */
	bool		use_nat_keepalive; 	/**< Send NAT keepalive ? */
	unsigned long	stun_public_ip_sip; 	/**< Public IP for SIP */
	unsigned short	stun_public_port_sip; 	/**< Public port for SIP */
	
	/** Number of presence subscriptions using the STUN binding. */
	unsigned short	stun_binding_inuse_presence;	
	
	/**< Subscribe to presence after STUN transaction completed. */
	bool		presence_subscribe_after_stun;	
	//@}
	
	/** 
	 * The constructor will create a copy of profile.
	 * @param profile [in] User profile of this phone user.
	 */
	t_phone_user(const t_user &profile);
	
	/** Destructor. */
	~t_phone_user();
	
	/** Send STUN request. */
	void send_stun_request(void);
	
	/** Cleanup STUN data if not in use anymore. */
	void cleanup_stun_data(void);
	
	/** Stop sending NAT keep alives when not necessary anymore. */
	void cleanup_nat_keepalive(void);
	
	/** 
	 * Synchronize the sending of NAT keep alives with the user config.
	 * Start sending if keep alives are enabled but currently not being
	 * sent.
	 */
	void sync_nat_keepalive(void);
	
	/** Stop sending TCP ping packets when not necessary anumore. */
	void cleanup_tcp_ping(void);
	
	/** @name Getters */
	//@{
	t_user *get_user_profile(void);
	t_buddy_list *get_buddy_list(void);
	t_presence_epa *get_presence_epa(void);
	//@}
	
	// Handle responses for out-of-dialog requests
	void handle_response_out_of_dialog(t_response *r, t_tuid tuid, t_tid tid);
	void handle_response_out_of_dialog(StunMessage *r, t_tuid tuid);
	
	/**
	 * Send a registration, de-registration or query registration request.
	 * @param register_type [in] Type of registration request.
	 * @param re_register [in] Indicates if this registration request is a re-registration.
	 * @param expires [in] Epxiry time to put in registration request.
	 * @note If needed a STUN request is sent before doing a registration.
	 */
	void registration(t_register_type register_type, bool re_register,
					unsigned long expires = 0);
					
	// OPTIONS outside dialog
	void options(const t_url &to_uri, const string &to_display = "");
	
	/** @name MWI */
	//@{
	/** Subscribe to MWI. */
	void subscribe_mwi(void);
	
	/** Unsusbcribe to MWI. */
	void unsubscribe_mwi(void);
	
	/**
	 * Check if an MWI subscription is established.
	 * @return true, if an MWI subscription is established.
	 * @return false, otherwise
	 */
	bool is_mwi_subscribed(void) const;
	
	/**
	 * Check if an MWI dialog does exist.
	 * @return true, if there is no MWI subscription dialog.
	 * @return false, otherwise
	 */
	bool is_mwi_terminated(void) const;
	
	/**
	 * Process an unsollicited NOTIFY for MWI.
	 * @param r [in] The NOTIFY request.
	 * @param tid [in] Transaction identifier of the NOTIFY transaction.
	 */
	void handle_mwi_unsollicited(t_request *r, t_tid tid);
	//@}
	
	/** @name Presence */
	//@{
	/** Subscribe to presence of buddies in buddy list. */
	void subscribe_presence(void);
	
	/** Unsusbcribe to presence of buddies in buddy list. */
	void unsubscribe_presence(void);
	
	/**
	 * Check if all presence subscriptions are terminated.
	 * @return true, if all presence subscriptions are terminated.
	 * @return false, otherwise
	 */
	bool is_presence_terminated(void) const;
	
	/**
	 * Publish presence.
	 * @param basic_state [in] The basic presence state to publish.
	 */
	void publish_presence(t_presence_state::t_basic_state basic_state);
	
	/** Unpublish presence. */
	void unpublish_presence(void);
	//@}
	
	/**
	 * Send a text message.
	 * @param to_uri [in] Destination URI of recipient.
	 * @param to_display [in] Display name of recipient.
	 * @param msg [in] The message to send.
	 * @return True if sending succeeded, otherwise false.
	 */
	bool send_message(const t_url &to_uri, const string &to_display, const t_msg &message);
	
	/**
	 * @param to_uri [in] Destination URI of recipient.
	 * @param to_display [in] Display name of recipient.
	 * @param state [in] Message composing state.
	 * @param refresh [in] The refresh interval in seconds (when state is active).
	 * @return True if sending succeeded, false otherwise.
	 * @note For the idle state, the value of refresh has no meaning.
	 */
	bool send_im_iscomposing(const t_url &to_uri, const string &to_display, 
			const string &state, time_t refresh);
	
	/**
	 * Process incoming MESSAGE request.
	 * @param r [in] The MESSAGE request.
	 * @param tid [in] Transaction id of the request transaction.
	 */
	void recvd_message(t_request *r, t_tid tid);
	
	/**
	 * Process incoming NOTIFY reqeust.
	 * @param r [in] The NOTIFY request.
	 * @param tid [in] Transaction id of the request transaction.
	 */
	void recvd_notify(t_request *r, t_tid tid);
	
	/** @name Process timeouts */
	//@{
	/** 
	 * Process phone timer expiry.
	 * @param timer [in] Type of expired phone timer.
	 */
	void timeout(t_phone_timer timer);
	
	/**
	 * Proces subscribe timer expiry.
	 * @param timer [in] Type of expired subscribe timer.
	 * @param id_timer [in] Id of expired timer.
	 */
	void timeout_sub(t_subscribe_timer timer, t_object_id id_timer);
	
	/**
	 * Proces publish timer expiry.
	 * @param timer [in] Type of expired subscribe timer.
	 * @param id_timer [in] Id of expired timer.
	 */
	void timeout_publish(t_publish_timer timer, t_object_id id_timer);
	//@}
	
	/** Handle a broken persistent connection. */
	void handle_broken_connection(void);
	
	/** Match subscribe timeout with a subcription
	 * @param timer [in] Type of expired subscribe timer.
	 * @param id_timer [in] Id of expired timer.
	 * @return True if timer matches a subscription owned by the phone user.
	 * @return False, otherwise.
	 */
	bool match_subscribe_timer(t_subscribe_timer timer, t_object_id id_timer) const;
	
	/** Match publish timeout with a subcription
	 * @param timer [in] Type of expired publish timer.
	 * @param id_timer [in] Id of expired timer.
	 * @return True if timer matches a publication owned by the phone user.
	 * @return False, otherwise.
	 */
	bool match_publish_timer(t_publish_timer timer, t_object_id id_timer) const;
	
	/**
	 * Start the re-subscribe timer after an MWI subscription failure.
	 * @param duration [in] Duration before trying a re-subscribe (s)
	 */
	void start_resubscribe_mwi_timer(unsigned long duration);
	
	/** Stop MWI re=subscribe timer. */
	void stop_resubscribe_mwi_timer(void);
	
	/**
	 * Create request. 
	 * Headers that are the same for each request
	 * are already populated: Via, From, Max-Forwards, User-Agent.
	 * All possible destinations for a failover are calculated.
	 * @param m [in] Request method.
	 * @param request_uri [in] Request-URI.
	 * @return The created request.
	 */
	t_request *create_request(t_method m, const t_url &request_uri) const;
	
	// Create a response to an OPTIONS request
	// Argument 'in-dialog' indicates if the OPTIONS response is
	// sent within a dialog.
	t_response *create_options_response(t_request *r,
					bool in_dialog = false) const;
	
	// Get registration status
	bool get_is_registered(void) const;
	bool get_last_reg_failed(void) const;
	
	/**
	 * Get local IP address for SIP.
	 * @param auto_ip [in] IP address to use if no IP address has been determined through
	 *                     some NAT procedure.
	 * @return The IP address.
	 */
	string get_ip_sip(const string &auto_ip) const;
	
	/**
	 * Get local port for SIP.
	 * @return SIP port.
	 */ 
	unsigned short get_public_port_sip(void) const;
	
	/** 
	 * Get the service route.
	 * @return The service route.
	 */
	list<t_route> get_service_route(void) const;

	// Try to match message with phone user
	bool match(t_response *r, t_tuid tuid) const;
	bool match(t_request *r) const;
	bool match(StunMessage *r, t_tuid tuid) const;
	
	/**
	 * Authorize the request based on the challenge in the response
	 * @param r [inout] The request to be authorized.
	 * @param resp [in] The response containing the challenge (401/407).
	 * @param True if authorization succeeds, false otherwise.
	 * @post On succesful return the request r contains the correct authorization
	 * header (based on 401/407 response).
	 */
	bool authorize(t_request *r, t_response *resp);
	
	/**
	 * Resend the request: a new sequence number will be assigned and a new via
	 * header created (new transaction).
	 * @param req [in] The request to resend.
	 * @param cr [in] is the current client request wrapper for this request.
	 * @note In case of a REGISTER, the internal register seqnr will be increased.
	 */
	void resend_request(t_request *req, t_client_request *cr);
	
	/**
	 * Remove cached credentials for a particular realm.
	 * @param realm [in] The realm.
	 */
	void remove_cached_credentials(const string &realm);
	
	/**
	 * Check if this phone user is active.
	 * @return True if phone user is active, false otherwise.
	 */
	bool is_active(void) const;
	
	/**
	 * Activate phone user.
	 * @param user [in] The user profile of the user.
	 * @note The passed user profile will replace the current user profile
	 * owned by phone user. During the deactivated state the profile may
	 * have been update.
	 */
	void activate(const t_user &user);
	
	/** Deactivate phone user. */
	void deactivate(void);
};

#endif
