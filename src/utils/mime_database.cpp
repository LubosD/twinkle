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

#include "mime_database.h"

#include <cassert>

#include "log.h"
#include "sys_settings.h"

using namespace utils;

//////////////////////////
// class t_mime_db_record
//////////////////////////

bool t_mime_db_record::create_file_record(vector<string> &v) const {
	// The mime database is read only. So this function should
	// never be called.
	assert(false);
	return false;
}

bool t_mime_db_record::populate_from_file_record(const vector<string> &v) {
	// Check number of fields
	if (v.size() != 2) return false;
	
	mimetype = v[0];
	file_glob = v[1];
	
	return true;
}

//////////////////////////
// class t_mime_database
//////////////////////////

t_mime_database::t_mime_database() {
	set_separator(':');
	set_filename(sys_config->get_mime_shared_database());

	mime_magic_ = magic_open(MAGIC_MIME | MAGIC_ERROR);
	if (mime_magic_ == (magic_t)NULL) {
		log_file->write_report("Failed to open magic number database", 
			"t_mime_database::t_mime_database", LOG_NORMAL, LOG_WARNING);
			
		return;
	}
	
	magic_load(mime_magic_, NULL);
}

t_mime_database::~t_mime_database() {
	magic_close(mime_magic_);
}

void t_mime_database::add_record(const t_mime_db_record &record) {
	map_mime2glob_.insert(make_pair(record.mimetype, record.file_glob));
}

string t_mime_database::get_glob(const string &mimetype) const {
	map<string, string>::const_iterator it = map_mime2glob_.find(mimetype);
	
	if (it != map_mime2glob_.end()) {
		return it->second;
	}
	
	return "";
}

string t_mime_database::get_mimetype(const string &filename) const {
	const char *mime_desc = magic_file(mime_magic_, filename.c_str());
	
	if (!mime_desc) return "";
	
	// Sometimes the magic libary adds additional info to the
	// returned mime type. Strip this info.
	string mime_type(mime_desc);
	string::size_type end_of_mime = mime_type.find_first_not_of(
			"abcdefghijklmnopqrstuvwxyz"
			"ABCDEFGHIJKLMNOPQSTUVWXYZ"
			"0123456789-.!%*_+`'~/");
	
	if (end_of_mime != string::npos) {
		mime_type = mime_type.substr(0, end_of_mime);
	}
	
	return mime_type;
}
