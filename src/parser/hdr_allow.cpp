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

#include <algorithm>

#include "definitions.h"
#include "hdr_allow.h"

using namespace std;

t_hdr_allow::t_hdr_allow() : t_header("Allow") {}

void t_hdr_allow::add_method(const t_method &m, const string &unknown) {
	populated = true;

	if (m != METHOD_UNKNOWN) {
		method_list.push_back(m);
	} else {
		unknown_methods.push_back(unknown);
	}
}

void t_hdr_allow::add_method(const string &s) {
	populated = true;

	t_method m = str2method(s);
	if (m != METHOD_UNKNOWN) {
		method_list.push_back(m);
	} else {
		unknown_methods.push_back(s);
	}
}

bool t_hdr_allow::contains_method(const t_method &m) const {
	return (find(method_list.begin(), method_list.end(), m) != method_list.end());
}

string t_hdr_allow::encode_value(void) const {
	string s;

	if (!populated) return s;

	for (list<t_method>::const_iterator i = method_list.begin();
	     i != method_list.end(); i++)
	{
		if (i != method_list.begin()) s += ",";
		s += method2str(*i);
	}

	for (list<string>::const_iterator i = unknown_methods.begin();
	     i != unknown_methods.end(); i++)
	{
		if (i != unknown_methods.begin() || method_list.size() != 0) {
			s += ",";
		}
		s += *i;
	}

	return s;
}
