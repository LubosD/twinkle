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

#include "hdr_event.h"
#include "parse_ctrl.h"

t_hdr_event::t_hdr_event() : t_header("Event", "o") {}

void t_hdr_event::set_event_type(const string &t) {
	populated = true;
	event_type = t;
}

void t_hdr_event::set_id(const string &s) {
	populated = true;
	id = s;
}

void t_hdr_event::add_event_param(const t_parameter &p) {
	populated = true;
	event_params.push_back(p);
}

string t_hdr_event::encode_value(void) const {
	string s;

	if (!populated) return s;

	s += event_type;

	if (id.size() > 0) {
		s += ";id=";
		s += id;
	}

	s += param_list2str(event_params);

	return s;
}
