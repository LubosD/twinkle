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

#include "hdr_in_reply_to.h"
#include "definitions.h"

t_hdr_in_reply_to::t_hdr_in_reply_to() : t_header("In-Reply-To") {};

void t_hdr_in_reply_to::add_call_id(const string &id) {
	populated = true;
	call_ids.push_back(id);
}

string t_hdr_in_reply_to::encode_value(void) const {
	string s;

	if (!populated) return s;

	for (list<string>::const_iterator i = call_ids.begin();
	     i != call_ids.end(); i++)
	{
		if (i != call_ids.begin()) s += ", ";
		s += *i;
	}

	return s;
}
