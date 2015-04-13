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

#include <signal.h>
#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <unistd.h>
#include "thread.h"

// Scratch variables for checking LinuxThreads vs NPTL
static pid_t pid_thread;

t_thread::t_thread(void *(*start_routine)(void *), void *arg) {
	int ret;

	ret = pthread_create(&tid, NULL, start_routine, arg);
	if (ret != 0) throw ret;
}

void t_thread::join(void) {
	int ret = pthread_join(tid, NULL);
	if (ret != 0) throw ret;
}

void t_thread::detach(void) {
	int ret = pthread_detach(tid);
	if (ret != 0) throw ret;
}

void t_thread::kill(void) {
	int ret = pthread_kill(tid, SIGKILL);
	if (ret != 0) throw ret;
}

void t_thread::cancel(void) {
	int ret = pthread_cancel(tid);
	if (ret != 0) throw ret;
}

void t_thread::set_sched_fifo(int priority) {
	struct sched_param sp;

	sp.sched_priority = priority;
	int ret = pthread_setschedparam(tid, SCHED_FIFO, &sp);
	if (ret != 0) throw ret;
}

pthread_t t_thread::get_tid(void) const {
	return tid;
}

pthread_t t_thread::self(void) {
	return pthread_self();
}

bool t_thread::is_equal(const t_thread &thr) const {
	return pthread_equal(tid, thr.get_tid());
}

bool t_thread::is_equal(const pthread_t &_tid) const {
	return pthread_equal(tid, _tid);
}

bool t_thread::is_self(void) const {
	return pthread_equal(tid, pthread_self());
}

bool t_thread::is_self(const pthread_t &_tid) {
	return pthread_equal(_tid, pthread_self());
}

void *check_threading_impl(void *arg) {
	pid_thread = getpid();
	pthread_exit(NULL);
}

bool t_thread::is_LinuxThreads(void) {
	t_thread *thr = new t_thread(check_threading_impl, NULL);

	try {
		thr->join();
	} catch (int) {
		// Thread is already terminated.
	}

	delete thr;

	// In LinuxThreads each thread has a different pid.
	// In NPTL all threads have the same pid.
	return (getpid() != pid_thread);
}

