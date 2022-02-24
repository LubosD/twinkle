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

// Min-SE header (RFC 4028)

#ifndef _HDR_MIN_SE_H
#define _HDR_MIN_SE_H

#include <list>
#include <string>
#include "header.h"
#include "parameter.h"

class t_hdr_min_se : public t_header {
public:
	unsigned long time; // expiry time in seconds
	list<t_parameter> params;

	t_hdr_min_se();

	void set_time(unsigned long t);

	void add_param(const t_parameter &p);
	void set_params(const std::list<t_parameter> &l);

	std::string encode_value(void) const;
};

#endif
