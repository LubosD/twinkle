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

#ifndef _STUN_TRANSACTION_H
#define _STUN_TRANSACTION_H

#include "stun.h"
#include "phone_user.h"
#include "protocol.h"
#include "user.h"
#include "transaction.h"
#include "threads/mutex.h"
#include "threads/thread.h"
#include "sockets/socket.h"
#include "sockets/url.h"

// Create a binding in a NAT.
// Returns true on success. Returns false when the STUN server returned
// an error. Throws an int exception (containing errno) when some
// socket operation fails.
bool get_stun_binding(t_user *user_config, unsigned short src_port, unsigned long &mapped_ip,
	unsigned short &mapped_port, int &err_code, string &err_reason);
	
	
// Discover the type of NAT and determine is STUN should be used or
// wether STUN is useless.
// It sets the use_stun attribute of phone if STUN should be used.
// Return false if STUN cannot be used. err_msg will contain an
// error message that can be displayed to the user.
bool stun_discover_nat(t_phone_user *pu, string &err_msg);


//////////////////////////////////////////////
// Base STUN transaction
//////////////////////////////////////////////

class t_stun_transaction {
private:
	static t_mutex		mtx_class; // protect static members
	static t_tid		next_id; // next id to be issued

protected:
	t_tid			id; 	// transaction id
	unsigned short		tuid;	// TU id
	t_trans_state		state;
	
	// Timer for retransmissions
	unsigned short		timer_req_timeout;
	
	// Duration for the next timer start in msec
	unsigned long		dur_req_timeout;
	
	// Number of transmissions of the request
	unsigned short		num_transmissions;

	// Destinations for the request	
	list<t_ip_port> 	destinations;
	
	// User profile of user that created this transaction
	t_user			*user_config;
	
	void start_timer_req_timeout(void);
	void stop_timer_req_timeout(void);
	
	// Retransmit STUN request
	virtual void retransmit(void) = 0;

public:
	StunMessage		*request;

	t_tid get_id(void) const;
	
	// Get state of the transaction
	t_trans_state get_state(void) const;

	// The transaction will keep a copy of the request
	t_stun_transaction(t_user *user, StunMessage *r,
			   unsigned short _tuid, const list<t_ip_port> &dst);

	// All request and response pointers contained by the
	// transaction will be deleted.
	virtual ~t_stun_transaction();

	// Process STUN response
	virtual void process_response(StunMessage *r);
	
	// Process ICMP error
	virtual void process_icmp(const t_icmp_msg &icmp);
	
	// Process timeout
	virtual void timeout(t_stun_timer t);	
	
	// Match response with transaction
	bool match(StunMessage *resp) const;
	
	// Match ICMP error with transaction
	bool match(const t_icmp_msg &icmp) const;
};

//////////////////////////////////////////////
// SIP STUN transaction
//////////////////////////////////////////////

// A SIP STUN transaction is a STUN request to get a binding
// for the SIP port. Such a request must be sent from the SIP port.
// So it must be sent out via the SIP sender thread.

class t_sip_stun_trans : public t_stun_transaction {
protected:
	virtual void retransmit(void);
	
public:
	// Create transaction and send out STUN request		
	t_sip_stun_trans(t_user *user, StunMessage *r,
			 unsigned short _tuid, const list<t_ip_port> &dst);
};

//////////////////////////////////////////////
// Media STUN transaction
//////////////////////////////////////////////

// TODO: this code is not used anymore. Remove?

// A media STUN transaction is a STUN request to get a binding
// for a media port. Such a request must be sent from the media
// port.

class t_media_stun_trans : public t_stun_transaction {
private:
	t_socket_udp	*sock;		// UDP socket for STUN
	t_thread	*thr_listen;	// Listener thread
	
protected:
	virtual void retransmit(void);
	
public:
	// Create transaction and send out STUN request		
	t_media_stun_trans(t_user *user, StunMessage *r,
			 unsigned short _tuid, const list<t_ip_port> &dst, 
			 unsigned short src_port);
	~t_media_stun_trans();
};

#endif
