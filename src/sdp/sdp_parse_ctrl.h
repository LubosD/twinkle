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

// Parser control

#ifndef _SDP_PARSE_CTRL_H
#define _SDP_PARSE_CTRL_H

#include "sdp.h"
#include "threads/mutex.h"

#define SDP		t_sdp_parser::sdp

#define CTX_INITIAL	(t_sdp_parser::context = t_sdp_parser::X_INITIAL)
#define CTX_SAFE	(t_sdp_parser::context = t_sdp_parser::X_SAFE)
#define CTX_NUM		(t_sdp_parser::context = t_sdp_parser::X_NUM)
#define CTX_LINE	(t_sdp_parser::context = t_sdp_parser::X_LINE)

// The t_sdp_parser controls the direction of the scanner/parser
// process and it stores the results from the parser.
class t_sdp_parser {
private:
	/** Mutex to synchronize parse operations */
	static t_mutex	mtx_parser;
public:
enum t_context {
	X_INITIAL,	// Initial context
	X_SAFE,		// Safe context
	X_NUM,		// Number context
	X_LINE,		// Whole line context
};

	static t_context	context;   // Scan context
	static t_sdp		*sdp;      // SDP that has been parsed

	// Parse string s. Throw int exception when parsing fails.
	static t_sdp *parse(const string &s);

};

// Error that can be thrown as exception
class t_sdp_syntax_error {
public:
	string error;
	t_sdp_syntax_error(const string &e);
};

#endif
