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
#include "hdr_unsupported.h"

t_hdr_unsupported::t_hdr_unsupported() : t_header("Unsupported") {};

void t_hdr_unsupported::add_feature(const string &f) {
	populated = true;
	features.push_back(f);
}

void t_hdr_unsupported::set_features(const list<string> &_features) {
	populated = true;
	features = _features;
}

bool t_hdr_unsupported::contains(const string &f) const {
	if (!populated) return false;

	for (list<string>::const_iterator i = features.begin();
	     i != features.end(); i++)
	{
		if (*i == f) return true;
	}

	return false;
}

string t_hdr_unsupported::encode_value(void) const {
	string s;

	if (!populated) return s;

	for (list<string>::const_iterator i = features.begin();
	     i != features.end(); i++)
	{
		if (i != features.begin()) s += ",";
		s += *i;
	}

	return s;
}
