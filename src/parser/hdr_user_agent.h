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

// User-Agent header

#ifndef _H_HDR_USER_AGENT
#define _H_HDR_USER_AGENT

#include <list>
#include <string>
#include "header.h"
#include "hdr_server.h"

using namespace std;

class t_hdr_user_agent : public t_header {
public:
	list<t_server>	ua_info;

	t_hdr_user_agent();
	void add_server(const t_server &s);
	
	// Get string representation of ua_info;
	string get_ua_info(void) const;
	
	string encode_value(void) const;
};

#endif
