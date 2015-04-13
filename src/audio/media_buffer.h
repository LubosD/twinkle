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

#ifndef _MEDIA_BUFFER_H
#define _MEDIA_BUFFER_H

#include "threads/mutex.h"

// A buffer for buffering media streams.
// Used for conference calls to buffer one stream that needs to be
// mixed with the main stream.

class t_media_buffer {
private:
	// The buffer. It is used as a ring buffer
	unsigned char	*buffer;

	// Size of the buffer
	int		buf_size;

	// Begin and end position of the buffer content.
	// pos_end points to the last byte of content.
	int		pos_start;
	int		pos_end;

	// Inidicates if buffer is empty
	bool		empty;

	// Mutex to protect operations on the buffer
	t_recursive_mutex mtx;

	// Prevent this constructor from being used.
	t_media_buffer() {};

public:
	// Create a media buffer of size size.
	t_media_buffer(int size);
	~t_media_buffer();

	// Add data to buffer. If there is more data than buffer
	// space left, then old content will be removed from the
	// buffer.
	// - data is the data to be added
	// - len is the number of bytes to be added
	void add(unsigned char *data, int len);

	// Get data from the buffer. If there is not enough data
	// in the buffer, then no data is retrieved at all.
	// False is returned if data cannot be retrieved.
	// - data must point to a buffer of at least size len
	// - len is the amount of bytes to be retrieved
	bool get(unsigned char *data, int len);

	// Return the number of bytes contained by the buffer.
	int size_content(void);
};

#endif
