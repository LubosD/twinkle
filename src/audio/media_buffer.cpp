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

#include "media_buffer.h"
#include <cstring>
#include "audits/memman.h"

////////////
// PUBLIC
////////////

t_media_buffer::t_media_buffer(int size) {
	buf_size = size;
	empty = true;
	buffer = new unsigned char[size];
	MEMMAN_NEW_ARRAY(buffer);
	pos_start = 0;
	pos_end = 0;
}

t_media_buffer::~t_media_buffer() {
	MEMMAN_DELETE_ARRAY(buffer);
	delete [] buffer;
}

void t_media_buffer::add(unsigned char *data, int len) {
	int data_start, data_end;

	mtx.lock();

	// The amount of data should fit in the buffer.
	if (len > buf_size) {
		mtx.unlock();
		return;
	}

	int current_size_content = size_content();
	if (empty) {
		data_start = 0;
		data_end = len - 1;
		pos_start = 0;
		empty = false;
	} else {
		data_start = (pos_end + 1) % buf_size;
		data_end = (data_start + len - 1) % buf_size;
	}

	// Copy the data into the buffer
	if (data_end >= data_start) {
		memcpy(buffer + data_start, data, len);
	} else {
		// The data wraps around the end of the buffer
		memcpy(buffer + data_start, data, buf_size - data_start);
		memcpy(buffer, data + buf_size - data_start, data_end + 1);
	}

	// Check if the new data wrapped over the start of the old data.
	// If so, then advance the start of the old data behind the end of the new
	// data as new data has erased the oldest data.
	if (buf_size - current_size_content < len) {
		pos_start =  (data_end + 1) % buf_size;
	}

	pos_end = data_end;

	mtx.unlock();
}

bool t_media_buffer::get(unsigned char *data, int len) {
	mtx.lock();

	if (len > size_content()) {
		mtx.unlock();
		return false;
	}

	// Retrieve the data from the buffer
	if (pos_start + len <= buf_size) {
		memcpy(data, buffer + pos_start, len);
	} else {
		// The data to be retrieved wraps around the end of
		// the buffer.
		memcpy(data, buffer + pos_start, buf_size - pos_start);
		memcpy(data + buf_size - pos_start, buffer, len - buf_size + pos_start);
	}

	pos_start = (pos_start + len) % buf_size;

	// Check if buffer is empty
	if (pos_start == (pos_end + 1) % buf_size) {
		empty = true;
	}

	mtx.unlock();
	return true;
}

int t_media_buffer::size_content(void) {
	int len;

	mtx.lock();

	if (empty) {
		len = 0;
	} else if (pos_end >= pos_start) {
		len = pos_end - pos_start + 1;
	} else {
		len = pos_end + buf_size - pos_start + 1;
	}

	mtx.unlock();
	return len;
}
