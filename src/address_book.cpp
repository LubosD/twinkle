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

#include "address_book.h"

#include "sys_settings.h"
#include "translator.h"
#include "userintf.h"
#include "util.h"

// Call history file
#define ADDRESS_BOOK_FILE	"twinkle.ab";

// Field seperator in call history file
#define REC_SEPARATOR		'|'

////////////////////////////
// class t_address_card
////////////////////////////

string t_address_card::get_display_name(void) const {
	string s;
	
	if (!name_first.empty()) {
		s = name_first;
	}
	
	if (!name_infix.empty()) {
		if (!s.empty()) s += ' ';
		s += name_infix;
	}
	
	if (!name_last.empty()) {
		if (!s.empty()) s += ' ';
		s += name_last;
	}
	
	return s;
}
	
bool t_address_card::create_file_record(vector<string> &v) const {
	v.clear();
	
	v.push_back(name_first);
	v.push_back(name_infix);
	v.push_back(name_last);
	v.push_back(sip_address);
	v.push_back(remark);
	
	return true;
}
	
bool t_address_card::populate_from_file_record(const vector<string> &v) {
	// Check number of fields
	if (v.size() != 5) return false;
	
	name_first = v[0];
	name_infix = v[1];
	name_last = v[2];
	sip_address = v[3];
	remark = v[4];
	
	return true;
}
	
bool t_address_card::operator==(const t_address_card other) const {
	return (name_first == other.name_first &&
		name_infix == other.name_infix &&
		name_last == other.name_last &&
		sip_address == other.sip_address &&
		remark == other.remark);
}


////////////////////////////
// class t_address_book
////////////////////////////

// Private

void t_address_book::find_address(t_user *user_config, const t_url &u) const {
	if (u == last_url) return;
	
	last_url = u;
	last_name.clear();
	
	// Normalize url using number conversion rules
	t_url u_normalized(u);
	u_normalized.apply_conversion_rules(user_config);
	
	for (list<t_address_card>::const_iterator i = records.begin();
	     i != records.end(); i++)
	{
		string full_address = ui->expand_destination(user_config, i->sip_address,
					u_normalized.get_scheme());
		t_url url_phone(full_address);
		if (!url_phone.is_valid()) continue;
		
		if (u_normalized.user_host_match(url_phone,
			user_config->get_remove_special_phone_symbols(),
			user_config->get_special_phone_symbols()))
		{
			last_name = i->get_display_name();
			return;
		}
	}
}


// Public

t_address_book::t_address_book() : utils::t_record_file<t_address_card>()
{
	set_header("first_name|infix_name|last_name|sip_address|remark");
	set_separator(REC_SEPARATOR);
	
	string s(DIR_HOME);
	s += "/";
	s += USER_DIR;
	s += "/";
	s += ADDRESS_BOOK_FILE;
	set_filename(s);
}

void t_address_book::add_address(const t_address_card &address) {
	mtx_records.lock();
	records.push_back(address);
	mtx_records.unlock();
}

bool t_address_book::del_address(const t_address_card &address) {
	mtx_records.lock();
	
	list<t_address_card>::iterator it = find(records.begin(), records.end(),
			address);
			
	if (it == records.end()) {
		mtx_records.unlock();
		return false;
	}	
	
	records.erase(it);
	
	// Invalidate the cache for the address finder
	last_url.set_url("");
	
	mtx_records.unlock();
	return true;
}

bool t_address_book::update_address(const t_address_card &old_address,
	const t_address_card &new_address)
{
	mtx_records.lock();
	
	if (!del_address(old_address)) {
		mtx_records.unlock();
		return false;
	}
	
	records.push_back(new_address);
	
	mtx_records.unlock();
	return true;
}

string t_address_book::find_name(t_user *user_config, const t_url &u) const {
	mtx_records.lock();
	find_address(user_config, u);
	string name = last_name;
	mtx_records.unlock();
	
	return name;
}

const list<t_address_card> &t_address_book::get_address_list(void) const {
	return records;
}
