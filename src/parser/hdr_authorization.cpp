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

#include "hdr_authorization.h"
#include "definitions.h"

t_hdr_authorization::t_hdr_authorization() : t_header("Authorization") {}

void t_hdr_authorization::add_credentials(const t_credentials &c) {
	populated = true;
	credentials_list.push_back(c);
}

string t_hdr_authorization::encode(void) const {
	string s;

	if (!populated) return s;

	// RFC 3261 20.7
	// Each authorization should appear as a separate header
	for (list<t_credentials>::const_iterator i = credentials_list.begin();
	     i != credentials_list.end(); i++)
	{
		s += header_name;
		s += ": ";
		s += i->encode();
		s += CRLF;
	}

	return s;
}

string t_hdr_authorization::encode_value(void) const {
	string s;
	
	if (!populated) return s;
	
	for (list<t_credentials>::const_iterator i = credentials_list.begin();
	     i != credentials_list.end(); i++)
	{
		if (i != credentials_list.begin()) s += ", ";
		s += i->encode();
	}
	
	return s;
}

bool t_hdr_authorization::contains(const string &realm,
	const t_url &uri) const
{
	for (list<t_credentials>::const_iterator i = credentials_list.begin();
	     i != credentials_list.end(); i++)
	{
		if (i->digest_response.realm == realm &&
		    i->digest_response.digest_uri == uri)
		{
			return true;
		}
	}

	return false;
}

void t_hdr_authorization::remove_credentials(const string &realm,
	const t_url &uri)
{
	for (list<t_credentials>::iterator i = credentials_list.begin();
	     i != credentials_list.end(); i++)
	{
		if (i->digest_response.realm == realm &&
		    i->digest_response.digest_uri == uri)
		{
			credentials_list.erase(i);
			return;
		}
	}
}
