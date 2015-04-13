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

// NOTE: the date functions are not thread safe

#include <sys/time.h>
#include "hdr_date.h"
#include "definitions.h"
#include "util.h"

t_hdr_date::t_hdr_date() : t_header("Date") {}

void t_hdr_date::set_date_gm(struct tm *tm) {
	populated = true;
	date = timegm(tm);
}

void t_hdr_date::set_now(void) {
	struct timeval t;

	populated = true;
	gettimeofday(&t, NULL);
	date = t.tv_sec;
}

string t_hdr_date::encode_value(void) const {
	string s;
	struct tm tm;

	if (!populated) return s;

	gmtime_r(&date, &tm);
	s = weekday2str(tm.tm_wday);
	s += ", ";
	s += int2str(tm.tm_mday, "%02d");
	s += ' ';
	s += month2str(tm.tm_mon);
	s += ' ';
	s += int2str(tm.tm_year + 1900, "%04d");
	s += ' ';
	s += int2str(tm.tm_hour, "%02d");
	s += ':';
	s += int2str(tm.tm_min, "%02d");
	s += ':';
	s += int2str(tm.tm_sec, "%02d");
	s += " GMT";

	return s;
}
