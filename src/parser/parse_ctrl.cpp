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

#include "parse_ctrl.h"
#include "protocol.h"
#include "util.h"
#include "audits/memman.h"

// Interface to Bison
extern int yyparse(void);

// Interface to Flex
struct yy_buffer_state;
extern struct yy_buffer_state *yy_scan_string(const char *);
extern void yy_delete_buffer(struct yy_buffer_state *);

t_mutex t_parser::mtx_parser;
bool t_parser::check_max_forwards = true;
bool t_parser::compact_headers = false;
bool t_parser::multi_values_as_list = true;
int t_parser::comment_level = 0;
list<string> t_parser::parse_errors;

string t_parser::unfold(const string &h) {
	string::size_type i;
	string s = h;

	while ((i = s.find("\r\n ")) != string::npos) {
		s.replace(i, 3, " ");
	}

	while ((i = s.find("\r\n\t")) != string::npos) {
		s.replace(i, 3, " ");
	}

	// This is only for easy testing of hand edited messages
	// in Linux where the end of line character is \n only.
	while ((i = s.find("\n ")) != string::npos) {
		s.replace(i, 2, " ");
	}

	while ((i = s.find("\n\t")) != string::npos) {
		s.replace(i, 2, " ");
	}

	return s;
}

t_parser::t_context t_parser::context = t_parser::X_INITIAL;
t_sip_message *t_parser::msg = NULL;

t_sip_message *t_parser::parse(const string &s, list<string> &parse_errors_) {
	t_mutex_guard guard(mtx_parser);
	
	int ret;
	struct yy_buffer_state *b;
	msg = NULL;

	parse_errors.clear();

	string x = unfold(s);

	b = yy_scan_string(x.c_str());
	ret = yyparse();
	yy_delete_buffer(b);

	if (ret != 0) {
		if (msg) {
			MEMMAN_DELETE(msg);
			delete msg;
			msg = NULL;
		}
		throw ret;
	}

	parse_errors_ = parse_errors;
	return msg;
}

t_sip_message *t_parser::parse_headers(const string &s, list<string> &parse_errors_) {
	string msg("INVITE sip:fake@fake.invalid SIP/2.0");
	msg += CRLF;
	
	list<t_parameter> hdr_list = str2param_list(s);
	for (list<t_parameter>::iterator i = hdr_list.begin();
	     i != hdr_list.end(); i++)
	{
		msg += unescape_hex(i->name);
		msg += ": ";
		msg += unescape_hex(i->value);
		msg += CRLF;
	}
	
	msg += CRLF;
	
	return parse(msg, parse_errors_);
}

void t_parser::enter_ctx_comment(void) {
	comment_level = 0;
	context = t_parser::X_COMMENT;
}

void t_parser::inc_comment_level(void) {
	comment_level++;
}

bool t_parser::dec_comment_level(void) {
	if (comment_level == 0) return false;
	comment_level--;
	return true;
}

void t_parser::add_header_error(const string &header_name) {
	string s = "Parse error in header: " + header_name;
	parse_errors.push_back(s);
}

t_syntax_error::t_syntax_error(const string &e) {
	error = e;
}
