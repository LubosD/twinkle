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

#include "msg_session.h"

#include <cassert>
#include <sys/time.h>

#include "im_iscomposing_body.h"
#include "log.h"
#include "phone.h"
#include "translator.h"
#include "parser/media_type.h"
#include "utils/file_utils.h"

#define COMPOSING_LOCAL_IDLE_TIMEOUT	15
#define COMPOSING_LOCAL_REFRESH_TIMEOUT	90

extern t_phone *phone;

using namespace im;
using namespace utils;

t_composing_state im::string2composing_state(const string &state_name) {
	if (state_name == IM_ISCOMPOSING_STATE_ACTIVE) {
		return COMPOSING_STATE_ACTIVE;
	}
	
	return COMPOSING_STATE_IDLE;
}

string im::composing_state2string(t_composing_state state) {
	switch (state) {
	case COMPOSING_STATE_IDLE:
		return "idle";
	case COMPOSING_STATE_ACTIVE:
		return "active";
	default:
		assert(false);
	}
	
	return "idle";
}

// class t_msg

t_msg::t_msg() :
	has_attachment(false)
{
	struct timeval t;

	gettimeofday(&t, NULL);
	timestamp = t.tv_sec;
}

t_msg::t_msg(const string &msg, t_direction dir, t_text_format fmt) :
	message(msg),
	direction(dir),
	format(fmt),
	has_attachment(false)
{
	struct timeval t;

	gettimeofday(&t, NULL);
	timestamp = t.tv_sec;
}

void t_msg::set_attachment(const string &filename, const t_media &media, const string &save_as) {
	attachment_filename = filename;
	attachment_media = media;
	attachment_save_as_name = save_as;
	has_attachment = true;
}

// class t_msg_session

t_msg_session::t_msg_session(t_user *u) :
	user_config(u),
	new_message_added(false),
	error_recvd(false),
	delivery_notification_recvd(false),
	msg_in_flight(false),
	send_composing_state(u->get_im_send_iscomposing()),
	local_composing_state(COMPOSING_STATE_IDLE),
	local_idle_timeout(0),
	local_refresh_timeout(0),
	remote_composing_state(COMPOSING_STATE_IDLE),
	remote_idle_timeout(0)
{}

t_msg_session::t_msg_session(t_user *u, t_display_url _remote_party) :
	user_config(u),
	remote_party(_remote_party),
	new_message_added(false),
	error_recvd(false),
	delivery_notification_recvd(false),
	msg_in_flight(false),
	send_composing_state(u->get_im_send_iscomposing()),
	local_composing_state(COMPOSING_STATE_IDLE),
	local_idle_timeout(0),
	local_refresh_timeout(0),
	remote_composing_state(COMPOSING_STATE_IDLE),
	remote_idle_timeout(0)
{}

t_msg_session::~t_msg_session() {
	// Remove temporary files.
	for (list<t_msg>::iterator it = messages.begin(); it != messages.end(); ++it) {
		// Temporary files are created for incoming messages only.
		if (it->has_attachment && it->direction == MSG_DIR_IN) {
			// Defensive check to make sure we are deleting tmp files only.
			if (sys_config->is_tmpfile(it->attachment_filename)) {
				log_file->write_header("t_msg_session::~t_msg_session");
				log_file->write_raw("Remove tmp file ");
				log_file->write_raw(it->attachment_filename);
				log_file->write_endl();
				log_file->write_footer();
				
				unlink(it->attachment_filename.c_str());
			}
		}
	}
}

t_user *t_msg_session::get_user(void) const {
	return user_config;
}

t_display_url t_msg_session::get_remote_party(void) const {
	return remote_party;
}

t_composing_state t_msg_session::get_remote_composing_state(void) const {
	return remote_composing_state;
}

void t_msg_session::set_user(t_user *u) {
	user_config = u;
}

void t_msg_session::set_remote_party(const t_display_url &du) {
	remote_party = du;
}

void t_msg_session::set_send_composing_state(bool enable) {
	send_composing_state = enable;
}

t_msg t_msg_session::get_last_message(void) {
	new_message_added = false;
	if (messages.empty()) {
		throw empty_list_exception();
	}
	return messages.back();
}

bool t_msg_session::is_new_message_added(void) const {
	return new_message_added;
}

void t_msg_session::set_display_if_empty(const string &display) {
	if (remote_party.display.empty()) {
		remote_party.display = display;
	}
}

const list<t_msg> &t_msg_session::get_messages(void) const {
	return messages;
}

void t_msg_session::recv_msg(const t_msg &msg) {
	// RFC 3994 3.3
	// The composing state of the remote party transitions to idle
	// when a message is received.
	remote_composing_state = COMPOSING_STATE_IDLE;
	remote_idle_timeout = 0;

	messages.push_back(msg);
	new_message_added = true;
	notify();
}

void t_msg_session::send_msg(const string &message, t_text_format format) {
	// RFC 3994 3.2
	// If a content message is sent before the idle threshold expires, no
	// "idle" state indication is needed.
	// The local state is set to idle without sending an indication to the
	// remote party.
	local_composing_state = COMPOSING_STATE_IDLE;
	local_idle_timeout = 0;
	local_refresh_timeout = 0;

	t_msg msg(message, im::MSG_DIR_OUT, format);
	messages.push_back(msg);
	new_message_added = true;
	
	bool ret = phone->pub_send_message(user_config, remote_party.url, remote_party.display, msg);
	
	if (ret) {
		msg_in_flight = true;
	} else {
		set_error(TRANSLATE("Failed to send message."));
	}
	
	notify();
}

void t_msg_session::send_file(const string &filename, const t_media &media, const string &subject) {
	// RFC 3994 3.2
	// If a content message is sent before the idle threshold expires, no
	// "idle" state indication is needed.
	// The local state is set to idle without sending an indication to the
	// remote party.
	local_composing_state = COMPOSING_STATE_IDLE;
	local_idle_timeout = 0;
	local_refresh_timeout = 0;

	t_msg msg;
	msg.set_attachment(filename, media, strip_path_from_filename(filename));
	msg.subject = subject;
	msg.direction = MSG_DIR_OUT;
	messages.push_back(msg);
	new_message_added = true;
	
	bool ret = phone->pub_send_message(user_config, remote_party.url, remote_party.display, msg);
	
	if (ret) {
		msg_in_flight = true;
		notify();
	} else {
		// Notify user interface about the sent message before
		// setting the error.
		notify();
		
		set_error(TRANSLATE("Failed to send message."));
	}
}

void t_msg_session::set_error(const string &message) {
	error_msg = message;
	error_recvd = true;
	notify();
}

bool t_msg_session::error_received(void) const {
	return error_recvd;
}

string t_msg_session::take_error(void) {
	if (!error_recvd) return "";
	error_recvd = false;
	return error_msg;
}

void t_msg_session::set_delivery_notification(const string &notification) {
	delivery_notification = notification;
	delivery_notification_recvd = true;
	notify();
}

bool t_msg_session::delivery_notification_received(void) const {
	return delivery_notification_recvd;
}

string t_msg_session::take_delivery_notification(void) {
	if (!delivery_notification_recvd) return "";
	delivery_notification_recvd = false;
	return delivery_notification;
}

bool t_msg_session::match(t_user *user, t_url _remote_party) {
	return user == user_config && _remote_party == remote_party.url;
}

void t_msg_session::set_msg_in_flight(bool in_flight) {
	msg_in_flight = in_flight;
	notify();
}

bool t_msg_session::is_msg_in_flight(void) const {
	return msg_in_flight;
}

void t_msg_session::set_local_composing_state(t_composing_state state) {
	if (!remote_party.is_valid()) {
		// The session is not yet established
		return;
	}

	switch (local_composing_state) {
	case COMPOSING_STATE_IDLE:
		switch (state) {
		case COMPOSING_STATE_IDLE:
			break;
		case COMPOSING_STATE_ACTIVE:
			local_composing_state = state;
			local_idle_timeout = COMPOSING_LOCAL_IDLE_TIMEOUT;
			local_refresh_timeout = COMPOSING_LOCAL_REFRESH_TIMEOUT - 10;
			
			if (send_composing_state) {
				(void)phone->pub_send_im_iscomposing(
					user_config, remote_party.url, 
					remote_party.display,
					IM_ISCOMPOSING_STATE_ACTIVE,
					COMPOSING_LOCAL_REFRESH_TIMEOUT);
			}
					
			break;
		default:
			assert(false);
		}
		
		break;	
	case COMPOSING_STATE_ACTIVE:
		switch (state) {
		case COMPOSING_STATE_IDLE:
			local_composing_state = state;
			local_idle_timeout = 0;
			local_refresh_timeout = 0;
			
			if (send_composing_state) {
				(void)phone->pub_send_im_iscomposing(
					user_config, remote_party.url, 
					remote_party.display,
					IM_ISCOMPOSING_STATE_IDLE,
					COMPOSING_LOCAL_REFRESH_TIMEOUT);
			}
					
			break;
		case COMPOSING_STATE_ACTIVE:
			local_idle_timeout = COMPOSING_LOCAL_IDLE_TIMEOUT;
			break;
		default:
			assert(false);
		}
		
		break;
	default:
		assert(false);
	}
}

void t_msg_session::set_remote_composing_state(t_composing_state state, time_t idle_timeout) {
	switch (remote_composing_state) {
	case COMPOSING_STATE_IDLE:
		switch (state) {
		case COMPOSING_STATE_IDLE:
			break;
		case COMPOSING_STATE_ACTIVE:
			remote_composing_state = state;
			remote_idle_timeout = idle_timeout;
			notify();
			
			break;
		default:
			assert(false);
		}
		
		break;
	case COMPOSING_STATE_ACTIVE:
		switch (state) {
		case COMPOSING_STATE_IDLE:
			remote_composing_state = state;
			remote_idle_timeout = 0;
			notify();
			
			break;
		case COMPOSING_STATE_ACTIVE:
			remote_idle_timeout = idle_timeout;
			break;
		}
		
		break;
	default:
		assert(false);
	}
}

void t_msg_session::dec_local_composing_timeout(void) {
	if (local_composing_state == COMPOSING_STATE_IDLE) return;
	
	local_idle_timeout--;
	if (local_idle_timeout == 0) {
		set_local_composing_state(COMPOSING_STATE_IDLE);
	} else {
		local_refresh_timeout--;
		if (local_refresh_timeout == 0) {
			local_refresh_timeout = COMPOSING_LOCAL_REFRESH_TIMEOUT - 10;
			
			if (send_composing_state) {
				(void)phone->pub_send_im_iscomposing(
					user_config, remote_party.url, 
					remote_party.display,
					IM_ISCOMPOSING_STATE_ACTIVE,
					COMPOSING_LOCAL_REFRESH_TIMEOUT);
			}
		}
	}
}

void t_msg_session::dec_remote_composing_timeout(void) {
	if (remote_composing_state == COMPOSING_STATE_IDLE) return;
	
	remote_idle_timeout--;
	if (remote_idle_timeout == 0) {
		set_remote_composing_state(COMPOSING_STATE_IDLE);
	}
}
