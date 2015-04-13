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

// Contact header

#ifndef _HDR_CONTACT
#define _HDR_CONTACT

#include <list>
#include <string>
#include "header.h"
#include "parameter.h"
#include "sockets/url.h"

using namespace std;

class t_contact_param {
private:
	bool			expires_present;
	unsigned long		expires;
	bool			qvalue_present;
	float			qvalue;
	
public:
	string			display; // display name
	t_url			uri;
	list<t_parameter>	extensions;

	t_contact_param();
	void add_extension(const t_parameter &p);
	string encode(void) const;
	bool is_expires_present(void) const;
	unsigned long get_expires(void) const;
	void set_expires(unsigned long e);
	float get_qvalue(void) const;
	void set_qvalue(float q);

	// Compare contacts on q-value.
	// The contacts with the highest q-value comes first in the order
	bool operator<(const t_contact_param &c) const;
};

class t_hdr_contact : public t_header {
public:
	bool			any_flag; // true if Contact: *
	list<t_contact_param>	contact_list;

	t_hdr_contact();
	void add_contact(const t_contact_param &contact);
	void add_contacts(const list<t_contact_param> &l);
	void set_contacts(const list<t_contact_param> &l);
	
	/**
	 * Set the contact list to a sequence of URI's with display names.
	 * The URI's are give a descending q-value starting at 0.9
	 * Each subsequent URI gets a q-value 0.1 less than the previous
	 * URI. If more than 9 URI's are passed then the tail of URI's all
	 * get a q-value of 0.1.
	 *
	 * @param l [in] The list of URI's to be put in the contact list.
	 */	
	void set_contacts(const list<t_url> &l);
	
	/**
	 * Set the contact list to a sequence of URI's with display names.
	 * The URI's are give a descending q-value starting at 0.9
	 * Each subsequent URI gets a q-value 0.1 less than the previous
	 * URI. If more than 9 URI's are passed then the tail of URI's all
	 * get a q-value of 0.1.
	 *
	 * @param l [in] The list of URI's to be put in the contact list.
	 */
	void set_contacts(const list<t_display_url> &l);

	// Set contact to any, eg. Contact: *
	void set_any(void);

	// Find contact with uri u. If no contact is found, then
	// NULL is returned.
	t_contact_param *find_contact(const t_url &u);

	string encode_value(void) const;
};

#endif
