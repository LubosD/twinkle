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

#ifndef _PARAMETER_H
#define _PARAMETER_H

#include <string>
#include <list>

using namespace std;

class t_parameter {
public:
enum t_param_type{
	NOVALUE,	// a parameter without a value
	VALUE		// parameter having a value (default)
};

	t_param_type	type;	// type of parameter
	string		name;	// name of parameter
	string		value;	// value of parameter if type is VALUE

	t_parameter();

	// Construct a NOVALUE parameter with name = n
	t_parameter(const string &n);

	// Construct a VALUE parameter with name = n, value = v
	t_parameter(const string &n, const string &v);

	string encode(void) const;
	
	bool operator==(const t_parameter &rhs);
};

// Decode a parameter
t_parameter str2param(const string &s);

// Encode a parameter list
string param_list2str(const list<t_parameter> &l);

// Decode a parameter list
list<t_parameter> str2param_list(const string &s);

#endif
