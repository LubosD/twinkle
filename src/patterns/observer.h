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

/**
 * @file
 * Abstract observer pattern.
 */

#ifndef _OBSERVER_H
#define _OBSERVER_H

#include <list>

#include "threads/mutex.h"

using namespace std;

namespace patterns {

/** Observer. */
class t_observer {
public:
	virtual ~t_observer() {};
	
	/**
	 * This method is called by an observed subject to indicate its state
	 * has changed.
	 */
	virtual void update(void) = 0;
	
	/** 
	 * This method is called when the subject is destroyed.
	 */
	virtual void subject_destroyed(void) = 0;
};

/** An observed subject. */
class t_subject {
private:
	/** Mutex to protect access to the observers. */
	mutable t_recursive_mutex mtx_observers;

	list<t_observer *> observers;	/** Observers of this subject. */
	
public:
	virtual ~t_subject();
	
	/**
	 * Attach an observer.
	 * @param o [in] The observer.
	 */
	void attach(t_observer *o);
	
	/**
	 * Detach an observer.
	 * @param o [in] The observer.
	 */
	void detach(t_observer *o);
	
	/**
	 * Notify all observers.
	 */
	void notify(void) const;
};

}; // namespace

#endif
