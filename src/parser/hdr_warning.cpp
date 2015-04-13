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

#include "definitions.h"
#include "hdr_warning.h"
#include "util.h"

t_warning::t_warning() {
	code = 0;
	port = 0;
}

t_warning::t_warning(const string &_host, int _port, int _code, string _text) {
	host = _host;
	port = _port;
	code = _code;

	switch(code) {
	case 300: text = WARNING_300; break;
	case 301: text = WARNING_301; break;
	case 302: text = WARNING_302; break;
	case 303: text = WARNING_303; break;
	case 304: text = WARNING_304; break;
	case 305: text = WARNING_305; break;
	case 306: text = WARNING_306; break;
	case 307: text = WARNING_307; break;
	case 330: text = WARNING_330; break;
	case 331: text = WARNING_331; break;
	case 370: text = WARNING_370; break;
	case 399: text = WARNING_399; break;
	default: text = "Warning";
	}

	if (_text != "") {
		text += ": ";
		text += _text;
	}
}

string t_warning::encode(void) const {
	string s;

	s = int2str(code, "%3d");
	s += ' ';
	s += host;
	if (port > 0) s += int2str(port, ":%d");
	s += ' ';
	s += '"';
	s += text;
	s += '"';
	return s;
}

t_hdr_warning::t_hdr_warning() : t_header("Warning") {}

void t_hdr_warning::add_warning(const t_warning &w) {
	populated = true;
	warnings.push_back(w);
}

string t_hdr_warning::encode_value(void) const {
	string s;

	if (!populated) return s;

	for (list<t_warning>::const_iterator i = warnings.begin();
	     i != warnings.end(); i++)
	{
		if (i != warnings.begin()) s += ", ";
		s += i->encode();
	}

	return s;
}
