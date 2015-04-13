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

#include <cstring>
#include "stun_transaction.h"
#include "events.h"
#include "log.h"
#include "phone.h"
#include "sys_settings.h"
#include "transaction_mgr.h"
#include "translator.h"
#include "util.h"
#include "audits/memman.h"

extern t_transaction_mgr	*transaction_mgr;
extern t_event_queue		*evq_trans_layer;
extern t_event_queue		*evq_trans_mgr;
extern t_event_queue		*evq_sender;
extern t_phone			*phone;


bool get_stun_binding(t_user *user_config, unsigned short src_port, unsigned long &mapped_ip,
	unsigned short &mapped_port, int &err_code, string &err_reason)
{
	list<t_ip_port> destinations = 
		user_config->get_stun_server().get_h_ip_srv("udp");
	
	if (destinations.empty()) {
		// Cannot resolve STUN server address.
		log_file->write_header("::get_stun_binding", LOG_NORMAL, LOG_CRITICAL);
		log_file->write_raw("Failed to resolve: ");
		log_file->write_raw(user_config->get_stun_server().encode());
		log_file->write_endl();
		log_file->write_raw("Return internal STUN bind error: 404 Not Found");
		log_file->write_endl();
		log_file->write_footer();
		
		err_code = 404;
		err_reason = "Not Found";
		return false;
	}
	
	int num_transmissions = 0;
	int wait_intval = DUR_STUN_START_INTVAL;

	t_socket_udp sock(src_port);
	sock.connect(destinations.front().ipaddr, destinations.front().port);
		
	// Build STUN request
	char buf[STUN_MAX_MESSAGE_SIZE + 1];
	StunMessage req_bind;
	StunAtrString stun_null_str;
	stun_null_str.sizeValue = 0;	
	stunBuildReqSimple(&req_bind, stun_null_str, false, false);	
	char req_msg[STUN_MAX_MESSAGE_SIZE];
	int req_msg_size = stunEncodeMessage(req_bind, req_msg, 
		STUN_MAX_MESSAGE_SIZE, stun_null_str, false);
		
	// Send STUN request and retransmit till a response is received.
	while (num_transmissions < STUN_MAX_TRANSMISSIONS) {
		bool ret;

		try {
			sock.send(req_msg, req_msg_size);
		}
		catch (int err) {
			// Socket error (probably ICMP error)
			// Failover to next destination
			log_file->write_report("Send failed. Failover to next destination.",
					"::get_stun_binding");	
						
			destinations.pop_front();
			if (destinations.empty()) {
				log_file->write_report("No next destination for failover.",
					"::get_stun_binding");
				break;
			}
			
			num_transmissions = 0;
			wait_intval = DUR_STUN_START_INTVAL;
			sock.connect(destinations.front().ipaddr, destinations.front().port);
			continue;
		}
		
		log_file->write_header("::get_stun_binding", LOG_STUN);
		log_file->write_raw("Send to: ");
		log_file->write_raw(h_ip2str(destinations.front().ipaddr));
		log_file->write_raw(":");
		log_file->write_raw(destinations.front().port);
		log_file->write_endl();
		log_file->write_raw(stunMsg2Str(req_bind));
		log_file->write_footer();
			
		try {
			ret = sock.select_read(wait_intval);
		}
		catch (int err) {
			// Socket error (probably ICMP error)
			// Failover to next destination
			log_file->write_report("Select failed. Failover to next destination.",
					"::get_stun_binding");	
						
			destinations.pop_front();
			if (destinations.empty()) {
				log_file->write_report("No next destination for failover.",
					"::get_stun_binding");
				break;
			}
			
			num_transmissions = 0;
			wait_intval = DUR_STUN_START_INTVAL;
			sock.connect(destinations.front().ipaddr, destinations.front().port);
			continue;
		}
			
		if (!ret) {
			// Time out
			num_transmissions++;
			if (wait_intval < DUR_STUN_MAX_INTVAL) {
				wait_intval *= 2;
			}
			continue;
		}
			
		// A message has been received
		int resp_msg_size;
		try {
			resp_msg_size = sock.recv(buf, STUN_MAX_MESSAGE_SIZE + 1);
		}
		catch (int err) {
			// Socket error (probably ICMP error)
			// Failover to next destination
			log_file->write_report("Recv failed. Failover to next destination.",
					"::get_stun_binding");	
						
			destinations.pop_front();
			if (destinations.empty()) {
				log_file->write_report("No next destination for failover.",
					"::get_stun_binding");
				break;
			}
			
			num_transmissions = 0;
			wait_intval = DUR_STUN_START_INTVAL;
			sock.connect(destinations.front().ipaddr, destinations.front().port);
			continue;
		}
			
		StunMessage resp_bind;
		
		if (!stunParseMessage(buf, resp_msg_size, resp_bind, false)) {
			log_file->write_report(
				"Received faulty STUN message", "::get_stun_binding", 
					LOG_STUN);
			num_transmissions++;
			if (wait_intval < DUR_STUN_MAX_INTVAL) {
				wait_intval *= 2;
			}
			continue;
		}
		
		log_file->write_header("::get_stun_binding", LOG_STUN);
		log_file->write_raw("Received from: ");
		log_file->write_raw(h_ip2str(destinations.front().ipaddr));
		log_file->write_raw(":");
		log_file->write_raw(destinations.front().port);
		log_file->write_endl();
		log_file->write_raw(stunMsg2Str(resp_bind));
		log_file->write_footer();
		
		// Check if id in msgHdr matches
		if (!stunEqualId(resp_bind, req_bind)) {
			num_transmissions++;
			if (wait_intval < DUR_STUN_MAX_INTVAL) {
				wait_intval *= 2;
			}
			continue;
		}
				
		if (resp_bind.msgHdr.msgType == BindResponseMsg && 
		    resp_bind.hasMappedAddress) {
		    	// Bind response received
			mapped_ip = resp_bind.mappedAddress.ipv4.addr;
			mapped_port = resp_bind.mappedAddress.ipv4.port;
			return true;
		}
			
		if (resp_bind.msgHdr.msgType == BindErrorResponseMsg &&
		    resp_bind.hasErrorCode) 
		{
			// Bind error received
			err_code = resp_bind.errorCode.errorClass * 100 +
				   resp_bind.errorCode.number;
			char s[STUN_MAX_STRING + 1];
			strncpy(s, resp_bind.errorCode.reason, STUN_MAX_STRING);
			s[STUN_MAX_STRING] = 0;
			err_reason = s;
			return false;
		}
			
		// A wrong response has been received.
		log_file->write_report(
			"Invalid STUN response received", "::get_stun_binding", 
				LOG_NORMAL);

		err_code = 500;
		err_reason = "Server Error";
		return false;
	}
		
	// Request timed out
	log_file->write_report("STUN request timeout", "::get_stun_binding", 
			LOG_NORMAL);
				
	err_code = 408;
	err_reason = "Request Timeout";
	return false;
}

bool stun_discover_nat(t_phone_user *pu, string &err_msg) {
	t_user *user_config = pu->get_user_profile();

	// By default enable STUN. If for some reason we cannot perform
	// NAT discovery, then enable STUN. It will not harm, but only
	// create non-needed STUN transactions if we are not behind a NAT.
	pu->use_stun = true;
	pu->use_nat_keepalive = true;

	list<t_ip_port> destinations = 
		user_config->get_stun_server().get_h_ip_srv("udp");

	if (destinations.empty()) {
		// Cannot resolve STUN server address.
		log_file->write_header("::main", LOG_NORMAL, LOG_CRITICAL);
		log_file->write_raw("Failed to resolve: ");
		log_file->write_raw(user_config->get_stun_server().encode());
		log_file->write_endl();
		log_file->write_footer();

		err_msg = TRANSLATE("Cannot resolve STUN server: %1");
		err_msg = replace_first(err_msg, "%1", user_config->get_stun_server().encode());
		return false;
	}

	while (!destinations.empty()) {
		StunAddress4 stun_ip4;
		stun_ip4.addr = destinations.front().ipaddr;
		stun_ip4.port = destinations.front().port;
		
		NatType nat_type = stunNatType(stun_ip4, false);
		log_file->write_header("::main");
		log_file->write_raw("STUN NAT type discovery for ");
		log_file->write_raw(user_config->get_profile_name());
		log_file->write_endl();
		log_file->write_raw("NAT type: ");
		log_file->write_raw(stunNatType2Str(nat_type));
		log_file->write_endl();
		log_file->write_footer();
		
		switch (nat_type) {
		case StunTypeOpen:
			// STUN is not needed.
			pu->use_stun = false;
			pu->use_nat_keepalive = false;
			return true;
		case StunTypeSymNat:
			err_msg = TRANSLATE("You are behind a symmetric NAT.\nSTUN will not work.\nConfigure a public IP address in the user profile\nand create the following static bindings (UDP) in your NAT.");
			err_msg += "\n\n";
			err_msg += TRANSLATE("public IP: %1 --> private IP: %2 (SIP signaling)");
			err_msg = replace_first(err_msg, "%1", int2str(sys_config->get_sip_port()));
			err_msg = replace_first(err_msg, "%2", int2str(sys_config->get_sip_port()));
			err_msg += "\n";
			err_msg += TRANSLATE("public IP: %1-%2 --> private IP: %3-%4 (RTP/RTCP)");
			err_msg = replace_first(err_msg, "%1", int2str(sys_config->get_rtp_port()));
			err_msg = replace_first(err_msg, "%2", int2str(sys_config->get_rtp_port() + 5));
			err_msg = replace_first(err_msg, "%3", int2str(sys_config->get_rtp_port()));
			err_msg = replace_first(err_msg, "%4", int2str(sys_config->get_rtp_port() + 5));
			
			pu->use_stun = false;
			pu->use_nat_keepalive = false;
			return false;
		case StunTypeSymFirewall:
			// STUN is not needed as we are on a pubic IP.
			// NAT keep alive is needed however to keep the firewall open.
			pu->use_stun = false;
			return true;
		case StunTypeBlocked:
			destinations.pop_front();
			
			// The code for NAT type discovery does not handle
			// ICMP errors. So if the conclusion is that the network
			// connection is blocked, it might be due to a down STUN
			// server. Try alternative destination if avaliable.
		
			if (destinations.empty()) {
				err_msg = TRANSLATE("Cannot reach the STUN server: %1");
				err_msg = replace_first(err_msg, "%1",
						user_config->get_stun_server().encode());
				err_msg += "\n\n";
				err_msg += TRANSLATE("If you are behind a firewall then you need to open the following UDP ports.");
				err_msg += "\n";
				err_msg += TRANSLATE("Port %1 (SIP signaling)");
				err_msg = replace_first(err_msg, "%1",
						int2str(sys_config->get_sip_port()));
				err_msg += "\n";
				err_msg += TRANSLATE("Ports %1-%2 (RTP/RTCP)");
				err_msg = replace_first(err_msg, "%1",
						int2str(sys_config->get_rtp_port()));
				err_msg = replace_first(err_msg, "%2",
						int2str(sys_config->get_rtp_port() + 5));
						
				return false;
			}
			
			log_file->write_report("Failover to next destination.",
				"::stun_discover_nat");			
			break;
		case StunTypeFailure:
			destinations.pop_front();
			log_file->write_report("Failover to next destination.",
				"::stun_discover_nat");
			break;
		default:
			// Use STUN.
			return true;
		}
	}

	err_msg = TRANSLATE("NAT type discovery via STUN failed.");	
	return false;
}


// Main function for STUN listener thread for media STUN requests.
void *stun_listen_main(void *arg) {
	char		buf[STUN_MAX_MESSAGE_SIZE + 1];
	int		data_size;
	
	t_socket_udp *sock = (t_socket_udp *)arg;
	
	while(true) {
		try {
			data_size = sock->recv(buf, STUN_MAX_MESSAGE_SIZE + 1);
		} catch (int err) {
			string msg("Failed to receive STUN response for media.\n");
			msg += get_error_str(err);
			log_file->write_report(msg, "::stun_listen_main",
				LOG_NORMAL, LOG_CRITICAL);
				
			// The request will timeout, no need to send a response now.
				
			return NULL;
		}
		
		StunMessage m;
		
		if (!stunParseMessage(buf, data_size, m, false)) {
			log_file->write_report("Faulty STUN message", "::stun_listen_main");
			continue;
		}
		
		log_file->write_header("::stun_listen_main", LOG_STUN);
		log_file->write_raw("Received: ");
		log_file->write_raw(stunMsg2Str(m));
		log_file->write_footer();
	
		evq_trans_mgr->push_stun_response(&m, 0, 0);
	}
}

//////////////////////////////////////////////
// Base STUN transaction
//////////////////////////////////////////////

t_mutex t_stun_transaction::mtx_class;
t_tid t_stun_transaction::next_id = 1;

t_stun_transaction::t_stun_transaction(t_user *user, StunMessage *r,
			   unsigned short _tuid, const list<t_ip_port> &dst) 
{
	mtx_class.lock();
	id = next_id++;
	if (next_id == 65535) next_id = 1;
	mtx_class.unlock();
	
	state = TS_NULL;
	request = new StunMessage(*r);
	MEMMAN_NEW(request);
	tuid = _tuid;
	
	dur_req_timeout = DUR_STUN_START_INTVAL;
	num_transmissions = 0;
	
	destinations = dst;
	
	user_config = user->copy();
}

t_stun_transaction::~t_stun_transaction() {
	MEMMAN_DELETE(request);
	delete request;
	MEMMAN_DELETE(user_config);
	delete user_config;
}

t_tid t_stun_transaction::get_id(void) const {
	return id;
}

t_trans_state t_stun_transaction::get_state(void) const {
	return state;
}

void t_stun_transaction::start_timer_req_timeout(void) {
	timer_req_timeout = transaction_mgr->start_stun_timer(dur_req_timeout,
		STUN_TMR_REQ_TIMEOUT, id);
		
	// RFC 3489 9.3
	// Double the retransmision interval till a maximum
	if (dur_req_timeout < DUR_STUN_MAX_INTVAL) {
		dur_req_timeout = 2 * dur_req_timeout;
	}
}

void t_stun_transaction::stop_timer_req_timeout(void) {
	if (timer_req_timeout) {
		transaction_mgr->stop_timer(timer_req_timeout);
		timer_req_timeout = 0;
	}
}

void t_stun_transaction::process_response(StunMessage *r) {
	stop_timer_req_timeout();
	evq_trans_layer->push_stun_response(r, tuid, id);
	state = TS_TERMINATED;
}

void t_stun_transaction::process_icmp(const t_icmp_msg &icmp) {
	stop_timer_req_timeout();
	
	log_file->write_report("Failover to next destination.",
				"t_stun_transaction::process_icmp");	
				
	destinations.pop_front();
	if (destinations.empty()) {
		log_file->write_report("No next destination for failover.",
				"t_stun_transaction::process_icmp");
						
		log_file->write_header("t_stun_transaction::process_icmp",
			LOG_NORMAL, LOG_INFO);
		log_file->write_raw("ICMP error received.\n\n");
		log_file->write_raw("Send internal: 500 Server Error\n");
		log_file->write_footer();	
	
		// No server could be reached, Notify the TU with 500 Server
		// Error.
		StunMessage *resp = stunBuildError(*request, 500, "Server Error");
		evq_trans_layer->push_stun_response(resp, tuid, id);
		MEMMAN_DELETE(resp);
		delete resp;
		
		state = TS_TERMINATED;
		return;
	}
	
	// Failover to next destination
	evq_sender->push_stun_request(user_config, request, TYPE_STUN_SIP, tuid, id,
		destinations.front().ipaddr, destinations.front().port);
	num_transmissions = 1;
	dur_req_timeout = DUR_STUN_START_INTVAL;
	start_timer_req_timeout();
}

void t_stun_transaction::timeout(t_stun_timer t) {
	// RFC 3489 9.3
	if (num_transmissions < STUN_MAX_TRANSMISSIONS) {
		retransmit();
		start_timer_req_timeout();
		return;
	}
	
	// Report timeout to TU
	StunMessage *timeout_resp;
	timeout_resp = stunBuildError(*request, 408, "Request Timeout");
	log_file->write_report("STUN request timeout", "t_stun_transaction::timeout", 
			LOG_NORMAL);
	
	evq_trans_layer->push_stun_response(timeout_resp, tuid, id);
	MEMMAN_DELETE(timeout_resp);
	delete timeout_resp;
	
	state = TS_TERMINATED;
}

bool t_stun_transaction::match(StunMessage *resp) const {
	return stunEqualId(*resp, *request);
}

// An ICMP error matches a transaction when the destination IP address/port
// of the packet that caused the ICMP error equals the destination 
// IP address/port of the transaction. Other information of the packet causing
// the ICMP error is not available.
// In theory when multiple transactions are open for the same destination, the
// wrong transaction may process the ICMP error. In practice this should rarely
// happen as the destination will be unreachable for all those transactions.
// If it happens a transaction gets aborted.
bool t_stun_transaction::match(const t_icmp_msg &icmp) const {
	return (destinations.front().ipaddr == icmp.ipaddr && 
	        destinations.front().port == icmp.port);
}

//////////////////////////////////////////////
// SIP STUN transaction
//////////////////////////////////////////////

void t_sip_stun_trans::retransmit(void) {
	// The SIP UDP sender will send out the STUN request.
	evq_sender->push_stun_request(user_config, request, TYPE_STUN_SIP, tuid, id,
		destinations.front().ipaddr, destinations.front().port);
	num_transmissions++;
}

t_sip_stun_trans::t_sip_stun_trans(t_user *user, StunMessage *r,
			unsigned short _tuid, const list<t_ip_port> &dst) :
		t_stun_transaction(user, r, _tuid, dst)
{
	// The SIP UDP sender will send out the STUN request.
	evq_sender->push_stun_request(user_config, request, TYPE_STUN_SIP, tuid, id,
		destinations.front().ipaddr, destinations.front().port);
	num_transmissions++;
	start_timer_req_timeout();	
	state = TS_PROCEEDING;
}

//////////////////////////////////////////////
// Media STUN transaction
//////////////////////////////////////////////

// TODO: this code is not used anymore. Remove?

void t_media_stun_trans::retransmit(void) {
	// Retransmit the STUN request
	StunAtrString stun_pass;
	stun_pass.sizeValue = 0;
	char m[STUN_MAX_MESSAGE_SIZE];
	int msg_size = stunEncodeMessage(*request, m, STUN_MAX_MESSAGE_SIZE, stun_pass, false);
	
	try {
		sock->sendto(destinations.front().ipaddr, destinations.front().port, 
			m, msg_size);
	} catch (int err) {
		string msg("Failed to send STUN request for media.\n");
		msg += get_error_str(err);
		log_file->write_report(msg, "::t_media_stun_trans::retransmit",
			LOG_NORMAL, LOG_CRITICAL);
			
		StunMessage *resp;
		resp = stunBuildError(*request, 500, "Could not send request");

		evq_trans_layer->push_stun_response(resp, tuid, id);
		MEMMAN_DELETE(resp);
		delete resp;
			
		return;
	}
	
	num_transmissions++;
}

t_media_stun_trans::t_media_stun_trans(t_user *user, StunMessage *r,
			 unsigned short _tuid, const list<t_ip_port> &dst, 
			 unsigned short src_port) :
		t_stun_transaction(user, r, _tuid, dst)
{
	thr_listen = NULL;
	
	try {
		sock = new t_socket_udp(src_port);
		MEMMAN_NEW(sock);
		sock->connect(destinations.front().ipaddr, destinations.front().port);
	} catch (int err) {
		string msg("Failed to create a UDP socket (STUN) on port ");
		msg += int2str(src_port);
		msg += "\n";
		msg += get_error_str(err);
		log_file->write_report(msg, "t_media_stun_trans::t_media_stun_trans", LOG_NORMAL, 
			LOG_CRITICAL);
		delete sock;
		sock = NULL;
		
		StunMessage *resp;
		resp = stunBuildError(*request, 500, "Could not create socket");

		evq_trans_layer->push_stun_response(resp, tuid, id);
		MEMMAN_DELETE(resp);
		delete resp;
		
		return;
	}
	
	// Send STUN request
	StunAtrString stun_pass;
	stun_pass.sizeValue = 0;
	char m[STUN_MAX_MESSAGE_SIZE];
	int msg_size = stunEncodeMessage(*r, m, STUN_MAX_MESSAGE_SIZE, stun_pass, false);
	
	try {
		sock->send(m, msg_size);
	} catch (int err) {
		string msg("Failed to send STUN request for media.\n");
		msg += get_error_str(err);
		log_file->write_report(msg, "::t_media_stun_trans::t_media_stun_trans",
			LOG_NORMAL, LOG_CRITICAL);

		StunMessage *resp;
		resp = stunBuildError(*request, 500, "Failed to send request");

		evq_trans_layer->push_stun_response(resp, tuid, id);
		MEMMAN_DELETE(resp);
		delete resp;
		
		return;
	}
	
	num_transmissions++;
	
	try {
		thr_listen = new t_thread(stun_listen_main, sock);
		MEMMAN_NEW(thr_listen);
	} catch (int) {
		log_file->write_report("Failed to create STUN listener thread.",
			"::t_media_stun_trans::t_media_stun_trans",
			LOG_NORMAL, LOG_CRITICAL);
		delete thr_listen;
		thr_listen = NULL;

		StunMessage *resp;
		resp = stunBuildError(*request, 500, "Failed to create STUN listen thread");

		evq_trans_layer->push_stun_response(resp, tuid, id);
		MEMMAN_DELETE(resp);
		delete resp;		
		
		return;
	}
	
	start_timer_req_timeout();
	state = TS_PROCEEDING;
}

t_media_stun_trans::~t_media_stun_trans() {
	if (sock) {
		MEMMAN_DELETE(sock);
		delete sock;
	}
	
	if (thr_listen) {
		thr_listen->cancel();
		thr_listen->join();
		MEMMAN_DELETE(thr_listen);
		delete thr_listen;
	}
}


