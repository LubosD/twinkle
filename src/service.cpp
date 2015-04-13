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
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include "service.h"
#include "log.h"
#include "userintf.h"
#include "util.h"

#define FLD_CF_ALWAYS			"cf_always"
#define FLD_CF_BUSY			"cf_busy"
#define FLD_CF_NOANSWER			"cf_noanswer"
#define FLD_DND				"dnd"
#define FLD_AUTO_ANSWER			"auto_answer"

void t_service::lock() {
	mtx_service.lock();
}

void t_service::unlock() {
	mtx_service.unlock();
}

t_service::t_service(t_user *user) {
	user_config = user;

	// Call redirection
	cf_always_active = false;
	cf_busy_active = false;
	cf_noanswer_active = false;

	// Do not disturb
	dnd_active = false;
	
	// Auto answer
	auto_answer_active = false;
	
	string msg;
	(void)read_config(msg);
}

bool t_service::multiple_services_active(void) {
	int num_services = 0;
	
	if (is_cf_active()) num_services++;
	if (is_dnd_active()) num_services++;
	if (is_auto_answer_active()) num_services++;
	
	if (num_services > 1) return true;
	
	return false;
}

void t_service::enable_cf(t_cf_type cf_type, const list<t_display_url> &cf_dest) {
	lock();

	switch (cf_type) {
	case CF_ALWAYS:
		cf_always_active = true;
		cf_always_dest = cf_dest;
		break;
	case CF_BUSY:
		cf_busy_active = true;
		cf_busy_dest = cf_dest;
		break;
	case CF_NOANSWER:
		cf_noanswer_active = true;
		cf_noanswer_dest = cf_dest;
		break;
	default:
		assert(false);
	}

	unlock();
	
	string msg;
	(void)write_config(msg);
}

void t_service::disable_cf(t_cf_type cf_type) {
	lock();

	switch (cf_type) {
	case CF_ALWAYS:
		cf_always_active = false;
		cf_always_dest.clear();
		break;
	case CF_BUSY:
		cf_busy_active = false;
		cf_busy_dest.clear();
		break;
	case CF_NOANSWER:
		cf_noanswer_active = false;
		cf_noanswer_dest.clear();
		break;
	default:
		assert(false);
	}

	unlock();
	
	string msg;
	(void)write_config(msg);
}

bool t_service::get_cf_active(t_cf_type cf_type, list<t_display_url> &dest) {
	bool active = false;

	lock();

	switch (cf_type) {
	case CF_ALWAYS:
		active = cf_always_active;
		dest = cf_always_dest;
		break;
	case CF_BUSY:
		active = cf_busy_active;
		dest = cf_busy_dest;
		break;
	case CF_NOANSWER:
		active = cf_noanswer_active;
		dest = cf_noanswer_dest;
		break;
	default:
		assert(false);
	}

	unlock();
	return active;
}

bool t_service::is_cf_active(void) {
	bool active = false;

	lock();
	active = cf_always_active || cf_busy_active || cf_noanswer_active;
	unlock();

	return active;
}

list<t_display_url> t_service::get_cf_dest(t_cf_type cf_type) {
	list<t_display_url> dest;

	lock();

	switch (cf_type) {
	case CF_ALWAYS:
		dest = cf_always_dest;
		break;
	case CF_BUSY:
		dest = cf_busy_dest;
		break;
	case CF_NOANSWER:
		dest = cf_noanswer_dest;
		break;
	default:
		assert(false);
	}

	unlock();
	return dest;
}

void t_service::enable_dnd(void) {
	lock();
	dnd_active = true;
	unlock();
	
	string msg;
	(void)write_config(msg);
}

void t_service::disable_dnd(void) {
	lock();
	dnd_active = false;
	unlock();
	
	string msg;
	(void)write_config(msg);
}

bool t_service::is_dnd_active(void) const {
	return dnd_active;
}

void t_service::enable_auto_answer(bool on) {
	lock();
	auto_answer_active = on;
	unlock();
	
	string msg;
	(void)write_config(msg);
}

bool t_service::is_auto_answer_active(void) const {
	return auto_answer_active;
}

bool t_service::read_config(string &error_msg) {
	struct stat stat_buf;
	
	lock();
	
	string filename = user_config->get_profile_name() + SVC_FILE_EXT;
	string f = user_config->expand_filename(filename);
	
	// Check if config file exists
	if (stat(f.c_str(), &stat_buf) != 0) {
		unlock();
		return true;
	}
	
	// Open file
	ifstream config(f.c_str());
	if (!config) {
		error_msg = "Cannot open file for reading: ";
		error_msg += f;
		log_file->write_report(error_msg, "t_service::read_config",
			LOG_NORMAL, LOG_CRITICAL);
		unlock();
		return false;
	}

	t_display_url display_url;
	cf_always_active = false;
	cf_always_dest.clear();
	cf_busy_active = false;
	cf_busy_dest.clear();
	cf_noanswer_active = false;
	cf_noanswer_dest.clear();
	
	while (!config.eof()) {
		string line;
		getline(config, line);

		// Check if read operation succeeded
		if (!config.good() && !config.eof()) {
			error_msg = "File system error while reading file ";
			error_msg += f;
			log_file->write_report(error_msg, "t_service::read_config",
				LOG_NORMAL, LOG_CRITICAL);
			unlock();
			return false;
		}

		line = trim(line);

		// Skip empty lines
		if (line.size() == 0) continue;

		// Skip comment lines
		if (line[0] == '#') continue;

		vector<string> l = split_on_first(line, '=');
		if (l.size() != 2) {
			error_msg = "Syntax error in file ";
			error_msg += f;
			error_msg += "\n";
			error_msg += line;
			log_file->write_report(error_msg, "t_service::read_config",
				LOG_NORMAL, LOG_CRITICAL);
			unlock();
			return false;
		}

		string parameter = trim(l[0]);
		string value = trim(l[1]);
		
		if (parameter == FLD_CF_ALWAYS) {
			ui->expand_destination(user_config, value, display_url);
			if (display_url.is_valid()) {
				cf_always_active = true;
				cf_always_dest.push_back(display_url);
			}
		} else if (parameter == FLD_CF_BUSY) {
			ui->expand_destination(user_config, value, display_url);
			if (display_url.is_valid()) {
				cf_busy_active = true;
				cf_busy_dest.push_back(display_url);
			}
		} else if (parameter == FLD_CF_NOANSWER) {
			ui->expand_destination(user_config, value, display_url);
			if (display_url.is_valid()) {
				cf_noanswer_active = true;
				cf_noanswer_dest.push_back(display_url);
			}
		} else if (parameter == FLD_DND) {
			dnd_active = yesno2bool(value);
		} else if (parameter == FLD_AUTO_ANSWER) {
			auto_answer_active = yesno2bool(value);
		} else {
			// Ignore unknown parameters. Only report in log file.
			log_file->write_header("t_service::read_config",
				LOG_NORMAL, LOG_WARNING);
			log_file->write_raw("Unknown parameter in service config: ");
			log_file->write_raw(parameter);
			log_file->write_endl();
			log_file->write_footer();
		}
	}
	
	unlock();
	return true;
}

bool t_service::write_config(string &error_msg) {
	struct stat stat_buf;
	
	lock();

	string filename = user_config->get_profile_name() + SVC_FILE_EXT;
	string f = user_config->expand_filename(filename);

	// Make a backup of the file if we are editing an existing file, so
	// that can be restored when writing fails.
	string f_backup = f + '~';
	if (stat(f.c_str(), &stat_buf) == 0) {
		if (rename(f.c_str(), f_backup.c_str()) != 0) {
			string err = get_error_str(errno);
			error_msg = "Failed to backup ";
			error_msg += f;
			error_msg += " to ";
			error_msg += f_backup;
			error_msg += "\n";
			error_msg += err;
			log_file->write_report(error_msg, "t_service::write_config",
				LOG_NORMAL, LOG_CRITICAL);
			unlock();
			return false;
		}
	}

	ofstream config(f.c_str());
	if (!config) {
		error_msg = "Cannot open file for writing: ";
		error_msg += f;
		log_file->write_report(error_msg, "t_user::write_config",
			LOG_NORMAL, LOG_CRITICAL);
		unlock();
		return false;
	}

	for (list<t_display_url>::iterator i = cf_always_dest.begin(); 
	     i != cf_always_dest.end(); i++)
	{
		config << FLD_CF_ALWAYS << '=' << i->encode() << endl;
	}
	
	for (list<t_display_url>::iterator i = cf_busy_dest.begin(); 
	     i != cf_busy_dest.end(); i++)
	{
		config << FLD_CF_BUSY << '=' << i->encode() << endl;
	}
	
	for (list<t_display_url>::iterator i = cf_noanswer_dest.begin(); 
	     i != cf_noanswer_dest.end(); i++)
	{
		config << FLD_CF_NOANSWER << '=' << i->encode() << endl;
	}
	
	config << FLD_DND << '=' << bool2yesno(dnd_active) << endl;
	config << FLD_AUTO_ANSWER << '=' << bool2yesno(auto_answer_active) << endl;
	
	// Check if writing succeeded
	if (!config.good()) {
		// Restore backup
		config.close();
		rename(f_backup.c_str(), f.c_str());

		error_msg = "File system error while writing file ";
		error_msg += f;
		log_file->write_report(error_msg, "t_service::write_config",
			LOG_NORMAL, LOG_CRITICAL);
		unlock();
		return false;
	}
	
	unlock();
	return true;
}
