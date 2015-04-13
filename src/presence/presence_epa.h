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
 * Presence Event Publication Agent (EPA) [RFC 3903]
 */

#ifndef _PRESENCE_EPA_H
#define _PRESENCE_EPA_H

#include <string>

#include "epa.h"
#include "presence_state.h"
#include "patterns/observer.h"

using namespace std;

/** Presence Event Publication Agent (EPA) [RFC 3903] */
class t_presence_epa : public t_epa, public patterns::t_subject {
private:
	/** Basic presence state. */
	t_presence_state::t_basic_state	basic_state;
	
	/** Tuple id to be put in the PIDF documents. */
	string tuple_id;
	
public:
	/** Constructor */
	t_presence_epa(t_phone_user *pu);

	/** @name Getters */
	//@{
	t_presence_state::t_basic_state get_basic_state(void) const;
	//@}
	
	virtual bool recv_response(t_response *r, t_tuid tuid, t_tid tid);

	/**
	 * Publish presence state.
	 * @param _basic_state [in] The basic presence state.
	 * @pre _basic_state must be one of the following values:
	 * - @ref ST_BASIC_CLOSED
	 * - @ref ST_BASIC_OPEN
	 */
	void publish_presence(t_presence_state::t_basic_state _basic_state);
	
	virtual void clear(void);
};

#endif
