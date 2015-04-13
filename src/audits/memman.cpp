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

#include "memman.h"
#include "log.h"
#include "util.h"

//////////////////////
// class t_ptr_info
//////////////////////

t_ptr_info::t_ptr_info(const string &_filename, int _lineno, bool _is_array) :
		filename(_filename)
{
	lineno = _lineno;
	is_array = _is_array;
}

//////////////////////
// class t_memman
//////////////////////

t_memman::t_memman() {
	num_new = 0;
	num_new_duplicate = 0;
	num_delete = 0;
	num_delete_mismatch = 0;
	num_array_mixing = 0;
}

void t_memman::trc_new(void *p, const string &filename, int lineno,
		bool is_array)
{
	mtx_memman.lock();

	num_new++;

	// Check if pointer already exists
	map<void *, t_ptr_info>::iterator i;
	i = pointer_map.find(p);
	if (i != pointer_map.end()) {
		// Most likely this is an error in the usage of the
		// MEMMAN_NEW. A wrong pointer has been passed.
		num_new_duplicate++;
		
		// Unlock now. If memman gets called again via the log,
		// there will be no dead lock.
		mtx_memman.unlock();
		
		log_file->write_header("t_memman::trc_new",
			LOG_MEMORY, LOG_WARNING);
		log_file->write_raw(filename);
		log_file->write_raw(", line ");
		log_file->write_raw(lineno);
		log_file->write_raw(": pointer to ");
		log_file->write_raw(ptr2str(p));
		log_file->write_raw(" has already been allocated.\n");
		log_file->write_raw("It was allocated here: ");
		log_file->write_raw(i->second.filename);
		log_file->write_raw(", line ");
		log_file->write_raw(i->second.lineno);
		log_file->write_endl();
		log_file->write_footer();
		return;
	}

	t_ptr_info pinfo(filename, lineno, is_array);
	pointer_map[p] = pinfo;

	mtx_memman.unlock();
}

void t_memman::trc_delete(void *p, const string &filename, int lineno,
		bool is_array)
{
	mtx_memman.lock();

	num_delete++;

	map<void *, t_ptr_info>::iterator i;
	i = pointer_map.find(p);

	// Check if the pointer allocation has been reported
	if (i == pointer_map.end()) {
		num_delete_mismatch++;
		mtx_memman.unlock();
		
		log_file->write_header("t_memman::trc_delete",
			LOG_MEMORY, LOG_WARNING);
		log_file->write_raw(filename);
		log_file->write_raw(", line ");
		log_file->write_raw(lineno);
		log_file->write_raw(": pointer to ");
		log_file->write_raw(ptr2str(p));
		log_file->write_raw(" is deleted.\n");
		log_file->write_raw("This pointer is not allocated however.\n");
		log_file->write_footer();
		
		return;
	}


	bool array_mismatch = (is_array != i->second.is_array);

	// Check mixing of array new/delete
	// NOTE: after the pointer has been erased from pointer_map, the
	//       iterator i is invalid.
	//       The mutex mtx_memman should be unlocked before logging to
	//       avoid dead locks.
	if (array_mismatch) {
		num_array_mixing++;
		string allocation_filename = i->second.filename;
		int allocation_lineno = i->second.lineno;
		bool allocation_is_array = i->second.is_array;
		pointer_map.erase(p);
		mtx_memman.unlock();
		
		log_file->write_header("t_memman::trc_delete",
			LOG_MEMORY, LOG_WARNING);
		log_file->write_raw(filename);
		log_file->write_raw(", line ");
		log_file->write_raw(lineno);
		log_file->write_raw(": pointer to ");
		log_file->write_raw(ptr2str(p));
		log_file->write_raw(" is deleted ");
		if (is_array) {
			log_file->write_raw("as array (delete []).\n");
		} else {
			log_file->write_raw("normally (delete).\n");
		}
		log_file->write_raw("But it was allocated ");
		if (allocation_is_array) {
			log_file->write_raw("as array (new []) \n");
		} else {
			log_file->write_raw("normally (new) \n");
		}
		log_file->write_raw(allocation_filename);
		log_file->write_raw(", line ");
		log_file->write_raw(allocation_lineno);
		log_file->write_endl();
		log_file->write_footer();
	} else {
		pointer_map.erase(p);
		mtx_memman.unlock();
	}
}

void t_memman::report_leaks(void) {
	mtx_memman.lock();

	if (pointer_map.empty()) {
		if (num_array_mixing == 0) {
			log_file->write_report(
				"All pointers have correctly been deallocated.",
				"t_memman::report_leaks",
				LOG_MEMORY, LOG_INFO);
		} else {
			log_file->write_header("t_memman::report_leaks",
				LOG_MEMORY, LOG_WARNING);
			log_file->write_raw("All pointers have been deallocated."),
			log_file->write_raw(
				"Mixing of array/non-array caused memory loss though.");
			log_file->write_footer();
		}

		mtx_memman.unlock();
		return;
	}

	log_file->write_header("t_memman::report_leaks", LOG_MEMORY, LOG_WARNING);
	log_file->write_raw("The following pointers were never deallocated:\n");

	for (map<void *, t_ptr_info>::const_iterator i = pointer_map.begin();
	     i != pointer_map.end(); i++)
	{
		log_file->write_raw(ptr2str(i->first));
		log_file->write_raw(" allocated from ");
		log_file->write_raw(i->second.filename);
		log_file->write_raw(", line ");
		log_file->write_raw(i->second.lineno);
		log_file->write_endl();
	}

	log_file->write_footer();

	mtx_memman.unlock();
}

void t_memman::report_stats(void) {
	mtx_memman.lock();

	log_file->write_header("t_memman::report_stats", LOG_MEMORY, LOG_INFO);

	log_file->write_raw("Number of allocations: ");
	log_file->write_raw(num_new);
	log_file->write_endl();

	log_file->write_raw("Number of duplicate allocations: ");
	log_file->write_raw(num_new_duplicate);
	log_file->write_endl();

	log_file->write_raw("Number of de-allocations: ");
	log_file->write_raw(num_delete);
	log_file->write_endl();

	log_file->write_raw("Number of mismatched de-allocations: ");
	log_file->write_raw(num_delete_mismatch);
	log_file->write_endl();

	log_file->write_raw("Number of array/non-array mixed operations: ");
	log_file->write_raw(num_array_mixing);
	log_file->write_endl();

	unsigned long num_unalloc = num_new - num_new_duplicate -
				    num_delete + num_delete_mismatch;
	log_file->write_raw("Number of unallocated pointers: ");
	log_file->write_raw(num_unalloc);
	log_file->write_endl();

	log_file->write_footer();

	mtx_memman.unlock();
}
