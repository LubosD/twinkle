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

#ifndef _PHONE_H
#define _PHONE_H

#include <list>
#include <string>
#include <vector>
#include <sys/time.h>
#include "auth.h"
#include "call_history.h"
#include "dialog.h"
#include "id_object.h"
#include "phone_user.h"
#include "protocol.h"
#include "service.h"
#include "transaction_layer.h"
#include "im/msg_session.h"
#include "mwi/mwi.h"
#include "sockets/url.h"
#include "parser/request.h"
#include "parser/response.h"
#include "presence/presence_state.h"

// Number of phone lines
// One line is used by Twinkle internally to park the call towards a
// referrer while the refer is in progress.
// Besides the lines for making calls, ephemeral lines will be created
// for parking calls that are being released. By parking a releasing
// call, the line visible to the user is free for making new calls.
#define NUM_CALL_LINES	3	// Total numbers of phone lines for making calls
#define NUM_USER_LINES	2	// #lines usable for the user

#define LINENO_REFERRER	2	// Internal lineno for referrer

// Number of seconds to wait till all lines are idle when terminating
// Twinkle
#define QUIT_IDLE_WAIT	2

using namespace std;
using namespace im;

// Forward declarations
class t_dialog;
class t_client_request;
class t_line;
class t_call_info;

enum t_phone_state {
	PS_IDLE,
	PS_BUSY
};

enum t_line_state {
	LS_IDLE,
	LS_BUSY
};

enum t_line_substate {
	// Idle sub states
	LSSUB_IDLE,			// line is idle
	LSSUB_SEIZED,			// user has seized the line to call

	// Busy sub states
	LSSUB_INCOMING_PROGRESS,	// incoming call in progress
	LSSUB_OUTGOING_PROGRESS,	// outgoing call in progress
	LSSUB_ANSWERING,		// sent 200 OK, waiting for ACK
	LSSUB_ESTABLISHED,		// call established
	LSSUB_RELEASING			// call is being released (BYE sent)
};

class t_transfer_data {
private:
	// The received REFER request
	t_request	*refer_request;
	
	// Line number on which REFER was received
	unsigned short	lineno;
	
	// Indicates if triggered INVITE must be anonymous
	bool		hide_user;
	
	t_phone_user	*phone_user;
	
public:
	t_transfer_data(t_request *r, unsigned short _lineno, bool _hide_user, 
		t_phone_user *pu);
	~t_transfer_data();
	
	t_request *get_refer_request(void) const;
	unsigned short get_lineno(void) const;
	bool get_hide_user(void) const;
	t_phone_user *get_phone_user(void) const;
};

class t_phone : public t_transaction_layer {
private:
	// Indicates if the phone is active, accepting calls.
	bool			is_active;

	// Phone users
	list<t_phone_user *>	phone_users;

	// Phone lines
	// The first NUM_CALL_LINES are for making phone calls.
	// The tail of the vector is for releasing lines in the background.
	vector<t_line *>	lines;

	// Operations like invite, end_call work on the active line
	unsigned short		active_line;

	// 3-way conference data
	bool			is_3way;	// indicates an acitive 3-way
	t_line			*line1_3way;	// first line in 3-way conf
	t_line			*line2_3way;	// second line in 3-way conf
	
	// Call transfer data. When a REFER comes in, the user has
	// to give permission before the triggered INVITE can be sent.
	// While the user interface presents the question to the user,
	// the data related to the incoming REFER is stored here.
	t_transfer_data		*incoming_refer_data;
	
	// Time of startup
	time_t			startup_time;
	
	// Line release operations
	// Move a line to the background so it will be released in the
	// background.
	void move_line_to_background(unsigned short lineno);
	
	// Move all call lines that are in releasing state to the
	// background.
	void move_releasing_lines_to_background(void);
	
	// Destroy lines in the background that are idle.
	void cleanup_dead_lines(void);
	
	// If a line was part of a 3way, then remove it from the
	// 3way conference data.
	void cleanup_3way_state(unsigned short lineno);
	
	// If one of the lines of a 3way calls has become idle, then
	// cleanup the 3way conference data.
	void cleanup_3way(void);

	/** @name Actions */
	//@{
	/**
	 * Send an INVITE
	 * @param pu The phone user making this call.
	 * @param to_uri The URI to be used a request-URI and To header URI
	 * @param to_display Display name for To header.
	 * @param subject If not empty, this string will go into the Subject header.
	 * @param no_fork If true, put a no-fork request disposition in the outgoing INVITE
	 * @param anonymous Inidicates if the INVITE should be sent anonymous.
	 */
	void invite(t_phone_user *pu, const t_url &to_uri, const string &to_display,
		const string &subject, bool no_fork, bool anonymous);
		
	void answer(void);
	void redirect(const list<t_display_url> &destinations, int code, string reason = "");
	void reject(void);
	void reject(unsigned short line);
	void end_call(void);
	void registration(t_phone_user *pu, t_register_type register_type,
					unsigned long expires = 0);
	//@}

	// OPTIONS outside dialog
	void options(t_phone_user *pu, const t_url &to_uri, const string &to_display = "");

	// OPTIONS inside dialog
	void options(void);

	bool hold(bool rtponly = false); // returns false is call cannot be put on hold
	void retrieve(void);

	// Transfer a call (send REFER to far-end)
	void refer(const t_url &uri, const string &display);
	
	// Call transfer with consultation (attended)
	// Transfer the far-end on line lineno_from to the far-end of lineno_to.
	void refer(unsigned short lineno_from, unsigned short lineno_to);
	void refer_attended(unsigned short lineno_from, unsigned short lineno_to);
	void refer_consultation(unsigned short lineno_from, unsigned short lineno_to);
	
	// Setup a consultation call for transferring the call on the active
	// line. The active line is put on-hold and the consultation call is
	// made on an idle line.
	void setup_consultation_call(const t_url &uri, const string &display);

	// Make line l active. If the current line is busy, then that call
	// will be put on-hold. If line l has a call on-hold, then that
	// call will be retrieved.
	void activate_line(unsigned short l);

	// Send a DTMF digit
	void send_dtmf(char digit, bool inband, bool info);

	void set_active_line(unsigned short l);

	// Handle responses for out-of-dialog requests
	void handle_response_out_of_dialog(t_response *r, t_tuid tuid, t_tid tid);
	void handle_response_out_of_dialog(StunMessage *r, t_tuid tuid);
	
	// Match an incoming message to a phone user
	t_phone_user *match_phone_user(t_response *r, t_tuid tuid, bool active_only = false);
	t_phone_user *match_phone_user(t_request *r, bool active_only = false);
	t_phone_user *match_phone_user(StunMessage *r, t_tuid tuid, bool active_only = false);
	
	/**
	 * Hunt for an idle line to hande an incoming call.
	 * @return The number of the line to handle the call (starting at 0).
	 * @return -1 if there is no line to handle the call.
	 */
	int hunt_line(void);

protected:
	/**
	 * Find a phone user that can handle an out-of-dialog request.
	 * If there is no phone user that can handle the request, then this
	 * method will send an appropriate failure response on the request.
	 * @param r [in] The request.
	 * @param tid [in] Transaction id of the request transaction.
	 * @return The phone user, if there is a phone user that can handle the request.
	 * @return NULL, otherwise.
	 */
	t_phone_user *find_phone_user_out_dialog_request(t_request *r, t_tid tid);
	
	/**
	 * Find a line that can handle an in-dialog request.
	 * If there is no line that can handle the request, then this
	 * method will send an appropriate failure response on the request.
	 * @param r [in] The request.
	 * @param tid [in] Transaction id of the request transaction.
	 * @return The line, if there is a line that can handle the request.
	 * @return NULL, otherwise.
	 */
	t_line *find_line_in_dialog_request(t_request *r, t_tid tid);

	// Events
	void recvd_provisional(t_response *r, t_tuid tuid, t_tid tid);
	void recvd_success(t_response *r, t_tuid tuid, t_tid tid);
	void recvd_redirect(t_response *r, t_tuid tuid, t_tid tid);
	void recvd_client_error(t_response *r, t_tuid tuid, t_tid tid);
	void recvd_server_error(t_response *r, t_tuid tuid, t_tid tid);
	void recvd_global_error(t_response *r, t_tuid tuid, t_tid tid);
	void post_process_response(t_response *r, t_tuid tuid, t_tid tid);

	void recvd_invite(t_request *r, t_tid tid);
	void recvd_initial_invite(t_request *r, t_tid tid);
	void recvd_re_invite(t_request *r, t_tid tid);
	void recvd_ack(t_request *r, t_tid tid);
	void recvd_cancel(t_request *r, t_tid cancel_tid, t_tid target_tid);
	void recvd_bye(t_request *r, t_tid tid);
	void recvd_options(t_request *r, t_tid tid);
	void recvd_register(t_request *r, t_tid tid);
	void recvd_prack(t_request *r, t_tid tid);
	void recvd_subscribe(t_request *r, t_tid tid);
	void recvd_notify(t_request *r, t_tid tid);
	void recvd_refer(t_request *r, t_tid tid);
	void recvd_info(t_request *r, t_tid tid);
	void recvd_message(t_request *r, t_tid tid);
	void post_process_request(t_request *r, t_tid cancel_tid, t_tid target_tid);

	void failure(t_failure failure, t_tid tid);
	
	void recvd_stun_resp(StunMessage *r, t_tuid tuid, t_tid tid);
	
	void recvd_refer_permission(bool permission);
	
	/** @name Timeout handlers */
	//@{
	virtual void handle_event_timeout(t_event_timeout *e);
	
	/**
	 * Process expiry of line timer.
	 * @param id [in] Line id of the line associate with the timer.
	 * @param timer [in] Type of line timer.
	 * @param did [in] Dialog id if timer is for a dialog, 0 otherwise.
	 */
	void line_timeout(t_object_id id, t_line_timer timer, t_object_id did);
	
	/**
	 * Process expiry of a line subscription timer (REFER subscription).
	 * @param id [in] Line id of the line associate with the timer.
	 * @param timer [in] Type of subcription timer.
	 * @param did [in] Dialog id associated with the timer.
	 * @param event_type [in] Event type of the subscription.
	 * @param event_id [in] Event id of the subscription.
	 */
	void line_timeout_sub(t_object_id id, t_subscribe_timer timer, t_object_id did,
		const string &event_type, const string &event_id);
		
	/**
	 * Process expiry of a subcription timer.
	 * @param timer [in] Type of subcription timer.
	 * @param id_timer [in] Timer id of expired timer.
	 */
	void subscription_timeout(t_subscribe_timer timer, t_object_id id_timer);
	
	/**
	 * Process expiry of a publication timer.
	 * @param timer [in] Type of publication timer.
	 * @param id_timer [in] Timer id of expired timer.
	 */
	void publication_timeout(t_publish_timer timer, t_object_id id_timer);
	
	/**
	 * Process expiry of phone timer.
	 * @param timer [in] Type of phone timer.
	 * @param id_timer [in] Timer id of expired timer.
	 */
	void timeout(t_phone_timer timer, unsigned short id_timer);
	//@}
	
	virtual void handle_broken_connection(t_event_broken_connection *e);

public:
	t_phone();
	virtual ~t_phone();
	
	// Get line based on object id
	// Returns NULL if there is no such line.
	t_line *get_line_by_id(t_object_id id) const;
	
	// Get line based on line number
	t_line *get_line(unsigned short lineno) const;

	// Get busy/idle state of the phone
	// PS_IDLE - at least one line is idle
	// PS_BUSY - all lines are busy
	t_phone_state get_state(void) const;
	
	// Returns true if all lines are in the LSSUB_IDLE state
	bool all_lines_idle(void) const;
	
	// Get an idle user line.
	// If no line is idle, then false is returned.
	bool get_idle_line(unsigned short &lineno) const;
	
	// Actions to be called by the user interface.
	// These methods first lock the phone, then call the corresponding
	// private method and then unlock the phone.
	// The private methods should only be called by the phone, line,
	// and dialog objects to avoid deadlocks.
	void pub_invite(t_user *user,
		const t_url &to_uri, const string &to_display,
		const string &subject, bool anonymous);
	void pub_answer(void);
	void pub_reject(void);
	void pub_reject(unsigned short line);
	void pub_redirect(const list<t_display_url> &destinations, int code, string reason = "");
	void pub_end_call(void);
	void pub_registration(t_user *user, t_register_type register_type,
						unsigned long expires = 0);
	void pub_options(t_user *user, 
			const t_url &to_uri, const string &to_display = "");
	void pub_options(void);
	bool pub_hold(void);
	void pub_retrieve(void);
	void pub_refer(const t_url &uri, const string &display);
	void pub_refer(unsigned short lineno_from, unsigned short lineno_to);
	void pub_setup_consultation_call(const t_url &uri, const string &display);
	void mute(bool enable);
	
	void pub_activate_line(unsigned short l);
	void pub_send_dtmf(char digit, bool inband, bool info);
	
	// ZRTP actions
	void pub_confirm_zrtp_sas(unsigned short line);
	void pub_confirm_zrtp_sas(void);
	void pub_reset_zrtp_sas_confirmation(unsigned short line);
	void pub_reset_zrtp_sas_confirmation(void);
	void pub_enable_zrtp(void);
	void pub_zrtp_request_go_clear(void);
	void pub_zrtp_go_clear_ok(unsigned short line);

	// Join 2 lines in a 3-way conference. Returns false if 3-way cannot
	// be setup
	bool join_3way(unsigned short lineno1, unsigned short lineno2);

	// Seize the line.
	// Returns false if seizure failed.
	bool pub_seize(void); // active line
	bool pub_seize(unsigned short line);
	
	// Unseize the line
	void pub_unseize(void); // active line
	void pub_unseize(unsigned short line);
	
	/** @name MWI */
	//@{
	/**
	 * Subscribe to MWI.
	 * @param user [in] The user profile of the subscribing user.
	 */
	void pub_subscribe_mwi(t_user *user);
	
	/**
	 * Unsubscribe to MWI.
	 * @param user [in] The user profile of the unsubscribing user.
	 */
	void pub_unsubscribe_mwi(t_user *user);
	//@}
	
	/** @name Presence */
	//@{
	/**
	 * Subscribe to presence of buddies in buddy list.
	  * @param user [in] The user profile of the subscribing user.
	 */
	void pub_subscribe_presence(t_user *user);
	
	/**
	 * Unsubscribe to presence of buddies in buddy list.
	 * @param user [in] The user profile of the unsubscribing user.
	 */
	void pub_unsubscribe_presence(t_user *user);
	
	/**
	 * Publish presence state.
	 * @param user [in] The user profile of the user publishing.
	 * @param basic_state [in] The basic presence state to publish.
	 */
	void pub_publish_presence(t_user *user, t_presence_state::t_basic_state basic_state);
	
	/**
	 * Unpublish presence state.
	 * @param user [in] The user profile of the user unpublishing.
	 */
	void pub_unpublish_presence(t_user *user);
	//@}
	
	/** @name Instant messaging */
	//@{
	/**
	 * Send a message.
	 * @param user [in] User profile of user sending the message.
	 * @param to_uri [in] Destination URI of recipient.
	 * @param to_display [in] Display name of recipient.
	 * @param msg [in] Message to send.
	 * @return True if sending succeeded, false otherwise.
	 */
	bool pub_send_message(t_user *user, const t_url &to_uri, const string &to_display,
			const t_msg &msg);
			
	/**
	 * Send a message composing state indication.
	 * @param user [in] User profile of user sending the message.
	 * @param to_uri [in] Destination URI of recipient.
	 * @param to_display [in] Display name of recipient.
	 * @param state [in] Message composing state.
	 * @param refresh [in] The refresh interval in seconds (when state is active).
	 * @return True if sending succeeded, false otherwise.
	 * @note For the idle state, the value of refresh has no meaning.
	 */
	bool pub_send_im_iscomposing(t_user *user, const t_url &to_uri, const string &to_display,
			const string &state, time_t refresh);
	//@}

	unsigned short get_active_line(void) const;

	// Authorize the request based on the challenge in the response
	// Returns false if authorization fails.
	bool authorize(t_user *user, t_request *r, t_response *resp);
	
	// Remove cached credentials for a particular user/realm
	void remove_cached_credentials(t_user *user, const string &realm);

	bool get_is_registered(t_user *user);
	bool get_last_reg_failed(t_user *user);
	t_line_state get_line_state(unsigned short lineno) const;
	t_line_substate get_line_substate(unsigned short lineno) const;
	bool is_line_on_hold(unsigned short lineno) const;
	bool is_line_muted(unsigned short lineno) const;
	bool is_line_transfer_consult(unsigned short lineno, 
		unsigned short &transfer_from_line) const;
	bool line_to_be_transferred(unsigned short lineno, 
		unsigned short &transfer_to_line) const;
	bool is_line_encrypted(unsigned short lineno) const;
	bool is_line_auto_answered(unsigned short lineno) const;
	t_refer_state get_line_refer_state(unsigned short lineno) const;
	t_user *get_line_user(unsigned short lineno);
	bool has_line_media(unsigned short lineno) const;
	bool is_mwi_subscribed(t_user *user) const;
	bool is_mwi_terminated(t_user *user) const;
	t_mwi get_mwi(t_user *user) const;
	
	/**
	 * Check if all presence subscriptions for a particular user are terminated.
	 * @param user [in] User profile of the user.
	 * @return True if all presence susbcriptions are terminated, otherwise false.
	 */
	bool is_presence_terminated(t_user *user) const;
	
	// Get remote uri/display of the active call on a line.
	// If there is no call, then an empty uri/display is returned.
	t_url get_remote_uri(unsigned short lineno) const;
	string get_remote_display(unsigned short lineno) const;

	// Return if a line is part of a 3-way conference
	bool part_of_3way(unsigned short lineno);

	// Get the peer line in a 3-way conference
	t_line *get_3way_peer_line(unsigned short lineno);

	// Notify progress of a reference. r is the response to the INVITE
	// caused by a REFER. referee_lineno is the line number of the line
	// that is setting up there reference call.
	void notify_refer_progress(t_response *r, unsigned short referee_lineno);

	// Get call info record for a line.
	t_call_info get_call_info(unsigned short lineno) const;
	
	// Get the call history record for a line
	t_call_record get_call_hist(unsigned short lineno) const;
	
	// Get ring tone for a line
	string get_ringtone(unsigned short lineno) const;
	
	// Get the startup time of the phone
	time_t get_startup_time(void) const;

	// Initialize the RTP port values for all lines.
	void init_rtp_ports(void);
	
	/**
	 * Add a phone user.
	 * @param user_config [in] User profile of the user to add.
	 * @param dup_user [out] Profile of duplicate user.
	 * @return false, if there is already a phone user with the same name
	 * and domain. In this case dup_user is a pointer to the user config
	 * of that user.
	 * @return true, if the phone user was added succesfully.
	 * @note if there is already a user with exactly the same user config
	 * then true is returned, but the user is not added again. The user
	 * will be activated if it was inactive though.
	 */
	bool add_phone_user(const t_user &user_config, t_user **dup_user);
	
	/**
	 * Deactivate the phone user.
	 * @param user_config [in] User profile of the user to deactivate.
	 */
	void remove_phone_user(const t_user &user_config);

	/**
	 * Get a list of user profiles of all phone users.
	 * @return List of user profiles.
	 */
	list<t_user *> ref_users(void);
	
	/**
	 * Get the user profile of a user for which user->get_display_uri() ==
	 * display_uri.
	 * @param display_uri [in] Display URI.
	 * @return User profile.
	 */
	t_user *ref_user_display_uri(const string &display_uri);
	
	/**
	 * Get the user profile matching the profile name.
	 * @param profile_name [in] User profile name.
	 * @return User profile.
	 */
	t_user *ref_user_profile(const string &profile_name);
	
	/**
	 * Get service information for a phone user.
	 * @param user [in] User profile of the phone user.
	 * @return Service object.
	 */
	t_service *ref_service(t_user *user);
	
	/**
	 * Get the buddy list of a phone user.
	 * @param user [in] User profile of the phone user.
	 * @return Buddy list.
	 */
	t_buddy_list *ref_buddy_list(t_user *user);
	
	/**
	 * Get the presence event publication agent of a phone user.
	 * @param user [in] User profile of the phone user.
	 * @return The presence EPA.
	 */
	t_presence_epa *ref_presence_epa(t_user *user);
	
	/**
	 * Find active phone user
	 * @param profile_name [in] User profile name.
	 * @return The phone user for the user profile, NULL if there is no active phone user.
	 */
	t_phone_user *find_phone_user(const string &profile_name) const;
	
	/** 
	 * Find active phone user
	 * @param user_uri [in] The user URI (AoR) of the user to find.
	 * @return The phone user for the URI, NULL if there is no active phone user.
	 */
	t_phone_user *find_phone_user(const t_url &user_uri) const;
	
	/**
	 * Get local IP address for SIP.
	 * @param user [in] The user profile of the user for whom to get the IP address.
	 * @param auto_ip [in] IP address to use if no IP address has been determined through
	 *                     some NAT procedure.
	 * @return The IP address.
	 */
	string get_ip_sip(const t_user *user, const string &auto_ip) const;
	
	/**
	 * Get local port for SIP.
	 * @param user [in] User profile for user for whom to get the port.
	 * @return SIP port.
	 */ 
	unsigned short get_public_port_sip(const t_user *user) const;
	
	/** Indicates if STUN is used. */
	bool use_stun(t_user *user);
	
	// Indicates if a NAT keepalive mechanism is used
	bool use_nat_keepalive(t_user *user);
	
	/** Disable STUN for a user. */
	void disable_stun(t_user *user);
	
	/** Synchronize sending of NAT keep alives with user configuration settings. */
	void sync_nat_keepalive(t_user *user);
	
	// Perform NAT discovery for all users having STUN enabled.
	// If NAT discovery indicates that STUN cannot be used for 1 or more
	// users, then false will be returned and msg_list contains a list
	// of messages to be shown to the user.
	bool stun_discover_nat(list<string> &msg_list);
	
	// Perform NAT discovery for a single user.
	bool stun_discover_nat(t_user *user, string &msg);
	
	// Create a response to an OPTIONS request
	// Argument 'in-dialog' indicates if the OPTIONS response is
	// sent within a dialog.
	t_response *create_options_response(t_user *user, t_request *r,
					bool in_dialog = false);
					
	// Timer operations
	void start_timer(t_phone_timer timer, t_phone_user *pu);
	void stop_timer(t_phone_timer timer, t_phone_user *pu);

	// Start a timer with the time set in the time-argument.
	void start_set_timer(t_phone_timer timer, long time, t_phone_user *pu);
	
	/**
	 * Initialize the phone functions.
	 * Register all active users with auto register.
	 * Initialize extensions for users without auto register.
	 */
	void init(void);
	
	/**
	 * Initialize SIP extensions like MWI and presence.
	 * @param user_config [in] User for which the extensions must be initialized.
	 */
	void init_extensions(t_user *user_config);
	
	/**
	 * Set the signal handler to handler for LinuxThreads.
	 * @return True if succesful, false otherwise.
	 */
	bool set_sighandler(void) const;
	
	/**
	 * Terminate the phone functions.
	 * Release all calls, don't accept any new calls.
	 * Deregister all active users.
	 */
	void terminate(void);
};

// Main function for the UAS part of the phone
void *phone_uas_main(void *arg);

// Entry function of thread catching signals to terminate
// the application in a graceful manner if NPLT is used.
void *phone_sigwait(void *arg);

// Signal handler to process signals if LinuxThreads is used.
void phone_sighandler(int sig);

#endif
