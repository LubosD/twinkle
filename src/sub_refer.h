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

// RFC 3515
// Refer event package

#ifndef _SUB_REFER_H
#define _SUB_REFER_H

#include "subscription.h"
#include "dialog.h"

// State of reference as seen by the referrer
enum t_sub_refer_result {
	SRR_INPROG,	// Referee is referring call
	SRR_FAILED,	// Refer failed
	SRR_SUCCEEDED,	// Refer succeeded
};

class t_sub_refer : public t_subscription {
private:
	// Result of the reference as seen by the referrer.
	t_sub_refer_result	sr_result;

	// Last response received from the refer-target
	t_response		*last_response;

	// Current substate of the notification
	string			current_substate;
	
	t_dialog *get_dialog(void) const;

public:
	t_sub_refer(t_dialog *_dialog, t_subscription_role _role);
	t_sub_refer(t_dialog *_dialog, t_subscription_role _role,
		const string &_event_id);
	virtual ~t_sub_refer();

	// Send a NOTIFY with the status line of the response as body
	// substate indicates the subscription state of refer
	// A reason should be given if substate == TERMINATED
	void send_notify(t_response *r, const string &substate,
		const string reason = "");

	bool recv_notify(t_request *r, t_tuid tuid, t_tid tid);
	bool recv_subscribe(t_request *r, t_tuid tuid, t_tid tid);

	bool timeout(t_subscribe_timer timer);

	t_sub_refer_result get_sr_result(void) const;
};

#endif
