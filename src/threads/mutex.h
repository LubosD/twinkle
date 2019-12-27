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

#ifndef _H_MUTEX
#define _H_MUTEX

#include <errno.h>
#include <pthread.h>
// #include <iostream>
#include <atomic>

/**
 * @file
 * Mutex operations
 */

class t_mutex {
protected:
	pthread_mutex_t		mutex;

public:
	t_mutex();

	// Throws a string exception (error message) when failing.
	t_mutex(bool recursive);

	virtual ~t_mutex();

	// These methods throw a string exception when the operation
	// fails.
	virtual void lock(void);

	// Returns:
	// 0 - success
	// EBUSY - already locked
	// For other errors a string exception is thrown
	virtual int trylock(void);
	virtual void unlock(void);
};

class t_recursive_mutex : public t_mutex {
public:
	t_recursive_mutex();
	~t_recursive_mutex();
};


/** 
 * Guard pattern for a mutex .
 * The constructor of a guard locks a mutex and the destructor
 * unlocks it. This way a guard object can be created at entrance
 * of a function. Then at exit, the mutex is automically unlocked
 * as the guard object goes out of scope.
 */
class t_mutex_guard {
private:
	/** The guarding mutex. */
	t_mutex		&mutex_;
	
public:
	/**
	 * The constructor will lock the mutex.
	 * @param mutex [in] Mutex to lock.
	 */
	t_mutex_guard(t_mutex &mutex);
	
	/**
	 * The destructor will unlock the mutex.
	 */
	~t_mutex_guard();
};


// Read-write-update lock
//
// Read-write lock with an additional "update" type of lock, which can later
// be upgraded to "write" (and downgraded back to "update" again).  An update
// lock can co-exist with other read locks, but only one update lock can be
// held at any time, representing ownership of upgrade rights.
//
// See https://stackoverflow.com/a/18785300 for details and further references.
//
// Note that our version is rather simplistic, and does not allow downgrading
// from update/write to read.

// A cheap substitute for std::optional<pthread_t>, only available in C++14.
// Unfortunately, POSIX.1-2004 no longer requires pthread_t to be an arithmetic
// type, so we can't simply use 0 as an (unofficial) invalid thread ID.
struct optional_pthread_t {
	bool has_value;
	pthread_t value;
};

class t_rwmutex {
protected:
	// Standard read-write lock
	pthread_rwlock_t _lock;
	// Mutex for upgrade ownership
	t_mutex _up_mutex;
	// Thread ID that currently owns the _up_mutex lock, if any
	std::atomic<optional_pthread_t> _up_mutex_thread;

	// Get/release upgrade ownership
	void getUpgradeOwnership();
	void releaseUpgradeOwnership();

	// Internal methods to manipulate _lock directly
	void _lockRead();
	void _lockWrite();
	void _unlock();
public:
	t_rwmutex();
	~t_rwmutex();

	// Returns true if the calling thread currently owns the _up_mutex lock
	bool isUpgradeOwnershipOurs() const;

	// The usual methods for obtaining/releasing locks
	void lockRead();
	void lockUpdate();
	void lockWrite();
	void unlock();

	// Upgrade an update lock to a write lock, or downgrade in the
	// opposite direction.  Note that this does not count as an additional
	// lock, so only one unlock() call will be needed at the end.
	void upgradeLock();
	void downgradeLock();
};


// Equivalent of t_mutex_guard for t_rwmutex
//
// These can be nested as indicated below.  Note that nesting a weaker guard
// will not downgrade the lock; for example, a Reader guard within a Writer
// guard will maintain the write lock.

// Base (abstract) class
class t_rwmutex_guard {
protected:
	// The lock itself
	t_rwmutex& _mutex;

	// Whether or not we had upgrade ownership beforehand, indicating that
	// we are nested within the scope of a writer/future_writer guard
	bool _previously_owned_upgrade;

	// A protected constructor to keep this class abstract
	t_rwmutex_guard(t_rwmutex& mutex);
};

// Reader: Can be nested within the scope of any guard
class t_rwmutex_reader : public t_rwmutex_guard {
public:
	t_rwmutex_reader(t_rwmutex& mutex);
	~t_rwmutex_reader();
};

// Future writer: Can be nested within the scope of a future_writer guard
class t_rwmutex_future_writer : public t_rwmutex_guard {
public:
	t_rwmutex_future_writer(t_rwmutex& mutex);
	~t_rwmutex_future_writer();
};

// Writer: Can be nested within the scope of a future_writer guard
class t_rwmutex_writer : public t_rwmutex_guard {
public:
	t_rwmutex_writer(t_rwmutex& mutex);
	~t_rwmutex_writer();
};


#endif
