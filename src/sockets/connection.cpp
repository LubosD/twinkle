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
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "connection.h"

#include <algorithm>
#include <iostream>

#include "connection_table.h"
#include "log.h"
#include "sys_settings.h"
#include "util.h"
#include "audits/memman.h"
#include "parser/parse_ctrl.h"

extern t_connection_table *connection_table;

using namespace std;

t_connection::t_connection(t_socket *socket) : 
	socket_(socket), 
	sip_msg_(NULL),
	pos_send_buf_(0),
	idle_time_(0),
	can_reuse_(true)
{}

t_connection::~t_connection() {
	MEMMAN_DELETE(socket_);
	delete socket_;
}

t_socket *t_connection::get_socket(void) {
	return socket_;
}

t_connection::size_type t_connection::data_size(void) const {
	return read_buf_.size();
}

void t_connection::read(bool &connection_closed) {
	connection_closed = false;
	char buf[READ_BLOCK_SIZE];
	
	ssize_t nread = socket_->recv(buf, READ_BLOCK_SIZE);
	
	if (nread > 0) {
		read_buf_.append(buf, buf + nread);
	} else {
		connection_closed = true;
	}
	
	idle_time_ = 0;
}

void t_connection::write(void) {
	if (send_buf_.empty()) return;
	
	ssize_t nwrite = send_buf_.size() - pos_send_buf_;
	if ((ssize_t)WRITE_BLOCK_SIZE < nwrite) nwrite = WRITE_BLOCK_SIZE;
	ssize_t nwritten = socket_->send(send_buf_.c_str() + pos_send_buf_, nwrite);
	pos_send_buf_ += nwritten;
	
	if (pos_send_buf_ >= send_buf_.size()) {
		// All data written
		send_buf_.clear();
		pos_send_buf_ = 0;
	}
}

ssize_t t_connection::send(const char *data, int data_size) {
	ssize_t bytes_sent = socket_->send(data, data_size);
	idle_time_ = 0;
	
	return bytes_sent;
}

void t_connection::async_send(const char *data, int data_size) {
	send_buf_ += string(data, data_size);
	connection_table->restart_write_select();
}

t_sip_message *t_connection::get_sip_msg(string &raw_headers, string &raw_body, bool &error,
		bool &msg_too_large) 
{
	string log_msg;
	
	raw_headers.clear();
	raw_body.clear();
	error = false;
	msg_too_large = false;
	
	if (!sip_msg_) {
		// RFC 3261 7.5
		// Ignore CRLF preceding the start-line of a SIP message
		while (read_buf_.size() >= 2 && read_buf_.substr(0, 2) == string(CRLF)) {
			remove_data(2);
		}
	
		// A complete list of headers has not been read yet, try
		// to find the boundary between headers and body.
		string seperator = string(CRLF) + string(CRLF);
		string::size_type pos_body = read_buf_.find(seperator);
		
		if (pos_body == string::npos) {
			// Still no complete list of headers.
			if (read_buf_.size() > sys_config->get_sip_max_tcp_size()) {
				log_file->write_report("Message too large",
					"t_connection::get_sip_msg", LOG_SIP, LOG_WARNING);
				error = true;
			}
			return NULL;
		}
		
		pos_body += seperator.size();
		
		// Parse SIP headers
		raw_sip_headers_ = read_buf_.substr(0, pos_body);
		list<string> parse_errors;
		try {
			sip_msg_ = t_parser::parse(raw_sip_headers_, parse_errors);
		}
		catch (int) {
			// Discard malformed SIP messages.
			log_msg = "Invalid SIP message.\n";
			log_msg += "Fatal parse error in headers.\n\n";
			log_msg += to_printable(raw_sip_headers_);
			log_file->write_report(log_msg, "t_connection::get_sip_msg", LOG_SIP, LOG_DEBUG);
			
			error = true;
			return NULL;
		}
		
		// Log non-fatal parse errors.
		if (!parse_errors.empty()) {
			log_msg = "Parse errors:\n";
			log_msg += "\n";
			for (list<string>::iterator i = parse_errors.begin(); 
			     i != parse_errors.end(); i++) 
			{
				log_msg += *i;
				log_msg += "\n";
			}
			log_msg += "\n";
			log_file->write_report(log_msg, "t_connection::get_sip_msg", LOG_SIP, LOG_DEBUG);
		}
		
		get_remote_address(sip_msg_->src_ip_port.ipaddr, sip_msg_->src_ip_port.port);
		sip_msg_->src_ip_port.transport = "tcp";
		
		// Remove the processed headers from the read buffer.
		remove_data(pos_body);
	}
	
	// RFC 3261 18.4
	// The Content-Length header field MUST be used with stream oriented transports.
	if (!sip_msg_->hdr_content_length.is_populated()) {
		// The transaction layer will send an error response.
		log_file->write_report("Content-Length header is missing.",
				"t_connection::get_sip_msg", LOG_SIP, LOG_WARNING);
	} else {
		if (read_buf_.size() < sip_msg_->hdr_content_length.length) {
			// No full body read yet.
			if (read_buf_.size() + raw_sip_headers_.size() <=
						sys_config->get_sip_max_tcp_size())
			{
				return NULL;
			} else {
				log_file->write_report("Message too large",
					"t_connection::get_sip_msg", LOG_SIP, LOG_WARNING);
					
				msg_too_large = true;
			}
		} else {
			if (sip_msg_->hdr_content_length.length > 0) {
				raw_body = read_buf_.substr(0, sip_msg_->hdr_content_length.length);
				remove_data(sip_msg_->hdr_content_length.length);
			}
		}
	}
	
	// Return data to caller. Clear internally cached data.
	t_sip_message *msg = sip_msg_;
	sip_msg_ = NULL;
	raw_headers = raw_sip_headers_;
	raw_sip_headers_.clear();
	
	return msg;
}

string t_connection::get_data(size_t nbytes) const {
	size_t nread = min(nbytes, read_buf_.size());

	return read_buf_.substr(0, nread);
}

void t_connection::remove_data(size_t nbytes) {
	if (nbytes == 0) return;
	
	if (nbytes >= read_buf_.size()) {
		read_buf_.clear();
	} else {
		read_buf_.erase(0, nbytes);
	}
}

void t_connection::get_remote_address(IPaddr &remote_addr, unsigned short &remote_port) {
	remote_addr = 0;
	remote_port = 0;

	try {
		t_socket_tcp *tcp_socket = dynamic_cast<t_socket_tcp *>(socket_);
		if (tcp_socket) {
			tcp_socket->get_remote_address(remote_addr, remote_port);
		} else {
			log_file->write_report("Socket is not connection oriented.",
					"t_connection::get_sip_msg",
					LOG_NORMAL, LOG_WARNING);
		}
	}
	catch (int err) {
			string errmsg = get_error_str(err);
			string log_msg = "Cannot get remote address: ";
			log_msg += errmsg;
			log_file->write_report(log_msg, "t_connection::get_sip_msg", 
					LOG_NORMAL, LOG_WARNING);
	}
}

unsigned long t_connection::increment_idle_time(unsigned long interval) {
	idle_time_ += interval;
	return idle_time_;
}

unsigned long t_connection::get_idle_time(void) const {
	return idle_time_;
}

bool t_connection::has_data_to_send(void) const {
	return !send_buf_.empty();
}

void t_connection::set_reuse(bool reuse) {
	can_reuse_ = reuse;
}

bool t_connection::may_reuse(void) const {
	return can_reuse_;
}

void t_connection::add_registered_uri(const t_url &uri) {
	// Add the URI if it is not in the set.
	if (find(registered_uri_set_.begin(), registered_uri_set_.end(), uri) == registered_uri_set_.end())
	{
		registered_uri_set_.push_back(uri);
	}
}

void t_connection::remove_registered_uri(const t_url &uri) {
	registered_uri_set_.remove(uri);
}

void t_connection::update_registered_uri_set(const t_request *req) {
	assert(req->method == REGISTER);
	
	if (req->is_registration_request()) {
		add_registered_uri(req->hdr_to.uri);
	} else if (req->is_de_registration_request()) {
		remove_registered_uri(req->hdr_to.uri);
	}
}

const list<t_url> &t_connection::get_registered_uri_set(void) const {
	return registered_uri_set_;
}

bool t_connection::has_registered_uri(void) const {
	return !registered_uri_set_.empty();
}
