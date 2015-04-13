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

// To header

#ifndef _H_HDR_TO
#define _H_HDR_TO

#include <string>
#include "header.h"
#include "parameter.h"
#include "sockets/url.h"

using namespace std;

class t_hdr_to : public t_header {
public:
	string			display; // display name
	t_url			uri;
	string			tag;
	list<t_parameter>	params;

	t_hdr_to();
	void set_display(const string &d);
	void set_uri(const string &u);
	void set_uri(const t_url &u);
	void set_tag(const string &t);
	void set_params(const list<t_parameter> &l);
	void add_param(const t_parameter &p);
	string encode_value(void) const;
};

#endif
