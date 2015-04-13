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

#ifndef _LOG_H
#define _LOG_H

#include <string>
#include <fstream>
#include "threads/mutex.h"
#include "threads/sema.h"
#include "threads/thread.h"

using namespace std;

#define LOG_FILENAME    "twinkle.log"

// Severity of a log message
enum t_log_severity {
	LOG_INFO,
	LOG_WARNING,
	LOG_CRITICAL,
	LOG_DEBUG
};

// Message class
enum t_log_class {
	LOG_NORMAL,
	LOG_SIP,
	LOG_STUN,
	LOG_MEMORY
};

class t_log {
private:
	/** Maximum length of a logged string (bytes) */
	static const string::size_type MAX_LEN_LOG_STRING = 1024;

        string          log_filename;
        ofstream        *log_stream;

	// Mutex for exclusive acces to the log file
	t_mutex		mtx_log;

	// Indicates if logging is disabled
	bool		log_disabled;
	bool		log_report_disabled;
	
	// Indicates if the user should be informed about log updates
	bool		inform_user;
	
	// Indicates if new data for the log viewer is available
	t_semaphore	*sema_logview;
	
	// Thread for updating the log viewer
	t_thread	*thr_logview;

        // Move the current log file to the .old log file
        bool move_current_to_old(void);

public:
        t_log();
        ~t_log();

        // Write a report with header and footer
        void write_report(const string &report, const string &func_name); // normal, info
	void write_report(const string &report, const string &func_name,
		t_log_class log_class, t_log_severity severity = LOG_INFO);

        // Write header
	// This locks the mtx_log. So you must call write footer to release
	// the log again!
        void write_header(const string &func_name); // class normal, severity info
	void write_header(const string &func_name, t_log_class log_class,
	                  t_log_severity severity = LOG_INFO);

        // Write footer
	// This unlocks the mtx_log.
        void write_footer(void);

        // Write raw data
        void write_raw(const string &raw);
        void write_raw(int raw);
        void write_raw(unsigned int raw);
        void write_raw(unsigned short raw);
        void write_raw(unsigned long raw);
	void write_raw(long raw);
	void write_bool(bool raw);

        // Write end of line
        void write_endl(void);
        
        // Return the full path name of the log file
        string get_filename(void) const;
        
        // Enable/disable user informs on updates
        void enable_inform_user(bool on);
        
        // Block till log information is available for log viewer
        void wait_for_log(void);
};

extern t_log *log_file;

#endif
