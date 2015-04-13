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

// Date header

#ifndef _HDR_DATE_H
#define _HDR_DATE_H

#include <ctime>
#include <string>
#include "header.h"

class t_hdr_date : public t_header {
public:
	time_t	date;

	t_hdr_date();
	void set_date_gm(struct tm *tm); // set date, tm is GMT
	void set_now(void); // Set date/time to current date/time
	string encode_value(void) const;
};

using namespace std;

#endif
