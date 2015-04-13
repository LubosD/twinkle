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

#include "diamondcard.h"

#include <cassert>
#include "twinkle_config.h"
#include "util.h"
#include "sockets/url.h"

#define DIAMONDCARD_DISTSITE	"twinkle"

#define DIAMONDCARD_URL_SIGNUP	"https://www.diamondcard.us/exec/voip-login?act=sgn&spo=%DISTSITE"
#define DIAMONDCARD_URL_ACTION  "https://www.diamondcard.us/exec/voip-login?"\
				"accId=%ACCID&pinCode=%PIN&act=%ACT&spo=%DISTSITE"

string diamondcard_url(t_dc_action action, const string &accountId, const string &pinCode)
{
	string url;
	
	if (action == DC_ACT_SIGNUP) {
		url = DIAMONDCARD_URL_SIGNUP;
	} else {
		url = DIAMONDCARD_URL_ACTION;
		url = replace_first(url, "%ACCID", t_url::escape_hnv(accountId));
		url = replace_first(url, "%PIN", t_url::escape_hnv(pinCode));
		
		switch (action) {
		case DC_ACT_BALANCE_HISTORY:
			url = replace_first(url, "%ACT", "bh");
			break;
		case DC_ACT_RECHARGE:
			url = replace_first(url, "%ACT", "rch");
			break;
		case DC_ACT_CALL_HISTORY:
			url = replace_first(url, "%ACT", "ch");
			break;
		case DC_ACT_ADMIN_CENTER:
			url = replace_first(url, "%ACT", "log");
			break;
		default:
			assert(false);
		}
	}
	
	url = replace_first(url, "%DISTSITE", DIAMONDCARD_DISTSITE);
	
	return url;
}

void diamondcard_set_user_config(t_user &user, const string &displayName,
                                 const string &accountId, const string &pinCode)
{
	// User
	user.set_display(displayName);
	user.set_name(accountId);
	
	// The real domain name is "diamondcard.us", but Diamondcard
	// instructs users to use "sip.diamondcard.us" for the domain.
	// This latter name is resolvable by both a DNS SRV and A lookup.
	// So clients not capable of DNS SRV lookups van still work with
	// this domain name. To be consistent with other client settings
	// Twinkle uses this domain name too.
	user.set_domain("sip.diamondcard.us");
	
	user.set_auth_name(accountId);
	user.set_auth_pass(pinCode);
	
	// SIP server
	user.set_use_outbound_proxy(true);
	user.set_outbound_proxy(t_url("sip:sip.diamondcard.us"));
	
	// Audio codecs
	list<t_audio_codec> codecs;
	codecs.push_back(CODEC_G711_ULAW);
	codecs.push_back(CODEC_G711_ALAW);
#ifdef HAVE_ILBC
	codecs.push_back(CODEC_ILBC);
#endif
	codecs.push_back(CODEC_GSM);
	user.set_codecs(codecs);
	
	// Voice mail
	user.set_mwi_vm_address("80");
	
	// IM
	user.set_im_send_iscomposing(false);
	
	// Presence
	user.set_pres_publish_startup(false);

	// NAT
	user.set_enable_nat_keepalive(true);
	user.set_timer_nat_keepalive(20);
	
	// Address format
	user.set_numerical_user_is_phone(true);
}

list<t_user *>diamondcard_get_users(t_phone *phone) {
	list<t_user *> users = phone->ref_users();
	list<t_user *> diamond_users;
	
	for (list<t_user *>::const_iterator it = users.begin(); it != users.end(); ++it) {
		if ((*it)->is_diamondcard_account()) {
			diamond_users.push_back(*it);
		}
	}
	
	return diamond_users;
}
