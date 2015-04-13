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

#include "connection_table.h"

#include <algorithm>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/time.h>
#include <cerrno>
#include <iostream>
#include <cstdlib>
#include <fcntl.h>

#include "log.h"
#include "protocol.h"
#include "util.h"
#include "audits/memman.h"

using namespace std;

extern t_connection_table *connection_table;

void t_connection_table::create_pipe(int p[2]) {
	if (pipe(p) == -1) {
		string err = get_error_str(errno);
		cerr << "FATAL: t_connection_table - Cannot create pipe.\n";
		cerr << err << endl;
		exit(-1);
	}
	
	if (fcntl(p[0], F_SETFL, O_NONBLOCK) == -1) {
		string err = get_error_str(errno);
		cerr << "FATAL: t_connection_table - fcntl fails on read side of pipe.\n";
		cerr << err << endl;
		exit(-1);
	}
	
	if (fcntl(p[1], F_SETFL, O_NONBLOCK) == -1) {
		string err = get_error_str(errno);
		cerr << "FATAL: t_connection_table - fcntl fails on write side of pipe.\n";
		cerr << err << endl;
		exit(-1);
	}
}

void t_connection_table::signal_modification_read(void) {
	t_mutex_guard guard(mtx_connections_);

	// Write a byte to the modified pipe, so a select can be retried.
	char x = 'x';
	(void)write(fd_pipe_modified_read_[1], &x, 1);
}

void t_connection_table::signal_modification_write(void) {
	t_mutex_guard guard(mtx_connections_);

	// Write a byte to the modified pipe, so a select can be retried.
	char x = 'x';
	(void)write(fd_pipe_modified_write_[1], &x, 1);
}

void t_connection_table::signal_quit(void) {
	t_mutex_guard guard(mtx_connections_);
	
	// Write a byte to the quit pipe, so a select can be halted.
	char x = 'x';
	(void)write(fd_pipe_quit_read_[1], &x, 1);
	(void)write(fd_pipe_quit_write_[1], &x, 1);
	
	terminated_ = true;
}

t_recursive_mutex t_connection_table::mtx_connections_;

t_connection_table::t_connection_table() :
	terminated_(false)
{
	create_pipe(fd_pipe_modified_read_);
	create_pipe(fd_pipe_modified_write_);
	create_pipe(fd_pipe_quit_read_);
	create_pipe(fd_pipe_quit_write_);
}

t_connection_table::~t_connection_table() {
	t_mutex_guard guard(mtx_connections_);

	for (list<t_connection *>::iterator it = connections_.begin();
	     it != connections_.end(); ++it)
	{
		MEMMAN_DELETE(*it);
		delete *it;
	}
}

void t_connection_table::unlock(void) const {
	mtx_connections_.unlock();
}

bool t_connection_table::empty(void) const {
	t_mutex_guard guard(mtx_connections_);
	return connections_.empty();
}

t_connection_table::size_type t_connection_table::size(void) const {
	t_mutex_guard guard(mtx_connections_);
	return connections_.size();
}

void t_connection_table::add_connection(t_connection *connection) {
	t_mutex_guard guard(mtx_connections_);
	connections_.push_back(connection);
	signal_modification_read();
	signal_modification_write();
}

void t_connection_table::remove_connection(t_connection *connection) {
	t_mutex_guard guard(mtx_connections_);
	connections_.remove(connection);
	signal_modification_read();
	signal_modification_write();
}

t_connection *t_connection_table::get_connection(unsigned long remote_addr, 
		unsigned short remote_port)
{
	mtx_connections_.lock();
	
	t_connection *found_connection = NULL;
	list<t_connection *> broken_connections;
	
	for (list<t_connection *>::iterator it = connections_.begin();
	     it != connections_.end(); ++it)
	{
		unsigned long addr;
		unsigned short port;

		if ((*it)->may_reuse()) {
			try {
				t_socket *socket = (*it)->get_socket();
				t_socket_tcp *tcp_socket = dynamic_cast<t_socket_tcp *>(socket);
				
				if (tcp_socket) {
					tcp_socket->get_remote_address(addr, port);
					if (addr == remote_addr && port == remote_port) {
						found_connection = *it;
						break;
					}
				}
			} catch (int err) {
				// This should never happen.
				cerr << "Cannot get remote address of socket." << endl;
				
				// Destroy and remove connection as it is probably broken.
				broken_connections.push_back(*it);
			}
		}
	}
	
	// Clear broken connections
	for (list<t_connection *>::iterator it = broken_connections.begin();
	     it != broken_connections.end(); ++it)
	{
		remove_connection(*it);
		MEMMAN_DELETE(*it);
		delete *it;
	}
	
	if (!found_connection) mtx_connections_.unlock();
	return found_connection;
}

list<t_connection *> t_connection_table::select_read(struct timeval *timeout) const {
	fd_set read_fds;
	int nfds = 0;
	bool retry = true;
	list<t_connection *> result;
	
	// Empty modification pipe
	char pipe_buf;
	while (read(fd_pipe_modified_read_[0], &pipe_buf, 1) > 0);
	
	while (retry) {
		FD_ZERO(&read_fds);
		
		// Add modification pipe so select can be restarted when the
		// connection table modifies.
		FD_SET(fd_pipe_modified_read_[0], &read_fds);
		nfds = fd_pipe_modified_read_[0];
		
		// Add quit pipe so select can quit on demand.
		FD_SET(fd_pipe_quit_read_[0], &read_fds);
		nfds = max(nfds, fd_pipe_quit_read_[0]);
		
		mtx_connections_.lock();
		
		for (list<t_connection *>::const_iterator it = connections_.begin();
		     it != connections_.end(); ++it)
		{
			t_socket *socket = (*it)->get_socket();
			int fd = socket->get_descriptor();
			FD_SET(fd, &read_fds);
			nfds = max(nfds, fd);
		}
		
		mtx_connections_.unlock();
		
		int ret = select(nfds + 1, &read_fds, NULL, NULL, timeout);
		if (ret < 0) throw errno;
		
		if (FD_ISSET(fd_pipe_quit_read_[0], &read_fds)) {
			// Quit was signalled, so stop immediately.
			break;
		}
	
		mtx_connections_.lock();
		
		// Determine which sockets have become readable
		for (list<t_connection *>::const_iterator it = connections_.begin();
		     it != connections_.end(); ++it)
		{
			t_socket *socket = (*it)->get_socket();
			int fd = socket->get_descriptor();
			if (FD_ISSET(fd, &read_fds)) {
				result.push_back(*it);
			}
		}
		
		if (!result.empty()) {
			// Connections have become readable, so return to the caller.
			retry = false;
		} else {
			mtx_connections_.unlock();
			
			// No connections have become readable. Check signal descriptors
			if (FD_ISSET(fd_pipe_modified_read_[0], &read_fds)) {
				// The connection table is modified. Retry select.
				read(fd_pipe_modified_read_[0], &pipe_buf, 1);
			} else {
				// This should never happen.
				cerr << "ERROR: select_read returned without any file descriptor." << endl;
			}
		}
	}
	
	return result;
}

list<t_connection *> t_connection_table::select_write(struct timeval *timeout) const {
	fd_set read_fds;
	fd_set write_fds;
	int nfds = 0;
	bool retry = true;
	list<t_connection *> result;
	
	// Empty modification pipe
	char pipe_buf;
	while (read(fd_pipe_modified_write_[0], &pipe_buf, 1) > 0);
	
	while (retry) {
		FD_ZERO(&read_fds);
		FD_ZERO(&write_fds);
		
		// Add modification pipe so select can be restarted when the
		// connection table modifies.
		FD_SET(fd_pipe_modified_write_[0], &read_fds);
		nfds = fd_pipe_modified_write_[0];
		
		// Add quit pipe so select can quit on demand.
		FD_SET(fd_pipe_quit_write_[0], &read_fds);
		nfds = max(nfds, fd_pipe_quit_write_[0]);
		
		mtx_connections_.lock();
		
		for (list<t_connection *>::const_iterator it = connections_.begin();
		     it != connections_.end(); ++it)
		{
			if ((*it)->has_data_to_send()) {
				t_socket *socket = (*it)->get_socket();
				int fd = socket->get_descriptor();
				FD_SET(fd, &write_fds);
				nfds = max(nfds, fd);
			}
		}
		
		mtx_connections_.unlock();
		
		int ret = select(nfds + 1, &read_fds, &write_fds, NULL, timeout);
		if (ret < 0) throw errno;
		
		if (FD_ISSET(fd_pipe_quit_write_[0], &read_fds)) {
			// Quit was signalled, so stop immediately.
			break;
		}
	
		mtx_connections_.lock();
		
		// Determine which sockets have become writable
		for (list<t_connection *>::const_iterator it = connections_.begin();
		     it != connections_.end(); ++it)
		{
			t_socket *socket = (*it)->get_socket();
			int fd = socket->get_descriptor();
			if (FD_ISSET(fd, &write_fds)) {
				result.push_back(*it);
			}
		}
		
		if (!result.empty()) {
			// Connections have become writable, so return to the caller.
			retry = false;
		} else {
			mtx_connections_.unlock();
			
			// No connections have become writable. Check signal descriptors
			if (FD_ISSET(fd_pipe_modified_write_[0], &read_fds)) {
				// The connection table is modified. Retry select.
				read(fd_pipe_modified_write_[0], &pipe_buf, 1);
			} else {
				// This should never happen.
				cerr << "ERROR: select_write returned without any file descriptor." << endl;
			}
		}
	}
	
	return result;
}

void t_connection_table::cancel_select(void) {
	signal_quit();
}

void t_connection_table::restart_write_select(void) {
	signal_modification_write();
}

void t_connection_table::close_idle_connections(unsigned long interval, bool &terminated) {
	t_mutex_guard guard(mtx_connections_);
	
	terminated = terminated_;

	list<t_connection *> expired_connections;
	
	// Update idle times and find expired connections.
	for (list<t_connection *>::iterator it = connections_.begin();
	     it != connections_.end(); ++it)
	{
		unsigned long idle_time = (*it)->increment_idle_time(interval);
		if (idle_time >= DUR_IDLE_CONNECTION || terminated) {
			// If a registered URI is associated with the connection, then
			// it is persistent and it should not be closed.
			if (!(*it)->has_registered_uri()) {
				expired_connections.push_back(*it);
			}
		}
	}
	
	// Close expired connections.
	for (list<t_connection *>::iterator it = expired_connections.begin();
	     it != expired_connections.end(); ++it)
	{
		unsigned long ipaddr;
		unsigned short port;
		
		(*it)->get_remote_address(ipaddr, port);
		log_file->write_header("t_connection_table::close_idle_connections", LOG_NORMAL, LOG_DEBUG);
		log_file->write_raw("Close connection to ");
		log_file->write_raw(h_ip2str(ipaddr));
		log_file->write_raw(":");
		log_file->write_raw(int2str(port));
		log_file->write_endl();
		log_file->write_footer();
	
		remove_connection(*it);
		MEMMAN_DELETE(*it);
		delete *it;
	}
}

void *connection_timeout_main(void *arg) {
	bool terminated = false;
	
	while (!terminated) {
		struct timespec sleep_timer;
		
		sleep_timer.tv_sec = 1;
		sleep_timer.tv_nsec = 0;
		nanosleep(&sleep_timer, NULL);
		connection_table->close_idle_connections(1000, terminated);
	}
	
	log_file->write_report("Connection timeout handler terminated.",
			"::connection_timeout_main");
			
	return NULL;
};
