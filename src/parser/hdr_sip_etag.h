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
 * SIP-ETag header (RFC 3903)
 */
 
#ifndef _HDR_SIP_ETAG_H
#define _HDR_SIP_ETAG_H

#include <string>
#include "header.h"

using namespace std;

/** SIP-ETag header (RFC 3903) */
class t_hdr_sip_etag : public t_header {
public:
	string	etag;	/**< Entity tag. */
	
	/** Constructor. */
	t_hdr_sip_etag();
	
	/**
	 * Set entity tag.
	 * @param _etag [in] Entity tag to set.
	 */
	void set_etag(const string &_etag);
	
	string encode_value(void) const;
};

#endif
