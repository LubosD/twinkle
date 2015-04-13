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

// Retry-After header

#ifndef _H_HDR_RETRY_AFTER
#define _H_HDR_RETRY_AFTER

#include <list>
#include <string>
#include "header.h"
#include "parameter.h"

using namespace std;

class t_hdr_retry_after : public t_header {
public:
	unsigned long		time; // in seconds
	string			comment;
	unsigned long		duration; // in seconds
	list<t_parameter>	params;

	t_hdr_retry_after();
	void set_time(unsigned long t);
	void set_comment(const string &c);
	void set_duration(unsigned long d);
	void add_param(const t_parameter &p);
	string encode_value(void) const;
};

#endif
