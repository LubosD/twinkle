/*
    Copyright (C) 2005-2007  Michel de Boer <michel@twinklephone.com>

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

#define COMMENT_SYMBOL	'#'

template< class R >
void t_record_file<R>::split_record(const string &record, vector<string> &v) const {
	v = split_escaped(record, field_separator);
	
	for (vector<string>::iterator it = v.begin(); it != v.end(); ++it) {
		*it = unescape(*it);
	}
}

template< class R >
string t_record_file<R>::join_fields(const vector<string> &v) const {
	string s;
	
	for (vector<string>::const_iterator it = v.begin(); it != v.end(); ++it) {
		if (it != v.begin()) s += field_separator;
		
		// Escape comment symbol.
		if (!it->empty() && it->at(0) == COMMENT_SYMBOL) s += '\\';
		
		s += escape(*it, field_separator);
	}
	
	return s;
}

template< class R >
void t_record_file<R>::add_record(const R &record) {
	records.push_back(record);
}

template< class R >
t_record_file<R>::t_record_file(): field_separator('|')
{}

template< class R >
t_record_file<R>::t_record_file(const string &_header, char _field_separator, 
		const string &_filename) :
	header(_header),
	field_separator(_field_separator),
	filename(_filename)
{}

template< class R >
void t_record_file<R>::set_header(const string &_header) {
	header = _header;
}

template< class R >
void t_record_file<R>::set_separator(char _separator) {
	field_separator = _separator;
}

template< class R >
void t_record_file<R>::set_filename(const string &_filename) {
	filename = _filename;
}

template< class R >
list<R> *t_record_file<R>::get_records() {
	return &records;
}

template< class R >
bool t_record_file<R>::load(string &error_msg) {
	struct stat stat_buf;
	
	mtx_records.lock();
	
	records.clear();
	
	// Check if file exists
	if (filename.empty() || stat(filename.c_str(), &stat_buf) != 0) {
		// There is no file.
		mtx_records.unlock();
		return true;
	}
	
	// Open call ile
	ifstream file(filename.c_str());
	if (!file) {
		error_msg = TRANSLATE("Cannot open file for reading: %1");
		error_msg = replace_first(error_msg, "%1", filename);
		mtx_records.unlock();
		return false;
	}
	
	// Read and parse history file.
	while (!file.eof()) {
		string line;
		
		getline(file, line);

		// Check if read operation succeeded
		if (!file.good() && !file.eof()) {
			error_msg = TRANSLATE("File system error while reading file %1 .");
			error_msg = replace_first(error_msg, "%1", filename);
			mtx_records.unlock();
			return false;
		}

		line = trim(line);

		// Skip empty lines
		if (line.size() == 0) continue;

		// Skip comment lines
		if (line[0] == COMMENT_SYMBOL) continue;
		
		// Add record. Skip records that cannot be parsed.
		R record;
		vector<string> v;
		split_record(line, v);
		if (record.populate_from_file_record(v)) {
			add_record(record);
		}
	}
	
	mtx_records.unlock();
	return true;
}

template< class R >
bool t_record_file<R>::save(string &error_msg) const {	
	if (filename.empty()) return false;
	
	mtx_records.lock();
	
	// Open file
	ofstream file(filename.c_str());
	if (!file) {
		error_msg = TRANSLATE("Cannot open file for writing: %1");
		error_msg = replace_first(error_msg, "%1", filename);
		mtx_records.unlock();
		return false;
	}
	
	// Write file header
	file << "# " << header << endl;
	      
	// Write records
	for (record_const_iterator i = records.begin(); i != records.end(); ++i) {
		vector<string> v;
		if (i->create_file_record(v)) {
			file << join_fields(v);
			file << endl;
		}
	}
	
	mtx_records.unlock();
	 
	if (!file.good()) {
		error_msg = TRANSLATE("File system error while writing file %1 .");
		error_msg = replace_first(error_msg, "%1", filename);
		return false;
	}
	
	return true;
}
