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
#include "hdr_call_info.h"

void t_info_param::add_param(const t_parameter &p) {
	parameter_list.push_back(p);
}

string t_info_param::encode(void) const {
	string s;

	s = '<' + uri.encode() + '>';
	s += param_list2str(parameter_list);

	return s;
}


t_hdr_call_info::t_hdr_call_info() : t_header("Call-Info") {};

void t_hdr_call_info::add_param(const t_info_param &p) {
	populated = true;
	info_param_list.push_back(p);
}

string t_hdr_call_info::encode_value(void) const {
	string s;

	if (!populated) return s;

	for (list<t_info_param>::const_iterator i = info_param_list.begin();
	     i != info_param_list.end(); i++)
	{
		if (i != info_param_list.begin()) s += ", ";
		s += i->encode();
	}
	
	return s;
}
