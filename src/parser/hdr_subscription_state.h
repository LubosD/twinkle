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

// Subscription-State header
// RFC 3265

#ifndef _HDR_SUBSCRIPTION_STATE
#define _HDR_SUBSCRIPTION_STATE

#include <list>
#include <string>
#include "header.h"
#include "parameter.h"

// Subscription states
#define SUBSTATE_ACTIVE		"active"
#define SUBSTATE_PENDING	"pending"
#define SUBSTATE_TERMINATED	"terminated"

// Event reasons
#define EV_REASON_DEACTIVATED	"deactivated"
#define EV_REASON_PROBATION	"probation"
#define EV_REASON_REJECTED	"rejected"
#define EV_REASON_TIMEOUT	"timeout"
#define EV_REASON_GIVEUP	"giveup"
#define EV_REASON_NORESOURCE	"noresource"

using namespace std;

class t_hdr_subscription_state : public t_header {
public:
	string			substate;
	string			reason;
	unsigned long		expires;
	unsigned long		retry_after;
	list<t_parameter>	extensions;

	t_hdr_subscription_state();
	void set_substate(const string &s);
	void set_reason(const string &s);
	void set_expires(unsigned long e);
	void set_retry_after(unsigned long r);
	void add_extension(const t_parameter &p);

	string encode_value(void) const;
};

#endif
