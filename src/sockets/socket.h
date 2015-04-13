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
 * Socket operations
 */

#ifndef _H_SOCKET
#define _H_SOCKET

#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

using namespace std;

// ports and addresses should be in host order

/** ICMP message */
class t_icmp_msg {
public:
	short		type;
	short		code;
	
	// ICMP source IP address
	unsigned long	icmp_src_ipaddr;
	
	// Destination IP address/port of packet causing the ICMP message.
	unsigned long	ipaddr;
	unsigned short	port;
	
	t_icmp_msg() {};
	t_icmp_msg(short _type, short _code, unsigned long _icmp_src_ipaddr,
		unsigned long _ipaddr, unsigned short _port);
};

/** Abstract socket */
class t_socket {
protected:
	int	sd; /**< Socket descriptor. */
	
	/**
	 * Constructor. This constructor does not create a valid socket.
	 * The subclasses will create the real socket.
	 */
	t_socket();

	/** 
	 * Constructor.
	 * @param _sd Socket desciptor.
	 */
	t_socket(int _sd);
	
public:
	/** Destructor */
	virtual ~t_socket();
	
	/**
	 * Get the socket descriptor.
	 * @return The socket descriptor.
	 */
	int get_descriptor(void) const;
	
	/** Get socket options */
	int getsockopt(int level, int optname, void *optval, socklen_t *optlen);
	
	/** Set socket options */
	int setsockopt(int level, int optname, const void *optval, socklen_t optlen);
	
	/** Receive data */
	virtual ssize_t recv(void *buf, int buf_size) = 0;
	
	/** Send data */
	virtual ssize_t send(const void *data, int data_size) = 0;
};

/** UDP socket */
class t_socket_udp : public t_socket {
public:
	// Create a socket and bind it to any port.
	// Throws an int exception if it fails. The int thrown is the value
	// of errno as set by 'socket' or 'bind'.
	t_socket_udp();

	// Create a socket and bind it to port.
	// Throws an int exception if it fails. The int thrown is the value
	// of errno as set by 'socket' or 'bind'.
	t_socket_udp(unsigned short port);
	
	// Connect a socket
	// Throws an int exception if it fails (errno as set by 'sendto')
	int connect(unsigned long dest_addr, unsigned short dest_port);

	// Throws an int exception if it fails (errno as set by 'sendto')
	int sendto(unsigned long dest_addr, unsigned short dest_port,
	           const char *data, int data_size);
	virtual ssize_t send(const void *data, int data_size);

	// Throws an int exception if it fails (errno as set by 'recvfrom')
	// On success the length of the data in buf is returned. After the
	// data in buf there will be a 0.
	int recvfrom(unsigned long &src_addr, unsigned short &src_port,
		     char *buf, int buf_size);
	
	virtual ssize_t recv(void *buf, int buf_size);
	
	// Do a select on the socket in read mode. timeout is in ms.
	// Returns true if the socket becomes unblocked. Returns false
	// on time out. Throws an int exception if select fails
	// (errno as set by 'select')
	bool select_read(unsigned long timeout);
	
	// Enable reception of ICMP errors on this socket.
	// Returns false if ICMP reception cannot be enabled.
	bool enable_icmp(void);
	
	// Get an ICMP message that was received on this socket.
	// Returns false if no ICMP message can be retrieved.
	bool get_icmp(t_icmp_msg &icmp);
};

/** TCP socket */
class t_socket_tcp : public t_socket {
public:
	/** 
	 * Constructor. Create a socket. 
	 * @throw int The errno value
	 */
	t_socket_tcp();
	
	/** 
	 * Constructor. Create a socket and bind it to a port.
	 * @throw int The errno value
	 */
	t_socket_tcp(unsigned short port);
	
	/**
	 * Constructor. Create a socket from an existing descriptor.
	 */
	t_socket_tcp(int _sd);
	
	/**
	 * Listen for a connection.
	 * @throw int Errno
	 */
	void listen(int backlog);
	 
	
	/** 
	 * Accept a connection 
	 * @param src_addr [out] Source IPv4 address of the connection.
	 * @param src_port [out] Source port of the connection.
	 * @return A socket for the new connection
	 * @throw int Errno
	 */
	t_socket_tcp *accept(unsigned long &src_addr, unsigned short &src_port);
	
	/**
	 * Connect to a destination
	 * @param dest_addr [in] Destination IPv4 address.
	 * @param dest_port [in] Destination port.
	 * @throw int Errno
	 */
	void connect(unsigned long dest_addr, unsigned short dest_port);
	
	/** Send data */
	virtual ssize_t send(const void *data, int data_size);
	
	
	/** Receive data */
	virtual ssize_t recv(void *buf, int buf_size);
	
	/**
	 * Get the remote address of a connection.
	 * @param remote_addr [out] Source IPv4 address of the connection.
	 * @param remote_port [out] Source port of the connection.
	 * @throw int Errno
	 */
	void get_remote_address(unsigned long &remote_addr, unsigned short &remote_port);
};

/** Local socket */
class t_socket_local : public t_socket {	
public:
	// Throws an int exception if it fails. The int thrown is the value
	// of errno as set by 'socket'
	t_socket_local();
	
	t_socket_local(int _sd);
	
	void bind(const string &name);
	void listen(int backlog);
	int accept(void);
	void connect(const string &name);
	int read(void *buf, int count);
	virtual ssize_t recv(void *buf, int buf_size);
	int write(const void *buf, int count);
	virtual ssize_t send(const void *buf, int count);
};

// Convert an IP address in host order to a string.
string h_ip2str(unsigned long ipaddr);

#endif
