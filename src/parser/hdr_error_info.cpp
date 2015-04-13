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
#include "hdr_error_info.h"

void t_error_param::add_param(const t_parameter &p) {
	parameter_list.push_back(p);
}

string t_error_param::encode(void) const {
	string s;

	s = '<' + uri.encode() + '>';
	s += param_list2str(parameter_list);

	return s;
}


t_hdr_error_info::t_hdr_error_info() : t_header("Error-Info") {};

void t_hdr_error_info::add_param(const t_error_param &p) {
	populated = true;
	error_param_list.push_back(p);
}

string t_hdr_error_info::encode_value(void) const {
	string s;

	if (!populated) return s;

	for (list<t_error_param>::const_iterator i = error_param_list.begin();
	     i != error_param_list.end(); i++)
	{
		if (i != error_param_list.begin()) s += ", ";
		s += i->encode();
	}

	return s;
}
