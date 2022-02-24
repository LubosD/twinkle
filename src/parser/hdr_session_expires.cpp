/*
    Copyright (C) 2005-2009  Michel de Boer <michel@twinklephone.com>
    Copyright (C) 2022       Frédéric Brière <fbriere@fbriere.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "definitions.h"
#include "hdr_session_expires.h"
#include "util.h"

t_hdr_session_expires::t_hdr_session_expires() :
	t_header("Session-Expires", "x"),
	time(0),
	refresher(REFRESHER_NONE)
{
}

void t_hdr_session_expires::set_time(unsigned long t) {
	populated = true;
	time = t;
}

void t_hdr_session_expires::set_refresher(t_refresher r) {
	refresher = r;
}

bool t_hdr_session_expires::set_refresher(const std::string &r) {
	if (r == SE_REFRESHER_UAS) {
		if (refresher == REFRESHER_UAC) return false;
		set_refresher(REFRESHER_UAS);
		return true;
	}

	if (r == SE_REFRESHER_UAC) {
		if (refresher == REFRESHER_UAS) return false;
		set_refresher(REFRESHER_UAC);
		return true;
	}

	return false;
}

void t_hdr_session_expires::add_param(const t_parameter &p) {
	params.push_back(p);
}

void t_hdr_session_expires::set_params(const std::list<t_parameter> &l) {
	params = l;
}

string t_hdr_session_expires::encode_value(void) const {
	if (!populated) return "";

	string s;

	std::list<t_parameter> p = params;

	std::string refresher_str;
	switch (refresher) {
		case REFRESHER_UAS:
			refresher_str = SE_REFRESHER_UAS;
			break;
		case REFRESHER_UAC:
			refresher_str = SE_REFRESHER_UAC;
			break;
	}
	if (!refresher_str.empty()) {
		t_parameter r("refresher", refresher_str);
		p.push_front(r);
	}

	s += ulong2str(time);
	s += param_list2str(p);

	return s;
}
