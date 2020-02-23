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

#include <string>
#include <iostream>
#include "mutex.h"
#include "thread.h"
#include <stdexcept>

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


///////////////////////////
// t_rwmutex
///////////////////////////

// Equivalent of an invalid thread ID, to be used when initializing t_rwmutex
// or when releasing upgrade ownership; the use of pthread_self() is only to
// provide a dummy value of the appropriate type.
static const optional_pthread_t invalid_thread_id = { false, pthread_self() };

t_rwmutex::t_rwmutex() :
	_up_mutex_thread( invalid_thread_id )
{
	int ret = pthread_rwlock_init(&_lock, nullptr);
	if (ret != 0) throw string(
		"t_rwmutex::t_rwmutex failed to create a r/w mutex.");
}

t_rwmutex::~t_rwmutex()
{
	pthread_rwlock_destroy(&_lock);
}

void t_rwmutex::getUpgradeOwnership()
{
	_up_mutex.lock();
	_up_mutex_thread = { true, pthread_self() };
}

void t_rwmutex::releaseUpgradeOwnership()
{
	_up_mutex_thread = invalid_thread_id;
	_up_mutex.unlock();
}

bool t_rwmutex::isUpgradeOwnershipOurs() const
{
	// Note that we don't need a mutex over _up_mutex_thread, being atomic
	// is enough for our purposes.  (We don't care about *who* owns
	// _up_mutex, only about whether or not *we* own it, a fact which only
	// our own thread can modify.)
	optional_pthread_t lockOwner = _up_mutex_thread;
	return lockOwner.has_value && pthread_equal(lockOwner.value, pthread_self());
}

void t_rwmutex::_lockRead()
{
	int err = pthread_rwlock_rdlock(&_lock);
	if (err != 0)
		throw std::logic_error("Mutex lock failed");
}

void t_rwmutex::_lockWrite()
{
	int err = pthread_rwlock_wrlock(&_lock);
	if (err != 0)
		throw std::logic_error("Mutex lock failed");
}

void t_rwmutex::_unlock()
{
	pthread_rwlock_unlock(&_lock);
}

void t_rwmutex::lockRead()
{
	if (isUpgradeOwnershipOurs()) {
		throw std::logic_error("Acquiring read lock while holding update/write lock is not supported");
	}

	_lockRead();
}

void t_rwmutex::lockUpdate()
{
	if (isUpgradeOwnershipOurs()) {
		throw std::logic_error("Acquiring update lock while holding update/write lock is not supported");
	}

	getUpgradeOwnership();
	_lockRead();
}

void t_rwmutex::lockWrite()
{
	if (isUpgradeOwnershipOurs()) {
		throw std::logic_error("Acquiring write lock while holding update/write lock is not supported");
	}

	getUpgradeOwnership();
	_lockWrite();
}

void t_rwmutex::unlock()
{
	_unlock();

	if (isUpgradeOwnershipOurs()) {
		releaseUpgradeOwnership();
	}
}

void t_rwmutex::upgradeLock()
{
	if (!isUpgradeOwnershipOurs()) {
		throw std::logic_error("Attempting to upgrade a lock without upgrade ownership");
	}

	_unlock();
	_lockWrite();
}

void t_rwmutex::downgradeLock()
{
	if (!isUpgradeOwnershipOurs()) {
		throw std::logic_error("Attempting to downgrade a lock without upgrade ownership");
	}

	_unlock();
	_lockRead();
}

///////////////////////////
// t_rwmutex_guard
///////////////////////////

t_rwmutex_guard::t_rwmutex_guard(t_rwmutex& mutex) :
	_mutex(mutex),
	_previously_owned_upgrade(_mutex.isUpgradeOwnershipOurs())
{
}

t_rwmutex_reader::t_rwmutex_reader(t_rwmutex& mutex) :
	t_rwmutex_guard(mutex)
{
	// No-op if we are nested within a writer/future_writer guard
	if (!_previously_owned_upgrade) {
		_mutex.lockRead();
	}
}

t_rwmutex_reader::~t_rwmutex_reader()
{
	// No-op if we are nested within a writer/future_writer guard
	if (!_previously_owned_upgrade) {
		_mutex.unlock();
	}
}

t_rwmutex_future_writer::t_rwmutex_future_writer(t_rwmutex& mutex) :
	t_rwmutex_guard(mutex)
{
	// No-op if we are nested within a future_writer guard
	if (!_previously_owned_upgrade) {
		_mutex.lockUpdate();
	}
}

t_rwmutex_future_writer::~t_rwmutex_future_writer() {
	// No-op if we are nested within a future_writer guard
	if (!_previously_owned_upgrade) {
		_mutex.unlock();
	}
}

t_rwmutex_writer::t_rwmutex_writer(t_rwmutex& mutex) :
	t_rwmutex_guard(mutex)
{
	if (_previously_owned_upgrade) {
		// Writer nested inside a future_writer: upgrade lock
		_mutex.upgradeLock();
	} else {
		// Stand-alone writer guard
		_mutex.lockWrite();
	}
}

t_rwmutex_writer::~t_rwmutex_writer()
{
	if (_previously_owned_upgrade) {
		// We were nested within a future_writer guard, so return
		// the mutex to its previous state
		_mutex.downgradeLock();
	} else {
		_mutex.unlock();
	}
}
