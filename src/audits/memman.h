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

#ifndef _MEMMAN_H
#define _MEMMAN_H

#include <string>
#include <map>
#include "threads/mutex.h"

#define MEMMAN_NEW(ptr) 	 memman->trc_new((ptr), __FILE__, __LINE__)
#define MEMMAN_NEW_ARRAY(ptr) 	 memman->trc_new((ptr), __FILE__, __LINE__, true)
#define MEMMAN_DELETE(ptr)	 memman->trc_delete((ptr), __FILE__, __LINE__)
#define MEMMAN_DELETE_ARRAY(ptr) memman->trc_delete((ptr), __FILE__, __LINE__, true)
#define MEMMAN_REPORT 		 { memman->report_stats(); memman->report_leaks(); }

using namespace std;

// Memory manager
// Trace memory allocations and deallocations

class t_ptr_info {
public:
	// Src file from which pointer has been allocated
	string		filename;

	// Line number of memman trace command tracing this pointer
	int		lineno;

	// Indicates if the pointer points to an array
	bool		is_array;

	t_ptr_info() {};
	t_ptr_info(const string &_filename, int _lineno, bool _is_array);
};

class t_memman {
private:
	// Map of allocated pointers
	map<void *, t_ptr_info>	pointer_map;

	// Statistics
	unsigned long num_new; // number of new's
	unsigned long num_new_duplicate; // number of duplicate new's
	unsigned long num_delete; // number of delete's
	unsigned long num_delete_mismatch; // number of delete's for without a new
	unsigned long num_array_mixing; // number of array/non-array mixes

	// Mutex to protect operations on the memory manager
	t_mutex mtx_memman;

public:
	t_memman();

	// Report pointer allocation
	void trc_new(void *p, const string &filename, int lineno,
			bool is_array = false);

	// Report pointer deallocation
	void trc_delete(void *p, const string &filename, int lineno,
			bool is_array = false);

	// Write a memory leak report to log
	void report_leaks(void);

	// Write statistics to log
	void report_stats(void);
};

extern t_memman *memman;

#endif
