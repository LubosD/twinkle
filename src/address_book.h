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
 * Local address book.
 */

#ifndef _ADDRESS_BOOK_H
#define _ADDRESS_BOOK_H

#include <string>
#include <list>

#include "user.h"
#include "sockets/url.h"
#include "threads/mutex.h"
#include "utils/record_file.h"

using namespace std;

/** A single address card. */
class t_address_card : public utils::t_record {
public:
	string		name_last;	/**< Last name. */
	string		name_first;	/**< First name. */
	string		name_infix;	/**< Infix name. */
	string		sip_address;	/**< SIP address. */
	string		remark;		/**< Remark. */

	/**
	 * Get the display name derived from first, last and infix name.
	 * @return The display name.
	 */
	string get_display_name(void) const;
	
	virtual bool create_file_record(vector<string> &v) const;
	virtual bool populate_from_file_record(const vector<string> &v);
	
	/** Equality check. */
	bool operator==(const t_address_card other) const;
};

/**
 * A book containing address cards. The user can
 * create different address books.
 */
class t_address_book : public utils::t_record_file<t_address_card> {
private:
	/** @name Cache for last searched name/url mapping */
	//@{
	mutable t_url		last_url;	/**< Last URL. */
	mutable string		last_name;	/**< Lat name. */
	//@}
	
	/**
	 * Find a matching address for a url and cache the display name.
	 * @param user_config [in] The user profile.
	 * @param u [in] The url to find.
	 * @post If a matching address is found, then the URL and name are
	 * put in the cache. Otherwise the cache is cleared.
	 */
	void find_address(t_user *user_config, const t_url &u) const;

public:
	/** Constructor. */
	t_address_book();

	/**
	 * Add an address.
	 * @param address [in] The address to be added.
	 */
	void add_address(const t_address_card &address);
	
	/**
	 * Delete an address.
	 * @return true, if the address was succesfully deleted.
	 * @return false, if the address does not exist.
	 */
	bool del_address(const t_address_card &address);

	/**
	 * Update an address.
	 * @param old_address [in] The address to be updated.
	 * @param new_address [in] The updated address information.
	 * @return true, if the update was successful.
	 * @return false, if the old address does not exist.
	 */
	bool update_address(const t_address_card &old_address,
		const t_address_card &new_address);
		
	/**
	 * Find the display name for a SIP URL.
	 * @param user_config [in] The user profile.
	 * @param u [in] The SIP URL.
	 * @return The display name if a match was found.
	 * @return Empty string if no match can be found.
	 */
	string find_name(t_user *user_config, const t_url &u) const;

	/**
	 * Get the list of addresses.
	 * @return The list of addresses.
	 */
	const list<t_address_card> &get_address_list(void) const;
};

extern t_address_book *ab_local;

#endif
