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
#include "redirect.h"

bool t_redirector::contact_already_added(const t_contact_param contact) const {
	for (list<t_contact_param>::const_iterator i = try_contacts.begin();
	     i != try_contacts.end(); i++)
	{
		if (i->uri == contact.uri) return true;
	}

	for (list<t_contact_param>::const_iterator i = done_contacts.begin();
	     i != done_contacts.end(); i++)
	{
		if (i->uri == contact.uri) return true;
	}

	if (contact.uri == org_dest) return true;

	return false;
}

t_redirector::t_redirector(const t_url &_org_dest, int _max_redirections) {
	num_contacts = 0;
	org_dest = _org_dest;
	max_redirections = _max_redirections;
}

bool t_redirector::get_next_contact(t_contact_param &contact) {
	if (try_contacts.empty()) return false;

	contact = try_contacts.front();
	try_contacts.pop_front();
	done_contacts.push_back(contact);

	return true;
}

void t_redirector::add_contacts(const list<t_contact_param> &contacts) {
	if (num_contacts >= max_redirections) return;

	list<t_contact_param> l = contacts;
	l.sort();

	for (list<t_contact_param>::iterator i = l.begin(); i != l.end(); i++) {
		if (!contact_already_added(*i)) {
			try_contacts.push_back(*i);
			num_contacts++;

			if (num_contacts >= max_redirections) break;
		}
	}
}
