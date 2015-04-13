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

#include "request.h"
#include "util.h"
#include "parse_ctrl.h"
#include "protocol.h"
#include "milenage.h"
#include "audits/memman.h"
#include <sstream>
#include <cc++/digest.h>

using namespace ost;

// AKAv1-MD5 algorithm specific helpers

#define B64_ENC_SZ(x) (4 * ((x + 2) / 3))
#define B64_DEC_SZ(x) (3 * ((x + 3) / 4))

int b64_enc(const u8 * src, u8 * dst, int len)
{
	static char tbl[64] = {
		'A','B','C','D','E','F','G','H',
		'I','J','K','L','M','N','O','P',
		'Q','R','S','T','U','V','W','X',
		'Y','Z','a','b','c','d','e','f',
		'g','h','i','j','k','l','m','n',
		'o','p','q','r','s','t','u','v',
		'w','x','y','z','0','1','2','3',
		'4','5','6','7','8','9','+','/'
	};
	u8 * dst0 = dst;
	int i, v, div = len / 3, mod = len % 3;

	for (i = 0; i < div * 3; i += 3) {
		
		v = (src[i+0] & 0xfc) >> 2;
		*(dst++) = tbl[v];

		v  = (src[i+0] & 0x03) << 4;
		v |= (src[i+1] & 0xf0) >> 4;
		*(dst++) = tbl[v];

		v  = (src[i+1] & 0x0f) << 2;
		v |= (src[i+2] & 0xc0) >> 6;
		*(dst++) = tbl[v];
		
		v = src[i+2] & 0x3f;
		*(dst++) = tbl[v];
	}

	if (mod == 1) {
		v = (src[i+0] & 0xfc) >> 2;
		*(dst++) = tbl[v];
		
		v = (src[i+0] & 0x03) << 4;
		*(dst++) = tbl[v];

		*(dst++) = '=';
		*(dst++) = '=';
	} else if (mod == 2) {
		v = (src[i+0] & 0xfc) >> 2;
		*(dst++) = tbl[v];
		
		v  = (src[i+0] & 0x03) << 4;
		v |= (src[i+1] & 0xf0) >> 4;
		*(dst++) = tbl[v];

		v = (src[i+1] & 0x0f) << 2;
		*(dst++) = tbl[v];

		*(dst++) = '=';
	}
	
	return dst - dst0;
}

static int b64_val(u8 x)
{
	if (x >= 'A' && x <= 'Z')
		return x - 'A';
	else if (x >= 'a' && x <= 'z')
		return x - 'a' + 26;
	else if (x >= '0' && x <= '9')
		return x - '0' + 52;
	else if (x == '+')
		return 62;
	else if (x == '/')
		return 63;
	//else if (x == '=')
		return -1;
}

int b64_dec(const u8 * src, u8 * dst, int len)
{
	u8 * dst0 = dst;
	int i, x1, x2, x3, x4;

	if (len % 4)
		return 0;

	for (i=0; i+4 < len; i += 4) {
		x1 = b64_val(*(src++));
		x2 = b64_val(*(src++));
		x3 = b64_val(*(src++));
		x4 = b64_val(*(src++));
		
		*(dst++) = (x1 << 2) | ((x2 & 0x30) >> 4);
		*(dst++) = ((x2 & 0x0F) << 4) | ((x3 & 0x3C) >> 2);
		*(dst++) = ((x3 & 0x03) << 6) | (x4 & 0x3F);
	}
	
	if (len) {
		x1 = b64_val(*(src++));
		x2 = b64_val(*(src++));
		x3 = b64_val(*(src++));
		x4 = b64_val(*(src++));
		
		*(dst++) = (x1 << 2) | ((x2 & 0x30) >> 4);
		if (x3 != -1) {
			*(dst++) = ((x2 & 0x0F) << 4) | ((x3 & 0x3C) >> 2);
			if (x4 != -1)
				*(dst++) = ((x3 & 0x03) << 6) | (x4 & 0x3F);
		}
	}

	return dst - dst0;
}

#define HASH_HEX_LEN	32
#define HASH_LEN		16

// authentication with AKAv1-MD5 algorithm (RFC 3310)

bool t_request::authorize_akav1_md5(const t_digest_challenge &dchlg,
	const string &username, const string &passwd, uint8 *op, uint8 *amf,
	unsigned long nc,
	const string &cnonce, const string &qop, string &resp, 
	string &fail_reason) const
{
	u8 nonce64[B64_DEC_SZ(dchlg.nonce.size())];
	int len = b64_dec((const u8 *)dchlg.nonce.c_str(), nonce64, dchlg.nonce.size());
	u8 rnd[AKA_RANDLEN];
  	u8 sqnxoraka[AKA_SQNLEN];
	u8 sqn[AKA_SQNLEN];
	u8 k[AKA_KLEN];
	u8 res[AKA_RESLEN];
	u8 ck[AKA_CKLEN];
	u8 ik[AKA_IKLEN];
	u8 ak[AKA_AKLEN];
	int i;

	if (len < AKA_RANDLEN+AKA_AUTNLEN) {
		fail_reason = "nonce base64 data too short (need 32 bytes)";
		return false;
	}

	memset(rnd, 0, AKA_RANDLEN);
	memset(sqnxoraka, 0, AKA_SQNLEN);
	memset(k, 0, AKA_KLEN);

	memcpy(rnd, nonce64, AKA_RANDLEN);
	memcpy(sqnxoraka, nonce64 + AKA_RANDLEN, AKA_SQNLEN);
	memcpy(k, passwd.c_str(), passwd.size());

	f2345(k, rnd, res, ck, ik, ak, op);
	
	for (i=0; i < AKA_SQNLEN; i++)
    	sqn[i] = sqnxoraka[i] ^ ak[i];
	
	string res_str = string((char *)res, AKA_RESLEN);
	
	return authorize_md5(dchlg, username, res_str, nc, cnonce, qop, 
			resp, fail_reason);
}

// authentication with MD5 algorithm

bool t_request::authorize_md5(const t_digest_challenge &dchlg,
	const string &username, const string &passwd, unsigned long nc,
	const string &cnonce, const string &qop, string &resp, 
	string &fail_reason) const
{
	string A1, A2;
	// RFC 2617 3.2.2.2
	A1 = username + ":" + dchlg.realm + ":" + passwd;

	// RFC 2617 3.2.2.3
	if (cmp_nocase(qop, QOP_AUTH) == 0 || qop == "") {
		A2 = method2str(method, unknown_method) + ":" + uri.encode();
	} else {
		A2 = method2str(method, unknown_method) + ":" + uri.encode();
		A2 += ":";
		if (body) {
			MD5Digest MD5body;
			MD5body << body->encode();
			ostringstream os;
			os << MD5body;
			A2 += os.str();
		} else {
			MD5Digest MD5body;
			MD5body << "";
			ostringstream os;
			os << MD5body;
			A2 += os.str();
		}
	}

	// RFC 2716 3.2.2.1
	// Caculate digest
	MD5Digest MD5A1;
	MD5Digest MD5A2;
	ostringstream HA1;
	ostringstream HA2;

	MD5A1 << A1;
	MD5A2 << A2;
	HA1 << MD5A1;
	HA2 << MD5A2;

	string x;

	if (cmp_nocase(qop, QOP_AUTH) == 0 || cmp_nocase(qop, QOP_AUTH_INT) == 0) {
		x = HA1.str() + ":";
		x += dchlg.nonce + ":";
		x += int2str(nc, "%08x") + ":";
		x += cnonce + ":";
		x += qop + ":";
		x += HA2.str();
	} else {
		x = HA1.str() + ":";
		x += dchlg.nonce + ":";
		x += HA2.str();
	}

	MD5Digest digest;
	digest << x;
	ostringstream dresp;
	dresp << digest;

	resp = dresp.str();

	return true;
}

bool t_request::authorize(const t_challenge &chlg, t_user *user_config,
	const string &username, const string &passwd, unsigned long nc,
	const string &cnonce, t_credentials &cr, string &fail_reason) const
{
	// Only Digest authentication is supported
	if (cmp_nocase(chlg.auth_scheme, AUTH_DIGEST) != 0) {
		fail_reason = "Authentication scheme " + chlg.auth_scheme;
		fail_reason += " not supported.";
		return false;
	}

	const t_digest_challenge &dchlg = chlg.digest_challenge;
	
	string qop = "";

	// Determine QOP
	// If both auth and auth-int are supported by the server, then
	// choose auth to avoid problems with SIP ALGs. A SIP ALG rewrites
	// the body of a message, thereby breaking auth-int authentication.
	if (!dchlg.qop_options.empty()) {
		const list<string>::const_iterator i = find(
			dchlg.qop_options.begin(), dchlg.qop_options.end(),
			QOP_AUTH_INT);
		const list<string>::const_iterator j = find(
			dchlg.qop_options.begin(), dchlg.qop_options.end(),
			QOP_AUTH);
		if (j != dchlg.qop_options.end())
			qop = QOP_AUTH;
		else {
			if (i != dchlg.qop_options.end())
				qop = QOP_AUTH_INT;
			else {
				fail_reason = "Non of the qop values are supported.";
				return false;
			}
		}
	}

	bool ret = false;
	string resp;

	if (cmp_nocase(dchlg.algorithm, ALG_MD5) == 0) {
		ret = authorize_md5(dchlg, username, passwd, nc, cnonce, 
				qop, resp, fail_reason);
	} else if (cmp_nocase(dchlg.algorithm, ALG_AKAV1_MD5) == 0) {
		uint8 aka_op[AKA_OPLEN];
		uint8 aka_amf[AKA_AMFLEN];
		user_config->get_auth_aka_op(aka_op);
		user_config->get_auth_aka_amf(aka_amf);
		
		ret = authorize_akav1_md5(dchlg, username, passwd, aka_op, aka_amf, nc, cnonce, 
				qop, resp, fail_reason);
	} else {
		fail_reason = "Authentication algorithm " + dchlg.algorithm;
		fail_reason += " not supported.";
		return false;
	}
	
	if (!ret) return false;
	
	// Create credentials
	cr.auth_scheme = AUTH_DIGEST;
	t_digest_response &dr = cr.digest_response;

	dr.dresponse = resp;
	dr.username = username;
	dr.realm = dchlg.realm;
	dr.nonce = dchlg.nonce;
	dr.digest_uri = uri;
	dr.algorithm = dchlg.algorithm;
	dr.opaque = dchlg.opaque;

	// RFC 2617 3.2.2
	if (qop != "") {
		dr.message_qop = qop;
		dr.cnonce = cnonce;
		dr.nonce_count = nc;
	}

	return true;
}

t_request::t_request() : t_sip_message(),
	transport_specified(false),
	method(METHOD_UNKNOWN)
{
}

t_request::t_request(const t_request &r) : t_sip_message(r),
		destinations(r.destinations),
		transport_specified(r.transport_specified),
		uri(r.uri),
		method(r.method),
		unknown_method(r.unknown_method)
{
}

t_request::t_request(const t_method m) : t_sip_message() {
	method = m;
}

void t_request::set_method(const string &s) {
	method = str2method(s);
	if (method == METHOD_UNKNOWN) {
		unknown_method = s;
	}
}

string t_request::encode(bool add_content_length) {
	string s;

	s = method2str(method, unknown_method) + ' ' + uri.encode();
	s += " SIP/";
	s += version;
	s += CRLF;
	s += t_sip_message::encode(add_content_length);
	return s;
}

list<string> t_request::encode_env(void) {
	string s;
	list<string> l = t_sip_message::encode_env();
	
	s = "SIPREQUEST_METHOD=";
	s += method2str(method, unknown_method);
	l.push_back(s);
	
	s = "SIPREQUEST_URI=";
	s += uri.encode();
	l.push_back(s);
	
	return l;
}

t_sip_message *t_request::copy(void) const {
	t_sip_message *m =  new t_request(*this);
	MEMMAN_NEW(m);
	return m;
}

void t_request::set_route(const t_url &target_uri, const list<t_route> &route_set) {
	// RFC 3261 12.2.1.1
        if (route_set.empty()) {
                uri = target_uri;
        } else {
                if (route_set.front().uri.get_lr()) {
			// Loose routing
                        uri = target_uri;
                        for (list<t_route>::const_iterator i = route_set.begin();
                             i != route_set.end(); ++i)
                        {
                                hdr_route.add_route(*i);
                        }
                        hdr_route.route_to_first_route = true;
                } else {
			// Strict routing
                        uri = route_set.front().uri;
                        for (list<t_route>::const_iterator i = route_set.begin();
                             i != route_set.end(); ++i)
                        {
                                if (i != route_set.begin()) {
                                        hdr_route.add_route(*i);
                                }
                        }

                        // Add target uri to the route list
                        t_route route;
                        route.uri = target_uri;
                        hdr_route.add_route(route);
                }
        }
}

t_response *t_request::create_response(int code, string reason) const
{
	t_response *r;

	r = new t_response(code, reason);
	MEMMAN_NEW(r);
	
	r->src_ip_port_request = src_ip_port;
	
	r->hdr_from = hdr_from;
	r->hdr_call_id = hdr_call_id;
	r->hdr_cseq = hdr_cseq;
	r->hdr_via = hdr_via;
	r->hdr_to = hdr_to;

	// Create a to-tag if none was present in the request
	// NOTE: 100 Trying should not get a to-tag
	if (hdr_to.tag.size() == 0 && code != R_100_TRYING) {
		r->hdr_to.set_tag(NEW_TAG);
	}

	// Server
	SET_HDR_SERVER(r->hdr_server);

	return r;
}

bool t_request::is_valid(bool &fatal, string &reason) const {
	if (!t_sip_message::is_valid(fatal, reason)) return false;

	fatal = false;

	if (t_parser::check_max_forwards && !hdr_max_forwards.is_populated()) {
		reason = "Max-Forwards header missing";
		return false;
	}

	// RFC 3261 8.1.1.5
	// The CSeq method must match the request method.
	if (hdr_cseq.method != method) {
		reason = "CSeq method does not match request method";
		return false;
	}

	switch(method) {
	case INVITE:
		if (!hdr_contact.is_populated()) {
			reason = "Contact header missing";
			return false;
		}
		break;
	case PRACK:
		// RFC 3262 7.1
		if (!hdr_rack.is_populated()) {
			reason = "RAck header missing";
			return false;
		}
		break;
	case SUBSCRIBE:
		// RFC 3265 7.1, 7.2
		if (!hdr_contact.is_populated()) {
			reason = "Contact header missing";
			return false;
		}

		if (!hdr_event.is_populated()) {
			reason = "Event header missing";
			return false;
		}
		break;
	case NOTIFY:
		// RFC 3265 7.1, 7.2
		if (!hdr_contact.is_populated()) {
			reason = "Contact header missing";
			return false;
		}

		if (!hdr_event.is_populated()) {
			reason = "Event header missing";
			return false;
		}
		
		// RFC 3265 7.2
		// Subscription-State header is mandatory
		// As an exception Twinkle allows an unsollicited NOTIFY for MWI
		// without a Subscription-State header. Asterisk sends
		// unsollicited NOTIFY requests.
		if (!hdr_to.tag.empty() || 
		    hdr_event.event_type != SIP_EVENT_MSG_SUMMARY)
		{
			if (!hdr_subscription_state.is_populated()) {
				reason = "Subscription-State header missing";
			 	return false;
			 }
		}

		// The Subscription-State header is mandatory.
		// However, Asterisk uses an expired draft for sending
		// unsollicitied NOTIFY messages without a Subscription-State
		// header. As Asterisk is popular, Twinkle allows this.
		break;
	case REFER:
		// RFC 3515 2.4.1
		if (!hdr_refer_to.is_populated()) {
			reason = "Refer-To header missing";
			return false;
		}
		break;
	default:
		break;
	}
	
	if (hdr_replaces.is_populated()) {
		// RFC 3891 3
		if (method != INVITE) {
			reason = "Replaces header not allowed";
			return false;
		}
	}

	return true;
}

void t_request::add_destinations(const t_user &user_profile, const t_url &dst_uri) {
	t_url dest;
	if (dst_uri.get_scheme() == "tel")
	{
		// Send a tel-URI to the configure domain for the user.
		dest = "sip:" + user_profile.get_domain();
	}
	else
	{
		dest = dst_uri;
	}

	if ((user_profile.get_sip_transport() == SIP_TRANS_AUTO ||
	     user_profile.get_sip_transport() == SIP_TRANS_TCP)
	    &&
	    (dest.get_transport().empty() ||
	     cmp_nocase(dest.get_transport(), "tcp") == 0))
	{
		list<t_ip_port> l = dest.get_h_ip_srv("tcp");
		destinations.insert(destinations.end(), l.begin(), l.end());
	}
	
	// Add UDP destinations after TCP, so UDP will be used as a fallback
	// for large messages, when TCP fails. If the message is not large,
	// then the TCP destinations will be removed later.
	// NOTE: If a message is larger than 64K, it cannot be sent via UDP
	if ((user_profile.get_sip_transport() == SIP_TRANS_AUTO ||
	    user_profile.get_sip_transport() == SIP_TRANS_UDP)
	   &&
	   (dest.get_transport().empty() ||
	    cmp_nocase(dest.get_transport(), "udp") == 0)
	   &&
	   (get_encoded_size() < 65536))
	{
		list<t_ip_port> l = dest.get_h_ip_srv("udp");
		destinations.insert(destinations.end(), l.begin(), l.end());
	}
	
	transport_specified = !dest.get_transport().empty();
}

void t_request::calc_destinations(const t_user &user_profile) {
	destinations.clear();

	// Send a REGISTER to the registrar if provisioned.
	if (method == REGISTER && user_profile.get_use_registrar()) {
		add_destinations(user_profile, user_profile.get_registrar());
		return;
	}
	
	// Bypass the proxy for an out-of-dialog SUBSCRIBE if provisioned.
	if (method == SUBSCRIBE && hdr_to.tag.empty()) {
		if (hdr_event.event_type == SIP_EVENT_MSG_SUMMARY) {
			if (!user_profile.get_mwi_via_proxy()) {
				// Take Request-URI
				add_destinations(user_profile, uri);
				return;
			}
		}
	}

	if (!user_profile.get_use_outbound_proxy() ||
	    (hdr_to.tag != "" && !user_profile.get_all_requests_to_proxy())) {
		// A mid dialog request will go to the host in the contact
		// header (put in the request-URI in this request) or route list
		// specified in the final response of the invite (the Route-header in
		// this request).
		// Note that an ACK for a failed INVITE (3XX-6XX) will be
		// sent by the transaction layer to the ipaddr/port of the
		// INVITE.
		if (hdr_route.is_populated() && hdr_route.route_to_first_route) {
			// Take URI from first route-header
			t_url &u = hdr_route.route_list.front().uri;
			add_destinations(user_profile, u);
		} else {
			// Take Request-URI
			add_destinations(user_profile, uri);
		}
	}

	// Send request to outbound proxy if configured
	if (user_profile.get_use_outbound_proxy()) {
		if (user_profile.get_non_resolvable_to_proxy() && !destinations.empty())
		{
			// The destination has been resolved, so do not
			// use the outbound proxy in this case.
			return;
		}

		if (user_profile.get_all_requests_to_proxy() || hdr_to.tag == "") {
			// All requests should go to the proxy.
			// Override destination by the outbound proxy address.
			destinations.clear();
			add_destinations(user_profile, user_profile.get_outbound_proxy());
		}
	}
}

void t_request::get_destination(t_ip_port &ip_port, const t_user &user_profile) {
	if (destinations.empty()) calc_destinations(user_profile);
	
	// RFC 3261 18.1.1
	// If the message size is larger than 1300 then the message must be
	// sent over TCP.
	// If the destination URI indicated an explicit transport, then the
	// destination calculation picked the possible destinations already.
	// The size cannot influence this calculation anymore.
	if (user_profile.get_sip_transport() == SIP_TRANS_AUTO &&
	    !destinations.empty() &&
	    destinations.front().transport == "tcp" &&
	    get_encoded_size() <= user_profile.get_sip_transport_udp_threshold() &&
	    !transport_specified)
	{
		// The message can be sent over UDP. Remove all TCP destinations.
		while (!destinations.empty() && destinations.front().transport == "tcp") {
			destinations.pop_front();
		}
	}
	
	get_current_destination(ip_port);
}

void t_request::get_current_destination(t_ip_port &ip_port) {
	if (destinations.empty()) {
		// No destinations could be found.
		ip_port.transport = "udp";
		ip_port.ipaddr = 0;
		ip_port.port = 0;
	} else {
		// Return first destination
		ip_port = destinations.front();
	}
}

bool t_request::next_destination(void) {		
	if (destinations.size() <= 1) return false;
	
	// Remove current destination
	destinations.pop_front();
	return true;	
}

void t_request::set_destination(const t_ip_port &ip_port) {
	destinations.clear();
	destinations.push_back(ip_port);
}

bool t_request::www_authorize(const t_challenge &chlg, t_user *user_config, 
	       const string &username, const string &passwd, unsigned long nc,
	       const string &cnonce, t_credentials &cr, string &fail_reason)
{
	if (!authorize(chlg, user_config, username, passwd, nc, cnonce, cr, fail_reason)) {
		return false;
	}

	hdr_authorization.add_credentials(cr);

	return true;
}

bool t_request::proxy_authorize(const t_challenge &chlg, t_user *user_config,
	       const string &username, const string &passwd, unsigned long nc,
	       const string &cnonce, t_credentials &cr, string &fail_reason)
{
	if (!authorize(chlg, user_config, username, passwd, nc, cnonce, cr, fail_reason)) {
		return false;
	}

	hdr_proxy_authorization.add_credentials(cr);

	return true;
}

void t_request::calc_local_ip(void) {
	t_ip_port dst;
	
	get_current_destination(dst);
	if (dst.ipaddr != 0) {
		local_ip_ = get_src_ip4_address_for_dst(dst.ipaddr);
	}
}

bool t_request::is_registration_request(void) const {
	if (method != REGISTER) return false;
	
	if (hdr_expires.is_populated() && hdr_expires.time > 0) return true;
	
	if (hdr_contact.is_populated() && 
	    !hdr_contact.contact_list.empty() &&
	    hdr_contact.contact_list.front().is_expires_present() &&
	    hdr_contact.contact_list.front().get_expires() > 0)
	{
		return true;
	}
	
	return false;
}

bool t_request::is_de_registration_request(void) const {
	if (method != REGISTER) return false;

	if (hdr_expires.is_populated() && hdr_expires.time == 0) return true;
	
	if (hdr_contact.is_populated() && 
	    !hdr_contact.contact_list.empty() &&
	    hdr_contact.contact_list.front().is_expires_present() &&
	    hdr_contact.contact_list.front().get_expires() == 0)
	{
		return true;
	}
	
	return false;
}
