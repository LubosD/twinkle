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
#include "route.h"
#include "parse_ctrl.h"
#include "util.h"

void t_route::add_param(const t_parameter &p) {
	params.push_back(p);
}

void t_route::set_params(const list<t_parameter> &l) {
	params = l;
}

string t_route::encode(void) const {
	string s;

	if (display.size() > 0) {
		s += '"';
		s += escape(display, '"');
		s += '"';
		s += ' ';
	}

	s += '<';
	s += uri.encode();
	s += '>';

	s += param_list2str(params);
	return s;
}
