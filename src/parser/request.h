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

// Request

#ifndef _REQUEST_H
#define _REQUEST_H

#include <string>
#include "response.h"
#include "sip_message.h"
#include "sockets/url.h"
#include "user.h"

// Forward declaration
class t_user;

using namespace std;

class t_request : public t_sip_message {
private:
	/**
	 * A DNS lookup on the request URI (or outbound proxy) might resolve 
	 * into multiple destinations. @ref get_destination() will return the first 
	 * destination. All destinations are stored here.
	 * @ref next_destination() will remove the first destination of this
	 * list.
	 */
	list<t_ip_port>		destinations;
	
	/**
	 * Indicates if the destination specified a transport, i.e. via the
	 * transport parameter in a URI.
	 */
	bool			transport_specified;
	
	/**
	 * Add destinations for a given URI based on transport settings.
	 * @param user_profile [in] User profile
	 * @param dst_uri [in] The URI to resolve.
	 */
	void add_destinations(const t_user &user_profile, const t_url &dst_uri);
	
	/**
	 * Calculate credentials based on the challenge.
	 * @param chlg [in] The challenge
	 * @param user_config [in] User configuration for user to be authorized.
	 * @param username [in] User authentication name
	 * @param passwd [in] Authentication password.
	 * @param nc [in] Nonce count
	 * @param cnonce [in] Client nonce
	 * @param cr [out] Credentials on succesful return.
	 * @param fail_reason [out] Failure reason on failure return.
	 * @return false, if authorization fails.
	 * @return true, if authorization succeeded
	 */
	bool authorize(const t_challenge &chlg, t_user *user_config, 
		       const string &username, const string &passwd, unsigned long nc,
		       const string &cnonce, t_credentials &cr,
		       string &fail_reason) const;

	/**
	 * Calculate MD5 response based on the challenge.
	 * @param chlg [in] The challenge
	 * @param username [in] User authentication name
	 * @param passwd [in] Authentication password.
	 * @param nc [in] Nonce count
	 * @param cnonce [in] Client nonce
	 * @param qop [in] Quality of protection
	 * @param resp [out] Response on succesful return.
	 * @param fail_reason [out] Failure reason on failure return.
	 * @return false, if authorization fails.
	 * @return true, if authorization succeeded
	 */
	bool authorize_md5(const t_digest_challenge &dchlg,
				const string &username, const string &passwd, unsigned long nc,
				const string &cnonce, const string &qop, string &resp, 
				string &fail_reason) const;

	/**
	 * Calculate AKAv1-MD5 response based on the challenge.
	 * @param chlg [in] The challenge
	 * @param username [in] User authentication name
	 * @param passwd [in] Authentication password.
	 * @param op [in] Operator variant key
	 * @param amf [in] Authentication method field
	 * @param nc [in] Nonce count
	 * @param cnonce [in] Client nonce
	 * @param qop [in] Quality of protection
	 * @param resp [out] Response on succesful return.
	 * @param fail_reason [out] Failure reason on failure return.
	 * @return false, if authorization fails.
	 * @return true, if authorization succeeded
	 */
	bool authorize_akav1_md5(const t_digest_challenge &dchlg,
				const string &username, const string &passwd, 
				uint8 *op, uint8 *amf,
				unsigned long nc,
				const string &cnonce, const string &qop, string &resp, 
				string &fail_reason) const;


public:
	t_url		uri;
	t_method	method;
	string		unknown_method; // set if method is UNKNOWN

	t_request();
	t_request(const t_request &r);
	t_request(const t_method m);

	t_msg_type get_type(void) const { return MSG_REQUEST; }
	void set_method(const string &s);
	string encode(bool add_content_length = true);
	list<string> encode_env(void);
	t_sip_message *copy(void) const;
	
	/**
	 * Set the Request-URI and the Route header.
	 * This is done according to the procedures of RFC 3261 12.2.1.1
	 * @param target_uri [in] The URI of the destination for this request.
	 * @param route_set [in] The route set for this request (may be empty).
	 */
	void set_route(const t_url &target_uri, const list<t_route> &route_set);

	// Create a response with response code based on the
	// request. The response is created following the general
	// rules in RFC 3261 8.2.6.2.
	// The to-hdr is simply copied from the request to the
	// response.
	// If the to-tag is missing, then
	// a to-tag will be generated and added to the to-header
	// of the response.
	// A specific reason may be added to the status code.
	t_response *create_response(int code, string reason = "") const;

	bool is_valid(bool &fatal, string &reason) const;
	
	// Calculate the set of possible destinations for this request.
	void calc_destinations(const t_user &user_profile);

	// Get destination to send this request to.
	void get_destination(t_ip_port &ip_port, const t_user &user_profile);
	void get_current_destination(t_ip_port &ip_port);
		
	// Move to next destination. This method should only be called after
	// calc_destination() was called.
	// Returns true if there is a next destination, otherwise returns false.
	bool next_destination(void);
	
	// Set a single destination to send this request to.
	void set_destination(const t_ip_port &ip_port);

	/** 
	 * Create WWW authorization credentials based on the challenge.
	 * @param chlg [in] The challenge
	 * @param user_config [in] User configuration for user to be authorized.
	 * @param username [in] User authentication name
	 * @param passwd [in] Authentication password.
	 * @param nc [in] Nonce count
	 * @param cnonce [in] Client nonce
	 * @param cr [out] Credentials on succesful return.
	 * @param fail_reason [out] Failure reason on failure return.
	 * @return false, if challenge is not supported.
	 * @return true, if authorization succeeded
	 */
	bool www_authorize(const t_challenge &chlg, t_user *user_config,
	       const string &username, const string &passwd, unsigned long nc,
	       const string &cnonce, t_credentials &cr, string &fail_reason);

	/** 
	 * Create proxy authorization credentials based on the challenge.
	 * @param chlg [in] The challenge
	 * @param user_config [in] User configuration for user to be authorized.
	 * @param username [in] User authentication name
	 * @param passwd [in] Authentication password.
	 * @param nc [in] Nonce count
	 * @param cnonce [in] Client nonce
	 * @param cr [out] Credentials on succesful return.
	 * @param fail_reason [out] Failure reason on failure return.
	 * @return false, if challenge is not supported.
	 * @return true, if authorization succeeded
	 */
	bool proxy_authorize(const t_challenge &chlg, t_user *user_config,
	       const string &username, const string &passwd, unsigned long nc,
	       const string &cnonce, t_credentials &cr, string &fail_reason);
	       
	virtual void calc_local_ip(void);
	
	/**
	 * Check if the request is a registration request.
	 * @return True if the request is a registration request, otherwise false.
	 */
	bool is_registration_request(void) const;
	
	/**
	 * Check if the request is a de-registration request.
	 * @return True if the request is a de-registration request, otherwise false.
	 */
	bool is_de_registration_request(void) const;
};

#endif
