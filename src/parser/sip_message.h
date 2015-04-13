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

// SIP message

#ifndef _H_SIP_MESSAGE
#define _H_SIP_MESSAGE

#include <list>
#include <string>
#include "definitions.h"
#include "hdr_accept.h"
#include "hdr_accept_encoding.h"
#include "hdr_accept_language.h"
#include "hdr_alert_info.h"
#include "hdr_allow.h"
#include "hdr_allow_events.h"
#include "hdr_auth_info.h"
#include "hdr_authorization.h"
#include "hdr_call_id.h"
#include "hdr_call_info.h"
#include "hdr_contact.h"
#include "hdr_content_disp.h"
#include "hdr_content_encoding.h"
#include "hdr_content_language.h"
#include "hdr_content_length.h"
#include "hdr_content_type.h"
#include "hdr_cseq.h"
#include "hdr_date.h"
#include "hdr_error_info.h"
#include "hdr_event.h"
#include "hdr_expires.h"
#include "hdr_from.h"
#include "hdr_in_reply_to.h"
#include "hdr_max_forwards.h"
#include "hdr_min_expires.h"
#include "hdr_mime_version.h"
#include "hdr_organization.h"
#include "hdr_p_asserted_identity.h"
#include "hdr_p_preferred_identity.h"
#include "hdr_priority.h"
#include "hdr_privacy.h"
#include "hdr_proxy_authenticate.h"
#include "hdr_proxy_authorization.h"
#include "hdr_proxy_require.h"
#include "hdr_rack.h"
#include "hdr_record_route.h"
#include "hdr_refer_sub.h"
#include "hdr_refer_to.h"
#include "hdr_referred_by.h"
#include "hdr_replaces.h"
#include "hdr_reply_to.h"
#include "hdr_require.h"
#include "hdr_request_disposition.h"
#include "hdr_retry_after.h"
#include "hdr_route.h"
#include "hdr_rseq.h"
#include "hdr_server.h"
#include "hdr_service_route.h"
#include "hdr_sip_etag.h"
#include "hdr_sip_if_match.h"
#include "hdr_subject.h"
#include "hdr_subscription_state.h"
#include "hdr_supported.h"
#include "hdr_timestamp.h"
#include "hdr_to.h"
#include "hdr_unsupported.h"
#include "hdr_user_agent.h"
#include "hdr_via.h"
#include "hdr_warning.h"
#include "hdr_www_authenticate.h"
#include "parameter.h"
#include "sip_body.h"

// Macro's to access the body of a message, eg msg.sdp_body
#define SDP_BODY	((t_sdp *)body)
#define OPAQUE_BODY	((t_sip_body_opaque)*body)

using namespace std;

enum t_msg_type {
	MSG_REQUEST,
	MSG_RESPONSE,
	MSG_SIPFRAG,	// Only a sequence of headers (RFC 3420)
};


class t_sip_message {
protected:
	/**
	 * Local IP address that will be uses for this SIP message.
	 * The local IP address can only be determined when the destination
	 * of a SIP message is known (because of multi homing).
	 */
	unsigned long		local_ip_;
	
public:
	// The source IP address and port are only set for messages
	// received from the network. So the transaction user knows
	// where a message comes from.
	t_ip_port		src_ip_port;

	// SIP version
	string			version;

	// All possible headers
	t_hdr_accept		hdr_accept;
	t_hdr_accept_encoding	hdr_accept_encoding;
	t_hdr_accept_language	hdr_accept_language;
	t_hdr_alert_info	hdr_alert_info;
	t_hdr_allow		hdr_allow;
	t_hdr_allow_events	hdr_allow_events;
	t_hdr_auth_info		hdr_auth_info;
	t_hdr_authorization	hdr_authorization;
	t_hdr_call_id		hdr_call_id;
	t_hdr_call_info		hdr_call_info;
	t_hdr_contact		hdr_contact;
	t_hdr_content_disp	hdr_content_disp;
	t_hdr_content_encoding	hdr_content_encoding;
	t_hdr_content_language	hdr_content_language;
	t_hdr_content_length	hdr_content_length;
	t_hdr_content_type	hdr_content_type;
	t_hdr_cseq		hdr_cseq;
	t_hdr_date		hdr_date;
	t_hdr_error_info	hdr_error_info;
	t_hdr_event		hdr_event;
	t_hdr_expires		hdr_expires;
	t_hdr_from		hdr_from;
	t_hdr_in_reply_to	hdr_in_reply_to;
	t_hdr_max_forwards	hdr_max_forwards;
	t_hdr_min_expires	hdr_min_expires;
	t_hdr_mime_version	hdr_mime_version;
	t_hdr_organization	hdr_organization;
	t_hdr_p_asserted_identity hdr_p_asserted_identity;
	t_hdr_p_preferred_identity hdr_p_preferred_identity;
	t_hdr_priority		hdr_priority;
	t_hdr_privacy		hdr_privacy;
	t_hdr_proxy_authenticate  hdr_proxy_authenticate;
	t_hdr_proxy_authorization hdr_proxy_authorization;
	t_hdr_proxy_require	hdr_proxy_require;
	t_hdr_rack		hdr_rack;
	t_hdr_record_route	hdr_record_route;
	t_hdr_refer_sub		hdr_refer_sub;
	t_hdr_refer_to		hdr_refer_to;
	t_hdr_referred_by	hdr_referred_by;
	t_hdr_replaces		hdr_replaces;
	t_hdr_reply_to		hdr_reply_to;
	t_hdr_require		hdr_require;
	t_hdr_request_disposition hdr_request_disposition;
	t_hdr_retry_after	hdr_retry_after;
	t_hdr_route		hdr_route;
	t_hdr_rseq		hdr_rseq;
	t_hdr_server		hdr_server;
	t_hdr_service_route	hdr_service_route;
	t_hdr_sip_etag		hdr_sip_etag;
	t_hdr_sip_if_match	hdr_sip_if_match;
	t_hdr_subject		hdr_subject;
	t_hdr_subscription_state hdr_subscription_state;
	t_hdr_supported		hdr_supported;
	t_hdr_timestamp		hdr_timestamp;
	t_hdr_to		hdr_to;
	t_hdr_unsupported	hdr_unsupported;
	t_hdr_user_agent	hdr_user_agent;
	t_hdr_via		hdr_via;
	t_hdr_warning		hdr_warning;
	t_hdr_www_authenticate	hdr_www_authenticate;

	// Unknown headers are represented by parameters.
	// Parameter.name = header name
	// Parameter.value = header value
	list<t_parameter>	unknown_headers;

	// A SIP message can carry a body
	t_sip_body		*body;

	t_sip_message();
	t_sip_message(const t_sip_message& m);
	virtual ~t_sip_message();

	virtual t_msg_type get_type(void) const;
	void add_unknown_header(const string &name, const string &value);

	// Check if the message is valid. At this class the
	// general rules applying to both requests and responses
	// is checked.
	// fatal is true if one of the headers mandatory for all
	// messages is missing (to, from, cseq, call-id, via).
	// reason contains a reason string if the message is invalid.
	virtual bool is_valid(bool &fatal, string &reason) const;

	// Return encoded headers
	// The version should be encode by the subclasses.
	// Parameter add_content_length indicates if a Content-Length
	// header must be added. Usually it must, only for sipfrag bodies
	// it may be omitted.
	virtual string encode(bool add_content_length = true);
	
	// Return list of environment variable settings for all headers
	// (see header.h for the format)
	// Besides the header variables the following variables will be
	// returned as well:
	//
	// SIP_REQUEST_METHOD, for a request
	// SIP_REQUEST_URI, for a request
	// SIP_STATUS_CODE, for a response
	// SIP_STATUS_REASON, for a response
	virtual list<string> encode_env(void);

	// Create a copy of the message
	virtual t_sip_message *copy(void) const;
	
	/**
	 * Set a plain text body in the message.
	 * @param text [in] The text.
	 * @param charset [in] The character set used for encoding.
	 * @post The Content-Type header is set to "text/plain".
	 * @post If a body was already present then it is deleted.
	 */
	void set_body_plain_text(const string &text, const string &charset);
	
	/**
	 * Set a body with the contents of a file.
	 * @param filename [in] The name of the file.
	 * @param media [in] The mime type of the contents.
	 * @return True of body is set, false if file could not be read.
	 */
	bool set_body_from_file(const string &filename, const t_media &media);
	
	/**
	 * Get the size of an encoded SIP message.
	 * @return Size in bytes.
	 */
	size_t get_encoded_size(void);
	
	/**
	 * Check if all local IP address are correctly filled in. This
	 * check is an integrity check to help debugging the auto IP
	 * discover feature.
	 */
	bool local_ip_check(void) const;
	
	/** Determine the local IP address for this SIP message. */
	virtual void calc_local_ip(void);
	
	/**
	 * Get the local IP address for this SIP message.
	 * The local IP address can be used as source address for sending
	 * the message.
	 * @return The local IP address.
	 * @return 0, if the local IP address is not determined yet.
	 */
	unsigned long get_local_ip(void);
};

#endif
