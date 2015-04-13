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

#include "credentials.h"
#include "definitions.h"
#include "util.h"

t_digest_response::t_digest_response() {
	nonce_count = 0;
}

string t_digest_response::encode(void) const {
	string s;

	if (username.size() > 0) {
		s += "username=";
		s += '"';
		s += username;
		s += '"';
	}

	if (realm.size() > 0) {
		if (s.size() > 0) s += ',';
		s += "realm=";
		s += '"';
		s += realm;
		s += '"';
	}

	if (nonce.size() > 0) {
		if (s.size() > 0) s += ',';
		s += "nonce=";
		s += '"';
		s += nonce;
		s += '"';
	}

	if (digest_uri.is_valid()) {
		if (s.size() > 0) s += ',';
		s += "uri=";
		s += '"';
		s += digest_uri.encode();
		s += '"';
	}

	if (dresponse.size() > 0) {
		if (s.size() > 0) s += ',';
		s += "response=";
		s += '"';
		s += dresponse;
		s += '"';
	}

	if (algorithm.size() > 0) {
		if (s.size() > 0) s += ',';
		s += "algorithm=";
		s += algorithm;
	}

	if (cnonce.size() > 0) {
		if (s.size() > 0) s += ',';
		s += "cnonce=";
		s += '"';
		s += cnonce;
		s += '"';
	}

	if (opaque.size() > 0) {
		if (s.size() > 0) s += ',';
		s += "opaque=";
		s += '"';
		s += opaque;
		s += '"';
	}

	if (message_qop.size() > 0) {
		if (s.size() > 0) s += ',';
		s += "qop=";
		s += message_qop;
	}

	if (nonce_count > 0) {
		if (s.size() > 0) s += ',';
		s += "nc=";
		s += ulong2str(nonce_count, "%08x");
	}

	for (list<t_parameter>::const_iterator i = auth_params.begin();
	     i != auth_params.end(); i++)
	{
		if (s.size() > 0) s += ',';
		s += i->encode();
	}

	return s;
}

bool t_digest_response::set_attr(const t_parameter &p) {
	if (p.name == "username")
		username = p.value;
	else if (p.name == "realm")
		realm = p.value;
	else if (p.name == "nonce")
		nonce = p.value;
	else if (p.name == "digest_uri") {
		digest_uri.set_url(p.value);
		if (!digest_uri.is_valid()) return false;
	}
	else if (p.name == "response")
		dresponse = p.value;
	else if (p.name == "cnonce")
		cnonce = p.value;
	else if (p.name == "opaque")
		opaque = p.value;
	else if (p.name == "algorithm")
		algorithm = p.value;
	else if (p.name == "qop")
		message_qop = p.value;
	else if (p.name == "nc")
		nonce_count = hex2int(p.value);
	else
		auth_params.push_back(p);

	return true;
}

string t_credentials::encode(void) const {
	string s = auth_scheme;
	s += ' ';

	if (auth_scheme == AUTH_DIGEST) {
		s += digest_response.encode();
	} else {
		for (list<t_parameter>::const_iterator i = auth_params.begin();
	     		i != auth_params.end(); i++)
		{
			if (i != auth_params.begin()) s += ',';
			s += i->encode();
		}
	}

	return s;
}
