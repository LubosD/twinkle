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

#ifndef _H_INTERFACES
#define _H_INTERFACES

#include <list>
#include <string>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <ifaddrs.h>

using namespace std;

class t_interface {
public:
	string name;		// interface name, eg. eth0
	struct in_addr address;	// interface IP address
	struct in_addr netmask; // interface netmask

	t_interface(string _name);

	// Get string representation of IP address
	string get_ip_addr(void) const;
	string get_ip_netmask(void) const;
};

// Return a list of all interfaces that are UP
// If include_loopback == true, then the loopback interface is returned as well.
list<t_interface> *get_interfaces(bool include_loopback = false);

// Check if an interface with a certain IP address exists
bool exists_interface(const string &hostname);

// Check if an interface exists and return its IP address
bool exists_interface_dev(const string &devname, string &ip_address);

#endif
