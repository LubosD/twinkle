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

// Supported header

#ifndef _H_HDR_SUPPORTED
#define _H_HDR_SUPPORTED

#include <list>
#include <string>
#include "header.h"

#define EXT_100REL	"100rel"	// RFC 3262
#define EXT_REPLACES	"replaces"	// RFC 3891
#define EXT_NOREFERSUB	"norefersub"	// RFC 4488

class t_hdr_supported : public t_header {
public:
	list<string>	features;

	t_hdr_supported();
	void add_feature(const string &f);
	void add_features(const list<string> &l);

	// Clear the list of features, but make the header 'populated'.
	// An empty header will be in the message.
	void set_empty(void);

	bool contains(const string &f) const;
	
	string encode_value(void) const;
};

#endif
