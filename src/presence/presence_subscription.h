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
 * Presence subscription (RFC 3856)
 */

#ifndef _PRESENCE_SUBSCRIPTION_H
#define _PRESENCE_SUBSCRIPTION_H

#include "presence_state.h"
#include "presence_dialog.h"
#include "subscription.h"

/** Subscription to the presence event (RFC 3856) */
class t_presence_subscription : public t_subscription {
private:
	t_presence_state	*presence_state;
	
protected:
	virtual t_request *create_subscribe(unsigned long expires) const;
	
public:
	/**
	 * Constructor.
	 * @param _dialog [in] Dialog for the presence subscription.
	 * @param _state [in] Current presence state.
	 */
	t_presence_subscription(t_presence_dialog *_dialog, t_presence_state *_state);

	virtual bool recv_notify(t_request *r, t_tuid tuid, t_tid tid);
	virtual bool recv_subscribe_response(t_response *r, t_tuid tuid, t_tid tid);
};

#endif
