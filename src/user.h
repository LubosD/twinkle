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

// NOTE:
// When adding attributes to t_user, make sure to add them to the
// copy constructor too!

#ifndef _H_USER
#define _H_USER

#include <string>
#include <list>
#include <cc++/config.h>
#include "protocol.h"
#include "sys_settings.h"
#include "audio/audio_codecs.h"
#include "sockets/url.h"
#include "threads/mutex.h"
#include "boost/regex.hpp"

// Forward declaration
class t_request;

// Default config file name
#define USER_CONFIG_FILE	"twinkle.cfg"
#define USER_FILE_EXT		".cfg"
#define USER_DIR		DIR_USER

#define USER_SCHEME		"sip"

#define PUBLIC_SIP_PORT(u)	phone->get_public_port_sip(u)
#define USER_HOST(u,local_ip)	phone->get_ip_sip(u,(local_ip))
#define LOCAL_IP		user_host
#define LOCAL_HOSTNAME		local_hostname

#define SPECIAL_PHONE_SYMBOLS	"-()/."

using namespace std;

enum t_hold_variant {
	HOLD_RFC2543,	// set IP = 0.0.0.0 in c-line
	HOLD_RFC3264	// use direction attribute to put call on-hold
};

/** SIP transport mode */
enum t_sip_transport {
	SIP_TRANS_UDP,	/**< SIP over UDP */
	SIP_TRANS_TCP,	/**< SIP over TCP */
	SIP_TRANS_AUTO	/**< UDP for small messages, TCP for large messages */
};

enum t_ext_support {
	EXT_INVALID,
	EXT_DISABLED,
	EXT_SUPPORTED,
	EXT_PREFERRED,
	EXT_REQUIRED
};

enum t_bit_rate_type {
	BIT_RATE_INVALID,
	BIT_RATE_CBR,	// Constant
	BIT_RATE_VBR,	// Variable
	BIT_RATE_ABR	// Average
};

enum t_dtmf_transport {
	DTMF_INBAND,
	DTMF_RFC2833,
	DTMF_AUTO,
	DTMF_INFO
};

enum t_g726_packing {
	G726_PACK_RFC3551,
	G726_PACK_AAL2
};

struct t_number_conversion {
	boost::regex	re;
	string		fmt;
	
	string str(void) const { return re.str() + " --> " + fmt; }
};


class t_user {
private:
	string			config_filename;
	
	// Mutex for exclusive access to the user profile
	mutable t_recursive_mutex	mtx_user;

	/** @name USER */
	//@{
	// SIP user
	/** User name (public user identity). */
	string			name;
	
	/** Domain of the user. */
	string			domain;
	
	/** Display name. */
	string			display;

	/**
	 * The organization will be put in an initial INVITE and in a
	 * 200 OK on an INVITE.
	 */
	string			organization;

	// SIP authentication
	/** Authentication realm. An empty realm matches with all realms. */
	string			auth_realm;
	
	/** Authentication name (private user identity). */
	string			auth_name;
	
	/** Authentication password (aka_k for akav1-md5 authentication) */
	string			auth_pass;
	
	/** Operator variant key for akav1-md5 authentication. */
	uint8			auth_aka_op[AKA_OPLEN];
	
	/** Authentication management field for akav1-md5 authentication. */
	uint8			auth_aka_amf[AKA_AMFLEN];
	//@}


	// SIP SERVER

	// Send all non-REGISTER requests to the outbound proxy
	bool			use_outbound_proxy;
	t_url			outbound_proxy;

	// By default only out-of-dialog requests (including the ones the
	// establish a dialog) are sent to the outbound proxy.
	// In-dialog requests go to the address established via the contact
	// header.
	// By setting this parameter to true, in-dialog requests go to the
	// outbound proxy as well.
	bool			all_requests_to_proxy;

	// Only send the request to the proxy, if the destination cannot
	// be resolved to an IP address, adhearing to the previous setting
	// though. I.e. use_outbound_proxy must be true. And an in-dialog
	// request will only be sent to the proxy if all_requests_to_proxy
	// is true.
	bool			non_resolvable_to_proxy;

	// Send REGISTER to registrar
	bool			use_registrar;
	t_url			registrar;
	
	// Registration time requested by the client. If set to zero, then
	// no specific time is requested. The registrar will set a time.
	unsigned long		registration_time;

	// Automatically register at startup of the client.
	bool			register_at_startup;

	// q-value for registration
	bool			reg_add_qvalue;
	float			reg_qvalue;


	// AUDIO

	list<t_audio_codec>	codecs; // in order of preference
	unsigned short		ptime; // ptime (ms) for G.711/G.726
	
	// For outgoing calls, obey the preference from the far-end (SDP answer),
	// i.e. pick the first codec from the SDP answer that we support.
	bool			out_obey_far_end_codec_pref;
	
	// For incoming calls, obey the preference from the far-end (SDP offer),
	// i.e. pick the first codec from the SDP offer that we support.
	bool			in_obey_far_end_codec_pref;
	
	// RTP dynamic payload types for speex
	unsigned short		speex_nb_payload_type;
	unsigned short		speex_wb_payload_type;
	unsigned short		speex_uwb_payload_type;
	
	// Speex preprocessing options
	bool			speex_dsp_vad;       // voice activity reduction
	bool			speex_dsp_agc;       // automatic gain control
	bool			speex_dsp_aec;       // acoustic echo cancellation
	bool			speex_dsp_nrd;       // noise reduction
	unsigned short		speex_dsp_agc_level; // gain level of AGC (1-100[%])

	// Speex coding options
	t_bit_rate_type		speex_bit_rate_type;
	int			speex_abr_nb;
	int			speex_abr_wb;
	bool			speex_dtx;
	bool			speex_penh;
	unsigned short		speex_complexity;
	unsigned short		speex_quality; // quality measure (worst 0-10 best)
	
	// RTP dynamic payload types for iLBC
	unsigned short		ilbc_payload_type;
	
	// iLBC options
	unsigned short		ilbc_mode; // 20 or 30 ms frame size
	
	// RTP dynamic payload types for G.726
	unsigned short		g726_16_payload_type;
	unsigned short		g726_24_payload_type;
	unsigned short		g726_32_payload_type;
	unsigned short		g726_40_payload_type;
	
	// Bit packing order for G,726
	t_g726_packing		g726_packing;
	
	// Transport mode for DTMF
	t_dtmf_transport	dtmf_transport;

	// RTP dynamic payload type for out-of-band DTMF.
	unsigned short		dtmf_payload_type;

	// DTMF duration and pause between 2 tones. During the pause the last
	// DTMF event will be repeated so the far end can detect the end of
	// the event in case of packet loss.
	unsigned short		dtmf_duration; // ms
	unsigned short		dtmf_pause; // ms
	
	// Volume of the tone in -dBm
	unsigned short		dtmf_volume;


	/** @name SIP PROTOCOL */
	//@{
	// SIP protocol options
	// hold variants: rfc2543, rfc3264
	// rfc2543 - set IP address to 0.0.0.0
	// rfc3264 - use direction attribute (sendrecv, sendonly, ...)
	t_hold_variant		hold_variant;

	// Indicate if the mandatory Max-Forwards header should be present.
	// If true and the header is missing, then the request will fail.
	bool			check_max_forwards;
	
	// RFC 3261 10.3 states that a registrar must include a contact
	// header in a 200 OK on a REGISTER. This contact should match the
	// contact that a UA puts in the REGISTER. Unfortunately many
	// registrars do not include the contact header or put a wrong
	// IP address in the host-part due to NAT.
	// This settings allows for a missing/non-matching contact header.
	// In that case Twinkle assumes that it is registered for the
	// requested interval.
	bool			allow_missing_contact_reg;

	// Indicate the place of the requested registration time in a REGISTER.
	// true - expires parameter in contact header
	// false - Expires header
	bool			registration_time_in_contact;

	// Indicate if compact header names should be used in outgoing messages.
	bool			compact_headers;
	
	// Indicate if headers containing multiple values should be encoded
	// as a comma separated list or as multiple headers.
	bool			encode_multi_values_as_list;
	
	// Indicate if a unique contact name should be created by using
	// the domain name: username_domain
	// If false then the SIP user name is used as contact name
	bool			use_domain_in_contact;
	
	// Allow SDP to change in different INVITE responses. 
	// According to RFC 3261 13.2.1, if SDP is received in a 1XX response,
	// then SDP received in subsequent responses should be ignored.
	// Some SIP proxies do send different SDP in 1XX and 200 though.
	// E.g. first SDP is to play ring tone, second SDP is to create
	// an end-to-end media path.
	bool			allow_sdp_change;

	// Redirections
	// Allow redirection of a request when a 3XX is received.
	bool			allow_redirection;

	// Ask user for permission to redirect a request when a 3XX is received.
	bool			ask_user_to_redirect;

	// Maximum number of locations to be tried when a request is redirected.
	unsigned short		max_redirections;

	// SIP extensions
	// 100rel extension (PRACK, RFC 3262)
	// Possible values:
	// - disabled	100rel extension is disabled
	// - supported	100rel is supported (it is added in the supported header of
	//               an outgoing INVITE). A far-end can now require a PRACK on a
	//               1xx response.
	// - required	100rel is required (it is put in the require header of an
	//               outgoing INVITE).
	//		If an incoming INVITE indicates that it supports 100rel, then
	//		Twinkle will require a PRACK when sending a 1xx response.
	// - preferred	Similar to required, but if a call fails because the far-end
	//		indicates it does not support 100rel (420 response) then the
	//		call will be re-attempted without the 100rel requirement.
	t_ext_support		ext_100rel;
	
	// Replaces (RFC 3891)
	bool			ext_replaces;
	//@}

	/** @name REFER options */
	//@{
	/** Hold the current call when an incoming REFER is accepted. */
	bool			referee_hold;

	/** Hold the current call before sending a REFER. */
	bool			referrer_hold;

	/** Allow an incoming refer */
	bool			allow_refer;

	/** Ask user for permission when a REFER is received. */
	bool			ask_user_to_refer;

	/** Referrer automatically refreshes subscription before expiry. */
	bool			auto_refresh_refer_sub;
	
	/**
	 * An attended transfer should use the contact-URI of the transfer target.
	 * This contact-URI is not always globally routable however. As an
	 * alternative the AoR (address of record) can be used. Disadvantage is
	 * that the AoR may route to multiple phones in case of forking, whereas
	 * the contact-URI routes to a particular phone.
	 */
	bool			attended_refer_to_aor;
	
	/** 
	 * Allow to transfer a call while the consultation call is still
	 * in progress.
	 */
	bool			allow_transfer_consultation_inprog;
	//@}
	
	/** @name Privacy options */
	//@{
	// Send P-Preferred-Identity header in initial INVITE when hiding
	// user identity.
	bool			send_p_preferred_id;
	//@}
	
	/** @name Transport */
	//@{
	/** SIP transport protocol */
	t_sip_transport		sip_transport;
	
	/** 
	 * Threshold to decide which transport to use in auto transport mode. 
	 * A message with a size up to this threshold is sent via UDP. Larger messages
	 * are sent via TCP.
	 */
	unsigned short		sip_transport_udp_threshold;
	//@}

	/** @name NAT */
	//@{
	/**
	 * NAT traversal
	 * You can set nat_public_ip to your public IP or FQDN if you are behind
	 * a NAT. This will then be used inside the SIP messages instead of your
	 * private IP. On your NAT you have to create static bindings for port 5060
	 * and ports 8000 - 8005 to the same ports on your private IP address.
	 */
	bool			use_nat_public_ip;
	
	/** The public IP address of the NAT device. */
	string			nat_public_ip;
	
	/** NAT traversal via STUN. */
	bool			use_stun;
	
	/** URL of the STUN server. */
	t_url			stun_server;
	
	/** User persistent TCP connections. */
	bool			persistent_tcp;
	
	/** Enable sending of NAT keepalive packets for UDP. */
	bool			enable_nat_keepalive;
	//@}

	/** @name TIMERS */
	//@{
	/** 
	 * Noanswer timer is started when an initial INVITE is received. If
	 * the user does not respond within the timer, then the call will be
	 * released with a 480 Temporarily Unavailable response.
	 */
	unsigned short		timer_noanswer; // seconds
	
	/** Duration of NAT keepalive timer (s) */
	unsigned short		timer_nat_keepalive;
	
	/** Duration of TCP ping timer (s) */
	unsigned short		timer_tcp_ping;
	//@}

	/** @name ADDRESS FORMAT */
	//@{
	/**
	 * Telephone numbers
	 * Display only the user-part of a URI if it is a telephone number
	 * I.e. the user=phone parameter is present, or the user indicated
	 * that the format of the user-part is a telephone number.
	 * If the URI is a tel-URI then display the telephone number.
	 */
	bool			display_useronly_phone;

	/**
	 * Consider user-parts that consist of 0-9,+,-,*,# as a telephone
	 * number. I.e. in outgoing messages the user=phone parameter will
	 * be added to the URI. For incoming messages the URI will be considered
	 * to be a telephone number regardless of the presence of the
	 * user=phone parameter.
	 */
	bool			numerical_user_is_phone;
	
	/** Remove special symbols from numerical dial strings */
	bool			remove_special_phone_symbols;
	
	/** Special symbols that must be removed from telephone numbers */
	string			special_phone_symbols;
	
	/**
	 * If the user enters a telephone number as address, then complete it
	 * to a tel-URI instead of a sip-URI.
	 */
	bool			use_tel_uri_for_phone;
	
	/** Number conversion */
	list<t_number_conversion>	number_conversions;
	//@}
	
	// RING TONES
	string		ringtone_file;
	string		ringback_file;
	
	// SCRIPTS
	// Script to be called on incoming call
	string		script_incoming_call;
	string		script_in_call_answered;
	string		script_in_call_failed;
	string		script_outgoing_call;
	string		script_out_call_answered;
	string		script_out_call_failed;
	string		script_local_release;
	string		script_remote_release;
	
	// SECURITY
	// zrtp setting
	bool		zrtp_enabled;
	
	// Popup warning when far-end sends goclear command
	bool		zrtp_goclear_warning;
	
	// Send a=zrtp in SDP
	bool		zrtp_sdp;
	
	// Only negotiate zrtp if far-end signalled support for zrtp
	bool		zrtp_send_if_supported;
	
	// MWI
	// Indicate if MWI is sollicited or unsollicited.
	// RFC 3842 specifies that MWI must be sollicited (SUBSCRIBE).
	// Asterisk however only supported non-standard unsollicited MWI.
	bool		mwi_sollicited;
	
	// User name for subscribing to the mailbox
	string		mwi_user;
	
	// The mailbox server to which the SUBSCRIBE must be sent
	t_url		mwi_server;
	
	// Send the SUBSCRIBE via the proxy to the mailbox server
	bool		mwi_via_proxy;
	
	// Requested MWI subscription duration
	unsigned long	mwi_subscription_time;
	
	// The voice mail address to call to access messages
	string		mwi_vm_address;
	
	/** @name INSTANT MESSAGE */
	//@{
	/** Maximum number of simultaneous IM sessions. */
	unsigned short	im_max_sessions;
	
	/** Flag to indicate that IM is-composing indications (RFC 3994) should be sent. */
	bool		im_send_iscomposing;
	//@}
	
	/** @name PRESENCE */
	//@{
	/** Requested presence subscription duration in seconds. */
	unsigned long	pres_subscription_time;
	
	/** Requested presence publication duration in seconds. */
	unsigned long	pres_publication_time;
	
	/** Publish online presence state at startup */
	bool		pres_publish_startup;
	//@}

	t_ext_support str2ext_support(const string &s) const;
	string ext_support2str(t_ext_support e) const;
	t_bit_rate_type str2bit_rate_type(const string &s) const;
	string bit_rate_type2str(t_bit_rate_type b) const;
	t_dtmf_transport str2dtmf_transport(const string &s) const;
	string dtmf_transport2str(t_dtmf_transport d) const;
	t_g726_packing str2g726_packing(const string &s) const;
	string g726_packing2str(t_g726_packing packing) const;
	t_sip_transport str2sip_transport(const string &s) const;
	string sip_transport2str(t_sip_transport transport) const;
	
	// Parse a number conversion rule
	// If the rule can be parsed, then c contains the conversion rule and
	// true is returned. Otherwise false is returned.
	bool parse_num_conversion(const string &value, t_number_conversion &c);
	
	// Set a server URL.
	// Returns false, if the passed value is not a valid URL.
	bool set_server_value(t_url &server, const string &scheme, const string &value);
	
public:
	t_user();
	t_user(const t_user &u);
	
	t_user *copy(void) const;
	
	/** @name Getters */
	//@{
	string get_name(void) const;
	string get_domain(void) const;
	string get_display(bool anonymous) const;	
	string get_organization(void) const;
	string get_auth_realm(void) const;
	string get_auth_name(void) const;
	string get_auth_pass(void) const;
	void get_auth_aka_op(uint8 *aka_op) const;
	void get_auth_aka_amf(uint8 *aka_amf) const;
	bool get_use_outbound_proxy(void) const;
	t_url get_outbound_proxy(void) const;
	bool get_all_requests_to_proxy(void) const;
	bool get_non_resolvable_to_proxy(void) const;
	bool get_use_registrar(void) const;
	t_url get_registrar(void) const;
	unsigned long get_registration_time(void) const;
	bool get_register_at_startup(void) const;
	bool get_reg_add_qvalue(void) const;
	float get_reg_qvalue(void) const;
	list<t_audio_codec> get_codecs(void) const;
	unsigned short get_ptime(void) const;
	bool get_out_obey_far_end_codec_pref(void) const;
	bool get_in_obey_far_end_codec_pref(void) const;
	unsigned short get_speex_nb_payload_type(void) const;
	unsigned short get_speex_wb_payload_type(void) const;
	unsigned short get_speex_uwb_payload_type(void) const;
	t_bit_rate_type get_speex_bit_rate_type(void) const;
	int get_speex_abr_nb(void) const;
	int get_speex_abr_wb(void) const;
	bool get_speex_dtx(void) const;
	bool get_speex_penh(void) const;
	unsigned short get_speex_quality(void) const;
	unsigned short get_speex_complexity(void) const;
	bool get_speex_dsp_vad(void) const;
	bool get_speex_dsp_agc(void) const;
	bool get_speex_dsp_aec(void) const;
	bool get_speex_dsp_nrd(void) const;
	unsigned short get_speex_dsp_agc_level(void) const;
	unsigned short get_ilbc_payload_type(void) const;
	unsigned short get_ilbc_mode(void) const;
	unsigned short get_g726_16_payload_type(void) const;
	unsigned short get_g726_24_payload_type(void) const;
	unsigned short get_g726_32_payload_type(void) const;
	unsigned short get_g726_40_payload_type(void) const;
	t_g726_packing get_g726_packing(void) const;
	t_dtmf_transport get_dtmf_transport(void) const;
	unsigned short get_dtmf_payload_type(void) const;
	unsigned short get_dtmf_duration(void) const;
	unsigned short get_dtmf_pause(void) const;
	unsigned short get_dtmf_volume(void) const;
	t_hold_variant get_hold_variant(void) const;
	bool get_check_max_forwards(void) const;
	bool get_allow_missing_contact_reg(void) const;
	bool get_registration_time_in_contact(void) const;
	bool get_compact_headers(void) const;
	bool get_encode_multi_values_as_list(void) const;
	bool get_use_domain_in_contact(void) const;
	bool get_allow_sdp_change(void) const;
	bool get_allow_redirection(void) const;
	bool get_ask_user_to_redirect(void) const;
	unsigned short get_max_redirections(void) const;
	t_ext_support get_ext_100rel(void) const;
	bool get_ext_replaces(void) const;
	bool get_referee_hold(void) const;
	bool get_referrer_hold(void) const;
	bool get_allow_refer(void) const;
	bool get_ask_user_to_refer(void) const;
	bool get_auto_refresh_refer_sub(void) const;
	bool get_attended_refer_to_aor(void) const;
	bool get_allow_transfer_consultation_inprog(void) const;
	bool get_send_p_preferred_id(void) const;
	t_sip_transport get_sip_transport(void) const;
	unsigned short get_sip_transport_udp_threshold(void) const;
	bool get_use_nat_public_ip(void) const;
	string get_nat_public_ip(void) const;
	bool get_use_stun(void) const;
	t_url get_stun_server(void) const;
	bool get_persistent_tcp(void) const;
	bool get_enable_nat_keepalive(void) const;
	unsigned short get_timer_noanswer(void) const;
	unsigned short get_timer_nat_keepalive(void) const; 
	unsigned short get_timer_tcp_ping(void) const;
	bool get_display_useronly_phone(void) const;
	bool get_numerical_user_is_phone(void) const;
	bool get_remove_special_phone_symbols(void) const;
	string get_special_phone_symbols(void) const;
	bool get_use_tel_uri_for_phone(void) const;
	string get_ringtone_file(void) const;
	string get_ringback_file(void) const;
	string get_script_incoming_call(void) const;
	string get_script_in_call_answered(void) const;
	string get_script_in_call_failed(void) const;
	string get_script_outgoing_call(void) const;
	string get_script_out_call_answered(void) const;
	string get_script_out_call_failed(void) const;
	string get_script_local_release(void) const;
	string get_script_remote_release(void) const;
	list<t_number_conversion> get_number_conversions(void) const;
	bool get_zrtp_enabled(void) const;
	bool get_zrtp_goclear_warning(void) const;
	bool get_zrtp_sdp(void) const;
	bool get_zrtp_send_if_supported(void) const;
	bool get_mwi_sollicited(void) const;
	string get_mwi_user(void) const;
	t_url get_mwi_server(void) const;
	bool get_mwi_via_proxy(void) const;
	unsigned long get_mwi_subscription_time(void) const;
	string get_mwi_vm_address(void) const;
	unsigned short get_im_max_sessions(void) const;
	bool get_im_send_iscomposing(void) const;
	unsigned long get_pres_subscription_time(void) const;
	unsigned long get_pres_publication_time(void) const;
	bool get_pres_publish_startup(void) const;
	//@}

	
	/** @name Setters */
	//@{
	void set_name(const string &_name);
	void set_domain(const string &_domain);
	void set_display(const string &_display);	
	void set_organization(const string &_organization);
	void set_auth_realm(const string &realm);
	void set_auth_name(const string &name);
	void set_auth_pass(const string &pass);
	void set_auth_aka_op(const uint8 *aka_op);
	void set_auth_aka_amf(const uint8 *aka_amf);
	void set_use_outbound_proxy(bool b);
	void set_outbound_proxy(const t_url &url);
	void set_all_requests_to_proxy(bool b);
	void set_non_resolvable_to_proxy(bool b);
	void set_use_registrar(bool b);
	void set_registrar(const t_url &url);
	void set_registration_time(const unsigned long time);
	void set_register_at_startup(bool b);
	void set_reg_add_qvalue(bool b);
	void set_reg_qvalue(float q);
	void set_codecs(const list<t_audio_codec> &_codecs);
	void set_ptime(unsigned short _ptime);
	void set_out_obey_far_end_codec_pref(bool b);
	void set_in_obey_far_end_codec_pref(bool b);
	void set_speex_nb_payload_type(unsigned short payload_type);
	void set_speex_wb_payload_type(unsigned short payload_type);
	void set_speex_uwb_payload_type(unsigned short payload_type);
	void set_speex_bit_rate_type(t_bit_rate_type bit_rate_type);
	void set_speex_abr_nb(int abr);
	void set_speex_abr_wb(int abr);
	void set_speex_dtx(bool b);
	void set_speex_penh(bool b);
	void set_speex_quality(unsigned short quality);
	void set_speex_complexity(unsigned short complexity);
	void set_speex_dsp_vad(bool b);
	void set_speex_dsp_agc(bool b);
	void set_speex_dsp_aec(bool b);
	void set_speex_dsp_nrd(bool b);
	void set_speex_dsp_agc_level(unsigned short level);
	void set_ilbc_payload_type(unsigned short payload_type);
	void set_g726_16_payload_type(unsigned short payload_type);
	void set_g726_24_payload_type(unsigned short payload_type);
	void set_g726_32_payload_type(unsigned short payload_type);
	void set_g726_40_payload_type(unsigned short payload_type);
	void set_g726_packing(t_g726_packing packing);
	void set_ilbc_mode(unsigned short mode);
	void set_dtmf_transport(t_dtmf_transport _dtmf_transport);
	void set_dtmf_payload_type(unsigned short payload_type);
	void set_dtmf_duration(unsigned short duration);
	void set_dtmf_pause(unsigned short pause);
	void set_dtmf_volume(unsigned short volume);
	void set_hold_variant(t_hold_variant _hold_variant);
	void set_check_max_forwards(bool b);
	void set_allow_missing_contact_reg(bool b);
	void set_registration_time_in_contact(bool b);
	void set_compact_headers(bool b);
	void set_encode_multi_values_as_list(bool b);
	void set_use_domain_in_contact(bool b);
	void set_allow_sdp_change(bool b);
	void set_allow_redirection(bool b);
	void set_ask_user_to_redirect(bool b);
	void set_max_redirections(unsigned short _max_redirections);
	void set_ext_100rel(t_ext_support ext_support);
	void set_ext_replaces(bool b);
	void set_referee_hold(bool b);
	void set_referrer_hold(bool b);
	void set_allow_refer(bool b);
	void set_ask_user_to_refer(bool b);
	void set_auto_refresh_refer_sub(bool b);
	void set_attended_refer_to_aor(bool b);
	void set_allow_transfer_consultation_inprog(bool b);
	void set_send_p_preferred_id(bool b);
	void set_sip_transport(t_sip_transport transport);
	void set_sip_transport_udp_threshold(unsigned short threshold);
	void set_use_nat_public_ip(bool b);
	void set_nat_public_ip(const string &public_ip);
	void set_use_stun(bool b);
	void set_stun_server(const t_url &url);
	void set_persistent_tcp(bool b);
	void set_enable_nat_keepalive(bool b);
	void set_timer_noanswer(unsigned short timer);
	void set_timer_nat_keepalive(unsigned short timer); 
	void set_timer_tcp_ping(unsigned short timer);
	void set_display_useronly_phone(bool b);
	void set_numerical_user_is_phone(bool b);
	void set_remove_special_phone_symbols(bool b);
	void set_special_phone_symbols(const string &symbols);
	void set_use_tel_uri_for_phone(bool b);
	void set_ringtone_file(const string &file);
	void set_ringback_file(const string &file);
	void set_script_incoming_call(const string &script);
	void set_script_in_call_answered(const string &script);
	void set_script_in_call_failed(const string &script);
	void set_script_outgoing_call(const string &script);
	void set_script_out_call_answered(const string &script);
	void set_script_out_call_failed(const string &script);
	void set_script_local_release(const string &script);
	void set_script_remote_release(const string &script);
	void set_number_conversions(const list<t_number_conversion> &l);
	void set_zrtp_enabled(bool b);
	void set_zrtp_goclear_warning(bool b);
	void set_zrtp_sdp(bool b);
	void set_zrtp_send_if_supported(bool b);
	void set_mwi_sollicited(bool b);
	void set_mwi_user(const string &user);
	void set_mwi_server(const t_url &url);
	void set_mwi_via_proxy(bool b);
	void set_mwi_subscription_time(unsigned long t);
	void set_mwi_vm_address(const string &address);
	void set_im_max_sessions(unsigned short max_sessions);
	void set_im_send_iscomposing(bool b);
	void set_pres_subscription_time(unsigned long t);
	void set_pres_publication_time(unsigned long t);
	void set_pres_publish_startup(bool b);
	//@}

	// Read and parse a config file into the user object.
	// Returns false if it fails. error_msg is an error message that can
	// be given to the user.
	bool read_config(const string &filename, string &error_msg);

	/**
	 * Write the settings into a config file.
	 * @param filename [in] Name of the file to write.
	 * @param error_msg [out] Human readable error message when writing fails.
	 * @return Returns true of writing succeeded, otherwise false.
	 */
	bool write_config(const string &filename, string &error_msg);
	
	/** Get the file name for this user profile */
	string get_filename(void) const;

	/** 
	 * Set a config file name.
	 * @return True if file name did not yet exist.
	 * @return False if file name already exists.
	 */
	bool set_config(string _filename);

	// Get the name of the profile (filename without extension)
	string get_profile_name(void) const;
	
	// Expand file name to a fully qualified file name
	string expand_filename(const string &filename);
	
	// The contact name is created from the name and domain values.
	// Just the name value is not unique when multiple user profiles are
	// activated.
	string get_contact_name(void) const;
	
	// Returns "display <sip:name@domain>"
	string get_display_uri(void) const;
	
	// Check if all required extensions are supported
	bool check_required_ext(t_request *r, list<string> &unsupported) const;
	
	/**
	 * Create contact URI.
	 * @param anonymous [in] Indicates if an anonymous contact should be created.
	 * @param auto_ip [in] Automatically determined local IP address that should be
	 *                     used if not IP address has been determined through other means.
	 * @return String representation of the contact URI.
	 */
	string create_user_contact(bool anonymous, const string &auto_ip);
	
	// Create user uri
	string create_user_uri(bool anonymous);
	
	// Convert a number by applying the number conversions.
	string convert_number(const string &number, const list<t_number_conversion> &l) const;
	string convert_number(const string &number) const;
	
	// Get URI for sending a SUBSCRIBE for MWI
	t_url get_mwi_uri(void) const;
	
	/** Is this a user profile for a Diamondcard account? */
	bool is_diamondcard_account(void) const;
};

#endif
