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
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <list>
#include "diamondcard.h"
#include "log.h"
#include "phone.h"
#include "twinkle_config.h"
#include "user.h"
#include "userintf.h"
#include "util.h"
#include "protocol.h"
#include "sys_settings.h"
#include "audits/memman.h"
#include "sdp/sdp.h"
#include "parser/parse_ctrl.h"
#include "parser/request.h"

extern t_phone		*phone;

// Field names in the config file
// USER fields
#define FLD_NAME			"user_name"
#define FLD_DOMAIN			"user_domain"
#define FLD_DISPLAY			"user_display"
#define	FLD_ORGANIZATION		"user_organization"
#define FLD_AUTH_REALM			"auth_realm"
#define FLD_AUTH_NAME			"auth_name"
#define FLD_AUTH_PASS			"auth_pass"
#define FLD_AUTH_AKA_OP			"auth_aka_op"
#define FLD_AUTH_AKA_AMF		"auth_aka_amf"

// SIP SERVER fields
#define FLD_OUTBOUND_PROXY		"outbound_proxy"
#define FLD_ALL_REQUESTS_TO_PROXY	"all_requests_to_proxy"
#define FLD_NON_RESOLVABLE_TO_PROXY	"non_resolvable_to_proxy"
#define FLD_REGISTRAR			"registrar"
#define FLD_REGISTRATION_TIME		"registration_time"
#define FLD_REGISTER_AT_STARTUP		"register_at_startup"
#define FLD_REG_ADD_QVALUE		"reg_add_qvalue"
#define FLD_REG_QVALUE			"reg_qvalue"

// AUDIO fields
#define FLD_CODECS			"codecs"
#define FLD_PTIME			"ptime"
#define FLD_OUT_FAR_END_CODEC_PREF	"out_far_end_codec_pref"
#define FLD_IN_FAR_END_CODEC_PREF	"in_far_end_codec_pref"
#define FLD_SPEEX_NB_PAYLOAD_TYPE	"speex_nb_payload_type"
#define FLD_SPEEX_WB_PAYLOAD_TYPE	"speex_wb_payload_type"
#define FLD_SPEEX_UWB_PAYLOAD_TYPE	"speex_uwb_payload_type"
#define FLD_SPEEX_BIT_RATE_TYPE		"speex_bit_rate_type"
#define FLD_SPEEX_ABR_NB		"speex_abr_nb"
#define FLD_SPEEX_ABR_WB		"speex_abr_wb"
#define FLD_SPEEX_DTX			"speex_dtx"
#define FLD_SPEEX_PENH			"speex_penh"
#define FLD_SPEEX_QUALITY		"speex_quality"
#define FLD_SPEEX_COMPLEXITY		"speex_complexity"
#define FLD_SPEEX_DSP_VAD		"speex_dsp_vad"
#define FLD_SPEEX_DSP_AGC		"speex_dsp_agc"
#define FLD_SPEEX_DSP_AGC_LEVEL		"speex_dsp_agc_level"
#define FLD_SPEEX_DSP_AEC		"speex_dsp_aec"
#define FLD_SPEEX_DSP_NRD		"speex_dsp_nrd"
#define FLD_ILBC_PAYLOAD_TYPE		"ilbc_payload_type"
#define FLD_ILBC_MODE			"ilbc_mode"
#define FLD_G726_16_PAYLOAD_TYPE	"g726_16_payload_type"
#define FLD_G726_24_PAYLOAD_TYPE	"g726_24_payload_type"
#define FLD_G726_32_PAYLOAD_TYPE	"g726_32_payload_type"
#define FLD_G726_40_PAYLOAD_TYPE	"g726_40_payload_type"
#define FLD_G726_PACKING		"g726_packing"
#define FLD_DTMF_TRANSPORT		"dtmf_transport"
#define FLD_DTMF_PAYLOAD_TYPE		"dtmf_payload_type"
#define FLD_DTMF_DURATION		"dtmf_duration"
#define FLD_DTMF_PAUSE			"dtmf_pause"
#define FLD_DTMF_VOLUME			"dtmf_volume"

// SIP PROTOCOL fields
#define FLD_HOLD_VARIANT		"hold_variant"
#define FLD_CHECK_MAX_FORWARDS		"check_max_forwards"
#define FLD_ALLOW_MISSING_CONTACT_REG	"allow_missing_contact_reg"	
#define FLD_REGISTRATION_TIME_IN_CONTACT	"registration_time_in_contact"
#define FLD_COMPACT_HEADERS		"compact_headers"
#define FLD_ENCODE_MULTI_VALUES_AS_LIST	"encode_multi_values_as_list"
#define FLD_USE_DOMAIN_IN_CONTACT	"use_domain_in_contact"
#define FLD_ALLOW_SDP_CHANGE		"allow_sdp_change"
#define FLD_ALLOW_REDIRECTION		"allow_redirection"
#define FLD_ASK_USER_TO_REDIRECT	"ask_user_to_redirect"
#define FLD_MAX_REDIRECTIONS		"max_redirections"
#define FLD_EXT_100REL			"ext_100rel"
#define FLD_EXT_REPLACES		"ext_replaces"
#define FLD_REFEREE_HOLD		"referee_hold"
#define FLD_REFERRER_HOLD		"referrer_hold"
#define FLD_ALLOW_REFER			"allow_refer"
#define FLD_ASK_USER_TO_REFER		"ask_user_to_refer"
#define FLD_AUTO_REFRESH_REFER_SUB	"auto_refresh_refer_sub"
#define FLD_ATTENDED_REFER_TO_AOR	"attended_refer_to_aor"
#define FLD_ALLOW_XFER_CONSULT_INPROG	"allow_xfer_consult_inprog"
#define FLD_SEND_P_PREFERRED_ID		"send_p_preferred_id"

// Transport/NAT fields
#define FLD_SIP_TRANSPORT		"sip_transport"
#define FLD_SIP_TRANSPORT_UDP_THRESHOLD	"sip_transport_udp_threshold"
#define FLD_NAT_PUBLIC_IP		"nat_public_ip"
#define FLD_STUN_SERVER			"stun_server"
#define FLD_PERSISTENT_TCP		"persistent_tcp"
#define FLD_ENABLE_NAT_KEEPALIVE	"enable_nat_keepalive"

// TIMER fields
#define FLD_TIMER_NOANSWER		"timer_noanswer"
#define FLD_TIMER_NAT_KEEPALIVE		"timer_nat_keepalive"
#define FLD_TIMER_TCP_PING		"timer_tcp_ping"

// ADDRESS FORMAT fields
#define FLD_DISPLAY_USERONLY_PHONE	"display_useronly_phone"
#define FLD_NUMERICAL_USER_IS_PHONE	"numerical_user_is_phone"
#define FLD_REMOVE_SPECIAL_PHONE_SYM	"remove_special_phone_symbols"
#define FLD_SPECIAL_PHONE_SYMBOLS	"special_phone_symbols"
#define FLD_USE_TEL_URI_FOR_PHONE	"use_tel_uri_for_phone"

// Ring tone settings
#define FLD_USER_RINGTONE_FILE		"ringtone_file"
#define FLD_USER_RINGBACK_FILE		"ringback_file"

// Incoming call script
#define FLD_SCRIPT_INCOMING_CALL	"script_incoming_call"
#define FLD_SCRIPT_IN_CALL_ANSWERED	"script_in_call_answered"
#define FLD_SCRIPT_IN_CALL_FAILED	"script_in_call_failed"
#define FLD_SCRIPT_OUTGOING_CALL	"script_outgoing_call"
#define FLD_SCRIPT_OUT_CALL_ANSWERED	"script_out_call_answered"
#define FLD_SCRIPT_OUT_CALL_FAILED	"script_out_call_failed"
#define FLD_SCRIPT_LOCAL_RELEASE	"script_local_release"
#define FLD_SCRIPT_REMOTE_RELEASE	"script_remote_release"

// Number conversion
#define FLD_NUMBER_CONVERSION		"number_conversion"

// Security
#define FLD_ZRTP_ENABLED		"zrtp_enabled"
#define FLD_ZRTP_GOCLEAR_WARNING	"zrtp_goclear_warning"
#define FLD_ZRTP_SDP			"zrtp_sdp"
#define FLD_ZRTP_SEND_IF_SUPPORTED	"zrtp_send_if_supported"

// MWI
#define FLD_MWI_SOLLICITED		"mwi_sollicited"
#define FLD_MWI_USER			"mwi_user"
#define FLD_MWI_SERVER			"mwi_server"
#define FLD_MWI_VIA_PROXY		"mwi_via_proxy"
#define FLD_MWI_SUBSCRIPTION_TIME	"mwi_subscription_time"
#define FLD_MWI_VM_ADDRESS		"mwi_vm_address"

// INSTANT MESSAGE
#define FLD_IM_MAX_SESSIONS		"im_max_sessions"
#define FLD_IM_SEND_ISCOMPOSING		"im_send_iscomposing"

// PRESENCE
#define FLD_PRES_SUBSCRIPTION_TIME	"pres_subscription_time"
#define FLD_PRES_PUBLICATION_TIME	"pres_publication_time"
#define FLD_PRES_PUBLISH_STARTUP	"pres_publish_startup"

/////////////////////////
// class t_user
/////////////////////////

////////////////////
// Private
////////////////////

t_ext_support t_user::str2ext_support(const string &s) const {
	if (s == "disabled") return EXT_DISABLED;
	if (s == "supported") return EXT_SUPPORTED;
	if (s == "preferred") return EXT_PREFERRED;
	if (s == "required") return EXT_REQUIRED;
	return EXT_INVALID;
}

string t_user::ext_support2str(t_ext_support e) const {
	switch(e) {
	case EXT_INVALID:	return "invalid";
	case EXT_DISABLED:	return "disabled";
	case EXT_SUPPORTED:	return "supported";
	case EXT_PREFERRED:	return "preferred";
	case EXT_REQUIRED:	return "required";
	default:
		assert(false);
	}

	return "";
}

t_bit_rate_type t_user::str2bit_rate_type(const string &s) const {
	if (s == "cbr") return BIT_RATE_CBR;
	if (s == "vbr") return BIT_RATE_VBR;
	if (s == "abr") return BIT_RATE_ABR;
	return BIT_RATE_INVALID;
}

string t_user::bit_rate_type2str(t_bit_rate_type b) const {
	switch (b) {
	case BIT_RATE_INVALID:	return "invalid";
	case BIT_RATE_CBR:	return "cbr";
	case BIT_RATE_VBR:	return "vbr";
	case BIT_RATE_ABR:	return "abr";
	default:
		assert(false);
	}
}

t_dtmf_transport t_user::str2dtmf_transport(const string &s) const {
	if (s == "inband") return DTMF_INBAND;
	if (s == "rfc2833") return DTMF_RFC2833;
	if (s == "auto") return DTMF_AUTO;
	if (s == "info") return DTMF_INFO;
	return DTMF_AUTO;
}

string t_user::dtmf_transport2str(t_dtmf_transport d) const {
	switch (d) {
	case DTMF_INBAND:	return "inband";
	case DTMF_RFC2833:	return "rfc2833";
	case DTMF_AUTO:		return "auto";
	case DTMF_INFO:		return "info";
	default:
		assert(false);
	}
}

t_g726_packing t_user::str2g726_packing(const string &s) const {
	if (s == "rfc3551") return G726_PACK_RFC3551;
	if (s == "aal2") return G726_PACK_AAL2;
	return G726_PACK_AAL2;
}

string t_user::g726_packing2str(t_g726_packing packing) const {
	switch (packing) {
	case G726_PACK_RFC3551:	return "rfc3551";
	case G726_PACK_AAL2:	return "aal2";
	default:
		assert(false);
	}
}

t_sip_transport t_user::str2sip_transport(const string &s) const {
	if (s == "udp") return SIP_TRANS_UDP;
	if (s == "tcp") return SIP_TRANS_TCP;
	if (s == "auto") return SIP_TRANS_AUTO;
	return SIP_TRANS_AUTO;
}

string t_user::sip_transport2str(t_sip_transport transport) const {
	switch (transport) {
	case SIP_TRANS_UDP:	return "udp";
	case SIP_TRANS_TCP:	return "tcp";
	case SIP_TRANS_AUTO:	return "auto";
	default:
		assert(false);
	}
}

string t_user::expand_filename(const string &filename) {
	string f;

	if (filename[0] == '/') {
		f = filename;
	} else {
        	f = string(DIR_HOME);
        	f += "/";
        	f += USER_DIR;
        	f += "/";
        	f += filename;
	}

	return f;
}

bool t_user::parse_num_conversion(const string &value, t_number_conversion &c) {
	vector<string> l = split_escaped(value, ',');
	
	if (l.size() != 2) {
		// Invalid conversion rule
		return false;
	}
	
	try {
		c.re.assign(l[0]);
		c.fmt = l[1];
	} catch (boost::bad_expression) {
		// Invalid regular expression
		log_file->write_header("t_user::parse_num_conversion", 
				LOG_NORMAL, LOG_WARNING);
		log_file->write_raw("Bad number conversion:\n");
		log_file->write_raw(l.front());
		log_file->write_raw(" --> ");
		log_file->write_raw(l.back());
		log_file->write_endl();
		log_file->write_footer();
		
		return false;
	}
	
	return true;
}

bool t_user::set_server_value(t_url &server, const string &scheme, const string &value) {
	if (value.empty()) {
		server.set_url("");
		return false;
	}

	string s = scheme + ":" + value;
	server.set_url(s);

	if (!server.is_valid() || server.get_user() != "")
	{
		string err_msg = "Invalid server value: ";
		err_msg += value;
		log_file->write_report(err_msg, "t_user::set_server_value",
			LOG_NORMAL, LOG_WARNING);
		server.set_url("");
		return false;
	}
	
	return true;
}


////////////////////
// Public
////////////////////

t_user::t_user() {
	// Set defaults
	memset(auth_aka_op, 0, AKA_OPLEN);
	memset(auth_aka_amf, 0, AKA_AMFLEN);
	use_outbound_proxy = false;
	all_requests_to_proxy = false;
	non_resolvable_to_proxy = false;
	use_registrar = false;
	registration_time = 3600;
#ifdef HAVE_SPEEX
	codecs.push_back(CODEC_SPEEX_WB);
	codecs.push_back(CODEC_SPEEX_NB);
#endif
#ifdef HAVE_ILBC
	codecs.push_back(CODEC_ILBC);
#endif
	codecs.push_back(CODEC_G711_ALAW);
	codecs.push_back(CODEC_G711_ULAW);
	codecs.push_back(CODEC_GSM);
	ptime = 20;
	out_obey_far_end_codec_pref = true;
	in_obey_far_end_codec_pref = true;
	hold_variant = HOLD_RFC3264;
	use_nat_public_ip = false;
	use_stun = false;
	persistent_tcp = true;
	enable_nat_keepalive = false;
	register_at_startup = true;
	reg_add_qvalue = false;
	reg_qvalue = 1.0;
	check_max_forwards = false;
	allow_missing_contact_reg = true;
	compact_headers = false;
	encode_multi_values_as_list = true;
	registration_time_in_contact = true;
	use_domain_in_contact = false;
	allow_sdp_change = false;
	allow_redirection = true;
	ask_user_to_redirect = true;
	max_redirections = 5;
	timer_noanswer = 30;
	timer_nat_keepalive = DUR_NAT_KEEPALIVE;
	timer_tcp_ping = DUR_TCP_PING;
	ext_100rel = EXT_SUPPORTED;
	ext_replaces = true;
	speex_nb_payload_type = 97;
	speex_wb_payload_type = 98;
	speex_uwb_payload_type = 99;
	speex_bit_rate_type = BIT_RATE_CBR;
	speex_abr_nb = 0;
	speex_abr_wb = 0;
	speex_dtx = false;
	speex_penh = true;
	speex_quality = 6;
	speex_complexity = 3;
	speex_dsp_vad = true;
	speex_dsp_agc = true;
	speex_dsp_aec = false;
	speex_dsp_nrd = true;
	speex_dsp_agc_level = 20;
	ilbc_payload_type = 96;
	ilbc_mode = 30;
	g726_16_payload_type = 102;
	g726_24_payload_type = 103;
	g726_32_payload_type = 104;
	g726_40_payload_type = 105;
	g726_packing = G726_PACK_RFC3551;
	dtmf_transport = DTMF_AUTO;
	dtmf_duration = 100;
	dtmf_pause = 40;
	dtmf_payload_type = 101;
	dtmf_volume = 10;
	display_useronly_phone = true;
	numerical_user_is_phone = false;
	remove_special_phone_symbols = true;
	special_phone_symbols = SPECIAL_PHONE_SYMBOLS;
	use_tel_uri_for_phone = false;
	referee_hold = false;
	referrer_hold = true;
	allow_refer = true;
	ask_user_to_refer = true;
	auto_refresh_refer_sub = false;
	attended_refer_to_aor = false;
	allow_transfer_consultation_inprog = false;
	send_p_preferred_id = false;
	sip_transport = SIP_TRANS_AUTO;
	sip_transport_udp_threshold = 1300; // RFC 3261 18.1.1
	ringtone_file.clear();
	ringback_file.clear();
	script_incoming_call.clear();
	script_in_call_answered.clear();
	script_in_call_failed.clear();
	script_outgoing_call.clear();
	script_out_call_answered.clear();
	script_out_call_failed.clear();
	script_local_release.clear();
	script_remote_release.clear();
	number_conversions.clear();
	zrtp_enabled = false;
	zrtp_goclear_warning = true;
	zrtp_sdp = true;
	zrtp_send_if_supported = false;
	mwi_sollicited = false;
	mwi_user.clear();
	mwi_via_proxy = false;
	mwi_subscription_time = 3600;
	mwi_vm_address.clear();
	im_max_sessions = 10;
	im_send_iscomposing = true;
	pres_subscription_time = 3600;
	pres_publication_time = 3600;
	pres_publish_startup = true;
}

t_user::t_user(const t_user &u) {
	u.mtx_user.lock();

	config_filename = u.config_filename;
	name = u.name;
	domain = u.domain;
	display = u.display;	
	organization = u.organization;
	auth_realm = u.auth_realm;
	auth_name = u.auth_name;
	auth_pass = u.auth_pass;
	memcpy(auth_aka_op, u.auth_aka_op, AKA_OPLEN);
	memcpy(auth_aka_amf, u.auth_aka_amf, AKA_AMFLEN);
	use_outbound_proxy = u.use_outbound_proxy;
	outbound_proxy = u.outbound_proxy;
	all_requests_to_proxy = u.all_requests_to_proxy;
	non_resolvable_to_proxy = u.non_resolvable_to_proxy;
	use_registrar = u.use_registrar;
	reg_add_qvalue = u.reg_add_qvalue;
	reg_qvalue = u.reg_qvalue;
	registrar = u.registrar;
	registration_time = u.registration_time;
	register_at_startup = u.register_at_startup;
	codecs = u.codecs;
	ptime = u.ptime;
	out_obey_far_end_codec_pref = u.out_obey_far_end_codec_pref;
	in_obey_far_end_codec_pref = u.in_obey_far_end_codec_pref;
	speex_nb_payload_type = u.speex_nb_payload_type;
	speex_wb_payload_type = u.speex_wb_payload_type;
	speex_uwb_payload_type = u.speex_uwb_payload_type;
	speex_bit_rate_type = u.speex_bit_rate_type;
	speex_abr_nb = u.speex_abr_nb;
	speex_abr_wb = u.speex_abr_wb;
	speex_dtx = u.speex_dtx;
	speex_penh = u.speex_penh;
	speex_quality = u.speex_quality;
	speex_complexity = u.speex_complexity;
	speex_dsp_vad = u.speex_dsp_vad;
	speex_dsp_agc = u.speex_dsp_agc;
	speex_dsp_agc_level = u.speex_dsp_agc_level;
	speex_dsp_aec = u.speex_dsp_aec;
	speex_dsp_nrd = u.speex_dsp_nrd;
	ilbc_payload_type = u.ilbc_payload_type;
	ilbc_mode = u.ilbc_mode;
	g726_16_payload_type = u.g726_16_payload_type;
	g726_24_payload_type = u.g726_24_payload_type;
	g726_32_payload_type = u.g726_32_payload_type;
	g726_40_payload_type = u.g726_40_payload_type;
	g726_packing = u.g726_packing;
	dtmf_transport = u.dtmf_transport;
	dtmf_payload_type = u.dtmf_payload_type;
	dtmf_duration = u.dtmf_duration;
	dtmf_pause = u.dtmf_pause;
	dtmf_volume = u.dtmf_volume;
	hold_variant = u.hold_variant;
	check_max_forwards = u.check_max_forwards;
	allow_missing_contact_reg = u.allow_missing_contact_reg;
	registration_time_in_contact = u.registration_time_in_contact;
	compact_headers = u.compact_headers;
	encode_multi_values_as_list = u.encode_multi_values_as_list;
	use_domain_in_contact = u.use_domain_in_contact;
	allow_sdp_change = u.allow_sdp_change;
	allow_redirection = u.allow_redirection;
	ask_user_to_redirect = u.ask_user_to_redirect;
	max_redirections = u.max_redirections;
	ext_100rel = u.ext_100rel;
	ext_replaces = u.ext_replaces;
	referee_hold = u.referee_hold;
	referrer_hold = u.referrer_hold;
	allow_refer = u.allow_refer;
	ask_user_to_refer = u.ask_user_to_refer;
	auto_refresh_refer_sub = u.auto_refresh_refer_sub;
	attended_refer_to_aor = u.attended_refer_to_aor;
	allow_transfer_consultation_inprog = u.allow_transfer_consultation_inprog;
	send_p_preferred_id = u.send_p_preferred_id;
	sip_transport = u.sip_transport;
	sip_transport_udp_threshold = u.sip_transport_udp_threshold;
	use_nat_public_ip = u.use_nat_public_ip;
	nat_public_ip = u.nat_public_ip;
	use_stun = u.use_stun;
	stun_server = u.stun_server;
	persistent_tcp = u.persistent_tcp;
	enable_nat_keepalive = u.enable_nat_keepalive;
	timer_noanswer = u.timer_noanswer;
	timer_nat_keepalive = u.timer_nat_keepalive; 
	timer_tcp_ping = u.timer_tcp_ping;
	display_useronly_phone = u.display_useronly_phone;
	numerical_user_is_phone = u.numerical_user_is_phone;
	remove_special_phone_symbols = u.remove_special_phone_symbols;
	special_phone_symbols = u.special_phone_symbols;
	use_tel_uri_for_phone = u.use_tel_uri_for_phone;
	ringtone_file = u.ringtone_file;
	ringback_file = u.ringback_file;
	script_incoming_call = u.script_incoming_call;
	script_in_call_answered = u.script_in_call_answered;
	script_in_call_failed = u.script_in_call_failed;
	script_outgoing_call = u.script_outgoing_call;
	script_out_call_answered = u.script_out_call_answered;
	script_out_call_failed = u.script_out_call_failed;
	script_local_release = u.script_local_release;
	script_remote_release = u.script_remote_release;
	number_conversions = u.number_conversions;
	zrtp_enabled = u.zrtp_enabled;
	zrtp_goclear_warning = u.zrtp_goclear_warning;
	zrtp_sdp = u.zrtp_sdp;
	zrtp_send_if_supported = u.zrtp_send_if_supported;
	mwi_sollicited = u.mwi_sollicited;
	mwi_user = u.mwi_user;
	mwi_server = u.mwi_server;
	mwi_via_proxy = u.mwi_via_proxy;
	mwi_subscription_time = u.mwi_subscription_time;
	mwi_vm_address = u.mwi_vm_address;
	im_max_sessions = u.im_max_sessions;
	im_send_iscomposing = u.im_send_iscomposing;
	pres_subscription_time = u.pres_subscription_time;
	pres_publication_time = u.pres_publication_time;
	pres_publish_startup = u.pres_publish_startup;
	
	u.mtx_user.unlock();
}

t_user *t_user::copy(void) const {
	t_user *u = new t_user(*this);
	MEMMAN_NEW(u);
	return u;
}

string t_user::get_name(void) const {
	string result;
	mtx_user.lock();
	result = name;
	mtx_user.unlock();
	return result;
}

string t_user::get_domain(void) const {
	string result;
	mtx_user.lock();
	result = domain;
	mtx_user.unlock();
	return result;
}

string t_user::get_display(bool anonymous) const {
	if (anonymous) return ANONYMOUS_DISPLAY;

	string result;
	mtx_user.lock();
	result = display;
	mtx_user.unlock();
	return result;
}
	
string t_user::get_organization(void) const {
	string result;
	mtx_user.lock();
	result = organization;
	mtx_user.unlock();
	return result;
}

string t_user::get_auth_realm(void) const {
	string result;
	mtx_user.lock();
	result = auth_realm;
	mtx_user.unlock();
	return result;
}

string t_user::get_auth_name(void) const {
	string result;
	mtx_user.lock();
	result = auth_name;
	mtx_user.unlock();
	return result;
}

string t_user::get_auth_pass(void) const {
	string result;
	mtx_user.lock();
	result = auth_pass;
	mtx_user.unlock();
	return result;
}

void t_user::get_auth_aka_op(uint8 *aka_op) const {
	t_mutex_guard guard(mtx_user);
	memcpy(aka_op, auth_aka_op, AKA_OPLEN);
}
	
void t_user::get_auth_aka_amf(uint8 *aka_amf) const {
	t_mutex_guard guard(mtx_user);
	memcpy(aka_amf, auth_aka_amf, AKA_AMFLEN);
}

bool t_user::get_use_outbound_proxy(void) const {
	bool result;
	mtx_user.lock();
	result = use_outbound_proxy;
	mtx_user.unlock();
	return result;
}

t_url t_user::get_outbound_proxy(void) const {
	t_url result;
	mtx_user.lock();
	result = outbound_proxy;
	mtx_user.unlock();
	return result;
}

bool t_user::get_all_requests_to_proxy(void) const {
	bool result;
	mtx_user.lock();
	result = all_requests_to_proxy;
	mtx_user.unlock();
	return result;
}

bool t_user::get_non_resolvable_to_proxy(void) const {
	bool result;
	mtx_user.lock();
	result = non_resolvable_to_proxy;
	mtx_user.unlock();
	return result;
}

bool t_user::get_use_registrar(void) const {
	bool result;
	mtx_user.lock();
	result = use_registrar;
	mtx_user.unlock();
	return result;
}

t_url t_user::get_registrar(void) const {
	t_url result;
	mtx_user.lock();
	result = registrar;
	mtx_user.unlock();
	return result;
}

unsigned long t_user::get_registration_time(void) const {
	unsigned long result;
	mtx_user.lock();
	result = registration_time;
	mtx_user.unlock();
	return result;
}

bool t_user::get_register_at_startup(void) const {
	bool result;
	mtx_user.lock();
	result = register_at_startup;
	mtx_user.unlock();
	return result;
}

bool t_user::get_reg_add_qvalue(void) const {
	bool result;
	mtx_user.lock();
	result = reg_add_qvalue;
	mtx_user.unlock();
	return result;
}

float t_user::get_reg_qvalue(void) const {
	float result;
	mtx_user.lock();
	result = reg_qvalue;
	mtx_user.unlock();
	return result;
}

list<t_audio_codec> t_user::get_codecs(void) const {
	list<t_audio_codec> result;
	mtx_user.lock();
	result = codecs;
	mtx_user.unlock();
	return result;
}

unsigned short t_user::get_ptime(void) const {
	unsigned short result;
	mtx_user.lock();
	result = ptime;
	mtx_user.unlock();
	return result;
}

bool t_user::get_out_obey_far_end_codec_pref(void) const {
	bool result;
	mtx_user.lock();
	result = out_obey_far_end_codec_pref;
	mtx_user.unlock();
	return result;
}

bool t_user::get_in_obey_far_end_codec_pref(void) const {
	bool result;
	mtx_user.lock();
	result = in_obey_far_end_codec_pref;
	mtx_user.unlock();
	return result;
}

unsigned short t_user::get_speex_nb_payload_type(void) const {
	unsigned short result;
	mtx_user.lock();
	result = speex_nb_payload_type;
	mtx_user.unlock();
	return result;
}

unsigned short t_user::get_speex_wb_payload_type(void) const {
	unsigned short result;
	mtx_user.lock();
	result = speex_wb_payload_type;
	mtx_user.unlock();
	return result;
}

unsigned short t_user::get_speex_uwb_payload_type(void) const {
	unsigned short result;
	mtx_user.lock();
	result = speex_uwb_payload_type;
	mtx_user.unlock();
	return result;
}

t_bit_rate_type t_user::get_speex_bit_rate_type(void) const {
	t_bit_rate_type result;
	mtx_user.lock();
	result = speex_bit_rate_type;
	mtx_user.unlock();
	return result;
}

int t_user::get_speex_abr_nb(void) const {
	int result;
	mtx_user.lock();
	result = speex_abr_nb;
	mtx_user.unlock();
	return result;
}

int t_user::get_speex_abr_wb(void) const {
	int result;
	mtx_user.lock();
	result = speex_abr_wb;
	mtx_user.unlock();
	return result;
}

bool t_user::get_speex_dtx(void) const {
	bool result;
	mtx_user.lock();
	result = speex_dtx;
	mtx_user.unlock();
	return result;
}

bool t_user::get_speex_penh(void) const {
	bool result;
	mtx_user.lock();
	result = speex_penh;
	mtx_user.unlock();
	return result;
}

unsigned short t_user::get_speex_quality(void) const {
	unsigned short result;
	mtx_user.lock();
	result = speex_quality;
	mtx_user.unlock();
	return result;
}

unsigned short t_user::get_speex_complexity(void) const {
	unsigned short result;
	mtx_user.lock();
	result = speex_complexity;
	mtx_user.unlock();
	return result;
}

bool t_user::get_speex_dsp_vad(void) const {
	bool result;
	mtx_user.lock();
	result = speex_dsp_vad;
	mtx_user.unlock();
	return result;
}

bool t_user::get_speex_dsp_agc(void) const {
	bool result;
	mtx_user.lock();
	result = speex_dsp_agc;
	mtx_user.unlock();
	return result;
}

unsigned short t_user::get_speex_dsp_agc_level(void) const {
	unsigned short result;
	mtx_user.lock();
	result = speex_dsp_agc_level;
	mtx_user.unlock();
	return result;
}

bool t_user::get_speex_dsp_aec(void) const {
	bool result;
	mtx_user.lock();
	result = speex_dsp_aec;
	mtx_user.unlock();
	return result;
}

bool t_user::get_speex_dsp_nrd(void) const {
	bool result;
	mtx_user.lock();
	result = speex_dsp_nrd;
	mtx_user.unlock();
	return result;
}

unsigned short t_user::get_ilbc_payload_type(void) const {
	unsigned short result;
	mtx_user.lock();
	result = ilbc_payload_type;
	mtx_user.unlock();
	return result;
}

unsigned short t_user::get_ilbc_mode(void) const {
	unsigned short result;
	mtx_user.lock();
	result = ilbc_mode;
	mtx_user.unlock();
	return result;
}

unsigned short t_user::get_g726_16_payload_type(void) const {
	unsigned short result;
	mtx_user.lock();
	result = g726_16_payload_type;
	mtx_user.unlock();
	return result;
}

unsigned short t_user::get_g726_24_payload_type(void) const {
	unsigned short result;
	mtx_user.lock();
	result = g726_24_payload_type;
	mtx_user.unlock();
	return result;
}

unsigned short t_user::get_g726_32_payload_type(void) const {
	unsigned short result;
	mtx_user.lock();
	result = g726_32_payload_type;
	mtx_user.unlock();
	return result;
}

unsigned short t_user::get_g726_40_payload_type(void) const {
	unsigned short result;
	mtx_user.lock();
	result = g726_40_payload_type;
	mtx_user.unlock();
	return result;
}

t_g726_packing t_user::get_g726_packing(void) const {
	t_g726_packing result;
	mtx_user.lock();
	result = g726_packing;
	mtx_user.unlock();
	return result;
}

t_dtmf_transport t_user::get_dtmf_transport(void) const {
	t_dtmf_transport result;
	mtx_user.lock();
	result = dtmf_transport;
	mtx_user.unlock();
	return result;
}

unsigned short t_user::get_dtmf_payload_type(void) const {
	unsigned short result;
	mtx_user.lock();
	result = dtmf_payload_type;
	mtx_user.unlock();
	return result;
}

unsigned short t_user::get_dtmf_duration(void) const {
	unsigned short result;
	mtx_user.lock();
	result = dtmf_duration;
	mtx_user.unlock();
	return result;
}

unsigned short t_user::get_dtmf_pause(void) const {
	unsigned short result;
	mtx_user.lock();
	result = dtmf_pause;
	mtx_user.unlock();
	return result;
}

unsigned short t_user::get_dtmf_volume(void) const {
	unsigned short result;
	mtx_user.lock();
	result = dtmf_volume;
	mtx_user.unlock();
	return result;
}

t_hold_variant t_user::get_hold_variant(void) const {
	t_hold_variant result;
	mtx_user.lock();
	result = hold_variant;
	mtx_user.unlock();
	return result;
}

bool t_user::get_check_max_forwards(void) const {
	bool result;
	mtx_user.lock();
	result = check_max_forwards;
	mtx_user.unlock();
	return result;
}

bool t_user::get_allow_missing_contact_reg(void) const {
	bool result;
	mtx_user.lock();
	result = allow_missing_contact_reg;
	mtx_user.unlock();
	return result;
}

bool t_user::get_registration_time_in_contact(void) const {
	bool result;
	mtx_user.lock();
	result = registration_time_in_contact;
	mtx_user.unlock();
	return result;
}

bool t_user::get_compact_headers(void) const {
	bool result;
	mtx_user.lock();
	result = compact_headers;
	mtx_user.unlock();
	return result;
}

bool t_user::get_encode_multi_values_as_list(void) const {
	bool result;
	mtx_user.lock();
	result = encode_multi_values_as_list;
	mtx_user.unlock();
	return result;
}

bool t_user::get_use_domain_in_contact(void) const {
	bool result;
	mtx_user.lock();
	result = use_domain_in_contact;
	mtx_user.unlock();
	return result;
}

bool t_user::get_allow_sdp_change(void) const {
	bool result;
	mtx_user.lock();
	result = allow_sdp_change;
	mtx_user.unlock();
	return result;
}

bool t_user::get_allow_redirection(void) const {
	bool result;
	mtx_user.lock();
	result = allow_redirection;
	mtx_user.unlock();
	return result;
}

bool t_user::get_ask_user_to_redirect(void) const {
	bool result;
	mtx_user.lock();
	result = ask_user_to_redirect;
	mtx_user.unlock();
	return result;
}

unsigned short t_user::get_max_redirections(void) const {
	unsigned short result;
	mtx_user.lock();
	result = max_redirections;
	mtx_user.unlock();
	return result;
}

t_ext_support t_user::get_ext_100rel(void) const {
	t_ext_support result;
	mtx_user.lock();
	result = ext_100rel;
	mtx_user.unlock();
	return result;
}

bool t_user::get_ext_replaces(void) const {
	bool result;
	mtx_user.lock();
	result = ext_replaces;
	mtx_user.unlock();
	return result;
}

bool t_user::get_referee_hold(void) const {
	t_mutex_guard guard(mtx_user);
	return referee_hold;
}

bool t_user::get_referrer_hold(void) const {
	t_mutex_guard guard(mtx_user);
	return referrer_hold;
}

bool t_user::get_allow_refer(void) const {
	t_mutex_guard guard(mtx_user);
	return allow_refer;
}

bool t_user::get_ask_user_to_refer(void) const {
	t_mutex_guard guard(mtx_user);
	return ask_user_to_refer;
}

bool t_user::get_auto_refresh_refer_sub(void) const {
	t_mutex_guard guard(mtx_user);
	return auto_refresh_refer_sub;
}

bool t_user::get_attended_refer_to_aor(void) const {
	t_mutex_guard guard(mtx_user);
	return attended_refer_to_aor;
}

bool t_user::get_allow_transfer_consultation_inprog(void) const {
	t_mutex_guard guard(mtx_user);
	return allow_transfer_consultation_inprog;
}

bool t_user::get_send_p_preferred_id(void) const {
	bool result;
	mtx_user.lock();
	result = send_p_preferred_id;
	mtx_user.unlock();
	return result;
}

t_sip_transport t_user::get_sip_transport(void) const {
	t_mutex_guard guard(mtx_user);
	return sip_transport;
}

unsigned short t_user::get_sip_transport_udp_threshold(void) const {
	t_mutex_guard guard(mtx_user);
	return sip_transport_udp_threshold;
}

bool t_user::get_use_nat_public_ip(void) const {
	bool result;
	mtx_user.lock();
	result = use_nat_public_ip;
	mtx_user.unlock();
	return result;
}

string t_user::get_nat_public_ip(void) const {
	string result;
	mtx_user.lock();
	result = nat_public_ip;
	mtx_user.unlock();
	return result;
}

bool t_user::get_use_stun(void) const {
	bool result;
	mtx_user.lock();
	result = use_stun;
	mtx_user.unlock();
	return result;
}

t_url t_user::get_stun_server(void) const {
	t_url result;
	mtx_user.lock();
	result = stun_server;
	mtx_user.unlock();
	return result;
}

bool t_user::get_persistent_tcp(void) const {
	t_mutex_guard guard(mtx_user);
	return persistent_tcp;
}

bool t_user::get_enable_nat_keepalive(void) const {
	t_mutex_guard guard(mtx_user);
	return enable_nat_keepalive;
}

unsigned short t_user::get_timer_noanswer(void) const {
	unsigned short result;
	mtx_user.lock();
	result = timer_noanswer;
	mtx_user.unlock();
	return result;
}

unsigned short t_user::get_timer_nat_keepalive(void) const {
	unsigned short result;
	mtx_user.lock();
	result = timer_nat_keepalive;
	mtx_user.unlock();
	return result;
}

unsigned short t_user::get_timer_tcp_ping(void) const {
	t_mutex_guard guard(mtx_user);
	return timer_tcp_ping;
}
 
bool t_user::get_display_useronly_phone(void) const {
	bool result;
	mtx_user.lock();
	result = display_useronly_phone;
	mtx_user.unlock();
	return result;
}

bool t_user::get_numerical_user_is_phone(void) const {
	bool result;
	mtx_user.lock();
	result = numerical_user_is_phone;
	mtx_user.unlock();
	return result;
}

bool t_user::get_remove_special_phone_symbols(void) const {
	bool result;
	mtx_user.lock();
	result = remove_special_phone_symbols;
	mtx_user.unlock();
	return result;
}

string t_user::get_special_phone_symbols(void) const {
	string result;
	mtx_user.lock();
	result = special_phone_symbols;
	mtx_user.unlock();
	return result;
}

bool t_user::get_use_tel_uri_for_phone(void) const {
	t_mutex_guard guard(mtx_user);
	return use_tel_uri_for_phone;
}

string t_user::get_ringtone_file(void) const {
	string result;
	mtx_user.lock();
	result = ringtone_file;
	mtx_user.unlock();
	return result;
}

string t_user::get_ringback_file(void) const {
	string result;
	mtx_user.lock();
	result = ringback_file;
	mtx_user.unlock();
	return result;
}

string t_user::get_script_incoming_call(void) const {
	string result;
	mtx_user.lock();
	result = script_incoming_call;
	mtx_user.unlock();
	return result;
}

string t_user::get_script_in_call_answered(void) const {
	string result;
	mtx_user.lock();
	result = script_in_call_answered;
	mtx_user.unlock();
	return result;
}

string t_user::get_script_in_call_failed(void) const {
	string result;
	mtx_user.lock();
	result = script_in_call_failed;
	mtx_user.unlock();
	return result;
}

string t_user::get_script_outgoing_call(void) const {
	string result;
	mtx_user.lock();
	result = script_outgoing_call;
	mtx_user.unlock();
	return result;
}

string t_user::get_script_out_call_answered(void) const {
	string result;
	mtx_user.lock();
	result = script_out_call_answered;
	mtx_user.unlock();
	return result;
}

string t_user::get_script_out_call_failed(void) const {
	string result;
	mtx_user.lock();
	result = script_out_call_failed;
	mtx_user.unlock();
	return result;
}

string t_user::get_script_local_release(void) const {
	string result;
	mtx_user.lock();
	result = script_local_release;
	mtx_user.unlock();
	return result;
}

string t_user::get_script_remote_release(void) const {
	string result;
	mtx_user.lock();
	result = script_remote_release;
	mtx_user.unlock();
	return result;
}

list<t_number_conversion> t_user::get_number_conversions(void) const {
	list<t_number_conversion> result;
	mtx_user.lock();
	result = number_conversions;
	mtx_user.unlock();
	return result;	
}

bool t_user::get_zrtp_enabled(void) const {
	bool result;
	mtx_user.lock();
	result = zrtp_enabled;
	mtx_user.unlock();
	return result;
}

bool t_user::get_zrtp_goclear_warning(void) const {
	bool result;
	mtx_user.lock();
	result = zrtp_goclear_warning;
	mtx_user.unlock();
	return result;
}

bool t_user::get_zrtp_sdp(void) const {
	bool result;
	mtx_user.lock();
	result = zrtp_sdp;
	mtx_user.unlock();
	return result;
}

bool t_user::get_zrtp_send_if_supported(void) const {
	bool result;
	mtx_user.lock();
	result = zrtp_send_if_supported;
	mtx_user.unlock();
	return result;
}

bool t_user::get_mwi_sollicited(void) const {
	bool result;
	mtx_user.lock();
	result = mwi_sollicited;
	mtx_user.unlock();
	return result;
}

string t_user::get_mwi_user(void) const {
	string result;
	mtx_user.lock();
	result = mwi_user;
	mtx_user.unlock();
	return result;
}

t_url t_user::get_mwi_server(void) const {
	t_url result;
	mtx_user.lock();
	result = mwi_server;
	mtx_user.unlock();
	return result;
}

bool t_user::get_mwi_via_proxy(void) const {
	bool result;
	mtx_user.lock();
	result = mwi_via_proxy;
	mtx_user.unlock();
	return result;
}

unsigned long t_user::get_mwi_subscription_time(void) const {
	unsigned long result;
	mtx_user.lock();
	result = mwi_subscription_time;
	mtx_user.unlock();
	return result;
}

string t_user::get_mwi_vm_address(void) const {
	string result;
	mtx_user.lock();
	result = mwi_vm_address;
	mtx_user.unlock();
	return result;
}

unsigned short t_user::get_im_max_sessions(void) const {
	unsigned short result;
	mtx_user.lock();
	result = im_max_sessions;
	mtx_user.unlock();
	return result;
}

bool t_user::get_im_send_iscomposing(void) const {
	t_mutex_guard guard(mtx_user);
	return im_send_iscomposing;
}

unsigned long t_user::get_pres_subscription_time(void) const {
	unsigned long result;
	mtx_user.lock();
	result = pres_subscription_time;
	mtx_user.unlock();
	return result;
}

unsigned long t_user::get_pres_publication_time(void) const {
	unsigned long result;
	mtx_user.lock();
	result = pres_publication_time;
	mtx_user.unlock();
	return result;
}

bool t_user::get_pres_publish_startup(void) const {
	bool result;
	mtx_user.lock();
	result = pres_publish_startup;
	mtx_user.unlock();
	return result;
}

	
void t_user::set_name(const string &_name) {
	mtx_user.lock();
	name = _name;
	mtx_user.unlock();
}

void t_user::set_domain(const string &_domain) {
	mtx_user.lock();
	domain = _domain;
	mtx_user.unlock();
}

void t_user::set_display(const string &_display) {	
	mtx_user.lock();
	display = _display;
	mtx_user.unlock();
}

void t_user::set_organization(const string &_organization) {
	mtx_user.lock();
	organization = _organization;
	mtx_user.unlock();
}

void t_user::set_auth_realm(const string &realm) {
	mtx_user.lock();
	auth_realm = realm;
	mtx_user.unlock();
}

void t_user::set_auth_name(const string &name) {
	mtx_user.lock();
	auth_name = name;
	mtx_user.unlock();
}

void t_user::set_auth_pass(const string &pass) {
	mtx_user.lock();
	auth_pass = pass;
	mtx_user.unlock();
}

void t_user::set_auth_aka_op(const uint8 *aka_op) {
	t_mutex_guard guard(mtx_user);
	memcpy(auth_aka_op, aka_op, AKA_OPLEN);
}

void t_user::set_auth_aka_amf(const uint8 *aka_amf) {
	t_mutex_guard guard(mtx_user);
	memcpy(auth_aka_amf, aka_amf, AKA_AMFLEN);
}

void t_user::set_use_outbound_proxy(bool b) {
	mtx_user.lock();
	use_outbound_proxy = b;
	mtx_user.unlock();
}

void t_user::set_outbound_proxy(const t_url &url) {
	mtx_user.lock();
	outbound_proxy = url;
	mtx_user.unlock();
}

void t_user::set_all_requests_to_proxy(bool b) {
	mtx_user.lock();
	all_requests_to_proxy = b;
	mtx_user.unlock();
}

void t_user::set_non_resolvable_to_proxy(bool b) {
	mtx_user.lock();
	non_resolvable_to_proxy = b;
	mtx_user.unlock();
}

void t_user::set_use_registrar(bool b) {
	mtx_user.lock();
	use_registrar = b;
	mtx_user.unlock();
}

void t_user::set_registrar(const t_url &url) {
	mtx_user.lock();
	registrar = url;
	mtx_user.unlock();
}

void t_user::set_registration_time(const unsigned long time) {
	mtx_user.lock();
	registration_time = time;
	mtx_user.unlock();
}

void t_user::set_register_at_startup(bool b) {
	mtx_user.lock();
	register_at_startup = b;
	mtx_user.unlock();
}

void t_user::set_reg_add_qvalue(bool b) {
	mtx_user.lock();
	reg_add_qvalue = b;
	mtx_user.unlock();
}

void t_user::set_reg_qvalue(float q) {
	mtx_user.lock();
	reg_qvalue = q;
	mtx_user.unlock();
}

void t_user::set_codecs(const list<t_audio_codec> &_codecs) {
	mtx_user.lock();
	codecs = _codecs;
	mtx_user.unlock();
}

void t_user::set_ptime(unsigned short _ptime) {
	mtx_user.lock();
	ptime = _ptime;
	mtx_user.unlock();
}

void t_user::set_out_obey_far_end_codec_pref(bool b) {
	mtx_user.lock();
	out_obey_far_end_codec_pref = b;
	mtx_user.unlock();
}

void t_user::set_in_obey_far_end_codec_pref(bool b) {
	mtx_user.lock();
	in_obey_far_end_codec_pref = b;
	mtx_user.unlock();
}

void t_user::set_speex_nb_payload_type(unsigned short payload_type) {
	mtx_user.lock();
	speex_nb_payload_type = payload_type;
	mtx_user.unlock();
}

void t_user::set_speex_wb_payload_type(unsigned short payload_type) {
	mtx_user.lock();
	speex_wb_payload_type = payload_type;
	mtx_user.unlock();
}

void t_user::set_speex_uwb_payload_type(unsigned short payload_type) {
	mtx_user.lock();
	speex_uwb_payload_type = payload_type;
	mtx_user.unlock();
}

void t_user::set_speex_bit_rate_type(t_bit_rate_type bit_rate_type) {
	mtx_user.lock();
	speex_bit_rate_type = bit_rate_type;
	mtx_user.unlock();
}

void t_user::set_speex_abr_nb(int abr) {
	mtx_user.lock();
	speex_abr_nb = abr;
	mtx_user.unlock();
}

void t_user::set_speex_abr_wb(int abr) {
	mtx_user.lock();
	speex_abr_wb = abr;
	mtx_user.unlock();
}

void t_user::set_speex_dtx(bool b) {
	mtx_user.lock();
	speex_dtx = b;
	mtx_user.unlock();
}

void t_user::set_speex_penh(bool b) {
	mtx_user.lock();
	speex_penh = b;
	mtx_user.unlock();
}

void t_user::set_speex_quality(unsigned short quality) {
	mtx_user.lock();
	speex_quality = quality;
	mtx_user.unlock();
}

void t_user::set_speex_complexity(unsigned short complexity) {
	mtx_user.lock();
	speex_complexity = complexity;
	mtx_user.unlock();
}

void t_user::set_speex_dsp_vad(bool b) {
	mtx_user.lock();
	speex_dsp_vad = b;
	mtx_user.unlock();
}

void t_user::set_speex_dsp_agc(bool b) {
	mtx_user.lock();
	speex_dsp_agc = b;
	mtx_user.unlock();
}

void t_user::set_speex_dsp_agc_level(unsigned short level) {
	mtx_user.lock();
	speex_dsp_agc_level = level;
	mtx_user.unlock();
}

void t_user::set_speex_dsp_aec(bool b) {
	mtx_user.lock();
	speex_dsp_aec = b;
	mtx_user.unlock();
}

void t_user::set_speex_dsp_nrd(bool b) {
	mtx_user.lock();
	speex_dsp_nrd = b;
	mtx_user.unlock();
}

void t_user::set_ilbc_payload_type(unsigned short payload_type) {
	mtx_user.lock();
	ilbc_payload_type = payload_type;
	mtx_user.unlock();
}

void t_user::set_ilbc_mode(unsigned short mode) {
	mtx_user.lock();
	ilbc_mode = mode;
	mtx_user.unlock();
}

void t_user::set_g726_16_payload_type(unsigned short payload_type) {
	mtx_user.lock();
	g726_16_payload_type = payload_type;
	mtx_user.unlock();
}

void t_user::set_g726_24_payload_type(unsigned short payload_type) {
	mtx_user.lock();
	g726_24_payload_type = payload_type;
	mtx_user.unlock();
}

void t_user::set_g726_32_payload_type(unsigned short payload_type) {
	mtx_user.lock();
	g726_32_payload_type = payload_type;
	mtx_user.unlock();
}

void t_user::set_g726_40_payload_type(unsigned short payload_type) {
	mtx_user.lock();
	g726_40_payload_type = payload_type;
	mtx_user.unlock();
}

void t_user::set_g726_packing(t_g726_packing packing) {
	mtx_user.lock();
	g726_packing = packing;
	mtx_user.unlock();
}

void t_user::set_dtmf_transport(t_dtmf_transport _dtmf_transport) {
	mtx_user.lock();
	dtmf_transport = _dtmf_transport;
	mtx_user.unlock();
}

void t_user::set_dtmf_payload_type(unsigned short payload_type) {
	mtx_user.lock();
	dtmf_payload_type = payload_type;
	mtx_user.unlock();
}

void t_user::set_dtmf_duration(unsigned short duration) {
	mtx_user.lock();
	dtmf_duration = duration;
	mtx_user.unlock();
}

void t_user::set_dtmf_pause(unsigned short pause) {
	mtx_user.lock();
	dtmf_pause = pause;
	mtx_user.unlock();
}

void t_user::set_dtmf_volume(unsigned short volume) {
	mtx_user.lock();
	dtmf_volume = volume;
	mtx_user.unlock();
}

void t_user::set_hold_variant(t_hold_variant _hold_variant) {
	mtx_user.lock();
	hold_variant = _hold_variant;
	mtx_user.unlock();
}

void t_user::set_check_max_forwards(bool b) {
	mtx_user.lock();
	check_max_forwards = b;
	mtx_user.unlock();
}

void t_user::set_allow_missing_contact_reg(bool b) {
	mtx_user.lock();
	allow_missing_contact_reg = b;
	mtx_user.unlock();
}

void t_user::set_registration_time_in_contact(bool b) {
	mtx_user.lock();
	registration_time_in_contact = b;
	mtx_user.unlock();
}

void t_user::set_compact_headers(bool b) {
	mtx_user.lock();
	compact_headers = b;
	mtx_user.unlock();
}

void t_user::set_encode_multi_values_as_list(bool b) {
	mtx_user.lock();
	encode_multi_values_as_list = b;
	mtx_user.unlock();
}

void t_user::set_use_domain_in_contact(bool b) {
	mtx_user.lock();
	use_domain_in_contact = b;
	mtx_user.unlock();
}

void t_user::set_allow_sdp_change(bool b) {
	mtx_user.lock();
	allow_sdp_change = b;
	mtx_user.unlock();
}

void t_user::set_allow_redirection(bool b) {
	mtx_user.lock();
	allow_redirection = b;
	mtx_user.unlock();
}

void t_user::set_ask_user_to_redirect(bool b) {
	mtx_user.lock();
	ask_user_to_redirect = b;
	mtx_user.unlock();
}

void t_user::set_max_redirections(unsigned short _max_redirections) {
	mtx_user.lock();
	max_redirections = _max_redirections;
	mtx_user.unlock();
}

void t_user::set_ext_100rel(t_ext_support ext_support) {
	mtx_user.lock();
	ext_100rel = ext_support;
	mtx_user.unlock();
}

void t_user::set_ext_replaces(bool b) {
	mtx_user.lock();
	ext_replaces = b;
	mtx_user.unlock();
}

void t_user::set_referee_hold(bool b) {
	t_mutex_guard guard(mtx_user);
	referee_hold = b;
}

void t_user::set_referrer_hold(bool b) {
	mtx_user.lock();
	referrer_hold = b;
	mtx_user.unlock();
}

void t_user::set_allow_refer(bool b) {
	t_mutex_guard guard(mtx_user);
	allow_refer = b;
}

void t_user::set_ask_user_to_refer(bool b) {
	t_mutex_guard guard(mtx_user);
	ask_user_to_refer = b;
}

void t_user::set_auto_refresh_refer_sub(bool b) {
	t_mutex_guard guard(mtx_user);
	auto_refresh_refer_sub = b;
}

void t_user::set_attended_refer_to_aor(bool b) {
	t_mutex_guard guard(mtx_user);
	attended_refer_to_aor = b;
}

void t_user::set_allow_transfer_consultation_inprog(bool b) {
	t_mutex_guard guard(mtx_user);
	allow_transfer_consultation_inprog = b;
}

void t_user::set_send_p_preferred_id(bool b) {
	mtx_user.lock();
	send_p_preferred_id = b;
	mtx_user.unlock();
}

void t_user::set_sip_transport(t_sip_transport transport) {
	t_mutex_guard guard(mtx_user);
	sip_transport = transport;
}

void t_user::set_sip_transport_udp_threshold(unsigned short threshold) {
	t_mutex_guard guard(mtx_user);
	sip_transport_udp_threshold = threshold;
}

void t_user::set_use_nat_public_ip(bool b) {
	mtx_user.lock();
	use_nat_public_ip = b;
	mtx_user.unlock();
}

void t_user::set_nat_public_ip(const string &public_ip) {
	mtx_user.lock();
	nat_public_ip = public_ip;
	mtx_user.unlock();
}

void t_user::set_use_stun(bool b) {
	mtx_user.lock();
	use_stun = b;
	mtx_user.unlock();
}

void t_user::set_stun_server(const t_url &url) {
	mtx_user.lock();
	stun_server = url;
	mtx_user.unlock();
}

void t_user::set_persistent_tcp(bool b) {
	t_mutex_guard guard(mtx_user);
	persistent_tcp = b;
}

void t_user::set_enable_nat_keepalive(bool b) {
	t_mutex_guard guard(mtx_user);
	enable_nat_keepalive = b;
}

void t_user::set_timer_noanswer(unsigned short timer) {
	mtx_user.lock();
	timer_noanswer = timer;
	mtx_user.unlock();
}

void t_user::set_timer_nat_keepalive(unsigned short timer) { 
	mtx_user.lock();
	timer_nat_keepalive = timer;
	mtx_user.unlock();
}

void t_user::set_timer_tcp_ping(unsigned short timer) {
	t_mutex_guard guard(mtx_user);
	timer_tcp_ping = timer;
}

void t_user::set_display_useronly_phone(bool b) {
	mtx_user.lock();
	display_useronly_phone = b;
	mtx_user.unlock();
}

void t_user::set_numerical_user_is_phone(bool b) {
	mtx_user.lock();
	numerical_user_is_phone = b;
	mtx_user.unlock();
}

void t_user::set_remove_special_phone_symbols(bool b) {
	mtx_user.lock();
	remove_special_phone_symbols = b;
	mtx_user.unlock();
}

void t_user::set_special_phone_symbols(const string &symbols) {
	mtx_user.lock();
	special_phone_symbols = symbols;
	mtx_user.unlock();
}

void t_user::set_use_tel_uri_for_phone(bool b) {
	t_mutex_guard guard(mtx_user);
	use_tel_uri_for_phone = b;
}

void t_user::set_ringtone_file(const string &file) {
	mtx_user.lock();
	ringtone_file = file;
	mtx_user.unlock();
}

void t_user::set_ringback_file(const string &file) {
	mtx_user.lock();
	ringback_file = file;
	mtx_user.unlock();
}

void t_user::set_script_incoming_call(const string &script) {
	mtx_user.lock();
	script_incoming_call = script;
	mtx_user.unlock();
}

void t_user::set_script_in_call_answered(const string &script) {
	mtx_user.lock();
	script_in_call_answered = script;
	mtx_user.unlock();
}

void t_user::set_script_in_call_failed(const string &script) {
	mtx_user.lock();
	script_in_call_failed = script;
	mtx_user.unlock();
}

void t_user::set_script_outgoing_call(const string &script) {
	mtx_user.lock();
	script_outgoing_call = script;
	mtx_user.unlock();
}

void t_user::set_script_out_call_answered(const string &script) {
	mtx_user.lock();
	script_out_call_answered = script;
	mtx_user.unlock();
}

void t_user::set_script_out_call_failed(const string &script) {
	mtx_user.lock();
	script_out_call_failed = script;
	mtx_user.unlock();
}

void t_user::set_script_local_release(const string &script) {
	mtx_user.lock();
	script_local_release = script;
	mtx_user.unlock();
}

void t_user::set_script_remote_release(const string &script) {
	mtx_user.lock();
	script_remote_release = script;
	mtx_user.unlock();
}

void t_user::set_number_conversions(const list<t_number_conversion> &l) {
	mtx_user.lock();
	number_conversions = l;
	mtx_user.unlock();
}

void t_user::set_zrtp_enabled(bool b) {
	mtx_user.lock();
	zrtp_enabled = b;
	mtx_user.unlock();
}

void t_user::set_zrtp_goclear_warning(bool b) {
	mtx_user.lock();
	zrtp_goclear_warning = b;
	mtx_user.unlock();
}

void t_user::set_zrtp_sdp(bool b) {
	mtx_user.lock();
	zrtp_sdp = b;
	mtx_user.unlock();
}

void t_user::set_zrtp_send_if_supported(bool b) {
	mtx_user.lock();
	zrtp_send_if_supported = b;
	mtx_user.unlock();
}

void t_user::set_mwi_sollicited(bool b) {
	mtx_user.lock();
	mwi_sollicited = b;
	mtx_user.unlock();
}

void t_user::set_mwi_user(const string &user) {
	mtx_user.lock();
	mwi_user = user;
	mtx_user.unlock();
}

void t_user::set_mwi_server(const t_url &url) {
	mtx_user.lock();
	mwi_server = url;
	mtx_user.unlock();
}

void t_user::set_mwi_via_proxy(bool b) {
	mtx_user.lock();
	mwi_via_proxy = b;
	mtx_user.unlock();
}

void t_user::set_mwi_subscription_time(unsigned long t) {
	mtx_user.lock();
	mwi_subscription_time = t;
	mtx_user.unlock();
}

void t_user::set_mwi_vm_address(const string &address) {
	mtx_user.lock();
	mwi_vm_address = address;
	mtx_user.unlock();
}

void t_user::set_im_max_sessions(unsigned short max_sessions) {
	mtx_user.lock();
	im_max_sessions = max_sessions;
	mtx_user.unlock();
}

void t_user::set_im_send_iscomposing(bool b) {
	t_mutex_guard guard(mtx_user);
	im_send_iscomposing = b;
}

void t_user::set_pres_subscription_time(unsigned long t) {
	mtx_user.lock();
	pres_subscription_time = t;
	mtx_user.unlock();
}

void t_user::set_pres_publication_time(unsigned long t) {
	mtx_user.lock();
	pres_publication_time = t;
	mtx_user.unlock();
}

void t_user::set_pres_publish_startup(bool b) {
	mtx_user.lock();
	pres_publish_startup = b;
	mtx_user.unlock();
}

bool t_user::read_config(const string &filename, string &error_msg) {
	string f;
	string msg;
	
	mtx_user.lock();
	
	if (filename.size() == 0) {
		error_msg = "Cannot read user profile: missing file name.";
		log_file->write_report(error_msg, "t_user::read_config",
			LOG_NORMAL, LOG_CRITICAL);
		mtx_user.unlock();
		return false;
	}

	config_filename = filename;
	f = expand_filename(filename);

	ifstream config(f.c_str());
	if (!config) {
		error_msg = "Cannot open file for reading: ";
		error_msg += f;
		log_file->write_report(error_msg, "t_user::read_config",
			LOG_NORMAL, LOG_CRITICAL);
		mtx_user.unlock();
		return false;
	}

	log_file->write_header("t_user::read_config");
	log_file->write_raw("Reading config: ");
	log_file->write_raw(filename);
	log_file->write_endl();
	log_file->write_footer();

	while (!config.eof()) {
		string line;
		getline(config, line);

		// Check if read operation succeeded
		if (!config.good() && !config.eof()) {
			error_msg = "File system error while reading file ";
			error_msg += f;
			log_file->write_report(error_msg, "t_user::read_config",
				LOG_NORMAL, LOG_CRITICAL);
			mtx_user.unlock();
			return false;
		}

		line = trim(line);

		// Skip empty lines
		if (line.size() == 0) continue;

		// Skip comment lines
		if (line[0] == '#') continue;

		vector<string> l = split_on_first(line, '=');
		if (l.size() != 2) {
			error_msg = "Syntax error in file ";
			error_msg += f;
			error_msg += "\n";
			error_msg += line;
			log_file->write_report(error_msg, "t_user::read_config",
				LOG_NORMAL, LOG_CRITICAL);
			mtx_user.unlock();
			return false;
		}

		string parameter = trim(l[0]);
		string value = trim(l[1]);
		
		if (parameter == FLD_NAME) {
			name = value;
		} else if (parameter == FLD_DOMAIN) {
			domain = value;
		} else if (parameter == FLD_DISPLAY) {
			display = value;
		} else if (parameter == FLD_ORGANIZATION) {
			organization = value;
		} else if (parameter == FLD_REGISTRATION_TIME) {
			registration_time = atol(value.c_str());
		} else if (parameter == FLD_REGISTRATION_TIME_IN_CONTACT) {
			registration_time_in_contact = yesno2bool(value);
		} else if (parameter == FLD_REGISTRAR) {
			use_registrar = set_server_value(registrar, USER_SCHEME, value); 
		} else if (parameter == FLD_REGISTER_AT_STARTUP) {
			register_at_startup = yesno2bool(value);
		} else if (parameter == FLD_REG_ADD_QVALUE) {
			reg_add_qvalue = yesno2bool(value);
		} else if (parameter == FLD_REG_QVALUE) {
			reg_qvalue = atof(value.c_str());
		} else if (parameter == FLD_OUTBOUND_PROXY) {
			use_outbound_proxy = set_server_value(outbound_proxy,
					USER_SCHEME, value);
		} else if (parameter == FLD_ALL_REQUESTS_TO_PROXY) {
			all_requests_to_proxy = yesno2bool(value);
		} else if (parameter == FLD_NON_RESOLVABLE_TO_PROXY) {
			non_resolvable_to_proxy = yesno2bool(value);
		} else if (parameter == FLD_AUTH_REALM) {
			auth_realm = value;
		} else if (parameter == FLD_AUTH_NAME) {
			auth_name = value;
		} else if (parameter == FLD_AUTH_PASS) {
			auth_pass = value;
		} else if (parameter == FLD_AUTH_AKA_OP) {
			hex2binary(value, auth_aka_op);
		} else if (parameter == FLD_AUTH_AKA_AMF) {
			hex2binary(value, auth_aka_amf);
		} else if (parameter == FLD_CODECS) {
			vector<string> l = split(value, ',');
			if (l.size() > 0) codecs.clear();
			for (vector<string>::iterator i = l.begin();
			     i != l.end(); i++)
			{
				string codec = trim(*i);
				if (codec == "g711a") {
					codecs.push_back(CODEC_G711_ALAW);
				} else if (codec == "g711u") {
					codecs.push_back(CODEC_G711_ULAW);
				} else if (codec == "gsm") {
					codecs.push_back(CODEC_GSM);
#ifdef HAVE_SPEEX
				} else if (codec == "speex-nb") {
					codecs.push_back(CODEC_SPEEX_NB);
				} else if (codec == "speex-wb") {
					codecs.push_back(CODEC_SPEEX_WB);
				} else if (codec == "speex-uwb") {
					codecs.push_back(CODEC_SPEEX_UWB);
#endif
#ifdef HAVE_ILBC
				} else if (codec == "ilbc") {
					codecs.push_back(CODEC_ILBC);
#endif
				} else if (codec == "g726-16") {
					codecs.push_back(CODEC_G726_16);
				} else if (codec == "g726-24") {
					codecs.push_back(CODEC_G726_24);
				} else if (codec == "g726-32") {
					codecs.push_back(CODEC_G726_32);
				} else if (codec == "g726-40") {
					codecs.push_back(CODEC_G726_40);
				} else {
					msg = "Syntax error in file ";
					msg += f;
					msg += "\n";
					msg += "Invalid codec: ";
					msg += value;
					log_file->write_report(msg,
						"t_user::read_config",
						LOG_NORMAL, LOG_WARNING);
				}
			}
		} else if (parameter == FLD_PTIME) {
			ptime = atoi(value.c_str());
		} else if (parameter == FLD_OUT_FAR_END_CODEC_PREF) {
			out_obey_far_end_codec_pref = yesno2bool(value);
		} else if (parameter == FLD_IN_FAR_END_CODEC_PREF) {
			in_obey_far_end_codec_pref = yesno2bool(value);
		} else if (parameter == FLD_HOLD_VARIANT) {
			if (value == "rfc2543") {
				hold_variant = HOLD_RFC2543;
			} else if (value == "rfc3264") {
				hold_variant = HOLD_RFC3264;
			} else {
				error_msg = "Syntax error in file ";
				error_msg += f;
				error_msg += "\n";
				error_msg += "Invalid hold variant: ";
				error_msg += value;
				log_file->write_report(error_msg, "t_user::read_config",
					LOG_NORMAL, LOG_CRITICAL);
				mtx_user.unlock();
				return false;
			}
		} else if (parameter == FLD_CHECK_MAX_FORWARDS) {
			check_max_forwards = yesno2bool(value);
		} else if (parameter == FLD_ALLOW_MISSING_CONTACT_REG) {
			allow_missing_contact_reg = yesno2bool(value);
		} else if (parameter == FLD_USE_DOMAIN_IN_CONTACT) {
			use_domain_in_contact = yesno2bool(value);
		} else if (parameter == FLD_ALLOW_SDP_CHANGE) {
			allow_sdp_change = yesno2bool(value);
		} else if (parameter == FLD_ALLOW_REDIRECTION) {
			allow_redirection = yesno2bool(value);
		} else if (parameter == FLD_ASK_USER_TO_REDIRECT) {
			ask_user_to_redirect = yesno2bool(value);
		} else if (parameter == FLD_MAX_REDIRECTIONS) {
			max_redirections = atoi(value.c_str());
		} else if (parameter == FLD_REFEREE_HOLD) {
			referee_hold = yesno2bool(value);
		} else if (parameter == FLD_REFERRER_HOLD) {
			referrer_hold = yesno2bool(value);
		} else if (parameter == FLD_ALLOW_REFER) {
			allow_refer = yesno2bool(value);
		} else if (parameter == FLD_ASK_USER_TO_REFER) {
			ask_user_to_refer = yesno2bool(value);
		} else if (parameter == FLD_AUTO_REFRESH_REFER_SUB) {
			auto_refresh_refer_sub = yesno2bool(value);
		} else if (parameter == FLD_ATTENDED_REFER_TO_AOR) {
			attended_refer_to_aor = yesno2bool(value);
		} else if (parameter == FLD_ALLOW_XFER_CONSULT_INPROG) {
			allow_transfer_consultation_inprog = yesno2bool(value);
		} else if (parameter == FLD_SEND_P_PREFERRED_ID) {
			send_p_preferred_id = yesno2bool(value);
		} else if (parameter == FLD_SIP_TRANSPORT) {
			sip_transport = str2sip_transport(value);
		} else if (parameter == FLD_SIP_TRANSPORT_UDP_THRESHOLD) {
			sip_transport_udp_threshold = atoi(value.c_str());
		} else if (parameter == FLD_NAT_PUBLIC_IP) {
			if (value.size() == 0) continue;
			use_nat_public_ip = true;
			nat_public_ip = value;
		} else if (parameter == FLD_STUN_SERVER) {
			use_stun = set_server_value(stun_server, "stun", value);
		} else if (parameter == FLD_PERSISTENT_TCP) {
			persistent_tcp = yesno2bool(value);
		} else if (parameter == FLD_ENABLE_NAT_KEEPALIVE) {
			enable_nat_keepalive = yesno2bool(value);
		} else if (parameter == FLD_TIMER_NOANSWER) {
			timer_noanswer = atoi(value.c_str());
		} else if (parameter == FLD_TIMER_NAT_KEEPALIVE) {
			timer_nat_keepalive = atoi(value.c_str());
		} else if (parameter == FLD_TIMER_TCP_PING) {
			timer_tcp_ping = atoi(value.c_str());
		} else if (parameter == FLD_EXT_100REL) {
			ext_100rel = str2ext_support(value);
			if (ext_100rel == EXT_INVALID) {
				error_msg = "Syntax error in file ";
				error_msg += f;
				error_msg += "\n";
				error_msg += "Invalid value for ext_100rel: ";
				error_msg += value;
				log_file->write_report(error_msg, "t_user::read_config",
					LOG_NORMAL, LOG_CRITICAL);
				mtx_user.unlock();
				return false;
			}
		} else if (parameter == FLD_EXT_REPLACES) {
			ext_replaces = yesno2bool(value);
		} else if (parameter == FLD_COMPACT_HEADERS) {
			compact_headers = yesno2bool(value);
		} else if (parameter == FLD_ENCODE_MULTI_VALUES_AS_LIST) {
			encode_multi_values_as_list = yesno2bool(value);
		} else if (parameter == FLD_SPEEX_NB_PAYLOAD_TYPE) {
			speex_nb_payload_type = atoi(value.c_str());
		} else if (parameter == FLD_SPEEX_WB_PAYLOAD_TYPE) {
			speex_wb_payload_type = atoi(value.c_str());
		} else if (parameter == FLD_SPEEX_UWB_PAYLOAD_TYPE) {
			speex_uwb_payload_type = atoi(value.c_str());
		} else if (parameter == FLD_SPEEX_BIT_RATE_TYPE) {
			speex_bit_rate_type = str2bit_rate_type(value);
			if (speex_bit_rate_type == BIT_RATE_INVALID) {
				error_msg = "Syntax error in file ";
				error_msg += f;
				error_msg += "\n";
				error_msg += "Invalid value for speex bit rate type: ";
				error_msg += value;
				log_file->write_report(error_msg, "t_user::read_config",
					LOG_NORMAL, LOG_CRITICAL);
				mtx_user.unlock();
				return false;		
			}
		} else if (parameter == FLD_SPEEX_ABR_NB) {
			speex_abr_nb = atoi(value.c_str());
		} else if (parameter == FLD_SPEEX_ABR_WB) {
			speex_abr_wb = atoi(value.c_str());
		} else if (parameter == FLD_SPEEX_DTX) {
			speex_dtx = yesno2bool(value);
		} else if (parameter == FLD_SPEEX_PENH) {
			speex_penh = yesno2bool(value);
		} else if (parameter == FLD_SPEEX_QUALITY) {
			speex_quality = atoi(value.c_str());
			if (speex_quality > 10) {
				error_msg = "Syntax error in file ";
				error_msg += f;
				error_msg += "\n";
				error_msg += "Invalid value for speex quality: ";
				error_msg += value;
				log_file->write_report(error_msg, "t_user::read_config",
					LOG_NORMAL, LOG_CRITICAL);
				mtx_user.unlock();
				return false;	
			}
		} else if (parameter == FLD_SPEEX_COMPLEXITY) {
			speex_complexity = atoi(value.c_str());
			if (speex_complexity < 1 || speex_complexity > 10) {
				error_msg = "Syntax error in file ";
				error_msg += f;
				error_msg += "\n";
				error_msg += "Invalid value for speex complexity: ";
				error_msg += value;
				log_file->write_report(error_msg, "t_user::read_config",
					LOG_NORMAL, LOG_CRITICAL);
				mtx_user.unlock();
				return false;	
			}
		} else if (parameter == FLD_SPEEX_DSP_VAD) {
			speex_dsp_vad = yesno2bool(value);
		} else if (parameter == FLD_SPEEX_DSP_AGC) {
			speex_dsp_agc = yesno2bool(value);
		} else if (parameter == FLD_SPEEX_DSP_AEC) {
			speex_dsp_aec = yesno2bool(value);
		} else if (parameter == FLD_SPEEX_DSP_NRD) {
			speex_dsp_nrd = yesno2bool(value);
		} else if (parameter == FLD_SPEEX_DSP_AGC_LEVEL) {
			speex_dsp_agc_level = atoi(value.c_str());
			if (speex_dsp_agc_level < 1 || speex_dsp_agc_level > 100) {
				error_msg = "Syntax error in file ";
				error_msg += f;
				error_msg += "\n";
				error_msg += "Invalid value for automatic gain control level: ";
				error_msg += value;
				log_file->write_report(error_msg, "t_user::read_config",
					LOG_NORMAL, LOG_CRITICAL);
				mtx_user.unlock();
				return false;	
			}
		} else if (parameter == FLD_ILBC_PAYLOAD_TYPE) {
			ilbc_payload_type = atoi(value.c_str());
		} else if (parameter == FLD_ILBC_MODE) {
			ilbc_mode = atoi(value.c_str());
		} else if (parameter == FLD_G726_16_PAYLOAD_TYPE) {
			g726_16_payload_type = atoi(value.c_str());
		} else if (parameter == FLD_G726_24_PAYLOAD_TYPE) {
			g726_24_payload_type = atoi(value.c_str());
		} else if (parameter == FLD_G726_32_PAYLOAD_TYPE) {
			g726_32_payload_type = atoi(value.c_str());
		} else if (parameter == FLD_G726_40_PAYLOAD_TYPE) {
			g726_40_payload_type = atoi(value.c_str());
		} else if (parameter == FLD_G726_PACKING) {
			g726_packing = str2g726_packing(value);
		} else if (parameter == FLD_DTMF_TRANSPORT) {
			dtmf_transport = str2dtmf_transport(value);	
		} else if (parameter == FLD_DTMF_PAYLOAD_TYPE) {
			dtmf_payload_type = atoi(value.c_str());
		} else if (parameter == FLD_DTMF_DURATION) {
			dtmf_duration = atoi(value.c_str());
		} else if (parameter == FLD_DTMF_PAUSE) {
			dtmf_pause = atoi(value.c_str());
		} else if (parameter == FLD_DTMF_VOLUME) {
			dtmf_volume = atoi(value.c_str());
		} else if (parameter == FLD_DISPLAY_USERONLY_PHONE) {
			display_useronly_phone = yesno2bool(value);
		} else if (parameter == FLD_NUMERICAL_USER_IS_PHONE) {
			numerical_user_is_phone = yesno2bool(value);
		} else if (parameter == FLD_REMOVE_SPECIAL_PHONE_SYM) {
			remove_special_phone_symbols = yesno2bool(value);
		} else if (parameter == FLD_SPECIAL_PHONE_SYMBOLS) {
			special_phone_symbols = value;
		} else if (parameter == FLD_USE_TEL_URI_FOR_PHONE) {
			use_tel_uri_for_phone = yesno2bool(value);
		} else if (parameter == FLD_USER_RINGTONE_FILE) {
			ringtone_file = value;
		} else if (parameter == FLD_USER_RINGBACK_FILE) {
			ringback_file = value;
		} else if (parameter == FLD_SCRIPT_INCOMING_CALL) {
			script_incoming_call = value;
		} else if (parameter == FLD_SCRIPT_IN_CALL_ANSWERED) {
			script_in_call_answered = value;
		} else if (parameter == FLD_SCRIPT_IN_CALL_FAILED) {
			script_in_call_failed = value;
		} else if (parameter == FLD_SCRIPT_OUTGOING_CALL) {
			script_outgoing_call = value;
		} else if (parameter == FLD_SCRIPT_OUT_CALL_ANSWERED) {
			script_out_call_answered = value;
		} else if (parameter == FLD_SCRIPT_OUT_CALL_FAILED) {
			script_out_call_failed = value;
		} else if (parameter == FLD_SCRIPT_LOCAL_RELEASE) {
			script_local_release = value;
		} else if (parameter == FLD_SCRIPT_REMOTE_RELEASE) {
			script_remote_release = value;
		} else if (parameter == FLD_NUMBER_CONVERSION) {
			t_number_conversion c;
			if (parse_num_conversion(value, c)) {
				number_conversions.push_back(c);
			}
		} else if (parameter == FLD_ZRTP_ENABLED) {
			zrtp_enabled = yesno2bool(value);
		} else if (parameter == FLD_ZRTP_GOCLEAR_WARNING) {
			zrtp_goclear_warning = yesno2bool(value);
		} else if (parameter == FLD_ZRTP_SDP) {
			zrtp_sdp = yesno2bool(value);
		} else if (parameter == FLD_ZRTP_SEND_IF_SUPPORTED) {
			zrtp_send_if_supported = yesno2bool(value);
		} else if (parameter == FLD_MWI_SOLLICITED) {
			mwi_sollicited = yesno2bool(value);
		} else if (parameter == FLD_MWI_USER) {
			mwi_user = value;
		} else if (parameter == FLD_MWI_SERVER) {
			(void)set_server_value(mwi_server, USER_SCHEME, value);
		} else if (parameter == FLD_MWI_VIA_PROXY) {
			mwi_via_proxy = yesno2bool(value);
		} else if (parameter == FLD_MWI_SUBSCRIPTION_TIME) {
			mwi_subscription_time = atol(value.c_str());
		} else if (parameter == FLD_MWI_VM_ADDRESS) {
			mwi_vm_address = value;
		} else if (parameter == FLD_IM_MAX_SESSIONS) {
			im_max_sessions = atoi(value.c_str());
		} else if (parameter == FLD_IM_SEND_ISCOMPOSING) {
			im_send_iscomposing = yesno2bool(value);
		} else if (parameter == FLD_PRES_SUBSCRIPTION_TIME) {
			pres_subscription_time = atol(value.c_str());
		} else if (parameter == FLD_PRES_PUBLICATION_TIME) {
			pres_publication_time = atol(value.c_str());
		} else if (parameter == FLD_PRES_PUBLISH_STARTUP) {
			pres_publish_startup = yesno2bool(value);
		} else {
			// Ignore unknown parameters. Only report in log file.
			log_file->write_header("t_user::read_config",
				LOG_NORMAL, LOG_WARNING);
			log_file->write_raw("Unknown parameter in user profile: ");
			log_file->write_raw(parameter);
			log_file->write_endl();
			log_file->write_footer();
		}
	}

	// Set parser options
	t_parser::check_max_forwards = check_max_forwards;
	t_parser::compact_headers = compact_headers;
	t_parser::multi_values_as_list = encode_multi_values_as_list;

	mtx_user.unlock();
	return true;
}

bool t_user::write_config(const string &filename, string &error_msg) {
	struct stat stat_buf;
	string f;
	
	mtx_user.lock();

	if (filename.size() == 0) {
		error_msg = "Cannot write user profile: missing file name.";
		log_file->write_report(error_msg, "t_user::write_config",
			LOG_NORMAL, LOG_CRITICAL);
		mtx_user.unlock();
		return false;
	}

	config_filename = filename;
	f = expand_filename(filename);

	// Make a backup of the file if we are editing an existing file, so
	// that can be restored when writing fails.
	string f_backup = f + '~';
	if (stat(f.c_str(), &stat_buf) == 0) {
		if (rename(f.c_str(), f_backup.c_str()) != 0) {
			string err = get_error_str(errno);
			error_msg = "Failed to backup ";
			error_msg += f;
			error_msg += " to ";
			error_msg += f_backup;
			error_msg += "\n";
			error_msg += err;
			log_file->write_report(error_msg, "t_user::write_config",
				LOG_NORMAL, LOG_CRITICAL);
			mtx_user.unlock();
			return false;
		}
	}

	ofstream config(f.c_str());
	if (!config) {
		error_msg = "Cannot open file for writing: ";
		error_msg += f;
		log_file->write_report(error_msg, "t_user::write_config",
			LOG_NORMAL, LOG_CRITICAL);
		mtx_user.unlock();
		return false;
	}

	log_file->write_header("t_user::write_config");
	log_file->write_raw("Writing config: ");
	log_file->write_raw(filename);
	log_file->write_endl();
	log_file->write_footer();

	// Write USER settings
	config << "# USER\n";
	config << FLD_NAME << '=' << name << endl;
	config << FLD_DOMAIN << '=' << domain << endl;
	config << FLD_DISPLAY << '=' << display << endl;
	config << FLD_ORGANIZATION << '=' << organization << endl;
	config << FLD_AUTH_REALM << '=' << auth_realm << endl;
	config << FLD_AUTH_NAME << '=' << auth_name << endl;
	config << FLD_AUTH_PASS << '=' << auth_pass << endl;
	config << FLD_AUTH_AKA_OP << '=' << binary2hex(auth_aka_op, AKA_OPLEN) << endl;
	config << FLD_AUTH_AKA_AMF << '=' << binary2hex(auth_aka_amf, AKA_AMFLEN) << endl;
	config << endl;

	// Write SIP SERVER settings
	config << "# SIP SERVER\n";
	if (use_outbound_proxy) {
		config << FLD_OUTBOUND_PROXY << '=';
		config << outbound_proxy.encode_noscheme() << endl;
		config << FLD_ALL_REQUESTS_TO_PROXY << '=';
		config << bool2yesno(all_requests_to_proxy) << endl;
		config << FLD_NON_RESOLVABLE_TO_PROXY << '=';
		config << bool2yesno(non_resolvable_to_proxy) << endl;
	} else {
		config << FLD_OUTBOUND_PROXY << '=' << endl;
		config << FLD_ALL_REQUESTS_TO_PROXY << "=no" << endl;
	}
	if (use_registrar) {
		config << FLD_REGISTRAR << '=' << registrar.encode_noscheme();
		config << endl;
	} else {
		config << FLD_REGISTRAR << '=' << endl;
	}
	config << FLD_REGISTER_AT_STARTUP << '=';
	config << bool2yesno(register_at_startup) << endl;
	config << FLD_REGISTRATION_TIME << '=' << registration_time << endl;
	config << FLD_REG_ADD_QVALUE << '=' << bool2yesno(reg_add_qvalue) << endl;
	config << FLD_REG_QVALUE << '=' << reg_qvalue << endl;
	config << endl;

	// Write AUDIO settings
	config << "# RTP AUDIO\n";
	config << FLD_CODECS << '=';
	for (list<t_audio_codec>::iterator i = codecs.begin();
	     i != codecs.end(); i++)
	{
		if (i != codecs.begin()) config << ',';
		switch(*i) {
		case CODEC_G711_ALAW:
			config << "g711a";
			break;
		case CODEC_G711_ULAW:
			config << "g711u";
			break;
		case CODEC_GSM:
			config << "gsm";
			break;
		case CODEC_SPEEX_NB:
			config << "speex-nb";
			break;
		case CODEC_SPEEX_WB:
			config << "speex-wb";
			break;
		case CODEC_SPEEX_UWB:
			config << "speex-uwb";
			break;
		case CODEC_ILBC:
			config << "ilbc";
			break;
		case CODEC_G726_16:
			config << "g726-16";
			break;
		case CODEC_G726_24:
			config << "g726-24";
			break;
		case CODEC_G726_32:
			config << "g726-32";
			break;
		case CODEC_G726_40:
			config << "g726-40";
			break;
		default:
			assert(false);
		}
	}
	config << endl;
	config << FLD_PTIME << '=' << ptime << endl;
	config << FLD_OUT_FAR_END_CODEC_PREF << '=' << bool2yesno(out_obey_far_end_codec_pref) << endl;
	config << FLD_IN_FAR_END_CODEC_PREF << '=' << bool2yesno(in_obey_far_end_codec_pref) << endl;
	config << FLD_SPEEX_NB_PAYLOAD_TYPE << '=' << speex_nb_payload_type << endl;
	config << FLD_SPEEX_WB_PAYLOAD_TYPE << '=' << speex_wb_payload_type << endl;
	config << FLD_SPEEX_UWB_PAYLOAD_TYPE << '=' << speex_uwb_payload_type << endl;
	config << FLD_SPEEX_BIT_RATE_TYPE << '=';
	// config << FLD_SPEEX_ABR_NB << '=' << speex_abr_nb << endl;
	// config << FLD_SPEEX_ABR_WB << '=' << speex_abr_wb << endl;
	config << bit_rate_type2str(speex_bit_rate_type) << endl;
	config << FLD_SPEEX_DTX << '=' << bool2yesno(speex_dtx) << endl;
	config << FLD_SPEEX_PENH << '=' << bool2yesno(speex_penh) << endl;
	config << FLD_SPEEX_QUALITY << '=' << speex_quality << endl;
	config << FLD_SPEEX_COMPLEXITY << '=' << speex_complexity << endl;
	config << FLD_SPEEX_DSP_VAD << '=' << bool2yesno(speex_dsp_vad) << endl;
	config << FLD_SPEEX_DSP_AGC << '=' << bool2yesno(speex_dsp_agc) << endl;
	config << FLD_SPEEX_DSP_AEC << '=' << bool2yesno(speex_dsp_aec) << endl;
	config << FLD_SPEEX_DSP_NRD << '=' << bool2yesno(speex_dsp_nrd) << endl;
	config << FLD_SPEEX_DSP_AGC_LEVEL << '=' << speex_dsp_agc_level << endl;
	config << FLD_ILBC_PAYLOAD_TYPE << '=' << ilbc_payload_type << endl;
	config << FLD_ILBC_MODE << '=' << ilbc_mode << endl;
	config << FLD_G726_16_PAYLOAD_TYPE << '=' << g726_16_payload_type << endl;
	config << FLD_G726_24_PAYLOAD_TYPE << '=' << g726_24_payload_type << endl;
	config << FLD_G726_32_PAYLOAD_TYPE << '=' << g726_32_payload_type << endl;
	config << FLD_G726_40_PAYLOAD_TYPE << '=' << g726_40_payload_type << endl;
	config << FLD_G726_PACKING << '=' << g726_packing2str(g726_packing) << endl;
	config << FLD_DTMF_TRANSPORT << '=' << dtmf_transport2str(dtmf_transport) << endl;
	config << FLD_DTMF_PAYLOAD_TYPE << '=' << dtmf_payload_type << endl;
	config << FLD_DTMF_DURATION << '=' << dtmf_duration << endl;
	config << FLD_DTMF_PAUSE << '=' << dtmf_pause << endl;
	config << FLD_DTMF_VOLUME << '=' << dtmf_volume << endl;
	config << endl;

	// Write SIP PROTOCOL settings
	config << "# SIP PROTOCOL\n";
	config << FLD_HOLD_VARIANT << '=';
	switch(hold_variant) {
	case HOLD_RFC2543:
		config << "rfc2543";
		break;
	case HOLD_RFC3264:
		config << "rfc3264";
		break;
	default:
		assert(false);
	}
	config << endl;
	config << FLD_CHECK_MAX_FORWARDS << '=';
	config << bool2yesno(check_max_forwards) << endl;
	config << FLD_ALLOW_MISSING_CONTACT_REG << '=';
	config << bool2yesno(allow_missing_contact_reg) << endl;
	config << FLD_REGISTRATION_TIME_IN_CONTACT << '=';
	config << bool2yesno(registration_time_in_contact) << endl;
	config << FLD_COMPACT_HEADERS << '=' << bool2yesno(compact_headers) << endl;
	config << FLD_ENCODE_MULTI_VALUES_AS_LIST << '=';
	config << bool2yesno(encode_multi_values_as_list) << endl;
	config << FLD_USE_DOMAIN_IN_CONTACT << '=';
	config << bool2yesno(use_domain_in_contact) << endl;
	config << FLD_ALLOW_SDP_CHANGE << '=' << bool2yesno(allow_sdp_change) << endl;
	config << FLD_ALLOW_REDIRECTION << '=' << bool2yesno(allow_redirection);
	config << endl;
	config << FLD_ASK_USER_TO_REDIRECT << '=';
	config << bool2yesno(ask_user_to_redirect) << endl;
	config << FLD_MAX_REDIRECTIONS << '=' << max_redirections << endl;
	config << FLD_EXT_100REL << '=' << ext_support2str(ext_100rel) << endl;
	config << FLD_EXT_REPLACES << '=' << bool2yesno(ext_replaces) << endl;
	config << FLD_REFEREE_HOLD << '=' << bool2yesno(referee_hold) << endl;
	config << FLD_REFERRER_HOLD << '=' << bool2yesno(referrer_hold) << endl;
	config << FLD_ALLOW_REFER << '=' << bool2yesno(allow_refer) << endl;
	config << FLD_ASK_USER_TO_REFER << '=';
	config << bool2yesno(ask_user_to_refer) << endl;
	config << FLD_AUTO_REFRESH_REFER_SUB << '=';
	config << bool2yesno(auto_refresh_refer_sub) << endl;
	config << FLD_ATTENDED_REFER_TO_AOR << '=';
	config << bool2yesno(attended_refer_to_aor) << endl;
	config << FLD_ALLOW_XFER_CONSULT_INPROG << '=';
	config << bool2yesno(allow_transfer_consultation_inprog) << endl;
	config << FLD_SEND_P_PREFERRED_ID << '=';
	config << bool2yesno(send_p_preferred_id) << endl;
	config << endl;

	// Write Transport/NAT settings
	config << "# Transport/NAT\n";
	config << FLD_SIP_TRANSPORT << '=' << sip_transport2str(sip_transport) << endl;
	config << FLD_SIP_TRANSPORT_UDP_THRESHOLD << '=' << sip_transport_udp_threshold << endl;
	if (use_nat_public_ip) {
		config << FLD_NAT_PUBLIC_IP << '=' << nat_public_ip << endl;
	} else {
		config << FLD_NAT_PUBLIC_IP << '=' << endl;
	}
	if (use_stun) {
		config << FLD_STUN_SERVER << '=' << 
			stun_server.encode_noscheme() << endl;
	} else {
		config << FLD_STUN_SERVER << '=' << endl;
	}
	config << FLD_PERSISTENT_TCP << '=' << bool2yesno(persistent_tcp) << endl;
	config << FLD_ENABLE_NAT_KEEPALIVE << '=' << bool2yesno(enable_nat_keepalive) << endl;
	config << endl;

	// Write TIMER settings
	config << "# TIMERS\n";
	config << FLD_TIMER_NOANSWER << '=' << timer_noanswer << endl;
	config << FLD_TIMER_NAT_KEEPALIVE << '=' << timer_nat_keepalive << endl;
	config << FLD_TIMER_TCP_PING << '=' << timer_tcp_ping << endl;
	config << endl;

	// Write ADDRESS FORMAT settings
	config << "# ADDRESS FORMAT\n";
	config << FLD_DISPLAY_USERONLY_PHONE << '=';
	config << bool2yesno(display_useronly_phone) << endl;
	config << FLD_NUMERICAL_USER_IS_PHONE << '=';
	config << bool2yesno(numerical_user_is_phone) << endl;
	config << FLD_REMOVE_SPECIAL_PHONE_SYM << '=';
	config << bool2yesno(remove_special_phone_symbols) << endl;
	config << FLD_SPECIAL_PHONE_SYMBOLS << '=' << special_phone_symbols << endl;
	config << FLD_USE_TEL_URI_FOR_PHONE << '=' << bool2yesno(use_tel_uri_for_phone) << endl;
	config << endl;
	
	// Write RING TONE settings
	config << "# RING TONES\n";
	config << FLD_USER_RINGTONE_FILE << '=' << ringtone_file << endl;
	config << FLD_USER_RINGBACK_FILE << '=' << ringback_file << endl;
	config << endl;
	
	// Write script settings
	config << "# SCRIPTS\n";
	config << FLD_SCRIPT_INCOMING_CALL << '=' << script_incoming_call << endl;
	config << FLD_SCRIPT_IN_CALL_ANSWERED << '=' << script_in_call_answered << endl;
	config << FLD_SCRIPT_IN_CALL_FAILED << '=' << script_in_call_failed << endl;
	config << FLD_SCRIPT_OUTGOING_CALL << '=' << script_outgoing_call << endl;
	config << FLD_SCRIPT_OUT_CALL_ANSWERED << '=' << script_out_call_answered << endl;
	config << FLD_SCRIPT_OUT_CALL_FAILED << '=' << script_out_call_failed << endl;
	config << FLD_SCRIPT_LOCAL_RELEASE << '=' << script_local_release << endl;
	config << FLD_SCRIPT_REMOTE_RELEASE << '=' << script_remote_release << endl;
	config << endl;
	
	// Write number conversion rules
	config << "# NUMBER CONVERSION\n";

	for (list<t_number_conversion>::iterator i = number_conversions.begin();
	     i != number_conversions.end(); i++)
	{
		config << FLD_NUMBER_CONVERSION << '=';
		config << escape(i->re.str(), ',');
		config << ',';
		config << escape(i->fmt, ',');
		config << endl;
	}
	config << endl;
	
	// Write security settings
	config << "# SECURITY\n";
	config << FLD_ZRTP_ENABLED << '=' << bool2yesno(zrtp_enabled) << endl;
	config << FLD_ZRTP_GOCLEAR_WARNING << '=' << bool2yesno(zrtp_goclear_warning) << endl;
	config << FLD_ZRTP_SDP << '=' << bool2yesno(zrtp_sdp) << endl;
	config << FLD_ZRTP_SEND_IF_SUPPORTED << '=' << bool2yesno(zrtp_send_if_supported) << endl;
	config << endl;
	
	// Write MWI settings
	config << "# MWI\n";
	config << FLD_MWI_SOLLICITED << '=' << bool2yesno(mwi_sollicited) << endl;
	config << FLD_MWI_USER << '=' << mwi_user << endl;
	if (mwi_server.is_valid()) {
		config << FLD_MWI_SERVER << '=' << mwi_server.encode_noscheme() << endl;
	} else {
		config << FLD_MWI_SERVER << '=' << endl;
	}
	config << FLD_MWI_VIA_PROXY << '=' << bool2yesno(mwi_via_proxy) << endl;
	config << FLD_MWI_SUBSCRIPTION_TIME << '=' << mwi_subscription_time << endl;
	config << FLD_MWI_VM_ADDRESS << '=' << mwi_vm_address << endl;
	config << endl;
	
	config << "# INSTANT MESSAGE\n";
	config << FLD_IM_MAX_SESSIONS << '=' << im_max_sessions << endl;
	config << FLD_IM_SEND_ISCOMPOSING << '=' << bool2yesno(im_send_iscomposing) << endl;
	config << endl;
	
	// Write presence settings
	config << "# PRESENCE\n";
	config << FLD_PRES_SUBSCRIPTION_TIME << '=' << pres_subscription_time << endl;
	config << FLD_PRES_PUBLICATION_TIME << '=' << pres_publication_time << endl;
	config << FLD_PRES_PUBLISH_STARTUP << '=' << bool2yesno(pres_publish_startup) << endl;

	// Check if writing succeeded
	if (!config.good()) {
		// Restore backup
		config.close();
		rename(f_backup.c_str(), f.c_str());

		error_msg = "File system error while writing file ";
		error_msg += f;
		log_file->write_report(error_msg, "t_user::write_config",
			LOG_NORMAL, LOG_CRITICAL);
		mtx_user.unlock();
		return false;
	}

	// Set parser options
	t_parser::check_max_forwards = check_max_forwards;
	t_parser::compact_headers = compact_headers;
	t_parser::multi_values_as_list = encode_multi_values_as_list;

	mtx_user.unlock();
	return true;
}

string t_user::get_filename(void) const {
	string result;
	
	mtx_user.lock();
	result = config_filename;
	mtx_user.unlock();
	
	return result;
}

bool t_user::set_config(string filename) {
	t_mutex_guard guard(mtx_user);
	
	struct stat stat_buf;

	config_filename = filename;
	string fullpath = expand_filename(filename);
	
	return (stat(fullpath.c_str(), &stat_buf) != 0);
}

string t_user::get_profile_name(void) const {
	string result;
	
	mtx_user.lock();
	
	string::size_type pos_ext = config_filename.find(USER_FILE_EXT);

	if (pos_ext == string::npos) {
		result = config_filename;
	} else {
		result = config_filename.substr(0, pos_ext);
	}
	
	mtx_user.unlock();
	
	return result;
}

string t_user::get_contact_name(void) const {
	mtx_user.lock();
	
	string s = name;
	
	// Some broken proxies expect the contact name to be the same
	// as the SIP user name.
	if (!use_domain_in_contact) {
		mtx_user.unlock();
		return s;
	}
	
	// Create a unique contact name from the user name and domain:
	// 
	//   username_domain, where all dots in domain are replace
	//
	// This way it is possible to activate 2 profiles that have the
	// same username, but different domains, e.g.
	//
	//   michel@domainA
	//   michel@domainB

	s += '_';
	
	// Cut of port and/or uri-parameters if present in domain
	string::size_type i = domain.find_first_of(":;");
	if (i != string::npos) {
		// Some broken SIP proxies think that their own address appears
		// in the contact header when they see the domain in the user part.
		// By replacing the dots with underscores Twinkle interoperates
		// with those proxies (yuck).
		s += replace_char(domain.substr(0, i), '.', '_');
	} else {
		s += replace_char(domain, '.', '_');
	}

	mtx_user.unlock();
	return s;
}

string t_user::get_display_uri(void) const {
	mtx_user.lock();
	string s;
	
	s = display;
	if (!s.empty()) s += ' ';
	s += '<';
	s += USER_SCHEME;
	s += ':';
	s += name;
	s += '@';
	s += domain;
	s += '>';
	
	mtx_user.unlock();
	return s;
}

bool t_user::check_required_ext(t_request *r, list<string> &unsupported) const {
	bool all_supported = true;
	
	mtx_user.lock();

	unsupported.clear();
	if (!r->hdr_require.is_populated()) {
		mtx_user.unlock();
		return true;
	}
	
	for (list<string>::iterator i = r->hdr_require.features.begin();
	     i != r->hdr_require.features.end(); i++)
	{
		if (*i == EXT_100REL) {
			if (ext_100rel != EXT_DISABLED) continue;
		} else if (*i == EXT_REPLACES) {
			if (ext_replaces) continue;
		} else if (*i == EXT_NOREFERSUB) {
			continue;
		}

		// Extension is not supported
		unsupported.push_back(*i);
		all_supported = false;
	}

	mtx_user.unlock();
	return all_supported;
}

string t_user::create_user_contact(bool anonymous, const string &auto_ip) {
	string s;
	
	mtx_user.lock();

	s = USER_SCHEME;
	s += ':';
	
	if (!anonymous) {
		s += t_url::escape_user_value(get_contact_name());
		s += '@';
	}
	
	s += USER_HOST(this, auto_ip);

	if (PUBLIC_SIP_PORT(this) != get_default_port(USER_SCHEME)) {
		s += ':';
		s += int2str(PUBLIC_SIP_PORT(this));
	}
	
	if (phone->use_stun(this)) {
		// The port discovered via STUN can only be used for UDP.
		s += ";transport=udp";
	} else {
		// Add transport parameter if a single transport is provisioned only.
		switch (sip_transport) {
		case SIP_TRANS_UDP:
			s += ";transport=udp";
			break;
		case SIP_TRANS_TCP:
			s += ";transport=tcp";
			break;
		default:
			break;
		}
	}

	if (!anonymous && 
	    numerical_user_is_phone && looks_like_phone(name, special_phone_symbols))
	{
		// RFC 3261 19.1.1
		// If the URI contains a telephone number it SHOULD contain
		// the user=phone parameter.
		s += ";user=phone";
	}

	mtx_user.unlock();
	return s;
}

string t_user::create_user_uri(bool anonymous) {
	if (anonymous) return ANONYMOUS_URI;
	
	string s;
	mtx_user.lock();

	s = USER_SCHEME;
	s += ':';
	s += t_url::escape_user_value(name);
	s += '@';
	s += domain;

	if (numerical_user_is_phone && looks_like_phone(name, special_phone_symbols))
	{
		// RFC 3261 19.1.1
		// If the URI contains a telephone number it SHOULD contain
		// the user=phone parameter.
		s += ";user=phone";
	}

	mtx_user.unlock();
	return s;
}

string t_user::convert_number(const string &number, const list<t_number_conversion> &l) const {
	for (list<t_number_conversion>::const_iterator i = l.begin();
	     i != l.end(); i++)
	{
		boost::smatch m;
		
		try {
			if (boost::regex_match(number, m, i->re)) {
				string result = m.format(i->fmt);
			
				log_file->write_header("t_user::convert_number", 
					LOG_NORMAL, LOG_DEBUG);
				log_file->write_raw("Apply conversion: ");
				log_file->write_raw(i->str());
				log_file->write_endl();
				log_file->write_raw(number);
				log_file->write_raw(" converted to ");
				log_file->write_raw(result);
				log_file->write_endl();
				log_file->write_footer();
					
				return result;
			}
		} catch (std::runtime_error) {
			log_file->write_header("t_user::convert_number", 
					LOG_NORMAL, LOG_WARNING);
			log_file->write_raw("Number conversion rule too complex:\n");
			log_file->write_raw("Number: ");
			log_file->write_raw(number);
			log_file->write_endl();
			log_file->write_raw(i->str());
			log_file->write_endl();
			log_file->write_footer();
			
			return number;
		}
	}
	
	// No match found
	return number;
}

string t_user::convert_number(const string &number) const {
	return convert_number(number, number_conversions);
}

t_url t_user::get_mwi_uri(void) const {
	t_url u(mwi_server);
	u.set_user(mwi_user);
	
	return u;
}

bool t_user::is_diamondcard_account(void) const {
	// A profile is a Diamondcard account if the end configured domain
	// is equal to the DIAMONDCARD_DOMAIN
	size_t domain_len = strlen(DIAMONDCARD_DOMAIN);
	if (domain.size() < domain_len) return false;
	
	size_t pos = domain.size() - domain_len;
	return (domain.substr(pos) == DIAMONDCARD_DOMAIN);
}
