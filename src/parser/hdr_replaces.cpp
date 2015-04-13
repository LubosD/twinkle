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

#include "hdr_replaces.h"

t_hdr_replaces::t_hdr_replaces() : t_header("Replaces"),
	early_only(false)
{}

void t_hdr_replaces::set_call_id(const string &id) {
	populated = true;
	call_id = id;
}

void t_hdr_replaces::set_to_tag(const string &tag) {
	populated = true;
	to_tag = tag;
}

void t_hdr_replaces::set_from_tag(const string &tag) {
	populated = true;
	from_tag = tag;
}

void t_hdr_replaces::set_early_only(const bool on) {
	populated = true;
	early_only = on;
}

void t_hdr_replaces::set_params(const list<t_parameter> &l) {
	populated = true;
	params = l;
}

void t_hdr_replaces::add_param(const t_parameter &p) {
	populated = true;
	params.push_back(p);
}

string t_hdr_replaces::encode_value(void) const {
	string s;

	if (!populated) return s;
	
	s += call_id;
	s += ";to-tag=";
	s += to_tag;
	s += ";from-tag=";
	s += from_tag;
	
	if (early_only) {
		s += ";early-only";
	}
	
	s += param_list2str(params);
	
	return s;
}

bool t_hdr_replaces::is_valid(void) const {
	return !(call_id.empty() || to_tag.empty() || from_tag.empty());
}
