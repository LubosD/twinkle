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
 * Dialog for presence subscription (RFC 3856)
 */

#ifndef _PRESENCE_DIALOG_H
#define _PRESENCE_DIALOG_H

#include "subscription_dialog.h"
#include "presence_state.h"

// Forward declaration
class t_phone_user;

/** Dialog for presence subscription (RFC 3856) */
class t_presence_dialog : public t_subscription_dialog {
public:
	/**
	 * Constructor.
	 * @param _phone_user [in] Phone user owning the dialog.
	 * @param presence_state [in] Presence state that is updated by notification
	 * on this dialog
	 */
	t_presence_dialog(t_phone_user *_phone_user, t_presence_state *presence_state);
	
	virtual t_presence_dialog *copy(void);
};

#endif
