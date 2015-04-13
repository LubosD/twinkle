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

// Server header

#ifndef _H_HDR_SERVER
#define _H_HDR_SERVER

#include <list>
#include <string>
#include "header.h"

using namespace std;

class t_server {
public:
	string	product;
	string	version;
	string	comment;

	t_server();
	t_server(const string &_product, const string &_version,
		 const string &_comment = "");
	string encode(void) const;
};

class t_hdr_server : public t_header {
public:
	list<t_server>	server_info;

	t_hdr_server();
	void add_server(const t_server &s);
	
	// Get a string representation of server_info
	string get_server_info(void) const;
	
	string encode_value(void) const;
};

#endif
