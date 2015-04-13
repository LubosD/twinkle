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

#ifndef _DEFINITIONS_H
#define _DEFINITIONS_H

#include <string>
#include "protocol.h"
#include "sockets/url.h"

using namespace std;

#define SIP_VERSION	"2.0"

// RFC 3261
#ifndef RFC3261_COOKIE
#define RFC3261_COOKIE	"z9hG4bK"
#endif

// Authentication schemes
#define AUTH_DIGEST	"Digest"

// Authentication algorithms
#define ALG_MD5		"MD5"
#define ALG_AKAV1_MD5	"AKAV1-MD5"
#define ALG_MD5_SESS	"MD5-sess"

// Authentication QOP
#define QOP_AUTH	"auth"
#define QOP_AUTH_INT	"auth-int"

/** SIP request methods. */
enum t_method {
	INVITE,
	ACK,
	OPTIONS,
	BYE,
	CANCEL,
	REGISTER,
	PRACK,
	SUBSCRIBE,
	NOTIFY,
	REFER,
	INFO,
	MESSAGE,
	PUBLISH,
	METHOD_UNKNOWN
};

/**
 * Convert a method to a string.
 * @param m The method.
 * @param unknown Method name if m is @ref METHOD_UNKNOWN.
 * @return The name of the method.
 */
string method2str(const t_method &m, const string &unknown = "");

/**
 * Convert a string to a method.
 * @param s The string.
 * @return The method having s as name. If s is an unknown name,
 * then @ref METHOD_UNKNOWN is returned.
 */
t_method str2method(const string &s);

#endif
