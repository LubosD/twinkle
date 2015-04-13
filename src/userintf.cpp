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

#include <iostream>
#include <cstdlib>
#include <readline/readline.h>
#include <readline/history.h>
#include "address_book.h"
#include "events.h"
#include "line.h"
#include "log.h"
#include "sys_settings.h"
#include "translator.h"
#include "userintf.h"
#include "util.h"
#include "user.h"
#include "audio/rtp_telephone_event.h"
#include "parser/parse_ctrl.h"
#include "sockets/interfaces.h"
#include "audits/memman.h"
#include "utils/file_utils.h"
#include "utils/mime_database.h"

#define CLI_PROMPT              "Twinkle> "
#define CLI_MAX_HISTORY_LENGTH	1000

extern string user_host;
extern t_event_queue *evq_trans_layer;

using namespace utils;



////////////////////////
// GNU Readline helpers
////////////////////////

char ** tw_completion (const char *text, int start, int end);
char * tw_command_generator (const char *text, int state);

char ** tw_completion (const char *text, int start, int end)
{
	char **matches;
	matches = (char **)NULL;
	if (start == 0)
		matches = rl_completion_matches (text, tw_command_generator);
	return (matches);
}



char * tw_command_generator (const char *text, int state)
{
	static int len;
	static list<string>::const_iterator i;

	if (!state){
		len = strlen(text);
		i = ui->get_all_commands().begin();
	}

	for (; i != ui->get_all_commands().end(); i++){
		const char * s = i->c_str();
		//cout << s << endl;
		if ( s && strncmp(s, text, len) == 0  ){
			i++;
			return strdup(s);
		}
	}

	/* If no names matched, then return NULL. */
	return ((char *)NULL);
}

char *tw_readline(const char *prompt)
{
	static char *line = NULL;
	
	if (!line) {
		free(line);
		line = NULL;
	}
	
	line = readline(prompt);
	
	if (line && *line) {
		add_history(line);
	}
	
	return line;
}

/////////////////////////////
// Private
/////////////////////////////

string t_userintf::expand_destination(t_user *user_config, const string &dst, const string &scheme) {
	assert(user_config);
	
	string s = dst;

	// Apply number conversion rules if applicable
	// Add domain if it is missing from a sip-uri
	if (s.find('@') == string::npos) {
		bool is_tel_uri = (s.substr(0, 4) == "tel:");
		
		// Strip tel-scheme
		if (is_tel_uri) s = s.substr(4);
	
		// Remove white space
		s = remove_white_space(s);
	
		// Remove special phone symbols
		if (user_config->get_remove_special_phone_symbols() &&
		    looks_like_phone(s, user_config->get_special_phone_symbols())) 
		{
			s = remove_symbols(s, user_config->get_special_phone_symbols());
		}
		
		// Convert number according to the number conversion rules
		s = user_config->convert_number(s);
			
		if (is_tel_uri) {
			// Add tel-scheme again.
			s = "tel:" + s;
		} else if (s.substr(0, 4) != "sip:" &&
		           (user_config->get_use_tel_uri_for_phone() || scheme == "tel") &&
		           user_config->get_numerical_user_is_phone() &&
		           looks_like_phone(s, user_config->get_special_phone_symbols()))
		{
			// Add tel-scheme if a telephone number must be expanded
		        // to a tel-uri according to user profile settings.
			s = "tel:" + s;
		} else {
			// Add domain
			s += '@';
			s += user_config->get_domain();
		}
	}
	
	// Add sip-scheme if a scheme is missing
	if (s.substr(0, 4) != "sip:" && s.substr(0, 4) != "tel:") {
		s = "sip:" + s;
	}

	// RFC 3261 19.1.1
	// Add user=phone for telehpone numbers in a SIP-URI
	// If the SIP-URI contains a telephone number it SHOULD contain
	// the user=phone parameter.
	if (user_config->get_numerical_user_is_phone() && s.substr(0, 4) == "sip:") {
		t_url u(s);
		if (u.get_user_param().empty() && 
		    u.user_looks_like_phone(user_config->get_special_phone_symbols())) {
			s += ";user=phone";
		}
	}

	return s;
}

void t_userintf::expand_destination(t_user *user_config, 
		const string &dst, string &display, string &dst_url) 
{
	display.clear();
	dst_url.clear();
		
	if (dst.empty()) {
		return;
	}
	
	// If there is a display name then the url part is between angle
	// brackets.
	if (dst[dst.size() - 1] != '>') {
		dst_url = expand_destination(user_config, dst);
		return;
	}
	
	// Find start of url
	string::size_type i = dst.rfind('<');
	if (i == string::npos) {
		// It seems the string is invalid.
		return;
	}
	
	dst_url = expand_destination(user_config, dst.substr(i + 1, dst.size() - i - 2));
	
	if (i > 0) {
		display = unquote(trim(dst.substr(0, i)));
	}
}

void t_userintf::expand_destination(t_user *user_config, 
		const string &dst, t_display_url &display_url) 
{
	string url_str;
	
	expand_destination(user_config, dst, display_url.display, url_str);
	display_url.url.set_url(url_str);
}

void t_userintf::expand_destination(t_user *user_config,
		const string &dst, t_display_url &display_url, string &subject,
		string &dst_no_headers)
{
	string headers;
	dst_no_headers = dst;
	t_url u(dst);	
	
	// Split headers from URI
	if (u.is_valid()) {
		// destination is a valid URI. Strip off the headers if any
		headers = u.get_headers();
		
		// Cut off headers
		// Note that a separator (?) will be in front of the 
		// headers string
		if (!headers.empty()) {
			string::size_type i = dst.find(headers);
			if (i != string::npos) {
				dst_no_headers = dst.substr(0, i - 1);
			}
		}
		
		expand_destination(user_config, dst_no_headers, display_url);
	} else {
		// destination may be a short URI.
		// Split at a '?' to find any headers.
		// NOTE: this is not fool proof. A user name may contain a '?'
		vector<string> l = split_on_first(dst, '?');
		dst_no_headers = l[0];
		expand_destination(user_config, dst_no_headers, display_url);
		if (display_url.is_valid() && l.size() == 2) {
			headers = l[1];
		}
	}
	
	// Parse headers to find subject header
	subject.clear();
	if (!headers.empty()) {
		try {
			list<string> parse_errors;
			t_sip_message *m = t_parser::parse_headers(headers, parse_errors);
			if (m->hdr_subject.is_populated()) {
				subject = m->hdr_subject.subject;
			}
			MEMMAN_DELETE(m);
			delete m;
		} catch (int) {
			// ignore invalid headers
		}
	}
}

bool t_userintf::parse_args(const list<string> command_list,
                            list<t_command_arg> &al)
{
	t_command_arg	arg;
	bool parsed_flag = false;

	al.clear();
	arg.flag = 0;
	arg.value = "";

	for (list<string>::const_iterator i = command_list.begin();
	     i != command_list.end(); i++)
	{
		if (i == command_list.begin()) continue;

		const string &s = *i;
		if (s[0] == '-') {
			if (s.size() == 1) return false;
			if (parsed_flag) al.push_back(arg);

			arg.flag = s[1];

			if (s.size() > 2) {
				arg.value = unquote(s.substr(2));
				al.push_back(arg);
				arg.flag = 0;
				arg.value = "";
				parsed_flag = false;
			} else {
				arg.value = "";
				parsed_flag = true;
			}
		} else {
			if (parsed_flag) {
				arg.value = unquote(s);
			} else {
				arg.flag = 0;
				arg.value = unquote(s);
			}

			al.push_back(arg);
			parsed_flag = false;
			arg.flag = 0;
			arg.value = "";
		}
	}

	// Last parsed argument was a flag only
	if (parsed_flag) al.push_back(arg);

	return true;
}

bool t_userintf::exec_invite(const list<string> command_list, bool immediate) {
	list<t_command_arg> al;
	string display;
	string subject;
	string destination;
	bool hide_user = false;

	if (!parse_args(command_list, al)) {
		exec_command("help call");
		return false;
	}

	for (list<t_command_arg>::iterator i = al.begin(); i != al.end(); i++) {
		switch (i->flag) {
		case 'd':
			display = i->value;
			break;
		case 's':
			subject = i->value;
			break;
		case 'h':
			hide_user = true;
			break;
		case 0:
			destination = i->value;
			break;
		default:
			exec_command("help call");
			return false;
			break;
		}
	}

	return do_invite(destination, display, subject, immediate, hide_user);
}

bool t_userintf::do_invite(const string &destination, const string &display,
		const string &subject, bool immediate, bool anonymous)
{
	t_url dest_url(expand_destination(active_user, destination));
	
	if (!dest_url.is_valid()) {
		exec_command("help call");
		return false;
	}

	t_url vm_url(expand_destination(active_user, active_user->get_mwi_vm_address()));
	if (dest_url != vm_url) {
		// Keep call information for redial
		last_called_url = dest_url;
		last_called_display = display;
		last_called_subject = subject;
		last_called_profile = active_user->get_profile_name();
		last_called_hide_user = anonymous;
	}

	phone->pub_invite(active_user, dest_url, display, subject, anonymous);
	return true;
}

bool t_userintf::exec_redial(const list<string> command_list) {
	if (can_redial()) {
		do_redial();
		return true;
	}
	
	return false;
}

void t_userintf::do_redial(void) {
	t_user *user_config = phone->ref_user_profile(last_called_profile);
	phone->pub_invite(user_config, last_called_url, last_called_display,
		last_called_subject, last_called_hide_user);
}

bool t_userintf::exec_answer(const list<string> command_list) {
	do_answer();
	return true;
}

void t_userintf::do_answer(void) {
	cb_stop_call_notification(phone->get_active_line());
	phone->pub_answer();
}

bool t_userintf::exec_answerbye(const list<string> command_list) {
	do_answerbye();
	return true;
}

void t_userintf::do_answerbye(void) {
	unsigned short line = phone->get_active_line();
	
	switch (phone->get_line_substate(line)) {
	case LSSUB_INCOMING_PROGRESS:
		do_answer();
		break;
	case LSSUB_OUTGOING_PROGRESS:
	case LSSUB_ESTABLISHED:
		do_bye();
		break;
	default:
		break;
	}
}

bool t_userintf::exec_reject(const list<string> command_list) {
	do_reject();
	return true;
}

void t_userintf::do_reject(void) {
	cb_stop_call_notification(phone->get_active_line());
	phone->pub_reject();
	cout << endl;
	cout << "Line " << phone->get_active_line() + 1 << ": call rejected.\n";
	cout << endl;
}

bool t_userintf::exec_redirect(const list<string> command_list, bool immediate) {
	list<t_command_arg> al;
	list<string> dest_list;
	int num_redirections = 0;
	bool show_status = false;
	bool action_present = false;
	bool enable = true;
	bool type_present = false;
	t_cf_type cf_type = CF_ALWAYS;

	if (!parse_args(command_list, al)) {
		exec_command("help redirect");
		return false;
	}

	for (list<t_command_arg>::iterator i = al.begin(); i != al.end(); i++) {
		switch (i->flag) {
		case 's':
			show_status = true;
			break;
		case 't':
			if (i->value == "always") {
				cf_type = CF_ALWAYS;
			} else if (i->value == "busy") {
				cf_type = CF_BUSY;
			} else if (i->value == "noanswer") {
				cf_type = CF_NOANSWER;
			} else {
				exec_command("help redirect");
				return false;
			}

			type_present = true;
			break;
		case 'a':
			if (i->value == "on") {
				enable = true;
			} else if (i->value == "off") {
				enable = false;
			} else {
				exec_command("help redirect");
				return false;
			}

			action_present = true;
			break;
		case 0:
			dest_list.push_back(i->value);
			num_redirections++;
			break;
		default:
			exec_command("help redirect");
			return false;
			break;
		}
	}

	if (type_present && enable && (num_redirections == 0 || num_redirections > 5)) {
		exec_command("help redirect");
		return false;
	}
	
	if (!type_present && action_present && enable) { 
		exec_command("help redirect");
		return false;
	}
	
	if (!type_present && !action_present && 
	    (num_redirections == 0 || num_redirections > 5)) 
	{
		exec_command("help redirect");
		return false;
	}
		
	do_redirect(show_status, type_present, cf_type, action_present, enable,
			num_redirections, dest_list, immediate);
	return true;
}

void t_userintf::do_redirect(bool show_status, bool type_present, t_cf_type cf_type, 
		bool action_present, bool enable, int num_redirections,
		const list<string> &dest_strlist, bool immediate)
{
	list<t_display_url> dest_list;
	for (list<string>::const_iterator i = dest_strlist.begin();
	     i != dest_strlist.end(); i++)
	{
		t_display_url du;
		du.url = expand_destination(active_user, *i);
		du.display.clear();
		if (!du.is_valid()) return;
		dest_list.push_back(du);
	}

	if (show_status) {
		list<t_display_url> cf_dest; // call forwarding destinations

		cout << endl;

		cout << "Redirect always: ";
		if (phone->ref_service(active_user)->get_cf_active(CF_ALWAYS, cf_dest)) {
			for (list<t_display_url>::iterator i = cf_dest.begin();
			     i != cf_dest.end(); i++)
			{
				if (i != cf_dest.begin()) cout << ", ";
				cout << i->encode();
			}
		} else {
			cout << "not active";
		}
		cout << endl;

		cout << "Redirect busy: ";
		if (phone->ref_service(active_user)->get_cf_active(CF_BUSY, cf_dest)) {
			for (list<t_display_url>::iterator i = cf_dest.begin();
			     i != cf_dest.end(); i++)
			{
				if (i != cf_dest.begin()) cout << ", ";
				cout << i->encode();
			}
		} else {
			cout << "not active";
		}
		cout << endl;

		cout << "Redirect noanswer: ";
		if (phone->ref_service(active_user)->get_cf_active(CF_NOANSWER, cf_dest)) {
			for (list<t_display_url>::iterator i = cf_dest.begin();
			     i != cf_dest.end(); i++)
			{
				if (i != cf_dest.begin()) cout << ", ";
				cout << i->encode();
			}
		} else {
			cout << "not active";
		}
		cout << endl;

		cout << endl;
		return;
	}

	// Enable/disable permanent redirections
	if (type_present) {
		if (enable) {
			phone->ref_service(active_user)->enable_cf(cf_type, dest_list);
			cout << "Redirection enabled.\n\n";
		} else {
			phone->ref_service(active_user)->disable_cf(cf_type);
			cout << "Redirection disabled.\n\n";
		}
		
		return;
	} else {
		if (action_present) {
			if (!enable) {
				phone->ref_service(active_user)->disable_cf(CF_ALWAYS);
				phone->ref_service(active_user)->disable_cf(CF_BUSY);
				phone->ref_service(active_user)->disable_cf(CF_NOANSWER);
				cout << "All redirections disabled.\n\n";
				return;
			}
			
			return;
		}
	}

	// Redirect current call
	cb_stop_call_notification(phone->get_active_line());
	phone->pub_redirect(dest_list, 302);
	cout << endl;
	cout << "Line " << phone->get_active_line() + 1 << ": call redirected.\n";
	cout << endl;
}

bool t_userintf::exec_dnd(const list<string> command_list) {
	list<t_command_arg> al;
	bool show_status = false;
	bool toggle = true;
	bool enable = false;

	if (!parse_args(command_list, al)) {
		exec_command("help dnd");
		return false;
	}

	for (list<t_command_arg>::iterator i = al.begin(); i != al.end(); i++) {
		switch (i->flag) {
		case 's':
			show_status = true;
			break;
		case 'a':
			if (i->value == "on") {
				enable = true;
			} else if (i->value == "off") {
				enable = false;
			} else {
				exec_command("help dnd");
				return false;
			}
			toggle = false;
			break;
		default:
			exec_command("help dnd");
			return false;
			break;
		}
	}
	
	do_dnd(show_status, toggle, enable);
	return true;
}

void t_userintf::do_dnd(bool show_status, bool toggle, bool enable) {
	if (show_status) {
		cout << endl;
		cout << "Do not disturb: ";
		if (phone->ref_service(active_user)->is_dnd_active()) {
			cout << "active";
		} else {
			cout << "not active";
		}
		cout << endl;
		return;
	}
	
	if (toggle) {
		enable = !phone->ref_service(active_user)->is_dnd_active();
	}

	if (enable) {
		phone->ref_service(active_user)->enable_dnd();
		cout << "Do not disturb enabled.\n\n";
		return;
	} else {
		phone->ref_service(active_user)->disable_dnd();
		cout << "Do not disturb disabled.\n\n";
		return;
	}
}

bool t_userintf::exec_auto_answer(const list<string> command_list) {
	list<t_command_arg> al;
	bool show_status = false;
	bool toggle = true;
	bool enable = false;

	if (!parse_args(command_list, al)) {
		exec_command("help auto_answer");
		return false;
	}

	for (list<t_command_arg>::iterator i = al.begin(); i != al.end(); i++) {
		switch (i->flag) {
		case 's':
			show_status = true;
			break;
		case 'a':
			if (i->value == "on") {
				enable = true;
			} else if (i->value == "off") {
				enable = false;
			} else {
				exec_command("help auto_answer");
				return false;
			}
			toggle = false;
			break;
		default:
			exec_command("help auto_answer");
			return false;
			break;
		}
	}
	
	do_auto_answer(show_status, toggle, enable);
	return true;
}

void t_userintf::do_auto_answer(bool show_status, bool toggle, bool enable) {
	if (show_status) {
		cout << endl;
		cout << "Auto answer: ";
		if (phone->ref_service(active_user)->is_auto_answer_active()) {
			cout << "active";
		} else {
			cout << "not active";
		}
		cout << endl;
		return;
	}
	
	if (toggle) {
		enable = !phone->ref_service(active_user)->is_auto_answer_active();
	}

	if (enable) {
		phone->ref_service(active_user)->enable_auto_answer(true);
		cout << "Auto answer enabled.\n\n";
		return;
	} else {
		phone->ref_service(active_user)->enable_auto_answer(false);
		cout << "Auto answer disabled.\n\n";
		return;
	}
}

bool t_userintf::exec_bye(const list<string> command_list) {
	do_bye();
	return true;
}

void t_userintf::do_bye(void) {
	phone->pub_end_call();
}

bool t_userintf::exec_hold(const list<string> command_list) {
	do_hold();
	return true;
}

void t_userintf::do_hold(void) {
	phone->pub_hold();
}

bool t_userintf::exec_retrieve(const list<string> command_list) {
	do_retrieve();
	return true;
}

void t_userintf::do_retrieve(void) {
	phone->pub_retrieve();
}

bool t_userintf::exec_refer(const list<string> command_list, bool immediate) {
	list<t_command_arg> al;
	string destination;
	bool dest_set = false;
	t_transfer_type transfer_type = TRANSFER_BASIC;

	if (!parse_args(command_list, al)) {
		exec_command("help transfer");
		return false;
	}
	
	for (list<t_command_arg>::iterator i = al.begin(); i != al.end(); i++) {
		switch (i->flag) {
		case 'c':
			if (transfer_type != TRANSFER_BASIC) {
				exec_command("help transfer");
				return false;
			}
			transfer_type = TRANSFER_CONSULT;
			if (!i->value.empty()) {
				destination = i->value;
				dest_set = true;
			}
			break;
		case 'l':
			if (transfer_type != TRANSFER_BASIC) {
				exec_command("help transfer");
				return false;
			}
			transfer_type = TRANSFER_OTHER_LINE;
			break;
		case 0:
			destination = i->value;
			dest_set = true;
			break;
		default:
			exec_command("help transfer");
			return false;
			break;
		}
	}

	if (!dest_set && transfer_type == TRANSFER_BASIC) {
		exec_command("help transfer");
		return false;
	}

	return do_refer(destination, transfer_type, immediate);
}

bool t_userintf::do_refer(const string &destination, t_transfer_type transfer_type, 
		bool immediate) 
{
	t_url dest_url;
	
	if (transfer_type == TRANSFER_BASIC || 
	    (transfer_type == TRANSFER_CONSULT && !destination.empty())) 
	{
		dest_url.set_url(expand_destination(active_user, destination));
	
		if (!dest_url.is_valid()) {
			exec_command("help transfer");
			return false;
		}
	}
	
	unsigned short active_line;
	unsigned short other_line;
	unsigned short line_to_be_transferred;
	
	switch (transfer_type) {
	case TRANSFER_BASIC:
		phone->pub_refer(dest_url, "");
		break;
	case TRANSFER_CONSULT:
		if (destination.empty()) {
			active_line = phone->get_active_line();
			if (!phone->is_line_transfer_consult(active_line,
					line_to_be_transferred)) 
			{
				// There is no call to transfer
				return false;
			}
			phone->pub_refer(line_to_be_transferred, active_line);
		} else {
			phone->pub_setup_consultation_call(dest_url, "");
		}
		break;
	case TRANSFER_OTHER_LINE:
		active_line = phone->get_active_line();
		other_line = (active_line == 0 ? 1 : 0);
		phone->pub_refer(active_line, other_line);
		break;
	}
	
	return true;
}


bool t_userintf::exec_conference(const list<string> command_list) {
	do_conference();
	return true;
}

void t_userintf::do_conference(void) {
	if (phone->join_3way(0, 1)) {
		cout << endl;
		cout << "Started 3-way conference.\n";
		cout << endl;
	} else {
		cout << endl;
		cout << "Failed to start 3-way conference.\n";
		cout << endl;
	}
}

bool t_userintf::exec_mute(const list<string> command_list) {
	list<t_command_arg> al;
	bool show_status = false;
	bool toggle = true;
	bool enable = true;

	if (!parse_args(command_list, al)) {
		exec_command("help mute");
		return false;
	}

	for (list<t_command_arg>::iterator i = al.begin(); i != al.end(); i++) {
		switch (i->flag) {
		case 's':
			show_status = true;
			break;
		case 'a':
			if (i->value == "on") {
				enable = true;
			} else if (i->value == "off") {
				enable = false;
			} else {
				exec_command("help mute");
				return false;
			}
			toggle = false;
			break;
		default:
			exec_command("help mute");
			return false;
			break;
		}
	}
	
	do_mute(show_status, toggle, enable);
	return true;
}

void t_userintf::do_mute(bool show_status, bool toggle, bool enable) {
	if (show_status) {
		cout << endl;
		cout << "Line is ";
		if (phone->is_line_muted(phone->get_active_line())) {
			cout << "muted.";
		} else {
			cout << "not muted.";
		}
		cout << endl;
		return;
	}

	if (toggle) enable = !phone->is_line_muted(phone->get_active_line());
	if (enable) {
		phone->mute(enable);
		cout << "Line muted.\n\n";
		return;
	} else {
		phone->mute(enable);
		cout << "Line unmuted.\n\n";
		return;
	}
}

bool t_userintf::exec_dtmf(const list<string> command_list) {
	list<t_command_arg> al;
	string digits;
	bool raw_mode = false;

	if (phone->get_line_state(phone->get_active_line()) == LS_IDLE) {
		return false;
	}

	if (!parse_args(command_list, al)) {
		exec_command("help dtmf");
		return false;
	}

	for (list<t_command_arg>::iterator i = al.begin(); i != al.end(); i++) {
		switch (i->flag) {
		case 'r':
			raw_mode = true;
			if (!i->value.empty()) digits = i->value;
			break;
		case 0:
			digits = i->value;
			break;
		default:
			exec_command("help dtmf");
			return false;
			break;
		}
	}

	if (!raw_mode) {
		digits = str2dtmf(digits);
	}
	
	if (digits == "") {
		exec_command("help dtmf");
		return false;
	}

	do_dtmf(digits);
	return true;
}

void t_userintf::do_dtmf(const string &digits) {
	const t_call_info call_info = phone->get_call_info(phone->get_active_line());
	throttle_dtmf_not_supported = false;
	
	if (!call_info.dtmf_supported) return;
	
	for (string::const_iterator i = digits.begin(); i != digits.end(); i++) {
		if (VALID_DTMF_SYM(*i)) {
			phone->pub_send_dtmf(*i, call_info.dtmf_inband, call_info.dtmf_info);
		}
	}
}

bool t_userintf::exec_register(const list<string> command_list) {
	list<t_command_arg> al;
	bool reg_all_profiles = false;

	if (!parse_args(command_list, al)) {
		exec_command("help register");
		return false;
	}

	for (list<t_command_arg>::iterator i = al.begin(); i != al.end(); i++) {
		switch (i->flag) {
		case 'a':
			reg_all_profiles = true;
			break;
		default:
			exec_command("help register");
			return false;
			break;
		}
	}
	
	do_register(reg_all_profiles);
	return true;
}

void t_userintf::do_register(bool reg_all_profiles) {
	if (reg_all_profiles) {
		list<t_user *> user_list = phone->ref_users();
		
		for (list<t_user *>::iterator i = user_list.begin();
		     i != user_list.end(); i++)
		{
			phone->pub_registration(*i, REG_REGISTER, 
					DUR_REGISTRATION(*i));
		}	
	} else {
		phone->pub_registration(active_user, REG_REGISTER, 
				DUR_REGISTRATION(active_user));
	}
}

bool t_userintf::exec_deregister(const list<string> command_list) {
	list<t_command_arg> al;
	bool dereg_all_devices = false;
	bool dereg_all_profiles = false;

	if (!parse_args(command_list, al)) {
		exec_command("help deregister");
		return false;
	}

	for (list<t_command_arg>::iterator i = al.begin(); i != al.end(); i++) {
		switch (i->flag) {
		case 'a':
			dereg_all_profiles = true;
			break;
		case 'd':
			dereg_all_devices = true;
			break;
		default:
			exec_command("help deregister");
			return false;
			break;
		}
	}

	do_deregister(dereg_all_profiles, dereg_all_devices);
	return true;
}

void t_userintf::do_deregister(bool dereg_all_profiles, bool dereg_all_devices) {
	t_register_type dereg_type = REG_DEREGISTER;
	
	if (dereg_all_devices) {
		dereg_type = REG_DEREGISTER_ALL;
	}
	
	if (dereg_all_profiles) {
		list<t_user *> user_list = phone->ref_users();
		
		for (list<t_user *>::iterator i = user_list.begin();
		     i != user_list.end(); i++)
		{
			phone->pub_registration(*i, dereg_type);
		}
	} else {
		phone->pub_registration(active_user, dereg_type);
	}
}

bool t_userintf::exec_fetch_registrations(const list<string> command_list) {
	do_fetch_registrations();
	return true;
}

void t_userintf::do_fetch_registrations(void) {
	phone->pub_registration(active_user, REG_QUERY);
}

bool t_userintf::exec_options(const list<string> command_list, bool immediate) {
	list<t_command_arg> al;
	string destination;
	bool dest_set = false;

	if (!parse_args(command_list, al)) {
		exec_command("help options");
		return false;
	}
	
	for (list<t_command_arg>::iterator i = al.begin(); i != al.end(); i++) {
		switch (i->flag) {
		case 0:
			destination = i->value;
			dest_set = true;
			break;
		default:
			exec_command("help options");
			return false;
			break;
		}
	}

	if (!dest_set) {
		if (phone->get_line_state(phone->get_active_line()) == LS_IDLE) {
			exec_command("help options");
			return false;
		}
	}

	return do_options(dest_set, destination, immediate);
}

bool t_userintf::do_options(bool dest_set, const string &destination, bool immediate) {
	if (!dest_set) {
		phone->pub_options();
	} else {
		t_url dest_url;
		dest_url.set_url(expand_destination(active_user, destination));
		
		if (!dest_url.is_valid()) {
			exec_command("help options");
			return false;
		}
		
		phone->pub_options(active_user, dest_url);
	}
	
	return true;
}

bool t_userintf::exec_line(const list<string> command_list) {
	list<t_command_arg> al;
	int line = 0;

	if (!parse_args(command_list, al)) {
		exec_command("help line");
		return false;
	}

	for (list<t_command_arg>::iterator i = al.begin(); i != al.end(); i++) {
		switch (i->flag) {
		case 0:
			line = atoi(i->value.c_str());
			break;
		default:
			exec_command("help line");
			return false;
			break;
		}
	}

	if (line < 0 || line > 2) {
		exec_command("help line");
		return false;
	}

	do_line(line);
	return true;
}

void t_userintf::do_line(int line) {
	if (line == 0) {
		cout << endl;
		cout << "Active line is: " << phone->get_active_line()+1 << endl;
		cout << endl;
		return;
	}

	int current = phone->get_active_line();

	if (line == current + 1) {
		cout << endl;
		cout << "Line " << current + 1 << " is already active.\n";
		cout << endl;
		return;
	}

	phone->pub_activate_line(line - 1);
	if (phone->get_active_line() == current) {
		cout << endl;
		cout << "Current call cannot be put on-hold.\n";
		cout << "Cannot switch to another line now.\n";
		cout << endl;
	} else {
		cout << endl;
		cout << "Line " << phone->get_active_line()+1 << " is now active.\n";
		cout << endl;
	}
}

bool t_userintf::exec_user(const list<string> command_list) {
	list<t_command_arg> al;
	string profile_name;

	if (!parse_args(command_list, al)) {
		exec_command("help user");
		return false;
	}
	
	for (list<t_command_arg>::iterator i = al.begin(); i != al.end(); i++) {
		switch (i->flag) {
		case 0:
			profile_name = i->value;
			break;
		default:
			exec_command("help user");
			return false;
			break;
		}
	}
	
	do_user(profile_name);
	return true;
}

void t_userintf::do_user(const string &profile_name) {
	list<t_user *> user_list = phone->ref_users();
	if (profile_name.empty()) {
		// Show all users
		cout << endl;
		for (list<t_user *>::iterator i = user_list.begin();
		     i != user_list.end(); i++)
		{
			if (*i == active_user) {
				cout << "* ";
			} else {
				cout << "  ";
			}
			
			cout << (*i)->get_profile_name();
			cout << "\n    ";
			cout << (*i)->get_display(false);
			cout << " <sip:" << (*i)->get_name();
			cout << "@" << (*i)->get_domain() << ">\n";
		}
		cout << endl;
		return;
	}

	for (list<t_user *>::iterator i = user_list.begin();
	     i != user_list.end(); i++)
	{
		if ((*i)->get_profile_name() == profile_name) {
			active_user = (*i);
			cout << endl;
			cout << profile_name;
			cout << " activated.\n";
			cout << endl;
			
			return;
		}
	}
	
	cout << endl;
	cout << "Unknown user profile: ";
	cout << profile_name;
	cout << endl << endl;
}

bool t_userintf::exec_zrtp(const list<string> command_list) {
	list<t_command_arg> al;
	t_zrtp_cmd zrtp_cmd = ZRTP_ENCRYPT;

	if (!parse_args(command_list, al)) {
		exec_command("help zrtp");
		return false;
	}
	
	if (al.size() != 1) {
		exec_command("help zrtp");
		return false;
	}
	
	for (list<t_command_arg>::iterator i = al.begin(); i != al.end(); i++) {
		switch (i->flag) {
		case 0:
			if (i->value == "encrypt") {
				zrtp_cmd = ZRTP_ENCRYPT;
			} else if (i->value == "go-clear") {
				zrtp_cmd = ZRTP_GO_CLEAR;
			} else if (i->value == "confirm-sas") {
				zrtp_cmd = ZRTP_CONFIRM_SAS;
			} else if (i->value == "reset-sas") {
				zrtp_cmd = ZRTP_RESET_SAS;
			} else {
				exec_command("help zrtp");
				return false;
			}
			break;
		default:
			exec_command("help zrtp");
			return false;
			break;
		}
	}
	
	do_zrtp(zrtp_cmd);
	return true;
}

void t_userintf::do_zrtp(t_zrtp_cmd zrtp_cmd) {
	switch (zrtp_cmd) {
	case ZRTP_ENCRYPT:
		phone->pub_enable_zrtp();
		break;
	case ZRTP_GO_CLEAR:
		phone->pub_zrtp_request_go_clear();
		break;
	case ZRTP_CONFIRM_SAS:
		phone->pub_confirm_zrtp_sas();
		break;
	case ZRTP_RESET_SAS:
		phone->pub_reset_zrtp_sas_confirmation();
		break;
	default:
		assert(false);
	}
}

bool t_userintf::exec_message(const list<string> command_list) {
	list<t_command_arg> al;
	string display;
	string subject;
	string filename;
	string destination;
	string text;

	if (!parse_args(command_list, al)) {
		exec_command("help message");
		return false;
	}

	for (list<t_command_arg>::iterator i = al.begin(); i != al.end(); i++) {
		switch (i->flag) {
		case 's':
			subject = i->value;
			break;
		case 'f':
			filename = i->value;
			break;
		case 'd':
			display = i->value;
			break;
		case 0:
			if (destination.empty()) {
				destination = i->value;
			} else {
				text = i->value;
			}
			break;
		default:
			exec_command("help message");
			return false;
			break;
		}
	}
	
	if (destination.empty() || (text.empty() && filename.empty())) {
		exec_command("help message");
		return false;
	}
	
	im::t_msg msg(text, im::MSG_DIR_OUT, im::TXT_PLAIN);
	msg.subject = subject;
	
	if (!filename.empty()) {
		t_media media("application/octet-stream");
		string mime_type = mime_database->get_mimetype(filename);
		
		if (!mime_type.empty()) {
			media = t_media(mime_type);
		}
		
		msg.set_attachment(filename, media, strip_path_from_filename(filename));
	}

	return do_message(destination, display, msg);
}

bool t_userintf::do_message(const string &destination, const string &display,
		const im::t_msg &msg)
{
	t_url dest_url(expand_destination(active_user, destination));
	
	if (!dest_url.is_valid()) {
		exec_command("help message");
		return false;
	}
	
	(void)phone->pub_send_message(active_user, dest_url, display, msg);
	return true;
}

bool t_userintf::exec_presence(const list<string> command_list) {
	list<t_command_arg> al;
	t_presence_state::t_basic_state basic_state = t_presence_state::ST_BASIC_OPEN;

	if (!parse_args(command_list, al)) {
		exec_command("help presence");
		return false;
	}

	for (list<t_command_arg>::iterator i = al.begin(); i != al.end(); i++) {
		switch (i->flag) {
		case 'b':
			if (i->value == "online") {
				basic_state = t_presence_state::ST_BASIC_OPEN;
			} else if (i->value == "offline") {
				basic_state = t_presence_state::ST_BASIC_CLOSED;
			} else {
				exec_command("help presence");
				return false;
			}
			break;
		default:
			exec_command("help presence");
			return false;
			break;
		}
	}

	do_presence(basic_state);
	return true;
}

void t_userintf::do_presence(t_presence_state::t_basic_state basic_state)
{
	phone->pub_publish_presence(active_user, basic_state);
}

bool t_userintf::exec_quit(const list<string> command_list) {
	do_quit();
	return true;
}

void t_userintf::do_quit(void) {
	end_interface = true;
}

bool t_userintf::exec_help(const list<string> command_list) {
	list<t_command_arg> al;

	if (!parse_args(command_list, al)) {
		exec_command("help help");
		return false;
	}

	if (al.size() > 1) {
		exec_command("help help");
		return false;
	}
	
	do_help(al);
	return true;
}
	
void t_userintf::do_help(const list<t_command_arg> &al) {
	if (al.size() == 0) {
		cout << endl;
		cout << "call		Call someone\n";
		cout << "answer		Answer an incoming call\n";
		cout << "answerbye	Answer an incoming call or end a call\n";
		cout << "reject		Reject an incoming call\n";
		cout << "redirect	Redirect an incoming call\n";
		cout << "transfer	Transfer a standing call\n";
		cout << "bye		End a call\n";
		cout << "hold		Put a call on-hold\n";
		cout << "retrieve	Retrieve a held call\n";
		cout << "conference	Join 2 calls in a 3-way conference\n";
		cout << "mute		Mute a line\n";
		cout << "dtmf		Send DTMF\n";
		cout << "redial		Repeat last call\n";
		cout << "register	Register your phone at a registrar\n";
		cout << "deregister	De-register your phone at a registrar\n";
		cout << "fetch_reg	Fetch registrations from registrar\n";
		cout << "options\t\tGet capabilities of another SIP endpoint\n";
		cout << "line		Toggle between phone lines\n";
		cout << "dnd		Do not disturb\n";
		cout << "auto_answer	Auto answer\n";
		cout << "user		Show users / set active user\n";
#ifdef HAVE_ZRTP
		cout << "zrtp		ZRTP command for voice encryption\n";
#endif
		cout << "message\t\tSend an instant message\n";
		cout << "presence	Publish your presence state\n";
		cout << "quit		Quit\n";
		cout << "help		Get help on a command\n";
		cout << endl;

		return;
	}

	bool ambiguous;
	string c = complete_command(tolower(al.front().value), ambiguous);

	if (c == "call") {
		cout << endl;
		cout << "Usage:\n";
		cout << "\tcall [-s subject] [-d display] [-h] dst\n";
		cout << "Description:\n";
		cout << "\tCall someone.\n";
		cout << "Arguments:\n";
		cout << "\t-s subject	Add a subject header to the INVITE\n";
		cout << "\t-d display	Add display name to To-header\n";
		cout << "\t-h		Hide your identity\n";
		cout << "\tdst		SIP uri of party to invite\n";
		cout << endl;

		return;
	}

	if (c == "answer") {
		cout << endl;
		cout << "Usage:\n";
		cout << "\tanswer\n";
		cout << "Description:\n";
		cout << "\tAnswer an incoming call.\n";
		cout << endl;

		return;
	}
	
	if (c == "answerbye") {
		cout << endl;
		cout << "Usage:\n";
		cout << "\tanswerbye\n";
		cout << "Description:\n";
		cout << "\tWith this command you can answer an incoming call or\n";
		cout << "\tend an established call.\n";
		cout << endl;

		return;
	}

	if (c == "reject") {
		cout << endl;
		cout << "Usage:\n";
		cout << "\treject\n";
		cout << "Description:\n";
		cout << "\tReject an incoming call. A 603 Decline response\n";
		cout << "\twill be sent.\n";
		cout << endl;

		return;
	}

	if (c == "redirect") {
		cout << endl;
		cout << "Usage:\n";
		cout << "\tredirect [-s] [-t type] [-a on|off] [dst ... dst]\n";
		cout << "Description:\n";
		cout << "\tRedirect an incoming call. A 302 Moved Temporarily\n";
		cout << "\tresponse will be sent.\n";
		cout << "\tYou can redirect the current incoming call by specifying\n";
		cout << "\tone or more destinations without any other arguments.\n";
		cout << "Arguments:\n";
		cout << "\t-s		Show which redirections are active.\n";
		cout << "\t-t type\t	Type for permanent redirection of calls.\n";
		cout << "\t		Values: always, busy, noanswer.\n";
		cout << "\t-a on|off	Enable/disable permanent redirection.\n";
		cout << "\t		The default action is 'on'.\n";
		cout << "\t		You can disable all redirections with the\n";
		cout << "\t		'off' action and no type.\n";
		cout << "\tdst		SIP uri where the call should be redirected.\n";
		cout << "\t		You can specify up to 5 destinations.\n";
		cout << "\t		The destinations will be tried in sequence.\n";
		cout << "Examples:\n";
		cout << "\tRedirect current incoming call to michel@twinklephone.com\n";
		cout << "\tredirect michel@twinklephone.com\n";
		cout << endl;
		cout << "\tRedirect busy calls permanently to michel@twinklephone.com\n";
		cout << "\tredirect -t busy michel@twinklephone.com\n";
		cout << endl;
		cout << "\tDisable redirection of busy calls.\n";
		cout << "\tredirect -t busy -a off\n";
		cout << endl;

		return;
	}

	if (c == "transfer") {
		cout << endl;
		cout << "Usage:\n";
		cout << "\ttransfer [-c] [-l] [dst]\n";
		cout << "Description:\n";
		cout << "\tTransfer a standing call to another destination.\n";
		cout << "\tFor a transfer with consultation, first use the -c flag with a\n";
		cout << "\tdestination. This sets up the consultation call. When the\n";
		cout << "\tconsulted party agrees, give the command with the -c flag once\n";
		cout << "\tmore, but now without a destination. This transfers the call.\n";
		cout << "Arguments:\n";
		cout << "\t-c	Consult destination before transferring call.\n";
		cout << "\t-l	Transfer call to party on other line.\n";
		cout << "\tdst	SIP uri of transfer destination\n";
		cout << endl;

		return;
	}

	if (c == "bye") {
		cout << endl;
		cout << "Usage:\n";
		cout << "\tbye\n";
		cout << "Description:\n";
		cout << "\tEnd a call.\n";
		cout << "\tFor a stable call a BYE will be sent.\n";
		cout << "\tIf the invited party did not yet sent a final answer,\n";
		cout << "\tthen a CANCEL will be sent.\n";
		cout << endl;

		return;
	}

	if (c == "hold") {
		cout << endl;
		cout << "Usage:\n";
		cout << "\thold\n";
		cout << "Description:\n";
		cout << "\tPut the current call on the acitve line on-hold.\n";
		cout << endl;

		return;
	}

	if (c == "retrieve") {
		cout << endl;
		cout << "Usage:\n";
		cout << "\tretrieve\n";
		cout << "Description:\n";
		cout << "\tRetrieve a held call on the active line.\n";
		cout << endl;

		return;
	}

	if (c == "conference") {
		cout << endl;
		cout << "Usage:\n";
		cout << "\tconference\n";
		cout << "Description:\n";
		cout << "\tJoin 2 calls in a 3-way conference. Before you give this\n";
		cout << "\tcommand you must have a call on each line.\n";
		cout << endl;

		return;
	}

	if (c == "mute") {
		cout << endl;
		cout << "Usage:\n";
		cout << "\tmute [-s] [-a on|off]\n";
		cout << "Description:\n";
		cout << "\tMute/unmute the active line.\n";
		cout << "\tYou can hear the other side of the line, but they cannot\n";
		cout << "\thear you.\n";
		cout << "Arguments:\n";
		cout << "\t-s		Show if line is muted.\n";
		cout << "\t-a on|off	Mute/unmute.\n";
		cout << "Notes:\n";
		cout << "\tWithout any arguments you can toggle the status.\n";
		cout << endl;

		return;
	}

	if (c == "dtmf") {
		cout << endl;
		cout << "Usage:\n";
		cout << "\tdtmf digits\n";
		cout << "Description:\n";
		cout << "\tSend the digits as out-of-band DTMF telephone events ";
		cout << "(RFC 2833).\n";
		cout << "\tThis command can only be given when a call is ";
		cout << "established.\n";
		cout << "Arguments:\n";
		cout << "\t-r	Raw mode: do not convert letters to digits.\n";
		cout << "\tdigits	0-9 | A-D | * | #\n";
		cout << "Example:\n";
		cout << "\tdtmf 1234#\n";
		cout << "\tdmtf movies\n";
		cout << "Notes:\n";
		cout << "\tThe overdecadic digits A-D can only be sent in raw mode.\n";
		cout << endl;

		return;
	}
	
	if (c == "redial") {
		cout << endl;
		cout << "Usage:\n";
		cout << "\tredial\n";
		cout << "Description:\n";
		cout << "\tRepeat last call.\n";
		cout << endl;

		return;
	}

	if (c == "register") {
		cout << endl;
		cout << "Usage:\n";
		cout << "\tregister\n";
		cout << "Description:\n";
		cout << "\tRegister your phone at a registrar.\n";
		cout << "Arguments:\n";
		cout << "\t-a	Register all enabled user profiles.\n";
		cout << endl;

		return;
	}

	if (c == "deregister") {
		cout << endl;
		cout << "Usage:\n";
		cout << "\tderegister [-a]\n";
		cout << "Description:\n";
		cout << "\tDe-register your phone at a registrar.\n";
		cout << "Arguments:\n";
		cout << "\t-a	De-register all enabled user profiles.\n";
		cout << "\t-d	De-register all devices.\n";
		cout << endl;

		return;
	}

	if (c == "fetch_reg") {
		cout << endl;
		cout << "Usage:\n";
		cout << "\tfetch_reg\n";
		cout << "Description:\n";
		cout << "\tFetch current registrations from registrar.\n";
		cout << endl;

		return;
	}

	if (c == "options") {
		cout << endl;
		cout << "Usage:\n";
		cout << "\toptions [dst]\n";
		cout << "Description:\n";
		cout << "\tGet capabilities of another SIP endpoint.\n";
		cout << "\tIf no destination is passed as an argument, then\n";
		cout << "\tthe capabilities of the far-end in the current call\n";
		cout << "\ton the active line are requested.\n";
		cout << "Arguments:\n";
		cout << "\tdst		SIP uri of end-point\n";
		cout << endl;

		return;
	}

	if (c == "line") {
		cout << endl;
		cout << "Usage:\n";
		cout << "\tline [lineno]\n";
		cout << "Description:\n";
		cout << "\tIf no argument is passed then the current active ";
		cout << "line is shown\n";
		cout << "\tOtherwise switch to another line. If the current active\n";
		cout << "\thas a call, then this call will be put on-hold.\n";
		cout << "\tIf the new active line has a held call, then this call\n";
		cout << "\twill be retrieved.\n";
		cout << "Arguments:\n";
		cout << "\tlineno		Switch to another line (values = ";
		cout << "1,2)\n";
		cout << endl;

		return;
	}

	if (c == "dnd") {
		cout << endl;
		cout << "Usage:\n";
		cout << "\tdnd [-s] [-a on|off]\n";
		cout << "Description:\n";
		cout << "\tEnable/disable the do not disturb service.\n";
		cout << "\tIf dnd is enabled then a 480 Temporarily Unavailable ";
		cout << "response is given\n";
		cout << "\ton incoming calls.\n";
		cout << "Arguments:\n";
		cout << "\t-s		Show if dnd is active.\n";
		cout << "\t-a on|off	Enable/disable dnd.\n";
		cout << "Notes:\n";
		cout << "\tWithout any arguments you can toggle the status.\n";
		cout << endl;

		return;
	}
	
	if (c == "auto_answer") {
		cout << endl;
		cout << "Usage:\n";
		cout << "\tauto_answer [-s] [-a on|off]\n";
		cout << "Description:\n";
		cout << "\tEnable/disable the auto answer service.\n";
		cout << "Arguments:\n";
		cout << "\t-s		Show if auto answer is active.\n";
		cout << "\t-a on|off	Enable/disable auto answer.\n";
		cout << "Notes:\n";
		cout << "\tWithout any arguments you can toggle the status.\n";
		cout << endl;

		return;
	}
	
	if (c == "user") {
		cout << endl;
		cout << "Usage:\n";
		cout << "\tuser [profile name]\n";
		cout << "Description:\n";
		cout << "\tMake a user profile the active profile.\n";
		cout << "\tCommands like 'invite' are executed for the active profile.\n";
		cout << "\tWithout an argument this command lists all users. The active\n";
		cout << "\tuser will be marked with '*'.\n";
		cout << "Arguments:\n";
		cout << "\tprofile name	The user profile to activate.\n";
		cout << endl;
		
		return;
	}
	
#ifdef HAVE_ZRTP
	if (c == "zrtp") {
		cout << endl;
		cout << "Usage:\n";
		cout << "\tzrtp <zrtp-command>\n";
		cout << "Description:\n";
		cout << "\tExecute a ZRTP command.\n";
		cout << "ZRTP commands:\n";
		cout << "\tencrypt      Start ZRTP negotiation for encryption.\n";
		cout << "\tgo-clear     Send ZRTP go-clear request.\n";
		cout << "\tconfirm-sas  Confirm the SAS value.\n";
		cout << "\treset-sas    Reset SAS confirmation.\n";
		cout << endl;
		
		return;
	}
#endif
	
	if (c == "message") {
		cout << endl;
		cout << "Usage:\n";
		cout << "\tmessage [-s subject] [-f file name] [-d display] dst [text]\n";
		cout << "Description:\n";
		cout << "\tSend an instant message.\n";
		cout << "Arguments:\n";
		cout << "\t-s subject	Subject of the message.\n";
		cout << "\t-f file name	File name of the file to send.\n";
		cout << "\t-d display	Add display name to To-header\n";
		cout << "\tdst		SIP uri of party to message\n";
		cout << "\ttext		Message text to send. Surround with double quotes\n";
		cout << "\t\t\twhen your text contains whitespace.\n";
		cout << "\t\t\tWhen you send a file, then the text is ignored.\n";
		cout << endl;
		
		return;
	}
	
	if (c == "presence") {
		cout << endl;
		cout << "Usage:\n";
		cout << "\tpresence -b [online|offline]\n";
		cout << "Description:\n";
		cout << "\tPublish your presence state to a presence agent\n";
		cout << "Arguments:\n";
		cout << "\t-b		A basic presence state: online or offline\n";
		cout << endl;
		
		return;
	}

	if (c == "quit") {
		cout << endl;
		cout << "Usage:\n";
		cout << "\tquit\n";
		cout << "Description:\n";
		cout << "\tQuit.\n";
		cout << endl;

		return;
	}

	if (c == "help") {
		cout << endl;
		cout << "Usage:\n";
		cout << "\thelp [command]\n";
		cout << "Description:\n";
		cout << "\tShow help on a command.\n";
		cout << "Arguments:\n";
		cout << "\tcommand		Command you want help with\n";
		cout << endl;

		return;
	}

	cout << endl;
	cout << "\nUnknown command\n\n";
	cout << endl;
}


/////////////////////////////
// Public
/////////////////////////////

t_userintf::t_userintf(t_phone *_phone) {
	phone = _phone;
	end_interface = false;
	tone_gen = NULL;
	active_user = NULL;
	use_stdout = true;
	throttle_dtmf_not_supported = false;
	thr_process_events = NULL;

	all_commands.push_back("invite");
	all_commands.push_back("call");
	all_commands.push_back("answer");
	all_commands.push_back("answerbye");
	all_commands.push_back("reject");
	all_commands.push_back("redirect");
	all_commands.push_back("bye");
	all_commands.push_back("hold");
	all_commands.push_back("retrieve");
	all_commands.push_back("refer");
	all_commands.push_back("transfer");
	all_commands.push_back("conference");
	all_commands.push_back("mute");
	all_commands.push_back("dtmf");
	all_commands.push_back("redial");
	all_commands.push_back("register");
	all_commands.push_back("deregister");
	all_commands.push_back("fetch_reg");
	all_commands.push_back("options");
	all_commands.push_back("line");
	all_commands.push_back("dnd");
	all_commands.push_back("auto_answer");
	all_commands.push_back("user");
#ifdef HAVE_ZRTP
	all_commands.push_back("zrtp");
#endif
	all_commands.push_back("message");
	all_commands.push_back("presence");
	all_commands.push_back("quit");
	all_commands.push_back("exit");
	all_commands.push_back("q");
	all_commands.push_back("x");
	all_commands.push_back("help");
	all_commands.push_back("h");
	all_commands.push_back("?");
}

t_userintf::~t_userintf() {
	if (tone_gen) {
		MEMMAN_DELETE(tone_gen);
		delete tone_gen;
	}
	
	if (thr_process_events) {
		evq_ui_events.push_quit();
		thr_process_events->join();
		log_file->write_report("thr_process_events stopped.", 
			"t_userintf::~t_userintf", LOG_NORMAL, LOG_DEBUG);
		MEMMAN_DELETE(thr_process_events);
		delete thr_process_events;
	}
}

string t_userintf::complete_command(const string &c, bool &ambiguous) {
	ambiguous = false;
	string full_command;

	for (list<string>::const_iterator i = all_commands.begin();
	     i != all_commands.end(); i++)
	{

		// If there is an exact match, then this is the command.
		// This allows a one command to be a prefix of another command.
		if (c == *i) {
			ambiguous = false;
			return c;
		}

		if (c.size() < i->size() && c == i->substr(0, c.size())) {
			if (full_command != "") {
				ambiguous = true;
				// Do not return here, as there might still be
				// an exact match.
			}

			full_command = *i;
		}
	}

	if (ambiguous) return "";
	return full_command;
}

bool t_userintf::exec_command(const string &command_line, bool immediate) {
	vector<string> v = split_ws(command_line, true);
	if (v.size() == 0) return false;

	bool ambiguous;
	string command = complete_command(tolower(v[0]), ambiguous);

	if (ambiguous) {
		if (use_stdout) {
			cout << endl;
			cout << "Ambiguous command\n";
			cout << endl;
		}
		
		return false;
	}
	
	list<string> l(v.begin(), v.end());

	if (command == "invite") return exec_invite(l, immediate);
	if (command == "call") return exec_invite(l, immediate);
	if (command == "answer") return exec_answer(l);
	if (command == "answerbye") return exec_answerbye(l);
	if (command == "reject") return exec_reject(l);
	if (command == "redirect") return exec_redirect(l, immediate);
	if (command == "bye") return exec_bye(l);
	if (command == "hold") return exec_hold(l);
	if (command == "retrieve") return exec_retrieve(l);
	if (command == "refer") return exec_refer(l, immediate);
	if (command == "transfer") return exec_refer(l, immediate);
	if (command == "conference") return exec_conference(l);
	if (command == "mute") return exec_mute(l);
	if (command == "dtmf") return exec_dtmf(l);
	if (command == "redial") return exec_redial(l);
	if (command == "register") return exec_register(l);
	if (command == "deregister") return exec_deregister(l);
	if (command == "fetch_reg") return exec_fetch_registrations(l);
	if (command == "options") return exec_options(l, immediate);
	if (command == "line") return exec_line(l);
	if (command == "dnd") return exec_dnd(l);
	if (command == "auto_answer") return exec_auto_answer(l);
	if (command == "user") return exec_user(l);
#ifdef HAVE_ZRTP
	if (command == "zrtp") return exec_zrtp(l);
#endif
	if (command == "message") return exec_message(l);
	if (command == "presence") return exec_presence(l);
	if (command == "quit") return exec_quit(l);
	if (command == "exit") return exec_quit(l);
	if (command == "x") return exec_quit(l);
	if (command == "q") return exec_quit(l);
	if (command == "help") return exec_help(l);
	if (command == "h") return exec_help(l);
	if (command == "?") return exec_help(l);

	if (use_stdout) {
		cout << endl;
		cout << "Unknown command\n";
		cout << endl;
	}
	
	return false;
}

string t_userintf::format_sip_address(t_user *user_config, const string &display,
	                              const t_url &uri) const
{
	string s;
	
	if (uri.encode() == ANONYMOUS_URI) {
		return TRANSLATE("Anonymous");
	}

	s = display;
	if (display != "") s += " <";
	
	string number;
	if (uri.get_scheme() == "tel") {
		number = uri.get_host();
	} else {
		number = uri.get_user();
	}

	if (user_config->get_display_useronly_phone() &&
	    uri.is_phone(user_config->get_numerical_user_is_phone(),
	    			user_config->get_special_phone_symbols()))
	{
		// Display telephone number only
		s += user_config->convert_number(number);
	} else {
		// Display full URI
		// Convert the username according to the number conversion
		// rules.
		t_url u(uri);
		string username = user_config->convert_number(number);
		if (username != number) {
			if (uri.get_scheme() == "tel") {
				u.set_host(username);
			} else {
				u.set_user(username);
			}
		}
		s += u.encode_no_params_hdrs(false);
	}

	if (display != "") s += ">";

	return s;
}

list<string> t_userintf::format_warnings(const t_hdr_warning &hdr_warning) const {
	string s;
	list<string> l;

	for (list<t_warning>::const_iterator i = hdr_warning.warnings.begin();
	     i != hdr_warning.warnings.end(); i++)
	{
		s = TRANSLATE("Warning:");
		s += " ";
		s += int2str(i->code);
		s += ' ';
		s += i->text;
		s += " (";
		s += i->host;
		if (i->port > 0) s += int2str(i->port, ":%d");
		s += ')';
		l.push_back(s);
	}

	return l;
}

string t_userintf::format_codec(t_audio_codec codec) const {
	switch (codec) {
	case CODEC_NULL: 	return "null";
	case CODEC_UNSUPPORTED:	return "???";
	case CODEC_G711_ALAW:	return "g711a";
	case CODEC_G711_ULAW:	return "g711u";
	case CODEC_GSM:		return "gsm";
	case CODEC_SPEEX_NB:	return "spx-nb";
	case CODEC_SPEEX_WB:	return "spx-wb";
	case CODEC_SPEEX_UWB:	return "spx-uwb";
	case CODEC_ILBC:	return "ilbc";
	case CODEC_G726_16:	return "g726-16";
	case CODEC_G726_24:	return "g726-24";
	case CODEC_G726_32:	return "g726-32";
	case CODEC_G726_40:	return "g726-40";
	default:		return "???";
	}
}

void t_userintf::run(void) {
	// Start asynchronous event processor
	thr_process_events = new t_thread(process_events_main, NULL);
	MEMMAN_NEW(thr_process_events);
	
	list<t_user *> user_list = phone->ref_users();
	active_user = user_list.front();

	cout << PRODUCT_NAME << " " << PRODUCT_VERSION << ", " << PRODUCT_DATE;
	cout << endl;
	cout << "Copyright (C) 2005-2009  " << PRODUCT_AUTHOR << endl;
	cout << endl;
	
	cout << "Users:";
	exec_command("user");
	
	cout << "Local IP:       " << user_host << endl;
	cout << endl;
	
	restore_state();

	// Initialize phone functions
	phone->init();

	//Initialize GNU readline functions
	rl_attempted_completion_function = tw_completion;
	using_history();
	read_history(sys_config->get_history_file().c_str());
	stifle_history(CLI_MAX_HISTORY_LENGTH);


	while (!end_interface) {
		char *command_line = tw_readline(CLI_PROMPT);
		if (!command_line){
			cout << endl;
			break;
		}
		
		exec_command(command_line);
	}
	
	// Terminate phone functions
	write_history(sys_config->get_history_file().c_str());
	phone->terminate();
	
	save_state();
	cout << endl;
}

void t_userintf::process_events(void) {
	t_event		*event;
	t_event_ui	*ui_event;
	
	bool quit = false;
	while (!quit) {
		event = evq_ui_events.pop();
		switch (event->get_type()) {
		case EV_UI:
			ui_event = dynamic_cast<t_event_ui *>(event);
			assert(ui_event);
			ui_event->exec(this);
			break;
		case EV_QUIT:
			quit = true;
			break;
		default:
			assert(false);
			break;
		}
			
		MEMMAN_DELETE(event);
		delete event;
	}
}

void t_userintf::save_state(void) {
	string err_msg;
	
	sys_config->set_redial_url(last_called_url);
	sys_config->set_redial_display(last_called_display);
	sys_config->set_redial_subject(last_called_subject);
	sys_config->set_redial_profile(last_called_profile);
	sys_config->set_redial_hide_user(last_called_hide_user);
	
	sys_config->write_config(err_msg);
}

void t_userintf::restore_state(void) {
	last_called_url = sys_config->get_redial_url();
	last_called_display = sys_config->get_redial_display();
	last_called_subject = sys_config->get_redial_subject();
	last_called_profile = sys_config->get_redial_profile();
	last_called_hide_user = sys_config->get_redial_hide_user();
}

void t_userintf::lock(void) {
	assert(!is_prohibited_thread());
	// TODO: lock for CLI
}

void t_userintf::unlock(void) {
	// TODO: lock for CLI
}

string t_userintf::select_network_intf(void) {
	string ip;
	list<t_interface> *l = get_interfaces();
	// As memman has no hooks in the socket routines, report it here.
	MEMMAN_NEW(l);
	if (l->size() == 0) {
		// cout << "Cannot find a network interface\n";
		cout << "Cannot find a network interface. Twinkle will use\n"
			"127.0.0.1 as the local IP address. When you connect to\n"
			"the network you have to restart Twinkle to use the correct\n"
			"IP address.\n";
		MEMMAN_DELETE(l);
		delete l;
		return "127.0.0.1";
	}

	if (l->size() == 1) {
		ip = l->front().get_ip_addr();
	} else {
		size_t num = 1;
		cout << "Multiple network interfaces found.\n";
		for (list<t_interface>::iterator i = l->begin(); i != l->end(); i++) {
			cout << num << ") " << i->name << ": ";
			cout << i->get_ip_addr() << endl;
			num++;
		}

		cout << endl;

		size_t selection = 0;
		while (selection < 1 || selection > l->size()) {
			cout << "Which interface do you want to use (enter number): ";
			string choice;
			getline(cin, choice);
			selection = atoi(choice.c_str());
		}

		num = 1;
		for (list<t_interface>::iterator i = l->begin(); i != l->end(); i++) {
			if (num == selection) {
				ip = i->get_ip_addr();
				break;
			}
			num++;
		}

	}

	MEMMAN_DELETE(l);
	delete l;
	return ip;
}

bool t_userintf::select_user_config(list<string> &config_files) {
	// In CLI mode, simply select the default config file
	config_files.clear();
	config_files.push_back(USER_CONFIG_FILE);
	return true;
}

void t_userintf::cb_incoming_call(t_user *user_config, int line, const t_request *r) {
	if (line >= NUM_USER_LINES) return;
	
	cout << endl;
	cout << "Line " << line + 1 << ": ";
	cout << "incoming call\n";
	cout << "From:\t\t";
	
	string from_party = format_sip_address(user_config, 
		r->hdr_from.get_display_presentation(), r->hdr_from.uri);
	cout << from_party << endl;

	if (r->hdr_organization.is_populated()) {
		cout << "Organization:\t" << r->hdr_organization.name << endl;
	}

	cout << "To:\t\t";
	cout << format_sip_address(user_config, r->hdr_to.display, r->hdr_to.uri) << endl;

	if (r->hdr_referred_by.is_populated()) {
		cout << "Referred-by:\t";
		cout << format_sip_address(user_config, r->hdr_referred_by.display,
			r->hdr_referred_by.uri);
		cout << endl;
	}

	if (r->hdr_subject.is_populated()) {
		cout << "Subject:\t" << r->hdr_subject.subject << endl;
	}

	cout << endl;
	cout.flush();

	cb_notify_call(line, from_party);
}

void t_userintf::cb_call_cancelled(int line) {
	if (line >= NUM_USER_LINES) return;
	
	cout << endl;
	cout << "Line " << line + 1 << ": ";
	cout << "far end cancelled call.\n";
	cout << endl;
	cout.flush();

	cb_stop_call_notification(line);
}

void t_userintf::cb_far_end_hung_up(int line) {
	if (line >= NUM_USER_LINES) return;
	
	cout << endl;
	cout << "Line " << line + 1 << ": ";
	cout << "far end ended call.\n";
	cout << endl;
	cout.flush();

	cb_stop_call_notification(line);
}

void t_userintf::cb_answer_timeout(int line) {
	if (line >= NUM_USER_LINES) return;
	
	cout << endl;
	cout << "Line " << line + 1 << ": ";
	cout << "answer timeout.\n";
	cout << endl;
	cout.flush();

	cb_stop_call_notification(line);
}

void t_userintf::cb_sdp_answer_not_supported(int line, const string &reason) {
	if (line >= NUM_USER_LINES) return;
	
	cout << endl;
	cout << "Line " << line + 1 << ": ";
	cout << "SDP answer from far end not supported.\n";
	cout << reason << endl;
	cout << endl;
	cout.flush();

	cb_stop_call_notification(line);
}

void t_userintf::cb_sdp_answer_missing(int line) {
	if (line >= NUM_USER_LINES) return;
	
	cout << endl;
	cout << "Line " << line + 1 << ": ";
	cout << "SDP answer from far end missing.\n";
	cout << endl;
	cout.flush();

	cb_stop_call_notification(line);
}

void t_userintf::cb_unsupported_content_type(int line, const t_sip_message *r) {
	if (line >= NUM_USER_LINES) return;
	
	cout << endl;
	cout << "Line " << line + 1 << ": ";
	cout << "Unsupported content type in answer from far end.\n";
	cout << r->hdr_content_type.media.type << "/";
	cout << r->hdr_content_type.media.subtype << endl;
	cout << endl;
	cout.flush();

	cb_stop_call_notification(line);
}

void t_userintf::cb_ack_timeout(int line) {
	if (line >= NUM_USER_LINES) return;
	
	cout << endl;
	cout << "Line " << line + 1 << ": ";
	cout << "no ACK received, call will be terminated.\n";
	cout << endl;
	cout.flush();

	cb_stop_call_notification(line);
}

void t_userintf::cb_100rel_timeout(int line) {
	if (line >= NUM_USER_LINES) return;
	
	cout << endl;
	cout << "Line " << line + 1 << ": ";
	cout << "no PRACK received, call will be terminated.\n";
	cout << endl;
	cout.flush();

	cb_stop_call_notification(line);
}

void t_userintf::cb_prack_failed(int line, const t_response *r) {
	if (line >= NUM_USER_LINES) return;
	
	cout << endl;
	cout << "Line " << line + 1 << ": PRACK failed.\n";
	cout << r->code << ' ' << r->reason << endl;
	cout << endl;
	cout.flush();

	cb_stop_call_notification(line);
}

void t_userintf::cb_provisional_resp_invite(int line, const t_response *r) {
	if (line >= NUM_USER_LINES) return;
	
	cout << endl;
	cout << "Line " << line + 1 << ": received ";
	cout << r->code << ' ' << r->reason << endl;
	cout << endl;
	cout.flush();
}

void t_userintf::cb_cancel_failed(int line, const t_response *r) {
	if (line >= NUM_USER_LINES) return;
	
	cout << endl;
	cout << "Line " << line + 1 << ": cancel failed.\n";
	cout << r->code << ' ' << r->reason << endl;
	cout << endl;
	cout.flush();
}

void t_userintf::cb_call_answered(t_user *user_config, int line, const t_response *r) {
	if (line >= NUM_USER_LINES) return;
	
	cout << endl;
	cout << "Line " << line + 1 << ": far end answered call.\n";
	cout << r->code << ' ' << r->reason << endl;

	cout << "To: ";
	cout << format_sip_address(user_config, r->hdr_to.display, r->hdr_to.uri) << endl;

	if (r->hdr_organization.is_populated()) {
		cout << "Organization: " << r->hdr_organization.name << endl;
	}

	cout << endl;
	cout.flush();
}

void t_userintf::cb_call_failed(t_user *user_config, int line, const t_response *r) {
	if (line >= NUM_USER_LINES) return;
	
	cout << endl;
	cout << "Line " << line + 1 << ": call failed.\n";
	cout << r->code << ' ' << r->reason << endl;

	// Warnings
	if (r->hdr_warning.is_populated()) {
		list<string> l = format_warnings(r->hdr_warning);
		for (list<string>::iterator i = l.begin(); i != l.end(); i++) {
			cout << *i << endl;
		}
	}

	// Redirection response
	if (r->get_class() == R_3XX && r->hdr_contact.is_populated()) {
		list<t_contact_param> l = r->hdr_contact.contact_list;
		l.sort();
		cout << "You can try the following contacts:\n";
		for (list<t_contact_param>::iterator i = l.begin();
		     i != l.end(); i++)
		{
			cout << format_sip_address(user_config, i->display, i->uri) << endl;
		}
	}

	// Unsupported extensions
	if (r->code == R_420_BAD_EXTENSION) {
		cout << r->hdr_unsupported.encode();
	}

	cout << endl;
	cout.flush();
}

void t_userintf::cb_stun_failed_call_ended(int line) {
	if (line >= NUM_USER_LINES) return;
	
	cout << endl;
	cout << "Line " << line + 1 << ": call failed.\n";
	cout << endl;
	cout.flush();		
}

void t_userintf::cb_call_ended(int line) {
	if (line >= NUM_USER_LINES) return;
	
	cout << endl;
	cout << "Line " << line + 1 << ": call ended.\n";
	cout.flush();
}

void t_userintf::cb_call_established(int line) {
	if (line >= NUM_USER_LINES) return;
	
	cout << endl;
	cout << "Line " << line + 1 << ": call established.\n";
	cout << endl;
	cout.flush();
}

void t_userintf::cb_options_response(const t_response *r) {
	cout << endl;
	cout << "OPTIONS response received: ";
	cout << r->code << ' ' << r->reason << endl;

	cout << "Capabilities of " << r->hdr_to.uri.encode() << endl;

	cout << "Accepted body types\n";
	if (r->hdr_accept.is_populated()) {
		cout << "\t" << r->hdr_accept.encode();
	} else {
		cout << "\tUnknown\n";
	}

	cout << "Accepted encodings\n";
	if (r->hdr_accept_encoding.is_populated()) {
		cout << "\t" << r->hdr_accept_encoding.encode();
	} else {
		cout << "\tUnknown\n";
	}

	cout << "Accepted languages\n";
	if (r->hdr_accept_language.is_populated()) {
		cout << "\t" << r->hdr_accept_language.encode();
	} else {
		cout << "\tUnknown\n";
	}

	cout << "Allowed requests\n";
	if (r->hdr_allow.is_populated()) {
		cout << "\t" << r->hdr_allow.encode();
	} else {
		cout << "\tUnknown\n";
	}

	cout << "Supported extensions\n";
	if (r->hdr_supported.is_populated()) {
		if (r->hdr_supported.features.empty()) {
			cout << "\tNone\n";
		} else {
			cout << "\t" << r->hdr_supported.encode();
		}
	} else {
		cout << "\tUnknown\n";
	}

	cout << "End point type\n";
	bool endpoint_known = false;
	if (r->hdr_server.is_populated()) {
		cout << "\t" << r->hdr_server.encode();
		endpoint_known = true;
	}

	if (r->hdr_user_agent.is_populated()) {
		// Some end-point put a User-Agent header in the response
		// instead of a Server header.
		cout << "\t" << r->hdr_user_agent.encode();
		endpoint_known = true;
	}

	if (!endpoint_known) {
		cout << "\tUnknown\n";
	}

	cout << endl;
	cout.flush();
}

void t_userintf::cb_reinvite_success(int line, const t_response *r) {
	if (line >= NUM_USER_LINES) return;
	
	cout << endl;
	cout << "Line " << line + 1 << ": re-INVITE successful.\n";
	cout << r->code << ' ' << r->reason << endl;
	cout << endl;
	cout.flush();
}

void t_userintf::cb_reinvite_failed(int line, const t_response *r) {
	if (line >= NUM_USER_LINES) return;
	
	cout << endl;
	cout << "Line " << line + 1 << ": re-INVITE failed.\n";
	cout << r->code << ' ' << r->reason << endl;
	cout << endl;
	cout.flush();
}

void t_userintf::cb_retrieve_failed(int line, const t_response *r) {
	if (line >= NUM_USER_LINES) return;
	
	// The status code from the response has already been reported
	// by cb_reinvite_failed.

	cout << endl;
	cout << "Line " << line + 1 << ": retrieve failed.\n";
	cout << endl;
	cout.flush();
}

void t_userintf::cb_invalid_reg_resp(t_user *user_config, 
		const t_response *r, const string &reason) 
{
	cout << endl;
	cout << user_config->get_profile_name();
	cout << ", registration failed: " << r->code << ' ' << r->reason << endl;
	cout << reason << endl;
	cout << endl;
	cout.flush();
}

void t_userintf::cb_register_success(t_user *user_config, 
	const t_response *r, unsigned long expires, bool first_success)
{
	// Only report success if this is the first success in a sequence
	if (!first_success) return;

	cout << endl;
	cout << user_config->get_profile_name();
	cout << ": registration succeeded (expires = " << expires << " seconds)\n";

	// Date at registrar
	if (r->hdr_date.is_populated()) {
		cout << "Registrar ";
		cout << r->hdr_date.encode() << endl;
	}

	cout << endl;
	cout.flush();
}

void t_userintf::cb_register_failed(t_user *user_config, 
		const t_response *r, bool first_failure) 
{
	// Only report the first failure in a sequence of failures
	if (!first_failure) return;

	cout << endl;
	cout << user_config->get_profile_name();
	cout << ", registration failed: " << r->code << ' ' << r->reason << endl;
	cout << endl;
	cout.flush();
}

void t_userintf::cb_register_stun_failed(t_user *user_config, bool first_failure) {
	// Only report the first failure in a sequence of failures
	if (!first_failure) return;

	cout << endl;
	cout << user_config->get_profile_name();
	cout << ", registration failed: STUN failure";
	cout << endl;
	cout.flush();
}

void t_userintf::cb_deregister_success(t_user *user_config, const t_response *r) {
	cout << endl;
	cout << user_config->get_profile_name();
	cout << ", de-registration succeeded: " << r->code << ' ' << r->reason << endl;
	cout << endl;
	cout.flush();
}

void t_userintf::cb_deregister_failed(t_user *user_config,  const t_response *r) {
	cout << endl;
	cout << user_config->get_profile_name();
	cout << ", de-registration failed: " << r->code << ' ' << r->reason << endl;
	cout << endl;
	cout.flush();
}
void t_userintf::cb_fetch_reg_failed(t_user *user_config, const t_response *r) {
	cout << endl;
	cout << user_config->get_profile_name();
	cout << ", fetch registrations failed: " << r->code << ' ' << r->reason << endl;
	cout << endl;
	cout.flush();
}

void t_userintf::cb_fetch_reg_result(t_user *user_config, const t_response *r) {
	cout << endl;

	cout << user_config->get_profile_name();
	const list<t_contact_param> &l = r->hdr_contact.contact_list;
	if (l.size() == 0) {
		cout << ": you are not registered\n";
	} else {
		cout << ": you have the following registrations\n";
		for (list<t_contact_param>::const_iterator i = l.begin();
		     i != l.end(); i++)
		{
			cout << i->encode() << endl;
		}
	}

	cout << endl;
	cout.flush();
}

void t_userintf::cb_register_inprog(t_user *user_config, t_register_type register_type) {
	switch (register_type) {
	case REG_REGISTER:
		// Do not report a register refreshment
		if (phone->get_is_registered(user_config)) return;

		// Do not report an automatic register re-attempt
		if (phone->get_last_reg_failed(user_config)) return;

		cout << endl;
		cout << user_config->get_profile_name();
		cout << ": registering phone...\n";
		break;
	case REG_DEREGISTER:
		cout << endl;
		cout << user_config->get_profile_name();
		cout << ": deregistering phone...\n";
		break;
	case REG_DEREGISTER_ALL:
		cout << endl;
		cout << user_config->get_profile_name();
		cout << ": deregistering all phones...";
		break;
	case REG_QUERY:
		cout << endl;
		cout << user_config->get_profile_name();
		cout << ": fetching registrations...";
		break;
	default:
		assert(false);
	}

	cout << endl;
	cout.flush();
}

void t_userintf::cb_redirecting_request(t_user *user_config, 
		int line, const t_contact_param &contact) 
{
	if (line >= NUM_USER_LINES) return;
	
	cout << endl;
	cout << "Line " << line + 1 << ": redirecting request to:\n";

	cout << format_sip_address(user_config, contact.display, contact.uri) << endl;

	cout << endl;
	cout.flush();
}

void t_userintf::cb_redirecting_request(t_user *user_config, const t_contact_param &contact) {
	cout << endl;
	cout << "Redirecting request to: ";

	cout << format_sip_address(user_config, contact.display, contact.uri) << endl;

	cout << endl;
	cout.flush();
}

void t_userintf::cb_play_ringtone(int line) {
	if (!sys_config->get_play_ringtone()) return;

	if (tone_gen) {
		tone_gen->stop();
		MEMMAN_DELETE(tone_gen);
		delete tone_gen;
	}
	
	// Determine ring tone
	string ringtone_file = phone->get_ringtone(line);

	tone_gen = new t_tone_gen(ringtone_file, sys_config->get_dev_ringtone());
	MEMMAN_NEW(tone_gen);
	
	// If ring tone does not exist, then fall back to system default.
	if (!tone_gen->is_valid() && ringtone_file != FILE_RINGTONE) {
		MEMMAN_DELETE(tone_gen);
		delete tone_gen;
		tone_gen = new t_tone_gen(FILE_RINGTONE, sys_config->get_dev_ringtone());
		MEMMAN_NEW(tone_gen);
	}
	
	// Play ring tone
	tone_gen->start_play_thread(true, INTERVAL_RINGTONE);
}

void t_userintf::cb_play_ringback(t_user *user_config) {
	if (!sys_config->get_play_ringback()) return;
	
	if (tone_gen) {
		tone_gen->stop();
		MEMMAN_DELETE(tone_gen);
		delete tone_gen;
	}
	
	// Determine ring back tone
	string ringback_file;
	if (!user_config->get_ringback_file().empty()) {
		ringback_file = user_config->get_ringback_file();
	} else if (!sys_config->get_ringback_file().empty()) {
		ringback_file = sys_config->get_ringback_file();
	} else {
		// System default
		ringback_file = FILE_RINGBACK;
	}

	tone_gen = new t_tone_gen(ringback_file, sys_config->get_dev_speaker());
	MEMMAN_NEW(tone_gen);
	
	// If ring back tone does not exist, then fall back to system default.
	if (!tone_gen->is_valid() && ringback_file != FILE_RINGBACK) {
		MEMMAN_DELETE(tone_gen);
		delete tone_gen;
		tone_gen = new t_tone_gen(FILE_RINGBACK, sys_config->get_dev_speaker());
		MEMMAN_NEW(tone_gen);
	}
	
	// Play ring back tone
	tone_gen->start_play_thread(true, INTERVAL_RINGBACK);
}

void t_userintf::cb_stop_tone(int line) {
	// Only stop the tone if the current line is the active line
	if (line != phone->get_active_line()) return;

	if (!tone_gen) return;
	tone_gen->stop();
	MEMMAN_DELETE(tone_gen);
	delete tone_gen;
	tone_gen = NULL;
}

void t_userintf::cb_notify_call(int line, string from_party) {
	// Play ringtone if the call is received on the active line
	if (line == phone->get_active_line() &&
	    !phone->is_line_auto_answered(line))
	{
		cb_play_ringtone(line);
	}
}

void t_userintf::cb_stop_call_notification(int line) {
	cb_stop_tone(line);
}

void t_userintf::cb_dtmf_detected(int line, char dtmf_event) {
	if (line >= NUM_USER_LINES) return;
	
	cout << endl;
	cout << "Line " << line + 1 << ": DTMF detected: ";

	if (VALID_DTMF_EV(dtmf_event)) {
		cout << dtmf_ev2char(dtmf_event) << endl;
	} else {
		cout << "invalid DTMF telephone event (" << (int)dtmf_event << endl;
	}

	cout << endl;
	cout.flush();
}

void t_userintf::cb_async_dtmf_detected(int line, char dtmf_event) {
	if (line >= NUM_USER_LINES) return;
	
	t_event_ui *event = new t_event_ui(TYPE_UI_CB_DTMF_DETECTED);
	MEMMAN_NEW(event);
	
	event->set_line(line);
	event->set_dtmf_event(dtmf_event);	
	evq_ui_events.push(event);
}

void t_userintf::cb_send_dtmf(int line, char dtmf_event) {
	// No feed back in CLI
}

void t_userintf::cb_async_send_dtmf(int line, char dtmf_event) {
	t_event_ui *event = new t_event_ui(TYPE_UI_CB_SEND_DTMF);
	MEMMAN_NEW(event);
	
	event->set_line(line);
	event->set_dtmf_event(dtmf_event);	
	evq_ui_events.push(event);
}

void t_userintf::cb_dtmf_not_supported(int line) {
	if (line >= NUM_USER_LINES) return;
	
	if (throttle_dtmf_not_supported) return;

	cout << endl;
	cout << "Line " << line + 1 << ": far end does not support DTMF events.\n";
	cout << endl;
	cout.flush();

	// Throttle subsequent call backs
	throttle_dtmf_not_supported = true;
}

void t_userintf::cb_dtmf_supported(int line) {
	if (line >= NUM_USER_LINES) return;
	
	cout << endl;
	cout << "Line " << line + 1 << ": far end supports DTMF telephone event.\n";
	cout << endl;
	cout.flush();
}

void t_userintf::cb_line_state_changed(void) {
	// Nothing to do for CLI
}

void t_userintf::cb_async_line_state_changed(void) {
	t_event_ui *event = new t_event_ui(TYPE_UI_CB_LINE_STATE_CHANGED);
	MEMMAN_NEW(event);
	
	evq_ui_events.push(event);
}

void t_userintf::cb_send_codec_changed(int line, t_audio_codec codec) {
	// No feedback in CLI
}

void t_userintf::cb_recv_codec_changed(int line, t_audio_codec codec) {
	// No feedback in CLI
}

void t_userintf::cb_async_recv_codec_changed(int line, t_audio_codec codec) {
	t_event_ui *event = new t_event_ui(TYPE_UI_CB_RECV_CODEC_CHANGED);
	MEMMAN_NEW(event);
	
	event->set_line(line);
	event->set_codec(codec);	
	evq_ui_events.push(event);
}

void t_userintf::cb_notify_recvd(int line, const t_request *r) {
	if (line >= NUM_USER_LINES) return;
	
	cout << endl;
	cout << "Line " << line + 1 <<  ": received notification.\n";
	cout << "Event:    " << r->hdr_event.event_type << endl;
	cout << "State:    " << r->hdr_subscription_state.substate << endl;

	if (r->hdr_subscription_state.substate == SUBSTATE_TERMINATED) {
		cout << "Reason:   " << r->hdr_subscription_state.reason << endl;
	}

	t_response *sipfrag = (t_response *)((t_sip_body_sipfrag *)r->body)->sipfrag;
	cout << "Progress: " << sipfrag->code << ' ' << sipfrag->reason << endl;

	cout << endl;
	cout.flush();
}

void t_userintf::cb_refer_failed(int line, const t_response *r) {
	if (line >= NUM_USER_LINES) return;
	
	cout << endl;
	cout << "Line " << line + 1 << ": refer request failed.\n";
	cout << r->code << ' ' << r->reason << endl;
	cout << endl;
	cout.flush();
}

void t_userintf::cb_refer_result_success(int line) {
	if (line >= NUM_USER_LINES) return;
	
	cout << endl;
	cout << "Line " << line + 1 << ": call succesfully referred.\n";
	cout << endl;
	cout.flush();
}

void t_userintf::cb_refer_result_failed(int line) {
	if (line >= NUM_USER_LINES) return;
	
	cout << endl;
	cout << "Line " << line + 1 << ": call refer failed.\n";
	cout << endl;
	cout.flush();
}

void t_userintf::cb_refer_result_inprog(int line) {
	if (line >= NUM_USER_LINES) return;
	
	cout << endl;
	cout << "Line " << line + 1 << ": call refer in progress.\n";
	cout << "No further notifications will be received.\n";
	cout << endl;
	cout.flush();
}

void t_userintf::cb_call_referred(t_user *user_config, int line, t_request *r) {
	if (line >= NUM_USER_LINES) return;
	
	cout << endl;
	cout << "Line " << line + 1 << ": transferring call to ";
	cout << format_sip_address(user_config, r->hdr_refer_to.display,
		r->hdr_refer_to.uri);
	cout << endl;

	if (r->hdr_referred_by.is_populated()) {
		cout << "Tranfer requested by ";
		cout << format_sip_address(user_config, r->hdr_referred_by.display,
			r->hdr_referred_by.uri);
		cout << endl;
	}

	cout << endl;
	cout.flush();
}

void t_userintf::cb_retrieve_referrer(t_user *user_config, int line) {
	if (line >= NUM_USER_LINES) return;
	
	const t_call_info call_info = phone->get_call_info(line);

	cout << endl;
	cout << "Line " << line + 1 << ": call transfer failed.\n";
	cout << "Retrieving call: \n";
	cout << "From:    ";
	cout << format_sip_address(user_config, call_info.from_display, call_info.from_uri);
	cout << endl;
	if (!call_info.from_organization.empty()) {
		cout << "         " << call_info.from_organization;
		cout << endl;
	}
	cout << "To:      ";
	cout << format_sip_address(user_config, call_info.to_display, call_info.to_uri);
	cout << endl;
	if (!call_info.to_organization.empty()) {
		cout << "         " << call_info.to_organization;
		cout << endl;
	}
	cout << "Subject: ";
	cout << call_info.subject;
	cout << endl << endl;
	cout.flush();
}

void t_userintf::cb_consultation_call_setup(t_user *user_config, int line) {
	if (line >= NUM_USER_LINES) return;
	
	const t_call_info call_info = phone->get_call_info(line);

	cout << endl;
	cout << "Line " << line + 1 << ": setup consultation call.\n";
	cout << "From:    ";
	cout << format_sip_address(user_config, call_info.from_display, call_info.from_uri);
	cout << endl;
	if (!call_info.from_organization.empty()) {
		cout << "         " << call_info.from_organization;
		cout << endl;
	}
	cout << "To:      ";
	cout << format_sip_address(user_config, call_info.to_display, call_info.to_uri);
	cout << endl;
	if (!call_info.to_organization.empty()) {
		cout << "         " << call_info.to_organization;
		cout << endl;
	}
	cout << "Subject: ";
	cout << call_info.subject;
	cout << endl << endl;
	cout.flush();
}

void t_userintf::cb_stun_failed(t_user *user_config, int err_code, const string &err_reason) {
	cout << endl;
	cout << user_config->get_profile_name();
	cout << ", STUN request failed: ";
	cout << err_code << " " << err_reason << endl;
	cout << endl;
	cout.flush();
}

void t_userintf::cb_stun_failed(t_user *user_config) {
	cout << endl;
	cout << user_config->get_profile_name();
	cout << ", STUN request failed.\n";
	cout << endl;
	cout.flush();
}


bool t_userintf::cb_ask_user_to_redirect_invite(t_user *user_config, 
		const t_url &destination, const string &display)
{
	// Cannot ask user for permission in CLI, so deny redirection.
	return false;
}

bool t_userintf::cb_ask_user_to_redirect_request(t_user *user_config, 
		const t_url &destination, const string &display, t_method method)
{
	// Cannot ask user for permission in CLI, so deny redirection.
	return false;
}

bool t_userintf::cb_ask_credentials(t_user *user_config, 
		const string &realm, string &username, string &password)
{
	// Cannot ask user for username/password in CLI
	return false;
}

void t_userintf::cb_ask_user_to_refer(t_user *user_config, 
			const t_url &refer_to_uri,
			const string &refer_to_display,
			const t_url &referred_by_uri,
			const string &referred_by_display)
{
	// Cannot ask user for permission in CLI, so deny REFER
	send_refer_permission(false);
}

void t_userintf::send_refer_permission(bool permission) {
	evq_trans_layer->push_refer_permission_response(permission);
}

void t_userintf::cb_show_msg(const string &msg, t_msg_priority prio) {
	cout << endl;

	switch (prio) {
	case MSG_NO_PRIO:
		break;
	case MSG_INFO:
		cout << "Info: ";
		break;
	case MSG_WARNING:
		cout << "Warning: ";
		break;
	case MSG_CRITICAL:
		cout << "Critical: ";
		break;
	default:
		cout << "???: ";
	}

	cout << msg << endl;

	cout << endl;
	cout.flush();
}

bool t_userintf::cb_ask_msg(const string &msg, t_msg_priority prio) {
	// Cannot ask questions in CLI mode.
	// Print message and return false
	cb_show_msg(msg, prio);
	return false;
}

void t_userintf::cb_display_msg(const string &msg, t_msg_priority prio) {
	// In CLI mode this is the same as cb_show_msg
	cb_show_msg(msg, prio);
}

void t_userintf::cb_async_display_msg(const string &msg, t_msg_priority prio) {
	t_event_ui *event = new t_event_ui(TYPE_UI_CB_DISPLAY_MSG);
	MEMMAN_NEW(event);
	
	event->set_display_msg(msg, prio);
	evq_ui_events.push(event);
}

void t_userintf::cb_log_updated(bool log_zapped) {
	// In CLI mode there is no log viewer.
}

void t_userintf::cb_call_history_updated(void) {
	// In CLI mode there is no call history viewer.
}

void t_userintf::cb_missed_call(int num_missed_calls) {
	// In CLI mode there is no missed call indication.
}

void t_userintf::cb_nat_discovery_progress_start(int num_steps) {
	cout << endl;
	cout << "Firewall/NAT discovery in progress.\n";
	cout << "Please wait.\n";
	cout << endl;
}

void t_userintf::cb_nat_discovery_finished(void) {
	// Nothing to do in CLI mode.
}

void t_userintf::cb_nat_discovery_progress_step(int step) {
	// Nothing to do in CLI mode.
}

bool t_userintf::cb_nat_discovery_cancelled(void) {
	// User cannot cancel NAT discovery in CLI mode.
	return false;
}

void t_userintf::cb_line_encrypted(int line, bool encrypted, const string &cipher_mode) {
	if (line >= NUM_USER_LINES) return;
	
	cout << endl;
	if (encrypted) {
		cout << "Line " << line + 1 << ": audio encryption enabled (";
		cout << cipher_mode << ").\n";
	} else {
		cout << "Line " << line + 1 << ": audio encryption disabled.\n";
	}
	cout << endl;
	cout.flush();
}

void t_userintf::cb_async_line_encrypted(int line, bool encrypted, const string &cipher_mode) {
	t_event_ui *event = new t_event_ui(TYPE_UI_CB_LINE_ENCRYPTED);
	MEMMAN_NEW(event);
	
	event->set_line(line);
	event->set_encrypted(encrypted);
	event->set_cipher_mode(cipher_mode);	
	evq_ui_events.push(event);
}

void t_userintf::cb_show_zrtp_sas(int line, const string &sas) {
	if (line >= NUM_USER_LINES) return;
	
	cout << endl;
	cout << "Line " << line + 1 << ": ZRTP SAS = " << sas << endl;
	cout << "Confirm the SAS if it is correct.\n";
	cout << endl;
	cout.flush();
}

void t_userintf::cb_async_show_zrtp_sas(int line, const string &sas) {
	t_event_ui *event = new t_event_ui(TYPE_UI_CB_SHOW_ZRTP_SAS);
	MEMMAN_NEW(event);
	
	event->set_line(line);
	event->set_zrtp_sas(sas);
	evq_ui_events.push(event);
}

void t_userintf::cb_zrtp_confirm_go_clear(int line) {
	if (line >= NUM_USER_LINES) return;
	
	cout << endl;
	cout << "Line " << line + 1 << ": remote user disabled encryption.\n";
	cout << endl;
	cout.flush();
	
	phone->pub_zrtp_go_clear_ok(line);
}

void t_userintf::cb_async_zrtp_confirm_go_clear(int line) {
	t_event_ui *event = new t_event_ui(TYPE_UI_CB_ZRTP_CONFIRM_GO_CLEAR);
	MEMMAN_NEW(event);
	
	event->set_line(line);
	evq_ui_events.push(event);
}

void t_userintf::cb_zrtp_sas_confirmed(int line) {
	if (line >= NUM_USER_LINES) return;
	
	cout << endl;
	cout << "Line " << line + 1 << ": SAS confirmed.\n";
	cout << endl;
	cout.flush();
}

void t_userintf::cb_zrtp_sas_confirmation_reset(int line) {
	if (line >= NUM_USER_LINES) return;
	
	cout << endl;
	cout << "Line " << line + 1 << ": SAS confirmation reset.\n";
	cout << endl;
	cout.flush();
}

void t_userintf::cb_update_mwi(void) {
	// Nothing to do in CLI mode.
}

void t_userintf::cb_mwi_subscribe_failed(t_user *user_config, t_response *r, bool first_failure) {
	// Only report the first failure in a sequence of failures
	if (!first_failure) return;

	cout << endl;
	cout << user_config->get_profile_name();
	cout << ", MWI subscription failed: " << r->code << ' ' << r->reason << endl;
	cout << endl;
	cout.flush();
}

void t_userintf::cb_mwi_terminated(t_user *user_config, const string &reason) {
	cout << endl;
	cout << user_config->get_profile_name();
	cout << ", MWI subscription terminated: " << reason << endl;
	cout << endl;
	cout.flush();
}

bool t_userintf::cb_message_request(t_user *user_config, t_request *r) {
	cout << endl;
	cout << "Received message\n";
	cout << "From:\t\t";
	
	string from_party = format_sip_address(user_config, 
		r->hdr_from.get_display_presentation(), r->hdr_from.uri);
	cout << from_party << endl;

	if (r->hdr_organization.is_populated()) {
		cout << "Organization:\t" << r->hdr_organization.name << endl;
	}

	cout << "To:\t\t";
	cout << format_sip_address(user_config, r->hdr_to.display, r->hdr_to.uri) << endl;

	if (r->hdr_subject.is_populated()) {
		cout << "Subject:\t" << r->hdr_subject.subject << endl;
	}
	
	cout << endl;
	if (r->body && r->body->get_type() == BODY_PLAIN_TEXT)
	{
		t_sip_body_plain_text *sb = dynamic_cast<t_sip_body_plain_text *>(r->body);
		cout << sb->text << endl;
	} else if (r->body && r->body->get_type() == BODY_HTML_TEXT) {
		t_sip_body_html_text *sb = dynamic_cast<t_sip_body_html_text *>(r->body);
		cout << sb->text << endl;
	} else {
		cout << "Unsupported content type.\n";
	}

	cout << endl;
	cout.flush();
	
	// There are no session in CLI mode, so all messages are accepted.
	return true;
}

void t_userintf::cb_message_response(t_user *user_config, t_response *r, t_request *req) {
	if (r->is_success()) return;
	
	cout << endl;
	cout << "Failed to send MESSAGE.\n";
	cout << r->code << " " << r->reason << endl;
	
	cout << endl;
	cout.flush();	
}

void t_userintf::cb_im_iscomposing_request(t_user *user_config, t_request *r,
			im::t_composing_state state, time_t refresh)
{
	// Nothing to do in CLI mode
	return;
}

void t_userintf::cb_im_iscomposing_not_supported(t_user *user_config, t_response *r) {
	// Nothing to do in CLI mode
	return;
}

bool t_userintf::get_last_call_info(t_url &url, string &display,
			string &subject, t_user **user_config, bool &hide_user) const
{
	if (!last_called_url.is_valid()) return false;
	
	url = last_called_url;
	display = last_called_display;
	subject = last_called_subject;
	*user_config = phone->ref_user_profile(last_called_profile);
	hide_user = last_called_hide_user;
	
	return *user_config != NULL;
}

bool t_userintf::can_redial(void) const {
	return last_called_url.is_valid() && 
	       phone->ref_user_profile(last_called_profile) != NULL;
}

void t_userintf::cmd_call(const string &destination, bool immediate) {
	string s = "invite ";
	s += destination;
	exec_command(s);
}

void t_userintf::cmd_quit(void) {
	exec_command("quit");
}

void t_userintf::cmd_quit_async(void) {
	t_event_ui *event = new t_event_ui(TYPE_UI_CB_QUIT);
	MEMMAN_NEW(event);
	evq_ui_events.push(event);
}

void t_userintf::cmd_cli(const string &command, bool immediate) {
	exec_command(command, immediate);
}

void t_userintf::cmd_show(void) {
	// Do nothing in CLI mode.
}

void t_userintf::cmd_hide(void) {
	// Do nothing in CLI mode.
}

string t_userintf::get_name_from_abook(t_user *user_config, const t_url &u) {
	return ab_local->find_name(user_config, u);
}

void *process_events_main(void *arg) {
	ui->process_events();
	return NULL;
}

const list<string>& t_userintf::get_all_commands(void)
{
	return all_commands;
}
