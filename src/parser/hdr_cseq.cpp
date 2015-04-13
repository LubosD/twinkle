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

#include "definitions.h"
#include "hdr_cseq.h"
#include "util.h"

t_hdr_cseq::t_hdr_cseq() : t_header("CSeq") {
	seqnr = 0;
	method = INVITE;
}

void t_hdr_cseq::set_seqnr(unsigned long l) {
	populated = true;
	seqnr = l;
}

void t_hdr_cseq::set_method(t_method m, const string &unknown) {
	populated = true;
	method = m;
	unknown_method = unknown;
}

void t_hdr_cseq::set_method(const string &s) {
	populated = true;
	method = str2method(s);
	if (method == METHOD_UNKNOWN) {
		unknown_method = s;
	}
}

string t_hdr_cseq::encode_value(void) const {
	string s;

	if (!populated) return s;

	s = ulong2str(seqnr) + ' ';
	s += method2str(method, unknown_method);

	return s;
}

bool t_hdr_cseq::operator==(const t_hdr_cseq &h) const {
	if (method != METHOD_UNKNOWN) {
		return (seqnr == h.seqnr && method == h.method);
	}

	return (seqnr == h.seqnr && unknown_method == h.unknown_method);
}
