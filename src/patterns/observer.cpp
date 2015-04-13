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

#include "observer.h"

using namespace patterns;

t_subject::~t_subject() {
	mtx_observers.lock();
	for (list<t_observer *>::const_iterator it = observers.begin();
	     it != observers.end(); ++it)
	{
		(*it)->subject_destroyed();
	}
	mtx_observers.unlock();
}

void t_subject::attach(t_observer *o) {
	mtx_observers.lock();
	observers.push_back(o);
	mtx_observers.unlock();
}

void t_subject::detach(t_observer *o) {
	mtx_observers.lock();
	observers.remove(o);
	mtx_observers.unlock();
}

void t_subject::notify(void) const {
	mtx_observers.lock();
	for (list<t_observer *>::const_iterator it = observers.begin();
	     it != observers.end(); ++it)
	{
		(*it)->update();
	}
	mtx_observers.unlock();
}
