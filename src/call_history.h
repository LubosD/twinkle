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
 * Call history
 */

#ifndef _CALL_HISTORY_H
#define _CALL_HISTORY_H

#include <list>
#include <string>
#include <sys/time.h>
#include "parser/request.h"
#include "parser/response.h"
#include "sockets/url.h"
#include "threads/mutex.h"
#include "utils/record_file.h"

using namespace std;

/** Call detail record. */
class t_call_record : public utils::t_record {
public:

/** Release cause of a call. */
enum t_rel_cause {
	CS_LOCAL_USER,	/**< Released by the local user. */
	CS_REMOTE_USER, /**< Released by the remote user. */
	CS_FAILURE	/**< Call ended due to failure. */
};

/** Direction of the call as seen from the user. */
enum t_direction {
	DIR_IN,		/**< Incoming call. */
	DIR_OUT		/**< Outgoing call. */
};

private:
	static t_mutex		mtx_class; 	/**< Protect static members. */
	static unsigned short	next_id;   	/**< Next id to be used. */
	
	unsigned short		id; 		/**< Record id. */

public:
	time_t		time_start;		/**< Timestamp of start of call. */
	time_t		time_answer;		/**< Timestamp when call got answered. */
	time_t		time_end;		/**< Timestamp of end of call. */
	t_direction	direction;
	string		from_display;
	t_url		from_uri;
	string		from_organization;
	string		to_display;
	t_url		to_uri;
	string		to_organization;
	string		reply_to_display;
	t_url		reply_to_uri;
	string		referred_by_display;
	t_url		referred_by_uri;
	string		subject;
	t_rel_cause	rel_cause;
	int		invite_resp_code;	/**< Response code sent/received on INVITE. */
	string		invite_resp_reason;	/**< Response reason sent/received on INVITE. */
	string		far_end_device;		/**< User-agent/Server description of device. */
	string		user_profile;
	
	/** Constructor. */
	t_call_record();
	
	/**
	 * Clear current settings and get a new record id.
	 * So this action creates a brand new call record.
	 */
	void renew();
	
	/**
	 * Record call start.
	 * @param invite [in] The INVITE request starting the call.
	 * @param dir [in] Call direction.
	 * @param _user_profile [in] The user profile.
	 */
	void start_call(const t_request *invite, t_direction dir, const string &_user_profile);
	
	/**
	 * Record call failure. This is also the end of the call.
	 * @param resp [in] The failure response.
	 */
	void fail_call(const t_response *resp);
	
	/**
	 * Record successful call answer.
	 * @param resp [in] The 2XX INVITE response.
	 */
	void answer_call(const t_response *resp);
	
	/**
	 * Record end of a successful call with an explicit cause.
	 * @param cause [in] The release cause.
	 */
	void end_call(t_rel_cause cause);
	
	/**
	 * Record end of a successful call.
	 * If far_end is true, then the far-end ended the call, otherwise
	 * the near-end ended the call. This indication together with the
	 * direction determines the correct cause of the call end.
	 * @param far_end [in] Indicates if the far end released the call.
	 */
	void end_call(bool far_end);
	
	/**
	 * Get user presentable release cause description.
	 * The release cause is returned in the language of the user.
	 * @return Release cause description.
	 */
	string get_rel_cause(void) const;
	
	/**
	 * Get release cause description for internal use.
	 * This description is written to file.
	 * @return Release cause description.
	 */
	string get_rel_cause_internal(void) const;
	
	/**
	 * Get user presentable direction description.
	 * The description is returned in the language of the user.
	 * @return Direction description.
	 */
	string get_direction(void) const;
	
	/**
	 * Get direction description for internal use.
	 * This description is written to file.
	 * @return Direction description.
	 */
	string get_direction_internal(void) const;
	
	/**
	 * Set the release cause from an internal description.
	 * @param cause [in] Internal release cause description.
	 * @return Indication if operation succeeded.
	 */
	bool set_rel_cause(const string &cause);
	
	/**
	 * Set the direction from an internal description.
	 * @param cause [in] Internal direction description.
	 * @return Indication if operation succeeded.
	 */
	bool set_direction(const string &dir);
	
	virtual bool create_file_record(vector<string> &v) const;
	virtual bool populate_from_file_record(const vector<string> &v);
	
	/**
	 * Check if this call record represents a valid call.
	 * @return Indication if call record is valid.
	 */
	bool is_valid(void) const;
	
	/** Get the record id. */
	unsigned short get_id(void) const;
};

/** History of calls. */
class t_call_history : public utils::t_record_file<t_call_record> {
private:
	/** Number of missed calls since this counter was cleared. */
	int		num_missed_calls;
	
public:
	/** Constructor. */
	t_call_history();
	
	/**
	 * Add a call record to the history.
	 * @param call_record [in] The call record to be added.
	 * @param write [in] Indicates if history must be written to file after adding.
	 */
	void add_call_record(const t_call_record &call_record, bool write = true);
	
	/**
	 * Delete record with a given id.
	 * @param id [in] The record id that must be deleted.
	 * @param write [in] Indicates if history must be written to file after deleting.
	 */
	void delete_call_record(unsigned short id, bool write = true);
	
	/** 
	 * Get list of historic call records.
	 * @param history [out] List of historic call records.
	 */
	void get_history(list<t_call_record> &history);
	
	/** 
	 * Clear call history file.
	 * @param write [in] Indicates if history must be written to file after adding.
	 */
	void clear(bool write = true);
	
	/** Get number of missed calls. */
	int get_num_missed_calls(void) const;
	
	/** Clear number of missed calls. */
	void clear_num_missed_calls(void);
};

extern t_call_history *call_history;

#endif
