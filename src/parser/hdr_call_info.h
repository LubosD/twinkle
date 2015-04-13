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

// Call-Info header

#ifndef _HDR_CALL_INFO_H
#define _HDR_CALL_INFO_H

#include <list>
#include <string>
#include "header.h"
#include "parameter.h"
#include "sockets/url.h"

using namespace std;

class t_info_param {
public:
	t_url uri;
	list<t_parameter> parameter_list;

	void add_param(const t_parameter &p);
	string encode(void) const;
};

class t_hdr_call_info : public t_header {
public:
	list<t_info_param> info_param_list;

	t_hdr_call_info();

	// Add a paramter to the list of alert parameters
	void add_param(const t_info_param &p);

	string encode_value(void) const;
};

#endif
