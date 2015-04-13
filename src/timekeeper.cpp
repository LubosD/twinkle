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
#include <sys/time.h>
#include <iostream>
#include <signal.h>
#include "events.h"
#include "line.h"
#include "log.h"
#include "phone.h"
#include "subscription.h"
#include "timekeeper.h"
#include "transaction_mgr.h"
#include "threads/thread.h"
#include "audits/memman.h"

extern t_phone		*phone;
extern t_event_queue	*evq_trans_layer;
extern t_event_queue	*evq_trans_mgr;
extern t_event_queue	*evq_timekeeper;
extern t_timekeeper	*timekeeper;
extern bool		threading_is_LinuxThreads;

string timer_type2str(t_timer_type t) {
	switch(t) {
	case TMR_TRANSACTION:	return "TMR_TRANSACTION";
	case TMR_PHONE:		return "TMR_PHONE";
	case TMR_LINE:		return "TMR_LINE";
	case TMR_SUBSCRIBE:	return "TMR_SUBSCRIBE";
	case TMR_PUBLISH:	return "TMR_PUBLISH";
	case TMR_STUN_TRANSACTION: return "TMR_STUN_TRANSACTION";
	}

	return "UNKNOWN";
}

///////////////////////////////////////////////////////////
// class t_timer
///////////////////////////////////////////////////////////

t_timer::t_timer(long dur) : t_id_object() {
	long d = dur;
	
	// HACK: if a timer is set to zero seconds, set it to 1 ms, otherwise
	//       the timer will not expire.
	if (dur == 0) d++;

	duration = d;
	relative_duration = d;
}

long t_timer::get_duration(void) const {
	return duration;
}

long t_timer::get_relative_duration(void) const {
	return relative_duration;
}

void t_timer::set_relative_duration(long d) {
	relative_duration = d;
}

///////////////////////////////////////////////////////////
// class t_tmr_transaction
///////////////////////////////////////////////////////////

t_tmr_transaction::t_tmr_transaction(long dur, t_sip_timer tmr,
	unsigned short tid) : t_timer(dur)
{
	sip_timer = tmr;
	transaction_id = tid;
}

void t_tmr_transaction::expired(void) {
	// Create a timeout event for the transaction manager
	evq_trans_mgr->push_timeout(this);
}

t_timer *t_tmr_transaction::copy(void) const {
	t_tmr_transaction *t = new t_tmr_transaction(*this);
	MEMMAN_NEW(t);
	return t;
}

t_timer_type t_tmr_transaction::get_type(void) const {
	return TMR_TRANSACTION;
}

unsigned short t_tmr_transaction::get_tid(void) const {
	return transaction_id;
}

t_sip_timer t_tmr_transaction::get_sip_timer(void) const {
	return sip_timer;
}

string t_tmr_transaction::get_name(void) const {
	switch(sip_timer) {
	case TIMER_T1:	return "TIMER_T1";
	case TIMER_T2:	return "TIMER_T2";
	case TIMER_T4:	return "TIMER_T4";
	case TIMER_A:	return "TIMER_A";
	case TIMER_B:	return "TIMER_B";
	case TIMER_C:	return "TIMER_C";
	case TIMER_D:	return "TIMER_D";
	case TIMER_E:	return "TIMER_E";
	case TIMER_F:	return "TIMER_F";
	case TIMER_G:	return "TIMER_G";
	case TIMER_H:	return "TIMER_H";
	case TIMER_I:	return "TIMER_I";
	case TIMER_J:	return "TIMER_J";
	case TIMER_K:	return "TIMER_K";
	}

	return "UNKNOWN";
}

///////////////////////////////////////////////////////////
// class t_tmr_phone
///////////////////////////////////////////////////////////
t_tmr_phone::t_tmr_phone(long dur, t_phone_timer ptmr, t_phone *p) : t_timer(dur)
{
	phone_timer = ptmr;
	the_phone = p;
}

void t_tmr_phone::expired(void) {
	evq_trans_layer->push_timeout(this);
}

t_timer *t_tmr_phone::copy(void) const {
	t_tmr_phone *t = new t_tmr_phone(*this);
	MEMMAN_NEW(t);
	return t;
}

t_timer_type t_tmr_phone::get_type(void) const {
	return TMR_PHONE;
}

t_phone_timer t_tmr_phone::get_phone_timer(void) const {
	return phone_timer;
}

t_phone *t_tmr_phone::get_phone(void) const {
	return the_phone;
}

string t_tmr_phone::get_name(void) const {
	switch(phone_timer) {
	case PTMR_REGISTRATION:		return "PTMR_REGISTRATION";
	case PTMR_NAT_KEEPALIVE: 	return "PTMR_NAT_KEEPALIVE";
	case PTMR_TCP_PING:		return "PTMR_TCP_PING";
	}

	return "UNKNOWN";
}

///////////////////////////////////////////////////////////
// class t_tmr_line
///////////////////////////////////////////////////////////
t_tmr_line::t_tmr_line(long dur, t_line_timer ltmr, t_object_id lid,
				t_object_id d) : t_timer(dur)
{
	line_timer = ltmr;
	line_id = lid;
	dialog_id = d;
}

void t_tmr_line::expired(void) {
	evq_trans_layer->push_timeout(this);
}

t_timer *t_tmr_line::copy(void) const {
	t_tmr_line *t = new t_tmr_line(*this);
	MEMMAN_NEW(t);
	return t;
}

t_timer_type t_tmr_line::get_type(void) const {
	return TMR_LINE;
}

t_line_timer t_tmr_line::get_line_timer(void) const {
	return line_timer;
}

t_object_id t_tmr_line::get_line_id(void) const {
	return line_id;
}

t_object_id t_tmr_line::get_dialog_id(void) const {
	return dialog_id;
}

string t_tmr_line::get_name(void) const {
	switch(line_timer) {
	case LTMR_ACK_TIMEOUT:		return "LTMR_ACK_TIMEOUT";
	case LTMR_ACK_GUARD:		return "LTMR_ACK_GUARD";
	case LTMR_INVITE_COMP:		return "LTMR_INVITE_COMP";
	case LTMR_NO_ANSWER:		return "LTMR_NO_ANSWER";
	case LTMR_RE_INVITE_GUARD:	return "LTMR_RE_INVITE_GUARD";
	case LTMR_100REL_TIMEOUT:	return "LTMR_100REL_TIMEOUT";
	case LTMR_100REL_GUARD:		return "LTMR_100REL_GUARD";
	case LTMR_CANCEL_GUARD:		return "LTMR_CANCEL_GUARD";
	case LTMR_GLARE_RETRY:		return "LTMR_GLARE_RETRY";
	}

	return "UNKNOWN";
}

///////////////////////////////////////////////////////////
// class t_tmr_subscribe
///////////////////////////////////////////////////////////
t_tmr_subscribe::t_tmr_subscribe(long dur, t_subscribe_timer stmr,
		t_object_id lid, t_object_id d, const string &event_type,
		const string &event_id) : t_timer(dur)
{
	subscribe_timer = stmr;
	line_id = lid;
	dialog_id = d;
	sub_event_type = event_type;
	sub_event_id = event_id;
}

void t_tmr_subscribe::expired(void) {
	evq_trans_layer->push_timeout(this);
}

t_timer *t_tmr_subscribe::copy(void) const {
	t_tmr_subscribe *t = new t_tmr_subscribe(*this);
	MEMMAN_NEW(t);
	return t;
}

t_timer_type t_tmr_subscribe::get_type(void) const {
	return TMR_SUBSCRIBE;
}

t_subscribe_timer t_tmr_subscribe::get_subscribe_timer(void) const {
	return subscribe_timer;
}

t_object_id t_tmr_subscribe::get_line_id(void) const {
	return line_id;
}

t_object_id t_tmr_subscribe::get_dialog_id(void) const {
	return dialog_id;
}

string t_tmr_subscribe::get_sub_event_type(void) const {
	return sub_event_type;
}

string t_tmr_subscribe::get_sub_event_id(void) const {
	return sub_event_id;
}

string t_tmr_subscribe::get_name(void) const {
	switch(subscribe_timer) {
	case STMR_SUBSCRIPTION:	return "STMR_SUBSCRIPTION";
	}

	return "UNKNOWN";
}

///////////////////////////////////////////////////////////
// class t_tmr_publish
///////////////////////////////////////////////////////////
t_tmr_publish::t_tmr_publish(long dur, t_publish_timer ptmr, const string &_event_type) :
	t_timer(dur),
	publish_timer(ptmr),
	event_type(_event_type)
{}

void t_tmr_publish::expired(void) {
	evq_trans_layer->push_timeout(this);
}

t_timer *t_tmr_publish::copy(void) const {
	t_tmr_publish *t = new t_tmr_publish(*this);
	MEMMAN_NEW(t);
	return t;
}

t_timer_type t_tmr_publish::get_type(void) const {
	return TMR_PUBLISH;
}

t_publish_timer t_tmr_publish::get_publish_timer(void) const {
	return publish_timer;
}

string t_tmr_publish::get_name(void) const {
	switch (publish_timer) {
	case PUBLISH_TMR_PUBLICATION: return "PUBLISH_TMR_PUBLICATION";
	}
	
	return "UNKNOWN";
}

///////////////////////////////////////////////////////////
// class t_tmr_stun_trans
///////////////////////////////////////////////////////////

t_tmr_stun_trans::t_tmr_stun_trans(long dur, t_stun_timer tmr,
	unsigned short tid) : t_timer(dur)
{
	stun_timer = tmr;
	transaction_id = tid;
}

void t_tmr_stun_trans::expired(void) {
	// Create a timeout event for the transaction manager
	evq_trans_mgr->push_timeout(this);
}

t_timer *t_tmr_stun_trans::copy(void) const {
	t_tmr_stun_trans *t = new t_tmr_stun_trans(*this);
	MEMMAN_NEW(t);
	return t;
}

t_timer_type t_tmr_stun_trans::get_type(void) const {
	return TMR_STUN_TRANSACTION;
}

unsigned short t_tmr_stun_trans::get_tid(void) const {
	return transaction_id;
}

t_stun_timer t_tmr_stun_trans::get_stun_timer(void) const {
	return stun_timer;
}

string t_tmr_stun_trans::get_name(void) const {
	switch(stun_timer) {
	case STUN_TMR_REQ_TIMEOUT:	return "STUN_TMR_REQ_TIMEOUT";
	}

	return "UNKNOWN";
}

///////////////////////////////////////////////////////////
// class t_timekeeper
///////////////////////////////////////////////////////////

t_timekeeper::t_timekeeper() : mutex() {
	stopped = false;
	timer_expired = false;
}

void t_timekeeper::start(void (*timeout_handler)(int)) {
	signal(SIGALRM, timeout_handler);
}

t_timekeeper::~t_timekeeper() {
	struct itimerval	itimer;

	mutex.lock();

	log_file->write_header("t_timekeeper::~t_timekeeper",
		LOG_NORMAL, LOG_INFO);
	log_file->write_raw("Clean up timekeeper.\n");

	// Stop timers
	itimer.it_interval.tv_sec = 0;
	itimer.it_interval.tv_usec = 0;
	itimer.it_value.tv_sec = 0;
	itimer.it_value.tv_usec = 0;
	setitimer(ITIMER_REAL, &itimer, NULL);

	for (list<t_timer *>::iterator i = timer_list.begin();
	     i != timer_list.end(); i++)
	{
		log_file->write_raw("\nDeleting timer:\n");
		log_file->write_raw("Id: ");
		log_file->write_raw((*i)->get_object_id());
		log_file->write_raw(", Type: ");
		log_file->write_raw(timer_type2str((*i)->get_type()));
		log_file->write_raw(", Timer: ");
		log_file->write_raw((*i)->get_name());
		log_file->write_raw("\nDuration: ");
		log_file->write_raw((*i)->get_duration());
		log_file->write_raw(", Relative duration: ");
		log_file->write_raw((*i)->get_relative_duration());
		log_file->write_endl();
		if ((*i)->get_type() == TMR_TRANSACTION) {
			log_file->write_raw("Transaction id: ");
			log_file->write_raw(
				((t_tmr_transaction *)(*i))->get_tid());
			log_file->write_endl();
		}
		MEMMAN_DELETE(*i);
		delete *i;
	}

	if (threading_is_LinuxThreads) {
		signal(SIGALRM, SIG_DFL);
	}

	log_file->write_footer();

	mutex.unlock();
}

void t_timekeeper::lock(void) {
	mutex.lock();
}

void t_timekeeper::unlock(void) {
	mutex.unlock();

	if (timer_expired) {
		timer_expired = false;
		report_expiry();
	}
}

void t_timekeeper::start_timer(t_timer *t) {
	struct itimerval	itimer;
	long			remain_msec;

	lock();

	// The next interval option is not used
	itimer.it_interval.tv_sec = 0;
	itimer.it_interval.tv_usec = 0;

	// Get duration of the timer to start
	long d = t->get_relative_duration();

	// If no timer is currently running then simply start the timer
	if (timer_list.empty()) {
		timer_list.push_back(t);
		itimer.it_value.tv_sec = d / 1000;
		itimer.it_value.tv_usec = (d % 1000) * 1000;
		setitimer(ITIMER_REAL, &itimer, NULL);

		unlock();
		return;
	}

	// Get remaining duration of current running timer
	getitimer(ITIMER_REAL, &itimer);
	remain_msec = itimer.it_value.tv_sec * 1000 +
		      itimer.it_value.tv_usec / 1000;

	// If the new timer is shorter than the current timer.
	// then the new timer should be run first.
	if (d < remain_msec) {
		// Change running timer to new timer
		itimer.it_value.tv_sec = d / 1000;
		itimer.it_value.tv_usec = (d % 1000) * 1000;
		setitimer(ITIMER_REAL, &itimer, NULL);

		// Calculate the relative duration the timer
		// that was running.
		t_timer *old_timer = timer_list.front();
		old_timer->set_relative_duration(remain_msec - d);

		// Add new timer at the front of the list
		timer_list.push_front(t);

		unlock();
		return;
	}

	// Calculate the relative duration for the new timer
	long new_duration = d - remain_msec;

	// Insert the new timer at the right position in the list.
	list<t_timer *>::iterator i;
	for (i = timer_list.begin(); i != timer_list.end(); i++)
	{
		// skip the first timer
		if (i == timer_list.begin()) continue;

		long dur = (*i)->get_relative_duration();
		if (new_duration < dur) {
			// Adjust relative duration existing timer
			(*i)->set_relative_duration(dur - new_duration);

			// Insert new timer before existing timer
			t->set_relative_duration(new_duration);
			timer_list.insert(i, t);

			unlock();
			return;
		}

		new_duration -= dur;
	}

	// Add the new timer to the end of the list
	t->set_relative_duration(new_duration);
	timer_list.push_back(t);

	unlock();
}

void t_timekeeper::stop_timer(t_object_id id) {
	struct itimerval	itimer;
	long			remain_msec;

	lock();

	// The next interval option is not used
	itimer.it_interval.tv_sec = 0;
	itimer.it_interval.tv_usec = 0;


	if (timer_list.empty()) {
		// Timer already expired or stopped
		unlock();
		return;
	}

	// Find timer
	list<t_timer *>::iterator i = timer_list.begin();
	while (i != timer_list.end()) {
		if ((*i)->get_object_id() == id) break;
		i++;
	}

	if (i == timer_list.end()) {
		// Timer already expired or stopped.
		unlock();
		return;
	}

	// If it is the current running timer, then it must be stopped
	if (i == timer_list.begin()) {
		getitimer(ITIMER_REAL, &itimer);

		// If remaining time is less then 100 msec then let it
		// expire to prevent race condition when timer expires
		// while stopping it now.
		remain_msec = itimer.it_value.tv_sec * 1000 +
			      itimer.it_value.tv_usec / 1000;
		if (remain_msec < 100) {
			stopped = true;
			unlock();
			return;
		}

		// Stop timer
		itimer.it_value.tv_sec = 0;
		itimer.it_value.tv_usec = 0;
		setitimer(ITIMER_REAL, &itimer, NULL);

		// Remove the timer
		MEMMAN_DELETE(timer_list.front());
		delete timer_list.front();
		timer_list.pop_front();

		// If a next timer exists then adjust its relative
		// duration and start it.
		if (!timer_list.empty()) {
			t_timer *next_timer = timer_list.front();
			long dur = next_timer->get_relative_duration();
			dur += remain_msec;
			next_timer->set_relative_duration(dur);
			itimer.it_value.tv_sec = dur / 1000;
			itimer.it_value.tv_usec = (dur % 1000) * 1000;
			setitimer(ITIMER_REAL, &itimer, NULL);
		}

		unlock();
		return;
	}

	// Timer is not the current running timer, so delete it
	// and adjust relative duration of the next timer.
	list<t_timer *>::iterator next = i;
	next++;

	if (next == timer_list.end()) {
		// There is no next timer
		MEMMAN_DELETE(timer_list.back());
		delete timer_list.back();
		timer_list.pop_back();
		unlock();
		return;
	}

	long dur = (*i)->get_relative_duration();
	long dur_next = (*next)->get_relative_duration();
	(*next)->set_relative_duration(dur + dur_next);
	MEMMAN_DELETE(*i);
	delete *i;
	timer_list.erase(i);

	unlock();
}

void t_timekeeper::report_expiry(void) {
	lock();
	
	if (timer_list.empty()) {
		unlock();
		return;
	}
	
	t_timer *t = timer_list.front();

	// Trigger action if timer was not stopped
	if (!stopped) {
		t->expired();
	}
	stopped = false;

	// Remove the timer
	MEMMAN_DELETE(timer_list.front());
	delete timer_list.front();
	timer_list.pop_front();

	if (timer_list.empty()) {
		unlock();
		return;
	}

	// If the relative duration of the next timer is 0, then
	// it also expired. Action should be triggerd. If not, then
	// it should be started.
	t_timer *next = timer_list.front();
	long dur = next->get_relative_duration();
	while (dur == 0) {
		next->expired();
		MEMMAN_DELETE(next);
		delete next;
		timer_list.pop_front();
		if (timer_list.empty()) break;
		next = timer_list.front();
		dur = next->get_relative_duration();
	}

	if (!timer_list.empty()) {
		struct itimerval	itimer;

		itimer.it_interval.tv_sec = 0;
		itimer.it_interval.tv_usec = 0;
		itimer.it_value.tv_sec = dur / 1000;
		itimer.it_value.tv_usec = (dur % 1000) * 1000;
		setitimer(ITIMER_REAL, &itimer, NULL);
	}

	unlock();
}

unsigned long t_timekeeper::get_remaining_time(t_object_id timer_id) {
	struct itimerval	itimer;
	unsigned long		remain_msec = 0;
	unsigned long		duration = 0;

	lock();

	// The next interval option is not used
	itimer.it_interval.tv_sec = 0;
	itimer.it_interval.tv_usec = 0;

	// Get remaining duration of current running timer
	getitimer(ITIMER_REAL, &itimer);
	remain_msec = itimer.it_value.tv_sec * 1000 +
		      itimer.it_value.tv_usec / 1000;

	// Find the timer
	list<t_timer *>::iterator i = timer_list.begin();
	while (i != timer_list.end()) {
		if (i != timer_list.begin()) {
			remain_msec += (*i)->get_relative_duration();
		}

		if ((*i)->get_object_id() == timer_id) break;

		i++;
	}

	// Return duration to originator of get event
	if (i == timer_list.end()) {
		duration = 0;
	} else {
		duration = remain_msec;
	}

	unlock();
	return duration;
}

// SIGALRM handler
void timeout_handler(int signum) {
	signal(SIGALRM, timeout_handler);
	// timekeeper.report_expiry();

	// This will signal an interrupt to the call to pop in the
	// main look t_timekeeper::run
	evq_timekeeper->interrupt();
}

void t_timekeeper::run(void) {
	t_event			*event;
	t_event_start_timer	*ev_start;
	t_event_stop_timer	*ev_stop;
	bool			timeout;
	
	// The timekeeper should not try to take the phone lock as
	// it may lead to a deadlock. Make sure an assert is raised
	// if this situation ever happens.
	phone->add_prohibited_thread();

	if (threading_is_LinuxThreads) {
		// In LinuxThreads SIGALRM caused by the expiration of a timer
		// started with setitimer is always delivered to the thread calling
		// setitimer. So the sigwait() call from another thread does not
		// work. Use a signal handler instead.
		start(timeout_handler);
	}

	bool quit = false;
	while (!quit) {
		event = evq_timekeeper->pop(timeout);

		if (timeout) {
			report_expiry();
			continue;
		}

		switch(event->get_type()) {
		case EV_START_TIMER:
			ev_start = (t_event_start_timer *)event;
			start_timer(ev_start->get_timer());
			break;
		case EV_STOP_TIMER:
			ev_stop = (t_event_stop_timer *)event;
			stop_timer(ev_stop->get_timer_id());
			break;
		case EV_QUIT:
			quit = true;
			break;
		default:
			assert(false);
		}

		MEMMAN_DELETE(event);
		delete event;
	}
}

void *timekeeper_main(void *arg) {
	timekeeper->run();
	return NULL;
}

void *timekeeper_sigwait(void *arg) {
	sigset_t	sigset;
	int		sig;

	sigemptyset(&sigset);
	sigaddset(&sigset, SIGALRM);

	while (true) {
		// When SIGCONT is received after SIGSTOP, sigwait returns
		// with EINTR ??
		if (sigwait(&sigset, &sig) == EINTR) continue;
		evq_timekeeper->interrupt();
	}
}
