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

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "dnssrv.h"
#include "log.h"
#include "socket.h"
#include "url.h"
#include "user.h"
#include "util.h"

using namespace std;

unsigned short get_default_port(const string &protocol) {
	if (protocol == "mailto")	return 25;
	if (protocol == "http")		return 80;
	if (protocol == "sip")		return 5060;
	if (protocol == "sips")		return 5061;
	if (protocol == "stun")		return 3478;

	return 0;
}

unsigned long gethostbyname(const string &name) {
	struct hostent *h;
	
	h = gethostbyname(name.c_str());
	if (h == NULL) return 0;
	return ntohl(*((unsigned long *)h->h_addr));
}

list<unsigned long> gethostbyname_all(const string &name) {
	struct hostent *h;
	list<unsigned long> l;
	
	h = gethostbyname(name.c_str());
	if (h == NULL) return l;
	
	char **ipaddr = h->h_addr_list;
	while (*ipaddr) {
		l.push_back(ntohl(*((unsigned long *)(*ipaddr))));
		ipaddr++;
	}
	
	return l;
}

string get_local_hostname(void) {
	char name[256];
	int rc = gethostname(name, 256);
	
	if (rc < 0) {
		return "localhost";
	}
	
	struct hostent *h = gethostbyname(name);
	
	if (h == NULL) {
		return "localhost";
	}
	
	return h->h_name;
}

unsigned long get_src_ip4_address_for_dst(unsigned long dst_ip4) {
	string log_msg;
	struct sockaddr_in addr;
	int ret;
	
	// Create UDP socket
	int sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sd < 0) {
		string err = get_error_str(errno);
		log_msg = "Cannot create socket: ";
		log_msg += err;
		log_file->write_report(log_msg, "::get_src_ip4_address_for_dst",
			LOG_NORMAL, LOG_CRITICAL);
		return 0;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(dst_ip4);
	addr.sin_port = htons(5060);

	// Connect to the destination. Note that no network traffic
	// is sent out as this is a UDP socket. The routing engine
	// will set the correct source address however.
	ret = connect(sd, (struct sockaddr *)&addr, sizeof(addr));
	if (ret < 0) {
		string err = get_error_str(errno);
		log_msg = "Cannot connect socket: ";
		log_msg += err;
		log_file->write_report(log_msg, "::get_src_ip4_address_for_dst",
			LOG_NORMAL, LOG_CRITICAL);
			
		close(sd);
		return 0;
	}

	// Get source address of socket
	memset(&addr, 0, sizeof(addr));
	socklen_t len_addr = sizeof(addr);
	ret = getsockname(sd, (struct sockaddr *)&addr, &len_addr);
	if (ret < 0) {
		string err = get_error_str(errno);
		log_msg = "Cannot get sockname: ";
		log_msg += err;
		log_file->write_report(log_msg, "::get_src_ip4_address_for_dst",
			LOG_NORMAL, LOG_CRITICAL);
			
		close(sd);
		return 0;
	}

	close(sd);
	return ntohl(addr.sin_addr.s_addr);
}

string display_and_url2str(const string &display, const string &url) {
	string s;
	
	if (!display.empty()) {
		if (must_quote(display)) s += '"';
		s += display;
		if (must_quote(display)) s += '"';
		s += " <";
	}
	
	s += url;
	
	if (!display.empty()) s += '>';
	
	return s;
}

// t_ip_port

t_ip_port::t_ip_port(unsigned long _ipaddr, unsigned short _port) :
	transport("udp"), ipaddr(_ipaddr), port(_port) {}
	
t_ip_port::t_ip_port(const string &proto, unsigned long _ipaddr, unsigned short _port) :
	transport(proto), ipaddr(_ipaddr), port(_port) {}

void t_ip_port::clear(void) {
	transport.clear();
	ipaddr = 0;
	port = 0;
}

bool t_ip_port::is_null(void) const {
	return (ipaddr == 0 && port == 0);
}

bool t_ip_port::operator==(const t_ip_port &other) const {
	return (transport == other.transport &&
	        ipaddr == other.ipaddr &&
	        port == other.port);
}

bool t_ip_port::operator!=(const t_ip_port &other) const {
	return !operator==(other);
}

string t_ip_port::tostring(void) const {
	string s;
	s = transport;
	s += ":";
	s += h_ip2str(ipaddr);
	s += ":";
	s += int2str(port);
	
	return s;
}

// Private

void t_url::construct_user_url(const string &s) {
	string::size_type i;
	string r;

	// Determine user/password (both are optional)
	i = s.find('@');
	if (i != string::npos) {
		if (i == 0 || i == s.size()-1) return;
		string userpass = s.substr(0, i);
		r = s.substr(i+1);
		i = userpass.find(':');
		if (i != string::npos) {
			if (i == 0 || i == userpass.size()-1) return;
			user = unescape_hex(userpass.substr(0, i));
			password = unescape_hex(userpass.substr(i+1));
			
			if (escape_passwd_value(password) != password) {
				modified = true;
			}
		} else {
			user = unescape_hex(userpass);
		}
		
		// Set modified flag if user contains reserved symbols.
		// This enforces escaping these symbols when the url gets
		// encoded.
		if (escape_user_value(user) != user) {
			modified = true;
		}
	} else {
		r = s;
	}

	// Determine host/port
	string hostport;

	i = r.find_first_of(";?");
	if (i != string::npos) {
		hostport = r.substr(0, i);
		if (!parse_params_headers(r.substr(i))) return;
	} else {
		hostport = r;
	}
	
	if (hostport.empty()) return;
	
	if (hostport.at(0) == '[') {
		// Host contains an IPv6 reference
		i = hostport.find(']');
		if (i == string::npos) return;
		// TODO: check format of an IPv6 address
		host = hostport.substr(0, i+1);
		if (i < hostport.size()-1) {
			if (hostport.at(i+1) != ':') return; // wrong port separator
			if (i+1 == hostport.size()-1) return; // port missing
			unsigned long p = atol(hostport.substr(i+2).c_str());
			if (p > 65535) return; // illegal port value
			port = (unsigned short)p;
		}
	} else {
		// Host contains a host name or IPv4 address
		i = hostport.find(':');
		if (i != string::npos) {
			if (i == 0 || i == hostport.size()-1) return;
			host = hostport.substr(0, i);
			unsigned long p = atol(hostport.substr(i+1).c_str());
			if (p > 65535) return; // illegal port value
			port = (unsigned short)p;
		} else {
			host = hostport;
		}
	}

	user_url = true;
	valid = true;
}


void t_url::construct_machine_url(const string &s) {
	string::size_type i;

	// Determine host
	string hostport;
	i = s.find_first_of("/?;");
	if ( i != string::npos) {
		hostport = s.substr(0, i);
		if (!parse_params_headers(s.substr(i))) return;
	} else {
		hostport = s;
	}

	if (hostport.empty()) return;
	
	if (hostport.at(0) == '[') {
		// Host contains an IPv6 reference
		i = hostport.find(']');
		if (i == string::npos) return;
		// TODO: check format of an IPv6 address
		host = hostport.substr(0, i+1);
		if (i < hostport.size()-1) {
			if (hostport.at(i+1) != ':') return; // wrong port separator
			if (i+1 == hostport.size()-1) return; // port missing
			unsigned long p = atol(hostport.substr(i+2).c_str());
			if (p > 65535) return; // illegal port value
			port = (unsigned short)p;
		}
	} else {
		// Host contains a host name or IPv4 address
		i = hostport.find(':');
		if (i != string::npos) {
			if (i == 0 || i == hostport.size()-1) return;
			host = hostport.substr(0, i);
			unsigned long p = atol(hostport.substr(i+1).c_str());
			if (p > 65535) return; // illegal port value
			port = (unsigned short)p;
		} else {
			host = hostport;
		}
	}

	user_url = false;
	valid = true;
}

bool t_url::parse_params_headers(const string &s) {
	string param_str = "";

	// Find start of headers
	// Note: parameters will not contain / or ?-symbol
	string::size_type header_start = s.find_first_of("/?");
	if (header_start != string::npos) {
		headers = s.substr(header_start + 1);

		if (s[0] == ';') {
			// The first symbol of the parameter list is ;
			// Remove this.
			param_str = s.substr(1, header_start - 1);
		}
	} else {
		// There are no headers
		// The first symbol of the parameter list is ;
		// Remove this.
		param_str = s.substr(1);
	}

	if (param_str == "") return true;

	// Create a list of single parameters. Parameters are
	// seperated by semi-colons.
	// Note: parameters will not contain a semi-colon in the
	//       name or value.
	vector<string> param_lst = split(param_str, ';');

	// Parse the parameters
	for (vector<string>::iterator i = param_lst.begin();
	     i != param_lst.end(); i++)
	{
		string pname;
		string pvalue;

		vector<string> param = split(*i, '=');
		if (param.size() > 2) return false;

		pname = tolower(unescape_hex(trim(param.front())));
		if (param.size() == 2) {
			pvalue = tolower(unescape_hex(trim(param.back())));
		}

		if (pname == "transport") {
			transport = pvalue;
		} else if (pname == "maddr") {
			maddr = pvalue;
		} else if (pname == "lr") {
			lr = true;
		} else if (pname == "user") {
			user_param = pvalue;
		} else if (pname == "method") {
			method = pvalue;
		} else if (pname == "ttl") {
			ttl = atoi(pvalue.c_str());
		} else {
			other_params += ';';
			other_params += *i;
		}
	}

	return true;
}

// Public static

string t_url::escape_user_value(const string &user_value) {
	// RFC 3261
	// user             =  1*( unreserved / escaped / user-unreserved )
	// user-unreserved  =  "&" / "=" / "+" / "$" / "," / ";" / "?" / "/"
	// unreserved       =  alphanum / mark
	// mark             =  "-" / "_" / "." / "!" / "~" / "*" / "'" / "(" / ")"

	return escape_hex(user_value,
		"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYX0123456789"\
		"-_.!~*'()&=+$,;?/");
}

string t_url::escape_passwd_value(const string &passwd_value) {
	// RFC 3261
	// password         = *( unreserved / escaped / "&" / "=" / "+" / "$" / "," )
	// unreserved       =  alphanum / mark
	// mark             =  "-" / "_" / "." / "!" / "~" / "*" / "'" / "(" / ")"
	
	return escape_hex(passwd_value,
		"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYX0123456789"\
		"-_.!~*'()&=+$,");
}

string t_url::escape_hnv(const string &hnv) {
	// RFC 3261
	// hname           =  1*( hnv-unreserved / unreserved / escaped )
	// hvalue          =  *( hnv-unreserved / unreserved / escaped )
	// hnv-unreserved  =  "[" / "]" / "/" / "?" / ":" / "+" / "$"
	// unreserved       =  alphanum / mark
	// mark             =  "-" / "_" / "." / "!" / "~" / "*" / "'" / "(" / ")"
	
	return escape_hex(hnv,
		"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYX0123456789"\
		"-_.!~*'()[]/?:+$");
}

// Public

t_url::t_url(void) {
	modified = false;
	valid = false;
	port = 0;
	lr = false;
	ttl = 0;
}

t_url::t_url(const string &s) {
	set_url(s);
}

t_url t_url::copy_without_headers(void) const {
	t_url u(*this);
	u.clear_headers();
	return u;
}

void t_url::set_url(const string &s) {
	string::size_type i;
	string r;

	modified = false;
	valid = false;
	scheme.clear();
	user.clear();
	password.clear();
	host.clear();
	port = 0;
	transport.clear();
	maddr.clear();
	lr = false;
	user_param.clear();
	method.clear();
	ttl = 0;
	other_params.clear();
	headers.clear();
	user_url = false;
	
	text_format = s;

	// Determine scheme. A scheme is mandatory. There should
	// be text following the scheme.
	i = s.find(':');
	if (i == string::npos || i == 0 || i == s.size()-1) return;
	scheme = tolower(s.substr(0, i));
	r = s.substr(i+1);

	if (r[0] == '/') {
		if (r.size() == 1) return;
		if (r[1] != '/') return;
		construct_machine_url(r.substr(2));
	} else {
		construct_user_url(r);
	}
}

string t_url::get_scheme(void) const {
	return scheme;
}

string t_url::get_user(void) const {
	return user;
}

string t_url::get_password(void) const {
	return password;
}

string t_url::get_host(void) const {
	return host;
}

int t_url::get_nport(void) const {
	return htons(get_hport());
}

int t_url::get_hport(void) const {
	if (port != 0) return port;
	return get_default_port(scheme);
}

int t_url::get_port(void) const {
	return port;
}

unsigned long t_url::get_n_ip(void) const {
	struct hostent *h;

	// TODO: handle multiple A RR's
	
	if (scheme == "tel") return 0;
	
	h = gethostbyname(host.c_str());
	if (h == NULL) return 0;
	return *((unsigned long *)h->h_addr);
}

unsigned long t_url::get_h_ip(void) const {
	if (scheme == "tel") return 0;
	return gethostbyname(host);
}

list<unsigned long> t_url::get_h_ip_all(void) const {
	if (scheme == "tel") return list<unsigned long>();
	return gethostbyname_all(host);
}

string t_url::get_ip(void) const {
	struct hostent *h;

	// TODO: handle multiple A RR's
	
	if (scheme == "tel") return 0;
	
	h = gethostbyname(host.c_str());
	if (h == NULL) return "";
	return inet_ntoa(*((struct in_addr *)h->h_addr));
}

list<t_ip_port> t_url::get_h_ip_srv(const string &transport) const {
	list<t_ip_port> ip_list;
	list<t_dns_result> srv_list;
	list<unsigned long> ipaddr_list;
	
	if (scheme == "tel") return list<t_ip_port>();
		
	// RFC 3263 4.2
	// Only do an SRV lookup if host is a hostname and no port is specified.
	if (!is_ipaddr(host) && port == 0) {
		int ret = insrv_lookup(scheme.c_str(), transport.c_str(), 
				host.c_str(), srv_list);
		
		if (ret >= 0 && !srv_list.empty()) {
			// SRV RR's found
			for (list<t_dns_result>::iterator i = srv_list.begin();
			i != srv_list.end(); i++)
			{
				// Get A RR's
				t_ip_port ip_port;
				ipaddr_list = gethostbyname_all(i->hostname);
				for (list<unsigned long>::iterator j = ipaddr_list.begin();
				     j != ipaddr_list.end(); j++)
				{
					ip_list.push_back(t_ip_port(transport, *j, i->port));
				}
			}
			
			return ip_list;
		}
	}
	
	// No SRV RR's found, do an A RR lookup
	t_ip_port ip_port;
	ipaddr_list = get_h_ip_all();
	for (list<unsigned long>::iterator j = ipaddr_list.begin();
		j != ipaddr_list.end(); j++)
	{
		ip_list.push_back(t_ip_port(transport, *j, get_hport()));
	}
	
	return ip_list;
}

string t_url::get_transport(void) const {
	return transport;
}

string t_url::get_maddr(void) const {
	return maddr;
}

bool t_url::get_lr(void) const {
	return lr;
}

string t_url::get_user_param(void) const {
	return user_param;
}

string t_url::get_method(void) const {
	return method;
}

int t_url::get_ttl(void) const {
	return ttl;
}

string t_url::get_other_params(void) const {
	return other_params;
}

string t_url::get_headers(void) const {
	return headers;
}

void t_url::set_user(const string &u) {
	modified = true;
	user = u;
}

void t_url::set_host(const string &h) {
	modified = true;
	host = h;
}

void t_url::add_header(const t_header &hdr) {
	if (!hdr.is_populated()) return;
	
	modified = true;
	
	if (!headers.empty()) headers += ';';
	headers += escape_hnv(hdr.get_name());
	headers += '=';
	headers += escape_hnv(hdr.get_value());
}

void t_url::clear_headers(void) {
	if (headers.empty()) {
		// No headers to clear
		return;
	}
	
	modified = true;
	headers.clear();
}

bool t_url::is_valid(void) const {
	return valid;
}

// RCF 3261 19.1.4
bool t_url::sip_match(const t_url &u) const {
	if (!u.is_valid() || !is_valid()) return false;

	// Compare schemes
	if (scheme != "sip" && scheme != "sips") return false;
	if (u.get_scheme() != "sip" && u.get_scheme() != "sips") {
		return false;
	}
	if (scheme != u.get_scheme()) return false;

	// Compare user info
	if (user != u.get_user()) return false;
	if (password != u.get_password()) return false;

	// Compare host/port
	if (cmp_nocase(host, u.get_host()) != 0) return false;
	if (port != u.get_port()) return false;

	// Compare parameters
	if (transport != "" && u.get_transport() != "" &&
	    cmp_nocase(transport, u.get_transport()) != 0)
	{
		return false;
	}

	if (maddr != u.get_maddr()) return false;
	if (cmp_nocase(user_param, u.get_user_param()) != 0) return false;
	if (cmp_nocase(method, u.get_method()) != 0) return false;
	if (ttl != u.get_ttl()) return false;

	// TODO: compare other params and headers

	return true;
}

bool t_url::operator==(const t_url &u) const {
	return sip_match(u);
}

bool t_url::operator!=(const t_url &u) const {
	return !sip_match(u);
}

bool t_url::user_host_match(const t_url &u, bool looks_like_phone, 
		const string &special_symbols) const
{
	string u1 = get_user();
	string u2 = u.get_user();
	
	// For tel-URIs the phone number is in the host part.
	if (scheme == "tel") u1 = get_host();
	if (u.scheme == "tel") u2 = u.get_host();
	
	bool u1_is_phone = false;
	bool u2_is_phone = false;
	
	if (is_phone(looks_like_phone, special_symbols)) {
		u1 = remove_symbols(u1, special_symbols);
		u1_is_phone = true;
	}
	
	if (u.is_phone(looks_like_phone, special_symbols)) {
		u2 = remove_symbols(u2, special_symbols);
		u2_is_phone = true;
	}
	
	if (u1 != u2) return false;
	
	if (u1_is_phone && u2_is_phone) {
		// Both URLs are phone numbers. Do not compare
		// the host-part.
		return true;
	}
	
	return (get_host() == u.get_host());
}

bool t_url::user_looks_like_phone(const string &special_symbols) const {
	return looks_like_phone(user, special_symbols);
}

bool t_url::is_phone(bool looks_like_phone, const string &special_symbols) const {
	if (scheme == "tel") return true;

	// RFC 3261 19.1.1
	if (user_param == "phone") return true;
	return (looks_like_phone && user_looks_like_phone(special_symbols));
}

string t_url::encode(void) const {
	if (modified) {
		if (!user_url) {
			// TODO: machine URL's are currently not used
			return text_format;
		}
	
		string s;
		
		s = scheme;
		s += ':';
		
		if (!user.empty()) {
			s += escape_user_value(user);
		
			if (!password.empty()) {
				s += ':';
				s += escape_passwd_value(password);
			}
			
			s += '@';
		}
		
		s += host;
		
		if (port > 0) {
			s += ':';
			s += int2str(port);
		}
		
		if (!transport.empty()) {
			s += ";transport=";
			s += transport;
		}
	
		if (!maddr.empty()) {
			s += ";maddr=";
			s += maddr;
		}
		
		if (lr) {
			s += ";lr";
		}
		
		if (!user_param.empty()) {
			s += ";user=";
			s += user_param;
		}
		
		if (!method.empty()) {
			s += ";method=";
			s += method;
		}
		
		if (ttl > 0) {
			s += ";ttl=";
			s += int2str(ttl);
		}
		
		if (!other_params.empty()) {
			s += other_params;
		}
		
		if (!headers.empty()) {
			s += "?";
			s += headers;
		}
		
		return s;
	} else {
		return text_format;
	}
}

string t_url::encode_noscheme(void) const {
	string s = encode();
	string::size_type i = s.find(':');

	if (i != string::npos && i < s.size()) {
		s = s.substr(i + 1);
	}

	return s;
}

string t_url::encode_no_params_hdrs(bool escape) const {
	if (!user_url) {
		// TODO: machine URL's are currently not used
		return text_format;
	}

	string s;
	
	s = scheme;
	s += ':';
	
	if (!user.empty()) {
		if (escape) {
			s += escape_user_value(user);
		} else {
			s += user;
		}
		
		if (!password.empty()) {
			s += ':';
			if (escape) {
				s += escape_passwd_value(password);
			} else {
				s += password;
			}
		}
		
		s += '@';
	}
	
	s += host;
	
	if (port > 0) {
		s += ':';
		s += int2str(port);
	}
	
	return s;
}

void t_url::apply_conversion_rules(t_user *user_config) {
	if (scheme == "tel") {
		host = user_config->convert_number(host);
	} else {
		// Convert user part for all other schemes
		user = user_config->convert_number(user);
	}
	
	modified = true;
}


t_display_url::t_display_url() {}

t_display_url::t_display_url(const t_url &_url, const string &_display) :
	url(_url), display(_display) {}
	
bool t_display_url::is_valid() {
	return url.is_valid();
}
	
string t_display_url::encode(void) const {
	string s;
	
	if (!display.empty()) {
		if (must_quote(display)) s += '"';
		s += display;
		if (must_quote(display)) s += '"';
		s += " <";
	}
	
	s += url.encode();
	
	if (!display.empty()) s += '>';
	
	return s;
}
