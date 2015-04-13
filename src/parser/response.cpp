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

#include <cassert>

#include "response.h"
#include "util.h"
#include "parse_ctrl.h"
#include "audits/memman.h"

t_response::t_response() : t_sip_message() {}

t_response::t_response(const t_response &r) : t_sip_message(r) ,
		code(r.code),
		reason(r.reason),
		src_ip_port_request(r.src_ip_port_request)
{
}

t_response::t_response(int _code, string _reason) : t_sip_message() {
	code = _code;

	if (_reason == "") {
		switch (code) {
		case 100: reason = REASON_100; break;
		case 180: reason = REASON_180; break;
		case 181: reason = REASON_181; break;
		case 182: reason = REASON_182; break;
		case 183: reason = REASON_183; break;
		case 200: reason = REASON_200; break;
		case 202: reason = REASON_202; break;
		case 300: reason = REASON_300; break;
		case 301: reason = REASON_301; break;
		case 302: reason = REASON_302; break;
		case 305: reason = REASON_305; break;
		case 380: reason = REASON_380; break;
		case 400: reason = REASON_400; break;
		case 401: reason = REASON_401; break;
		case 402: reason = REASON_402; break;
		case 403: reason = REASON_403; break;
		case 404: reason = REASON_404; break;
		case 405: reason = REASON_405; break;
		case 406: reason = REASON_406; break;
		case 407: reason = REASON_407; break;
		case 408: reason = REASON_408; break;
		case 410: reason = REASON_410; break;
		case 412: reason = REASON_412; break;
		case 413: reason = REASON_413; break;
		case 414: reason = REASON_414; break;
		case 415: reason = REASON_415; break;
		case 416: reason = REASON_416; break;
		case 420: reason = REASON_420; break;
		case 421: reason = REASON_421; break;
		case 423: reason = REASON_423; break;
		case 480: reason = REASON_480; break;
		case 481: reason = REASON_481; break;
		case 482: reason = REASON_482; break;
		case 483: reason = REASON_483; break;
		case 484: reason = REASON_484; break;
		case 485: reason = REASON_485; break;
		case 486: reason = REASON_486; break;
		case 487: reason = REASON_487; break;
		case 488: reason = REASON_488; break;
		case 489: reason = REASON_489; break;
		case 491: reason = REASON_491; break;
		case 493: reason = REASON_493; break;
		case 500: reason = REASON_500; break;
		case 501: reason = REASON_501; break;
		case 502: reason = REASON_502; break;
		case 503: reason = REASON_503; break;
		case 504: reason = REASON_504; break;
		case 505: reason = REASON_505; break;
		case 513: reason = REASON_513; break;
		case 600: reason = REASON_600; break;
		case 603: reason = REASON_603; break;
		case 604: reason = REASON_604; break;
		case 606: reason = REASON_606; break;
		default:  reason = "Unknown Error";
		}
	} else {
		reason = _reason;
	}
}

int t_response::get_class(void) const {
	return code / 100;
}

bool t_response::is_provisional(void) const {
	return (get_class() == R_1XX);
}

bool t_response::is_final(void) const {
	return (get_class() != R_1XX);
}

bool t_response::is_success(void) const {
	return (get_class() == R_2XX);
}

string t_response::encode(bool add_content_length) {
	string s;

	s = "SIP/" + version + ' ' + int2str(code, "%3d") + ' ' + reason;
	s += CRLF;
	s += t_sip_message::encode(add_content_length);

	return s;
}

list<string> t_response::encode_env(void) {
	string s;

	list<string> l = t_sip_message::encode_env();
	
	s = "SIPSTATUS_CODE=";
	s += int2str(code, "%3d");
	l.push_back(s);
	
	s = "SIPSTATUS_REASON=";
	s += reason;
	l.push_back(s);
	
	return l;
}

t_sip_message *t_response::copy(void) const {
	t_sip_message *m = new t_response(*this);
	MEMMAN_NEW(m);
	return m;
}

bool t_response::is_valid(bool &fatal, string &reason) const {
	if (!t_sip_message::is_valid(fatal, reason)) return false;

	fatal = false;

	switch(hdr_cseq.method) {
	case INVITE:
		if (get_class() == R_2XX && !hdr_contact.is_populated()) {
			reason = "Contact header missing";
			return false;
		}
		break;
	case SUBSCRIBE:
		// RFC 3265 7.1, 7.2
		/*
		Some SIP servers do not send the mandatory Expires header.
		For interoperability this deviation is allowed.
		if (get_class()== R_2XX && !hdr_expires.is_populated()) {
			reason = "Expires header missing";
			return false;
		}
		*/

		switch (code) {
		case R_489_BAD_EVENT:
			if (!hdr_allow_events.is_populated()) {
				reason = "Allow-Events header missing";
				return false;
			}
			break;
		}

		break;
	case NOTIFY:
		// RFC 3265 7.1, 7.2
		switch (code) {
		case R_489_BAD_EVENT:
			if (!hdr_allow_events.is_populated()) {
				reason = "Allow-Events header is missing";
				return false;
			}
			break;
		}

		break;
	default:
		break;
	}
	
	if (hdr_rseq.is_populated()) {
		// RFC 3262 7.1
		// The value ranges from 1 to 2**32 - 1
		if (hdr_rseq.resp_nr == 0) {
			reason = "RSeq is zero";
			return false;
		}
	}

	return true;
}

bool t_response::must_authenticate(void) const {
	return (code == R_401_UNAUTHORIZED &&
	        hdr_www_authenticate.is_populated() ||
	        code == R_407_PROXY_AUTH_REQUIRED &&
	        hdr_proxy_authenticate.is_populated());
}

void t_response::get_destination(t_ip_port &ip_port) const {
	assert(hdr_via.is_populated());
	
	if (src_ip_port_request.transport == "tcp") {
		// RFC 3261 18.2.2
		// For TCP the response should be sent on the connection on which
		// the request was received. So the address returned here is the
		// alternative destination when the connection is closed already.
		ip_port = src_ip_port_request;
	} else {
		hdr_via.get_response_dst(ip_port);
	}
}

void t_response::calc_local_ip(void) {
	t_ip_port dst;
	
	get_destination(dst);
	if (dst.ipaddr != 0) {
		local_ip_ = get_src_ip4_address_for_dst(dst.ipaddr);
	}
}
