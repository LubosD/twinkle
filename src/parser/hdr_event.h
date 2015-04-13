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

// Event header
// RFC 3265

#ifndef _HDR_EVENT
#define _HDR_EVENT

#include <list>
#include <string>
#include "header.h"
#include "parameter.h"

#define SIP_EVENT_REFER		"refer"			// RFC 3515
#define SIP_EVENT_MSG_SUMMARY	"message-summary"	// RFC 3842
#define SIP_EVENT_PRESENCE	"presence"		// RFC 3856

using namespace std;

class t_hdr_event : public t_header {
public:
	// The event_type attribute contains the event-template as well
	// if present, e.g. event.template
	string			event_type;
	string			id;
	list<t_parameter>	event_params;

	t_hdr_event();
	void set_event_type(const string &t);
	void set_id(const string &s);
	void add_event_param(const t_parameter &p);

	string encode_value(void) const;
};

#endif
