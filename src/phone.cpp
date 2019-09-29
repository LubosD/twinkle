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
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <assert.h>
#include <iostream>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "call_history.h"
#include "call_script.h"
#include "exceptions.h"
#include "phone.h"
#include "line.h"
#include "log.h"
#include "sdp/sdp.h"
#include "translator.h"
#include "util.h"
#include "user.h"
#include "userintf.h"
#include "audits/memman.h"
#include "parser/parse_ctrl.h"
#include "sockets/socket.h"
#include "stun/stun_transaction.h"

extern t_phone 		*phone;
extern t_event_queue	*evq_timekeeper;
extern string		user_host;

// t_transfer_data

t_transfer_data::t_transfer_data(t_request *r, unsigned short _lineno, bool _hide_user, 
		t_phone_user *pu) :
	refer_request(dynamic_cast<t_request *>(r->copy())),
	lineno(_lineno),
	hide_user(_hide_user),
	phone_user(pu)
{}
	
t_transfer_data::~t_transfer_data() {
	MEMMAN_DELETE(refer_request);
	delete refer_request;
}

t_request *t_transfer_data::get_refer_request(void) const {
	return refer_request;
}

bool t_transfer_data::get_hide_user(void) const {
	return hide_user;
}

unsigned short t_transfer_data::get_lineno(void) const {
	return lineno;
}

t_phone_user *t_transfer_data::get_phone_user(void) const {
	return phone_user;
}


// t_phone	

///////////
// Private
///////////

void t_phone::move_line_to_background(unsigned short lineno) {
	// R/W lock is held by callers

	// The line will be released in the background. It should
	// immediately release its RTP ports as these maybe needed
	// for new calls.
	lines.at(lineno)->kill_rtp();
	
	cleanup_3way_state(lineno);
	
	// Move the line to the back of the vector.
	lines.push_back(lines.at(lineno));
	lines.back()->line_number = lines.size() - 1;
	
	// Create a new line for making calls.
	lines.at(lineno) = new t_line(this, lineno);
	MEMMAN_NEW(lines.at(lineno));
	
	// The new line must have the same RTP port as the
	// releasing line, otherwise it may conflict with
	// the other lines. Due to call transfers, the port
	// number may be unrelated to the line position.
	lines.at(lineno)->rtp_port = lines.back()->get_rtp_port();
	
	log_file->write_header("t_phone::move_line_to_background",
		LOG_NORMAL, LOG_DEBUG);
	log_file->write_raw("Moved line ");
	log_file->write_raw(lineno + 1);
	log_file->write_raw(" to background position ");
	log_file->write_raw(lines.back()->get_line_number() + 1);
	log_file->write_endl();
	log_file->write_footer();
	
	// Notify the user interface of the line state change
	ui->cb_line_state_changed();
}

void t_phone::cleanup_dead_lines(void) {
	t_rwmutex_writer x(lines_mtx);

	// Only remove idle lines at the end of the dead pool to avoid
	// moving lines in the vector.
	while (lines.size() > NUM_CALL_LINES && lines.back()->get_state() == LS_IDLE)
	{
		
		log_file->write_header("t_phone::cleanup_dead_lines",
			LOG_NORMAL, LOG_DEBUG);
		log_file->write_raw("Removed dead line ");
		log_file->write_raw(lines.back()->get_line_number() + 1);
		log_file->write_endl();
		log_file->write_footer();
		
		MEMMAN_DELETE(lines.back());
		delete lines.back();
		lines.pop_back();
	}
}

void t_phone::move_releasing_lines_to_background(void) {
	t_rwmutex_writer x(lines_mtx);

	// NOTE: the line on the REFERRER position is not moved to the
	//       background as a subscription may still be active.
	for (int i = 0; i < NUM_USER_LINES; i++) {
		if (lines.at(i)->get_substate() == LSSUB_RELEASING &&
		    !lines.at(i)->get_keep_seized()) 
		{
			move_line_to_background(i);
		}
	}
}

void t_phone::cleanup_3way_state(unsigned short lineno) {
	assert(lineno < lines.size());

	t_mutex_guard x(mutex_3way);

	// Clean up 3-way data if the line was involved in a 3-way
	if (is_3way)
	{
		bool line_in_3way = false;
		t_audio_session *as_peer;
		t_line *line_peer;

		if (lineno == line1_3way->get_line_number()) {
			line_in_3way = true;
			line_peer = line2_3way;
		} else if (lineno == line2_3way->get_line_number()) {
			line_in_3way = true;
			line_peer = line1_3way;
		}

		if (line_in_3way) {
			// Stop the 3-way mixing on the peer line
			as_peer = line_peer->get_audio_session();
			if (as_peer) as_peer->stop_3way();

			// Make the peer line the active line
			set_active_line(line_peer->get_line_number());
			
			// If the 3-way was with mixed codec sample rates, then
			// the remaining audio session might have a mismatch
			// between the sound card sample rate and the codec
			// sample rate. In that case clear the sample rate by
			// toggling the audio session off and on.
			if (!as_peer->matching_sample_rates()) {
				log_file->write_report(
					"Hold/retrieve call to align codec and sound card.",
					"t_phone::line_cleared", LOG_NORMAL, LOG_DEBUG);
				line_peer->hold(true);
				line_peer->retrieve();
			}

			is_3way = false;
			line1_3way = NULL;
			line2_3way = NULL;
			ui->cb_line_state_changed();
		}
	}

}

void t_phone::cleanup_3way(void) {
	if (!is_3way) return;
	
	if (line1_3way->get_substate() == LSSUB_IDLE) {
		cleanup_3way_state(line1_3way->get_line_number());
	} else if (line2_3way->get_substate() == LSSUB_IDLE) {
		cleanup_3way_state(line2_3way->get_line_number());
	}
}

void t_phone::invite(t_phone_user *pu, const t_url &to_uri, const string &to_display,
		const string &subject, bool no_fork, bool anonymous)
{
	t_rwmutex_reader x(lines_mtx);

	// Ignore if active line is not idle
	if (lines[active_line]->get_state() != LS_IDLE) {
		return;
	}

	lines[active_line]->invite(pu, to_uri, to_display, subject, no_fork, anonymous);
}

void t_phone::answer(void) {
	t_rwmutex_reader x(lines_mtx);

	// Ignore if active line is idle
	if (lines[active_line]->get_state() == LS_IDLE) return;

	lines[active_line]->answer();
}

void t_phone::reject(void) {
	t_rwmutex_reader x(lines_mtx);

	// Ignore if active line is idle
	if (lines[active_line]->get_state() == LS_IDLE) return;

	lines[active_line]->reject();
}

void t_phone::reject(unsigned short line) {
	t_rwmutex_reader x(lines_mtx);

	if (line > NUM_USER_LINES) return;
	if (lines[line]->get_state() == LS_IDLE) return;
	
	lines[line]->reject();
}

void t_phone::redirect(const list<t_display_url> &destinations, int code, string reason)
{
	t_rwmutex_reader x(lines_mtx);

	// Ignore if active line is idle
	if (lines[active_line]->get_state() == LS_IDLE) return;

	lines[active_line]->redirect(destinations, code, reason);
}

void t_phone::end_call(void) {
	t_rwmutex_writer x(lines_mtx);

	// If 3-way is active then end call on both lines
	if (is_3way && (
	    active_line == line1_3way->get_line_number() ||
	    active_line == line2_3way->get_line_number()))
	{
		if (sys_config->get_hangup_both_3way()) {
			line1_3way->end_call();
			line2_3way->end_call();
			
			// NOTE: moving a line to the dying pool causes the
			// 3way line pointers to be cleared.
			unsigned short lineno1 = line1_3way->get_line_number();
			unsigned short lineno2 = line2_3way->get_line_number();
			move_line_to_background(lineno1);
			move_line_to_background(lineno2);
		} else {
			// Hangup the active line, and make the next
			// line active.
			int l = active_line;
			activate_line((l+1) % NUM_USER_LINES);
			lines.at(l)->end_call();
			move_line_to_background(l);
		}
		
		return;
	}

	// Ignore if active line is idle
	if (lines.at(active_line)->get_state() == LS_IDLE) return;

	lines.at(active_line)->end_call();

	move_line_to_background(active_line);
}

void t_phone::registration(t_phone_user *pu, t_register_type register_type, 
		unsigned long expires)
{
	pu->registration(register_type, false, expires);
}

void t_phone::options(t_phone_user *pu, const t_url &to_uri, const string &to_display) {
	pu->options(to_uri, to_display);
}

void t_phone::options(void) {
	t_rwmutex_reader x(lines_mtx);

	lines[active_line]->options();
}

bool t_phone::hold(bool rtponly) {
	// A line in a 3-way call cannot be held
	if (is_3way && (
	    active_line == line1_3way->get_line_number() ||
	    active_line == line2_3way->get_line_number()))
	{
		return false;
	}

	t_rwmutex_reader x(lines_mtx);
	return lines[active_line]->hold(rtponly);
}

void t_phone::retrieve(void) {
	t_rwmutex_reader x(lines_mtx);
	lines[active_line]->retrieve();
}

void t_phone::refer(const t_url &uri, const string &display) {
	t_rwmutex_reader x(lines_mtx);
	lines[active_line]->refer(uri, display);
}

void t_phone::refer(unsigned short lineno_from, unsigned short lineno_to) 
{
	t_rwmutex_future_writer x(lines_mtx);

	// The nicest transfer is an attended transfer. An attended transfer
	// is only possible of the transfer target supports the 'replaces'
	// extension (RFC 3891).
	// If 'replaces' is not supported, then a transfer with consultation
	// is done. First hang up the consultation call, then transfer the
	// line.
	// HACK: if the call is in progress, then assume that Replaces is
	//       supported. We don't know if it is as the call is not established
	//       yet. An in-progress call can only be replaced if the user
	//       deliberately allowed this (allow_transfer_consultation_inprog).
	if (lines.at(lineno_to)->remote_extension_supported(EXT_REPLACES)) {
		log_file->write_report("Remote end supports 'replaces'.\n"\
			"Attended transfer.",
			"t_phone::refer");
		refer_attended(lineno_from, lineno_to);
	} else if (get_line_substate(lineno_to) == LSSUB_OUTGOING_PROGRESS) {
		log_file->write_report("Call transfer while consultation in progress.\n"\
			"Attended transfer.",
			"t_phone::refer");
		refer_attended(lineno_from, lineno_to);	
	} else {
		log_file->write_report("Remote end does not support 'replaces'.\n"\
			"Transfer with consultation.",
			"t_phone::refer");
		refer_consultation(lineno_from, lineno_to);
	}
}

// Attended call transfer
// See draft-ietf-sipping-cc-transfer-07 7.3
void t_phone::refer_attended(unsigned short lineno_from, unsigned short lineno_to) 
{
	t_rwmutex_reader x(lines_mtx);

	t_line *line = lines.at(lineno_to);
	
	switch (line->get_substate())
	{
	case LSSUB_ESTABLISHED:
		// Transfer allowed
		break;
	case LSSUB_OUTGOING_PROGRESS:
		{
			unsigned short dummy;
			t_user *user_config = get_line_user(lineno_to);
			if (!user_config->get_allow_transfer_consultation_inprog() ||
			    !is_line_transfer_consult(lineno_to, dummy))
			{
				// Transfer not allowed
				return;
			}
		}
		
		// Transfer allowed
		break;
	default:
		// Transfer not allowed
		return;
	};
	
	t_user *user_config = get_line_user(lineno_from);
	
	// draft-ietf-sipping-cc-transfer-07 section 7.3
	// The call must be referred to the contact URI of the far-end.
	// As the contact URI may not be globally routable, the AoR
	// may be used alternatively.
	t_url uri;
	string display;
	if (user_config->get_attended_refer_to_aor()) 
	{
		if (line->get_substate() == LSSUB_OUTGOING_PROGRESS) {
			uri = line->get_remote_uri_pending();
			display = line->get_remote_target_display_pending();
		} else {
			uri = line->get_remote_uri();
			display = line->get_remote_target_display();
		}
	} else {
		if (line->get_substate() == LSSUB_OUTGOING_PROGRESS) {
			uri = line->get_remote_target_uri_pending();
			display = line->get_remote_target_display_pending();
		} else {
			uri = line->get_remote_target_uri();
			display = line->get_remote_target_display();
		}
	}

	// Create Replaces header for replacing the call on lineno_to
	t_hdr_replaces hdr_replaces;
	
	if (line->get_substate() == LSSUB_OUTGOING_PROGRESS) {
		hdr_replaces.set_call_id(line->get_call_id_pending());
		hdr_replaces.set_from_tag(line->get_local_tag_pending());
		hdr_replaces.set_to_tag(line->get_remote_tag_pending());
	} else {
		hdr_replaces.set_call_id(line->get_call_id());
		hdr_replaces.set_from_tag(line->get_local_tag());
		hdr_replaces.set_to_tag(line->get_remote_tag());
	}
	
	uri.add_header(hdr_replaces);
	
	// draft-ietf-sipping-cc-transfer-07 section 7.3
	// If the call is referred to the AoR, then add a Require header
	// that requires the 'Replaces' extension, to make the correct phone
	// ring in case of forking.
	if (user_config->get_attended_refer_to_aor()) {
		t_hdr_require hdr_require;
		hdr_require.add_feature(EXT_REPLACES);
		uri.add_header(hdr_require);
	}
	
	// Transfer call
	lines.at(lineno_from)->refer(uri, display);
}

// Call transfer with consultation
// See draft-ietf-sipping-cc-transfer-07 7
void t_phone::refer_consultation(unsigned short lineno_from, unsigned short lineno_to) 
{
	t_rwmutex_writer x(lines_mtx);
	t_line *line = lines.at(lineno_to);
	
	if (line->get_substate() != LSSUB_ESTABLISHED) {
		return;
	}
	
	// Refer call to the URI of the far-end
	t_url uri = line->get_remote_uri();
	string display = line->get_remote_display();
	
	// End consultation call
	line->end_call();

	move_line_to_background(lineno_to);
	
	// Transfer call
	lines.at(lineno_from)->refer(uri, display);
}

void t_phone::setup_consultation_call(const t_url &uri, const string &display) {
	unsigned short consult_line;
	if (!get_idle_line(consult_line)) {
		log_file->write_report("Cannot get idle line for consultation call.",
			"t_phone::setup_consultation_call");
		return;
	}
	
	unsigned short xfer_line = active_line;
	t_user *user_config = get_line_user(xfer_line);
	
	t_phone_user *pu = find_phone_user(user_config->get_profile_name());
	if (!pu) {
		log_file->write_header("t_phone::setup_consultation_call", 
				LOG_NORMAL, LOG_WARNING);
		log_file->write_raw("User profile not active: ");
		log_file->write_raw(user_config->get_profile_name());
		log_file->write_footer();
	}
	
	activate_line(consult_line);
	
	string subject = TRANSLATE("Call transfer - %1");
	subject = replace_first(subject, "%1",
			ui->format_sip_address(user_config, 
				get_remote_display(xfer_line), 
				get_remote_uri(xfer_line)));
			
	bool no_fork = false;
	if (user_config->get_allow_transfer_consultation_inprog()) {
		// If the configuration allows a call to be transferred
		// while the consultation call is still in progress, then
		// we send a no-fork request disposition in the INVTE
		// to setup the consultation call. This way we know that
		// the call has not been forked and it should be possible
		// to replace the early dialog.
		// 
		// The scenario is:
		// A calls B
		// B sets up a consultation call to C (no-fork)
		// B sends a REFER with replaces to A
		// A sends an INVITE with replaces to C.
		//
		// NOTE: this is a non-standard implementation. RFC 3891
		//       does not allow to replace an early dialog not
		//       setup by the UA. In this case the REFER from A to C
		//       intends to replace the dialog from B to C, but C
		//       did not setup the B-C dialog itself.
		no_fork = true;
	}
	
	invite(pu, uri, display, subject, no_fork, false);
	
	t_rwmutex_reader x(lines_mtx);
	lines.at(consult_line)->set_is_transfer_consult(true, xfer_line);
	lines.at(xfer_line)->set_to_be_transferred(true, consult_line);
	
	ui->cb_consultation_call_setup(user_config, consult_line);
}

void t_phone::activate_line(unsigned short l) {
	unsigned short a = get_active_line();
	if (a == l) return;

	// Just switch the active line if there is a conference.
	if (is_3way) {
		set_active_line(l);
		ui->cb_line_state_changed();
		return;
	}


	// Put the current active line on hold if it has a call.
	// Only established calls can be put on-hold. Transient calls
	// should be torn down or just kept in the same transient state
	// when switching to the other line.
	if (get_line(a)->get_state() == LS_BUSY && !hold()) {
		// The line is busy but could not be put on-hold. Determine
		// what to do based on the line sub state.
		switch(get_line(a)->get_substate()) {
		case LSSUB_OUTGOING_PROGRESS:
			// User has outgoing call in progress on the active
			// line, but decided to switch line, so tear down
			// the call.
			end_call();
			ui->cb_stop_call_notification(a);
			break;
		case LSSUB_INCOMING_PROGRESS:
			// The incoming call on the current active will stay,
			// just stop the ring tone.
			ui->cb_stop_call_notification(a);
			break;
		case LSSUB_ANSWERING:
			// Answering is in progress, so call cannot be put
			// on-hold. Tear down the call.
			end_call();
			break;
		case LSSUB_RELEASING:
			// The releasing call on the current line will get
			// released. No need to take any action here.
			break;
		default:
			// This should not happen.
			log_file->write_report("ERROR: Call cannot be put on hold.",
				"t_phone::activate_line");
		}
	}

	set_active_line(l);

	t_rwmutex_reader x(lines_mtx);

	// Retrieve the call on the new active line unless that line
	// is transferring a call and the user profile indicates that
	// the referrer holds the call during call transfer.
	t_user *user_config = lines[l]->get_user();
	if (get_line_refer_state(l) == REFST_NULL || 
	    (user_config && !user_config->get_referrer_hold()))
	{
		retrieve();
	}

	// Play ring tone, if the new active line has an incoming call
	// in progress.
	if (get_line(l)->get_substate() == LSSUB_INCOMING_PROGRESS) {
		ui->cb_play_ringtone(l);
	}

	ui->cb_line_state_changed();
}

void t_phone::send_dtmf(char digit, bool inband, bool info) {
	t_rwmutex_reader x(lines_mtx);
	lines[active_line]->send_dtmf(digit, inband, info);
}

void t_phone::start_timer(t_phone_timer timer, t_phone_user *pu) {
	t_tmr_phone	*t;
	t_user		*user_config = pu->get_user_profile();

	switch(timer) {
	case PTMR_NAT_KEEPALIVE:
		t = new t_tmr_phone(user_config->get_timer_nat_keepalive() * 1000, timer, this);
		MEMMAN_NEW(t);
		pu->id_nat_keepalive = t->get_object_id();
		break;
	case PTMR_TCP_PING:
		t = new t_tmr_phone(user_config->get_timer_tcp_ping() * 1000, timer, this);
		MEMMAN_NEW(t);
		pu->id_tcp_ping = t->get_object_id();
		break;
	default:
		assert(false);
	}

	evq_timekeeper->push_start_timer(t);
	MEMMAN_DELETE(t);
	delete t;
}

void t_phone::stop_timer(t_phone_timer timer, t_phone_user *pu) {
	unsigned short	*id;

	switch(timer) {
	case PTMR_REGISTRATION:
		id = &pu->id_registration;
		break;
	case PTMR_NAT_KEEPALIVE:
		id = &pu->id_nat_keepalive;
		break;
	case PTMR_TCP_PING:
		id = &pu->id_tcp_ping;
		break;
	default:
		assert(false);
	}

	if (*id != 0) evq_timekeeper->push_stop_timer(*id);
	*id = 0;
}

void t_phone::start_set_timer(t_phone_timer timer, long time, t_phone_user *pu) {
	t_tmr_phone	*t;


	switch(timer) {
	case PTMR_REGISTRATION:
		long new_time;

		// Re-register before registration expires
		if (pu->get_last_reg_failed() || time <= RE_REGISTER_DELTA * 1000) {
			new_time = time;
		} else {
			new_time = time - (RE_REGISTER_DELTA * 1000);
		}
		t = new t_tmr_phone(new_time, timer, this);
		MEMMAN_NEW(t);
		pu->id_registration = t->get_object_id();
		break;
	default:
		assert(false);
	}

	evq_timekeeper->push_start_timer(t);
	MEMMAN_DELETE(t);
	delete t;
}

void t_phone::handle_response_out_of_dialog(t_response *r, t_tuid tuid, t_tid tid) {
	t_phone_user *pu = match_phone_user(r, tuid);
	if (!pu) {
		log_file->write_report("Response does not match any pending request.",
			"t_phone::handle_response_out_of_dialog");
		return;
	}
	
	log_file->write_header("t_phone::handle_response_out_of_dialog", LOG_NORMAL, LOG_DEBUG);
	log_file->write_raw("Out of dialog matches phone user: ");
	log_file->write_raw(pu->get_user_profile()->get_profile_name());
	log_file->write_endl();
	log_file->write_footer();
	
	pu->handle_response_out_of_dialog(r, tuid, tid);
}

void t_phone::handle_response_out_of_dialog(StunMessage *r, t_tuid tuid) {
	t_phone_user *pu = match_phone_user(r, tuid);
	if (!pu) {
		log_file->write_report("STUN response does not match any pending request.",
			"t_phone::handle_response_out_of_dialog");
		return;
	}
	
	pu->handle_response_out_of_dialog(r, tuid);
}

t_phone_user *t_phone::find_phone_user(const string &profile_name) const {

	for (list<t_phone_user *>::const_iterator i = phone_users.begin();
	     i != phone_users.end(); ++i)
	{
		if (!(*i)->is_active()) continue;
		
		t_user *user_config = (*i)->get_user_profile();
		if (user_config->get_profile_name() == profile_name) {
			return *i;
		}
	}
	
	return NULL;
}

t_phone_user *t_phone::find_phone_user(const t_url &user_uri) const {

	for (list<t_phone_user *>::const_iterator i = phone_users.begin();
		     i != phone_users.end(); ++i)
	{
		if (!(*i)->is_active()) continue;
		
		t_user *user_config = (*i)->get_user_profile();
		if (t_url(user_config->create_user_uri(false)) == user_uri) {
			return *i;
		}
	}
	
	return NULL;
}

t_phone_user *t_phone::match_phone_user(t_response *r, t_tuid tuid, bool active_only) {
	t_rwmutex_reader x(phone_users_mtx);

	for (list<t_phone_user *>::iterator i = phone_users.begin();
	     i != phone_users.end(); ++i)
	{
		if (active_only && !(*i)->is_active()) continue;
		if ((*i)->match(r, tuid)) return *i;
	}
	
	return NULL;
}

t_phone_user *t_phone::match_phone_user(t_request *r, bool active_only) {
	t_rwmutex_reader x(phone_users_mtx);

	for (list<t_phone_user *>::iterator i = phone_users.begin();
	     i != phone_users.end(); ++i)
	{
		if (active_only && !(*i)->is_active()) continue;
		if ((*i)->match(r)) return *i;
	}
	
	return NULL;
}

t_phone_user *t_phone::match_phone_user(StunMessage *r, t_tuid tuid, bool active_only) {
	t_rwmutex_reader x(phone_users_mtx);

	for (list<t_phone_user *>::iterator i = phone_users.begin();
	     i != phone_users.end(); ++i)
	{
		if (active_only && !(*i)->is_active()) continue;
		if ((*i)->match(r, tuid)) return *i;
	}
	
	return NULL;
}

int t_phone::hunt_line(void) {
	t_rwmutex_reader x(lines_mtx);

	// Send incoming call to active line if it is idle.
	if (lines.at(active_line)->get_substate() == LSSUB_IDLE) {
		return active_line;
	}
	
	if (sys_config->get_call_waiting() || all_lines_idle()) {
		// Send the INVITE to the first idle unseized line
		for (unsigned short i = 0; i < NUM_USER_LINES; i++) {
			if (lines[i]->get_substate() == LSSUB_IDLE) {
				return i;
			}
		}
	}
	
	return -1;
}

//////////////
// Protected
//////////////

void t_phone::recvd_provisional(t_response *r, t_tuid tuid, t_tid tid) {
	t_rwmutex_reader x(lines_mtx);

	for (unsigned short i = 0; i < lines.size(); i++) {
		if (lines[i]->match(r, tuid)) {
			lines[i]->recvd_provisional(r, tuid, tid);
			return;
		}
	}

	// out-of-dialog response
	// Provisional responses should only be given for INVITE.
	// A response for an INVITE is always in a dialog.
	// Ignore provisional responses for other requests.
}

void t_phone::recvd_success(t_response *r, t_tuid tuid, t_tid tid) {
	t_rwmutex_reader x(lines_mtx);

	for (unsigned short i = 0; i < lines.size(); i++) {
		if (lines[i]->match(r, tuid)) {
			lines[i]->recvd_success(r, tuid, tid);
			return;
		}
	}

	// out-of-dialog responses
	handle_response_out_of_dialog(r, tuid, tid);
}

void t_phone::recvd_redirect(t_response *r, t_tuid tuid, t_tid tid) {
	t_rwmutex_reader x(lines_mtx);

	for (unsigned short i = 0; i < lines.size(); i++) {
		if (lines[i]->match(r, tuid)) {
			lines[i]->recvd_redirect(r, tuid, tid);
			return;
		}
	}

	// out-of-dialog responses
	handle_response_out_of_dialog(r, tuid, tid);
}

void t_phone::recvd_client_error(t_response *r, t_tuid tuid, t_tid tid) {
	t_rwmutex_reader x(lines_mtx);

	for (unsigned short i = 0; i < lines.size(); i++) {
		if (lines[i]->match(r, tuid)) {
			lines[i]->recvd_client_error(r, tuid, tid);
			return;
		}
	}

	// out-of-dialog responses
	handle_response_out_of_dialog(r, tuid, tid);
}

void t_phone::recvd_server_error(t_response *r, t_tuid tuid, t_tid tid) {
	t_rwmutex_reader x(lines_mtx);

	for (unsigned short i = 0; i < lines.size(); i++) {
		if (lines[i]->match(r, tuid)) {
			lines[i]->recvd_server_error(r, tuid, tid);
			return;
		}
	}

	// out-of-dialog responses
	handle_response_out_of_dialog(r, tuid, tid);
}

void t_phone::recvd_global_error(t_response *r, t_tuid tuid, t_tid tid) {
	t_rwmutex_reader x(lines_mtx);

	for (unsigned short i = 0; i < lines.size(); i++) {
		if (lines[i]->match(r, tuid)) {
			lines[i]->recvd_global_error(r, tuid, tid);
			return;
		}
	}

	// out-of-dialog responses
	handle_response_out_of_dialog(r, tuid, tid);
}

void t_phone::post_process_response(t_response *r, t_tuid tuid, t_tid tid) {
	cleanup_dead_lines();
	move_releasing_lines_to_background();
	cleanup_3way();
}

void t_phone::recvd_invite(t_request *r, t_tid tid) {
	t_rwmutex_future_writer x(lines_mtx);

	// Check if this INVITE is a retransmission.
	// Once the TU sent a 2XX repsonse on an INVITE it has to deal
	// with retransmissions.
	for (unsigned short i = 0; i < lines.size(); i++) {
		if (lines[i]->is_invite_retrans(r)) {
			lines[i]->process_invite_retrans();
			return;
		}
	}
	
	// RFC 3261 12.2.2
	// An INVITE with a To-header without a tag is an initial
	// INVITE
	if (r->hdr_to.tag == "") {
		recvd_initial_invite(r, tid);
	} else {
		recvd_re_invite(r, tid);
	}	
}

void t_phone::recvd_initial_invite(t_request *r, t_tid tid) {
	t_response *resp;
	list <string> unsupported;
	t_call_record call_record;
	
	// Find out for which user this INVITE is.
	t_phone_user *pu = match_phone_user(r, true);
	if (!pu) {
		resp = r->create_response(R_404_NOT_FOUND);
		send_response(resp, 0, tid);
		
		// Do not create a call history record as this is a misrouted
		// call.
		
		MEMMAN_DELETE(resp);
		delete resp;
		return;
	}
	
	// Reject call if phone is not active
	if (!is_active) {
		resp = r->create_response(R_480_TEMP_NOT_AVAILABLE);
		send_response(resp, 0, tid);
		MEMMAN_DELETE(resp);
		delete resp;
		return;
	}
	
	t_user *user_config = pu->get_user_profile();

	// Check if the far end requires any unsupported extensions
	if (!user_config->check_required_ext(r, unsupported))
	{
		// Not all required extensions are supported
		resp = r->create_response(R_420_BAD_EXTENSION);
		resp->hdr_unsupported.set_features(unsupported);
		send_response(resp, 0, tid);
		
		// Do not create a call history record here. The far-end
		// should retry the call without the extension, so this
		// is not a missed call from the user point of view.
				
		MEMMAN_DELETE(resp);
		delete resp;
		return;
	}
	
	// RFC 3891 3
	// If a replaces header is present, check if it matches a dialog
	int replace_line = -1;
	if (r->hdr_replaces.is_populated() && user_config->get_ext_replaces()) {
		bool early_matched = false;
		bool no_fork_req_disposition =
			r->hdr_request_disposition.is_populated() &&
			r->hdr_request_disposition.fork_directive == t_hdr_request_disposition::NO_FORK;
		
		t_rwmutex_reader x(lines_mtx);

		for (size_t i = 0; i < lines.size(); i++) {
			if (lines.at(i)->match_replaces(r->hdr_replaces.call_id,
				r->hdr_replaces.to_tag,
				r->hdr_replaces.from_tag,
				no_fork_req_disposition,
				early_matched))
			{
				replace_line = i;
				break;
			}
		}
		
		if (replace_line >= NUM_CALL_LINES) {
			// Replaces header matches a releasing line.
			resp = r->create_response(R_603_DECLINE);
			send_response(resp, 0, tid);
			MEMMAN_DELETE(resp);
			delete resp;
			return;
		} else if (replace_line >= 0) {
			if (replace_line == active_line) {
				if (r->hdr_replaces.early_only && !early_matched) {
					resp = r->create_response(R_486_BUSY_HERE);
					send_response(resp, 0, tid);
					MEMMAN_DELETE(resp);
					delete resp;
					return;
				}
				
				// The existing call will be torn down only after
				// it has been checked that this incoming INVITE
				// is not rejected by the user, e.g. DND.
			} else {
				// Implementation decision:
				// Don't allow a held call to be replaced.
				resp = r->create_response(R_486_BUSY_HERE);
				send_response(resp, 0, tid);
				
				// Create a call history record
				call_record.start_call(r, t_call_record::DIR_IN, 
					user_config->get_profile_name());
				call_record.fail_call(resp);
				call_history->add_call_record(call_record);
		
				MEMMAN_DELETE(resp);
				delete resp;
				return;
			}
		} else {
			// Replaces does not match any line.
			resp = r->create_response(R_481_TRANSACTION_NOT_EXIST);
			send_response(resp, 0, tid);
			MEMMAN_DELETE(resp);
			delete resp;
			return;
		}
	}
	
	// Hunt for an idle line to handle the call.
	int hunted_line = -1;
	if (replace_line >= 0) {
		hunted_line = replace_line;
	} else {
		hunted_line = hunt_line();
	}
	
	t_display_url display_url;
	list<t_display_url> cf_dest; // call forwarding destinations
	
	// Call user defineable incoming call script to determine how
	// to handle this call
	t_script_result script_result;
	
	if (!user_config->get_script_incoming_call().empty()) {
		// Send 100 Trying as the script might take a while
		resp = r->create_response(R_100_TRYING);
		send_response(resp, 0, tid);
		MEMMAN_DELETE(resp);
		delete resp;
		
		t_call_script script(user_config, t_call_script::TRIGGER_IN_CALL, hunted_line + 1);
		script.exec_action(script_result, r);
		
		if (!script_result.display_msgs.empty()) {
			string text(join_strings(script_result.display_msgs, "\n"));
			ui->cb_display_msg(text, MSG_NO_PRIO);
		}
		
		// Override display name with caller name returned by script
		if (!script_result.caller_name.empty()) {
			r->hdr_from.display_override = script_result.caller_name;
			log_file->write_header("t_phone::recvd_invite", 
					LOG_NORMAL, LOG_DEBUG);
			log_file->write_raw("Override display name with caller name:\n");
			log_file->write_raw(script_result.caller_name);
			log_file->write_endl();
			log_file->write_footer();
		}
	}
	
	t_call_script script_in_call_failed(user_config, t_call_script::TRIGGER_IN_CALL_FAILED, 0);
	
	// Lookup address in address book.
	if (script_result.caller_name.empty() &&
		sys_config->get_ab_lookup_name() && 
		(sys_config->get_ab_override_display() || r->hdr_from.display.empty())) 
	{
		// Send 100 Trying as name lookup might take a while
		resp = r->create_response(R_100_TRYING);
		send_response(resp, 0, tid);
		MEMMAN_DELETE(resp);
		delete resp;
		
		string name = ui->get_name_from_abook(user_config, r->hdr_from.uri);
		if (!name.empty()) {
			r->hdr_from.display_override = name;
			log_file->write_header("t_phone::recvd_invite", 
					LOG_NORMAL, LOG_DEBUG);
			log_file->write_raw(
				"Override display name with address book name:\n");
			log_file->write_raw(name);
			log_file->write_endl();
			log_file->write_footer();
		}
	}	
	
	// Perform the action in the script_result.
	// NOTE: the default action is "continue"
	switch (script_result.action) {
	case t_script_result::ACTION_CONTINUE:
		// Continue with call
		break;
	case t_script_result::ACTION_AUTOANSWER:
		log_file->write_report("Incoming call script action: autoanswer",
			"t_phone::recvd_invite");
		break;
	case t_script_result::ACTION_REJECT:
		log_file->write_report("Incoming call script action: reject",
			"t_phone::recvd_invite");
		resp = r->create_response(R_603_DECLINE, script_result.reason);
		send_response(resp, 0, tid);
		
		// Create a call history record
		call_record.start_call(r, t_call_record::DIR_IN, 
			user_config->get_profile_name());
		call_record.fail_call(resp);
		call_history->add_call_record(call_record);
		
		// Trigger call script
		script_in_call_failed.exec_notify(resp);
		
		MEMMAN_DELETE(resp);
		delete resp;
		return;
		break;
	case t_script_result::ACTION_DND:
		log_file->write_report("Incoming call script action: dnd",
			"t_phone::recvd_invite");
		resp = r->create_response(R_480_TEMP_NOT_AVAILABLE, 
				script_result.reason);
		send_response(resp, 0, tid);
		
		// Create a call history record
		call_record.start_call(r, t_call_record::DIR_IN, 
			user_config->get_profile_name());
		call_record.fail_call(resp);
		call_history->add_call_record(call_record);
		
		// Trigger call script
		script_in_call_failed.exec_notify(resp);
		
		MEMMAN_DELETE(resp);
		delete resp;
		return;
		break;
	case  t_script_result::ACTION_REDIRECT:
		log_file->write_report("Incoming call script action: redirect",
			"t_phone::recvd_invite");
		ui->expand_destination(user_config, 
			script_result.contact, display_url);
		if (display_url.is_valid()) {
			cf_dest.clear();
			cf_dest.push_back(display_url);
			resp = r->create_response(R_302_MOVED_TEMPORARILY);
			resp->hdr_contact.set_contacts(cf_dest);
		} else {
			log_file->write_report("Invalid redirect contact",
				"t_phone::recvd_invite",
				LOG_NORMAL, LOG_WARNING);
			resp = r->create_response(R_500_INTERNAL_SERVER_ERROR); 
		}
		send_response(resp, 0, tid);
		
		// Create a call history record
		call_record.start_call(r, t_call_record::DIR_IN, 
			user_config->get_profile_name());
		call_record.fail_call(resp);
		call_history->add_call_record(call_record);
		
		// Trigger call script
		script_in_call_failed.exec_notify(resp);
		
		MEMMAN_DELETE(resp);
		delete resp;
		return;
		break;
	default:
		log_file->write_report("Error in incoming call script",
			"t_phone::recvd_invite", LOG_NORMAL, LOG_WARNING);
		resp = r->create_response(R_500_INTERNAL_SERVER_ERROR); 
		send_response(resp, 0, tid);
		
		// Create a call history record
		call_record.start_call(r, t_call_record::DIR_IN, 
			user_config->get_profile_name());
		call_record.fail_call(resp);
		call_history->add_call_record(call_record);
		
		// Trigger call script
		script_in_call_failed.exec_notify(resp);
		
		MEMMAN_DELETE(resp);
		delete resp;
		return;
		break;
	}		

	// Call forwarding always
	// NOTE: if a call script returned the autoanswer action, then
	//       call forwarding should be bypassed
	if (pu->service->get_cf_active(CF_ALWAYS, cf_dest) &&
		script_result.action == t_script_result::ACTION_CONTINUE) 
	{
		log_file->write_report("Call redirection unconditional",
			"t_phone::recvd_invite");
		resp = r->create_response(R_302_MOVED_TEMPORARILY);
		resp->hdr_contact.set_contacts(cf_dest);
		send_response(resp, 0, tid);
		
		// Create a call history record
		call_record.start_call(r, t_call_record::DIR_IN, 
			user_config->get_profile_name());
		call_record.fail_call(resp);
		call_history->add_call_record(call_record);
		
		// Trigger call script
		script_in_call_failed.exec_notify(resp);
	
		MEMMAN_DELETE(resp);
		delete resp;
		return;
	}

	// Do not disturb
	// RFC 3261 21.4.18
	// NOTE: if a call script returned the autoanswer action, then
	//       do not disturb should be bypassed
	if (pu->service->is_dnd_active() &&
		script_result.action == t_script_result::ACTION_CONTINUE) 
	{
		log_file->write_report("Do not disturb",
			"t_phone::recvd_invite");
		resp = r->create_response(R_480_TEMP_NOT_AVAILABLE);
		send_response(resp, 0, tid);

		// Create a call history record
		call_record.start_call(r, t_call_record::DIR_IN, 
			user_config->get_profile_name());
		call_record.fail_call(resp);
		call_history->add_call_record(call_record);
		
		// Trigger call script
		script_in_call_failed.exec_notify(resp);
		
		MEMMAN_DELETE(resp);
		delete resp;
		return;
	}
	
	// RFC 3891
	bool auto_answer_replace_call = false;
	if (replace_line >= 0) {
		// This call replaces an existing call. Tear down this existing
		// call. This will clear the active line.
		log_file->write_report("End call due to Replaces header.",
				"t_phone::recvd_initial_invite");
				
		t_rwmutex_writer x(lines_mtx);
		if (lines.at(replace_line)->get_substate() == LSSUB_INCOMING_PROGRESS) {
			ui->cb_stop_call_notification(replace_line);
			lines.at(replace_line)->reject();
		} else {
			lines.at(replace_line)->end_call();
			auto_answer_replace_call = true;
		}
		
		move_line_to_background(replace_line);
	}

	// Auto answer
	if (hunted_line == active_line) {
		// Auto-answer is only applicable to the active line.
		
		t_rwmutex_reader x(lines_mtx);
		if (replace_line >= 0 && auto_answer_replace_call) {
			// RFC 3891
			// This call replaces an existing established call, answer immediate.
			lines.at(active_line)->set_auto_answer(true);
		} else if (pu->service->is_auto_answer_active() ||
			script_result.action == t_script_result::ACTION_AUTOANSWER) 
		{
			// Auto answer
			log_file->write_report("Auto answer",
				"t_phone::recvd_invite");
			lines.at(active_line)->set_auto_answer(true);
		}
	}
	
	t_rwmutex_reader x(lines_mtx);
	// Send INVITE to hunted line
	if (hunted_line >= 0) {
		lines.at(hunted_line)->recvd_invite(pu, r, tid,
			script_result.ringtone);
		return;
	}
	
	// The phone is busy
	// Call forwarding busy
	if (pu->service->get_cf_active(CF_BUSY, cf_dest)) {
		log_file->write_report("Call redirection busy",
			"t_phone::recvd_invite");
		resp = r->create_response(R_302_MOVED_TEMPORARILY);
		resp->hdr_contact.set_contacts(cf_dest);
		send_response(resp, 0, tid);
		
		// Create a call history record
		call_record.start_call(r, t_call_record::DIR_IN, 
			user_config->get_profile_name());
		call_record.fail_call(resp);
		call_history->add_call_record(call_record);
		
		// Trigger call script
		script_in_call_failed.exec_notify(resp);
		
		MEMMAN_DELETE(resp);
		delete resp;
		return;
	}

	// Send busy response
	resp = r->create_response(R_486_BUSY_HERE);
	send_response(resp, 0, tid);
	
	// Create a call history record
	call_record.start_call(r, t_call_record::DIR_IN, 
		user_config->get_profile_name());
	call_record.fail_call(resp);
	call_history->add_call_record(call_record);
	
	// Trigger call script
	script_in_call_failed.exec_notify(resp);
		
	MEMMAN_DELETE(resp);
	delete resp;
}

void t_phone::recvd_re_invite(t_request *r, t_tid tid) {
	t_response *resp;
	list <string> unsupported;
	t_rwmutex_reader x(lines_mtx);

	// RFC 3261 12.2.2
	// A To-header with a tag is a mid-dialog request.
	// Find a line that matches the request
	for (unsigned short i = 0; i < lines.size(); i++) {
		if (lines[i]->match(r)) {
			t_phone_user *pu = lines[i]->get_phone_user();
			assert(pu);
			t_user *user_config = pu->get_user_profile();
			
			// Check if the far end requires any unsupported extensions
			if (!user_config->check_required_ext(r, unsupported))
			{
				// Not all required extensions are supported
				resp = r->create_response(R_420_BAD_EXTENSION);
				resp->hdr_unsupported.set_features(unsupported);
				send_response(resp, 0, tid);
				MEMMAN_DELETE(resp);
				delete resp;
				return;
			}
		
			lines[i]->recvd_invite(pu, r, tid, "");
			return;
		}
	}

	// No dialog matches with the request.
	resp = r->create_response(R_481_TRANSACTION_NOT_EXIST);
	send_response(resp, 0, tid);
	MEMMAN_DELETE(resp);
	delete resp;
}

void t_phone::recvd_ack(t_request *r, t_tid tid) {
	t_response *resp;
	t_rwmutex_reader x(lines_mtx);

	for (unsigned short i = 0; i < lines.size(); i++) {
		if (lines[i]->match(r)) {
			lines[i]->recvd_ack(r, tid);
			return;
		}
	}

	resp = r->create_response(R_481_TRANSACTION_NOT_EXIST);
	send_response(resp, 0, tid);
	MEMMAN_DELETE(resp);
	delete resp;
}

void t_phone::recvd_cancel(t_request *r, t_tid cancel_tid,
		t_tid target_tid)
{
	t_response *resp;
	t_rwmutex_reader x(lines_mtx);

	for (unsigned short i = 0; i < lines.size(); i++) {
		if (lines[i]->match_cancel(r, target_tid)) {
			lines[i]->recvd_cancel(r, cancel_tid, target_tid);
			return;
		}
	}

	resp = r->create_response(R_481_TRANSACTION_NOT_EXIST);
	send_response(resp, 0, cancel_tid);
	MEMMAN_DELETE(resp);
	delete resp;
}

void t_phone::recvd_bye(t_request *r, t_tid tid) {
	t_response *resp;
	list <string> unsupported;
	t_rwmutex_reader x(lines_mtx);

	for (unsigned short i = 0; i < lines.size(); i++) {
		if (lines[i]->match(r)) {
			t_user *user_config = lines[i]->get_user();
			assert(user_config);

			if (!user_config->check_required_ext(r, unsupported))
			{
				// Not all required extensions are supported
				resp = r->create_response(R_420_BAD_EXTENSION);
				resp->hdr_unsupported.set_features(unsupported);
				send_response(resp, 0, tid);
				MEMMAN_DELETE(resp);
				delete resp;
				return;
			}			
		
			lines[i]->recvd_bye(r, tid);
			return;
		}
	}

	resp = r->create_response(R_481_TRANSACTION_NOT_EXIST);
	send_response(resp, 0, tid);
	MEMMAN_DELETE(resp);
	delete resp;
}

void t_phone::recvd_options(t_request *r, t_tid tid) {
	t_response *resp;
	if (r->hdr_to.tag =="") {
		// Out-of-dialog OPTIONS
		t_phone_user *pu = find_phone_user_out_dialog_request(r, tid);
		if (pu) {
			resp = pu->create_options_response(r);
			send_response(resp, 0, tid);
			MEMMAN_DELETE(resp);
			delete resp;
		}		
	} else {
		// In-dialog OPTIONS
		t_line *l = find_line_in_dialog_request(r, tid);
		if (l) {
			l->recvd_options(r, tid);
		}
	}
}

t_phone_user *t_phone::find_phone_user_out_dialog_request(t_request *r, t_tid tid) {
	t_response *resp;
	list <string> unsupported;
	
	// Find out for which user this request is.
	t_phone_user *pu = match_phone_user(r, true);
	if (!pu) {
		resp = r->create_response(R_404_NOT_FOUND);
		send_response(resp, 0, tid);
		MEMMAN_DELETE(resp);
		delete resp;
		return NULL;
	}

	// Check if the far end requires any unsupported extensions
	if (!pu->get_user_profile()->check_required_ext(r, unsupported))
	{
		// Not all required extensions are supported
		resp = r->create_response(R_420_BAD_EXTENSION);
		resp->hdr_unsupported.set_features(unsupported);
		send_response(resp, 0, tid);
		MEMMAN_DELETE(resp);
		delete resp;
		return NULL;
	}
	
	return pu;
}

t_line *t_phone::find_line_in_dialog_request(t_request *r, t_tid tid) {
	t_response *resp;
	list <string> unsupported;
	t_rwmutex_reader x(lines_mtx);
	
	// RFC 3261 12.2.2
	// A To-header with a tag is a mid-dialog request.
	// No dialog matches with the request.
	for (unsigned short i = 0; i < lines.size(); i++) {
		if (lines[i]->match(r)) {
			t_user *user_config = lines[i]->get_user();
			assert(user_config);		
		
			// Check if the far end requires any unsupported extensions
			if (!user_config->check_required_ext(r, unsupported))
			{
				// Not all required extensions are supported
				resp = r->create_response(R_420_BAD_EXTENSION);
				resp->hdr_unsupported.set_features(unsupported);
				send_response(resp, 0, tid);
				MEMMAN_DELETE(resp);
				delete resp;
				return NULL;
			}		

			return lines[i];
		}
	}

	resp = r->create_response(R_481_TRANSACTION_NOT_EXIST);
	send_response(resp, 0, tid);
	MEMMAN_DELETE(resp);
	delete resp;
	return NULL;
}

void t_phone::recvd_register(t_request *r, t_tid tid) {
	// The softphone is not a registrar.
	t_response *resp = r->create_response(R_403_FORBIDDEN);
	send_response(resp, 0, tid);
	MEMMAN_DELETE(resp);
	delete resp;

	// TEST ONLY: code for testing a 423 Interval Too Brief
	/*
	if (r->hdr_contact.contact_list.front().get_expires() < 30) {
		t_response *resp = r->create_response(
					R_423_INTERVAL_TOO_BRIEF);
		resp->hdr_min_expires.set_time(30);
		send_response(resp, 0, tid);
		delete resp;
		return;
	}

	// Code for testing a 200 OK response (register)
	t_response *resp = r->create_response(R_200_OK);
	resp->hdr_contact.set_contacts(r->hdr_contact.contact_list);
	resp->hdr_contact.contact_list.front().set_expires(30);
	resp->hdr_date.set_now();
	send_response(resp, 0, tid);
	delete resp;

	// Code for testing 200 OK response (de-register)
	t_response *resp = r->create_response(R_200_OK);
	send_response(resp, 0, tid);
	delete resp;

	// Code for testing 200 OK response (query)
	t_response *resp = r->create_response(R_200_OK);
	t_contact_param contact;
	contact.uri.set_url("sip:aap@xs4all.nl");
	resp->hdr_contact.add_contact(contact);
	contact.uri.set_url("sip:noot@xs4all.nl");
	resp->hdr_contact.add_contact(contact);
	send_response(resp, 0, tid);
	delete resp;

	// Code for testing a 401 response (register)
	if (r->hdr_authorization.is_populated() &&
	    r->hdr_authorization.credentials_list.front().digest_response.
	    	nonce == "0123456789abcdef")
	{
		t_response *resp = r->create_response(R_200_OK);
		resp->hdr_contact.set_contacts(r->hdr_contact.contact_list);
		resp->hdr_contact.contact_list.front().set_expires(30);
		resp->hdr_date.set_now();
		send_response(resp, 0, tid);
		delete resp;
	} else {
		t_response *resp = r->create_response(R_401_UNAUTHORIZED);
		t_challenge c;
		c.auth_scheme = AUTH_DIGEST;
		c.digest_challenge.realm = "mtel.nl";
		if (r->hdr_authorization.is_populated()) {
			c.digest_challenge.nonce = "0123456789abcdef";
			c.digest_challenge.stale = true;
		} else {
			c.digest_challenge.nonce = "aaaaaa0123456789";
		}
		c.digest_challenge.opaque = "secret";
		c.digest_challenge.algorithm = ALG_MD5;
		// c.digest_challenge.qop_options.push_back(QOP_AUTH);
		// c.digest_challenge.qop_options.push_back(QOP_AUTH_INT);
		resp->hdr_www_authenticate.set_challenge(c);
		send_response(resp, 0, tid);
	}
	*/
}

void t_phone::recvd_prack(t_request *r, t_tid tid) {
	t_response *resp;
	t_rwmutex_reader x(lines_mtx);

	for (unsigned short i = 0; i < lines.size(); i++) {
		if (lines[i]->match(r)) {
			lines[i]->recvd_prack(r, tid);
			return;
		}
	}

	resp = r->create_response(R_481_TRANSACTION_NOT_EXIST);
	send_response(resp, 0, tid);
	MEMMAN_DELETE(resp);
	delete resp;
}

void t_phone::recvd_subscribe(t_request *r, t_tid tid) {
	t_response *resp;

	if (r->hdr_event.event_type != SIP_EVENT_REFER) {
		// Non-supported event type
		resp = r->create_response(R_489_BAD_EVENT);
		resp->hdr_allow_events.add_event_type(SIP_EVENT_REFER);
		send_response(resp, 0 ,tid);
		MEMMAN_DELETE(resp);
		delete resp;
		return;
	}

	t_rwmutex_reader x(lines_mtx);
	for (unsigned short i = 0; i < lines.size(); i++) {
		if (lines[i]->match(r)) {
			lines[i]->recvd_subscribe(r, tid);
			return;
		}
	}

	if (r->hdr_to.tag == "") {
		// A REFER outside a dialog is not allowed by Twinkle
		if (r->hdr_event.event_type == SIP_EVENT_REFER) {
			// RFC 3515 2.4.4
			resp = r->create_response(R_403_FORBIDDEN);
			send_response(resp, 0 ,tid);
			MEMMAN_DELETE(resp);
			delete resp;
			return;
		}
	}

	resp = r->create_response(R_481_TRANSACTION_NOT_EXIST);
	send_response(resp, 0, tid);
	MEMMAN_DELETE(resp);
	delete resp;
}

void t_phone::recvd_notify(t_request *r, t_tid tid) {
	t_response *resp;
	t_phone_user *pu;
	
	// Check support for the notified event
	if (!SIP_EVENT_SUPPORTED(r->hdr_event.event_type)) {
		// Non-supported event type
		resp = r->create_response(R_489_BAD_EVENT);
		ADD_SUPPORTED_SIP_EVENTS(resp->hdr_allow_events);
		send_response(resp, 0 ,tid);
		MEMMAN_DELETE(resp);
		delete resp;
		return;
	}
	
	// MWI or presence notification
	if (r->hdr_event.event_type == SIP_EVENT_MSG_SUMMARY ||
	    r->hdr_event.event_type == SIP_EVENT_PRESENCE)
	{
		pu = match_phone_user(r, true);
		if (pu) {
			pu->recvd_notify(r, tid);
		} else {
			resp = r->create_response(R_481_TRANSACTION_NOT_EXIST);
			send_response(resp, 0 ,tid);
			MEMMAN_DELETE(resp);
			delete resp;
		}
		
		return;
	}

	// REFER notification
	t_rwmutex_future_writer x(lines_mtx);
	for (unsigned short i = 0; i < lines.size(); i++) {
		if (lines[i]->match(r)) {
			lines[i]->recvd_notify(r, tid);
			if (lines[i]->get_refer_state() == REFST_NULL) {
				// Refer subscription has finished.
				log_file->write_report("Refer subscription terminated.",
					"t_phone::recvd_notify");

				if (lines[i]->get_substate() == LSSUB_RELEASING ||
				    lines[i]->get_substate() == LSSUB_IDLE)
				{
					// The line is (being) cleared already. So this
					// NOTIFY signals the end of the refer subscription
					// attached to this line.
					cleanup_dead_lines();
					return;
				} else if (lines[i]->is_refer_succeeded()) {
					log_file->write_report(
						"Refer succeeded. End call with referee,",
						"t_phone::recvd_notify");
					lines[i]->end_call();
				} else {
					log_file->write_report("Refer failed.",
						"t_phone::recvd_notify");

					t_user *user_config = lines[i]->get_user();
					assert(user_config);
					if (user_config->get_referrer_hold() &&
					    lines[i]->get_is_on_hold())
					{
						// Retrieve the call if the line is active.
						if (i == active_line) {
							log_file->write_report(
								"Retrieve call with referee.",
								"t_phone::recvd_notify");
							lines[i]->retrieve();
						}
					}
				}
			}
			return;
		}
	}

	if (r->hdr_to.tag == "") {
		// NOTIFY outside a dialog is not allowed.
		resp = r->create_response(R_403_FORBIDDEN);
		send_response(resp, 0 ,tid);
		MEMMAN_DELETE(resp);
		delete resp;
		return;
	}

	resp = r->create_response(R_481_TRANSACTION_NOT_EXIST);
	send_response(resp, 0, tid);
	MEMMAN_DELETE(resp);
	delete resp;
}

void t_phone::recvd_refer(t_request *r, t_tid tid) {
	t_response *resp;
	t_rwmutex_reader x(lines_mtx);

	for (unsigned short i = 0; i < lines.size(); i++) {
		if (lines[i]->match(r)) {
			t_phone_user *pu = lines[i]->get_phone_user();
			assert(pu);
			t_user *user_config = pu->get_user_profile();
			
			list <string> unsupported;
			if (!user_config->check_required_ext(r, unsupported))
			{
				// Not all required extensions are supported
				resp = r->create_response(R_420_BAD_EXTENSION);
				resp->hdr_unsupported.set_features(unsupported);
				send_response(resp, 0, tid);
				MEMMAN_DELETE(resp);
				delete resp;
				return;
			}
		
			// Reject if a 3-way call is established.
			if (is_3way) {
				log_file->write_report("3-way call active. Reject REFER.",
					"t_phone::recvd_refer");
				resp = r->create_response(R_603_DECLINE);
				send_response(resp, 0, tid);
				MEMMAN_DELETE(resp);
				delete resp;
				return;
			}
			
			// Reject if the line is on-hold.
			if (is_3way || lines[i]->get_is_on_hold()) {
				log_file->write_report("Line is on-hold. Reject REFER.",
					"t_phone::recvd_refer");
				resp = r->create_response(R_603_DECLINE);
				send_response(resp, 0, tid);
				MEMMAN_DELETE(resp);
				delete resp;
				return;
			}

			// Check if a refer is already in progress
			if (i == LINENO_REFERRER ||
			    lines[LINENO_REFERRER]->get_state() != LS_IDLE ||
			    incoming_refer_data != NULL)
			{
				log_file->write_report(
					"A REFER is still in progress. Reject REFER.",
					"t_phone::recvd_refer");
				resp = r->create_response(R_603_DECLINE);
				send_response(resp, 0, tid);
				MEMMAN_DELETE(resp);
				delete resp;
				return;
			}

			if (!lines[i]->recvd_refer(r, tid)) {
				// Refer has been rejected.
				log_file->write_report("Incoming REFER rejected.",
					"t_phone::recvd_refer");
				return;
			}
			
			// Make sure the line stays seized if the far-end ends the
			// call, so a line will be available if the user gives permission
			// for the call transfer.
			lines[i]->set_keep_seized(true);
			incoming_refer_data = new t_transfer_data(r, i, 
					lines[i]->get_hide_user(), pu);
			MEMMAN_NEW(incoming_refer_data);
			return;
		}
	}
	
	if (r->hdr_to.tag == "") {
		// Twinkle does not allow a REFER outside a dialog.
		resp = r->create_response(R_403_FORBIDDEN);
		send_response(resp, 0 ,tid);
		MEMMAN_DELETE(resp);
		delete resp;
		return;
	}

	resp = r->create_response(R_481_TRANSACTION_NOT_EXIST);
	send_response(resp, 0, tid);
	MEMMAN_DELETE(resp);
	delete resp;
}	
	
void t_phone::recvd_refer_permission(bool permission) {
	if (!incoming_refer_data) {
		// This should not happen
		log_file->write_report("Incoming REFER data is gone.",
			"t_phone::recvd_refer_permission",
			LOG_NORMAL, LOG_WARNING);
		return;
	}

	unsigned short i = incoming_refer_data->get_lineno();
	t_request *r = incoming_refer_data->get_refer_request();
	bool hide_user = incoming_refer_data->get_hide_user();
	t_phone_user *pu = incoming_refer_data->get_phone_user();
	t_user *user_config = pu->get_user_profile();
			
	t_rwmutex_future_writer x(lines_mtx);
	lines[i]->recvd_refer_permission(permission, r);
	
	if (!permission) {
		log_file->write_report("Incoming REFER rejected.",
			"t_phone::recvd_refer_permission");
		lines[i]->set_keep_seized(false);
		move_releasing_lines_to_background();
		
		MEMMAN_DELETE(incoming_refer_data);
		delete incoming_refer_data;
		incoming_refer_data = NULL;
		return;
	} else {
		log_file->write_report("Incoming REFER allowed.",
			"t_phone::recvd_refer_permission");
	}
	
	if (lines[i]->get_substate() == LSSUB_ESTABLISHED) {
		// Put line on-hold and place it in the referrer line
		log_file->write_report(
			"Hold call before calling the refer-target.",
			"t_phone::recvd_refer_permission");

		if (user_config->get_referee_hold()) {
			lines[i]->hold();
		} else {
			// The user profile indicates that the line should
			// not be put on-hold, i.e. do not send re-INVITE.
			// So only stop RTP.
			lines[i]->hold(true);
		}
	}

	// Move the original line to the REFERRER line (the line may be idle
	// already).
	t_line *l = lines[i];
	lines[i] = lines[LINENO_REFERRER];
	lines[i]->line_number = i;
	lines[LINENO_REFERRER] = l;
	lines[LINENO_REFERRER]->line_number = LINENO_REFERRER;
	lines[LINENO_REFERRER]->set_keep_seized(false);

	ui->cb_line_state_changed();

	// Setup call to the Refer-To destination
	log_file->write_report("Call refer-target.",
		"t_phone::recvd_refer_permission");
	
	t_hdr_replaces hdr_replaces;
	t_hdr_require hdr_require;
	
	// Analyze headers in Refer-To URI.
	// For an attended call transfer the Refer-To URI
	// will contain a Replaces header and possibly a Require
	// header. Other headers are ignored for now.
	// See draft-ietf-sipping-cc-transfer-07 7.3
	if (!r->hdr_refer_to.uri.get_headers().empty()) {
		try {
			list<string> parse_errors;
			t_sip_message *m = t_parser::parse_headers(
					r->hdr_refer_to.uri.get_headers(),
					parse_errors);
			hdr_replaces = m->hdr_replaces;
			hdr_require = m->hdr_require;
			MEMMAN_DELETE(m);
			delete m;
		} catch (int) {
			log_file->write_header("t_phone::recvd_refer_permission",
					LOG_NORMAL, LOG_WARNING);
			log_file->write_raw("Cannot parse headers in Refer-To URI\n");
			log_file->write_raw(r->hdr_refer_to.uri.encode());
			log_file->write_endl();
			log_file->write_footer();
		}
	}
	
	ui->cb_call_referred(user_config, i, r);
	
	lines[i]->invite(pu, 
		r->hdr_refer_to.uri.copy_without_headers(),
		r->hdr_refer_to.display, "", r->hdr_referred_by, 
		hdr_replaces, hdr_require, t_hdr_request_disposition(),
		hide_user);
	lines[i]->open_dialog->is_referred_call = true;
	
	MEMMAN_DELETE(incoming_refer_data);
	delete incoming_refer_data;
	incoming_refer_data = NULL;
	
	return;
}

void t_phone::recvd_info(t_request *r, t_tid tid) {
	t_response *resp;
	list <string> unsupported;
	t_rwmutex_reader x(lines_mtx);

	for (unsigned short i = 0; i < lines.size(); i++) {
		if (lines[i]->match(r)) {
			t_user *user_config = lines[i]->get_user();
			assert(user_config);

			if (!user_config->check_required_ext(r, unsupported))
			{
				// Not all required extensions are supported
				resp = r->create_response(R_420_BAD_EXTENSION);
				resp->hdr_unsupported.set_features(unsupported);
				send_response(resp, 0, tid);
				MEMMAN_DELETE(resp);
				delete resp;
				return;
			}			
		
			lines[i]->recvd_info(r, tid);
			return;
		}
	}

	resp = r->create_response(R_481_TRANSACTION_NOT_EXIST);
	send_response(resp, 0, tid);
	MEMMAN_DELETE(resp);
	delete resp;
}

void t_phone::recvd_message(t_request *r, t_tid tid) {
	if (r->hdr_to.tag =="") {
		// Out-of-dialog MESSAGE
		t_phone_user *pu = find_phone_user_out_dialog_request(r, tid);
		if (pu) {
			pu->recvd_message(r, tid);
		}		
	} else {
		// In-dialog MESSAGE
		t_line *l = find_line_in_dialog_request(r, tid);
		if (l) {
			l->recvd_message(r, tid);
		}
	}
}

void t_phone::post_process_request(t_request *r, t_tid cancel_tid, t_tid target_tid) {
	cleanup_dead_lines();
	move_releasing_lines_to_background();
	cleanup_3way();
}


void t_phone::failure(t_failure failure, t_tid tid) {
	// TODO
}

void t_phone::recvd_stun_resp(StunMessage *r, t_tuid tuid, t_tid tid) {
	t_rwmutex_reader x(lines_mtx);

	for (unsigned short i = 0; i < lines.size(); i++) {
		if (lines[i]->match(r, tuid)) {
			lines[i]->recvd_stun_resp(r, tuid, tid);
			return;
		}
	}

	// out-of-dialog STUN responses
	handle_response_out_of_dialog(r, tuid);
}

void t_phone::handle_event_timeout(t_event_timeout *e) {
	t_timer			*t = e->get_timer();
	t_tmr_phone		*tmr_phone;
	t_tmr_line		*tmr_line;
	t_tmr_subscribe		*tmr_subscribe;
	t_tmr_publish		*tmr_publish;
	t_object_id		line_id;
	
	switch (t->get_type()) {
	case TMR_PHONE:
		tmr_phone = dynamic_cast<t_tmr_phone *>(t);
		timeout(tmr_phone->get_phone_timer(), tmr_phone->get_object_id());
		break;
	case TMR_LINE:
		tmr_line = dynamic_cast<t_tmr_line *>(t);
		line_timeout(tmr_line->get_line_id(), tmr_line->get_line_timer(), 
			tmr_line->get_dialog_id());
		break;
	case TMR_SUBSCRIBE:
		tmr_subscribe = dynamic_cast<t_tmr_subscribe *>(t);
		line_id = tmr_subscribe->get_line_id();
		if (line_id == 0) {
			subscription_timeout(tmr_subscribe->get_subscribe_timer(), 
				tmr_subscribe->get_object_id());
		} else {	
			line_timeout_sub(line_id, tmr_subscribe->get_subscribe_timer(),
				tmr_subscribe->get_dialog_id(),
				tmr_subscribe->get_sub_event_type(), 
				tmr_subscribe->get_sub_event_id());
		}
		break;
	case TMR_PUBLISH:
		tmr_publish = dynamic_cast<t_tmr_publish *>(t);
		publication_timeout(tmr_publish->get_publish_timer(), 
			tmr_publish->get_object_id());
		break;
	default:
		assert(false);
		break;
	}
}

void t_phone::line_timeout(t_object_id id, t_line_timer timer, t_object_id did) {
	// If there is no line with id anymore, then the timer expires
	// silently.
	t_line *line = get_line_by_id(id);
	if (line) {
		line->timeout(timer, did);
	}
}

void t_phone::line_timeout_sub(t_object_id id, t_subscribe_timer timer, t_object_id did,
		const string &event_type, const string &event_id)
{
	// If there is no line with id anymore, then the timer expires
	// silently.
	t_line *line = get_line_by_id(id);
	if (line) {
		line->timeout_sub(timer, did, event_type, event_id);
	}
}

void t_phone::subscription_timeout(t_subscribe_timer timer, t_object_id id_timer)
{
	t_rwmutex_reader x(phone_users_mtx);

	for (list<t_phone_user *>::iterator i = phone_users.begin();
		     i != phone_users.end(); i++)
	{
		if ((*i)->match_subscribe_timer(timer, id_timer)) {
			(*i)->timeout_sub(timer, id_timer);
		}
	}
}

void t_phone::publication_timeout(t_publish_timer timer, t_object_id id_timer) {
	t_rwmutex_reader x(phone_users_mtx);

	for (list<t_phone_user *>::iterator i = phone_users.begin();
		     i != phone_users.end(); i++)
	{
		if ((*i)->match_publish_timer(timer, id_timer)) {
			(*i)->timeout_publish(timer, id_timer);
		}
	}
}

void t_phone::timeout(t_phone_timer timer, unsigned short id_timer) {
	t_rwmutex_reader x(phone_users_mtx);

	switch (timer) {
	case PTMR_REGISTRATION:
		for (list<t_phone_user *>::iterator i = phone_users.begin();
		     i != phone_users.end(); ++i)
		{
			if ((*i)->id_registration == id_timer) {
				(*i)->timeout(timer);
			}
		}
		break;
	case PTMR_NAT_KEEPALIVE:
		for (list<t_phone_user *>::iterator i = phone_users.begin();
		     i != phone_users.end(); ++i)
		{
			if ((*i)->id_nat_keepalive == id_timer) {
				(*i)->timeout(timer);
			}
		}
		break;
	case PTMR_TCP_PING:
		for (list<t_phone_user *>::iterator i = phone_users.begin();
		     i != phone_users.end(); ++i)
		{
			if ((*i)->id_tcp_ping == id_timer) {
				(*i)->timeout(timer);
			}
		}
		break;
	default:
		assert(false);
	}
}

void t_phone::handle_broken_connection(t_event_broken_connection *e) {
	t_rwmutex_reader x(phone_users_mtx);

	// Find the phone user that was associated with the connection.
	// This phone user has to handle the event.
	t_phone_user *pu = find_phone_user(e->get_user_uri());
	if (pu) {
		pu->handle_broken_connection();
	} else {
		log_file->write_header("t_phone::handle_broken_connection", LOG_NORMAL, LOG_WARNING);
		log_file->write_raw("Cannot find active phone user ");
		log_file->write_raw(e->get_user_uri().encode());
		log_file->write_endl();
		log_file->write_footer();
	}
}


///////////
// Public
///////////

t_phone::t_phone() : t_transaction_layer(), lines(NUM_CALL_LINES) {
	is_active = true;
	active_line = 0;

	// Create phone lines
	for (unsigned short i = 0; i < NUM_CALL_LINES; i++) {
		lines[i] = new t_line(this, i);
		MEMMAN_NEW(lines[i]);
	}

	// Initialize 3-way conference data
	is_3way = false;
	line1_3way = NULL;
	line2_3way = NULL;
	
	incoming_refer_data = NULL;
	
	struct timeval t;
	gettimeofday(&t, NULL);
	startup_time = t.tv_sec;
	
	// NOTE: The RTP ports for the lines are initialized after the
	// system settings have been read.
}

t_phone::~t_phone() {
	// Delete phone lines
	log_file->write_header("t_phone::~t_phone");
	log_file->write_raw("Number of lines to cleanup: ");
	log_file->write_raw(lines.size());
	log_file->write_endl();
	log_file->write_footer();
	
	if (incoming_refer_data) {
		MEMMAN_DELETE(incoming_refer_data);
		delete incoming_refer_data;
	}
	
	for (unsigned short i = 0; i < lines.size(); i++) {
		MEMMAN_DELETE(lines[i]);
		delete lines[i];
	}
	
	t_rwmutex_writer x(phone_users_mtx);
	// Delete all phone users
	for (list<t_phone_user *>::iterator i = phone_users.begin();
	     i != phone_users.end(); i++)
	{
		MEMMAN_DELETE(*i);
		delete *i;
	}
}

void t_phone::pub_invite(t_user *user, 
		const t_url &to_uri, const string &to_display,
		const string &subject, bool anonymous)
{
	t_rwmutex_reader x(phone_users_mtx);
	
	t_phone_user *pu = find_phone_user(user->get_profile_name());
	if (pu) {
		invite(pu, to_uri, to_display, subject, false, anonymous);
	} else {
		log_file->write_header("t_phone::pub_invite", LOG_NORMAL, LOG_WARNING);
		log_file->write_raw("User profile not active: ");
		log_file->write_raw(user->get_profile_name());
		log_file->write_footer();
	}
}

void t_phone::pub_answer(void) {
	lock();
	answer();
	unlock();
}

void t_phone::pub_reject(void) {
	lock();
	reject();
	unlock();
}

void t_phone::pub_reject(unsigned short line) {
	lock();
	reject(line);
	unlock();
}

void t_phone::pub_redirect(const list<t_display_url> &destinations, int code, string reason)
{
	lock();
	redirect(destinations, code, reason);
	unlock();
}

void t_phone::pub_end_call(void) {
	lock();
	end_call();
	unlock();
}

void t_phone::pub_registration(t_user *user,
		t_register_type register_type,
		unsigned long expires)
{
	t_rwmutex_reader x(phone_users_mtx);
	
	t_phone_user *pu = find_phone_user(user->get_profile_name());
	if (pu) {
		registration(pu, register_type, expires);
	} else {
		log_file->write_header("t_phone::pub_registration", LOG_NORMAL, LOG_WARNING);
		log_file->write_raw("User profile not active: ");
		log_file->write_raw(user->get_profile_name());
		log_file->write_footer();
	}
}

void t_phone::pub_options(t_user *user,
		const t_url &to_uri, const string &to_display) 
{
	t_rwmutex_reader x(phone_users_mtx);
	
	t_phone_user *pu = find_phone_user(user->get_profile_name());
	if (pu) {
		options(pu, to_uri, to_display);
	} else {
		log_file->write_header("t_phone::pub_options", LOG_NORMAL, LOG_WARNING);
		log_file->write_raw("User profile not active: ");
		log_file->write_raw(user->get_profile_name());
		log_file->write_footer();
	}
}

void t_phone::pub_options(void) {
	lock();
	options();
	unlock();
}

bool t_phone::pub_hold(void) {
	lock();
	bool retval = hold();
	unlock();
	return retval;
}

void t_phone::pub_retrieve(void) {
	lock();
	retrieve();
	unlock();
}

void t_phone::pub_refer(const t_url &uri, const string &display) {
	lock();
	refer(uri, display);
	unlock();
}

void t_phone::pub_setup_consultation_call(const t_url &uri, const string &display) {
	lock();
	setup_consultation_call(uri, display);
	unlock();
}

void t_phone::pub_refer(unsigned short lineno_from, unsigned short lineno_to) {
	lock();
	refer(lineno_from, lineno_to);
	unlock();
}

void t_phone::mute(bool enable) {
	lock();

	// In a 3-way call, both lines must be muted
	if (is_3way && (
	    active_line == line1_3way->get_line_number() ||
	    active_line == line2_3way->get_line_number()))
	{
		line1_3way->mute(enable);
		line2_3way->mute(enable);
	}
	else
	{
		t_rwmutex_reader x(lines_mtx);
		lines[active_line]->mute(enable);
	}

	unlock();
}

void t_phone::pub_activate_line(unsigned short l) {
	lock();
	activate_line(l);
	unlock();
}

void t_phone::pub_send_dtmf(char digit, bool inband, bool info) {
	lock();
	send_dtmf(digit, inband, info);
	unlock();
}

bool t_phone::pub_seize(void) {
	bool retval;
	t_rwmutex_reader x(lines_mtx);
	
	retval = lines[active_line]->seize();
	
	return retval;
}

bool t_phone::pub_seize(unsigned short line) {
	assert(line < NUM_USER_LINES);
	bool retval;
	t_rwmutex_reader x(lines_mtx);
	
	retval = lines[line]->seize();
	
	return retval;
}

void t_phone::pub_unseize(void) {
	t_rwmutex_reader x(lines_mtx);
	lines[active_line]->unseize();
}

void t_phone::pub_unseize(unsigned short line) {
	assert(line < NUM_USER_LINES);
	
	t_rwmutex_reader x(lines_mtx);
	lines[line]->unseize();
}

void t_phone::pub_confirm_zrtp_sas(unsigned short line) {
	assert(line < NUM_USER_LINES);

	t_rwmutex_reader x(lines_mtx);
	lines[line]->confirm_zrtp_sas();
}

void t_phone::pub_confirm_zrtp_sas(void) {
	t_rwmutex_reader x(lines_mtx);
	lines[active_line]->confirm_zrtp_sas();
}

void t_phone::pub_reset_zrtp_sas_confirmation(unsigned short line) {
	assert(line < NUM_USER_LINES);
	t_rwmutex_reader x(lines_mtx);
	lines[line]->reset_zrtp_sas_confirmation();
}

void t_phone::pub_reset_zrtp_sas_confirmation(void) {
	t_rwmutex_reader x(lines_mtx);
	lines[active_line]->reset_zrtp_sas_confirmation();
}

void t_phone::pub_enable_zrtp(void) {
	t_rwmutex_reader x(lines_mtx);
	lines[active_line]->enable_zrtp();
}

void t_phone::pub_zrtp_request_go_clear(void) {
	t_rwmutex_reader x(lines_mtx);
	lines[active_line]->zrtp_request_go_clear();
}

void t_phone::pub_zrtp_go_clear_ok(unsigned short line) {
	assert(line < NUM_USER_LINES);
	t_rwmutex_reader x(lines_mtx);
	lines[line]->zrtp_go_clear_ok();
}

void t_phone::pub_subscribe_mwi(t_user *user) {
	t_rwmutex_reader x(phone_users_mtx);
	
	t_phone_user *pu = find_phone_user(user->get_profile_name());
	if (pu) {
		pu->subscribe_mwi();
	} else {
		log_file->write_header("t_phone::pub_subscribe_mwi", LOG_NORMAL, LOG_WARNING);
		log_file->write_raw("User profile not active: ");
		log_file->write_raw(user->get_profile_name());
		log_file->write_footer();
	}
}

void t_phone::pub_unsubscribe_mwi(t_user *user) {
	t_rwmutex_reader x(phone_users_mtx);
	
	t_phone_user *pu = find_phone_user(user->get_profile_name());
	if (pu) {
		pu->unsubscribe_mwi();
	} else {
		log_file->write_header("t_phone::pub_unsubscribe_mwi", LOG_NORMAL, LOG_WARNING);
		log_file->write_raw("User profile not active: ");
		log_file->write_raw(user->get_profile_name());
		log_file->write_footer();
	}
}

void t_phone::pub_subscribe_presence(t_user *user) {
	t_rwmutex_reader x(phone_users_mtx);
	
	t_phone_user *pu = find_phone_user(user->get_profile_name());
	if (pu) {
		pu->subscribe_presence();
	} else {
		log_file->write_header("t_phone::pub_subscribe_presence", LOG_NORMAL, LOG_WARNING);
		log_file->write_raw("User profile not active: ");
		log_file->write_raw(user->get_profile_name());
		log_file->write_footer();
	}
}

void t_phone::pub_unsubscribe_presence(t_user *user) {
	t_rwmutex_reader x(phone_users_mtx);
	
	t_phone_user *pu = find_phone_user(user->get_profile_name());
	if (pu) {
		pu->unsubscribe_presence();
	} else {
		log_file->write_header("t_phone::pub_unsubscribe_presence", LOG_NORMAL, LOG_WARNING);
		log_file->write_raw("User profile not active: ");
		log_file->write_raw(user->get_profile_name());
		log_file->write_footer();
	}
}

void t_phone::pub_publish_presence(t_user *user, t_presence_state::t_basic_state basic_state) {
	t_rwmutex_reader x(phone_users_mtx);
	
	t_phone_user *pu = find_phone_user(user->get_profile_name());
	if (pu) {
		pu->publish_presence(basic_state);
	} else {
		log_file->write_header("t_phone::pub_publish_presence", LOG_NORMAL, LOG_WARNING);
		log_file->write_raw("User profile not active: ");
		log_file->write_raw(user->get_profile_name());
		log_file->write_footer();
	}
}

void t_phone::pub_unpublish_presence(t_user *user) {
	t_rwmutex_reader x(phone_users_mtx);
	
	t_phone_user *pu = find_phone_user(user->get_profile_name());
	if (pu) {
		pu->unpublish_presence();
	} else {
		log_file->write_header("t_phone::pub_publish_presence", LOG_NORMAL, LOG_WARNING);
		log_file->write_raw("User profile not active: ");
		log_file->write_raw(user->get_profile_name());
		log_file->write_footer();
	}
}

bool t_phone::pub_send_message(t_user *user, const t_url &to_uri, const string &to_display,
		const t_msg &msg)
{
	bool retval = true;
	
	t_rwmutex_reader x(phone_users_mtx);
	
	t_phone_user *pu = find_phone_user(user->get_profile_name());
	if (pu) {
		retval = pu->send_message(to_uri, to_display, msg);
	} else {
		log_file->write_header("t_phone::pub_send_message", LOG_NORMAL, LOG_WARNING);
		log_file->write_raw("User profile not active: ");
		log_file->write_raw(user->get_profile_name());
		log_file->write_endl();
		log_file->write_footer();
		
		retval = false;
	}
	
	return retval;
}

bool t_phone::pub_send_im_iscomposing(t_user *user, const t_url &to_uri, const string &to_display,
			const string &state, time_t refresh)
{
	bool retval = true;
	
	t_rwmutex_reader x(phone_users_mtx);
	
	t_phone_user *pu = find_phone_user(user->get_profile_name());
	if (pu) {
		retval = pu->send_im_iscomposing(to_uri, to_display, state, refresh);
	} else {
		log_file->write_header("t_phone::pub_send_im_iscomposing", LOG_NORMAL, LOG_WARNING);
		log_file->write_raw("User profile not active: ");
		log_file->write_raw(user->get_profile_name());
		log_file->write_endl();
		log_file->write_footer();
		
		retval = false;
	}
	
	return retval;
}

t_phone_state t_phone::get_state(void) const {
	t_rwmutex_reader x(lines_mtx);
	for (unsigned short i = 0; i < NUM_USER_LINES; i++) {
		if (lines[i]->get_state() == LS_IDLE) {

			return PS_IDLE;
		}
	}

	// All lines are busy, so the phone is busy.
	return PS_BUSY;
}

bool t_phone::all_lines_idle(void) const {
	t_rwmutex_reader x(lines_mtx);
	for (unsigned short i = 0; i < NUM_USER_LINES; i++) {
		if (lines[i]->get_substate() != LSSUB_IDLE) {

			return false;
		}
	}
	
	// All lines are idle
	return true;
}

bool t_phone::get_idle_line(unsigned short &lineno) const {
	t_rwmutex_reader x(lines_mtx);
	
	bool found_idle_line = false;
	for (unsigned short i = 0; i < NUM_USER_LINES; i++) {
		if (lines[i]->get_substate() == LSSUB_IDLE) {
			lineno = i;
			found_idle_line = true;
			break;
		}
	}
	
	return found_idle_line;
}

void t_phone::set_active_line(unsigned short l) {
	t_rwmutex_reader x(lines_mtx);
	assert (l < NUM_USER_LINES);
	active_line = l;
}

unsigned short t_phone::get_active_line(void) const {
	return active_line;
}

t_line *t_phone::get_line_by_id(t_object_id id) const {
	t_rwmutex_reader x(lines_mtx);
	for (size_t i = 0; i < lines.size(); i++) {
		if (lines[i]->get_object_id() == id) {
			return lines[i];
		}
	}
	
	return NULL;
}

t_line *t_phone::get_line(unsigned short lineno) const {
	assert(lineno < lines.size());
	return lines[lineno];
}

bool t_phone::authorize(t_user *user, t_request *r, t_response *resp) 
{
	bool result = false;
	
	t_rwmutex_reader x(phone_users_mtx);
	t_phone_user *pu = find_phone_user(user->get_profile_name());
	if (pu) result = pu->authorize(r, resp);
	
	return result;
}

void t_phone::remove_cached_credentials(t_user *user, const string &realm) {
	t_rwmutex_reader x(phone_users_mtx);
	t_phone_user *pu = find_phone_user(user->get_profile_name());
	if (pu) pu->remove_cached_credentials(realm);
}

bool t_phone::get_is_registered(t_user *user) {
	bool result = false;
	
	t_rwmutex_reader x(phone_users_mtx);
	t_phone_user *pu = find_phone_user(user->get_profile_name());
	if (pu) result = pu->get_is_registered();
	
	return result;
}

bool t_phone::get_last_reg_failed(t_user *user) {
	bool result = false;
	
	t_rwmutex_reader x(phone_users_mtx);
	t_phone_user *pu = find_phone_user(user->get_profile_name());
	if (pu) result = pu->get_last_reg_failed();
	
	return result;
}

t_line_state t_phone::get_line_state(unsigned short lineno) const {
	assert(lineno < lines.size());

	t_rwmutex_reader x(lines_mtx);
	t_line_state s = get_line(lineno)->get_state();
	return s;
}

t_line_substate t_phone::get_line_substate(unsigned short lineno) const {
	assert(lineno < lines.size());

	t_rwmutex_reader x(lines_mtx);
	t_line_substate s = get_line(lineno)->get_substate();
	return s;
}

bool t_phone::is_line_on_hold(unsigned short lineno) const {
	assert(lineno < lines.size());

	t_rwmutex_reader x(lines_mtx);
	bool b = get_line(lineno)->get_is_on_hold();
	return b;
}

bool t_phone::is_line_muted(unsigned short lineno) const {
	assert(lineno < lines.size());

	t_rwmutex_reader x(lines_mtx);
	bool b = get_line(lineno)->get_is_muted();
	return b;
}

bool t_phone::is_line_transfer_consult(unsigned short lineno,
		unsigned short &transfer_from_line) const 
{
	assert(lineno < lines.size());
	
	t_rwmutex_reader x(lines_mtx);
	bool b = get_line(lineno)->get_is_transfer_consult(transfer_from_line);
	return b;	
}

bool t_phone::line_to_be_transferred(unsigned short lineno, 
		unsigned short &transfer_to_line) const
{
	assert(lineno < lines.size());
	
	t_rwmutex_reader x(lines_mtx);
	bool b = get_line(lineno)->get_to_be_transferred(transfer_to_line);
	return b;
}

bool t_phone::is_line_encrypted(unsigned short lineno) const {
	assert(lineno < lines.size());

	t_rwmutex_reader x(lines_mtx);
	bool b = get_line(lineno)->get_is_encrypted();
	return b;
}

bool t_phone::is_line_auto_answered(unsigned short lineno) const {
	assert(lineno < lines.size());

	t_rwmutex_reader x(lines_mtx);
	bool b = get_line(lineno)->get_auto_answer();
	return b;
}

t_refer_state t_phone::get_line_refer_state(unsigned short lineno) const {
	assert(lineno < lines.size());

	t_rwmutex_reader x(lines_mtx);
	t_refer_state s = get_line(lineno)->get_refer_state();
	return s;
}

t_user *t_phone::get_line_user(unsigned short lineno) {
	assert(lineno < lines.size());
	t_rwmutex_reader x(lines_mtx);
	t_user *user = get_line(lineno)->get_user();
	return user;
}

bool t_phone::has_line_media(unsigned short lineno) const {
	assert(lineno < lines.size());
	
	t_rwmutex_reader x(lines_mtx);
	bool b = get_line(lineno)->has_media();
	return b;
}

bool t_phone::is_mwi_subscribed(t_user *user) const {
	bool result = false;
	
	t_rwmutex_reader x(phone_users_mtx);
	t_phone_user *pu = find_phone_user(user->get_profile_name());
	if (pu) result = pu->is_mwi_subscribed();
	
	return result;
}

bool t_phone::is_mwi_terminated(t_user *user) const {
	bool result = false;
	
	t_rwmutex_reader x(phone_users_mtx);
	t_phone_user *pu = find_phone_user(user->get_profile_name());
	if (pu) result = pu->is_mwi_terminated();
	
	return result;
}

t_mwi t_phone::get_mwi(t_user *user) const {
	t_mwi result;
	
	t_rwmutex_reader x(phone_users_mtx);
	t_phone_user *pu = find_phone_user(user->get_profile_name());
	if (pu) result = pu->mwi;
	
	return result;
}

bool t_phone::is_presence_terminated(t_user *user) const {
	bool result = false;
	
	t_rwmutex_reader x(phone_users_mtx);
	t_phone_user *pu = find_phone_user(user->get_profile_name());
	if (pu) result = pu->is_presence_terminated();
	
	return result;
}

t_url t_phone::get_remote_uri(unsigned short lineno) const {
	assert(lineno < lines.size());
	
	t_rwmutex_reader x(lines_mtx);
	t_url uri = get_line(lineno)->get_remote_uri();
	return uri;
}

string t_phone::get_remote_display(unsigned short lineno) const {
	assert(lineno < lines.size());
	
	t_rwmutex_reader x(lines_mtx);
	string display = get_line(lineno)->get_remote_display();
	return display;
}

bool t_phone::part_of_3way(unsigned short lineno) {
	t_mutex_guard x(mutex_3way);

	if (!is_3way) {
		return false;
	}

	if (line1_3way->get_line_number() == lineno) {
		return true;
	}

	if (line2_3way->get_line_number() == lineno) {
		return true;
	}

	return false;
}

t_line *t_phone::get_3way_peer_line(unsigned short lineno) {
	t_mutex_guard x(mutex_3way);

	if (!is_3way) {
		return NULL;
	}

	if (line1_3way->get_line_number() == lineno) {
		return line2_3way;
	}

	return line1_3way;
}

bool t_phone::join_3way(unsigned short lineno1, unsigned short lineno2) {
	assert(lineno1 < NUM_USER_LINES);
	assert(lineno2 < NUM_USER_LINES);

	t_mutex_guard x3(mutex_3way);
	t_rwmutex_reader x(lines_mtx);

	// Check if there isn't a 3-way already
	if (is_3way) {
		return false;
	}

	// Both lines must have a call.
	if (lines[lineno1]->get_substate() != LSSUB_ESTABLISHED ||
	    lines[lineno2]->get_substate() != LSSUB_ESTABLISHED)
	{
		return false;
	}

	// One of the lines must be on-hold
	t_line *held_line, *talking_line;
	if (lines[lineno1]->get_is_on_hold()) {
		held_line = lines[lineno1];
		talking_line = lines[lineno2];
	} else if (lines[lineno2]->get_is_on_hold()) {
		held_line = lines[lineno2];
		talking_line = lines[lineno1];
	} else {
		return false;
	}

	// Set 3-way data
	is_3way = true;
	line1_3way = talking_line;
	line2_3way = held_line;

	// The user may have put both lines on-hold. In this case the
	// talking line is on-hold too!
	if (talking_line->get_is_on_hold()) {
		// Retrieve the held call
		// As the 3-way indication (is_3way) is set, the audio sessions
		// will automatically connect to each other.
		talking_line->retrieve();
	} else {	
		// Start the 3-way on the talking line
		t_audio_session *as_talking = talking_line->get_audio_session();
		if (as_talking) as_talking->start_3way();
	}

	// Retrieve the held call
	held_line->retrieve();

	return true;
}

void t_phone::notify_refer_progress(t_response *r, unsigned short referee_lineno) {
	t_rwmutex_reader x(lines_mtx);

	if (lines[LINENO_REFERRER]->get_state() != LS_IDLE) {
		lines[LINENO_REFERRER]->notify_refer_progress(r);

		if (!lines[LINENO_REFERRER]->active_dialog ||
		    lines[LINENO_REFERRER]->active_dialog->get_state() != DS_CONFIRMED)
		{
			// The call to the referrer has already been
			// terminated.
			return;
		}

		if (r->is_final()) {
			if (r->is_success()) {
				// Reference was successful, end the call with
				// with the referrer.
				log_file->write_header(
					"t_phone::notify_refer_progress");
				log_file->write_raw(
					"Call to refer-target succeeded.\n");
				log_file->write_raw(
					"End call with referrer.\n");
				log_file->write_footer();
				
				lines[LINENO_REFERRER]->end_call();
			} else {
				// Reference failed, retrieve the call with the
				// referrer.
				log_file->write_header(
					"t_phone::notify_refer_progress");
				log_file->write_raw(
					"Call to refer-target failed.\n");
				log_file->write_raw(
					"Restore call with referrer.\n");
				log_file->write_footer();

				// Retrieve the parked line
				t_line *l = lines[referee_lineno];
				lines[referee_lineno] = lines[LINENO_REFERRER];
				lines[referee_lineno]->line_number = referee_lineno;
				lines[LINENO_REFERRER] = l;
				lines[LINENO_REFERRER]->line_number = LINENO_REFERRER;
				
				// Retrieve the call if the line is active
				if (referee_lineno == active_line) {
					log_file->write_report(
						"Retrieve call with referrer.",
						"t_phone::notify_refer_progress");
					lines[referee_lineno]->retrieve();
				}
				
				t_user *user_config = lines[referee_lineno]->get_user();
				assert(user_config);
				
				ui->cb_retrieve_referrer(user_config, referee_lineno);
			}
		}
	}
}

t_call_info t_phone::get_call_info(unsigned short lineno) const {
	assert(lineno < lines.size());

	t_rwmutex_reader x(lines_mtx);
	t_call_info call_info = get_line(lineno)->get_call_info();
	return call_info;
}

t_call_record t_phone::get_call_hist(unsigned short lineno) const {
	assert(lineno < lines.size());

	t_rwmutex_reader x(lines_mtx);
	t_call_record call_hist = get_line(lineno)->call_hist_record;
	return call_hist;
}

string t_phone::get_ringtone(unsigned short lineno) const {
	assert(lineno < lines.size());

	t_rwmutex_reader x(lines_mtx);
	string ringtone = get_line(lineno)->get_ringtone();
	return ringtone;
}

time_t t_phone::get_startup_time(void) const {
	return startup_time;
}

void t_phone::init_rtp_ports(void) {
	t_rwmutex_reader x(lines_mtx);
	for (size_t i = 0; i < lines.size(); i++) {
		lines[i]->init_rtp_port();
	}
}

bool t_phone::add_phone_user(const t_user &user_config, t_user **dup_user) {
	
	t_phone_user *existing_phone_user = NULL;
	t_rwmutex_writer x(phone_users_mtx);

	for (list<t_phone_user *>::iterator i = phone_users.begin();
	     i != phone_users.end(); i++)
	{
		t_user *user = (*i)->get_user_profile();
		
		// If the profile is already added, then just activate it.
		if (user->get_profile_name() == user_config.get_profile_name())
		{	
			existing_phone_user = (*i);
			// Continue checking to see if activating this user
			// does not conflict with another already active user.
			continue;
		}
		
		// Check if there is already another profile for the same
		// user.
		if (user->get_name() == user_config.get_name() &&
		    user->get_domain() == user_config.get_domain() &&
		    (*i)->is_active())
		{
			*dup_user = user;
			return false;
		}
		
		// Check if there is already another profile having
		// the same contact name.
		if (user->get_contact_name() == user_config.get_contact_name() &&
		    USER_HOST(user, AUTO_IP4_ADDRESS) == USER_HOST(&user_config, AUTO_IP4_ADDRESS) &&
		    (*i)->is_active())
		{
			*dup_user = user;
			return false;
		}
	}
	
	// Activate existing profile
	if (existing_phone_user) {
		if (!existing_phone_user->is_active()) {
			existing_phone_user->activate(user_config);
		}

		return true;
	}
	
	// Add the user
	t_phone_user *pu = new t_phone_user(user_config);
	MEMMAN_NEW(pu);
	phone_users.push_back(pu);
	
	return true;
}

void t_phone::remove_phone_user(const t_user &user_config) {
	t_rwmutex_writer x(phone_users_mtx);
	t_phone_user *pu = find_phone_user(user_config.get_profile_name());
	if (pu) pu->deactivate();
}

list<t_user *> t_phone::ref_users(void) {
	list<t_user *> l;
	
	t_rwmutex_reader x(phone_users_mtx);
	for (list<t_phone_user *>::iterator i = phone_users.begin();
	     i != phone_users.end(); i++)
	{
		if (!(*i)->is_active()) continue;
		l.push_back((*i)->get_user_profile());
	}
	
	return l;
}

t_user *t_phone::ref_user_display_uri(const string &display_uri) {
	t_user *u = NULL;
	
	t_rwmutex_reader x(phone_users_mtx);
	for (list<t_phone_user *>::iterator i = phone_users.begin();
	     i != phone_users.end(); i++)
	{
		if (!(*i)->is_active()) continue;
		if ((*i)->get_user_profile()->get_display_uri() == display_uri) {
			u = (*i)->get_user_profile();
			break;
		}
	}
	
	return u;
}

t_user *t_phone::ref_user_profile(const string &profile_name) {
	t_user *u = NULL;
	
	t_rwmutex_reader x(phone_users_mtx);
	t_phone_user *pu = find_phone_user(profile_name);
	if (pu) u = pu->get_user_profile();
	
	return u;
}

t_service *t_phone::ref_service(t_user *user) {
	assert(user);
	t_service *srv = NULL;
	
	t_rwmutex_reader x(phone_users_mtx);
	t_phone_user *pu = find_phone_user(user->get_profile_name());
	if (pu) srv = pu->service;
	
	return srv;
}

t_buddy_list *t_phone::ref_buddy_list(t_user *user) {
	assert(user);
	t_buddy_list *l = NULL;
	
	t_rwmutex_reader x(phone_users_mtx);
	t_phone_user *pu = find_phone_user(user->get_profile_name());
	if (pu) l = pu->get_buddy_list();
	
	return l;
}

t_presence_epa *t_phone::ref_presence_epa(t_user *user) {
	assert(user);
	t_presence_epa *epa = NULL;
	
	t_rwmutex_reader x(phone_users_mtx);
	t_phone_user *pu = find_phone_user(user->get_profile_name());
	if (pu) epa = pu->get_presence_epa();
	
	return epa;
}

string t_phone::get_ip_sip(const t_user *user, const string &auto_ip) const {
	string result;

	t_rwmutex_reader x(phone_users_mtx);
	t_phone_user *pu = find_phone_user(user->get_profile_name());
	if (pu) {
		result = pu->get_ip_sip(auto_ip);
	} else {
		result = LOCAL_IP;
	}
	
	if (result == AUTO_IP4_ADDRESS) result = auto_ip;
	
	return result;
}

unsigned short t_phone::get_public_port_sip(const t_user *user) const {
	unsigned short result;
	
	t_rwmutex_reader x(phone_users_mtx);
	t_phone_user *pu = find_phone_user(user->get_profile_name());
	if (pu) {
		result = pu->get_public_port_sip();
	} else {
		result = sys_config->get_sip_port();
	}
	
	return result;
}

bool t_phone::use_stun(t_user *user) {
	bool result;

	t_rwmutex_reader x(phone_users_mtx);
	t_phone_user *pu = find_phone_user(user->get_profile_name());
	if (pu) {
		result = pu->use_stun;
	} else {
		result = false;
	}
	
	return result;
}

bool t_phone::use_nat_keepalive(t_user *user) {
	bool result;

	t_rwmutex_reader x(phone_users_mtx);
	t_phone_user *pu = find_phone_user(user->get_profile_name());
	if (pu) {
		result = pu->use_nat_keepalive;
	} else {
		result = false;
	}
	
	return result;	
}

void t_phone::disable_stun(t_user *user) {
	t_rwmutex_reader x(phone_users_mtx);
	t_phone_user *pu = find_phone_user(user->get_profile_name());
	if (pu) pu->use_stun = false;
}

void t_phone::sync_nat_keepalive(t_user *user) {
	t_rwmutex_reader x(phone_users_mtx);
	t_phone_user *pu = find_phone_user(user->get_profile_name());
	if (pu) pu->sync_nat_keepalive();
}

bool t_phone::stun_discover_nat(list<string> &msg_list) {
	bool retval = true;
	
	t_rwmutex_reader x(phone_users_mtx);
	for (list<t_phone_user *>::iterator i = phone_users.begin();
	     i != phone_users.end(); ++i)
	{
		if (!(*i)->is_active()) continue;
		t_user *user_config = (*i)->get_user_profile();
		
		if (user_config->get_sip_transport() == SIP_TRANS_UDP ||
		    user_config->get_sip_transport() == SIP_TRANS_AUTO)
		{
			if (user_config->get_use_stun())
			{
				string msg;
				if (!::stun_discover_nat(*i, msg)) {
					string s("User profile: ");
					s + user_config->get_profile_name();
					s += "\n\n";
					s += msg;
					msg_list.push_back(s);
					retval = false;
				}
			}
			else
			{
				(*i)->use_nat_keepalive = user_config->get_enable_nat_keepalive();
			}
		}
	}
	
	return retval;
}

bool t_phone::stun_discover_nat(t_user *user, string &msg) {
	bool retval = true;
	
	t_rwmutex_reader x(phone_users_mtx);
	if (user->get_sip_transport() == SIP_TRANS_UDP ||
	    user->get_sip_transport() == SIP_TRANS_AUTO)
	{
		t_phone_user *pu = find_phone_user(user->get_profile_name());
		if (user->get_use_stun()) {
			if (pu) retval = ::stun_discover_nat(pu, msg);
		}
		else
		{
			if (pu) pu->use_nat_keepalive = user->get_enable_nat_keepalive();
		}
	}
	
	return retval;
}

t_response *t_phone::create_options_response(t_user *user, t_request *r,
					bool in_dialog)
{
	t_response *resp;
	
	t_rwmutex_reader x(phone_users_mtx);
	t_phone_user *pu = find_phone_user(user->get_profile_name());
	if (pu) {
		resp = pu->create_options_response(r, in_dialog);
	} else {
		resp = r->create_response(R_500_INTERNAL_SERVER_ERROR);
	}
	
	return resp;
}

void t_phone::init(void) {
	
	list<t_user *> user_list = ref_users();
	
	for (list<t_user *>::iterator i = user_list.begin(); i != user_list.end(); i++)
	{
		// Automatic registration at startup if requested
		if ((*i)->get_register_at_startup()) {
			pub_registration(*i, REG_REGISTER, DUR_REGISTRATION(*i));
		} else {
			// No registration will be done, so initialize extensions now.
			init_extensions(*i);
		}
		
		// NOTE: Extension initialization is done after registration. 
		//       This way STUN will have set the correct
		//       IP adres (STUN is done as part of registration.)
	}
}

void t_phone::init_extensions(t_user *user_config) {
	// Subscribe to MWI
	if (user_config->get_mwi_sollicited()) {
		pub_subscribe_mwi(user_config);
	}
	
	// Publish presence
	if (user_config->get_pres_publish_startup()) {
		pub_publish_presence(user_config, t_presence_state::ST_BASIC_OPEN);
	}
	
	// Subscribe to presence
	pub_subscribe_presence(user_config);
}

bool t_phone::set_sighandler(void) const {
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));

	sa.sa_handler = phone_sighandler;
	sa.sa_flags = SA_RESTART;
	if (sigaction (SIGCHLD, &sa, NULL) < 0) return false;
	if (sigaction (SIGTERM, &sa, NULL) < 0) return false;
	if (sigaction (SIGINT, &sa, NULL) < 0) return false;
	
	return true;
}


void t_phone::terminate(void) {
	string msg;
	
	lines_mtx.lockRead();
	// Clear all lines
	log_file->write_report("Clear all lines.",
		"t_phone::terminate", LOG_NORMAL, LOG_DEBUG);
	for (size_t i = 0; i < NUM_CALL_LINES; i++) {
		switch (lines[i]->get_substate()) {
		case LSSUB_IDLE:
		case LSSUB_RELEASING:
			break;
		case LSSUB_SEIZED:
			lines[i]->unseize();
			break;
		case LSSUB_INCOMING_PROGRESS:
			ui->cb_stop_call_notification(i);
			lines[i]->reject();
			break;
		case LSSUB_OUTGOING_PROGRESS:
			ui->cb_stop_call_notification(i);
			// Fall thru
		case LSSUB_ANSWERING:
		case LSSUB_ESTABLISHED:
			lines[i]->end_call();
			break;
		}
	}
	lines_mtx.unlock();
	
	// Deactivate phone
	is_active = false;
	
	// De-register all registered users.
	list<t_user *> user_list = ref_users();
	ui->cb_display_msg("Deregistering phone...");
	for (list<t_user *>::iterator i = user_list.begin();
	     i != user_list.end(); i++)
	{
		// Unsubscribe MWI
		if (is_mwi_subscribed(*i)) {
			msg = (*i)->get_profile_name();
			msg += ": Unsubscribe MWI.";
			log_file->write_report(msg,
				"t_phone::terminate", LOG_NORMAL, LOG_DEBUG);
			pub_unsubscribe_mwi(*i);
		}
		
		// Unpublish presence
		pub_unpublish_presence(*i);
		
		// Unsubscribe presence
		pub_unsubscribe_presence(*i);
		
		// De-register
		if (get_is_registered(*i)) {
			msg = (*i)->get_profile_name();
			msg += ": Deregister.";
			log_file->write_report(msg,
				"t_phone::terminate", LOG_NORMAL, LOG_DEBUG);
			pub_registration(*i, REG_DEREGISTER);
		}
	}
	
	// Wait till phone is deregistered.
	for (list<t_user *>::iterator i = user_list.begin(); i != user_list.end(); i++)
	{
		while (get_is_registered(*i)) {
			sleep(1);
		}
		msg = (*i)->get_profile_name();
		msg += ": Registration terminated.";
		log_file->write_report(msg, "t_phone::terminate", LOG_NORMAL, LOG_DEBUG);
	}
	
	// Wait for MWI unsubscription
	int mwi_wait = 0;
	for (list<t_user *>::iterator i = user_list.begin(); i != user_list.end(); i++)
	{
		while (!is_mwi_terminated(*i) && mwi_wait <= DUR_UNSUBSCRIBE_GUARD/1000) {
			sleep(1);
			mwi_wait++;
		}
		msg = (*i)->get_profile_name();
		msg += ": MWI subscription terminated.";
		log_file->write_report(msg, "t_phone::terminate", LOG_NORMAL, LOG_DEBUG);
	}
	
	// Wait for presence unsubscription
	int presence_wait = 0;
	for (list<t_user *>::iterator i = user_list.begin(); i != user_list.end(); i++)
	{
		while (!is_presence_terminated(*i) && presence_wait <= DUR_UNSUBSCRIBE_GUARD/1000) {
			sleep(1);
			presence_wait++;
		}
		msg = (*i)->get_profile_name();
		msg += ": presence subscriptions terminated.";
		log_file->write_report(msg, "t_phone::terminate", LOG_NORMAL, LOG_DEBUG);
	}
		
	// Wait till all lines are idle
	log_file->write_report("Waiting for all lines to become idle.",
		"t_phone::terminate", LOG_NORMAL, LOG_DEBUG);
	int dur = 0;
	while (dur < QUIT_IDLE_WAIT) {
		if (all_lines_idle()) break;
		sleep(1);
		dur++;
	}
		
	// Force lines to idle state if they could not be cleared
	// gracefully
	lines_mtx.lockRead();
	for (size_t i = 0; i < lines.size(); i++) {
		if (lines[i]->get_substate() != LSSUB_IDLE) {
			msg = "Force line %1 to idle state.";
			msg = replace_first(msg, "%1", int2str(i));
			log_file->write_report(msg, "t_phone::terminate", 
				LOG_NORMAL, LOG_DEBUG);
			lines[i]->force_idle();
		}
	}
	lines_mtx.unlock();
	
	log_file->write_report("Finished phone termination.",
		"t_phone::terminate",  LOG_NORMAL, LOG_DEBUG);
}

void *phone_uas_main(void *arg) {
	phone->run();
	return NULL;
}

void *phone_sigwait(void *arg) {
	sigset_t	sigset;
	int		sig;
	int		child_status;
	pid_t		pid;

	sigemptyset(&sigset);
	sigaddset(&sigset, SIGINT);
	sigaddset(&sigset, SIGTERM);
	sigaddset(&sigset, SIGCHLD);

	while (true) {
		// When SIGCONT is received after SIGSTOP, sigwait returns
		// with EINTR ??
		if (sigwait(&sigset, &sig) == EINTR) continue;
		
		switch (sig) {
		case SIGINT:
			log_file->write_report("SIGINT received.", "::phone_sigwait");
			ui->cmd_quit();
			return NULL;
		case SIGTERM:
			log_file->write_report("SIGTERM received.", "::phone_sigwait");
			ui->cmd_quit();
			return NULL;
		case SIGCHLD:
			// Cleanup terminated child process
			pid = wait(&child_status);
			log_file->write_header("::phone_sigwait");
			log_file->write_raw("SIGCHLD received.\n");
			log_file->write_raw("Pid ");
			log_file->write_raw((int)pid);
			log_file->write_raw(" terminated.\n");
			log_file->write_footer();
			break;
		default:
			log_file->write_header("::phone_sigwait", LOG_NORMAL, LOG_WARNING);
			log_file->write_raw("Unexpected signal (");
			log_file->write_raw(sig);
			log_file->write_raw(") received.\n");
			log_file->write_footer();
		}
	}
}

void phone_sighandler(int sig) {
	int		child_status;
	pid_t		pid;
	
	// Minimal processing should be done in a signal handler.
	// No I/O should be performed.
	switch (sig) {
	case SIGINT:
		// Post a quit command instead of executing it. As executing
		// involves a lock that may lead to a deadlock.
		ui->cmd_quit_async();
		break;
	case SIGTERM:
		ui->cmd_quit_async();
		break;
	case SIGCHLD:
		// Cleanup terminated child process
		pid = wait(&child_status);
		break;
	}
}
