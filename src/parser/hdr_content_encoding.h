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

// Content-Encoding header

#ifndef _HDR_CONTENT_ENCODING_H
#define _HDR_CONTENT_ENCODING_H

#include <list>
#include <string>
#include "coding.h"
#include "header.h"

using namespace std;

class t_hdr_content_encoding : public t_header {
public:
	list<t_coding> coding_list; // list of content codings;

	t_hdr_content_encoding();

	// Add a coding to the list of content codings
	void add_coding(const t_coding &coding);

	string encode_value(void) const;
};

#endif
