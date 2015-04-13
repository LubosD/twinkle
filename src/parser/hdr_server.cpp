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
#include "hdr_server.h"
#include "util.h"

t_server::t_server() {}

t_server::t_server(const string &_product, const string &_version,
		 const string &_comment)
{
	product = _product;
	version = _version;
	comment = _comment;
}

string t_server::encode(void) const {
	string s;

	s = product;

	if (version.size() > 0) {
		s += '/';
		s += version;
	}

	if (comment.size() > 0) {
		if (s.size() > 0) s += ' ';
		s += "(";
		s += comment;
		s += ')';
	}

	return s;
}

t_hdr_server::t_hdr_server() : t_header("Server") {};

void t_hdr_server::add_server(const t_server &s) {
	populated = true;
	server_info.push_back(s);
}

string t_hdr_server::get_server_info(void) const {
	string s;
	
	for (list<t_server>::const_iterator i = server_info.begin();
	     i != server_info.end(); i++ )
	{
		if (i != server_info.begin()) s += ' ';
		s += i->encode();
	}
	
	return s;
}

string t_hdr_server::encode_value(void) const {
	if (!populated) return "";

	return get_server_info();
}
