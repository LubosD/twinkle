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
 * Presence state (RFC 3863)
 */

#ifndef _PRESENCE_STATE_H
#define _PRESENCE_STATE_H

#include <string>
#include "threads/mutex.h"

using namespace std;

// Forward declaration
class t_buddy;

/** Presence state */
class t_presence_state {
public:
	/** Basic state (RFC 3863 4.1.4) */
	enum t_basic_state {
		ST_BASIC_UNKNOWN, /**< Presence state is unknown. */
		ST_BASIC_CLOSED,  /**< Unable to accept communication. */
		ST_BASIC_OPEN,	  /**< Ready to accept communication. */
		ST_BASIC_FAILED,  /**< Failed to determine basic state. */
		ST_BASIC_REJECTED,/**< Subscription has been rejected. */
	};
	
	/**
	 * Convert a basic state to a string representation for internal usage.
	 * @param state [in] A basic state value.
	 * @return String representation of the basic state.
	 */
	static string basic_state2str(t_basic_state state);
	
	/**
	 * Convert a basic state to a PIDF string representation.
	 * @param state [in] A basic state value.
	 * @return PIDF representation of the basic state.
	 */
	static string basic_state2pidf_str(t_basic_state state);
	
private:
	/** Mutex for concurrent access to the presence state. */
	mutable t_mutex	mtx_state;
	
	/** Buddy owning this state. */
	t_buddy		*buddy;
	
	/** Basic presence state. */
	t_basic_state	basic_state;
	
	/** Detailed failure message */
	string		failure_msg;
	
	/** Protect the default constructor from being used. */
	t_presence_state();
	
public:
	/** Constructor. */
	t_presence_state(t_buddy *_buddy);
	
	/** @name Getters */
	//@{
	t_basic_state get_basic_state(void) const;
	string get_failure_msg(void) const;
	//@}
	
	/** @name Setters */
	//@{
	void set_basic_state(t_basic_state state);
	void set_failure_msg(const string &msg);
	//@}
};

#endif
