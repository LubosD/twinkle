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

#include "presence_state.h"

#include <cassert>
#include "buddy.h"
#include "pidf_body.h"
#include "log.h"

string t_presence_state::basic_state2str(t_presence_state::t_basic_state state) {
	switch (state) {
	case ST_BASIC_UNKNOWN:
		return "unknown";
	case ST_BASIC_CLOSED:
		return "closed";
	case ST_BASIC_OPEN:
		return "open";
	case ST_BASIC_FAILED:
		return "failed";
	case ST_BASIC_REJECTED:
		return "rejeceted";
	default:
		return "UNKNOWN";
	}
}

string t_presence_state::basic_state2pidf_str(t_presence_state::t_basic_state state) {
	if (state == ST_BASIC_OPEN) {
		return PIDF_STATUS_BASIC_OPEN;
	}
	
	// Convert all other states to "closed".
	return PIDF_STATUS_BASIC_CLOSED;
}

t_presence_state::t_presence_state() {
	assert(false);
}

t_presence_state::t_presence_state(t_buddy *_buddy) :
	buddy(_buddy),
	basic_state(ST_BASIC_UNKNOWN)
{
}

t_presence_state::t_basic_state t_presence_state::get_basic_state(void) const {
	t_basic_state result;
	mtx_state.lock();
	result = basic_state;
	mtx_state.unlock();
	return result;
}

string t_presence_state::get_failure_msg(void) const {
	string result;
	mtx_state.lock();
	result = failure_msg;
	mtx_state.unlock();
	return result;
}

void t_presence_state::set_basic_state(t_presence_state::t_basic_state state) {
	mtx_state.lock();
	basic_state = state;
	
	log_file->write_header("t_presence_state::set_basic_state", LOG_NORMAL, LOG_DEBUG);
	log_file->write_raw("Presence state changed to: ");
	log_file->write_raw(basic_state2str(basic_state));
	log_file->write_endl();
	log_file->write_raw(buddy->get_sip_address());
	log_file->write_endl();
	log_file->write_footer();
	
	mtx_state.unlock();
	
	// Notify the stat change to all observers of the buddy.
	buddy->notify();
}

void t_presence_state::set_failure_msg(const string &msg) {
	mtx_state.lock();
	failure_msg = msg;
	mtx_state.unlock();
}
