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
 * Buddy list
 */

#ifndef _BUDDY_H
#define _BUDDY_H

#include <string>
#include <set>

#include "presence_state.h"
#include "presence_dialog.h"

#include "sockets/url.h"
#include "utils/record_file.h"
#include "patterns/observer.h"

// Forward declaration
class t_phone_user;
class t_buddy_list;

#define BUDDY_FILE_EXT		".bud"

using namespace std;

/** Buddy */
class t_buddy : public utils::t_record, public patterns::t_subject {
private:
	/** Phone user owning this buddy. */
	t_phone_user	*phone_user;

	/** Name of buddy (for display only) */
	string	name;
	
	/** SIP address of the buddy. */
	string	sip_address;
	
	/** Indicates if the user may subscribe to the presence state of the buddy. */
	bool	may_subscribe_presence;
	
	/** Presence state. */
	t_presence_state	presence_state;
	
	/** Presence subscription dialog. */
	t_presence_dialog	*presence_dialog;
	
	/** Subscribe to presence after STUN transaction completed. */
	bool	subscribe_after_stun;
	
	/**
	 * Indicates if presence must be automatically resubscribed to, if the
	 * subscription terminates with a reason telling that resubscription
	 * is possible.
	 */
	bool	presence_auto_resubscribe;
	
	/** 
	 * Indicates if the buddy must be deleted after the presence subscription
	 * has been terminated.
	 */
	bool	delete_after_presence_terminated;
	
	/** Handle presence dialog termination. */
	void cleanup_presence_dialog(void);
	
public:
	/** Interval before trying to resubscribe to presence after a failure. */
	t_object_id	id_resubscribe_presence;
	
	/** Constructor. */
	t_buddy();

	/** Constructor. */
	t_buddy(t_phone_user *_phone_user);

	/** Constructor. */
	t_buddy(t_phone_user *_phone_user, const string _name, const string &_sip_address);
	
	/** Copy constructor. */
	t_buddy(const t_buddy &other);
	
	/** Destructor. */
	virtual ~t_buddy();
	
	/** @name Getters */
	//@{
	string get_name(void) const;
	string get_sip_address(void) const;
	bool get_may_subscribe_presence(void) const;
	const t_presence_state *get_presence_state(void) const;
	//@}
	
	/**
	 * Get user profile for the user owning this buddy.
	 * @return User profile.
	 */
	t_user *get_user_profile(void);
	
	/**
	 * Get the buddy list containing this buddy.
	 * @return Buddy list
	 */
	t_buddy_list *get_buddy_list(void);
	
	/** @name Setters */
	//@{
	void set_phone_user(t_phone_user *_phone_user);
	void set_name(const string &_name);
	void set_sip_address(const string &_sip_address);
	void set_may_subscribe_presence(bool _may_subscribe_presence);
	//@}
	
	/**
	 * Match response with a buddy. It matches if the buddy
	 * has a presence dialog that matches with the response.
	 * @param r [in] The response.
	 * @param tuid [in] Transaction user id.
	 * @return True if the response matches, otherwise false.
	 */
	bool match_response(t_response *r, t_tuid tuid) const;
	
	/**
	 * Match request with buddy list. It matches if a buddy in the list
	 * has a presence dialog that matches with the request.
	 * @param r [in] The request.
	 * @return True if the request matches, otherwise false.
	 */
	bool match_request(t_request *r) const;
	
	/**
	 * Match a timer id with a running timer.
	 * @param timer [in] The running timer.
	 * @param id_timer [in] The timer id.
	 * @return true, if timer id matches with timer.
	 * @return false, otherwise.
	 */
	bool match_timer(t_subscribe_timer timer, t_object_id id_timer) const;
	
	/**
	 * Process timeout.
	 * @param timer [in] The timer that expired.
	 * @param id_timer [in] The timer id.
	 */
	void timeout(t_subscribe_timer timer, t_object_id id_timer);

	/**
	 * Handle received response.
	 * @param r [in] The response.
	 * @param tuid [in] Transaction user id.
	 * @param tid [in] Transaction id.
	 */
	void recvd_response(t_response *r, t_tuid tuid, t_tid tid);
	
	/**
	 * Handle received request.
	 * @param r [in] The request.
	 * @param tuid [in] Transaction user id.
	 * @param tid [in] Transaction id.
	 */
	void recvd_request(t_request *r, t_tuid tuid, t_tid tid);
	
	/**
	 * Start the re-subscribe timer after a presence subscription failure.
	 * @param duration [in] Duration before trying a re-subscribe (s)
	 */
	void start_resubscribe_presence_timer(unsigned long duration);
	
	/** Stop presence re-subscribe timer. */
	void stop_resubscribe_presence_timer(void);
	
	/**
	 * By calling this method, succesful STUN completion is signalled to
	 * the buddy. It will subscribe to presence if it was waiting for STUN.
	 */
	void stun_completed(void);
	
	/**
	 * By calling this method, a STUN failure is signalled to
	 * the buddy. It will reschedule a presence subscription if it is
	 * waiting for STUN to complete.
	 */
	void stun_failed(void);
	
	/** Subscribe to presence of the buddy if we may do so. */
	void subscribe_presence(void);
	
	/** Unsubscribe to presence.
	 * @param remove [in] Indicates if the buddy must be deleted after unsubscription.
	 */
	void unsubscribe_presence(bool remove = false);
	
	virtual bool create_file_record(vector<string> &v) const;
	virtual bool populate_from_file_record(const vector<string> &v);
	
	/** Compare 2 buddies for equality (same SIP address) */
	bool operator==(const t_buddy &other) const;
	
	/** Clear presence state. */
	void clear_presence(void);
	
	/**
	 * Check if presence subscription is terminated.
	 * @return true, if presence subscriptions is terminated.
	 * @return false, otherwise
	 */
	bool is_presence_terminated(void) const;
	
	/**
	 * Check if buddy must be deleted.
	 * @return true, if the buddy must be deleted immediately.
	 * @return false, otherwise.
	 */
	bool must_delete_now(void) const;
};

/** List of buddies for a particular account. */
class t_buddy_list : public utils::t_record_file<t_buddy> {
private:
	/** Phone user owning this buddy list. */
	t_phone_user	*phone_user;
	
	/** 
	 * Indicates if subscribe is done. This indicator will be set to false
	 * when you call unsubscribe.
	 */
	bool		is_subscribed;

protected:
	virtual void add_record(const t_buddy &record);
	
public:
	/** Constructor. */
	t_buddy_list(t_phone_user *_phone_user);
	
	/**
	 * Get the user profile for this buddy list.
	 * @return User profile.
	 */
	t_user *get_user_profile(void);

	/**
	 * Add a buddy.
	 * @param buddy [in] Buddy to add.
	 * @return Pointer to added buddy.
	 * @note This method adds a copy of the buddy. It returns a pointer to this copy.
	 */
	t_buddy *add_buddy(const t_buddy &buddy);
	
	/**
	 * Delete a buddy.
	 * @param buddy [in] Buddy to delete.
	 */
	void del_buddy(const t_buddy &buddy);
	
	/**
	 * Match response with buddy list. It matches if a buddy in the list
	 * has a presence dialog that matches with the response.
	 * @param r [in] The response.
	 * @param tuid [in] Transaction user id.
	 * @param buddy [out] On a match, this parameter contains the matching buddy.
	 * @return True if the response matches, otherwise false.
	 */
	bool match_response(t_response *r, t_tuid tuid, t_buddy **buddy);
	
	/**
	 * Match request with buddy list. It matches if a buddy in the list
	 * has a presence dialog that matches with the request.
	 * @param r [in] The request.
	 * @param buddy [out] On a match, this parameter contains the matching buddy.
	 * @return True if the request matches, otherwise false.
	 */
	bool match_request(t_request *r, t_buddy **buddy);
	
	/**
	 * Match a timer id with a running timer. A timer id matches with the
	 * buddy list if it matches with one of the buddies in the list.
	 * @param timer [in] The running timer.
	 * @param id_timer [in] The timer id.
	 * @param buddy [out] On a match, this parameter contains the matching buddy.
	 * @return true, if timer id matches with timer.
	 * @return false, otherwise.
	 */
	bool match_timer(t_subscribe_timer timer, t_object_id id_timer, t_buddy **buddy);
	
	/**
	 * By calling this method, succesful STUN completion is signalled to the buddy
	 * list. The buddy list will now start presence subscriptions that were waiting
	 * for STUN to complete.
	 */
	void stun_completed(void);
	
	/**
	 * By calling this method, a STUN failure is signalled to the buddy list.
	 * The buddy list will reschedule presence subscriptions that were waiting
	 * for STUN to complete.
	 */
	void stun_failed(void);
	
	/** Subscribe to presence of all buddies in the list. */
	void subscribe_presence(void);
	
	/** Unsubscribe to presence of all buddies in the list. */
	void unsubscribe_presence(void);
	
	/**
	 * Check if user is subcribed to buddy list presence.
	 * @return True if subscribed, otherwise false.
	 */
	bool get_is_subscribed() const;
	
	/** Clear presence state of all buddies. */
	void clear_presence(void);
	
	/**
	 * Check if all presence subscriptions are terminated.
	 * @return true, if all presence subscriptions are terminated.
	 * @return false, otherwise
	 */
	bool is_presence_terminated(void) const;
};

#endif
