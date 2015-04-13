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

#ifndef _TRANSACTION_H
#define _TRANSACTION_H

#include <string>
#include "protocol.h"
#include "parser/request.h"
#include "parser/response.h"
#include "sockets/socket.h"
#include "threads/mutex.h"

using namespace std;

typedef unsigned short	t_tid;

/////////////////////////////////////////////////////////////
// Transaction state (see RFC 3261 17)
/////////////////////////////////////////////////////////////
enum t_trans_state {
	TS_NULL,	// non-state used for initialization
	TS_CALLING,
	TS_TRYING,
	TS_PROCEEDING,
	TS_COMPLETED,
	TS_CONFIRMED,
	TS_TERMINATED,
};

string trans_state2str(t_trans_state s);

/////////////////////////////////////////////////////////////
// General transaction
/////////////////////////////////////////////////////////////
//
// Concurrent creation of transactions is not allowed. If this
// is needed then updates to static members need to be
// synchronized with a mutex.
// All transactions are created by the transaction manager. This
// should not be changed as transactions start timers and all timers
// must be started from a single thread.

class t_transaction {
private:
	static t_mutex		mtx_class; // protect static members
	static t_tid		next_id; // next id to be issued

protected:
	t_tid			id; 	// transaction id
	unsigned short		tuid;	// TU id
	t_trans_state		state;
	string			to_tag;	// tag for to-header

public:
	// Request that created the transaction
	t_request		*request;

	t_tid get_id(void) const;

	// Provisional responses in order of arrival/sending
	list<t_response *>	provisional;

	// Final response for the transaction
	t_response		*final;

	// The transaction will keep a copy of the request
	t_transaction(t_request *r, unsigned short _tuid);

	// All request and response pointers contained by the
	// request will be deleted.
	virtual ~t_transaction();

	// Process a provisional repsonse
	// Transaction will keep a copy of the response
	virtual void process_provisional(t_response *r);

	// Process a final response
	// Transaction will keep a copy of the response
	virtual void process_final(t_response *r);

	// Process a response
	virtual void process_response(t_response *r);

	// Process timer expiry
	virtual void timeout(t_sip_timer t) = 0;

	// Get state of the transaction
	t_trans_state get_state(void) const;

	// Set TU ID
	void set_tuid(unsigned short _tuid);

	// Get type of request
	t_method get_method(void) const;

	// Get tag for to-header
	string get_to_tag(void);

	// Create response according to general rules
	t_response *create_response(int code, string reason = "");
};

/////////////////////////////////////////////////////////////
// Client transaction
/////////////////////////////////////////////////////////////
class t_trans_client : public t_transaction {
protected:
	/** Destination for request. */
	t_ip_port	dst_ip_port;

public:
	/**
	 * Create transaction and send request to destination.
	 * @param r [in] Request creating the transaction.
	 * @param ip_port [in] Destination of the request.
	 * @param _tuid [in] Transaction user id assigned to this transaction.
	 */
	t_trans_client(t_request *r, const t_ip_port &ip_port,
		unsigned short _tuid);

	/**
	 * Match a response with a transaction.
	 * @param r [in] The response to match.
	 * @return true if the response matches the transaction.
	 */
	bool match(t_response *r) const;
	
	/**
	 * @param icmp [in] ICMP message to match.
	 * @return true if the ICMP error matches the transaction
	 */
	bool match(const t_icmp_msg &icmp) const;
	
	/**
	 * Match transaction with a branch and CSeq method value.
	 * @param branch [in] Branch to match.
	 * @param cseq_method [in] CSeq method to match.
	 * @return true if transaction matches, otherwise false.
	 */
	bool match(const string &branch, const t_method &cseq_method) const;
	
	virtual void process_provisional(t_response *r);
	
	/** 
	 * Process ICMP errors.
	 * @param icmp [in] ICMP message.
	 */
	virtual void process_icmp(const t_icmp_msg &icmp) = 0;
	
	/**
	 * Process failures.
	 * @param failure [in] Type of failure.
	 */
	virtual void process_failure(t_failure failure) = 0;

	/**
	 * Abort a transaction.
	 * This will send a 408 response internally to finish the transaction.
	 */
	virtual void abort(void) = 0;
};

/////////////////////////////////////////////////////////////
// Client INVITE transaction
/////////////////////////////////////////////////////////////
class t_tc_invite : public t_trans_client {
private:
	// Timers
	unsigned short	timer_A;
	unsigned short	timer_B;
	unsigned short	timer_D;

	// Duration of next timer A in msec
	long	duration_A;

	void start_timer_A(void);
	void start_timer_B(void);
	void start_timer_D(void);
	void stop_timer_A(void);
	void stop_timer_B(void);
	void stop_timer_D(void);


public:
	t_request		*ack;	// ACK request

	// Create transaction and send request to destination
	// Start timer A and timer B
	t_tc_invite(t_request *r, const t_ip_port &ip_port,
		unsigned short _tuid);

	virtual ~t_tc_invite();

	// Process a provisional repsonse
	// Stop timer A
	void process_provisional(t_response *r);

	// Process a final response
	// Stop timer B.
	// Start timer D (for non-2xx final).
	void process_final(t_response *r);
	
	void process_icmp(const t_icmp_msg &icmp);
	
	void process_failure(t_failure failure);

	void timeout(t_sip_timer t);

	void abort(void);
};

/////////////////////////////////////////////////////////////
// Client non-INVITE transaction
/////////////////////////////////////////////////////////////
class t_tc_non_invite : public t_trans_client {
private:
	// Timers
	unsigned short	timer_E;
	unsigned short	timer_F;
	unsigned short	timer_K;

	// Duration of next timer E in msec
	long	duration_E;

	void start_timer_E(void);
	void start_timer_F(void);
	void start_timer_K(void);
	void stop_timer_E(void);
	void stop_timer_F(void);
	void stop_timer_K(void);

public:
	// Create transaction and send request to destination
	// Stop timer E and timer F
	t_tc_non_invite(t_request *r, const t_ip_port &ip_port,
		unsigned short _tuid);

	virtual ~t_tc_non_invite();

	// Process a provisional repsonse
	void process_provisional(t_response *r);

	// Process final response
	// Stop timer E and F. Start timer K.
	void process_final(t_response *r);
	
	void process_icmp(const t_icmp_msg &icmp);
	
	void process_failure(t_failure failure);

	void timeout(t_sip_timer t);

	void abort(void);
};

/////////////////////////////////////////////////////////////
// Server transaction
/////////////////////////////////////////////////////////////
class t_trans_server : public t_transaction {
private:
	// Match a the transaction to a request. Argument
	// If cancel==true then the target for a CANCEL
	// is matched.
	// If cancel==false then the request itself is matched,
	// eg. retransmission or ACK to INVITE matching
	bool match(t_request *r, bool cancel) const;
	
	// Indicates if a 100 Trying has already been sent.
	// A 100 Trying should only be sent once.
	// The reason for sending a 100 Trying is to indicate that
	// the request has been received but that processing will
	// take some time.
	// Based on the tasks to perform several parts of the transaction
	// user can decide independently to send a 100 Trying. This
	// flag assures that only one 100 Trying will be sent out
	// though.
	bool resp_100_trying_sent;

public:
	t_trans_server(t_request *r, unsigned short _tuid);

	// Process a provisional repsonse
	// Send provisional response
	void process_provisional(t_response *r);

	// Process a final response
	// Send the final response
	void process_final(t_response *r);

	// Process a received retransmission of the request
	virtual void process_retransmission(void);

	// Returns true if request matches transaction
	bool match(t_request *r) const;

	// Returns true if the transaction is the target of CANCEL
	bool match_cancel(t_request *r) const;
};

/////////////////////////////////////////////////////////////
// Server INIVITE transaction
/////////////////////////////////////////////////////////////
class t_ts_invite : public t_trans_server {
private:
	// Timers
	unsigned short	timer_G;
	unsigned short	timer_H;
	unsigned short	timer_I;

	// Duration of next timer G in msec
	long	duration_G;

	void start_timer_G(void);
	void start_timer_H(void);
	void start_timer_I(void);
	void stop_timer_G(void);
	void stop_timer_H(void);
	void stop_timer_I(void);

public:
	t_request		*ack;	// ACK request

	t_ts_invite(t_request *r, unsigned short _tuid);
	virtual ~t_ts_invite();

	void process_provisional(t_response *r);
	void process_final(t_response *r);
	void process_retransmission(void);
	void timeout(t_sip_timer t);

	// Transaction will keep a copy of the ACK.
	void acknowledge(t_request *ack_request);
};

/////////////////////////////////////////////////////////////
// Server non-INVITE transaction
/////////////////////////////////////////////////////////////
class t_ts_non_invite : public t_trans_server {
private:
	// Timers
	unsigned short	timer_J;

	void start_timer_J(void);
	void stop_timer_J(void);

public:
	t_ts_non_invite(t_request *r, unsigned short _tuid);
	virtual ~t_ts_non_invite();

	void process_provisional(t_response *r);
	void process_final(t_response *r);
	void process_retransmission(void);
	void timeout(t_sip_timer t);
};

#endif
