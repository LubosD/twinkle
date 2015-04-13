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

// Authorization header

#ifndef _HDR_AUTHORIZATION_H
#define _HDR_AUTHORIZATION_H

#include <list>
#include <string>
#include "credentials.h"
#include "header.h"
#include "parameter.h"
#include "sockets/url.h"

using namespace std;

class t_hdr_authorization : public t_header {
public:
	list<t_credentials>	credentials_list;

	t_hdr_authorization();

	void add_credentials(const t_credentials &c);
	string encode(void) const;
	string encode_value(void) const;

	// Return true if the header contains credentials for a realm/dest
	bool contains(const string &realm, const t_url &uri) const;

	// Remove credentials for a realm/dest
	void remove_credentials(const string &realm, const t_url &uri);
};

#endif

