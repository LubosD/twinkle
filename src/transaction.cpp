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

#include <assert.h>
#include <iostream>
#include "log.h"
#include "events.h"
#include "timekeeper.h"
#include "transaction.h"
#include "transaction_mgr.h"
#include "user.h"
#include "util.h"
#include "audits/memman.h"

extern t_event_queue		*evq_sender;
extern t_event_queue		*evq_trans_layer;
extern t_transaction_mgr	*transaction_mgr;

string trans_state2str(t_trans_state s) {
	switch(s) {
	case TS_NULL:		return "TS_NULL";
	case TS_CALLING:	return "TS_CALLING";
	case TS_TRYING:		return "TS_TRYING";
	case TS_PROCEEDING:	return "TS_PROCEEDING";
	case TS_COMPLETED:	return "TS_COMPLETED";
	case TS_CONFIRMED:	return "TS_CONFIRMED";
	case TS_TERMINATED:	return "TS_TERMINATED";
	}

	return "UNKNOWN";
}

///////////////////////////////////////////////////////////
// RFC 3261 17
// General transaction
///////////////////////////////////////////////////////////

t_mutex t_transaction::mtx_class;
t_tid t_transaction::next_id = 1;

t_transaction::t_transaction(t_request *r, unsigned short _tuid) {
	mtx_class.lock();
	id = next_id++;
	if (next_id == 65535) next_id = 1;
	mtx_class.unlock();
	
	state = TS_NULL;
	request = (t_request *)r->copy();
	final = NULL;
	tuid = _tuid;
}

t_transaction::~t_transaction() {
	MEMMAN_DELETE(request);
	delete request;
	if (final != NULL) {
		MEMMAN_DELETE(final);
		delete final;
	}

	for (list<t_response *>::iterator i = provisional.begin();
	     i != provisional.end(); i++)
	{
		MEMMAN_DELETE(*i);
		delete *i;
	}
}

t_tid t_transaction::get_id(void) const {
	return id;
}

void t_transaction::process_provisional(t_response *r) {
	provisional.push_back((t_response *)r->copy());
}

void t_transaction::process_final(t_response *r) {
	final = (t_response *)r->copy();
}

void t_transaction::process_response(t_response *r) {
	if (r->is_provisional()) {
		process_provisional(r);
	} else {
		process_final(r);
	}
}

t_trans_state t_transaction::get_state(void) const {
	return state;
}

void t_transaction::set_tuid(unsigned short _tuid) {
	tuid = _tuid;
}

t_method t_transaction::get_method(void) const {
	return request->method;
}

string t_transaction::get_to_tag(void) {
	string	tag;

	tag = request->hdr_to.tag;
	if (tag.size() > 0) return tag;
	if (to_tag.size() > 0) return to_tag;
	to_tag = random_token(TAG_LEN);
	return to_tag;
}

// RCF 3261 section 8.2.6.2
t_response *t_transaction::create_response(int code, string reason) {
	t_response *r;

	r = request->create_response(code, reason);
	
	// NOTE: 100 Trying does not establish a dialog
	if (code != R_100_TRYING) {
		r->hdr_to.set_tag(get_to_tag());
	}

	return r;
}

///////////////////////////////////////////////////////////
// RFC 3261 17.1
// Client transaction
///////////////////////////////////////////////////////////

t_trans_client::t_trans_client(t_request *r, const t_ip_port &ip_port,
		unsigned short _tuid) :
	t_transaction(r, _tuid),
	dst_ip_port(ip_port)
{
	// Send request
	evq_sender->push_network(r, dst_ip_port);
}

// RFC 3261 17.1.3, 8.2.6.2
// Section 17.1.3 states that only the branch and CSeq method should match.
// This can lead to the following problem however:
//
// 1) A response matches a BYE request, but has a wrong call id.
// 2) As the response matches the request, the transaction finishes.
// 3) Then the response is delivered to the TU which tries to match the
//    response to a dialog.
// 4) As the call id is wrong, no match is found an the response is discarded.
// 5) Now the TU keeps waiting forever for a response on the BYE
//
// By taking the call id into account here, this scenario is prevented.
// When a call id is wrong, the BYE request will be retransmitted due to
// timeouts until the transaction times out completely and a 408 is sent
// to the TU.
//
// Same problem can occur when tags do not match, so tag is take into account
// as well. So tags are take into account as well.
bool t_trans_client::match(t_response *r) const {
	t_via	&req_top_via = request->hdr_via.via_list.front();
	t_via	&resp_top_via = r->hdr_via.via_list.front();

	return (req_top_via.branch == resp_top_via.branch &&
		request->hdr_cseq.method == r->hdr_cseq.method &&
		request->hdr_call_id.call_id == r->hdr_call_id.call_id &&
		request->hdr_from.tag == r->hdr_from.tag &&
		(request->hdr_to.tag.empty() || request->hdr_to.tag == r->hdr_to.tag));
}

// An ICMP error matches a transaction when the destination IP address/port
// of the packet that caused the ICMP error equals the destination 
// IP address/port of the transaction. Other information of the packet causing
// the ICMP error is not available.
// In theory when multiple transactions are open for the same destination, the
// wrong transaction may process the ICMP error. In practice this should rarely
// happen as the destination will be unreachable for all those transactions.
// If it happens a transaction gets aborted.
bool t_trans_client::match(const t_icmp_msg &icmp) const {
	return (dst_ip_port.ipaddr == icmp.ipaddr && dst_ip_port.port == icmp.port);
}

bool t_trans_client::match(const string &branch, const t_method &cseq_method) const {
	t_via	&req_top_via = request->hdr_via.via_list.front();
	
	return (req_top_via.branch == branch &&
	        request->hdr_cseq.method == cseq_method);
}

void t_trans_client::process_provisional(t_response *r) {
	// Set the to_tag, such that an internally genrated answer (when needed) 
	// will have the correct tag.
	// An INVITE transaction may receive provisional responses with
	// different to-tags. Only the first to-tag will be kept and an
	// internally generated response will match this tag.
	if (!r->hdr_to.tag.empty() && to_tag.empty()) {
		to_tag = r->hdr_to.tag;
	}
	
	t_transaction::process_provisional(r);
}

///////////////////////////////////////////////////////////
// RFC 3261 17.1.1
// Client INVITE transaction
///////////////////////////////////////////////////////////

void t_tc_invite::start_timer_A(void) {
	timer_A = transaction_mgr->start_timer(duration_A, TIMER_A, id);

	// Double duration for a next start
	duration_A = 2 * duration_A;
}

void t_tc_invite::start_timer_B(void) {
	timer_B = transaction_mgr->start_timer(DURATION_B, TIMER_B, id);
}

void t_tc_invite::start_timer_D(void) {
	// RFC 3261 17.1.1.2
	// For reliable transport timer D must be set to zero seconds.
	if (dst_ip_port.transport == "udp") {
		timer_D = transaction_mgr->start_timer(DURATION_D, TIMER_D, id);
	} else {
		timer_D = transaction_mgr->start_timer(0, TIMER_D, id);
	}
}

void t_tc_invite::stop_timer_A(void) {
	if (timer_A) {
		transaction_mgr->stop_timer(timer_A);
		timer_A = 0;
	}
}

void t_tc_invite::stop_timer_B(void) {
	if (timer_B) {
		transaction_mgr->stop_timer(timer_B);
		timer_B = 0;
	}
}

void t_tc_invite::stop_timer_D(void) {
	if (timer_D) {
		transaction_mgr->stop_timer(timer_D);
		timer_D = 0;
	}
}

t_tc_invite::t_tc_invite(t_request *r, const t_ip_port &ip_port,
		unsigned short _tuid) :
	t_trans_client(r, ip_port, _tuid)
{
	assert(r->method == INVITE);

	ack = NULL;
	duration_A = DURATION_A;
	state = TS_CALLING;

	// RFC 3261 17.1.1.2
	// Start timer A for unreliable transports.
	if (ip_port.transport == "udp") start_timer_A();
	
	// RFC 3261 17.1.1.2
	// Start timer B for all transports
	start_timer_B();
	
	timer_D = 0;
}

t_tc_invite::~t_tc_invite() {
	if (ack != NULL) {
		MEMMAN_DELETE(ack);
		delete ack;
	}
	stop_timer_A();
	stop_timer_B();
	stop_timer_D();
}

void t_tc_invite::process_provisional(t_response *r) {
	assert(r->is_provisional());

	switch (state) {
	case TS_CALLING:
		stop_timer_A();
		stop_timer_B();
		// fall through
	case TS_PROCEEDING:
		t_trans_client::process_provisional(r);
		state = TS_PROCEEDING;

		// Report to TU
		evq_trans_layer->push_user(r, tuid, id);
		break;
	default:
		// Discard provisional response in other states
		break;
	}
}

void t_tc_invite::process_final(t_response *r) {
	assert(r->is_final());

	t_ip_port ip_port;

	switch (state) {
	case TS_CALLING:
		stop_timer_A();
		stop_timer_B();
		// fall through
	case TS_PROCEEDING:
		t_trans_client::process_final(r);

		if (r->is_success()) {
			state = TS_TERMINATED;
		} else {
			// RFC 3261 17.1.1.3
			// construct ACK
			ack = new t_request(ACK);
			MEMMAN_NEW(ack);
			ack->uri = request->uri;
			ack->hdr_call_id = request->hdr_call_id;
			ack->hdr_from = request->hdr_from;
			ack->hdr_to = r->hdr_to;
			ack->hdr_via.add_via(
				request->hdr_via.via_list.front());
			ack->hdr_cseq.set_seqnr(request->hdr_cseq.seqnr);
			ack->hdr_cseq.set_method(ACK);
			ack->hdr_route = request->hdr_route;
			ack->hdr_max_forwards.set_max_forwards(MAX_FORWARDS);
			SET_HDR_USER_AGENT(ack->hdr_user_agent)

			// RFC 3261 22.1
			// Duplicate Authorization and Proxy-Authorization
			// headers from INVITE if the credentials in the
			// INVITE are accepted.
			if (r->code != R_401_UNAUTHORIZED &&
			    r->code != R_407_PROXY_AUTH_REQUIRED)
			{
				ack->hdr_authorization =
					request->hdr_authorization;
				ack->hdr_proxy_authorization =
					request->hdr_proxy_authorization;
			}
			
			// RFC 3263 4
			// ACK for non-2xx SIP responses to INVITE MUST be sent t
			// to the same host.
			request->get_current_destination(ip_port);
			ack->set_destination(ip_port);			

			// Send ACK
			evq_sender->push_network(ack, dst_ip_port);

			start_timer_D();
			state = TS_COMPLETED;
		}

		// Report to TU
		evq_trans_layer->push_user(r, tuid, id);
		break;
	case TS_COMPLETED:
		// A failure has been received. So 2XX is not
		// expected anymore. Discard 2XX.
		if (r->is_success()) {
			break;
		}

		// Retransmit ACK
		evq_sender->push_network(ack, dst_ip_port);
		break;
	default:
		break;
	}
}

void t_tc_invite::process_icmp(const t_icmp_msg &icmp) {
	log_file->write_report("ICMP error received.", "t_tc_invite::process_icmp");
	process_failure(FAIL_TRANSPORT);
}

void t_tc_invite::process_failure(t_failure failure) {
	t_response *r;

	switch(state) {
	case TS_CALLING:
		stop_timer_A();
		stop_timer_B();
		
		switch (failure) {
		case FAIL_TRANSPORT:
			// A transport failure indicates a kind of network problem.
			// So the server is not available. Generate an internal
			// 503 Service Unavailable repsponse to notify the TU.
			r = create_response(R_503_SERVICE_UNAVAILABLE);
			break;
		case FAIL_TIMEOUT:
			r = create_response(R_408_REQUEST_TIMEOUT);
			break;
		default:
			log_file->write_header("t_tc_invite::process_failure",
				LOG_NORMAL, LOG_WARNING);
			log_file->write_raw("Unknown type of failure: ");
			log_file->write_raw((int)failure);
			log_file->write_endl();
			log_file->write_footer();
			
			r = create_response(R_400_BAD_REQUEST);
			break;
		}
			

		log_file->write_header("t_tc_invite::process_failure",
			LOG_NORMAL, LOG_INFO);
		log_file->write_raw("Transaction failed.\n\n");
		log_file->write_raw("Send internal:\n");
		log_file->write_raw(r->encode());
		log_file->write_footer();

		evq_trans_layer->push_user(r, tuid, id);
		MEMMAN_DELETE(r);
		delete r;
		state = TS_TERMINATED;
		break;
	default:
		// In other states a response has been received already,
		// so this failure seems to be a mismatch. Discard.
		break;
	}
}

void t_tc_invite::timeout(t_sip_timer t) {
	t_response	*r;

	assert (t == TIMER_A || t == TIMER_B || t == TIMER_D);

	switch (state) {
	case TS_CALLING:
		switch (t) {
		case TIMER_A:
			// Resend request
			evq_sender->push_network(request, dst_ip_port);
			start_timer_A();
			break;
		case TIMER_B:
			stop_timer_A();
			timer_B = 0;
			// Report timer expiry to TU
			r = create_response(R_408_REQUEST_TIMEOUT);

			log_file->write_header("t_tc_invite::timeout",
				LOG_NORMAL, LOG_INFO);
			log_file->write_raw("Timer B expired.\n\n");
			log_file->write_raw("Send internal:\n");
			log_file->write_raw(r->encode());
			log_file->write_footer();

			evq_trans_layer->push_user(r, tuid, id);
			MEMMAN_DELETE(r);
			delete r;
			state = TS_TERMINATED;
			break;
		default:
			// Ignore expiry of other timers.
			// Other timers should have been stopped.
			break;
		}
		break;
	case TS_COMPLETED:
		switch (t) {
		case TIMER_D:
			timer_D = 0;
			state = TS_TERMINATED;
			break;
		default:
			// Ignore expiry of other timers.
			// Other timers should have been stopped.
			break;
		}
		break;
	default:
		// Ignore timer expiries in other states
		// Other timers should have been stopped.
		break;
	}
}

void t_tc_invite::abort(void) {
	t_response	*r;

	switch (state) {
	case TS_PROCEEDING:
		r = create_response(R_408_REQUEST_TIMEOUT, "Request Aborted");

		log_file->write_header("t_tc_invite::abort",
			LOG_NORMAL, LOG_INFO);
		log_file->write_raw("Invite transaction aborted.\n\n");
		log_file->write_raw("Send internal:\n");
		log_file->write_raw(r->encode());
		log_file->write_footer();

		evq_trans_layer->push_user(r, tuid, id);
		MEMMAN_DELETE(r);
		delete r;
		state = TS_TERMINATED;
		break;
	default:
		// Ignore abortion in other states.
		// In other states the request can be terminated in
		// a normal way.
		break;
	}
}

///////////////////////////////////////////////////////////
// RFC 3261 17.1.2
// Client non-INVITE transaction
///////////////////////////////////////////////////////////

void t_tc_non_invite::start_timer_E(void) {
	if (state == TS_PROCEEDING) duration_E = DURATION_T2;
	timer_E = transaction_mgr->start_timer(duration_E, TIMER_E, id);
	duration_E = 2 * duration_E;
	if (duration_E > DURATION_T2) duration_E = DURATION_T2;
}

void t_tc_non_invite::start_timer_F(void) {
	timer_F = transaction_mgr->start_timer(DURATION_F, TIMER_F, id);
}

void t_tc_non_invite::start_timer_K(void) {
	// RFC 3261 17.1.2.2
	// For reliable transports set timer K to zero seconds.
	if (dst_ip_port.transport == "udp") {
		timer_K = transaction_mgr->start_timer(DURATION_K, TIMER_K, id);
	} else {
		timer_K = transaction_mgr->start_timer(0, TIMER_K, id);
	}
}

void t_tc_non_invite::stop_timer_E(void) {
	if (timer_E) {
		transaction_mgr->stop_timer(timer_E);
		timer_E = 0;
	}
}

void t_tc_non_invite::stop_timer_F(void) {
	if (timer_F) {
		transaction_mgr->stop_timer(timer_F);
		timer_F = 0;
	}
}

void t_tc_non_invite::stop_timer_K(void) {
	if (timer_K) {
		transaction_mgr->stop_timer(timer_K);
		timer_K = 0;
	}
}

t_tc_non_invite::t_tc_non_invite(t_request *r, const t_ip_port &ip_port,
		unsigned short _tuid) :
	t_trans_client(r, ip_port, _tuid)
{
	assert(r->method != INVITE);

	state = TS_TRYING;
	duration_E = DURATION_E;
	
	// RFC 3261 17.1.2.2
	// Start timer E for unreliable transports.
	if (ip_port.transport == "udp") start_timer_E();
	
	// RFC 3261 17.1.2.2
	// Start timer F for all transports.
	start_timer_F();
	
	timer_K = 0;
}

t_tc_non_invite::~t_tc_non_invite() {
	stop_timer_E();
	stop_timer_F();
	stop_timer_K();
}

void t_tc_non_invite::process_provisional(t_response *r) {
	assert(r->is_provisional());

	switch (state) {
	case TS_TRYING:
	case TS_PROCEEDING:
		t_trans_client::process_provisional(r);
		state = TS_PROCEEDING;
		// Report to TU
		evq_trans_layer->push_user(r, tuid, id);
		break;
	default:
		// Discard provisional response in other states
		break;
	}
}

void t_tc_non_invite::process_final(t_response *r) {
	assert(r->is_final());

	switch (state) {
	case TS_TRYING:
	case TS_PROCEEDING:
		t_trans_client::process_final(r);
		stop_timer_E();
		stop_timer_F();
		// Report to TU
		evq_trans_layer->push_user(r, tuid, id);
		start_timer_K();
		state = TS_COMPLETED;
		break;
	case TS_COMPLETED:
		// The received response is a retransmission.
		// AS the final response is already received this
		// retransmission can be discarded.
		// fall through
	default:
		break;
	}
}

void t_tc_non_invite::process_icmp(const t_icmp_msg &icmp) {
	log_file->write_report("ICMP error received.", "t_tc_non_invite::process_icmp");
	process_failure(FAIL_TRANSPORT);
}

void t_tc_non_invite::process_failure(t_failure failure) {
	t_response *r;

	switch(state) {
	case TS_TRYING:
		stop_timer_E();
		stop_timer_F();
		
		switch (failure) {
		case FAIL_TRANSPORT:
			// A transport failure indicates a kind of network problem.
			// So the server is not available. Generate an internal
			// 503 Service Unavailable repsponse to notify the TU.
			r = create_response(R_503_SERVICE_UNAVAILABLE);
			break;
		case FAIL_TIMEOUT:
			r = create_response(R_408_REQUEST_TIMEOUT);
			break;
		default:
			log_file->write_header("t_tc_non_invite::process_failure",
				LOG_NORMAL, LOG_WARNING);
			log_file->write_raw("Unknown type of failure: ");
			log_file->write_raw((int)failure);
			log_file->write_endl();
			log_file->write_footer();
			
			r = create_response(R_400_BAD_REQUEST);
			break;
		}
			

		log_file->write_header("t_tc_non_invite::process_failure",
			LOG_NORMAL, LOG_INFO);
		log_file->write_raw("Transaction failed.\n\n");
		log_file->write_raw("Send internal:\n");
		log_file->write_raw(r->encode());
		log_file->write_footer();

		evq_trans_layer->push_user(r, tuid, id);
		MEMMAN_DELETE(r);
		delete r;
		state = TS_TERMINATED;
		break;
	default:
		// In other states a response has been received already,
		// so this failure seems to be a mismatch. Discard.
		break;
	}
}

void t_tc_non_invite::timeout(t_sip_timer t) {
	t_response *r;

	assert (t == TIMER_E || t == TIMER_F || t == TIMER_K);

	switch (state) {
	case TS_TRYING:
	case TS_PROCEEDING:
		switch (t) {
		case TIMER_E:
			// Resend request
			evq_sender->push_network(request, dst_ip_port);
			start_timer_E();
			break;
		case TIMER_F:
			timer_F = 0;
			stop_timer_E();
			// Report timer expiry to TU
			r = create_response(R_408_REQUEST_TIMEOUT);

			log_file->write_header("t_tc_non_invite::timeout",
				LOG_NORMAL, LOG_INFO);
			log_file->write_raw("Timer F expired.\n\n");
			log_file->write_raw("Send internal:\n");
			log_file->write_raw(r->encode());
			log_file->write_footer();

			evq_trans_layer->push_user(r, tuid, id);
			MEMMAN_DELETE(r);
			delete r;
			state = TS_TERMINATED;
			break;
		default:
			// Ignore expiry of other timers.
			// Other timers should have been stopped.
			break;
		}
		break;
	case TS_COMPLETED:
		switch (t) {
		case TIMER_K:
			timer_K = 0;
			state = TS_TERMINATED;
			break;
		default:
			// Ignore expiry of other timers.
			// Other timers should have been stopped.
			break;

		}
	default:
		// Ignore timer expiries in other states
		// Other timers should have been stopped.
		break;
	}
}

void t_tc_non_invite::abort(void) {
	t_response	*r;

	switch (state) {
	case TS_TRYING:
	case TS_PROCEEDING:
		stop_timer_E();
		stop_timer_F();
		r = create_response(R_408_REQUEST_TIMEOUT, "Request Aborted");

		log_file->write_header("t_tc_non_invite::abort",
			LOG_NORMAL, LOG_INFO);
		log_file->write_raw("Non-invite transaction aborted.\n\n");
		log_file->write_raw("Send internal:\n");
		log_file->write_raw(r->encode());
		log_file->write_footer();

		evq_trans_layer->push_user(r, tuid, id);
		MEMMAN_DELETE(r);
		delete r;
		state = TS_TERMINATED;
		break;
	default:
		// Ignore abortion in other states.
		// In other states the request can be terminated in
		// a normal way.
		break;
	}
}

///////////////////////////////////////////////////////////
// RFC 3261 17.2
// Server transaction
///////////////////////////////////////////////////////////

t_trans_server::t_trans_server(t_request *r, unsigned short _tuid) :
	t_transaction(r, _tuid), resp_100_trying_sent(false)
{
	t_trans_server	*t;
	t_tid		tid_cancel = 0;

	// Report to TU
	if (request->method == CANCEL) {
		t = transaction_mgr->find_cancel_target(r);
		if (t) tid_cancel = t->get_id();
		evq_trans_layer->push_user_cancel(r, tuid, id, tid_cancel);
	} else {
		evq_trans_layer->push_user(r, tuid, id);
	}
}

void t_trans_server::process_provisional(t_response *r) {
	t_ip_port ip_port;
	
	if (r->code == R_100_TRYING && resp_100_trying_sent) {
		// Send 100 Trying only once
		return;
	}

	t_transaction::process_provisional(r);
	r->get_destination(ip_port);
	if (ip_port.ipaddr == 0) {
		// The response cannot be sent.
		state = TS_TERMINATED;
		// Report failure to TU
		evq_trans_layer->push_failure(FAIL_TRANSPORT, id);
	} else {
		// Send response
		evq_sender->push_network(r, ip_port);
		
		if (r->code == R_100_TRYING) {
			resp_100_trying_sent = true;
		}
	}
}

void t_trans_server::process_final(t_response *r) {
	t_ip_port ip_port;

	t_transaction::process_final(r);
	r->get_destination(ip_port);

	if (ip_port.ipaddr == 0) {
		// The response cannot be sent.
		state = TS_TERMINATED;
		// Report failure to TU
		evq_trans_layer->push_failure(FAIL_TRANSPORT, id);
	} else {
		// Send response
		evq_sender->push_network(r, ip_port);
	}
}

void t_trans_server::process_retransmission(void) {
	// nothing to do
}

// RFC 3261 17.2.3
// NOTE: retransmission of an incoming INVITE for which a 2XX response
//       has been sent already is checked by the TU.
//       see dialog::is_invite_retrans
bool t_trans_server::match(t_request *r, bool cancel) const {
	t_via &orig_top_via = request->hdr_via.via_list.front();
	t_via &recv_top_via = r->hdr_via.via_list.front();

	if (recv_top_via.rfc3261_compliant()) {
		if (orig_top_via.branch != recv_top_via.branch) return false;
		if (orig_top_via.host != recv_top_via.host) return false;
		if (orig_top_via.port != recv_top_via.port) return false;

		switch(r->method) {
		case ACK:
			// return (request->hdr_cseq.method == INVITE);
			return (request->method == INVITE);
			break;
		case CANCEL:
			if (!cancel) {
				// return (request->hdr_cseq.method ==
				//	 		r->hdr_cseq.method);
				return (request->method == r->method);
			}

			// The target of CANCEL cannot be a CANCEL request
			// return (request->hdr_cseq.method != CANCEL);
			return (request->method != CANCEL);
			break;
		default:
			// return (request->hdr_cseq.method ==
			//	 		r->hdr_cseq.method);
			return (request->method == r->method);
			break;
		}
	}

	// Matching rules for backward compatibiliy with RFC 2543
	// TODO: verify rules for matching via headers
	switch (r->method) {
	case INVITE:
		return (request->method == INVITE &&
			request->uri.sip_match(r->uri) &&
			request->hdr_to.tag == r->hdr_to.tag &&
			request->hdr_from.tag == r->hdr_from.tag &&
			request->hdr_call_id.call_id ==
					 r->hdr_call_id.call_id &&
			request->hdr_cseq.seqnr == r->hdr_cseq.seqnr &&
			orig_top_via.host == recv_top_via.host &&
			orig_top_via.port == recv_top_via.port);
		break;
	case ACK:
		return (request->method == INVITE &&
			request->uri.sip_match(r->uri) &&
			request->hdr_from.tag == r->hdr_from.tag &&
			request->hdr_call_id.call_id ==
					 r->hdr_call_id.call_id &&
			request->hdr_cseq.seqnr == r->hdr_cseq.seqnr &&
			orig_top_via.host == recv_top_via.host &&
			orig_top_via.port == recv_top_via.port &&
			final != NULL &&
			final->hdr_to.tag == r->hdr_to.tag);
		break;
	case CANCEL:
		if (cancel) {
			return (request->uri.sip_match(r->uri) &&
				request->hdr_from.tag == r->hdr_from.tag &&
				request->hdr_call_id.call_id ==
					 r->hdr_call_id.call_id &&
				request->hdr_cseq.seqnr ==
					r->hdr_cseq.seqnr &&
				request->hdr_cseq.method != CANCEL &&
				orig_top_via.host == recv_top_via.host &&
				orig_top_via.port == recv_top_via.port);
		}
		// fall through
	default:
		return (request->uri.sip_match(r->uri) &&
			request->hdr_from.tag == r->hdr_from.tag &&
			request->hdr_call_id.call_id ==
					 r->hdr_call_id.call_id &&
			request->hdr_cseq == r->hdr_cseq &&
			orig_top_via.host == recv_top_via.host &&
			orig_top_via.port == recv_top_via.port);
		break;
	}

	// Should not get here
	return false;
}

bool t_trans_server::match(t_request *r) const {
	return match(r, false);
}

bool t_trans_server::match_cancel(t_request *r) const {
	assert(r->method == CANCEL);
	return match(r, true);
}

///////////////////////////////////////////////////////////
// RFC 3261 17.2.1
// Server INVITE transaction
///////////////////////////////////////////////////////////

void t_ts_invite::start_timer_G(void) {
	timer_G = transaction_mgr->start_timer(duration_G, TIMER_G, id);
	duration_G = 2 * duration_G;
	if (duration_G > DURATION_T2) duration_G = DURATION_T2;
}

void t_ts_invite::start_timer_H(void) {
	timer_H = transaction_mgr->start_timer(DURATION_H, TIMER_H, id);
}

void t_ts_invite::start_timer_I(void) {
	// RFC 17.2.1
	// Set timer I to T4 seconds for unreliable transports and to 0 for
	// reliable transports.
	if (request->src_ip_port.transport == "udp") {
		timer_I = transaction_mgr->start_timer(DURATION_I, TIMER_I, id);
	} else {
		timer_I = transaction_mgr->start_timer(0, TIMER_I, id);
	}
}

void t_ts_invite::stop_timer_G(void) {
	if (timer_G) {
		transaction_mgr->stop_timer(timer_G);
		timer_G = 0;
	}
}

void t_ts_invite::stop_timer_H(void) {
	if (timer_H) {
		transaction_mgr->stop_timer(timer_H);
		timer_H = 0;
	}
}

void t_ts_invite::stop_timer_I(void) {
	if (timer_I) {
		transaction_mgr->stop_timer(timer_I);
		timer_I = 0;
	}
}

t_ts_invite::t_ts_invite(t_request *r, unsigned short _tuid) :
	t_trans_server(r, _tuid)
{
	assert(r->method == INVITE);

	state = TS_PROCEEDING;
	ack = NULL;
	timer_G = 0;
	timer_H = 0;
	timer_I = 0;
	duration_G = DURATION_G;
}

t_ts_invite::~t_ts_invite() {
	if (ack != NULL) {
		MEMMAN_DELETE(ack);
		delete ack;
	}
	stop_timer_G();
	stop_timer_H();
	stop_timer_I();
}

void t_ts_invite::process_provisional(t_response *r) {
	assert(r->is_provisional());

	switch (state) {
	case TS_PROCEEDING:
		t_trans_server::process_provisional(r);
		break;
	default:
		// TU should not send a provisional response
		// in other states.
		assert(false);
		break;
	}
}

void t_ts_invite::process_final(t_response *r) {
	assert(r->is_final());

	switch (state) {
	case TS_PROCEEDING:
		t_trans_server::process_final(r);
		if (r->is_success()) {
			state = TS_TERMINATED;
		} else {
			// RFC 3261 17.2.1
			// Start timer G for unreliable transports.
			if (request->src_ip_port.transport == "udp") {
				start_timer_G();
			}
			
			// RFC 3261 17.2.1
			// Start timer H for all transports
			start_timer_H();
			
			state = TS_COMPLETED;
		}
		break;
	default:
		// No final responses are expected anymore. Discard.
		break;
	}
}

void t_ts_invite::process_retransmission(void) {
	t_ip_port ip_port;

	switch (state) {
	case TS_PROCEEDING:
		// Retransmit the latest provisional response (if present)
		t_trans_server::process_retransmission();
		if (provisional.size() > 0) {
			t_response *r = provisional.back();
			r->get_destination(ip_port);
			if (ip_port.ipaddr == 0) {
				// The response cannot be sent.
				state = TS_TERMINATED;
				// Report failure to TU
				evq_trans_layer->push_failure(
						FAIL_TRANSPORT, id);
			} else {
				// Send response
				evq_sender->push_network(r, ip_port);
			}
		}
		break;
	case TS_COMPLETED:
		// Retransmit the final response
		t_trans_server::process_retransmission();
		final->get_destination(ip_port);
		if (ip_port.ipaddr == 0) {
			// The response cannot be sent.
			state = TS_TERMINATED;
			// Report failure to TU
			evq_trans_layer->push_failure(FAIL_TRANSPORT, id);
		} else {
			// Send response
			evq_sender->push_network(final, ip_port);
		}
		break;
	default:
		// Retransmissions should not happen in other states.
		// Discard.
		break;
	}
}

void t_ts_invite::timeout(t_sip_timer t) {
	t_ip_port ip_port;

	assert(t == TIMER_G || t == TIMER_I || t == TIMER_H);

	switch (state) {
	case TS_COMPLETED:
		switch (t) {
		case TIMER_G:
			timer_G = 0;

			// Retransmit the final response
			final->get_destination(ip_port);
			if (ip_port.ipaddr == 0) {
				// The response cannot be sent.
				stop_timer_H();
				state = TS_TERMINATED;
				// Report failure to TU
				evq_trans_layer->push_failure(
						FAIL_TRANSPORT, id);
			} else {
				// Send response
				evq_sender->push_network(final, ip_port);
				start_timer_G();
			}
			break;
		case TIMER_H:
			timer_H = 0;
			stop_timer_G();
			state = TS_TERMINATED;
			// Report timer expiry to TU
			evq_trans_layer->push_failure(FAIL_TIMEOUT, id);
			break;
		default:
			// No other timers should be running. Discard.
			break;
		}
		break;
	case TS_CONFIRMED:
		switch (t) {
		case TIMER_I:
			timer_I = 0;
			state = TS_TERMINATED;
			break;
		default:
			// No other timers should be running. Discard.
			break;
		}
	default:
		// In other states no timers should be running.
		break;
	}
}

void t_ts_invite::acknowledge(t_request *ack_request) {
	assert(ack_request->method == ACK);

	switch (state) {
	case TS_COMPLETED:
		ack = (t_request *)ack_request->copy();
		stop_timer_G();
		stop_timer_H();
		start_timer_I();
		state = TS_CONFIRMED;
		// Report TU
		// ACK should not be reported to TU for non-2xx
		// evq_trans_layer->push_user(ack_request, tuid, id);
		break;
	default:
		// ACK is not expected in other states. Discard;
		break;
	}
}

///////////////////////////////////////////////////////////
// RFC 3261 17.2.2
// Server non-INVITE transaction
///////////////////////////////////////////////////////////

void t_ts_non_invite::start_timer_J(void) {
	// RFC 3261 17.2.2
	// For unreliable transports set timer J to 64*T1, for reliable
	// transports set it to 0.
	if (request->src_ip_port.transport == "udp") {
		timer_J = transaction_mgr->start_timer(DURATION_J, TIMER_J, id);
	} else {
		timer_J = transaction_mgr->start_timer(0, TIMER_J, id);
	}
}

void t_ts_non_invite::stop_timer_J(void) {
	if (timer_J) {
		transaction_mgr->stop_timer(timer_J);
		timer_J = 0;
	}
}

t_ts_non_invite::t_ts_non_invite(t_request *r, unsigned short _tuid) :
	t_trans_server(r, _tuid)
{
	assert(r->method != INVITE);
	timer_J = 0;
	state = TS_TRYING;
}

t_ts_non_invite::~t_ts_non_invite() {
	stop_timer_J();
}

void t_ts_non_invite::process_provisional(t_response *r) {
	assert(r->is_provisional());

	switch (state) {
	case TS_TRYING:
	case TS_PROCEEDING:
		t_trans_server::process_provisional(r);
		state = TS_PROCEEDING;
		break;
	default:
		// TU should not send a provisional response
		// in other states.
		assert(false);
		break;
	}
}

void t_ts_non_invite::process_final(t_response *r) {
	assert(r->is_final());

	switch (state) {
	case TS_TRYING:
	case TS_PROCEEDING:
		t_trans_server::process_final(r);
		start_timer_J();
		state = TS_COMPLETED;
		break;
	default:
		// No final responses are expected anymore. Discard.
		break;
	}
}

void t_ts_non_invite::process_retransmission(void) {
	t_ip_port	ip_port;
	t_response 	*r;

	switch (state) {
	case TS_PROCEEDING:
		// Retransmit the latest provisional response
		t_trans_server::process_retransmission();
		r = provisional.back();
		r->get_destination(ip_port);
		if (ip_port.ipaddr == 0) {
			// The response cannot be sent.
			state = TS_TERMINATED;
			// Report failure to TU
			evq_trans_layer->push_failure(FAIL_TRANSPORT, id);
		} else {
			// Send response
			evq_sender->push_network(r, ip_port);
		}
		break;
	case TS_COMPLETED:
		// Retransmit the final response
		t_trans_server::process_retransmission();
		final->get_destination(ip_port);
		if (ip_port.ipaddr == 0) {
			// The response cannot be sent.
			stop_timer_J();
			state = TS_TERMINATED;
			// Report failure to TU
			evq_trans_layer->push_failure(FAIL_TRANSPORT, id);
		} else {
			// Send response
			evq_sender->push_network(final, ip_port);
		}
		break;
	default:
		// Retransmissions should not happen in other states.
		// Discard.
		break;
	}
}

void t_ts_non_invite::timeout(t_sip_timer t) {
	assert (t == TIMER_J);

	switch (state) {
	case TS_COMPLETED:
		switch (t) {
		case TIMER_J:
			timer_J = 0;
			state = TS_TERMINATED;
			break;
		default:
			break;
		}
	default:
		break;
	}
}
