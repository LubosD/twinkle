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
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "hdr_refer_sub.h"

t_hdr_refer_sub::t_hdr_refer_sub() : t_header("Refer-Sub"),
	create_refer_sub(true)
{}

void t_hdr_refer_sub::set_create_refer_sub(bool on) {
	populated = true;
	create_refer_sub = on;
}

void t_hdr_refer_sub::add_extension(const t_parameter &p) {
	populated = true;
	extensions.push_back(p);
}

void t_hdr_refer_sub::set_extensions(const list<t_parameter> &l) {
	populated = true;
	extensions = l;
}

string t_hdr_refer_sub::encode_value(void) const {
	string s;

	if (!populated) return s;
	
	s = (create_refer_sub ? "true" : "false");
	s += param_list2str(extensions);
	
	return s;
}
