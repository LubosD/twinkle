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

#include "buddy.h"

#include <cassert>

#include "log.h"
#include "phone.h"
#include "phone_user.h"
#include "userintf.h"
#include "audits/memman.h"

extern t_phone		*phone;
extern t_event_queue	*evq_timekeeper;

/** Buddy */

void t_buddy::cleanup_presence_dialog(void) {
	assert(phone_user);

	if (presence_dialog && presence_dialog->get_subscription_state() == SS_TERMINATED) {
		string reason_termination = presence_dialog->get_reason_termination();
		bool may_resubscribe = presence_dialog->get_may_resubscribe();
		unsigned long dur_resubscribe = presence_dialog->get_resubscribe_after();
		
		MEMMAN_DELETE(presence_dialog);
		delete presence_dialog;
		presence_dialog = NULL;
		phone_user->stun_binding_inuse_presence--;
		phone_user->cleanup_stun_data();
		phone_user->cleanup_nat_keepalive();
		
		if (presence_auto_resubscribe) {
			if (may_resubscribe) {
				if (dur_resubscribe > 0) {
					start_resubscribe_presence_timer(dur_resubscribe * 1000);
				} else {
					subscribe_presence();
				}
			} else if (reason_termination.empty()) {
				start_resubscribe_presence_timer(DUR_PRESENCE_FAILURE * 1000);
			}
		}
	}
}

t_buddy::t_buddy() :
	phone_user(NULL),
	may_subscribe_presence(false),
	presence_state(this),
	presence_dialog(NULL),
	subscribe_after_stun(false),
	presence_auto_resubscribe(false),
	delete_after_presence_terminated(false),
	id_resubscribe_presence(0)
{
}

t_buddy::t_buddy(t_phone_user *_phone_user) :
	phone_user(_phone_user),
	may_subscribe_presence(false),
	presence_state(this),
	presence_dialog(NULL),
	subscribe_after_stun(false),
	presence_auto_resubscribe(false),
	delete_after_presence_terminated(false),
	id_resubscribe_presence(0)
{
}

t_buddy::t_buddy(t_phone_user *_phone_user, const string _name, const string &_sip_address) :
	phone_user(_phone_user),
	name(_name),
	sip_address(_sip_address),
	may_subscribe_presence(false),
	presence_state(this),
	presence_dialog(NULL),
	subscribe_after_stun(false),
	presence_auto_resubscribe(false),
	delete_after_presence_terminated(false),
	id_resubscribe_presence(0)
{
}

t_buddy::t_buddy(const t_buddy &other) :
	phone_user(other.phone_user),
	name(other.name),
	sip_address(other.sip_address),
	may_subscribe_presence(other.may_subscribe_presence),
	presence_state(this),
	presence_dialog(NULL),
	subscribe_after_stun(false),
	presence_auto_resubscribe(false),
	delete_after_presence_terminated(false),
	id_resubscribe_presence(0)
{}

t_buddy::~t_buddy() {
	if (presence_dialog) {
		MEMMAN_DELETE(presence_dialog);
		delete presence_dialog;
	}
}

string t_buddy::get_name(void) const {
	return name;
}

string t_buddy::get_sip_address(void) const {
	return sip_address;
}

bool t_buddy::get_may_subscribe_presence(void) const {
	return may_subscribe_presence;
}

const t_presence_state *t_buddy::get_presence_state(void) const {
	return &presence_state;
}

t_user *t_buddy::get_user_profile(void) {
	assert(phone_user);
	return phone_user->get_user_profile();
}

t_buddy_list *t_buddy::get_buddy_list(void) {
	assert(phone_user);
	return phone_user->get_buddy_list();
}

void t_buddy::set_phone_user(t_phone_user *_phone_user) {
	phone_user = _phone_user;
}

void t_buddy::set_name(const string &_name) {
	name = _name;
	notify();
}

void t_buddy::set_sip_address(const string &_sip_address) {
	sip_address = _sip_address;
	notify();
}

void t_buddy::set_may_subscribe_presence(bool _may_subscribe_presence) {
	may_subscribe_presence = _may_subscribe_presence;
	notify();
}

bool t_buddy::match_response(t_response *r, t_tuid tuid) const {
	return (presence_dialog && presence_dialog->match_response(r, tuid));
}

bool t_buddy::match_request(t_request *r) const {
	if (!presence_dialog) return false;
	
	bool partial_match = false;
	bool match = presence_dialog->match_request(r, partial_match);
	
	if (match) return true;
	
	if (partial_match && presence_dialog->get_remote_tag().empty()) {
		// A NOTIFY may be received before a 2XX on SUBSCRIBE.
		// In this case the NOTIFY will establish the dialog.
		return true;
	}
	
	return false;
}

bool t_buddy::match_timer(t_subscribe_timer timer, t_object_id id_timer) const {
	if (presence_dialog && presence_dialog->match_timer(timer, id_timer)) {
		return true;
	}
	
	return id_timer == id_resubscribe_presence;
}

void t_buddy::timeout(t_subscribe_timer timer, t_object_id id_timer) {
	switch (timer) {
	case STMR_SUBSCRIPTION:
		if (presence_dialog && presence_dialog->match_timer(timer, id_timer)) {
			(void)presence_dialog->timeout(timer);
			cleanup_presence_dialog();
		} else if (id_timer == id_resubscribe_presence) {
			// Try to subscribe to presence
			id_resubscribe_presence = 0;
			subscribe_presence();
		}
		break;
	default:
		assert(false);
	}
}

void t_buddy::recvd_response(t_response *r, t_tuid tuid, t_tid tid) {
	if (presence_dialog) {
		presence_dialog->recvd_response(r, tuid, tid);
		cleanup_presence_dialog();
	}
}

void t_buddy::recvd_request(t_request *r, t_tuid tuid, t_tid tid) {
	if (presence_dialog) {
		presence_dialog->recvd_request(r, tuid, tid);
		cleanup_presence_dialog();
	}
}

void t_buddy::start_resubscribe_presence_timer(unsigned long duration) {
	t_tmr_subscribe	*t;
	t = new t_tmr_subscribe(duration, STMR_SUBSCRIPTION, 0, 0, SIP_EVENT_PRESENCE, "");
	MEMMAN_NEW(t);
	id_resubscribe_presence = t->get_object_id();
	
	evq_timekeeper->push_start_timer(t);
	MEMMAN_DELETE(t);
	delete t;
}

void t_buddy::stop_resubscribe_presence_timer(void) {
	if (id_resubscribe_presence != 0) {
		evq_timekeeper->push_stop_timer(id_resubscribe_presence);
		id_resubscribe_presence = 0;
	}
}

void t_buddy::stun_completed(void) {
	if (subscribe_after_stun) {
		subscribe_after_stun = false;
		subscribe_presence();
	}
}

void t_buddy::stun_failed(void) {
	if (subscribe_after_stun) {
		subscribe_after_stun = false;
		start_resubscribe_presence_timer(DUR_PRESENCE_FAILURE * 1000);
	}
}

void t_buddy::subscribe_presence(void) {
	assert(phone_user);
	t_user *user_config = phone_user->get_user_profile();
	
	if (!may_subscribe_presence) return;
	
	presence_auto_resubscribe = true;

	if (presence_dialog) {
		// Already subscribed.
		log_file->write_header("t_buddy::subscribe_presence", LOG_NORMAL, LOG_DEBUG);
		log_file->write_raw("Already subscribed to presence: ");
		log_file->write_raw(name);
		log_file->write_raw(", ");
		log_file->write_raw(sip_address);
		log_file->write_endl();
		log_file->write_footer();
		return;
	}
	
	// If STUN is enabled, then do a STUN query before registering to
	// determine the public IP address.
	if (phone_user->use_stun) {
		if (phone_user->stun_public_ip_sip == 0)
		{
			phone_user->send_stun_request();
			phone_user->presence_subscribe_after_stun = true;
			subscribe_after_stun = true;
			return;
		}
		phone_user->stun_binding_inuse_presence++;
	}
	
	presence_dialog = new t_presence_dialog(phone_user, &presence_state);
	MEMMAN_NEW(presence_dialog);
	
	string dest = ui->expand_destination(user_config, sip_address);
	t_url dest_url(dest);
	if (!dest_url.is_valid()) {
		log_file->write_header("t_buddy::subscribe_presence", LOG_NORMAL, LOG_WARNING);
		log_file->write_raw("Invalid SIP address: ");
		log_file->write_raw(sip_address);
		log_file->write_endl();
		log_file->write_footer();
		return;
	}
	
	presence_dialog->subscribe(DUR_PRESENCE(user_config), dest_url, dest_url, "");
	
	// Start sending NAT keepalive packets when STUN is used
	// (or in case of symmetric firewall)
	if (phone_user->use_nat_keepalive && phone_user->id_nat_keepalive == 0) {
		// Just start the NAT keepalive timer. The SUBSCRIBE
		// message will create the NAT binding. So there is
		// no need to send a NAT keep alive packet now.
		phone->start_timer(PTMR_NAT_KEEPALIVE, phone_user);
	}
	
	cleanup_presence_dialog();
}

void t_buddy::unsubscribe_presence(bool remove) {
	presence_auto_resubscribe = false;
	stop_resubscribe_presence_timer();
	presence_state.set_basic_state(t_presence_state::ST_BASIC_UNKNOWN);
	delete_after_presence_terminated = remove;

	if (presence_dialog) {
		presence_dialog->unsubscribe();
		cleanup_presence_dialog();
	}
}

bool t_buddy::create_file_record(vector<string> &v) const {
	if (delete_after_presence_terminated) return false;

	v.clear();
	v.push_back(name);
	v.push_back(sip_address);
	v.push_back((may_subscribe_presence ? "y" : "n"));
	
	return true;
}

bool t_buddy::populate_from_file_record(const vector<string> &v) {
	if (v.size() !=3 ) return false;
	
	name = v[0];
	sip_address = v[1];
	may_subscribe_presence = (v[2] == "y");
	
	return true;
}

bool t_buddy::operator==(const t_buddy &other) const {
	return (name == other.name && sip_address == other.sip_address);
}

void t_buddy::clear_presence(void) {
	if (id_resubscribe_presence) stop_resubscribe_presence_timer();
	
	if (presence_dialog) {
		MEMMAN_DELETE(presence_dialog);
		delete presence_dialog;
		presence_dialog = NULL;
	}
	
	presence_state.set_basic_state(t_presence_state::ST_BASIC_UNKNOWN);
}

bool t_buddy::is_presence_terminated(void) const {
	return presence_dialog == NULL;
}

bool t_buddy::must_delete_now(void) const {
	return delete_after_presence_terminated && is_presence_terminated();
}

/** Buddy list */

void t_buddy_list::add_record(const t_buddy &record) {
	t_buddy r(record);
	r.set_phone_user(phone_user);
	utils::t_record_file<t_buddy>::add_record(r);
}

t_buddy_list::t_buddy_list(t_phone_user *_phone_user) :
	phone_user(_phone_user),
	is_subscribed(false)
{
	t_user *user_config = phone_user->get_user_profile();

	set_header("name|sip_address|subscribe");
	set_separator('|');
	
	string filename = user_config->get_profile_name() + BUDDY_FILE_EXT;
	string f = user_config->expand_filename(filename);
	set_filename(f);
}

t_user *t_buddy_list::get_user_profile(void) {
	return phone_user->get_user_profile();
}

t_buddy *t_buddy_list::add_buddy(const t_buddy &buddy) {
	t_buddy *b = NULL;
	
	mtx_records.lock();
	add_record(buddy);
	
	// KLUDGE: this code assumes that the buddy is added at the end.
	b = &records.back();
	mtx_records.unlock();
	
	log_file->write_header("t_buddy_list::add_buddy");
	log_file->write_raw("Added buddy: ");
	log_file->write_raw(b->get_name());
	log_file->write_raw(", ");
	log_file->write_raw(b->get_sip_address());
	log_file->write_endl();
	log_file->write_footer();
	
	return b;
}

void t_buddy_list::del_buddy(const t_buddy &buddy) {
	mtx_records.lock();
	
	list<t_buddy>::iterator it = find(records.begin(), records.end(), buddy);
			
	if (it == records.end()) {
		mtx_records.unlock();
		return;
	}
	
	log_file->write_header("t_buddy_list::del_buddy");
	log_file->write_raw("Delete buddy: ");
	log_file->write_raw(buddy.get_name());
	log_file->write_raw(", ");
	log_file->write_raw(buddy.get_sip_address());
	log_file->write_endl();
	log_file->write_footer();
	
	records.erase(it);
	
	mtx_records.unlock();
}

bool t_buddy_list::match_response(t_response *r, t_tuid tuid, t_buddy **buddy) {
	*buddy = NULL;
	
	mtx_records.lock();
	
	for (list<t_buddy>::iterator it = records.begin(); it != records.end(); ++it) {
		if (it->match_response(r, tuid)) {
			*buddy = &(*it);
			break;
		}
	}
	
	mtx_records.unlock();
	return *buddy != NULL;
}

bool t_buddy_list::match_request(t_request *r, t_buddy **buddy) {
	*buddy = NULL;
	
	mtx_records.lock();
	
	for (list<t_buddy>::iterator it = records.begin(); it != records.end(); ++it) {
		if (it->match_request(r)) {
			*buddy = &(*it);
			break;
		}
	}
	
	mtx_records.unlock();
	return *buddy != NULL;
}

bool t_buddy_list::match_timer(t_subscribe_timer timer, t_object_id id_timer, t_buddy **buddy) {
	*buddy = NULL;
	
	mtx_records.lock();
	
	for (list<t_buddy>::iterator it = records.begin(); it != records.end(); ++it) {
		if (it->match_timer(timer, id_timer)) {
			*buddy = &(*it);
			break;
		}
	}
	
	mtx_records.unlock();
	return *buddy != NULL;
}

void t_buddy_list::stun_completed(void) {
	mtx_records.lock();
	
	for (list<t_buddy>::iterator it = records.begin(); it != records.end(); ++it) {
		it->stun_completed();
	}
	
	mtx_records.unlock();
}

void t_buddy_list::stun_failed(void) {
	mtx_records.lock();
	
	for (list<t_buddy>::iterator it = records.begin(); it != records.end(); ++it) {
		it->stun_failed();
	}
	
	mtx_records.unlock();
}

void t_buddy_list::subscribe_presence(void) {
	mtx_records.lock();
	
	for (list<t_buddy>::iterator it = records.begin(); it != records.end(); ++it) {
		it->subscribe_presence();
	}
	
	is_subscribed = true;
	
	mtx_records.unlock();
}

void t_buddy_list::unsubscribe_presence(void) {
	mtx_records.lock();
	
	for (list<t_buddy>::iterator it = records.begin(); it != records.end(); ++it) {
		it->unsubscribe_presence();
	}
	
	is_subscribed = false;
	
	mtx_records.unlock();
}

bool t_buddy_list::get_is_subscribed() const {
	bool result;
	
	mtx_records.lock();
	result = is_subscribed;
	mtx_records.unlock();
	
	return result;
}

void t_buddy_list::clear_presence(void) {
	mtx_records.lock();
	
	for (list<t_buddy>::iterator it = records.begin(); it != records.end(); ++it) {
		it->clear_presence();
	}
	
	is_subscribed = false;
	
	mtx_records.unlock();
}

bool t_buddy_list::is_presence_terminated(void) const {
	bool result = true;
	mtx_records.lock();
	
	for (list<t_buddy>::const_iterator it = records.begin(); it != records.end(); ++it) {
		if (!it->is_presence_terminated()) {
			result = false;
			break;
		}
	}
	
	mtx_records.unlock();
	
	return result;
}
