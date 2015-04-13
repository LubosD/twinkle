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

#ifndef _ADDRESS_FINDER_H
#define _ADDRESS_FINDER_H

#include "twinkle_config.h"

#include <string>
#include "user.h"
#include "sockets/url.h"
#include "threads/mutex.h"
#include "qobject.h"
#include "qimage.h"

#ifdef HAVE_KDE
#include <kabc/addressbook.h>
#include <kabc/addressee.h>
#include <kabc/addresseelist.h>
#include <kabc/phonenumber.h>
#include <kabc/stdaddressbook.h>
#endif

using namespace std;

class t_address_finder : public QObject {
private:
	Q_OBJECT
	static t_address_finder *instance;
	static t_mutex mtx_instance;
	
	t_mutex	mtx_finder;
#ifdef HAVE_KDE
	KABC::AddressBook *abook;
#endif
	
	// Cached data for the last looked up URL.
	t_url	last_url;
	string	last_name;
	QImage	last_photo;
	
	t_address_finder();
	
	// Find the address based on a URL and put the found
	// data in the cache.
	void find_address(t_user *user_config, const t_url &u);
	
public:
	// Preload KAddressbook
	static void preload(void);
	
	static t_address_finder *get_instance(void);
	
	// Find a name given a URL
	string find_name(t_user *user_config, const t_url &u);
	
	// Find a photo give a URL
	QImage find_photo(t_user *user_config, const t_url &u);
	
public slots:
	void invalidate_cache(void);
};

#endif
