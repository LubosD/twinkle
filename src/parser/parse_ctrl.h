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

#ifndef _PARSE_CTRL_H
#define _PARSE_CTRL_H

#include "sip_message.h"
#include "threads/mutex.h"

#define MSG		t_parser::msg

#define CTXT_INITIAL	(t_parser::context = t_parser::X_INITIAL)
#define CTXT_URI	(t_parser::context = t_parser::X_URI)
#define CTXT_URI_SPECIAL (t_parser::context = t_parser::X_URI_SPECIAL)
#define CTXT_LANG	(t_parser::context = t_parser::X_LANG)
#define CTXT_WORD	(t_parser::context = t_parser::X_WORD)
#define CTXT_NUM	(t_parser::context = t_parser::X_NUM)
#define CTXT_DATE	(t_parser::context = t_parser::X_DATE)
#define CTXT_LINE	(t_parser::context = t_parser::X_LINE)
#define CTXT_COMMENT	(t_parser::enter_ctx_comment())
#define CTXT_NEW	(t_parser::context = t_parser::X_NEW)
#define CTXT_AUTH_SCHEME (t_parser::context = t_parser::X_AUTH_SCHEME)
#define CTXT_IPV6ADDR	(t_parser::context = t_parser::X_IPV6ADDR)
#define CTXT_PARAMVAL	(t_parser::context = t_parser::X_PARAMVAL)

#define PARSE_ERROR(h)	{ t_parser::add_header_error(h); CTXT_INITIAL; }

// The t_parser controls the direction of the scanner/parser
// process and it stores the results from the parser.
class t_parser {
private:
	/** Mutex to synchronize parse operations */
	static t_mutex	mtx_parser;

	// Level for nested comments
	static int comment_level;

	// Non-fatal parse errors generated during parsing.
	static list<string> parse_errors;

	// Unfold SIP headers
	static string unfold(const string &h);

public:
enum t_context {
	X_INITIAL,	// Initial context
	X_URI,		// URI context where parameters belong to URI
	X_URI_SPECIAL,	// URI context where parameters belong to SIP header
			// if URI is not enclosed by < and >
	X_LANG,		// Language tag context
	X_WORD,		// Word context
	X_NUM,		// Number context
	X_DATE,		// Date context
	X_LINE,		// Whole line context
	X_COMMENT,	// Comment context
	X_NEW,		// Start of a new SIP message to distinguish
			// request from responses
	X_AUTH_SCHEME,	// Authorization scheme context
	X_IPV6ADDR,	// IPv6 address context
	X_PARAMVAL,	// Generic parameter value context
};

	// Parser options
	// According to RFC3261 the Max-Forwards header is mandatory, but
	// many implementations do not send this header.
	static bool		check_max_forwards;

	// Encode headers in compact forom
	static bool		compact_headers;
	
	// Encode multiple values as comma separated list or multiple headers
	static bool		multi_values_as_list;

	static t_context	context;    // Scan context
	static t_sip_message	*msg;       // Message that has been parsed

	/** 
	 * Parse a string representing a SIP message.
	 * @param s [in] String to parse.
	 * @param parse_errors_ [out] List of non-fatal parse errors.
	 * @return The parsed SIP message.
	 * @throw int exception when parsing fails.
	 */
	static t_sip_message *parse(const string &s, list<string> &parse_errors_);
	
	/**
	 * Parse a string of headers (hdr1=val1;hdr=val2;...)
	 * The resulting SIP message is a SIP request with a fake request line.
	 * @param s [in] String to parse.
	 * @param parse_errors_ [out] List of non-fatal parse errors.
	 * @return The parsed SIP message.
	 * @throw int exception when parsing fails.
	 */
	static t_sip_message *parse_headers(const string &s, list<string> &parse_errors_);

	static void enter_ctx_comment(void);

	// Increment and decrement levels for nested comments
	// dec_comment_level returns false if the level cannot be decremented.
	static void inc_comment_level(void);
	static bool dec_comment_level(void);

	// Add parsing error for a header to the list of parse errors
	static void add_header_error(const string &header_name);
};

// Error that can be thrown as exception
class t_syntax_error {
public:
	string error;
	t_syntax_error(const string &e);
};

#endif
