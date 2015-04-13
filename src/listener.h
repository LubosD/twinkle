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
 * Network listener threads
 */

#ifndef _H_LISTENER
#define _H_LISTENER

/** Thread listening on SIP UDP port */
void *listen_udp(void *arg);

/** Thread listening on established TCP connections */
void *listen_for_data_tcp(void *arg);

/** Thread listening for incoming TCP connection requests */
void *listen_for_conn_requests_tcp(void *arg);

#endif
