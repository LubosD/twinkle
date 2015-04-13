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

// Authentication-Info header

#ifndef _HDR_AUTH_INFO_H
#define _HDR_AUTH_INFO_H

#include <string>
#include "header.h"

using namespace std;

class t_hdr_auth_info : public t_header {
public:
	string			next_nonce;
	string			message_qop;
	string			response_auth;
	string			cnonce;
	unsigned long		nonce_count;

	t_hdr_auth_info();

	void set_next_nonce(const string &nn);
	void set_message_qop(const string &mq);
	void set_response_auth(const string &ra);
	void set_cnonce(const string &cn);
	void set_nonce_count(const unsigned long &nc);
	string encode_value(void) const;
};

#endif
