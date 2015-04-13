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

#ifndef _H_MUTEX
#define _H_MUTEX

#include <errno.h>
#include <pthread.h>

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

#endif
