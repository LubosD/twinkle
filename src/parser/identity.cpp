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

#include "identity.h"
#include "util.h"

t_identity::t_identity() : display(), uri() {}

void t_identity::set_display(const string &d) {
	display = d;
}

void t_identity::set_uri(const string &u) {
	uri.set_url(u);
}

void t_identity::set_uri(const t_url &u) {
	uri = u;
}

string t_identity::encode(void) const {
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

	return s;
}
