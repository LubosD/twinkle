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

#ifndef _H_REDIRECT
#define _H_REDIRECT

#include <list>
#include "user.h"
#include "parser/hdr_contact.h"
#include "sockets/url.h"

using namespace std;

class t_redirector {
private:
	// Total number of contacts in try and done lists
	int			num_contacts;

	// Contacts to try
	list<t_contact_param>	try_contacts;

	// Contacts already tried, but unsuccesful
	list<t_contact_param>	done_contacts;

	// Original destination
	t_url			org_dest;
	
	// Maximum number of redirections that will be tried
	int			max_redirections;

	bool contact_already_added(const t_contact_param contact) const;

	// Constructor without parameter should not be used.
	t_redirector();

public:
	t_redirector(const t_url &_org_dest, int _max_redirections);

	// Get the next contact to try
	// Returns false if there is no next contact
	bool get_next_contact(t_contact_param &contact);

	// Add contacts. The passed contacts will be sorted on decreasing
	// q-value before adding.
	// Contacts that are already in the try list or are tried already
	// will not be added.
	// If the maximum number of redirections is reached then all contacts
	// exceeding the maximum will be discarded.
	void add_contacts(const list<t_contact_param> &contacts);
};

#endif
