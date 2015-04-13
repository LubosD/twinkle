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
#include "hdr_subscription_state.h"
#include "util.h"

t_hdr_subscription_state::t_hdr_subscription_state() : t_header("Subscription-State") {
	expires = 0;
	retry_after = 0;
}

void t_hdr_subscription_state::set_substate(const string &s) {
	populated = true;
	substate = s;
}

void t_hdr_subscription_state::set_reason(const string &s) {
	populated = true;
	reason = s;
}

void t_hdr_subscription_state::set_expires(unsigned long e) {
	populated = true;
	expires = e;
}

void t_hdr_subscription_state::set_retry_after(unsigned long r) {
	populated = true;
	retry_after = r;
}

void t_hdr_subscription_state::add_extension(const t_parameter &p) {
	populated = true;
	extensions.push_back(p);
}

string t_hdr_subscription_state::encode_value(void) const {
	string s;

	if (!populated) return s;

	s = substate;

	if (reason.size() > 0) {
		s += ";reason=";
		s += reason;
	}

	if (expires > 0) {
		s += ";expires=";
		s += ulong2str(expires);
	}

	if (retry_after > 0) {
		s += ";retry-after=";
		s += ulong2str(retry_after);
	}

	s += param_list2str(extensions);

	return s;
}
