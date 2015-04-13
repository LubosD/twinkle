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

#include "simple_msg_sum_body.h"

#include <iostream>
#include <cstdlib>
#include <boost/regex.hpp>

#include "protocol.h"
#include "util.h"
#include "audits/memman.h"

t_msg_summary::t_msg_summary() :
	newmsgs(0),
	newmsgs_urgent(0),
	oldmsgs(0),
	oldmsgs_urgent(0)
{}

bool t_msg_summary::parse(const string &s) {
	newmsgs = 0;
	oldmsgs = 0;
	newmsgs_urgent = 0;
	oldmsgs_urgent = 0;
	
	// RFC 3842 5.2
	// msg-summary-line = message-context-class HCOLON newmsgs SLASH oldmsgs
        //                    [ LPAREN new-urgentmsgs SLASH old-urgentmsgs RPAREN ]
        // This regex matches the part after HCOLON
	boost::regex re("(\\d+)\\s*/\\s*(\\d+)(?:\\s*\\((\\d+)\\s*/\\s*(\\d+)\\s*\\))?");
	
	boost::smatch m;
	if (!boost::regex_match(s, m, re)) return false;
	
	if (m.size() == 3) {
		newmsgs = strtoul(m.str(1).c_str(), NULL, 10);
		oldmsgs = strtoul(m.str(2).c_str(), NULL, 10);
		return true;
	} else if (m.size() == 5) {
		newmsgs = strtoul(m.str(1).c_str(), NULL, 10);
		oldmsgs = strtoul(m.str(2).c_str(), NULL, 10);
		newmsgs_urgent = strtoul(m.str(3).c_str(), NULL, 10);
		oldmsgs_urgent = strtoul(m.str(4).c_str(), NULL, 10);
		return true;
	}
	
	return false;	
}

void t_msg_summary::clear(void) {
	newmsgs = 0;
	newmsgs_urgent = 0;
	oldmsgs = 0;
	oldmsgs_urgent = 0;
}

bool t_simple_msg_sum_body::is_context(const string &s) {
	return ( s == MSG_CONTEXT_VOICE ||
	         s == MSG_CONTEXT_FAX ||
	         s == MSG_CONTEXT_MULTIMEDIA ||
	         s == MSG_CONTEXT_TEXT ||
	         s == MSG_CONTEXT_NONE);
}

t_simple_msg_sum_body::t_simple_msg_sum_body() : t_sip_body()
{}

string t_simple_msg_sum_body::encode(void) const {
	string s = "Messages-Waiting: ";
	s += (msg_waiting ? "yes" : "no");
	s += CRLF;
	
	if (msg_account.is_valid()) {
		s += "Message-Account: ";
		s += msg_account.encode();
		s += CRLF;
	}
	
	for (t_msg_sum_const_iter i = msg_summary.begin(); i != msg_summary.end(); ++i) {
		const t_msg_summary &summary = i->second;
		s += i->first;
		s += ": ";
		s += ulong2str(summary.newmsgs);
		s += "/";
		s += ulong2str(summary.oldmsgs);
		
		if (summary.newmsgs_urgent > 0 || summary.oldmsgs_urgent > 0) {
			s += " (";
			s += ulong2str(summary.newmsgs_urgent);
			s += "/";
			s += ulong2str(summary.oldmsgs_urgent);
			s += ")";
		}
		
		s += CRLF;
	}
	
	return s;
}

t_sip_body *t_simple_msg_sum_body::copy(void) const {
	t_simple_msg_sum_body *body = new t_simple_msg_sum_body(*this);
	MEMMAN_NEW(body);
	return body;
}

t_body_type t_simple_msg_sum_body::get_type(void) const {
	return BODY_SIMPLE_MSG_SUM;
}

t_media t_simple_msg_sum_body::get_media(void) const {
	return t_media("application", "simple-message-summary");
}

void t_simple_msg_sum_body::add_msg_summary(const string &context, const t_msg_summary summary) {
	msg_summary.insert(make_pair(context, summary));
}

bool t_simple_msg_sum_body::get_msg_waiting(void) const {
	return msg_waiting;
}

t_url t_simple_msg_sum_body::get_msg_account(void) const {
	return msg_account;
}

bool t_simple_msg_sum_body::get_msg_summary(const string &context, t_msg_summary &summary) const {
	t_msg_sum_const_iter it = msg_summary.find(context);
	if (it == msg_summary.end()) return false;
	summary = it->second;
	return true;
}

bool t_simple_msg_sum_body::parse(const string &s) {
	bool valid = false;
	vector<string> lines = split_linebreak(s);
	
	for (vector<string>::iterator i = lines.begin(); i != lines.end(); ++i) {
		string line = trim(*i);
		if (line.empty()) continue;
		
		vector<string> l = split_on_first(line, ':');
		if (l.size() != 2) continue;
		
		string header = tolower(trim(l[0]));
		string value = tolower(trim(l[1]));
		
		if (value.empty()) continue;
		
		if (header == "messages-waiting") {
			if (value == "yes") {
				msg_waiting = true;
				valid = true;
			} else if (value == "no") {
				msg_waiting = false;
				valid = true;
			}
		} else if (header == "message-account") {
			msg_account.set_url(value);
		} else if (is_context(header)) {
			t_msg_summary summary;
			if (summary.parse(value)) {
				add_msg_summary(header, summary);
			}
		}
	}
	
	invalid = !valid;
	return valid;
}
