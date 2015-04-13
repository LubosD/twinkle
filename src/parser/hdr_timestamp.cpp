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
#include "hdr_timestamp.h"
#include "util.h"

t_hdr_timestamp::t_hdr_timestamp() : t_header("Timestamp") {
	timestamp = 0;
	delay = 0;
}

void t_hdr_timestamp::set_timestamp(float t) {
	populated = true;
	timestamp = t;
}

void t_hdr_timestamp::set_delay(float d) {
	populated = true;
	delay = d;
}

string t_hdr_timestamp::encode_value(void) const {
	string s;

	if (!populated) return s;

	s += float2str(timestamp, 3);

	if (delay != 0) {
		s += " ";
		s += float2str(delay, 3);
	}

	return s;
}
