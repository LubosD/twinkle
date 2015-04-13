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

#include "sdp_parse_ctrl.h"
#include "audits/memman.h"

// Interface to Bison
extern int yysdpparse(void);

// Interface to Flex
struct yy_buffer_state;
extern struct yy_buffer_state *yysdp_scan_string(const char *);
extern void yysdp_delete_buffer(struct yy_buffer_state *);

t_mutex t_sdp_parser::mtx_parser;
t_sdp_parser::t_context t_sdp_parser::context = t_sdp_parser::X_INITIAL;
t_sdp *t_sdp_parser::sdp = NULL;

t_sdp *t_sdp_parser::parse(const string &s) {
	int ret;
	struct yy_buffer_state *b;
	
	t_mutex_guard guard(mtx_parser);
	
	sdp = new t_sdp();
	MEMMAN_NEW(sdp);
	
	// The SDP body should end with a CRLF. Some implementations
	// do not send this last CRLF. Allow this deviation by adding
	// the last CRLF if it is missing.
	char last_char = s.at(s.size()-1);
	if (last_char == '\n' || last_char == '\r') {
		// The SDP parser allows \r, \r\n and \n as CRLF
		b = yysdp_scan_string(s.c_str());
	} else {
		// Last CRLF is missing.
		b = yysdp_scan_string((s + "\r\n").c_str());
	}
	
	ret = yysdpparse();
	yysdp_delete_buffer(b);

	if (ret != 0) {
		MEMMAN_DELETE(sdp);
		delete sdp;
		sdp = NULL;
		throw ret;
	}

	return sdp;
}

t_sdp_syntax_error::t_sdp_syntax_error(const string &e) {
	error = e;
}
