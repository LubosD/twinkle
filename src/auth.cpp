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

#include <cassert>
#include "auth.h"
#include "log.h"
#include "protocol.h"
#include "user.h"
#include "userintf.h"
#include "util.h"

extern string		user_host;

t_cr_cache_entry:: t_cr_cache_entry(const t_url &_to, const t_credentials &_cr,
	const string &_passwd, bool _proxy) :
		to(_to)
{
	credentials = _cr;
	passwd = _passwd;
	proxy = _proxy;
}


list<t_cr_cache_entry>::iterator t_auth::find_cache_entry(
	const t_url &_to, const string &realm, bool proxy)
{
	for (list<t_cr_cache_entry>::iterator i = cache.begin();
	     i != cache.end(); i++)
	{
		// RFC 3261 22.1
		// Only the realm determines the protection space.
		// So the to-uri must not need to be compared as for HTTP
		// i.e. check i->to == _to must not be done.
		// As realm strings are globally unique there is no need
		// to check if the credentials are for a 407 or 401 response.
		// i.e. check i->proxy == proxy is not needed.
		if (i->credentials.digest_response.realm == realm)
		{
			return i;
		}
	}

	return cache.end();
}

void t_auth::update_cache(const t_url &to, const t_credentials &cr,
	const string &passwd, bool proxy)
{
	list<t_cr_cache_entry>::iterator i, j;

	i = find_cache_entry(to, cr.digest_response.realm, proxy);

	if (i == cache.end()) {
		if (cache.size() > AUTH_CACHE_SIZE) {
			cache.erase(cache.begin());
		}
		cache.push_back(t_cr_cache_entry(to, cr, passwd, proxy));
	} else {
		i->credentials = cr;
		i->passwd = passwd;

		// Move cache entry to end of the cache.
		// TODO: this can be more efficient by checking if the
		//       entry is already at the end.
		t_cr_cache_entry e = *i;
		cache.erase(i);
		cache.push_back(e);
	}
}

bool t_auth::auth_failed(t_request *r, const t_challenge &c,
	bool proxy) const
{
	if (c.digest_challenge.stale) {
		log_file->write_report("Stale nonce value.", "t_auth::auth_failed");
		return false;
	}

	if (proxy) {
		return r->hdr_proxy_authorization.contains(
			c.digest_challenge.realm, r->uri);
	} else {
		return r->hdr_authorization.contains(
			c.digest_challenge.realm, r->uri);
	}
}

void t_auth::remove_credentials(t_request *r, const t_challenge &c,
	bool proxy) const
{
	if (proxy) {
		r->hdr_proxy_authorization.remove_credentials(
			c.digest_challenge.realm, r->uri);
	} else {
		r->hdr_authorization.remove_credentials(
			c.digest_challenge.realm, r->uri);
	}
}

t_auth::t_auth() {
	re_register = false;
}

bool t_auth::authorize(t_user *user_config, t_request *r, t_response *resp) {
	string username;
	string passwd;
	list<t_cr_cache_entry>::iterator i;
	t_challenge c;
	bool proxy;

	assert(resp->must_authenticate());

	if (resp->code == R_401_UNAUTHORIZED) {
		c = resp->hdr_www_authenticate.challenge;
		proxy = false;
	} else {
		c = resp->hdr_proxy_authenticate.challenge;
		proxy = true;
	}

	// Only DIGEST is supported
	if (c.auth_scheme != AUTH_DIGEST) {
		log_file->write_header("t_auth::authorize");
		log_file->write_raw("Unsupported authentication scheme: ");
		log_file->write_raw(c.auth_scheme);
		log_file->write_endl();
		log_file->write_footer();
		return false;
	}

	const t_digest_challenge &dc = c.digest_challenge;
	i = find_cache_entry(r->uri, dc.realm, proxy);

	if (auth_failed(r, c, proxy)) {
		// The current credentials are wrong. Remove them and
		// ask the user for a username and password.
		remove_credentials(r, c, proxy);
	} else {
		// Determine user name and password
		if (i != cache.end()) {
			username = i->credentials.digest_response.username;
			passwd = i->passwd;
		} else if (dc.realm == user_config->get_auth_realm() ||
		           user_config->get_auth_realm() == "") {
			username = user_config->get_auth_name();
			passwd = user_config->get_auth_pass();
		}

		if (dc.stale) {
			// The current credentials are stale. Remove them.
			remove_credentials(r, c, proxy);
		}
	}

	// Ask user for username/password
	if ((username == "" || passwd == "") && !re_register) {
		if (!ui->cb_ask_credentials(user_config, dc.realm, username, passwd)) {
			log_file->write_report("Asking user name and password failed.",
						"t_auth::authorize");
			return false;
		}
	}

	// No valid username/passwd
	if (username == "" && passwd == "") {
		log_file->write_report("Incorrect user name and/or password.",
						"t_auth::authorize");
		return false;
	}
	
	bool auth_success;
	string fail_reason;
	if (!proxy) {
		t_credentials cr;
		auth_success = r->www_authorize(c, user_config,
			username, passwd, 1, NEW_CNONCE, cr, fail_reason);

		if (auth_success) {
			update_cache(r->uri, cr, passwd, proxy);
		}
	} else {
		t_credentials cr;
		auth_success = r->proxy_authorize(c, user_config,
			username, passwd, 1, NEW_CNONCE, cr, fail_reason);

		if (auth_success) {
			update_cache(r->uri, cr, passwd, proxy);
		}
	}

	if (!auth_success) {
		log_file->write_report(fail_reason, "t_auth::authorize");
		return false;
	}

	return true;
}

void t_auth::remove_from_cache(const string &realm) {
	if (realm.empty()) {
		cache.clear();
	} else {	
		list<t_cr_cache_entry>::iterator i = find_cache_entry(t_url(), realm);
		if (i != cache.end()) {
			cache.erase(i);
		}
	}
}

void t_auth::set_re_register(bool on) {
	re_register = on;
}

bool t_auth::get_re_register(void) const {
	return re_register;
}
