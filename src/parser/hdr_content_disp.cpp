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
#include "hdr_content_disp.h"

t_hdr_content_disp::t_hdr_content_disp() : t_header("Content-Disposition") {};

void t_hdr_content_disp::set_type(const string &t) {
	populated = true;
	type = t;
}

void t_hdr_content_disp::set_filename(const string &name) {
	populated = true;
	filename = name;
}

void t_hdr_content_disp::add_param(const t_parameter &p) {
	populated = true;
	params.push_back(p);
}

void t_hdr_content_disp::set_params(const list<t_parameter> &l) {
	populated = true;
	params = l;
}

string t_hdr_content_disp::encode_value(void) const {
	string s;

	if (!populated) return s;

	s = type;
	
	if (!filename.empty()) {
		s += ";filename=\"";
		s += filename;
		s += "\"";
	}
	
	s += param_list2str(params);

	return s;
}
