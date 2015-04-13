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

#include <cstdio>
#include <iostream>
#include <fstream>
#include <sys/time.h>
#include "log.h"
#include "sys_settings.h"
#include "translator.h"
#include "userintf.h"
#include "user.h"
#include "util.h"

// Pointer allocations/de-allocations are not checked by MEMMAN as the
// log file will be deleted after the MEMMAN reports are logged and hence
// would show false memory leaks.

extern t_userintf cli;

// Main function for log viewer
void *main_logview(void *arg) {
	while (true) {
		log_file->wait_for_log();
		// TODO: handle situation where log file was zapped.
		if (ui) ui->cb_log_updated(false);
	}
}

bool t_log::move_current_to_old(void) {
        string old_log = log_filename + ".old";
        if (rename(log_filename.c_str(), old_log.c_str()) != 0) {
		return false;
	}

	return true;
}

t_log::t_log() {
	log_disabled = false;
	log_report_disabled = false;
	inform_user = false;
	sema_logview = NULL;
	thr_logview = NULL;

        log_filename = DIR_HOME;
        log_filename += "/";
        log_filename += DIR_USER;
        log_filename += "/";
        log_filename += LOG_FILENAME;

        // If there is a previous log file, then move that to the .old file
        // before zapping the current log file.
        (void)move_current_to_old();

	log_stream = new ofstream(log_filename.c_str());
	if (!*log_stream) {
		log_disabled = true;
		string err = TRANSLATE("Failed to create log file %1 .");
		err = replace_first(err, "%1", log_filename);
		err += "\nLogging is now disabled.";
		if (ui) ui->cb_show_msg(err, MSG_WARNING);
		return;
	}

	string s = PRODUCT_NAME;
	s += ' ';
	s += PRODUCT_VERSION;
	s += ", ";
	s += PRODUCT_DATE;
	write_report(s, "t_log::t_log");
	
	string options_built = sys_config->get_options_built();
	if (!options_built.empty()) {
		s = "Built with support for: ";
		s += options_built;
		write_report(s, "t_log::t_log");
	}
}

t_log::~t_log() {
	if (thr_logview) delete thr_logview;
	if (sema_logview) delete sema_logview;
	delete log_stream;
}

void t_log::write_report(const string &report, const string &func_name) {
	write_report(report, func_name, LOG_NORMAL, LOG_INFO);
}

void t_log::write_report(const string &report, const string &func_name,
		t_log_class log_class, t_log_severity severity)
{
	if (log_disabled) return;

	write_header(func_name, log_class, severity);
	write_raw(report);
	write_endl();
	write_footer();
}

void t_log::write_header(const string &func_name) {
	write_header(func_name, LOG_NORMAL, LOG_INFO);
}

void t_log::write_header(const string &func_name, t_log_class log_class,
	                  t_log_severity severity)
{
	if (log_disabled) return;
	
	mtx_log.lock();
	
	if (severity == LOG_DEBUG) {
		 if (!sys_config->get_log_show_debug()) {
		 	log_report_disabled = true;
		 	return;
		 }
	}
	
	switch (log_class) {
	case LOG_SIP:
		if (!sys_config->get_log_show_sip()) {
			log_report_disabled = true;
			return;
		}
		break;
	case LOG_STUN:
		if (!sys_config->get_log_show_stun()) {
			log_report_disabled = true;
			return;
		}
		break;
	case LOG_MEMORY:
		if (!sys_config->get_log_show_memory()) {
			log_report_disabled = true;
			return;
		}
		break;
	default:
		break;
	}

	struct timeval t;
	struct tm tm;
	time_t	date;

	gettimeofday(&t, NULL);
	date = t.tv_sec;
	localtime_r(&date, &tm);

	*log_stream << "+++ ";
	*log_stream << tm.tm_mday;
	*log_stream << "-";
	*log_stream << tm.tm_mon + 1;
	*log_stream << "-";
	*log_stream << tm.tm_year + 1900;
	*log_stream << " ";
	*log_stream << int2str(tm.tm_hour, "%02d");
	*log_stream << ":";
	*log_stream << int2str(tm.tm_min, "%02d");
	*log_stream << ":";
	*log_stream << int2str(tm.tm_sec, "%02d");
	*log_stream << ".";
	*log_stream << ulong2str(t.tv_usec, "%06d");
	*log_stream << " ";

	// Severity
	switch (severity) {
	case LOG_INFO:
		*log_stream << "INFO";
		break;
	case LOG_WARNING:
		*log_stream << "WARNING";
		break;
	case LOG_CRITICAL:
		*log_stream << "CRITICAL";
		break;
	case LOG_DEBUG:
		*log_stream << "DEBUG";
		break;
	default:
		*log_stream << "UNNKOWN";
		break;
	}
	*log_stream << " ";

	// Message class
	switch (log_class) {
	case LOG_NORMAL:
		*log_stream << "NORMAL";
		break;
	case LOG_SIP:
		*log_stream << "SIP";
		break;
	case LOG_STUN:
		*log_stream << "STUN";
		break;
	case LOG_MEMORY:
		*log_stream << "MEMORY";
		break;
	default:
		*log_stream << "UNNKOWN";
		break;
	}
	*log_stream << " ";

	*log_stream << func_name;
	*log_stream << endl;
}

void t_log::write_footer(void) {
	if (log_disabled) return;
	
	if (log_report_disabled) {
		log_report_disabled = false;
		mtx_log.unlock();
		return;
	}

	*log_stream << "---\n\n";
	log_stream->flush();

	// Check if log file is still in a good state
	if (!log_stream->good()) {
		// Log file is bad, disable logging
		log_disabled = true;
		if (ui) ui->cb_display_msg("Writing to log file failed. Logging disabled.",
			MSG_WARNING);
		mtx_log.unlock();
		return;
	}

	bool log_zapped = false;
	if (log_stream->tellp() >= sys_config->get_log_max_size() * 1000000) {
		*log_stream << "*** Log full. Rotate to new log file. ***\n";
		log_stream->flush();
		log_stream->close();

		if (!move_current_to_old()) {
			// Failed to move log file. Disable logging
			if (ui) ui->cb_display_msg("Renaming log file failed. Logging disabled.",
				MSG_WARNING);
			log_disabled = true;
			mtx_log.unlock();
			return;
		}

		delete log_stream;

		log_stream = new ofstream(log_filename.c_str());
		if (!*log_stream) {
			// Failed to create a new log file. Disable logging
			if (ui) ui->cb_display_msg("Creating log file failed. Logging disabled.",
				MSG_WARNING);
			log_disabled = true;
			mtx_log.unlock();
			return;
		}
		
		log_zapped = true;
	}

	mtx_log.unlock();
	
	// Inform user about log update.
	// This code must be outside the locked region, otherwise it causes
	// a deadlock between the GUI and log mutexes.
	if (inform_user && sema_logview) sema_logview->up();
}

void t_log::write_raw(const string &raw) {
	if (log_disabled || log_report_disabled) return;
	
	if (raw.size() < MAX_LEN_LOG_STRING) {
		*log_stream << to_printable(raw);
	} else {
		*log_stream << to_printable(raw.substr(0, MAX_LEN_LOG_STRING));
		*log_stream << "\n\n";
		*log_stream << "<cut off>\n";
	}
}

void t_log::write_raw(int raw) {
	if (log_disabled || log_report_disabled) return;

	*log_stream << raw;
}

void t_log::write_raw(unsigned int raw) {
	if (log_disabled || log_report_disabled) return;

	*log_stream << raw;
}

void t_log::write_raw(unsigned short raw) {
	if (log_disabled || log_report_disabled) return;

	*log_stream << raw;
}

void t_log::write_raw(unsigned long raw) {
	if (log_disabled || log_report_disabled) return;

	*log_stream << raw;
}

void t_log::write_raw(long raw) {
	if (log_disabled || log_report_disabled) return;

	*log_stream << raw;
}

void t_log::write_bool(bool raw) {
	if (log_disabled || log_report_disabled) return;
	
	*log_stream << (raw ? "yes" : "no");
}

void t_log::write_endl(void) {
	if (log_disabled || log_report_disabled) return;

	*log_stream << endl;
}

string t_log::get_filename(void) const {
	return log_filename;
}

void t_log::enable_inform_user(bool on) {
	if (on) {
		if (!sema_logview) {
			sema_logview = new t_semaphore(0);
		}
		
		if (!thr_logview) {
			thr_logview = new t_thread(main_logview, NULL);
			thr_logview->detach();
		}
	} else {
		if (thr_logview) {
			thr_logview->cancel();
			delete thr_logview;
			thr_logview = NULL;
		}
		
		if (sema_logview) {
			delete sema_logview;
			sema_logview = NULL;
		}
	}

	inform_user = on;
}

void t_log::wait_for_log(void) {
	if (sema_logview) sema_logview->down();
}
