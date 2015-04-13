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

#include <cassert>
#include <cstdlib>

#include "media_type.h"
#include "util.h"
#include "utils/mime_database.h"

using namespace std;
using namespace utils;

t_media::t_media() : q(1.0) {}

t_media::t_media(const string &t, const string &s) :
	type(t),
	subtype(s),
	q(1.0)
{}

t_media::t_media(const string &mime_type) : q(1.0) 
{
	vector<string> v = split(mime_type, '/');
	
	if (v.size() == 2) {
		type = v[0];
		subtype = v[1];
	}
}

void t_media::add_params(const list<t_parameter> &l) {
	list<t_parameter>::const_iterator i = l.begin();

	media_param_list.clear();
	accept_extension_list.clear();

	// Add media parameters
	while (i != l.end() && i->name != "q") {
		if (i->name == "charset") {
			charset = i->value;
		} else {
			media_param_list.push_back(*i);
		}
		++i;
	}

	// Set the quality factor
	if (i != l.end()) {
		q = atof(i->value.c_str());
		i++;
	}

	// Add accept extension parameters
	while (i != l.end()) {
		accept_extension_list.push_back(*i);
		i++;
	}
}


string t_media::encode(void) const {
	string s;

	s = type + '/' + subtype;
	if (!charset.empty()) {
		s += ";charset=";
		s += charset;
	}
	s += param_list2str(media_param_list);
	
	if (q != 1) {
		s += ";q=";
		s += float2str(q, 1);
	}
	
	s += param_list2str(accept_extension_list);

	return s;
}

string t_media::get_file_glob(void) const {
	string file_glob = mime_database->get_glob(type + '/' + subtype);
	return file_glob;
}
