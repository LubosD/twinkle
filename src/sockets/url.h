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

#ifndef _H_URL
#define _H_URL

#include <list>
#include <string>
#include "parser/header.h"

/** @name Forward declarations */
//@{
class t_user;
//@}

using namespace std;

class t_ip_port {
public:
	string			transport;
	unsigned long		ipaddr;
	unsigned short		port;
	
	t_ip_port() : transport("udp") {};
	t_ip_port(unsigned long _ipaddr, unsigned short _port);
	t_ip_port(const string &proto, unsigned long _ipaddr, unsigned short _port);
	
	void clear(void);
	bool is_null(void) const;
	bool operator==(const t_ip_port &other) const;
	bool operator!=(const t_ip_port &other) const;
	string tostring(void) const;
};

// Return the default port for a protocol (host order)
unsigned short get_default_port(const string &protocol);

// Return the first IP address of host name.
// Return 0 if no IP address can be found.
unsigned long gethostbyname(const string &name);

// Return all IP address of host name
list<unsigned long> gethostbyname_all(const string &name);

/**
 * Get local host name.
 * @return Local host name.
 */
string get_local_hostname(void);

/**
 * Get the source IP address that will be used for sending
 * a packet to a certain destination.
 * @param dst_ip4 [in] The destination IPv4 address.
 * @return The source IPv4 address.
 * @return 0 if the source address cannot be determined.
 */
unsigned long get_src_ip4_address_for_dst(unsigned long dst_ip4);

class t_url {
private:
	/**
	 * A t_url object is created with a string represnetation of
	 * the URL. The encode method just returns this string.
	 * If one of the components of the t_url object is modified
	 * however, then encode will build a new string representation.
	 * The modified flag indicates if the object was modified after
	 * construction.
	 */
	bool		modified;

	/** URL scheme. */
	string		scheme;
	
	/** The user part of a URL. For a tel URL this is empty. */
	string		user;
	
	/** The user password. */
	string		password;
	
	/** 
	 * The host part of a URL. For a tel URL, it contains the part before
	 * the first semi-colon.
	 */
	string		host;
	
	unsigned short	port; 		// host order

	// parameters
	string		transport;
	string		maddr;
	bool		lr;
	string		user_param;
	string		method;
	int		ttl;
	string		other_params;	// unparsed other parameters
					// starting with a semi-colon

	// headers
	string		headers;	// unparsed headers

	bool		user_url;	// true -> user url
					// false -> machine
	bool		valid;		// indicates if the url is valid

	string		text_format;	// url in string format

	void construct_user_url(const string &s);    // eg sip:, mailto:
	void construct_machine_url(const string &s); // eg http:, ftp:

	// Parse uri parameters and headers. Returns false if parsing
	// fails.
	bool parse_params_headers(const string &s);
	
public:
	// Escape reserved symbols in a user value
	static string escape_user_value(const string &user_value);
	
	// Escape reserved symbols in a password value
	static string escape_passwd_value(const string &passwd_value);
	
	// Escape reserved symbols in a header name or value
	static string escape_hnv(const string &hnv);

public:
	t_url();
	t_url(const string &s);
	
	// Return a copy of the URI without headers.
	// If the URI does not contain any headers, then the copy is
	// identical to the URI.
	t_url copy_without_headers(void) const;

	void set_url(const string &s);

	// Returns "" or 0 if item cannot be found
	string get_scheme(void) const;
	string get_user(void) const;
	string get_password(void) const;
	string get_host(void) const;

	// The following methods will return the default port if
	// no port is present in the url.
	int get_nport(void) const; // Get port in network order.
	int get_hport(void) const; // get port in host order.

	// The following method returns 0 if no port is present
	// in the url.
	int get_port(void) const;

	// ip address network order. Return 0 if address not found
	// DNS A RR lookup
	unsigned long get_n_ip(void) const;

	// ip address host order. Return 0 if address not found
	// DNS A RR lookup
	unsigned long get_h_ip(void) const;
	list<unsigned long> get_h_ip_all(void) const;

	// DNS A RR lookup
	string get_ip(void) const; // ip address as string
	
	// Get list op IP address/ports in host order.
	// First do DNS SRV lookup. If no SRV RR's are found, then
	// do a DNS A RR lookup.
	// transport = the transport protocol for the service
	list<t_ip_port> get_h_ip_srv(const string &transport) const;
	
	/** @name Getters */
	//@{
	string get_transport(void) const;
	string get_maddr(void) const;
	bool get_lr(void) const;
	string get_user_param(void) const;
	string get_method(void) const;
	int get_ttl(void) const;
	string get_other_params(void) const;
	string get_headers(void) const;
	//@}
	
	/** @name Setters */
	//@{
	void set_user(const string &u);
	void set_host(const string &h);
	//@}
	
	/**
	 * Add a header to the URI.
	 * The encoded header will be concatenated to the headers field.
	 * @param hdr [in] Header to be added.
	 */
	void add_header(const t_header &hdr);
	
	/** Remove headers from the URI. */
	void clear_headers(void);

	/**
	 * Check if the URI is valid.
	 * @return True if valid, otherwise false.
	 */
	bool is_valid(void) const;

	// Check if 2 sip or sips url's are equivalent
	bool sip_match(const t_url &u) const;
	bool operator==(const t_url &u) const;
	bool operator!=(const t_url &u) const;
	
	/**
	 * Check if the user-host part of 2 url's are equal.
	 * If the user-part is a phone number, then only compare
	 * the user parts. If the url is a tel-url then the host
	 * contains a telephone number for comparison.
	 * @param u [in] Other URL to compare with.
	 * @param looks_like_phone [in] Flag to indicate is a SIP URL
	 *        that looks like a phone number must be treated as such.
	 * @param special_symbols [in] Interpuction symbols in a phone number.
	 * @return true if the URLs match, false otherwise.
	 */
	bool user_host_match(const t_url &u, bool looks_like_phone, 
		const string &special_symbols) const;

	// Return true if the user part looks like a phone number, i.e.
	// consists of digits, *, # and special symbols
	bool user_looks_like_phone(const string &special_symbols) const;

	// Return true if the URI indicates a phone number, i.e.
	// - the user=phone parameter is present
	// or
	// - if looks_like_phone == true and the user part looks like
	//   a phone number
	bool is_phone(bool looks_like_phone, const string &special_symbols) const;

	// Return string encoding of url
	string encode(void) const;

	// Return string encoding of url without scheme information
	string encode_noscheme(void) const;
	
	// Return string encoding of url without parameters/headers
	string encode_no_params_hdrs(bool escape = true) const;
	
	/**
	 * Apply number conversion rules to modify the URL.
	 * @param user_config [in] The user profile having the conversion
	 *        rules to apply.
	 */
	void apply_conversion_rules(t_user *user_config);
};

// Display name and url combined

class t_display_url {
public:
	t_url		url;
	string		display;
	
	t_display_url();
	t_display_url(const t_url &_url, const string &_display);
	
	bool is_valid();
	string encode(void) const;
};

#endif
