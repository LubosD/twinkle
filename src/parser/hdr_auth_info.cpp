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

#include "hdr_auth_info.h"
#include "definitions.h"
#include "util.h"

t_hdr_auth_info::t_hdr_auth_info() : t_header("Authentication-Info") {
	nonce_count = 0;
}

void t_hdr_auth_info::set_next_nonce(const string &nn) {
	populated = true;
	next_nonce = nn;
}

void t_hdr_auth_info::set_message_qop(const string &mq) {
	populated = true;
	message_qop = mq;
}

void t_hdr_auth_info::set_response_auth(const string &ra) {
	populated = true;
	response_auth = ra;
}

void t_hdr_auth_info::set_cnonce(const string &cn) {
	populated = true;
	cnonce = cn;
}

void t_hdr_auth_info::set_nonce_count(const unsigned long &nc) {
	populated = true;
	nonce_count = nc;
}

string t_hdr_auth_info::encode_value(void) const {
	string s;
	bool add_comma = false;

	if (!populated) return s;

	if (next_nonce.size() > 0) {
		s += "nextnonce=";
		s += '"';
		s += next_nonce;
		s += '"';
		add_comma = true;
	}

	if (message_qop.size() > 0) {
		if (add_comma) s += ',';
		s += "qop=";
		s += message_qop;
		add_comma = true;
	}

	if (response_auth.size() > 0) {
		if (add_comma) s += ',';
		s += "rspauth=";
		s += '"';
		s += response_auth;
		s += '"';
		add_comma = true;
	}

	if (cnonce.size() > 0) {
		if (add_comma) s += ',';
		s += "cnonce=";
		s += '"';
		s += cnonce;
		s += '"';
		add_comma = true;
	}

	if (nonce_count > 0) {
		if (add_comma) s += ',';
		s += "nc=";
		s += ulong2str(nonce_count, "%08x");
		add_comma = true;
	}

	return s;
}
