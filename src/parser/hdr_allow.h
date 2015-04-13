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

// Allow header

#ifndef _HDR_ALLOW_H
#define _HDR_ALLOW_H

#include <list>
#include <string>
#include "header.h"
#include "definitions.h"

using namespace std;

class t_hdr_allow : public t_header {
public:
	list<t_method> method_list;

	// Unknown methods are represented as strings
	list<string> unknown_methods;

	t_hdr_allow();
	void add_method(const t_method &m, const string &unknown = "");
	void add_method(const string &s);
	bool contains_method(const t_method &m) const;
	string encode_value(void) const;
};

#endif
