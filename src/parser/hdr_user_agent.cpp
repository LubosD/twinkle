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
#include "hdr_user_agent.h"
#include "util.h"

t_hdr_user_agent::t_hdr_user_agent() : t_header("User-Agent") {};

void t_hdr_user_agent::add_server(const t_server &s) {
	populated = true;
	ua_info.push_back(s);
}

string t_hdr_user_agent::get_ua_info(void) const {
	string s;
	
	for (list<t_server>::const_iterator i = ua_info.begin();
	     i != ua_info.end(); i++ )
	{
		if (i != ua_info.begin()) s += ' ';
		s += i->encode();
	}
	
	return s;	
}

string t_hdr_user_agent::encode_value(void) const {
	if (!populated) return "";

	return get_ua_info();
}
