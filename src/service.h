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

#ifndef _H_SERVICE
#define _H_SERVICE

#include <list>
#include "user.h"
#include "sockets/url.h"
#include "threads/mutex.h"

#define SVC_FILE_EXT	".svc"

using namespace std;

// Call forwarding types
enum t_cf_type {
	CF_ALWAYS,
	CF_BUSY,
	CF_NOANSWER
};

class t_service {
private:
	// Protect operations on the service
	t_mutex		mtx_service;
	
	t_user		*user_config;

	// Call redirection (call forwarding)
	bool			cf_always_active;
	list<t_display_url>	cf_always_dest;
	bool			cf_busy_active;
	list<t_display_url>	cf_busy_dest;
	bool			cf_noanswer_active;
	list<t_display_url>	cf_noanswer_dest;

	// Do not disturb
	// Note: CF_ALWAYS takes precedence over DND
	bool		dnd_active;
	
	// Auto answer
	bool		auto_answer_active;

	void lock();
	void unlock();
	
	t_service() {};

public:
	t_service(t_user *user);

	// All methods first lock the mtx_service mutex before executing
	// and unlock on return to guarantee the service data does not
	// get changed by other threads during execution.
	
	// General
	// Is more than 1 service active?
	// Different types of call forwarding counts as 1.
	bool multiple_services_active(void);

	// Call forwarding
	void enable_cf(t_cf_type cf_type, const list<t_display_url> &cf_dest);
	void disable_cf(t_cf_type cf_type);
	bool get_cf_active(t_cf_type cf_type, list<t_display_url> &dest);
	bool is_cf_active(void); // is any cf active?
	list<t_display_url> get_cf_dest(t_cf_type cf_type);

	// Do not disturb
	void enable_dnd(void);
	void disable_dnd(void);
	bool is_dnd_active(void) const;
	
	// Auto answer
	void enable_auto_answer(bool on);
	bool is_auto_answer_active(void) const;
	
	// Read/write service settings to file
	bool read_config(string &error_msg);
	bool write_config(string &error_msg);
};

#endif
