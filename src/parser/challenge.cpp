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

#include <iostream>
#include "challenge.h"
#include "definitions.h"
#include "log.h"
#include "util.h"

t_digest_challenge::t_digest_challenge() {
	stale = false;

	// The default algorithm is MD5.
	algorithm = ALG_MD5;
}

string t_digest_challenge::encode(void) const {
	string s;

	if (realm.size() > 0) {
		if (s.size() > 0) s += ',';
		s += "realm=";
		s += '"';
		s += realm;
		s += '"';
	}

	if (domain.size() > 0) {
		if (s.size() > 0) s += ',';
		s += "domain=";
		s += '"';

		for (list<t_url>::const_iterator i = domain.begin();
	     	     i != domain.end(); i++)
		{
			if (i != domain.begin()) s += ' ';
			s += i->encode();
		}

		s += '"';
	}

	if (nonce.size() > 0) {
		if (s.size() > 0) s += ',';
		s += "nonce=";
		s += '"';
		s += nonce;
		s += '"';
	}

	if (opaque.size() > 0) {
		if (s.size() > 0) s += ',';
		s += "opaque=";
		s += '"';
		s += opaque;
		s += '"';
	}

	// RFC 2617 3.2.1
	// If the stale flag is absent it means stale=false.
	if (stale) {
		if (s.size() > 0) s += ',';
		s += "stale=true";
	}

	if (algorithm.size() > 0) {
		if (s.size() > 0) s += ',';
		s += "algorithm=";
		s += algorithm;
	}

	if (qop_options.size() > 0) {
		if (s.size() > 0) s += ',';
		s += "qop=";
		s += '"';

		for (list<string>::const_iterator i = qop_options.begin();
	     	     i != qop_options.end(); i++)
		{
			if (i != qop_options.begin()) s += ',';
			s += *i;
		}

		s += '"';
	}

	for (list<t_parameter>::const_iterator i = auth_params.begin();
	     i != auth_params.end(); i++)
	{
		if (s.size() > 0) s += ',';
		s += i->encode();
	}

	return s;
}

bool t_digest_challenge::set_attr(const t_parameter &p) {
	if (p.name == "realm")
		realm = p.value;
	else if (p.name == "nonce")
		nonce = p.value;
	else if (p.name == "opaque")
		opaque = p.value;
	else if (p.name == "algorithm")
		algorithm = p.value;
	else if (p.name == "domain") {
		vector<string> l = split_ws(p.value);
		for (vector<string>::iterator i = l.begin();
		     i != l.end(); i++)
		{
			t_url u(*i);
			if (u.is_valid()) {
				domain.push_back(u);
			} else {
				log_file->write_header("t_digest_challenge::set_attr",
					LOG_SIP, LOG_WARNING);
				log_file->write_raw("Invalid domain in digest challenge: ");
				log_file->write_raw(*i);
				log_file->write_endl();
				log_file->write_footer();
			}
		}
	}
	else if (p.name == "qop") {
		vector<string> l = split(p.value, ',');
		for (vector<string>::iterator i = l.begin();
		     i != l.end(); i++)
		{
			qop_options.push_back(trim(*i));
		}
	}
	else if (p.name == "stale") {
		if (cmp_nocase(p.value, "true") == 0)
			stale = true;
		else
			// RFC 2617 3.2.1
			// Any value other than false should be interpreted
			// as false.
			stale = false;
	}
	else
		auth_params.push_back(p);

	return true;
}

string t_challenge::encode(void) const {
	string s = auth_scheme;
	s += ' ';

	if (auth_scheme == AUTH_DIGEST) {
		s += digest_challenge.encode();
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
