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

#include "parameter.h"
#include "util.h"

t_parameter::t_parameter() {
	type = VALUE;
}

t_parameter::t_parameter(const string &n) {
	type = NOVALUE;
	name = n;
}

t_parameter::t_parameter(const string &n, const string &v) {
	type = VALUE;
	name = n;
	value = v;
}

string t_parameter::encode(void) const {
	string s;

	s += name;

	if (type == VALUE) {
		s += '=';
		if (must_quote(value)) {
			s += '\"' + value + '\"';
		} else {
			s += value;
		}
	}

	return s;
}

bool t_parameter::operator==(const t_parameter &rhs) {
	return (type == rhs.type && name == rhs.name);
}

t_parameter str2param(const string &s) {
	vector<string> l = split_on_first(s, '=');
	if (l.size() == 1) {
		return t_parameter(s);
	} else {
		return t_parameter(trim(l[0]), trim(l[1]));
	}
}

string param_list2str(const list<t_parameter> &l) {
	string s;

	for (list<t_parameter>::const_iterator i = l.begin();
	     i != l.end(); i++)
	{
		s += ';';
		s += i->encode();
	}

	return s;
}

list<t_parameter> str2param_list(const string &s) {
	list<t_parameter> result;
	
	vector<string> l = split(s, ';');
	for (vector<string>::const_iterator i = l.begin(); i != l.end(); i++) {
		t_parameter p = str2param(trim(*i));
		result.push_back(p);
	}
	
	return result;
}
