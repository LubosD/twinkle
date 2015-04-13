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

// Credentials are used in Proxy-Authorization and
// Authorization headers

#ifndef _CREDENTIALS_H
#define _CREDENTIALS_H

#include <list>
#include <string>
#include "parameter.h"
#include "sockets/url.h"

using namespace std;

class t_digest_response {
public:
	string			username;
	string			realm;
	string			nonce;
	t_url			digest_uri;
	string			dresponse;
	string			algorithm;
	string			cnonce;
	string			opaque;
	string			message_qop;
	unsigned long		nonce_count;
	list<t_parameter>	auth_params;

	t_digest_response();

	// Set one of the attributes to a value. The parameter p
	// indicated wich attribute (p.name) should be set to
	// which value (p.value).
	// Returns false if p does not contain a valid attribute
	// setting.
	bool set_attr(const t_parameter &p);

	string encode(void) const;
};

class t_credentials {
public:
	string			auth_scheme;
	t_digest_response	digest_response;

	// auth_params is used when auth_scheme is not Digest.
	list<t_parameter>	auth_params;

	string encode(void) const;
};

#endif
