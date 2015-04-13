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

// Refer-Sub header
// RFC 4488

#ifndef _H_HDR_REFER_SUB
#define _H_HDR_REFER_SUB

#include <list>
#include <string>

#include "header.h"
#include "parameter.h"

using namespace std;

class t_hdr_refer_sub : public t_header {
public:
	bool			create_refer_sub;
	list<t_parameter>	extensions;
	
	t_hdr_refer_sub();
	void set_create_refer_sub(bool on);
	void add_extension(const t_parameter &p);
	void set_extensions(const list<t_parameter> &l);

	string encode_value(void) const;
};

#endif
