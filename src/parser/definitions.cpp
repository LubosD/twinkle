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
#include "definitions.h"
#include "parse_ctrl.h"

string method2str(const t_method &m, const string &unknown) {
	switch (m) {
	case INVITE:		return "INVITE";
	case ACK:		return "ACK";
	case OPTIONS:		return "OPTIONS";
	case BYE:		return "BYE";
	case CANCEL:		return "CANCEL";
	case REGISTER:		return "REGISTER";
	case PRACK:		return "PRACK";
	case SUBSCRIBE:		return "SUBSCRIBE";
	case NOTIFY:		return "NOTIFY";
	case REFER:		return "REFER";
	case INFO:		return "INFO";
	case MESSAGE:		return "MESSAGE";
	case PUBLISH:		return "PUBLISH";
	case METHOD_UNKNOWN:	return unknown;
	default:		assert(false);
	}
}

t_method str2method(const string &s) {
	if (s == "INVITE") return INVITE;
	if (s == "ACK") return ACK;
	if (s == "OPTIONS") return OPTIONS;
	if (s == "BYE") return BYE;
	if (s == "CANCEL") return CANCEL;
	if (s == "REGISTER") return REGISTER;
	if (s == "PRACK") return PRACK;
	if (s == "SUBSCRIBE") return SUBSCRIBE;
	if (s == "NOTIFY") return NOTIFY;
	if (s == "REFER") return REFER;
	if (s == "INFO") return INFO;
	if (s == "MESSAGE") return MESSAGE;
	if (s == "PUBLISH") return PUBLISH;

	return METHOD_UNKNOWN;
}
