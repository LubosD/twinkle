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

// Base class for message and response headers

#ifndef _HEADER_H
#define _HEADER_H

#include <string>

using namespace std;

class t_header {
private:
	t_header();

protected:
	bool	populated;	// true = header is populated
	string	header_name;	// Full name of header in SIP messages
	string	compact_name;	// Compact name of header in SIP messages

public:
	virtual ~t_header() {}
	t_header(const string &_header_name, const string &_compact_name = "");

	// Return the text encoded header (CRLF at end of string)
	virtual string encode(void) const;
	
	// Return the text encoded value part (no CRLF at end of string)
	virtual string encode_value(void) const = 0;
	
	// Return a environemnt variable setting
	// The format of the setting is:
	//
	// SIP_<header name>=<value>
	//
	// The header name is in capitals. Dashes are replaced by underscores.
	virtual string encode_env(void) const;
	
	// Get the header name
	string get_name(void) const;

	// Get text encoding of the header value only.
	// I.e. without header name and no trailing CRLF
	string get_value(void) const;

	// Return true if the header is populated
	bool is_populated(void) const;
};

#endif
