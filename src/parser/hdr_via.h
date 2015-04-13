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

// Via header

#ifndef _H_HDR_VIA
#define _H_HDR_VIA

#include <list>
#include <string>
#include "header.h"
#include "parameter.h"

class t_via {
public:
	string			protocol_name;
	string			protocol_version;
	string			transport;
	string			host;
	int			port;
	int			ttl;
	string			maddr;
	string			received;
	string			branch;

	// RFC 3581: symetric response routing
	bool			rport_present;
	int			rport;

	list <t_parameter>	extensions;

	t_via();
	t_via(const string &_host, const int _port, bool add_rport = true);
	void add_extension(const t_parameter &p);
	string encode(void) const;

	// Get the response destination
	void get_response_dst(t_ip_port &ip_port) const;

	// Returns true if branch starts with RFC 3261 magic cookie
	bool rfc3261_compliant(void) const;
};

class t_hdr_via : public t_header {
public:
	list<t_via>	via_list;

	t_hdr_via();
	void add_via(const t_via &v);
	string encode(void) const;
	string encode_multi_header(void) const;
	string encode_value(void) const;

	// Get the response destination
	void get_response_dst(t_ip_port &ip_port) const;
};

#endif
