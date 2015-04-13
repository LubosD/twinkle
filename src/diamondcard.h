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

/** @file 
 * Diamondcard settings (www.diamondcard.us)
 */
 
#ifndef _DIAMONDCARD_H
#define _DIAMONDCARD_H

#include <string>
#include <list>
#include "user.h"
#include "phone.h"

#define DIAMONDCARD_DOMAIN "diamondcard.us"

using namespace std;

/** Actions that can be performed on the Diamondcard web site */
enum t_dc_action {
	DC_ACT_SIGNUP,
	DC_ACT_BALANCE_HISTORY,
	DC_ACT_RECHARGE,
	DC_ACT_CALL_HISTORY,
	DC_ACT_ADMIN_CENTER
};

/** 
 * Get the URL of a Diamondcard web page for an action. 
 * @param action [in] Action for which the URL is requested.
 * @param displayName [in] The display name of the user.
 * @param accountId [in] Account ID of the user. N/A for signup.
 * @param pinCode [in] PIN code of the user. N/A for signup.
 * @return URL of the web page for the requested action.
 */
string diamondcard_url(t_dc_action action, const string &accountId, const string &pinCode);

/** 
 * Configure a user profile for a Diamondcard account.
 * @param user [inout] The user profile to configure.
 * @param accountId [in] Account ID of the user.
 * @param pinCode [in] PIN code of the user.
 */
void diamondcard_set_user_config(t_user &user, const string &displayName,
                                 const string &accountId, const string &pinCode);

/**
 * Get all active Diamondcard users.
 * @return List of active Diamondcard users.
 */
list<t_user *>diamondcard_get_users(t_phone *phone);
#endif
