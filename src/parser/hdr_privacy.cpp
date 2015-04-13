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
#include "hdr_privacy.h"

using namespace std;

t_hdr_privacy::t_hdr_privacy() : t_header("Privacy") {};

void t_hdr_privacy::add_privacy(const string &privacy) {
	populated = true;
	privacy_list.push_back(privacy);
}

bool t_hdr_privacy::contains_privacy(const string &privacy) const {
	return (find(privacy_list.begin(), privacy_list.end(), privacy) !=
			privacy_list.end());
}

string t_hdr_privacy::encode_value(void) const {
	string s;

	if (!populated) return s;

	for (list<string>::const_iterator i = privacy_list.begin();
	     i != privacy_list.end(); i++)
	{
		if (i != privacy_list.begin()) s += ";";
		s += *i;
	}
	
	return s;
}
