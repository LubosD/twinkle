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

#include <cerrno>
#include <cstring>
#include <string>
#include "sema.h"
#include "util.h"

using namespace std;

t_semaphore::t_semaphore(unsigned int value) {
	int ret;

	ret = sem_init(&sem, 0, value);
	if (ret != 0) {
		string err = get_error_str(errno);
		string exception =
			"t_semaphore::t_semaphore failed to create a semaphore.\n";
		exception += err;
		throw exception;
	}
}

t_semaphore::~t_semaphore() {
	sem_destroy(&sem);
}

void t_semaphore::up(void) {
	int ret;

	ret = sem_post(&sem);
	if (ret != 0) {
		string err = get_error_str(errno);
		string exception = "t_semaphore::up failed.\n";
		exception += err;
		throw exception;
	}
}

void t_semaphore::down(void) {
	while (true) {
		int ret = sem_wait(&sem);

		if (ret != 0 && errno == EINTR) {
			// In NPTL threading sem_wait can be interrupted.
			// In LinuxThreads threading sem_wait is non-interruptable.
			// Continue with sem_wait if an interrupt is caught.
			continue;
		}

		break;
	}
}

bool t_semaphore::try_down(void) {
	int ret;

	ret = sem_trywait(&sem);
	return (ret == 0);
}
