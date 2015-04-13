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

#include "hdr_accept.h"
#include "definitions.h"

t_hdr_accept::t_hdr_accept() : t_header("Accept") {};

void t_hdr_accept::add_media(const t_media &media) {
	populated = true;
	media_list.push_back(media);
}

void t_hdr_accept::set_empty(void) {
	populated = true;
	media_list.clear();
}

string t_hdr_accept::encode_value(void) const {
	string s;
	
	if (!populated) return s;
	
	for (list<t_media>::const_iterator i = media_list.begin();
	     i != media_list.end(); i++)
	{
		if (i != media_list.begin()) s += ",";
		s += i->encode();
	}
	
	return s;
}
