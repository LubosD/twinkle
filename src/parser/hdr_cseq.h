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

// CSeq header

#ifndef _HDR_CSEQ
#define _HDR_CSEQ

#include <string>
#include "header.h"
#include "definitions.h"

using namespace std;

class t_hdr_cseq : public t_header {
public:
	unsigned long	seqnr;
	t_method	method;
	string		unknown_method; // set if method is UNKNOWN

	t_hdr_cseq();
	void set_seqnr(unsigned long l);
	void set_method(t_method m, const string &unknown = "");
	void set_method(const string &s);

	string encode_value(void) const;

	bool operator==(const t_hdr_cseq &h) const;
};

#endif
