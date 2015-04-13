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
 * RFC 3842 simple-message-summary body
 */

#ifndef SIMPLE_MSG_SUM_BODY_HH
#define SIMPLE_MSG_SUM_BODY_HH

#include <string>
#include <map>
#include <cc++/config.h>
#include "parser/sip_body.h"
#include "sockets/url.h"

// RFC 3458 6.2
// Message contexts
#define MSG_CONTEXT_VOICE	"voice-message"
#define MSG_CONTEXT_FAX		"fax-message"
#define MSG_CONTEXT_MULTIMEDIA	"multimedia-message"
#define MSG_CONTEXT_TEXT	"text-message"
#define MSG_CONTEXT_NONE	"none"

using namespace std;

/** Message summary counters */
struct t_msg_summary {
	uint32		newmsgs;
	uint32		newmsgs_urgent;
	uint32		oldmsgs;
	uint32		oldmsgs_urgent;

	t_msg_summary();
	
	/**
	 * Parse a text representation of a message summary.
	 * @param s [in] The text to parse.
	 * @return false if parsing fails, true if it succeeds.
	 */
	bool parse(const string &s);

	/** Set all counters to zero */
	void clear(void);
};

typedef string t_msg_context;
typedef map<t_msg_context, t_msg_summary>::const_iterator t_msg_sum_const_iter;

class t_simple_msg_sum_body : public t_sip_body {
private:
	bool			msg_waiting;
	t_url			msg_account;
	map<t_msg_context, t_msg_summary> msg_summary;
	
	// Returns true if string is a valid message context
	bool is_context(const string &s);
		
public:
	t_simple_msg_sum_body();

	// Return text encoded body
	virtual string encode(void) const;

	// Create a copy of the body
	virtual t_sip_body *copy(void) const;

	// Get type of body
	virtual t_body_type get_type(void) const;
	
	virtual t_media get_media(void) const;
	
	// Add a message summary
	void add_msg_summary(const string &context, const t_msg_summary summary);
	
	bool get_msg_waiting(void) const;
	t_url get_msg_account(void) const;
	
	// Get the message summary for a particular context
	// If the context is not present, then false is returned
	bool get_msg_summary(const string &context, t_msg_summary &summary) const;
	
	// Parse a text representation of the body.
	bool parse(const string &s);
};

#endif
