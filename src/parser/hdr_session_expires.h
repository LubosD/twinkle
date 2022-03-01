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

// Session-Expires header (RFC 4028)

#ifndef _HDR_SESSION_EXPIRES_H
#define _HDR_SESSION_EXPIRES_H

#include <list>
#include <string>
#include "header.h"
#include "parameter.h"

#define SE_REFRESHER_UAS	"uas"
#define SE_REFRESHER_UAC	"uac"

class t_hdr_session_expires : public t_header {
public:
	enum t_refresher {
		REFRESHER_NONE,
		REFRESHER_UAS,
		REFRESHER_UAC
	};

	unsigned long time; // expiry time in seconds
	t_refresher refresher;
	list<t_parameter> params;

	t_hdr_session_expires();

	void set_time(unsigned long t);
	void set_refresher(t_refresher r);
	bool set_refresher(const std::string &r);

	void add_param(const t_parameter &p);
	void set_params(const std::list<t_parameter> &l);

	std::string encode_value(void) const;
};

#endif
