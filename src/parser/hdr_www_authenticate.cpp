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
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "hdr_www_authenticate.h"
#include "definitions.h"
#include "util.h"

t_hdr_www_authenticate::t_hdr_www_authenticate() : t_header("WWW-Authenticate") {}

void t_hdr_www_authenticate::set_challenge(const t_challenge &c) {
	// The server may send multiple WWW-Authenticate/Proxy-Authenticate
	// headers, with different digest algorithms, in decreasing order of
	// preference.  We must therefore avoid overwriting any supported
	// challenge once we've got a hold of one.  (We don't simply ignore
	// all unsupported challenges, however, just in case the server forgot
	// to include a Digest challenge.)
	if (populated) {
		// Don't overwrite the previous challenge if it was supported
		if (cmp_nocase(challenge.auth_scheme, AUTH_DIGEST) == 0) {
			return;
		}
	}
	populated = true;
	challenge = c;
}

string t_hdr_www_authenticate::encode_value(void) const {
	if (!populated) return "";

	return challenge.encode();
}
