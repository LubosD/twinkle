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
 * Connection table
 */
 
#ifndef _H_CONNECTION_TABLE
#define _H_CONNECTION_TABLE

#include <list>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "connection.h"
#include "threads/mutex.h"

using namespace std;

/** Table of established connections. */
class t_connection_table {
private:
	/** Established connections */
	list<t_connection *>	connections_;
	
	/** Mutex to protect concurrent access to the connections. */
	static t_recursive_mutex mtx_connections_;
	
	/** Indicates if the connection table is terminated. */
	bool			terminated_;
	
	/** Pipe to signal modification of the list of readable connections. */
	int			fd_pipe_modified_read_[2];
	
	/** Pipe to signal modification of the list of connections with data to send. */
	int			fd_pipe_modified_write_[2];
	
	/** Pipe to signal the read select operation to quit. */
	int			fd_pipe_quit_read_[2];
	
	/** Pipe to signal the write select operation to quit. */
	int			fd_pipe_quit_write_[2];
	
	/** Create a pipe. */
	void create_pipe(int p[2]);
	
	/** Send a modification signal on the read modification pipe. */
	void signal_modification_read(void);
	
	/** Send a modification signal on the write modification pipe. */
	void signal_modification_write(void);
	
	/** Send a quit signal on the modification pipes. */
	void signal_quit(void);
	
public:
	typedef list<t_connection *>::size_type size_type;
	
	/** Constructor */
	t_connection_table();
	
	/**
	 * Destructor.
	 * @note All connections in the table will be closed
	 *       and destroyed.
	 */
	~t_connection_table();
	
	/**
	 * Unlock connection table.
	 * @note After some operations, the table stays lock to avoid race
	 *       conditions. The caller should unlock the table explicitly
	 *       when it has finished working on the connections.
	 */
	void unlock(void) const;
	
	/**
	 * Check if connection table is empty.
	 * @return true if empty, false if not empty.
	 */
	bool empty(void) const;
	
	/**
	 * Get number of connections in table.
	 * @return number of connections.
	 */
	size_type size(void) const;
	
	/**
	 * Add a connection to the table.
	 * @param connection [in] Connection to add.
	 */
	void add_connection(t_connection *connection);
	
	/**
	 * Remove a TCP connection from the table.
	 * @param connection [in] TCP connection to remove.
	 */
	void remove_connection(t_connection *connection);
	
	/**
	 * Get a connection to a particular destination.
	 * @param remote_addr [in] IP address of destination.
	 * @param remote_port [in] Port of destination.
	 * @return The connection to the destination. If there is no
	 *         connection to the destination, then NULL is returned.
	 * @post If a connection is returned, the table is locked. The caller must
	 *       unlock the tbale when it is finished with the connection.
	 * @note Only re-usable connections are considered.
	 */
	t_connection *get_connection(unsigned long remote_addr, unsigned short remote_port);
	
	/**
	 * Wait for connections to become readable.
	 * @param timeout [in] Maxmimum time to wait. If NULL, then wait indefinitely.
	 * @return List of sockets that are readable.
	 * @throw int Errno
	 * @post The transaction table is locked if a non-empty list of sockets is returned.
	 * @note The caller should unlock the table when processing of the sockets is finished.
	 */
	list<t_connection *> select_read(struct timeval *timeout) const;
	
	/**
	 * Wait for connections to become writeable.
	 * Only connections with data to send are waited for.
	 * @param timeout [in] Maxmimum time to wait. If NULL, then wait indefinitely.
	 * @return List of sockets that are writable.
	 * @throw int Errno
	 * @post The transaction table is locked if a non-empty list of sockets is returned.
	 * @note The caller should unlock the table when processing of the sockets is finished.
	 */
	list<t_connection *> select_write(struct timeval *timeout) const;
	
	/** Cancel all selects. */
	void cancel_select(void);
	
	/** Restart write select, so new connections with data are picked up. */
	void restart_write_select(void);
	
	/**
	 * Close all idle connections.
	 * Increment the idle time of all connections with interval.
	 * A persistent connection with associated registered URI's will not be closed.
	 * @param interval [in] Interval to add to idle time (ms).
	 * @param terminated [out] Indicates if the connection table has been terminated.
	 */
	void close_idle_connections(unsigned long interval, bool &terminated);
};

/** Main for thread handling connection timeouts */
void *connection_timeout_main(void *arg);

#endif
