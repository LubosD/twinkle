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

#include "file_utils.h"
#include "util.h"

#include <fstream>
#include <string>
#include <vector>
#include <cerrno>
#include <cstdlib>
#include <unistd.h>

using namespace std;
using namespace utils;

bool utils::filecopy(const string &from, const string &to) {
	ifstream from_file(from.c_str());
	if (!from_file) {
		return false;
	}
	
	ofstream to_file(to.c_str());
	if (!to_file) {
		return false;
	}
	
	to_file << from_file.rdbuf();
	
	if (!to_file.good() || !from_file.good()) {
		return false;
	}
	
	return true;
}

string utils::strip_path_from_filename(const string &filename) {
	vector<string> v = split_on_last(filename, PATH_SEPARATOR);
	return v.back();
}

string utils::get_path_from_filename(const string &filename) {
	vector<string> v = split_on_last(filename, PATH_SEPARATOR);
	
	if (v.size() == 1) {
		// There is no path.
		return "";
	}
	
	return v.front();
}

string utils::get_extension_from_filename(const string &filename) {
	vector<string> v = split_on_last(filename, '.');
	
	if (v.size() == 1) {
		// There is no file extension.
		return "";
	} else {
		return v.back();
	}
}

string utils::apply_glob_to_filename(const string &filename, const string &glob) {
	string name = strip_path_from_filename(filename);
	string path = get_path_from_filename(filename);
	string new_name = glob;
	
	string::size_type idx = new_name.find('*');
	
	if (idx == string::npos) 
	{
		// The glob expression does not contain a wild card to replace.
		return filename;
	}
	
	new_name.replace(new_name.begin() + idx, new_name.begin() + idx + 1, name);
	
	string new_filename = path;
	if (!new_filename.empty()) {
		new_filename += PATH_SEPARATOR;
	}
	new_filename += new_name;
	
	return new_filename;
}

string get_working_dir(void) {
	size_t buf_size = 1024;
	char *buf = (char*)malloc(buf_size);
	char *dir = NULL;
	
	while (true) {
		if ((dir = getcwd(buf, buf_size)) != NULL) break;
		if (errno != ERANGE) break;
		
		// The buffer is too small.
		// Avoid eternal looping.
		if (buf_size > 8192) break;
		
		// Increase the buffer size
		free(buf);
		buf_size *= 2;
		buf = (char*)malloc(buf_size);
	}
	
	string result;
	if (dir) result = dir;
	
	free(buf);
	
	return result;
}
