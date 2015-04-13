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

// RFC 3323
// Privacy header

#ifndef _H_HDR_PRIVACY
#define _H_HDR_PRICACY

#include <list>
#include <string>
#include "header.h"

#define PRIVACY_HEADER		"header"
#define PRIVACY_SESSION		"session"
#define PRIVACY_USER		"user"
#define PRIVACY_NONE		"none"
#define PRIVACY_CRITICAL	"critical"

// RFC 3325 9.3 defines id privacy
#define PRIVACY_ID		"id"

class t_hdr_privacy : public t_header {
public:
	list<string>	privacy_list;
	
	t_hdr_privacy();
	void add_privacy(const string &privacy);
	bool contains_privacy(const string &privacy) const;
	string encode_value(void) const;
};

#endif
