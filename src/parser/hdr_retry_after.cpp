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

#include "definitions.h"
#include "hdr_retry_after.h"
#include "util.h"

t_hdr_retry_after::t_hdr_retry_after() : t_header("Retry-After") {
	time = 0;
	duration = 0;
}

void t_hdr_retry_after::set_time(unsigned long t) {
	populated = true;
	time = t;
}

void t_hdr_retry_after::set_comment(const string &c) {
	populated = true;
	comment = c;
}

void t_hdr_retry_after::set_duration(unsigned long d) {
	populated = true;
	duration = d;
}

void t_hdr_retry_after::add_param(const t_parameter &p) {
	populated = true;
	params.push_back(p);
}

string t_hdr_retry_after::encode_value(void) const {
	string s;

	if (!populated) return s;

	s = ulong2str(time);

	if (comment.size() > 0) {
		s += " (";
		s += comment;
		s += ')';
	}

	if (duration > 0) {
		s += ";duration=";
		s += ulong2str(duration);
	}

	s += param_list2str(params);

	return s;
}
