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

#include <assert.h>
#include <iostream>
#include <signal.h>
#include "exceptions.h"
#include "line.h"
#include "log.h"
#include "sdp/sdp.h"
#include "util.h"
#include "user.h"
#include "userintf.h"
#include "audits/memman.h"

extern t_event_queue	*evq_timekeeper;

///////////////
// t_call_info
///////////////

t_call_info::t_call_info() {
	clear();
}

void t_call_info::clear(void) {
	from_uri.set_url("");
	from_display.clear();
	from_display_override.clear();
	from_organization.clear();
	to_uri.set_url("");
	to_display.clear();
	to_organization.clear();
	subject.clear();
	dtmf_supported = false;
	hdr_referred_by = t_hdr_referred_by();
	last_provisional_reason.clear();
	send_codec = CODEC_NULL;
	recv_codec = CODEC_NULL;
	refer_supported = false;
}

string t_call_info::get_from_display_presentation(void) const {
	if (from_display_override.empty()) {
		return from_display;
	} else {
		return from_display_override;
	}
}


///////////
// t_line
///////////

///////////
// Private
///////////

t_dialog *t_line::match_response(t_response *r,
		const list<t_dialog *> &l) const
{
	list<t_dialog *>::const_iterator i;
	for (i = l.begin(); i != l.end(); i++) {
		if ((*i)->match_response(r, 0)) return *i;
	}

	return NULL;
}

t_dialog *t_line::match_response(StunMessage *r, t_tuid tuid,
		const list<t_dialog *> &l) const
{
	list<t_dialog *>::const_iterator i;
	for (i = l.begin(); i != l.end(); i++) {
		if ((*i)->match_response(r, tuid)) return *i;
	}

	return NULL;
}

t_dialog *t_line::match_call_id_tags(const string &call_id,
		const string &to_tag, const string &from_tag,
		const list<t_dialog *> &l) const
{
	list<t_dialog *>::const_iterator i;
	for (i = l.begin(); i != l.end(); i++) {
		if ((*i)->match(call_id, to_tag, from_tag)) return *i;
	}

	return NULL;
}

t_dialog *t_line::get_dialog(t_object_id did) const {
	list<t_dialog *>::const_iterator i;

	if (did == 0) return NULL;

	if (open_dialog && open_dialog->get_object_id() == did) {
		return open_dialog;
	}

	if (active_dialog && active_dialog->get_object_id() == did) {
		return active_dialog;
	}

	for (i = pending_dialogs.begin(); i != pending_dialogs.end(); i++) {
		if ((*i)->get_object_id() == did) return *i;
	}

	for (i = dying_dialogs.begin(); i != dying_dialogs.end(); i++) {
		if ((*i)->get_object_id() == did) return *i;
	}

	return NULL;
}

void t_line::cleanup(void) {
	list<t_dialog *>::iterator i;

	if (open_dialog && open_dialog->get_state() == DS_TERMINATED) {
		MEMMAN_DELETE(open_dialog);
		delete open_dialog;
		open_dialog = NULL;
	}

	if (active_dialog && active_dialog->get_state() == DS_TERMINATED) {
		MEMMAN_DELETE(active_dialog);
		delete active_dialog;
		active_dialog = NULL;

		stop_timer(LTMR_INVITE_COMP);
		stop_timer(LTMR_NO_ANSWER);

		// If the call has been ended within 64*T1 seconds
		// after the reception of the first 2XX response, there
		// might still be open and pending dialogs. To be nice these
		// dialogs should be kept till the 64*T1 timer expires.
		// This complicates the setup of new call however. For
		// now the dialogs will be killed. If a slow UAS
		// still responds, it has bad luck and will time out.
		//
		// TODO:
		// A nice solution would be to move the pending and open
		// dialog to the dying dialog and start a new time 64*T1
		// timer to keep the dying dialogs alive. A sequence of
		// a few short calls would add to the dying dialogs and
		// keep some dialogs alive longer than necessary. This
		// only has an impact on resources, not on signalling.
		// Note that the open dialog must be appended after the
		// pending dialogs, otherwise all received responses for
		// a pending dialog will match the open dialog if that
		// match is tried first by match_response()
		for (i = pending_dialogs.begin(); i != pending_dialogs.end();
			i++)
		{
			MEMMAN_DELETE(*i);
			delete *i;
		}
		pending_dialogs.clear();

		if (open_dialog) {
			MEMMAN_DELETE(open_dialog);
			delete open_dialog;
		}
		open_dialog = NULL;
	}

	if (active_dialog) {
		if (active_dialog->get_state() == DS_CONFIRMED_SUB) {
			// The calls have been released but a subscription is
			// still active.
			substate = LSSUB_RELEASING;
		} else if (active_dialog->will_release()) {
			substate = LSSUB_RELEASING;
		}
	}

	for (i = pending_dialogs.begin(); i != pending_dialogs.end(); i++) {
		if ((*i)->get_state() == DS_TERMINATED) {
			MEMMAN_DELETE(*i);
			delete *i;
			*i = NULL;
		}
	}
	pending_dialogs.remove(NULL);

	for (i = dying_dialogs.begin(); i != dying_dialogs.end(); i++) {
		if ((*i)->get_state() == DS_TERMINATED) {
			MEMMAN_DELETE(*i);
			delete *i;
			*i = NULL;
		}
	}
	dying_dialogs.remove(NULL);

	if (!open_dialog && !active_dialog && pending_dialogs.size() == 0) {
		state = LS_IDLE;
		
		if (keep_seized) {
			substate = LSSUB_SEIZED;
		} else {
			substate = LSSUB_IDLE;
		}
		
		is_on_hold = false;
		is_muted = false;
		hide_user = false;
		cleanup_transfer_consult_state();
		try_to_encrypt = false;
		auto_answer = false;
		call_info.clear();
		call_history->add_call_record(call_hist_record);
		call_hist_record.renew();
		phone_user = NULL;
		user_defined_ringtone.clear();
		ui->cb_line_state_changed();
	}
}

void t_line::cleanup_open_pending(void) {
	if (open_dialog) {
		MEMMAN_DELETE(open_dialog);
		delete open_dialog;
		open_dialog = NULL;
	}

	list<t_dialog *>::iterator i;
	for (i = pending_dialogs.begin(); i != pending_dialogs.end(); i++) {
		MEMMAN_DELETE(*i);
		delete *i;
	}
	pending_dialogs.clear();

	if (!active_dialog) {
		is_on_hold = false;
		is_muted = false;
		hide_user = false;
		cleanup_transfer_consult_state();
		try_to_encrypt = false;
		auto_answer = false;
		state = LS_IDLE;
		
		if (keep_seized) {
			substate = LSSUB_SEIZED;
		} else {
			substate = LSSUB_IDLE;
		}

		call_info.clear();
		call_history->add_call_record(call_hist_record);
		call_hist_record.renew();
		phone_user = NULL;
		user_defined_ringtone.clear();
		ui->cb_line_state_changed();
	}
}

void t_line::cleanup_forced(void) {
	list<t_dialog *>::iterator i;

	if (open_dialog) {
		MEMMAN_DELETE(open_dialog);
		delete open_dialog;
		open_dialog = NULL;
	}

	if (active_dialog) {
		MEMMAN_DELETE(active_dialog);
		delete active_dialog;
		active_dialog = NULL;
	}

	for (i = pending_dialogs.begin(); i != pending_dialogs.end(); i++) {
		MEMMAN_DELETE(*i);
		delete *i;
		*i = NULL;
	}
	pending_dialogs.remove(NULL);

	for (i = dying_dialogs.begin(); i != dying_dialogs.end(); i++) {
		MEMMAN_DELETE(*i);
		delete *i;
		*i = NULL;
	}
	dying_dialogs.remove(NULL);
	
	// TODO: stop running timers?

	state = LS_IDLE;
	substate = LSSUB_IDLE;
	keep_seized = false;
	is_on_hold = false;
	is_muted = false;
	hide_user = false;
	cleanup_transfer_consult_state();
	auto_answer = false;
	call_info.clear();
	call_history->add_call_record(call_hist_record);
	call_hist_record.renew();
	phone_user = NULL;
	user_defined_ringtone.clear();
	ui->cb_line_state_changed();
}

void t_line::cleanup_transfer_consult_state(void) {
	if (is_transfer_consult) {
		t_line *from_line = phone->get_line(consult_transfer_from_line);
		from_line->set_to_be_transferred(false, 0);
		is_transfer_consult = false;
	}
	
	if (to_be_transferred) {
		t_line *to_line = phone->get_line(consult_transfer_to_line);
		to_line->set_is_transfer_consult(false, 0);
		to_be_transferred = false;
	}
}


///////////
// Public
///////////

t_line::t_line(t_phone *_phone, unsigned short _line_number) : 
	t_id_object()
{
	// NOTE: The rtp_port attribute can only be initialized when
	//       a user profile has been selected.

	phone = _phone;
	state = LS_IDLE;
	substate = LSSUB_IDLE;
	open_dialog = NULL;
	active_dialog = NULL;
	is_on_hold = false;
	is_muted = false;
	hide_user = false;
	is_transfer_consult = false;
	to_be_transferred = false;
	try_to_encrypt = false;
	auto_answer = false;
	line_number = _line_number;
	id_invite_comp = 0;
	id_no_answer = 0;
	phone_user = NULL;
	user_defined_ringtone.clear();
	keep_seized = false;
}

t_line::~t_line() {
	list<t_dialog *>::iterator i;

	// Stop timers
	if (id_invite_comp) stop_timer(LTMR_INVITE_COMP);
	if (id_no_answer) stop_timer(LTMR_NO_ANSWER);

	// Delete pointers
	if (open_dialog) {
		MEMMAN_DELETE(open_dialog);
		delete open_dialog;
	}
	if (active_dialog) {
		MEMMAN_DELETE(active_dialog);
		delete active_dialog;
	}

	// Delete dialogs
	for (i = pending_dialogs.begin(); i != pending_dialogs.end(); i++) {
		MEMMAN_DELETE(*i);
		delete *i;
	}

	for (i = dying_dialogs.begin(); i != dying_dialogs.end(); i++) {
		MEMMAN_DELETE(*i);
		delete *i;
	}
}

t_line_state t_line::get_state(void) const {
	return state;
}

t_line_substate t_line::get_substate(void) const {
	return substate;
}

t_refer_state t_line::get_refer_state(void) const {
	if (active_dialog) return active_dialog->refer_state;
	return REFST_NULL;
}

void t_line::start_timer(t_line_timer timer, t_object_id did) {
	t_tmr_line	*t;
	t_dialog	*dialog = get_dialog(did);
	unsigned long	dur;
	
	assert(phone_user);

	switch(timer) {
	case LTMR_ACK_TIMEOUT:
		assert(dialog);
		// RFC 3261 13.3.1.4
		if (dialog->dur_ack_timeout == 0) {
			dialog->dur_ack_timeout = DURATION_T1;
		} else {
			dialog->dur_ack_timeout *= 2;
			if (dialog->dur_ack_timeout > DURATION_T2 ) {
				dialog->dur_ack_timeout = DURATION_T2;
			}
		}
		t = new t_tmr_line(dialog->dur_ack_timeout , timer, get_object_id(),
					did);
		MEMMAN_NEW(t);
		dialog->id_ack_timeout = t->get_object_id();
		break;
	case LTMR_ACK_GUARD:
		assert(dialog);
		// RFC 3261 13.3.1.4
		t = new t_tmr_line(64 * DURATION_T1, timer, get_object_id(), did);
		MEMMAN_NEW(t);
		dialog->id_ack_guard = t->get_object_id();
		break;
	case LTMR_INVITE_COMP:
		// RFC 3261 13.2.2.4
		t = new t_tmr_line(64 * DURATION_T1, timer, get_object_id(), did);
		MEMMAN_NEW(t);
		id_invite_comp = t->get_object_id();
		break;
	case LTMR_NO_ANSWER:
		t = new t_tmr_line(DUR_NO_ANSWER(phone_user->get_user_profile()), 
				timer, get_object_id(), did);
		MEMMAN_NEW(t);
		id_no_answer = t->get_object_id();
		break;
	case LTMR_RE_INVITE_GUARD:
		assert(dialog);
		t = new t_tmr_line(DUR_RE_INVITE_GUARD, timer, get_object_id(), did);
		MEMMAN_NEW(t);
		dialog->id_re_invite_guard = t->get_object_id();
		break;
	case LTMR_GLARE_RETRY:
		assert(dialog);
		if (dialog->is_call_id_owner()) {
			dur = DUR_GLARE_RETRY_OWN;
		} else {
			dur = DUR_GLARE_RETRY_NOT_OWN;
		}
		t = new t_tmr_line(dur, timer, get_object_id(), did);
		MEMMAN_NEW(t);
		dialog->id_glare_retry = t->get_object_id();
		break;
	case LTMR_100REL_TIMEOUT:
		assert(dialog);
		// RFC 3262 3
		if (dialog->dur_100rel_timeout == 0) {
			dialog->dur_100rel_timeout = DUR_100REL_TIMEOUT;
		} else {
			dialog->dur_100rel_timeout *= 2;
		}
		t = new t_tmr_line(dialog->dur_100rel_timeout , timer, get_object_id(),
					did);
		MEMMAN_NEW(t);
		dialog->id_100rel_timeout = t->get_object_id();
		break;
	case LTMR_100REL_GUARD:
		assert(dialog);
		// RFC 3262 3
		t = new t_tmr_line(DUR_100REL_GUARD, timer, get_object_id(), did);
		MEMMAN_NEW(t);
		dialog->id_100rel_guard = t->get_object_id();
		break;
	case LTMR_CANCEL_GUARD:
		assert(dialog);
		t = new t_tmr_line(DUR_CANCEL_GUARD, timer, get_object_id(), did);
		MEMMAN_NEW(t);
		dialog->id_cancel_guard = t->get_object_id();
		break;		
	default:
		assert(false);
	}

	evq_timekeeper->push_start_timer(t);
	MEMMAN_DELETE(t);
	delete t;
}

void t_line::stop_timer(t_line_timer timer, t_object_id did) {
	t_object_id	*id;
	t_dialog	*dialog = get_dialog(did);

	switch(timer) {
	case LTMR_ACK_TIMEOUT:
		assert(dialog);
		dialog->dur_ack_timeout = 0;
		id = &dialog->id_ack_timeout;
		break;
	case LTMR_ACK_GUARD:
		assert(dialog);
		id = &dialog->id_ack_guard;
		break;
	case LTMR_INVITE_COMP:
		id = &id_invite_comp;
		break;
	case LTMR_NO_ANSWER:
		id = &id_no_answer;
		break;
	case LTMR_RE_INVITE_GUARD:
		assert(dialog);
		id = &dialog->id_re_invite_guard;
		break;
	case LTMR_GLARE_RETRY:
		assert(dialog);
		id = &dialog->id_glare_retry;
		break;
	case LTMR_100REL_TIMEOUT:
		assert(dialog);
		dialog->dur_100rel_timeout = 0;
		id = &dialog->id_100rel_timeout;
		break;
	case LTMR_100REL_GUARD:
		assert(dialog);
		id = &dialog->id_100rel_guard;
		break;
	case LTMR_CANCEL_GUARD:
		assert(dialog);
		id = &dialog->id_cancel_guard;
		
		// KLUDGE
		if (*id == 0) {
			// Cancel is always sent on the open dialog.
			// The timer is probably stopped from a pending dialog,
			// therefore the timer is stopped on the wrong dialog.
			// Check if the open dialog has a CANCEL guard timer.
			if (open_dialog) id = &open_dialog->id_cancel_guard;
		}
		break;
	default:
		assert(false);
	}

	if (*id != 0) evq_timekeeper->push_stop_timer(*id);
	*id = 0;
}

void t_line::invite(t_phone_user *pu, const t_url &to_uri, const string &to_display,
		const string &subject, bool no_fork, bool anonymous)
{
	t_hdr_request_disposition hdr_request_disposition;
	
	if (no_fork) {
		hdr_request_disposition.set_fork_directive(
			t_hdr_request_disposition::NO_FORK);
	}

	invite(pu, to_uri, to_display, subject, t_hdr_referred_by(), 
			t_hdr_replaces(), t_hdr_require(), hdr_request_disposition,
			anonymous);
}

void t_line::invite(t_phone_user *pu, const t_url &to_uri, const string &to_display,
		const string &subject, const t_hdr_referred_by &hdr_referred_by,
		const t_hdr_replaces &hdr_replaces,
		const t_hdr_require &hdr_require, 
		const t_hdr_request_disposition &hdr_request_disposition,
		bool anonymous)
{
	assert(pu);
	
	// Ignore if line is not idle
	if (state != LS_IDLE) {
		return;
	}

	assert(!open_dialog);
	
	// Validate speaker and mic
	string error_msg;
	if (!sys_config->exec_audio_validation(false, true, true, error_msg)) {
		ui->cb_show_msg(error_msg, MSG_CRITICAL);
		return;
	}
	
	phone_user = pu;
	t_user *user_config = pu->get_user_profile();

	call_info.from_uri = create_user_uri(); // NOTE: hide_user is not set yet
	call_info.from_display = user_config->get_display(false);
	call_info.from_organization = user_config->get_organization();
	call_info.to_uri = to_uri;
	call_info.to_display = to_display;
	call_info.to_organization.clear();
	call_info.subject = subject;
	call_info.hdr_referred_by = hdr_referred_by;
	
	try_to_encrypt = user_config->get_zrtp_enabled();

	state = LS_BUSY;
	substate = LSSUB_OUTGOING_PROGRESS;
	hide_user = anonymous;
	ui->cb_line_state_changed();

	open_dialog = new t_dialog(this);
	MEMMAN_NEW(open_dialog);
	open_dialog->send_invite(to_uri, to_display, subject, hdr_referred_by, 
			hdr_replaces, hdr_require, hdr_request_disposition,
			anonymous);

	cleanup();
}

void t_line::answer(void) {
	// Ignore if line is idle
	if (state == LS_IDLE) return;
	assert(active_dialog);
	
	// Validate speaker and mic
	string error_msg;
	if (!sys_config->exec_audio_validation(false, true, true, error_msg)) {
		ui->cb_show_msg(error_msg, MSG_CRITICAL);
		return;
	}

	stop_timer(LTMR_NO_ANSWER);

	try {
		substate = LSSUB_ANSWERING;
		ui->cb_line_state_changed();
		active_dialog->answer();
	}
	catch (t_exception x) {
		// TODO: there is no call to answer
	}

	cleanup();
}

void t_line::reject(void) {
	// Ignore if line is idle
	if (state == LS_IDLE) return;
	assert(active_dialog);

	stop_timer(LTMR_NO_ANSWER);

	try {
		active_dialog->reject(R_603_DECLINE);
	}
	catch (t_exception x) {
		// TODO: there is no call to reject
	}

	cleanup();
}

void t_line::redirect(const list<t_display_url> &destinations, int code, string reason)
{
	// Ignore if line is idle
	if (state == LS_IDLE) return;
	assert(active_dialog);

	stop_timer(LTMR_NO_ANSWER);

	try {
		active_dialog->redirect(destinations, code, reason);
	}
	catch (t_exception x) {
		// TODO: there is no call to redirect
	}

	cleanup();
}

void t_line::end_call(void) {
	// Ignore if phone is idle
	if (state == LS_IDLE) return;

	if (active_dialog) {
		substate = LSSUB_RELEASING;
		ui->cb_line_state_changed();
		ui->cb_stop_call_notification(line_number);
		active_dialog->send_bye();
		
		// If the line was part of a transfer with consultation,
		// then clean the consultation state as the transfer cannot
		// proceed anymore.
		cleanup_transfer_consult_state();
		
		cleanup();
		return;
	}

	// Always send the CANCEL on the open dialog.
	// The pending dialogs will be cleared when the INVITE gets
	// terminated.
	// CANCEL is send on the open dialog as the CANCEL must have
	// the same tags as the INVITE.
	if (open_dialog) {
		substate = LSSUB_RELEASING;
		ui->cb_line_state_changed();
		ui->cb_stop_call_notification(line_number);
		open_dialog->send_cancel(!pending_dialogs.empty());
		
		// Make sure dialog is terminated if CANCEL glares with
		// 2XX on INVITE.
		for (list<t_dialog *>::iterator i = pending_dialogs.begin();
		     i != pending_dialogs.end(); i++)
		{
			(*i)->set_end_after_2xx_invite(true);
		}
		
		cleanup();
		return;
	}

	// NOTE:
	// The call is only ended for real when the dialog reaches
	// the DS_TERMINATED state, i.e. a 200 OK on BYE is received
	// or a 487 TERMINATED on INVITE is received.
}

void t_line::send_dtmf(char digit, bool inband, bool info) {
	// DTMF may be sent on an early media session, so find
	// a dialog that has an RTP session. There can be at most 1.
	t_dialog *d = get_dialog_with_active_session();

	if (d) {
		d->send_dtmf(digit, inband, info);
		cleanup();
		return;
	}
}

void t_line::options(void) {
	if (active_dialog && active_dialog->get_state() == DS_CONFIRMED) {
		active_dialog->send_options();
		cleanup();
		return;
	}
}

bool t_line::hold(bool rtponly) {
	if (is_on_hold) return true;

	if (active_dialog && active_dialog->get_state() == DS_CONFIRMED) {
		active_dialog->hold(rtponly);
		is_on_hold = true;
		ui->cb_line_state_changed();
		cleanup();
		return true;
	}

	return false;
}

void t_line::retrieve(void) {
	if (!is_on_hold) return;

	if (active_dialog && active_dialog->get_state() == DS_CONFIRMED) {
		active_dialog->retrieve();
		is_on_hold = false;
		ui->cb_line_state_changed();
		cleanup();
		return;
	}
}

void t_line::kill_rtp(void) {
	if (active_dialog) active_dialog->kill_rtp();
	
	for (list<t_dialog *>::iterator i = pending_dialogs.begin();
		     i != pending_dialogs.end(); i++)
	{
		(*i)->kill_rtp();
	}
	
	for (list<t_dialog *>::iterator i = dying_dialogs.begin();
		     i != dying_dialogs.end(); i++)
	{
		(*i)->kill_rtp();
	}
}

void t_line::refer(const t_url &uri, const string &display) {
	if (active_dialog && active_dialog->get_state() == DS_CONFIRMED) {
		active_dialog->send_refer(uri, display);
		ui->cb_line_state_changed();
		cleanup();
		return;
	}
}

void t_line::mute(bool enable) {
	is_muted = enable;
}

void t_line::recvd_provisional(t_response *r, t_tuid tuid, t_tid tid) {
	t_dialog *d;

	if (active_dialog && active_dialog->match_response(r, 0)) {
		active_dialog->recvd_response(r, tuid, tid);
		cleanup();
		return;
	}

	d = match_response(r, pending_dialogs);
	if (d) {
		d->recvd_response(r, tuid, tid);
		cleanup();
		return;
	}

	d = match_response(r, dying_dialogs);
	if (d) {
		d->recvd_response(r, tuid, tid);
		cleanup();
		return;
	}

	if (open_dialog && open_dialog->match_response(r, tuid)) {
		if (r->hdr_cseq.method == INVITE) {
			if (r->hdr_to.tag.size() > 0) {
				// Create a new pending dialog
				d = open_dialog->copy();
				pending_dialogs.push_back(d);
				d->recvd_response(r, tuid, tid);
			} else {
				open_dialog->recvd_response(r, tuid, tid);
			}
		} else {
			open_dialog->recvd_response(r, tuid, tid);
		}

		cleanup();
		return;
	}

	// out-of-dialog response
	// Provisional responses should only be given for INVITE.
	// A response for an INVITE is always in a dialog.
	// Ignore provisional responses for other requests.
}

void t_line::recvd_success(t_response *r, t_tuid tuid, t_tid tid) {
	t_dialog *d;

	if (active_dialog && active_dialog->match_response(r, 0)) {
		active_dialog->recvd_response(r, tuid, tid);
		cleanup();
		return;
	}

	d = match_response(r, pending_dialogs);
	if (d) {
		d->recvd_response(r, tuid, tid);
		if (r->hdr_cseq.method == INVITE) {
			if (!active_dialog) {
				// Make the dialog the active dialog
				active_dialog = d;
				pending_dialogs.remove(d);
				start_timer(LTMR_INVITE_COMP);
				substate = LSSUB_ESTABLISHED;
				ui->cb_line_state_changed();
			} else {
				// An active dialog already exists.
				// Terminate this dialog by sending BYE
				d->send_bye();
			}
		}

		cleanup();
		return;
	}

	d = match_response(r, dying_dialogs);
	if (d) {
		d->recvd_response(r, tuid, tid);
		if (r->hdr_cseq.method == INVITE) {
			d->send_bye();
		}
		cleanup();
		return;
	}

	if (open_dialog && open_dialog->match_response(r, tuid)) {
		if (r->hdr_cseq.method == INVITE) {
			// Create a new dialog
			d = open_dialog->copy();

			if (!active_dialog) {
				active_dialog = d;
				active_dialog->recvd_response(r, tuid, tid);
				start_timer(LTMR_INVITE_COMP);
				substate = LSSUB_ESTABLISHED;
				ui->cb_line_state_changed();
			} else {
				pending_dialogs.push_back(d);
				d->recvd_response(r, tuid, tid);

				// An active dialog already exists.
				// Terminate this dialog by sending BYE
				d->send_bye();
			}
		} else {
			open_dialog->recvd_response(r, tuid, tid);
		}

		cleanup();
		return;
	}

	// Response does not match with any pending request. Discard.
}

void t_line::recvd_redirect(t_response *r, t_tuid tuid, t_tid tid) {
	t_dialog *d;
	
	assert(phone_user);
	t_user *user_config = phone_user->get_user_profile();

	if (active_dialog) {
		// If an active dialog exists then non-2XX should
		// only be for this dialog.
		if (active_dialog->match_response(r, 0)) {
			// Redirection of mid-dialog request
			if (!user_config->get_allow_redirection() ||
			    !active_dialog->redirect_request(r))
			{
				// Redirection not allowed/failed
				active_dialog->recvd_response(r, tuid, tid);
			}
			
			// Retrieve a held line after a REFER failure
			if (r->hdr_cseq.method == REFER &&
			    active_dialog->out_refer_req_failed)
			{
				active_dialog->out_refer_req_failed = false;
				if (phone->get_active_line() == line_number &&
				    user_config->get_referrer_hold()) 
				{
					retrieve();
				}
			}
		}

		cleanup();
		return;
	}

	d = match_response(r, pending_dialogs);
	if (d) {
		d->recvd_response(r, tuid, tid);
		if (r->hdr_cseq.method == INVITE) {
			pending_dialogs.remove(d);
			MEMMAN_DELETE(d);
			delete d;

			// RFC 3261 13.2.2.3
			// All early dialogs are considered terminated
			// upon reception of the non-2xx final response.
			list<t_dialog *>::iterator i;
			for (i = pending_dialogs.begin();
			     i != pending_dialogs.end(); i++)
			{
				MEMMAN_DELETE(*i);
				delete *i;
			}
			pending_dialogs.clear();

			if (open_dialog) {
				if (!user_config->get_allow_redirection() ||
				    !open_dialog->redirect_invite(r))
				{
					MEMMAN_DELETE(open_dialog);
					delete open_dialog;
					open_dialog = NULL;
				}
			}
		}

		cleanup();
		return;
	}

	d = match_response(r, dying_dialogs);
	if (d) {
		d->recvd_response(r, tuid, tid);
		cleanup();
		return;
	}

	if (open_dialog && open_dialog->match_response(r, tuid)) {
		if (r->hdr_cseq.method != INVITE) {
			// TODO: can there be a non-INVITE response for an
			//       open dialog??
			open_dialog->recvd_response(r, tuid, tid);
		}

		if (r->hdr_cseq.method == INVITE) {
			if (!user_config->get_allow_redirection() ||
			    !open_dialog->redirect_invite(r))
			{
				// Redirection failed/not allowed
				open_dialog->recvd_response(r, tuid, tid);
				MEMMAN_DELETE(open_dialog);
				delete open_dialog;
				open_dialog = NULL;
			}

			// RFC 3261 13.2.2.3
			// All early dialogs are considered terminated
			// upon reception of the non-2xx final response.
			list<t_dialog *>::iterator i;
			for (i = pending_dialogs.begin();
			     i != pending_dialogs.end(); i++)
			{
				MEMMAN_DELETE(*i);
				delete *i;
			}
			pending_dialogs.clear();
		}

		cleanup();
		return;
	}

	// out-of-dialog responses should be handled by the phone
}

void t_line::recvd_client_error(t_response *r, t_tuid tuid, t_tid tid) {
	t_dialog *d;
	
	assert(phone_user);
	t_user *user_config = phone_user->get_user_profile();

	if (active_dialog) {
		// If an active dialog exists then non-2XX should
		// only be for this dialog.
		if (active_dialog->match_response(r, 0)) {
			bool response_processed = false;

			if (r->must_authenticate()) {
				// Authentication for mid-dialog request
				if (active_dialog->resend_request_auth(r))
				{
					// Authorization successul.
					// The response does not need to be
					// processed any further
					response_processed = true;
				}
			}

			if (!response_processed) {
				// The request failed, redirect it if there
				// are other destinations available.
				if (!user_config->get_allow_redirection() ||
				    !active_dialog->redirect_request(r))
				{
					// Request failed
					active_dialog->
						recvd_response(r, tuid, tid);
				}
			}
			
			// Retrieve a held line after a REFER failure
			if (r->hdr_cseq.method == REFER &&
			    active_dialog->out_refer_req_failed)
			{
				active_dialog->out_refer_req_failed = false;
				if (phone->get_active_line() == line_number &&
				    user_config->get_referrer_hold()) 
				{
					retrieve();
				}
			}
		}

		cleanup();
		return;
	}

	d = match_response(r, pending_dialogs);
	if (d) {
		if (r->hdr_cseq.method != INVITE) {
			if (r->must_authenticate()) {
				// Authentication for non-INVITE request in pending dialog
				if (!d->resend_request_auth(r)) {
					// Could not authorize, send response to dialog
					// where it will be handle as a client failure.
					d->recvd_response(r, tuid, tid);
				}
			} else {
				d->recvd_response(r, tuid, tid);
			}
		} else {
			d->recvd_response(r, tuid, tid);
			pending_dialogs.remove(d);
			MEMMAN_DELETE(d);
			delete d;

			// RFC 3261 13.2.2.3
			// All early dialogs are considered terminated
			// upon reception of the non-2xx final response.
			list<t_dialog *>::iterator i;
			for (i = pending_dialogs.begin();
			     i != pending_dialogs.end(); i++)
			{
				MEMMAN_DELETE(*i);
				delete *i;
			}
			pending_dialogs.clear();

			if (open_dialog) {
				bool response_processed = false;

				if (r->must_authenticate()) {
					// INVITE authentication
					if (open_dialog->resend_invite_auth(r))
					{
						// Authorization successul.
						// The response does not need to
						// be processed any further
						response_processed = true;
					}
				}

				// Resend INVITE if the response indicated that
				// required extensions are not supported.
				if (!response_processed &&
				    open_dialog->resend_invite_unsupported(r))
				{
					response_processed = true;
				}

				if (!response_processed) {
					// The request failed, redirect it if there
					// are other destinations available.
					if (!user_config->get_allow_redirection() ||
					    !open_dialog->redirect_invite(r))
					{
						// Request failed
						MEMMAN_DELETE(open_dialog);
						delete open_dialog;
						open_dialog = NULL;
					}
				}
			}
		}

		cleanup();
		return;
	}

	d = match_response(r, dying_dialogs);
	if (d) {
		d->recvd_response(r, tuid, tid);
		cleanup();
		return;
	}

	if (open_dialog && open_dialog->match_response(r, tuid)) {
		// If the response is a 401/407 then do not send the
		// response to the dialog as the request must be resent.
		// For an INVITE request, the transaction layer has already
		// sent ACK for a failure response.
		if (r->hdr_cseq.method != INVITE) {
			if (r->must_authenticate()) {
				// Authenticate non-INVITE request
				if (!open_dialog->resend_request_auth(r)) {
					// Could not authorize, handle as other client
					// errors.
					open_dialog->recvd_response(r, tuid, tid);
				}
			} else {
				open_dialog->recvd_response(r, tuid, tid);
			}
		}

		if (r->hdr_cseq.method == INVITE) {
			bool response_processed = false;

			if (r->must_authenticate()) {
				// INVITE authentication
				if (open_dialog->resend_invite_auth(r))
				{
					// Authorization successul.
					// The response does not need to
					// be processed any further
					response_processed = true;
				}
			}

			// Resend INVITE if the response indicated that
			// required extensions are not supported.
			if (!response_processed &&
			    open_dialog->resend_invite_unsupported(r))
			{
				response_processed = true;
			}

			if (!response_processed) {
				// The request failed, redirect it if there
				// are other destinations available.
				if (!user_config->get_allow_redirection() ||
				    !open_dialog->redirect_invite(r))
				{
					// Request failed
					open_dialog->recvd_response(r, tuid, tid);
					MEMMAN_DELETE(open_dialog);
					delete open_dialog;
					open_dialog = NULL;
				}
			}

			// RFC 3261 13.2.2.3
			// All early dialogs are considered terminated
			// upon reception of the non-2xx final response.
			list<t_dialog *>::iterator i;
			for (i = pending_dialogs.begin();
			     i != pending_dialogs.end(); i++)
			{
				MEMMAN_DELETE(*i);
				delete *i;
			}
			pending_dialogs.clear();
		}

		cleanup();
		return;
	}

	// out-of-dialog responses should be handled by the phone
}

void t_line::recvd_server_error(t_response *r, t_tuid tuid, t_tid tid) {
	t_dialog *d;

	assert(phone_user);
	t_user *user_config = phone_user->get_user_profile();
	
	if (active_dialog) {
		// If an active dialog exists then non-2XX should
		// only be for this dialog.
		if (active_dialog->match_response(r, 0)) {
			bool response_processed = false;

			if (r->code == R_503_SERVICE_UNAVAILABLE) {
				// RFC 3263 4.3
				// Failover to next destination
				if (active_dialog->failover_request(r))
				{
					// Failover successul.
					// The response does not need to be
					// processed any further
					response_processed = true;
				}
			}

			if (!response_processed) {
				// The request failed, redirect it if there
				// are other destinations available.
				if (!user_config->get_allow_redirection() ||
				    !active_dialog->redirect_request(r))
				{
					// Request failed
					active_dialog->
						recvd_response(r, tuid, tid);
				}
			}
			
			// Retrieve a held line after a REFER failure
			if (r->hdr_cseq.method == REFER &&
			    active_dialog->out_refer_req_failed)
			{
				active_dialog->out_refer_req_failed = false;
				if (phone->get_active_line() == line_number &&
				    user_config->get_referrer_hold()) 
				{
					retrieve();
				}
			}
		}

		cleanup();
		return;
	}

	d = match_response(r, pending_dialogs);
	if (d) {
		d->recvd_response(r, tuid, tid);
		if (r->hdr_cseq.method == INVITE) {
			pending_dialogs.remove(d);
			MEMMAN_DELETE(d);
			delete d;

			// RFC 3261 13.2.2.3
			// All early dialogs are considered terminated
			// upon reception of the non-2xx final response.
			list<t_dialog *>::iterator i;
			for (i = pending_dialogs.begin();
			     i != pending_dialogs.end(); i++)
			{
				MEMMAN_DELETE(*i);
				delete *i;
			}
			pending_dialogs.clear();

			if (open_dialog) {
				bool response_processed = false;

				if (r->code == R_503_SERVICE_UNAVAILABLE) {
					// INVITE failover
					if (open_dialog->failover_invite())
					{
						// Failover successul.
						// The response does not need to
						// be processed any further
						response_processed = true;
					}
				}

				if (!response_processed) {
					// The request failed, redirect it if there
					// are other destinations available.
					if (!user_config->get_allow_redirection() ||
					    !open_dialog->redirect_invite(r))
					{
						// Request failed
						MEMMAN_DELETE(open_dialog);
						delete open_dialog;
						open_dialog = NULL;
					}
				}
			}
		}

		cleanup();
		return;
	}

	d = match_response(r, dying_dialogs);
	if (d) {
		d->recvd_response(r, tuid, tid);
		cleanup();
		return;
	}

	if (open_dialog && open_dialog->match_response(r, tuid)) {
		// If the response is a 503 then do not send the
		// response to the dialog as the request must be resent.
		// For an INVITE request, the transaction layer has already
		// sent ACK for a failure response.
		if (r->code != R_503_SERVICE_UNAVAILABLE && r->hdr_cseq.method != INVITE) {
			open_dialog->recvd_response(r, tuid, tid);
		}

		if (r->hdr_cseq.method == INVITE) {
			bool response_processed = false;

			if (r->code == R_503_SERVICE_UNAVAILABLE) {
				// INVITE failover
				if (open_dialog->failover_invite())
				{
					// Failover successul.
					// The response does not need to
					// be processed any further
					response_processed = true;
				}
			}

			if (!response_processed) {
				// The request failed, redirect it if there
				// are other destinations available.
				if (!user_config->get_allow_redirection() ||
				    !open_dialog->redirect_invite(r))
				{
					// Request failed
					open_dialog->recvd_response(r, tuid, tid);
					MEMMAN_DELETE(open_dialog);
					delete open_dialog;
					open_dialog = NULL;
				}
			}

			// RFC 3261 13.2.2.3
			// All early dialogs are considered terminated
			// upon reception of the non-2xx final response.
			list<t_dialog *>::iterator i;
			for (i = pending_dialogs.begin();
			     i != pending_dialogs.end(); i++)
			{
				MEMMAN_DELETE(*i);
				delete *i;
			}
			pending_dialogs.clear();
		}

		cleanup();
		return;
	}

	// out-of-dialog responses should be handled by the phone
}

void t_line::recvd_global_error(t_response *r, t_tuid tuid, t_tid tid) {
	recvd_redirect(r, tuid, tid);
}

void t_line::recvd_invite(t_phone_user *pu, t_request *r, t_tid tid, const string &ringtone) {
	t_user *user_config = NULL;
	
	switch (state) {
	case LS_IDLE:
		assert(!active_dialog);
		assert(r->hdr_to.tag == "");

		/*
		// TEST ONLY
		// Test code to test INVITE authentication
		if (!r->hdr_authorization.is_populated()) {
			resp = r->create_response(R_401_UNAUTHORIZED);
			t_challenge c;
			c.auth_scheme = AUTH_DIGEST;
			c.digest_challenge.realm = "mtel.nl";
			c.digest_challenge.nonce = "0123456789abcdef";
			c.digest_challenge.opaque = "secret";
			c.digest_challenge.algorithm = ALG_MD5;
			c.digest_challenge.qop_options.push_back(QOP_AUTH);
			c.digest_challenge.qop_options.push_back(QOP_AUTH_INT);
			resp->hdr_www_authenticate.set_challenge(c);
			send_response(resp, 0, tid);
			return;
		}
		*/
		
		assert(pu);
		phone_user = pu;
		user_config = phone_user->get_user_profile();
		user_defined_ringtone = ringtone;
		
		call_info.from_uri = r->hdr_from.uri;
		call_info.from_display = r->hdr_from.display;
		call_info.from_display_override = r->hdr_from.display_override;
		if (r->hdr_organization.is_populated()) {
			call_info.from_organization = r->hdr_organization.name;
		} else {
			call_info.from_organization.clear();
		}
		call_info.to_uri = r->hdr_to.uri;
		call_info.to_display = r->hdr_to.display;
		call_info.to_organization.clear();
		call_info.subject = r->hdr_subject.subject;
		
		try_to_encrypt = user_config->get_zrtp_enabled();

		// Check for REFER support
		// If the Allow header is not present then assume REFER
		// is supported.
		if (!r->hdr_allow.is_populated() ||
		    r->hdr_allow.contains_method(REFER))
		{
			call_info.refer_supported = true;
		}

		active_dialog = new t_dialog(this);
		MEMMAN_NEW(active_dialog);
		active_dialog->recvd_request(r, 0, tid);
		state = LS_BUSY;
		substate = LSSUB_INCOMING_PROGRESS;
		ui->cb_line_state_changed();
		start_timer(LTMR_NO_ANSWER);
		cleanup();
		
		// Answer if auto answer mode is activated
		if (auto_answer) {
			// Validate speaker and mic
			string error_msg;
			if (!sys_config->exec_audio_validation(false, true, true, error_msg)) {
				ui->cb_display_msg(error_msg, MSG_CRITICAL);
			} else {
				answer();
			}
		}
		break;
	case LS_BUSY:
		// Only re-INVITEs can be sent to a busy line
		assert(r->hdr_to.tag != "");

		/*
		// TEST ONLY
		// Test code to test re-INVITE authentication
		if (!r->hdr_authorization.is_populated()) {
			resp = r->create_response(R_401_UNAUTHORIZED);
			t_challenge c;
			c.auth_scheme = AUTH_DIGEST;
			c.digest_challenge.realm = "mtel.nl";
			c.digest_challenge.nonce = "0123456789abcdef";
			c.digest_challenge.opaque = "secret";
			c.digest_challenge.algorithm = ALG_MD5;
			c.digest_challenge.qop_options.push_back(QOP_AUTH);
			c.digest_challenge.qop_options.push_back(QOP_AUTH_INT);
			resp->hdr_www_authenticate.set_challenge(c);
			send_response(resp, 0, tid);
			return;
		}
		*/

		if (active_dialog && active_dialog->match_request(r)) {
			// re-INVITE
			active_dialog->recvd_request(r, 0, tid);
			cleanup();
			return;
		}

		// Should not get here as phone already checked that
		// the request matched with this line
		assert(false);
		break;
	default:
		assert(false);
	}
}

void t_line::recvd_ack(t_request *r, t_tid tid) {
	if (active_dialog && active_dialog->match_request(r)) {
		active_dialog->recvd_request(r, 0, tid);
		substate = LSSUB_ESTABLISHED;
		ui->cb_line_state_changed();
	} else {
		// Should not get here as phone already checked that
		// the request matched with this line
		assert(false);
	}
	cleanup();
}

void t_line::recvd_cancel(t_request *r, t_tid cancel_tid,
		t_tid target_tid)
{
	// A CANCEL matches a dialog if the target tid equals the tid
	// of the INVITE request. This will be checked by
	// dialog::recvd_cancel() itself.
	if (active_dialog) {
		active_dialog->recvd_cancel(r, cancel_tid, target_tid);
	} else {
		// Should not get here as phone already checked that
		// the request matched with this line
		assert(false);
	}
	cleanup();
}

void t_line::recvd_bye(t_request *r, t_tid tid) {
	if (active_dialog && active_dialog->match_request(r)) {
		active_dialog->recvd_request(r, 0, tid);
	} else {
		// Should not get here as phone already checked that
		// the request matched with this line
		assert(false);
	}
	cleanup();
}

void t_line::recvd_options(t_request *r, t_tid tid) {
	if (active_dialog && active_dialog->match_request(r)) {
		active_dialog->recvd_request(r, 0, tid);
	} else {
		// Should not get here as phone already checked that
		// the request matched with this line
		assert(false);
	}
	cleanup();
}

void t_line::recvd_prack(t_request *r, t_tid tid) {
	if (active_dialog && active_dialog->match_request(r)) {
		active_dialog->recvd_request(r, 0, tid);
	} else {
		// Should not get here as phone already checked that
		// the request matched with this line
		assert(false);
	}
	cleanup();
}

void t_line::recvd_subscribe(t_request *r, t_tid tid) {
	if (active_dialog && active_dialog->match_request(r)) {
		active_dialog->recvd_request(r, 0, tid);
	} else {
		// Should not get here as phone already checked that
		// the request matched with this line
		assert(false);
	}
	cleanup();
}

void t_line::recvd_notify(t_request *r, t_tid tid) {
	if (active_dialog && active_dialog->match_request(r)) {
		active_dialog->recvd_request(r, 0, tid);
	} else {
		// Should not get here as phone already checked that
		// the request matched with this line
		assert(false);
	}
	cleanup();
}

void t_line::recvd_info(t_request *r, t_tid tid) {
	if (active_dialog && active_dialog->match_request(r)) {
		active_dialog->recvd_request(r, 0, tid);
	} else {
		// Should not get here as phone already checked that
		// the request matched with this line
		assert(false);
	}
	cleanup();
}

void t_line::recvd_message(t_request *r, t_tid tid) {
	if (active_dialog && active_dialog->match_request(r)) {
		active_dialog->recvd_request(r, 0, tid);
	} else {
		// Should not get here as phone already checked that
		// the request matched with this line
		assert(false);
	}
	cleanup();
}

bool t_line::recvd_refer(t_request *r, t_tid tid) {
	bool retval = false;

	if (active_dialog && active_dialog->match_request(r)) {
		active_dialog->recvd_request(r, 0, tid);
		retval = active_dialog->refer_accepted;
	} else {
		// Should not get here as phone already checked that
		// the request matched with this line
		assert(false);
	}
	cleanup();
	return retval;
}

void t_line::recvd_refer_permission(bool permission, t_request *r) {
	if (active_dialog && active_dialog->match_request(r)) {
		active_dialog->recvd_refer_permission(permission, r);
	}
	cleanup();
}

void t_line::recvd_stun_resp(StunMessage *r, t_tuid tuid, t_tid tid) {
	t_dialog *d;

	if (active_dialog && active_dialog->match_response(r, tuid)) {
		active_dialog->recvd_stun_resp(r, tuid, tid);
		cleanup();
		return;
	}
	
	if (open_dialog && open_dialog->match_response(r, tuid)) {
		open_dialog->recvd_stun_resp(r, tuid, tid);
		cleanup();
		return;
	}
	
	d = match_response(r, tuid, pending_dialogs);
	if (d) {
		d->recvd_stun_resp(r, tuid, tid);
		cleanup();
		return;
	}
	
	d = match_response(r, tuid, dying_dialogs);
	if (d) {
		d->recvd_stun_resp(r, tuid, tid);
		cleanup();
		return;
	}
}

void t_line::failure(t_failure failure, t_tid tid) {
	// TODO
}

void t_line::timeout(t_line_timer timer, t_object_id did) {
	t_dialog *dialog = get_dialog(did);
	list<t_display_url> cf_dest; // call forwarding destinations

	switch (timer) {
	case LTMR_ACK_TIMEOUT:
		// If there is no dialog then ignore the timeout
		if (dialog) {
			dialog->id_ack_timeout = 0;
			dialog->timeout(timer);
		}
		break;
	case LTMR_ACK_GUARD:
		// If there is no dialog then ignore the timeout
		if (dialog) {
			dialog->id_ack_guard = 0;
			dialog->dur_ack_timeout = 0;
			dialog->timeout(timer);
		}
		break;
	case LTMR_INVITE_COMP:
		id_invite_comp = 0;
		// RFC 3261 13.2.2.4
		// The UAC core considers the INVITE transaction completed
		// 64*T1 seconds after the reception of the first 2XX
		// response.
		// Cleanup all open and pending dialogs
		cleanup_open_pending();
		break;
	case LTMR_NO_ANSWER:
		// User did not answer the call.
		// Reject call or redirect it if CF_NOANSWER is active.
		// If there is no active dialog then ignore the timeout.
		// The timer should have been stopped already.
		log_file->write_report("No answer timeout",
					"t_line::timeout");
		
		if (active_dialog) {
			assert(phone_user);
			t_user *user_config = phone_user->get_user_profile();
			t_service *srv = phone->ref_service(user_config);
			if (srv->get_cf_active(CF_NOANSWER, cf_dest)) {
				log_file->write_report("Call redirection no answer",
					"t_line::timeout");
				active_dialog->redirect(cf_dest,
					R_302_MOVED_TEMPORARILY);
			} else {
				active_dialog->reject(R_480_TEMP_NOT_AVAILABLE,
					REASON_480_NO_ANSWER);
			}
			
			ui->cb_answer_timeout(get_line_number());
		}
		break;
	case LTMR_RE_INVITE_GUARD:
		// If there is no dialog then ignore the timeout
		if (dialog) {
			dialog->id_re_invite_guard = 0;
			dialog->timeout(timer);
		}
		break;
	case LTMR_GLARE_RETRY:
		// If there is no dialog then ignore the timeout
		if (dialog) {
			dialog->id_glare_retry = 0;
			dialog->timeout(timer);
		}
		break;
	case LTMR_100REL_TIMEOUT:
		// If there is no dialog then ignore the timeout
		if (dialog) {
			dialog->id_100rel_timeout = 0;
			dialog->timeout(timer);
		}
		break;
	case LTMR_100REL_GUARD:
		// If there is no dialog then ignore the timeout
		if (dialog) {
			dialog->id_100rel_guard = 0;
			dialog->dur_100rel_timeout = 0;
			dialog->timeout(timer);
		}
		break;
	case LTMR_CANCEL_GUARD:
		// If there is no dialog then ignore the timeout
		if (dialog) {
			dialog->id_cancel_guard = 0;
			dialog->timeout(timer);
		}
		break;
	default:
		assert(false);
	}

	cleanup();
}

void t_line::timeout_sub(t_subscribe_timer timer, t_object_id did,
		const string &event_type, const string &event_id)
{
	t_dialog *dialog = get_dialog(did);
	if (dialog) dialog->timeout_sub(timer, event_type, event_id);
	cleanup();
}

bool t_line::match(t_response *r, t_tuid tuid) const {
	if (open_dialog && open_dialog->match_response(r, tuid)) {
		return true;
	}

	if (active_dialog && active_dialog->match_response(r, 0)) {
		return true;
	}

	if (match_response(r, pending_dialogs)) {
		return true;
	}

	if (match_response(r, dying_dialogs)) {
		return true;
	}

	return false;
}

bool t_line::match(t_request *r) const {
	assert(r->method != CANCEL);
	return (active_dialog && active_dialog->match_request(r));
}

bool t_line::match_cancel(t_request *r, t_tid target_tid) const {
	assert(r->method == CANCEL);

	// A CANCEL matches a dialog if the target tid equals the tid
	// of the INVITE request.
	return (active_dialog && active_dialog->match_cancel(r, target_tid));
}

bool t_line::match(StunMessage *r, t_tuid tuid) const {
	if (open_dialog && open_dialog->match_response(r, tuid)) {
		return true;
	}

	if (active_dialog && active_dialog->match_response(r, tuid)) {
		return true;
	}

	if (match_response(r, tuid, pending_dialogs)) {
		return true;
	}

	if (match_response(r, tuid, dying_dialogs)) {
		return true;
	}

	return false;
}

bool t_line::match_replaces(const string &call_id, const string &to_tag, 
		const string &from_tag, bool no_fork_req_disposition,
		bool &early_matched) const
{
	if (active_dialog && active_dialog->match(call_id, to_tag, from_tag)) {
		early_matched = false;
		return true;
	}

	// RFC 3891 3
	// An early dialog only matches when it was created by the UA
	// As an exception to this rule we accept a match when the incoming
	// request contained a no-fork request disposition. This disposition
	// indicated that the request did not fork. The reason why RFC 3891 3
	// does not allow a match is to avoid problems with forked requests.
	// With this exception, call transfer scenario's during ringing can
	// be implemented.
	t_dialog *d;
	if ((d = match_call_id_tags(call_id, to_tag, from_tag, pending_dialogs)) != NULL &&
	    (d->is_call_id_owner() || no_fork_req_disposition)) 
	{
		early_matched = true;
		return true;
	}

	return false;
}

bool t_line::is_invite_retrans(t_request *r) {
	assert(r->method == INVITE);
	return (active_dialog && active_dialog->is_invite_retrans(r));
}

void t_line::process_invite_retrans(void) {
	if (active_dialog) active_dialog->process_invite_retrans();
}

string t_line::create_user_contact(const string &auto_ip) const {
	assert(phone_user);
	t_user *user_config = phone_user->get_user_profile();
	return user_config->create_user_contact(hide_user, auto_ip);
}

string t_line::create_user_uri(void) const {
	assert(phone_user);
	t_user *user_config = phone_user->get_user_profile();
	return user_config->create_user_uri(hide_user);
}

t_response *t_line::create_options_response(t_request *r, bool in_dialog) const
{
	assert(phone_user);
	t_user *user_config = phone_user->get_user_profile();
	return phone->create_options_response(user_config, r, in_dialog);
}

void t_line::send_response(t_response *r, t_tuid tuid, t_tid tid) {
	if (hide_user) {
		r->hdr_privacy.add_privacy(PRIVACY_ID);
	}
	phone->send_response(r, tuid, tid);
}

void t_line::send_request(t_request *r, t_tuid tuid) {
	assert(phone_user);
	t_user *user_config = phone_user->get_user_profile();
	phone->send_request(user_config, r, tuid);
}

t_phone *t_line::get_phone(void) const {
	return phone;
}

unsigned short t_line::get_line_number(void) const {
	return line_number;
}

bool t_line::get_is_on_hold(void) const {
	return is_on_hold;
}

bool t_line::get_is_muted(void) const {
	return is_muted;
}

bool t_line::get_hide_user(void) const {
	return hide_user;
}

bool t_line::get_is_transfer_consult(unsigned short &lineno) const {
	lineno = consult_transfer_from_line;
	return is_transfer_consult;
}

void t_line::set_is_transfer_consult(bool enable, unsigned short lineno) {
	is_transfer_consult = enable;
	consult_transfer_from_line = lineno;
}

bool t_line::get_to_be_transferred(unsigned short &lineno) const {
	lineno = consult_transfer_to_line;
	return to_be_transferred;
}

void t_line::set_to_be_transferred(bool enable, unsigned short lineno) {
	to_be_transferred = enable;
	consult_transfer_to_line = lineno;
}

bool t_line::get_is_encrypted(void) const {
	t_audio_session *as = get_audio_session();
	if (as) return as->get_is_encrypted();
	return false;
}

bool t_line::get_try_to_encrypt(void) const {
	return try_to_encrypt;
}

bool t_line::get_auto_answer(void) const {
	return auto_answer;
}

void t_line::set_auto_answer(bool enable) {
	auto_answer = enable;
}

bool t_line::is_refer_succeeded(void) const {
	if (active_dialog) return active_dialog->refer_succeeded;
	return false;
}

bool t_line::has_media(void) const {
	t_session *session = get_session();
	return (session && !session->receive_host.empty() && !session->dst_rtp_host.empty());
}

t_url t_line::get_remote_target_uri(void) const {
	if (!active_dialog) return t_url();
	return active_dialog->get_remote_target_uri();
}

t_url t_line::get_remote_target_uri_pending(void) const {
	if (pending_dialogs.empty()) return t_url();
	return pending_dialogs.front()->get_remote_target_uri();
}

string t_line::get_remote_target_display(void) const {
	if (!active_dialog) return "";
	return active_dialog->get_remote_target_display();
}

string t_line::get_remote_target_display_pending(void) const {
	if (pending_dialogs.empty()) return "";
	return pending_dialogs.front()->get_remote_target_display();
}

t_url t_line::get_remote_uri(void) const {
	if (!active_dialog) return t_url();
	return active_dialog->get_remote_uri();
}

t_url t_line::get_remote_uri_pending(void) const {
	if (pending_dialogs.empty()) return t_url();
	return pending_dialogs.front()->get_remote_uri();
}

string t_line::get_remote_display(void) const {
	if (!active_dialog) return "";
	return active_dialog->get_remote_display();
}

string t_line::get_remote_display_pending(void) const {
	if (pending_dialogs.empty()) return "";
	return pending_dialogs.front()->get_remote_display();
}

string t_line::get_call_id(void) const {
	if (!active_dialog) return "";
	return active_dialog->get_call_id();
}

string t_line::get_call_id_pending(void) const {
	if (pending_dialogs.empty()) return "";
	return pending_dialogs.front()->get_call_id();
}

string t_line::get_local_tag(void) const {
	if (!active_dialog) return "";
	return active_dialog->get_local_tag();
}

string t_line::get_local_tag_pending(void) const {
	if (pending_dialogs.empty()) return "";
	return pending_dialogs.front()->get_local_tag();
}

string t_line::get_remote_tag(void) const {
	if (!active_dialog) return "";
	return active_dialog->get_remote_tag();
}

string t_line::get_remote_tag_pending(void) const {
	if (pending_dialogs.empty()) return "";
	return pending_dialogs.front()->get_remote_tag();
}

bool t_line::remote_extension_supported(const string &extension) const {
	if (!active_dialog) return false;
	return active_dialog->remote_extension_supported(extension);
}

bool t_line::seize(void) {
	// Only an idle line can be seized.
	if (substate != LSSUB_IDLE) return false;

	substate = LSSUB_SEIZED;
	ui->cb_line_state_changed();
	
	return true;
}

void t_line::unseize(void) {
	// Only a seized line can be unseized.
	if (substate != LSSUB_SEIZED) return;

	substate = LSSUB_IDLE;
	ui->cb_line_state_changed();
}

t_session *t_line::get_session(void) const {
	if (!active_dialog) return NULL;

	return active_dialog->get_session();
}

t_audio_session *t_line::get_audio_session(void) const {
	if (!active_dialog) return NULL;

	return active_dialog->get_audio_session();
}

void t_line::notify_refer_progress(t_response *r) {
	if (active_dialog) active_dialog->notify_refer_progress(r);
}

void t_line::failed_retrieve(void) {
	// Call retrieve failed, so line is still on-hold
	is_on_hold = true;
	ui->cb_line_state_changed();
}

void t_line::failed_hold(void) {
	// Call hold failed, so line is not on-hold
	is_on_hold = false;
	ui->cb_line_state_changed();
}

void t_line::retry_retrieve_succeeded(void) {
	// Retry of retrieve succeeded, so line is not on-hold anymore
	is_on_hold = false;
	ui->cb_line_state_changed();
}

t_call_info t_line::get_call_info(void) const {
	return call_info;
}

void t_line::ci_set_dtmf_supported(bool supported, bool inband, bool info) {
	call_info.dtmf_supported = supported;
	call_info.dtmf_inband = inband;
	call_info.dtmf_info = info;
}

void t_line::ci_set_last_provisional_reason(const string &reason) {
	call_info.last_provisional_reason = reason;
}

void t_line::ci_set_send_codec(t_audio_codec codec) {
	call_info.send_codec = codec;
}

void t_line::ci_set_recv_codec(t_audio_codec codec) {
	call_info.recv_codec = codec;
}

void t_line::ci_set_refer_supported(bool supported) {
	call_info.refer_supported = supported;
}

void t_line::init_rtp_port(void) {
	rtp_port = sys_config->get_rtp_port() + line_number * 2;
}

unsigned short t_line::get_rtp_port(void) const {
	return rtp_port;
}

t_user *t_line::get_user(void) const {
	t_user *user_config = NULL;
	
	if (phone_user) {
		user_config = phone_user->get_user_profile();
	}
	
	return user_config;
}

t_phone_user *t_line::get_phone_user(void) const {
	return phone_user;
}

string t_line::get_ringtone(void) const {
	assert(phone_user);
	t_user *user_config = phone_user->get_user_profile();
	
	if (!user_defined_ringtone.empty()) {
		// Ring tone returned by incoming call script
		return user_defined_ringtone;
	} else if (!user_config->get_ringtone_file().empty()) {
		// Ring tone from user profile
		return user_config->get_ringtone_file();
	} else if (!sys_config->get_ringtone_file().empty()) {
		// Ring tone from system settings
		return sys_config->get_ringtone_file();
	} else {
		// Twinkle default
		return FILE_RINGTONE;
	}	
}

void t_line::confirm_zrtp_sas(void) {
	t_audio_session *as = get_audio_session();
	
	if (as && !as->get_zrtp_sas_confirmed()) {
		as->confirm_zrtp_sas();
		ui->cb_zrtp_sas_confirmed(line_number);
		ui->cb_line_state_changed();
		log_file->write_header("t_line::confirm_zrtp_sas");
		log_file->write_raw("Line ");
		log_file->write_raw(line_number + 1);
		log_file->write_raw(": User confirmed ZRTP SAS\n");
		log_file->write_footer();
	}
}

void t_line::reset_zrtp_sas_confirmation(void) {
	t_audio_session *as = get_audio_session();
	
	if (as && as->get_zrtp_sas_confirmed()) {
		as->reset_zrtp_sas_confirmation();
		ui->cb_zrtp_sas_confirmation_reset(line_number);
		ui->cb_line_state_changed();
		log_file->write_header("t_line::reset_zrtp_sas_confirmation");
		log_file->write_raw("Line ");
		log_file->write_raw(line_number + 1);
		log_file->write_raw(": User reset ZRTP SAS confirmation\n");
		log_file->write_footer();
	}
}

void t_line::enable_zrtp(void) {
	t_audio_session *as = get_audio_session();
	if (as) {
		as->enable_zrtp();
	}
}

void t_line::zrtp_request_go_clear(void) {
	t_audio_session *as = get_audio_session();
	if (as) {
		as->zrtp_request_go_clear();
	}
}

void t_line::zrtp_go_clear_ok(void) {
	t_audio_session *as = get_audio_session();
	if (as) {
		as->zrtp_go_clear_ok();
	}
}

void t_line::force_idle(void) {
	cleanup_forced();
}

void t_line::set_keep_seized(bool seize) {
	keep_seized = seize;
	cleanup();
}

bool t_line::get_keep_seized(void) const {
	return keep_seized;
}

t_dialog *t_line::get_dialog_with_active_session(void) const {
	if (open_dialog && open_dialog->has_active_session()) {
		return open_dialog;
	}
	
	if (active_dialog && active_dialog->has_active_session()) {
		return active_dialog;
	}
	
	for (list<t_dialog *>::const_iterator it = pending_dialogs.begin();
	     it != pending_dialogs.end(); ++it)
	{
		if ((*it)->has_active_session()) {
			return *it;
		}
	}
	
	for (list<t_dialog *>::const_iterator it = dying_dialogs.begin();
	     it != dying_dialogs.end(); ++it)
	{
		if ((*it)->has_active_session()) {
			return *it;
		}
	}
	
	return NULL;
}
