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

#include <string>
#include <iostream>
#include "mutex.h"
#include "thread.h"

using namespace std;

///////////////////////////
// t_mutex
///////////////////////////

t_mutex::t_mutex() {
	pthread_mutex_init(&mutex, NULL);
}

t_mutex::t_mutex(bool recursive) {
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);


	int ret = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
	if (ret != 0) throw string(
		"t_mutex::t_mutex failed to create a recursive mutex.");

	pthread_mutex_init(&mutex, &attr);
	pthread_mutexattr_destroy(&attr);
}

t_mutex::~t_mutex() {
	pthread_mutex_destroy(&mutex);
}

void t_mutex::lock(void) {
	int ret = pthread_mutex_lock(&mutex);
	if (ret != 0) throw string("t_mutex::lock failed.");
}

int t_mutex::trylock(void) {
	int ret = pthread_mutex_trylock(&mutex);
	switch (ret) {
	case 0:
	case EBUSY:
		return ret;
	default:
		throw string("t_mutex::trylock failed.");
	}
}

void t_mutex::unlock(void) {
	int ret = pthread_mutex_unlock(&mutex);
	if (ret != 0) throw ("t_mutex::unlock failed.");
}

///////////////////////////
// t_recursive_mutex
///////////////////////////

t_recursive_mutex::t_recursive_mutex() : t_mutex(true) {}

t_recursive_mutex::~t_recursive_mutex() {}

///////////////////////////
// t_guard_mutex
///////////////////////////

t_mutex_guard::t_mutex_guard(t_mutex &mutex) : mutex_(mutex) {
	mutex_.lock();
}

t_mutex_guard::~t_mutex_guard() {
	mutex_.unlock();
}
