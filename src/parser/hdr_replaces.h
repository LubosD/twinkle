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

// Replaces header
// RFC 3891

#ifndef _H_HDR_REPLACES
#define _H_HDR_REPLACES

#include <string>
#include <list>
#include "header.h"
#include "parameter.h"

using namespace std;

class t_hdr_replaces : public t_header {
public:
	string			call_id;
	string			to_tag;
	string			from_tag;
	bool			early_only;
	list<t_parameter>	params;
	
	t_hdr_replaces();
	void set_call_id(const string &id);
	void set_to_tag(const string &tag);
	void set_from_tag(const string &tag);
	void set_early_only(const bool on);
	void set_params(const list<t_parameter> &l);
	void add_param(const t_parameter &p);
	string encode_value(void) const;
	bool is_valid(void) const;
};

#endif
