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

#include "prohibit_thread.h"

void i_prohibit_thread::add_prohibited_thread(void) {
	prohibited_mutex.lock();
	prohibited_threads.insert(t_thread::self());
	prohibited_mutex.unlock();
}

void i_prohibit_thread::remove_prohibited_thread(void) {
	prohibited_mutex.lock();
	prohibited_threads.erase(t_thread::self());
	prohibited_mutex.unlock();
}

bool i_prohibit_thread::is_prohibited_thread(void) const {
	prohibited_mutex.lock();
	bool result = (prohibited_threads.find(t_thread::self()) != prohibited_threads.end());
	prohibited_mutex.unlock();
	
	return result;
}
