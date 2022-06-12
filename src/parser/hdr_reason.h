/*
    Copyright (C) 2005-2009  Michel de Boer <michel@twinklephone.com>
    Copyright (C) 2022       Frédéric Brière <fbriere@fbriere.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

// Reason header (RFC 3326)

#ifndef _HDR_REASON_H
#define _HDR_REASON_H

#include <list>
#include <string>

#include "header.h"
#include "parameter.h"


class t_reason {
public:
	std::string       	protocol;	// "SIP", "Q.850" or other
	std::string       	cause;		// For SIP, status code
	std::string       	text;		// Reason text
	std::list<t_parameter>	extensions;

	t_reason(const std::string &_protocol);

	void set_cause(const std::string &_cause);
	void set_text(const std::string &_text);
	void add_extension(const t_parameter &p);

	string encode(void) const;
};

class t_hdr_reason : public t_header {
public:
	std::list<t_reason> reason_list;

	t_hdr_reason();

	void add_reason(const t_reason &reason);

	// Returns a (possibly empty) string that can be displayed as an
	// explanation to the user
	std::string get_display_text() const;

	string encode(void) const;
	string encode_multi_header(void) const;
	string encode_value(void) const;
};

#endif
