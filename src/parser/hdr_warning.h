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

// Warning header

#ifndef _H_HDR_WARNING
#define _H_HDR_WARNING

#include <list>
#include <string>
#include "header.h"

// Warning codes
#define W_300_INCOMPATIBLE_NWK_PROT	300
#define W_301_INCOMPATIBLE_ADDR_FORMAT	301
#define W_302_INCOMPATIBLE_TRANS_PROT	302
#define W_303_INCOMPATIBLE_BW_UNITS	303
#define W_304_MEDIA_TYPE_NOT_AVAILABLE	304
#define W_305_INCOMPATIBLE_MEDIA_FORMAT	305
#define W_306_ATTRIBUTE_NOT_UNDERSTOOD	306
#define W_307_PARAMETER_NOT_UNDERSTOOD	307
#define W_330_MULTICAST_NOT_AVAILABLE	330
#define W_331_UNICAST_NOT_AVAILABLE	331
#define W_370_INSUFFICIENT_BANDWITH	370
#define W_399_MISCELLANEOUS		399

// Warning texts
#define WARNING_300	"Incompatible network protocol"
#define WARNING_301	"Incompatible network address formats"
#define WARNING_302	"Incompatible transport protocol"
#define WARNING_303	"Incompatible bandwith units"
#define WARNING_304	"Media type not available"
#define WARNING_305	"Incompatible media format"
#define WARNING_306	"Attribute not understood"
#define WARNING_307	"Session description parameter not understood"
#define WARNING_330	"Multicast not available"
#define WARNING_331	"Unicast not available"
#define WARNING_370	"Insufficient bandwith"
#define WARNING_399	"Miscellanous warning"

using namespace std;

class t_warning {
public:
	int		code;
	string		host;
	int		port;
	string		text;

	t_warning();

	// The default text will be used as warning appended with passed
	// text if present
	t_warning(const string &_host, int _port, int _code, string _text);

	string encode(void) const;
};

class t_hdr_warning : public t_header {
public:
	list<t_warning>	warnings;

	t_hdr_warning();
	void add_warning(const t_warning &w);
	string encode_value(void) const;
};

#endif
