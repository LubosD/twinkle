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

// Accept header

#ifndef _HDR_ACCEPT_H
#define _HDR_ACCEPT_H

#include <list>
#include "header.h"
#include "media_type.h"

class t_hdr_accept : public t_header {
public:
	list<t_media>	media_list;	// list of accepted media

	t_hdr_accept();

	// Add a media to the list of accepted media
	void add_media(const t_media &media);

	// Clear the list of features, but make the header 'populated'.
	// An empty header will be in the message.
	void set_empty(void);

	string encode_value(void) const;
};

#endif
