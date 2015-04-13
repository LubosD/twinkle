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

#ifndef _PROHIBIT_THREAD_H
#define _PROHIBIT_THREAD_H

#include <set>
#include "threads/mutex.h"
#include "threads/thread.h"

using namespace std;

// This class implements an interface to keep track of thread id's that are
// prohibited from some actions, e.g. taking a certain lock

class i_prohibit_thread {
private:
	// List of thread id's that are prohibited from some action
	mutable t_mutex			prohibited_mutex;
	set<pthread_t>		prohibited_threads;
	
public:
	// Operations on the prohibited set of thread id's
	// Add/remove the thread id of the calling thread
	void add_prohibited_thread(void);
	void remove_prohibited_thread(void);
	
	// Returns true if the current thread is prohibited
	bool is_prohibited_thread(void) const;
};

#endif
