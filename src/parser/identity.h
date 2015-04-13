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

#ifndef _IDENTITY_H
#define _IDENTITY_H

#include <string>
#include "sockets/url.h"

using namespace std;

class t_identity {
public:
	string	display; // display name	
	t_url	uri;
	
	t_identity();
	void set_display(const string &d);
	void set_uri(const string &u);
	void set_uri(const t_url &u);
	
	string encode(void) const;
};

#endif
