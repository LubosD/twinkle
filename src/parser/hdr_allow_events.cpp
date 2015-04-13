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

#include "hdr_allow_events.h"
#include "parse_ctrl.h"

t_hdr_allow_events::t_hdr_allow_events() : t_header("Allow-Events", "u") {}

void t_hdr_allow_events::add_event_type(const string &t) {
	populated = true;
	event_types.push_back(t);
}

string t_hdr_allow_events::encode_value(void) const {
	string s;

	if (!populated) return s;

	for (list<string>::const_iterator i = event_types.begin();
	     i != event_types.end(); i++)
	{
		if (i != event_types.begin()) s += ",";
		s += *i;
	}

	return s;
}
