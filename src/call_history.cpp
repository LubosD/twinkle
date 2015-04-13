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

#include <cassert>
#include <cstdlib>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <vector>
#include "call_history.h"
#include "log.h"
#include "sys_settings.h"
#include "translator.h"
#include "userintf.h"
#include "util.h"

// Call history file
#define CALL_HISTORY_FILE	"twinkle.ch";

// Field seperator in call history file
#define REC_SEPARATOR		'|'

////////////////////////
// class t_call_record
////////////////////////

t_mutex t_call_record::mtx_class;
unsigned short t_call_record::next_id = 1;

t_call_record::t_call_record() {
	mtx_class.lock();
	id = next_id++;
	if (next_id == 65535) next_id = 1;
	mtx_class.unlock();

	time_start = 0;
	time_answer = 0;
	time_end = 0;
	invite_resp_code = 0;
}

void t_call_record::renew() {
	mtx_class.lock();
	id = next_id++;
	if (next_id == 65535) next_id = 1;
	mtx_class.unlock();

	time_start = 0;
	time_answer = 0;
	time_end = 0;
	direction = DIR_IN;
	from_display.clear();
	from_uri.set_url("");
	from_organization.clear();
	to_display.clear();
	to_uri.set_url("");
	to_organization.clear();
	reply_to_display.clear();
	reply_to_uri.set_url("");
	referred_by_display.clear();
	referred_by_uri.set_url("");
	subject.clear();
	rel_cause = CS_LOCAL_USER;
	invite_resp_code = 0;
	invite_resp_reason.clear();
	far_end_device.clear();
	user_profile.clear();
}

void t_call_record::start_call(const t_request *invite, t_direction dir, 
		const string &_user_profile) 
{
	assert(invite->method == INVITE);

	struct timeval t;
	
	gettimeofday(&t, NULL);
	time_start = t.tv_sec;
	
	from_display = invite->hdr_from.get_display_presentation();
	from_uri = invite->hdr_from.uri;
	
	if (invite->hdr_organization.is_populated()) {
		from_organization = invite->hdr_organization.name;
	}
	
	to_display = invite->hdr_to.display;
	to_uri = invite->hdr_to.uri;

	if (invite->hdr_reply_to.is_populated()) {
		reply_to_display = invite->hdr_reply_to.display;
		reply_to_uri = invite->hdr_reply_to.uri;
	}
	
	if (invite->hdr_referred_by.is_populated()) {
		referred_by_display = invite->hdr_referred_by.display;
		referred_by_uri = invite->hdr_referred_by.uri;
	}
		
	if (invite->hdr_subject.is_populated()) {
		subject = invite->hdr_subject.subject;
	}
	
	direction = dir;
	user_profile = _user_profile;
	
	if (direction == DIR_IN && invite->hdr_user_agent.is_populated()) {
		far_end_device = invite->hdr_user_agent.get_ua_info();
	}
}

void t_call_record::fail_call(const t_response *resp) {
	assert(resp->get_class() >= 3);
	assert(resp->hdr_cseq.method == INVITE);
	
	struct timeval t;
	
	gettimeofday(&t, NULL);
	time_end = t.tv_sec;
	rel_cause = CS_FAILURE;
	invite_resp_code = resp->code;
	invite_resp_reason = resp->reason;
	
	if (resp->hdr_organization.is_populated()) {
		to_organization = resp->hdr_organization.name;
	}
	
	if (direction == DIR_OUT && resp->hdr_server.is_populated()) {
		far_end_device = resp->hdr_server.get_server_info();
	}
}

void t_call_record::answer_call(const t_response *resp) {
	assert(resp->is_success());
	
	struct timeval t;
	
	gettimeofday(&t, NULL);
	time_answer = t.tv_sec;
	invite_resp_code = resp->code;
	invite_resp_reason = resp->reason;
	
	if (resp->hdr_organization.is_populated()) {
		to_organization = resp->hdr_organization.name;
	}
	
	if (direction == DIR_OUT && resp->hdr_server.is_populated()) {
		far_end_device = resp->hdr_server.get_server_info();
	}
}

void t_call_record::end_call(t_rel_cause cause) {
	struct timeval t;
	
	gettimeofday(&t, NULL);
	time_end = t.tv_sec;
	rel_cause = cause;
}

void t_call_record::end_call(bool far_end) {
	if (far_end) {
		end_call(CS_REMOTE_USER);
	} else {
		end_call(CS_LOCAL_USER);
	}
}

string t_call_record::get_rel_cause(void) const {
	switch (rel_cause) {
	case CS_LOCAL_USER:
		return TRANSLATE2("CoreCallHistory", "local user");
	case CS_REMOTE_USER:
		return TRANSLATE2("CoreCallHistory", "remote user");
	case CS_FAILURE:
		return TRANSLATE2("CoreCallHistory", "failure");
	}
	
	return TRANSLATE2("CoreCallHistory", "unknown");
}

string t_call_record::get_rel_cause_internal(void) const {
	switch (rel_cause) {
	case CS_LOCAL_USER:
		return "local user";
	case CS_REMOTE_USER:
		return "remote user";
	case CS_FAILURE:
		return "failure";
	}
	
	return "unknown";
}

string t_call_record::get_direction(void) const {
	switch (direction) {
	case DIR_IN:
		return TRANSLATE2("CoreCallHistory", "in");
	case DIR_OUT:
		return TRANSLATE2("CoreCallHistory", "out");
	}
	
	return TRANSLATE2("CoreCallHistory", "unknown");
}

string t_call_record::get_direction_internal(void) const {
	switch (direction) {
	case DIR_IN:
		return "in";
	case DIR_OUT:
		return "out";
	}
	
	return "unknown";
}

bool t_call_record::set_rel_cause(const string &cause) {
	// NOTE: caller and callee were used before version 0.7
	// They are still checked here for backward compatibility

	if (cause == "caller" || cause == "local user") {
		rel_cause = CS_LOCAL_USER;
	} else if (cause == "callee" || cause == "remote user") {
		rel_cause = CS_REMOTE_USER;
	} else if (cause == "failure") {
		rel_cause = CS_FAILURE;
	} else {
		return false;
	}
	
	return true;
}

bool t_call_record::set_direction(const string &dir) {
	if (dir == "in") {
		direction = DIR_IN;
	} else if (dir == "out") {
		direction = DIR_OUT;
	} else {
		return false;
	}
	
	return true;
}

bool t_call_record::create_file_record(vector<string> &v) const {
	v.clear();
	
	v.push_back(ulong2str(time_start));
	v.push_back(ulong2str(time_answer));
	v.push_back(ulong2str(time_end));
	v.push_back(get_direction_internal());
	v.push_back(from_display);
	v.push_back(from_uri.encode());
	v.push_back(from_organization);
	v.push_back(to_display);
	v.push_back(to_uri.encode());
	v.push_back(to_organization);
	v.push_back(reply_to_display);
	v.push_back(reply_to_uri.encode());
	v.push_back(referred_by_display);
	v.push_back(referred_by_uri.encode());
	v.push_back(subject);
	v.push_back(get_rel_cause_internal());
	v.push_back(int2str(invite_resp_code));
	v.push_back(invite_resp_reason);
	v.push_back(far_end_device);
	v.push_back(user_profile);
	
	return true;
}

bool t_call_record::populate_from_file_record(const vector<string> &v) {
	// Check number of fields
	if (v.size() != 20) return false;
	
	time_start = strtoul(v[0].c_str(), NULL, 10);
	time_answer = strtoul(v[1].c_str(), NULL, 10);
	time_end = strtoul(v[2].c_str(), NULL, 10);
	
	if (!set_direction(v[3])) return false;
	
	from_display = v[4];
	from_uri.set_url(v[5]);
	if (!from_uri.is_valid()) return false;
	from_organization = v[6];
	
	to_display = v[7];
	to_uri.set_url(v[8]);
	if (!to_uri.is_valid()) return false;
	to_organization = v[9];
	
	reply_to_display = v[10];
	reply_to_uri.set_url(v[11]);
	
	referred_by_display = v[12];
	referred_by_uri.set_url(v[13]);
	
	subject = v[14];
	
	if (!set_rel_cause(v[15])) return false;
	
	invite_resp_code = atoi(v[16].c_str());
	invite_resp_reason = v[17];
	far_end_device = v[18];
	user_profile = v[19];
	
	return true;
}

bool t_call_record::is_valid(void) const {
	if (time_start == 0 || time_end == 0) return false;
	if (time_answer > 0 && rel_cause == CS_FAILURE) return false;
	
	return true;
}

unsigned short t_call_record::get_id(void) const {
	return id;
}

////////////////////////
// class t_call_history
////////////////////////

t_call_history::t_call_history() : utils::t_record_file<t_call_record>() {
	set_header("time_start|time_answer|time_end|direction|from_display|from_uri|"
	      "from_organization|to_display|to_uri|to_organization|"
	      "reply_to_display|reply_to_uri|referred_by_display|referred_by_uri|"
	      "subject|rel_cause|invite_resp_code|invite_resp_reason|"
	      "far_end_device|user_profile");
	      
	set_separator(REC_SEPARATOR);
	
	string s(DIR_HOME);
	s += "/";
	s += USER_DIR;
	s += "/";
	s += CALL_HISTORY_FILE;
	set_filename(s);
	
	num_missed_calls = 0;
}

void t_call_history::add_call_record(const t_call_record &call_record, bool write) {
	if (!call_record.is_valid()) {
		log_file->write_report("Call history record is not valid.",
			"t_call_history::add_call_record", LOG_NORMAL, LOG_WARNING);
		return;
	}
	
	mtx_records.lock();

	records.push_back(call_record);
	
	while (records.size() > (size_t)sys_config->get_ch_max_size()) {
		records.pop_front();
	}
	
	// Increment missed calls counter
	if (call_record.rel_cause == t_call_record::CS_FAILURE && 
	    call_record.direction == t_call_record::DIR_IN)
	{
		++num_missed_calls;
		ui->cb_missed_call(num_missed_calls);
	}
	
	mtx_records.unlock();
	
	if (write) {
		string msg;
		if (!save(msg)) {
			log_file->write_report(msg, "t_call_history::add_call_record",
				LOG_NORMAL, LOG_WARNING);
		}
	}
	
	// Update call history in user interface.
	ui->cb_call_history_updated();
}

void t_call_history::delete_call_record(unsigned short id, bool write) {
	mtx_records.lock();
	for (list<t_call_record>::iterator i = records.begin();
	     i != records.end(); i++)
	{
		if (i->get_id() == id) {
			records.erase(i);
			break;
		}
	}
	mtx_records.unlock();
	
	if (write) {
		string msg;
		if (!save(msg)) {
			log_file->write_report(msg, "t_call_history::delete_call_record",
				LOG_NORMAL, LOG_WARNING);
		}
	}
	
	// Update call history in user interface.
	ui->cb_call_history_updated();
}

void t_call_history::get_history(list<t_call_record> &history) {
	mtx_records.lock();
	history = records;
	mtx_records.unlock();
}

void t_call_history::clear(bool write) {
	mtx_records.lock();
	records.clear();
	mtx_records.unlock();
	
	if (write) {
		string msg;
		if (!save(msg)) {
			log_file->write_report(msg, "t_call_history::clear",
				LOG_NORMAL, LOG_WARNING);
		}
	}
	
	// Update call history in user interface.
	ui->cb_call_history_updated();	
	
	clear_num_missed_calls();	
}

int t_call_history::get_num_missed_calls(void) const {
	return num_missed_calls;
}

void t_call_history::clear_num_missed_calls(void) {
	mtx_records.lock();
	num_missed_calls = 0;
	mtx_records.unlock();
	
	ui->cb_missed_call(0);
}
