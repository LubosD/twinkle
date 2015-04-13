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

#ifndef _H_THREAD
#define _H_THREAD

#include <pthread.h>

class t_thread {
private:
	pthread_t	tid;		// Thread id

public:
	t_thread(void *(*start_routine)(void *), void *arg);

	// These methods throw an int (return value of libpthread function)
	// when they fail
	void join(void);
	void detach(void);
	void kill(void);
	void cancel(void);
	void set_sched_fifo(int priority);

	// Get thread id
	pthread_t get_tid(void) const;

	// Get thread id of current thread
	static pthread_t self(void);

	// Check if 2 threads are equal
	bool is_equal(const t_thread &thr) const;
	bool is_equal(const pthread_t &_tid) const;
	bool is_self(void) const;
	static bool is_self(const pthread_t &_tid);

	// Check if LinuxThreads or NPTL is active. This check is needed
	// for correctly handling signals. Signal handling in LinuxThreads
	// is quite different from signal handling in NPTL.
	// This checks creates a new thread and waits on its termination,
	// so you better cache its result for efficient future checks.
	static bool is_LinuxThreads(void);
};

#endif
