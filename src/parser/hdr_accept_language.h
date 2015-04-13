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

// Accept_Language header

#ifndef _HDR_ACCEPT_LANGUAGE_H
#define _HDR_ACCEPT_LANGUAGE_H

#include <list>
#include <string>
#include "header.h"

using namespace std;

class t_language {
public:
	string language;
	float q; // quality factor

	t_language();
	t_language(const string &l);
	string encode(void) const;
};

class t_hdr_accept_language : public t_header {
public:
	list<t_language> language_list;	// list of accepted languages

	t_hdr_accept_language();

	// Add a language to the list of accepted media
	void add_language(const t_language &language);

	string encode_value(void) const;
};

#endif
