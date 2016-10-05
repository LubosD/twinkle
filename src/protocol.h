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
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _PROTOCOL_H
#define _PROTOCOL_H

#include "twinkle_config.h"
#include "parser/hdr_supported.h"

/** Carriage Return Line Feed */
#define CRLF		"\r\n"

/** TCP PING packet to be sent on a TCP connection. */
#define TCP_PING_PACKET	CRLF CRLF

/** Product name */
#define PRODUCT_NAME	"Twinkle"

/** Product version */
#define PRODUCT_VERSION	VERSION

/**
 * When a SIP message is created, some addresses will be filled in
 * by the sender thread as only this thread knows the source IP
 * address of an outgoing message.
 * The sender thread will look for occurrences of the AUTO_IP4_ADDRESS
 * and replace it with the source IP address.
 */
#define AUTO_IP4_ADDRESS	"255.255.255.255"

/** Display name for anonymous calling */
#define ANONYMOUS_DISPLAY	"Anonymous"

/** SIP-URI for anonymous calling */
#define ANONYMOUS_URI		"sip:anonymous@anonymous.invalid"

/** Types of failures. */
enum t_failure {
	FAIL_TIMEOUT,	/**< Transaction timed out */
	FAIL_TRANSPORT	/**< Transport failure */
};

/** Call transfer types */
enum t_transfer_type {
	TRANSFER_BASIC,		/**< Basic transfer (blind) */
	TRANSFER_CONSULT,	/**< Transfer with consultation (possibly attended) */
	TRANSFER_OTHER_LINE	/**< Transfer call to other line */
};

/** State of a call transfer at the referrer. */
enum t_refer_state {
	REFST_NULL,		/**< No REFER in progress */
	REFST_W4RESP,		/**< REFER sent, waiting for response */
	REFST_W4NOTIFY,		/**< Response received, waiting for 1st NOTIFY */
	REFST_PENDING,		/**< REFER received, but not granted yet */
	REFST_ACTIVE,		/**< Referee granted refer */
};

/** Types of registration requests */
enum t_register_type {
	REG_REGISTER,
	REG_QUERY,
	REG_DEREGISTER,
	REG_DEREGISTER_ALL
};

/**
 * RFC 3261 Annex A
 * SIP timers
 */
enum t_sip_timer {
	TIMER_T1,
	TIMER_T2,
	TIMER_T4,
	TIMER_A,
	TIMER_B,
	TIMER_C,
	TIMER_D,
	TIMER_E,
	TIMER_F,
	TIMER_G,
	TIMER_H,
	TIMER_I,
	TIMER_J,
	TIMER_K
};

// All durations are in msec
#define DURATION_T1	500
#define DURATION_T2	4000
#define DURATION_T4	5000
#define DURATION_A	DURATION_T1
#define DURATION_B	(64 * DURATION_T1)
#define DURATION_C	180000
#define DURATION_D	32000
#define DURATION_E	DURATION_T1
#define DURATION_F	(64 * DURATION_T1)
#define DURATION_G	DURATION_T1
#define DURATION_H	(64 * DURATION_T1)
#define DURATION_I	DURATION_T4
#define DURATION_J	(64 * DURATION_T1)
#define DURATION_K	DURATION_T4

/** Time to keep an idle connection open before closing */
#define DUR_IDLE_CONNECTION	(64 * DURATION_T1)

/** UA (phone) timers */
enum t_phone_timer {
	PTMR_REGISTRATION,	/**< Registration (failure) timeout */
	PTMR_NAT_KEEPALIVE,	/**< NAT binding refresh timeout for STUN */
	PTMR_TCP_PING,		/**< TCP ping interval */
};

/** UA (line) timers */
enum t_line_timer {
	LTMR_ACK_TIMEOUT,	/**< Waiting for ACK */
	LTMR_ACK_GUARD,		/**< After this timer ACK is lost for good */
	LTMR_INVITE_COMP,	/**< After this timer INVITE transiction is considered complete. */
	LTMR_NO_ANSWER,		/**< This timer expires if the callee does not answer. The call will be torn down. */
	LTMR_RE_INVITE_GUARD,	/**< re-INVITE timeout */
	LTMR_100REL_TIMEOUT,	/**< Waiting for PRACK */
	LTMR_100REL_GUARD,	/**< After this timer PRACK is lost for good */
	LTMR_GLARE_RETRY,	/**< Waiting before retry re-INVITE after glare */
	LTMR_CANCEL_GUARD,	/**< Guard for situation where CANCEL has been responded to, but 487 on INVITE is never received. */
};

/** Subscription timers. */
enum t_subscribe_timer {
	STMR_SUBSCRIPTION,	/**< Subscription timeout */
};

/** Publication timers. */
enum t_publish_timer {
	PUBLISH_TMR_PUBLICATION,	/**< Publication timeout */
};

/** STUN timers. */
enum t_stun_timer {
	STUN_TMR_REQ_TIMEOUT,	/**< Waiting for response */
};


/** No answer timer (ms) */
#define DUR_NO_ANSWER(u)	((u)->get_timer_noanswer() * 1000)

/** @name Registration timers */
//@{
/** Registration duration (seconds) */
#define DUR_REGISTRATION(u)	((u)->get_registration_time())

/**< Re-register 5 seconds before expiry **/
#define RE_REGISTER_DELTA	5

/** Re-registration interval after reg. failure */
#define DUR_REG_FAILURE         30
//@}

/** NAT keepalive timer (s) default value */
#define DUR_NAT_KEEPALIVE	30

/** Default TCP ping interval (s) */
#define DUR_TCP_PING		30

/**
 * re-INVITE guard timer (ms). This timer guards against the situation
 * where a UAC has sent a re-INVITE, received a 1XX but never receives
 * a final response. No timer for this is defined in RFC 3261
 */
#define DUR_RE_INVITE_GUARD	10000

/**
 * Guard for situation where CANCEL has been 
 * responded to, but 487 on INVITE is never eceived.
 * This situation is not defined by RFC 3261
 */
#define DUR_CANCEL_GUARD	(64 * DURATION_T1)

// MWI timers (s)
#define DUR_MWI(u)		((u)->get_mwi_subscription_time())
#define DUR_MWI_FAILURE		30

// Presence timers (s)
#define DUR_PRESENCE(u)		((u)->get_pres_subscription_time())
#define DUR_PRESENCE_FAILURE	30

// RFC 3261 14.1
// Maximum values (10th of sec) for timers for retrying a re-INVITE after
// a glare (491 response).
#define MAX_GLARE_RETRY_NOT_OWN	20
#define MAX_GLARE_RETRY_OWN	40

// Calculate the glare retry duration (ms)
#define DUR_GLARE_RETRY_NOT_OWN	((rand() % (MAX_GLARE_RETRY_NOT_OWN + 1)) * 100)
#define DUR_GLARE_RETRY_OWN	((rand() % (MAX_GLARE_RETRY_OWN - \
				MAX_GLARE_RETRY_NOT_OWN) + 1 + MAX_GLARE_RETRY_NOT_OWN)\
				* 100)

// RFC 3262
// PRACK timers
#define DUR_100REL_TIMEOUT	DURATION_T1
#define DUR_100REL_GUARD	(64 * DURATION_T1)

// refer subscription timer (s)
// RFC 3515 does not define the length of the timer.
// It should be long enough to notify the result of an INVITE.
#define DUR_REFER_SUBSCRIPTION	60 // Used when refer is always permitted
#define DUR_REFER_SUB_INTERACT	90 // Used when user has to grant permission

// Minimum duration of a subscription
#define MIN_DUR_SUBSCRIPTION	60

// Duration to wait before re-subscribing after termination of
// a subscription
#define DUR_RESUBSCRIBE		30

// After an unsubscribe has been sent, a NOTIFY will should come in.
// In case the NOTIFY does not come, this guard timer (ms) will assure
// that the subscription will be cleaned up.
#define DUR_UNSUBSCRIBE_GUARD	4000

// RFC 3489
// STUN retransmission timer intervals (ms)
// The RFC states that the interval should start at 100ms. But that
// seems to short. We start at 200 and do 8 instead of 9 transmissions.
#define DUR_STUN_START_INTVAL	200
#define DUR_STUN_MAX_INTVAL	1600

// Maximum number of transmissions
#define STUN_MAX_TRANSMISSIONS	8

// RFC 3261
#ifndef RFC3261_COOKIE
#define RFC3261_COOKIE	"z9hG4bK"
#endif

// Max forwards RFC 3261 8.1.1.6
#define MAX_FORWARDS	70

// Length of tags in from and to headers
#define TAG_LEN		5

// Create a new tag
#define NEW_TAG		random_token(TAG_LEN)

// Length of call-id (before domain)
#define CALL_ID_LEN	15

// Create a new call-id
#define NEW_CALL_ID(u)	(random_token(CALL_ID_LEN) + '@' + LOCAL_HOSTNAME)

// Create a new sequence number fo CSeq header
#define NEW_SEQNR	rand() % 1000 + 1

// Length of cnonce
#define CNONCE_LEN	10

// Create a cnonce
#define NEW_CNONCE	random_hexstr(CNONCE_LEN)

/** Length of tuple id in PIDF documents. */
#define PIDF_TUPLE_ID_LEN	6

/** Create a new PIDF tuple id. */
#define NEW_PIDF_TUPLE_ID	random_token(PIDF_TUPLE_ID_LEN)

/** Character set encoding for outgoing text messages */
#define MSG_TEXT_CHARSET	"utf-8"

/** @name Definitions for akav1-md5 authentication. */
#define AKA_RANDLEN	16
#define AKA_AUTNLEN	16
#define AKA_CKLEN	16
#define AKA_IKLEN	16
#define AKA_AKLEN	6
#define AKA_OPLEN	16
#define AKA_RESLEN	8
#define AKA_SQNLEN	6
#define AKA_RESHEXLEN	16
#define AKA_AMFLEN	2
#define AKA_KLEN	16

// Set Allow header with methods that can be handled by the phone
#define SET_HDR_ALLOW(h, u)	{ (h).add_method(INVITE); \
				  (h).add_method(ACK); \
				  (h).add_method(BYE); \
				  (h).add_method(CANCEL); \
				  (h).add_method(OPTIONS); \
				  if ((u)->get_ext_100rel() != EXT_DISABLED) {\
				  	(h).add_method(PRACK);\
				  }\
				  (h).add_method(REFER); \
				  (h).add_method(NOTIFY); \
				  (h).add_method(SUBSCRIBE); \
				  (h).add_method(INFO); \
				  (h).add_method(MESSAGE); \
				}

// Set Supported header with supported extensions
#define SET_HDR_SUPPORTED(h, u)	{ if ((u)->get_ext_replaces()) {\
					(h).add_feature(EXT_REPLACES);\
				  }\
				  (h).add_feature(EXT_NOREFERSUB);\
				}

// Set Accept header with accepted body types
#define SET_HDR_ACCEPT(h)	{ (h).add_media(t_media("application",\
				  "sdp")); }
				  
/** 
 * Check if the content type of an instant message is supported 
 * @param h [in] A SIP message.
 */
#define MESSAGE_CONTENT_TYPE_SUPPORTED(h)\
				((h).hdr_content_type.media.type == "application" ||\
				 (h).hdr_content_type.media.type == "audio" ||\
				 (h).hdr_content_type.media.type == "image" ||\
				 (h).hdr_content_type.media.type == "text" ||\
				 (h).hdr_content_type.media.type == "video")
				 
/** 
 * Set Accept header with accepted body types for instant messaging. 
 * @param h [inout] A SIP message.
 */
#define SET_MESSAGE_HDR_ACCEPT(h)	{ (h).add_media(t_media("application/*"));\
					  (h).add_media(t_media("audio/*"));\
					  (h).add_media(t_media("image/*"));\
					  (h).add_media(t_media("text/*"));\
					  (h).add_media(t_media("video/*")); }

/** 
 * Set Accept header with accepted body types for presence.
 * @param h [inout] A SIP message.
 */
#define SET_PRESENCE_HDR_ACCEPT(h)	{ (h).add_media(t_media("application",\
				  "pidf+xml")); }
				  
/** 
 * Set Accept header with accepted body types for MWI. 
 * @param h [inout] A SIP message.
 */
#define SET_MWI_HDR_ACCEPT(h)	{ (h).add_media(t_media("application",\
				  "simple-message-summary")); }

/** 
 * Set Accept-Encoding header with accepted encodings. 
 * @param h [inout] A SIP message.
 */
#define SET_HDR_ACCEPT_ENCODING(h)\
				{ (h).add_coding(t_coding("identity")); }
				
/** 
 * Check if content encoding is supported 
 * @param h [inout] A SIP message.
 */
#define CONTENT_ENCODING_SUPPORTED(ce)\
				(cmp_nocase(ce, "identity") == 0)

// Set Accept-Language header with accepted languages
#define SET_HDR_ACCEPT_LANGUAGE(h)\
				{ (h).add_language(t_language("en")); }

// Set User-Agent header
#define SET_HDR_USER_AGENT(h)	{ (h).add_server(t_server(PRODUCT_NAME,\
					PRODUCT_VERSION)); }

// Set Server header
#define SET_HDR_SERVER(h)	{ (h).add_server(t_server(PRODUCT_NAME,\
					PRODUCT_VERSION)); }

// Set Organization header
#define SET_HDR_ORGANIZATION(h, u)	{ if ((u)->get_organization() != "") {\
					(h).set_name((u)->get_organization()); }}

// Check if an event is supported by Twinkle				
#define SIP_EVENT_SUPPORTED(e)	((e) == SIP_EVENT_REFER ||\
				 (e) == SIP_EVENT_MSG_SUMMARY ||\
				 (e) == SIP_EVENT_PRESENCE)

// Add the supported events to the Allow-Events header
#define ADD_SUPPORTED_SIP_EVENTS(h)	{ (h).add_event_type(SIP_EVENT_REFER);\
					  (h).add_event_type(SIP_EVENT_MSG_SUMMARY);\
					  (h).add_event_type(SIP_EVENT_PRESENCE); }

#endif
