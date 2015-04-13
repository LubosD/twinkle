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

// Response

#ifndef _H_RESPONSE
#define _H_RESPONSE

#include <string>
#include "sip_message.h"

using namespace std;

// Repsonse codes
// Informational
#define	R_100_TRYING 100
#define	R_180_RINGING 180
#define	R_181_CALL_IS_BEING_FORWARDED 181
#define	R_182_QUEUED 182
#define	R_183_SESSION_PROGRESS 183

// Success
#define	R_200_OK 200
#define R_202_ACCEPTED 202

// Redirection
#define	R_300_MULTIPLE_CHOICES 300
#define	R_301_MOVED_PERMANENTLY 301
#define	R_302_MOVED_TEMPORARILY 302
#define	R_305_USE_PROXY 305
#define	R_380_ALTERNATIVE_SERVICE 380

// Client error
#define	R_400_BAD_REQUEST 400
#define	R_401_UNAUTHORIZED 401
#define	R_402_PAYMENT_REQUIRED 402
#define	R_403_FORBIDDEN 403
#define	R_404_NOT_FOUND 404
#define	R_405_METHOD_NOT_ALLOWED 405
#define	R_406_NOT_ACCEPTABLE 406
#define	R_407_PROXY_AUTH_REQUIRED 407
#define	R_408_REQUEST_TIMEOUT 408
#define	R_410_GONE 410
#define R_412_CONDITIONAL_REQUEST_FAILED 412
#define	R_413_REQ_ENTITY_TOO_LARGE 413
#define	R_414_REQ_URI_TOO_LARGE 414
#define	R_415_UNSUPPORTED_MEDIA_TYPE 415
#define	R_416_UNSUPPORTED_URI_SCHEME 416
#define	R_420_BAD_EXTENSION 420
#define	R_421_EXTENSION_REQUIRED 421
#define	R_423_INTERVAL_TOO_BRIEF 423
#define	R_480_TEMP_NOT_AVAILABLE 480
#define	R_481_TRANSACTION_NOT_EXIST 481
#define	R_482_LOOP_DETECTED 482
#define	R_483_TOO_MANY_HOPS 483
#define	R_484_ADDRESS_INCOMPLETE 484
#define	R_485_AMBIGUOUS 485
#define	R_486_BUSY_HERE 486
#define	R_487_REQUEST_TERMINATED 487
#define	R_488_NOT_ACCEPTABLE_HERE 488
#define R_489_BAD_EVENT 489
#define	R_491_REQUEST_PENDING 491
#define	R_493_UNDECIPHERABLE 493

// Server error
#define	R_500_INTERNAL_SERVER_ERROR 500
#define	R_501_NOT_IMPLEMENTED 501
#define	R_502_BAD_GATEWAY 502
#define	R_503_SERVICE_UNAVAILABLE 503
#define	R_504_SERVER_TIMEOUT 504
#define	R_505_SIP_VERSION_NOT_SUPPORTED 505
#define	R_513_MESSAGE_TOO_LARGE 513

// Global failure
#define	R_600_BUSY_EVERYWHERE 600
#define	R_603_DECLINE 603
#define	R_604_NOT_EXIST_ANYWHERE 604
#define	R_606_NOT_ACCEPTABLE 606

// Response classes
#define	R_1XX	1	// Informational
#define	R_2XX	2	// Success
#define	R_3XX	3	// Redirection
#define	R_4XX	4	// Client error
#define	R_5XX	5	// Server error
#define	R_6XX	6	// Global failure

// Default reason strings
#define REASON_100 "Trying"
#define REASON_180 "Ringing"
#define REASON_181 "Call Is Being Forwarded"
#define REASON_182 "Queued"
#define REASON_183 "Session Progress"

#define REASON_200 "OK"
#define REASON_202 "Accepted"

#define REASON_300 "Multiple Choices"
#define REASON_301 "Moved Permanently"
#define REASON_302 "Moved Temporarily"
#define REASON_305 "Use Proxy"
#define REASON_380 "Alternative Service"

#define REASON_400 "Bad Request"
#define REASON_401 "Unauthorized"
#define REASON_402 "Payment Required"
#define REASON_403 "Forbidden"
#define REASON_404 "Not Found"
#define REASON_405 "Method Not Allowed"
#define REASON_406 "Not Acceptable"
#define REASON_407 "Proxy Authentication Required"
#define REASON_408 "Request Timeout"
#define REASON_410 "Gone"
#define REASON_412 "Conditional Request Failed"
#define REASON_413 "Request Entity Too Large"
#define REASON_414 "Request-URI Too Large"
#define REASON_415 "Unsupported Media Type"
#define REASON_416 "Unsupported URI Scheme"
#define REASON_420 "Bad Extension"
#define REASON_421 "Extension Required"
#define REASON_423 "Interval Too Brief"
#define REASON_480 "Temporarily Not Available"
#define REASON_481 "Call Leg/Transaction Does Not Exist"
#define REASON_482 "Loop Detected"
#define REASON_483 "Too Many Hops"
#define REASON_484 "Address Incomplete"
#define REASON_485 "Ambiguous"
#define REASON_486 "Busy Here"
#define REASON_487 "Request Terminated"
#define REASON_488 "Not Acceptable Here"
#define REASON_489 "Bad Event"
#define REASON_491 "Request Pending"
#define REASON_493 "Undecipherable"

#define REASON_500 "Internal Server Error"
#define REASON_501 "Not Implemented"
#define REASON_502 "Bad Gateway"
#define REASON_503 "Service Unavailable"
#define REASON_504 "Server Time-out"
#define REASON_505 "SIP Version Not Supported"
#define REASON_513 "Message Too Large"

#define REASON_600 "Busy Everywhere"
#define REASON_603 "Decline"
#define REASON_604 "Does Not Exist Anywhere"
#define REASON_606 "Not Acceptable"

// The protocol allows a SIP response to have a non-default reason
// phrase that gives a more detailed reason.

// RFC 3261 21.4.18
// Code 480 should have a specific reason phrase
#define REASON_480_NO_ANSWER			"User not responding"

// RFC 3265 3.2.4
#define REASON_481_SUBSCRIPTION_NOT_EXIST	"Subscription does not exist"


class t_response : public t_sip_message {
public:
	int		code;
	string		reason;
	
	/** The source address of the request generating this response. */
	t_ip_port	src_ip_port_request;

	t_response();
	t_response(const t_response &r);
	t_response(int _code, string _reason = "");

	t_msg_type get_type(void) const { return MSG_RESPONSE; }

	// Return the response class 1,2,3,4,5,6
	int get_class(void) const;

	bool is_provisional(void) const;
	bool is_final(void) const;
	bool is_success(void) const;

	string encode(bool add_content_length = true);
	list<string> encode_env(void);
	t_sip_message *copy(void) const;

	bool is_valid(bool &fatal, string &reason) const;

	// Returns true if the response is a 401/407 with
	// the proper authenticate header.
	bool must_authenticate(void) const;
	
	/**
	 * Get the destination address for sending the response.
	 * @param ip_port [out] The destination address.
	 */
	void get_destination(t_ip_port &ip_port) const;
	
	virtual void calc_local_ip(void);
};

#endif
