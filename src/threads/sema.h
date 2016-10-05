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

#ifndef _H_SEMAPHORE
#define _H_SEMAPHORE

#include <semaphore.h>

class t_semaphore {
private:
	sem_t	sem;

public:
	// Throws a string exception (error message) when failing.
	t_semaphore(unsigned int value);

	~t_semaphore();

	// Throws a string exception (error message) when failing.
	void up(void);

	void down(void);

	// Returns true if downing the semaphore succeeded.
	// Returns false if the semaphore was zero already.
	bool try_down(void);
};

#endif
