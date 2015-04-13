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

/**
 * @file
 * SIP authentication
 */

#ifndef _AUTH_H
#define _AUTH_H

#include "parser/credentials.h"
#include "parser/request.h"
#include "sockets/url.h"
#include <list>

using namespace std;

/** Size of the credentials cache. */
#define AUTH_CACHE_SIZE	50

/** Credentials cache entry. */
class t_cr_cache_entry {
public:
	/**
	 * Destination for which credentials are cached.
	 * This is not used for the SIP authentication itself.
	 */
	t_url		to;
	
	/** The credentials. */
	t_credentials	credentials;
	
	/** Password. */
	string 		passwd;
	
	/** Indicates if proxy authentication was requested. */
	bool		proxy;

	/** Constructor. */
	t_cr_cache_entry(const t_url &_to, const t_credentials &_cr,
		const string &_passwd, bool _proxy);
};


/** An object of this class authorizes a request given some credentials. */
class t_auth {
private:
	/** Indicates if the current registration request is a re-REGISTER. */
	bool re_register;

	/**
	 * LRU cache credentials for a destination.
	 * The first entry in the list is the least recently used.
	 */
	list<t_cr_cache_entry>	cache;

	/**
	 * Find a cache entry that matches the realm.
	 * @param _to [in] Destination for which authentication is needed.
	 * @param realm [in] The authentication realm.
	 * @param proxy [in] Indicates if proxy authentication was requested.
	 * @return An iterator to the cached credentials if found.
	 * @return The end iterator if not found.
	 */
	list<t_cr_cache_entry>::iterator find_cache_entry(const t_url &_to, 
		const string &realm, bool proxy=false);

	/**
	 * Update cached credentials.
	 * If the cache does not contain the credentials already
	 * then it will be added to the end of the list. If the cache
	 * already contains the maximum number of entries, then the least
	 * recently used entry will be removed.
	 * If the cache already contains an entry for credentials, then
	 * this entry will be moved to the end of the list.
	 * @param to [in] Destination for which authentication is needed.
	 * @param cr [in] Credentials to update.
	 * @param passwd [in] The password to store.
	 * @param proxy Indicates if proxy authentication was requested.
	 */
	void update_cache(const t_url &to, const t_credentials &cr,
		const string &passwd, bool proxy);

	/**
	 * Check if authorization failed.
	 * Authorization failed if the challenge is for a realm for which
	 * the request already contains an authorization header and the
	 * challenge is not stale.
	 * @return true, if authorization failed.
	 * @return false, otherwise.
	 */
	bool auth_failed(t_request *r, const t_challenge &c,
		bool proxy=false) const;

	/**
	 * Remove existing credentials for this challenge from the
	 * authorization or proxy-authorization header.
	 * @param r [in] The request from which the credentials must be removed.
	 * @param c [in] The challenge for which the credentials must be removed.
	 * @param proxy [in] Indicates if proxy authentication was requested.
	 */
	void remove_credentials(t_request *r, const t_challenge &c,
		bool proxy=false) const;

public:
	/** Constructor. */
	t_auth();
	
	/**
	 * Authorize the request based on the challenge in the response
	 * @param user_config [in] The user profile.
	 * @param r [in] The request to be authorized.
	 * @param resp [in] The response containing the challenge.
	 * @return true, if authorization succeeds.
	 * @return false, if authorization fails.
	 * @post On succesful authorization, the credentials has been added to
	 * the request in the proper header (Authorization or Proxy-Authorization).
	 */
	bool authorize(t_user *user_config, t_request *r, t_response *resp);
	
	/**
	 * Remove credentials for a particular realm from cache.
	 * @param realm [in] The authentication realm.
	 */
	void remove_from_cache(const string &realm);
	
	/**
	 * Set the re-REGISTER indication.
	 * @param on [in] Value to set.
	 */
	void set_re_register(bool on);
	
	/** Get the re-REGISTER indication. */
	bool get_re_register(void) const;
};

#endif
