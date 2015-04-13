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

#ifndef _TIMEKEEPER_H
#define _TIMEKEEPER_H

#include <list>
#include "id_object.h"
#include "protocol.h"
#include "transaction.h"
#include "threads/mutex.h"
#include "threads/sema.h"

using namespace std;

// Forward declarations
class t_phone;
class t_line;
class t_subscription;

/** Timer type */
enum t_timer_type {
	TMR_TRANSACTION,	/**< Transaction timer */
	TMR_PHONE,		/**< Timer associated with the phone */
	TMR_LINE,		/**< Timer associated with a line */
	TMR_SUBSCRIBE,		/**< Subscription timer */
	TMR_PUBLISH,		/**< Publication timer */
	TMR_STUN_TRANSACTION	/**< STUN timer */
};
////////////////////////////////////////////////////////////////
// General timer.
////////////////////////////////////////////////////////////////
// Instances should be created from subclasses.
class t_timer : public t_id_object {
private:
	long			duration; // milliseconds
	long			relative_duration; // milliseconds

public:
	t_timer(long dur);
	virtual ~t_timer() {}

	// This method is invoked on expiry
	// Subclasses should implent the action to be taken.
	virtual void expired(void) = 0;

	long get_duration(void) const;
	long get_relative_duration(void) const;
	void set_relative_duration(long d);
	virtual t_timer *copy(void) const = 0;
	virtual t_timer_type get_type(void) const = 0;

	// Get the name of the timer (for debugging purposes)
	virtual string get_name(void) const = 0;
};

////////////////////////////////////////////////////////////////
// Transaction timer
////////////////////////////////////////////////////////////////
class t_tmr_transaction : public t_timer {
private:
	unsigned short	transaction_id;
	t_sip_timer	sip_timer;

public:
	t_tmr_transaction(long dur, t_sip_timer tmr, unsigned short tid);

	void expired(void);
	t_timer *copy(void) const;
	t_timer_type get_type(void) const;
	unsigned short get_tid(void) const;
	t_sip_timer get_sip_timer(void) const;
	string get_name(void) const;
};

////////////////////////////////////////////////////////////////
// Phone timer
////////////////////////////////////////////////////////////////
class t_tmr_phone : public t_timer {
private:
	t_phone		*the_phone;
	t_phone_timer	phone_timer;

public:
	t_tmr_phone(long dur, t_phone_timer ptmr, t_phone *p);

	void expired(void);
	t_timer *copy(void) const;
	t_timer_type get_type(void) const;
	t_phone_timer get_phone_timer(void) const;
	t_phone *get_phone(void) const;
	string get_name(void) const;
};

////////////////////////////////////////////////////////////////
// Line timer
////////////////////////////////////////////////////////////////
class t_tmr_line : public t_timer {
private:
	t_object_id	line_id;
	t_line_timer	line_timer;
	t_object_id	dialog_id;

public:
	t_tmr_line(long dur, t_line_timer ltmr, t_object_id lid,
			t_object_id d);

	void expired(void);
	t_timer *copy(void) const;
	t_timer_type get_type(void) const;
	t_line_timer get_line_timer(void) const;
	t_object_id get_line_id(void) const;
	t_object_id get_dialog_id(void) const;
	string get_name(void) const;
};

////////////////////////////////////////////////////////////////
// Subscribe timer
////////////////////////////////////////////////////////////////
class t_tmr_subscribe : public t_timer {
private:
	t_subscribe_timer	subscribe_timer;
	t_object_id		line_id;
	t_object_id		dialog_id;
	string			sub_event_type;
	string			sub_event_id;


public:
	t_tmr_subscribe(long dur, t_subscribe_timer stmr, t_object_id lid, t_object_id d,
		const string &event_type, const string &event_id);

	void expired(void);
	t_timer *copy(void) const;
	t_timer_type get_type(void) const;
	t_subscribe_timer get_subscribe_timer(void) const;
	t_object_id get_line_id(void) const;
	t_object_id get_dialog_id(void) const;
	string get_sub_event_type(void) const;
	string get_sub_event_id(void) const;
	string get_name(void) const;
};

/** Publication timer */
class t_tmr_publish : public t_timer {
private:
	t_publish_timer		publish_timer;	/**< Type of timer */
	string			event_type;	/**< Event type of publication */


public:
	t_tmr_publish(long dur, t_publish_timer ptmr, const string &_event_type);

	void expired(void);
	t_timer *copy(void) const;
	t_timer_type get_type(void) const;
	t_publish_timer get_publish_timer(void) const;
	string get_name(void) const;
};

////////////////////////////////////////////////////////////////
// STUN transaction timer
////////////////////////////////////////////////////////////////
class t_tmr_stun_trans : public t_timer {
private:
	unsigned short	transaction_id;
	t_stun_timer	stun_timer;

public:
	t_tmr_stun_trans(long dur, t_stun_timer tmr, unsigned short tid);

	void expired(void);
	t_timer *copy(void) const;
	t_timer_type get_type(void) const;
	unsigned short get_tid(void) const;
	t_stun_timer get_stun_timer(void) const;
	string get_name(void) const;
};


////////////////////////////////////////////////////////////////
// Timekeeper
////////////////////////////////////////////////////////////////
// A timekeeper keeps track of multiple timers per thread.
// Only one single thread should call the methods of a single
// timekeeper. Multiple threads using the same timekeeper will
// cause havoc.

class t_timekeeper {
private:
	// List of running timers in order of timeout. As there
	// is only 1 real timer running on the OS. Each timer gets
	// a duration relative to its predecessor in the list.
	list<t_timer *>		timer_list;

	// Mutex to synchronize timekeeper actions.
	t_mutex			mutex;

	// Indicate if current timer was stopped, but not removed
	// to prevent race conditions. Expiry of a stopped timer
	// will not trigger any actions.
	bool			stopped;

	// Indicate if the current timer expired while the
	// mutex was locked.
	bool			timer_expired;

	// Every method should start with locking the timekeeper
	// and end with unlocking. The unlocking method will check
	// if a timer expired during the locked state. If so, then
	// the expiry will be processed.
	void lock(void);
	void unlock(void);

	// Start the timekeeper from the thread that will handle
	// the SIGALRM signal. Start must be called before the
	// timekeeper can be used.
	void start(void (*timeout_handler)(int));

	// Start a timer. The timer id is returned. This id is
	// needed to stop a timer. Pointer t should not be used
	// or deleted after starting. When the timer expires or
	// is stopped it will be deleted.
	void start_timer(t_timer *t);

	void stop_timer(t_object_id id);

public:
	// The timeout_handler must be a signal handler for SIGALRM
	t_timekeeper();
	~t_timekeeper();

	// Report that the current timer has expired.
	void report_expiry(void);

	// Get remaining time of a running timer.
	// Returns 0 if the timer is not running anymore.
	unsigned long get_remaining_time(t_object_id timer_id);

	// Main loop to be run in a separate thread
	void run(void);
};

// Entry function for timekeeper thread
void *timekeeper_main(void *arg);

// Entry function of the thread waiting for SIGALRM
// A dedicated thread is started to catch the SIGALRM signal and take
// the appropriate action. All threads must block the SIGALRM signal.
void *timekeeper_sigwait(void *arg);

#endif
