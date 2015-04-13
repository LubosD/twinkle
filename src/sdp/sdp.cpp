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

#include <algorithm>
#include <assert.h>
#include <cstdlib>
#include <iostream>
#include "protocol.h"
#include "sdp_parse_ctrl.h"
#include "sdp.h"
#include "util.h"
#include "parser/hdr_warning.h"
#include "parser/parameter.h"
#include "audits/memman.h"

using namespace std;

string sdp_ntwk_type2str(t_sdp_ntwk_type n) {
	switch(n) {
	case SDP_NTWK_NULL:	return "NULL";
	case SDP_NTWK_IN:	return "IN";
	default:
		assert(false);
	}
}

t_sdp_ntwk_type str2sdp_ntwk_type(string s) {
	if (s == "IN") return SDP_NTWK_IN;

	throw (t_sdp_syntax_error("unknown network type: " + s));
}

string sdp_addr_type2str(t_sdp_addr_type a) {
	switch(a) {
	case SDP_ADDR_NULL:	return "NULL";
	case SDP_ADDR_IP4:	return "IP4";
	case SDP_ADDR_IP6:	return "IP6";
	default:
		assert(false);
	}
}

t_sdp_addr_type str2sdp_addr_type(string s) {
	if (s == "IP4") return SDP_ADDR_IP4;
	if (s == "IP6") return SDP_ADDR_IP6;

	throw (t_sdp_syntax_error("unknown address type: " + s));
}

string sdp_transport2str(t_sdp_transport t) {
	switch(t) {
	case SDP_TRANS_RTP:	return "RTP/AVP";
	case SDP_TRANS_UDP:	return "udp";
	default:
		assert(false);
	}
}

t_sdp_transport str2sdp_transport(string s) {
	if (s == "RTP/AVP") return SDP_TRANS_RTP;
	if (s == "udp") return SDP_TRANS_UDP;

	// Other transports are not recognized and are mapped to other.
	return SDP_TRANS_OTHER;
}

t_sdp_media_type str2sdp_media_type(string s) {
	if (s == "audio") return SDP_AUDIO;
	if (s == "video") return SDP_VIDEO;
	return SDP_OTHER;
}

string sdp_media_type2str(t_sdp_media_type m) {
	switch(m) {
	case SDP_AUDIO:		return "audio";
	case SDP_VIDEO:		return "video";
	default:
		assert(false);
	}
}

string get_rtpmap(unsigned format, t_audio_codec codec) {
	string rtpmap;
	
	rtpmap = int2str(format);
	rtpmap += ' ';

	switch(codec) {
	case CODEC_G711_ULAW:
		rtpmap += SDP_RTPMAP_G711_ULAW;
		break;
	case CODEC_G711_ALAW:
		rtpmap += SDP_RTPMAP_G711_ALAW;
		break;
	case CODEC_GSM:
		rtpmap += SDP_RTPMAP_GSM;
		break;
	case CODEC_SPEEX_NB:
		rtpmap += SDP_RTPMAP_SPEEX_NB;
		break;
	case CODEC_SPEEX_WB:
		rtpmap += SDP_RTPMAP_SPEEX_WB;
		break;
	case CODEC_SPEEX_UWB:
		rtpmap += SDP_RTPMAP_SPEEX_UWB;
		break;
	case CODEC_ILBC:
		rtpmap += SDP_RTPMAP_ILBC;
		break;
	case CODEC_G726_16:
		rtpmap += SDP_RTPMAP_G726_16;
		break;
	case CODEC_G726_24:
		rtpmap += SDP_RTPMAP_G726_24;
		break;
	case CODEC_G726_32:
		rtpmap += SDP_RTPMAP_G726_32;
		break;
	case CODEC_G726_40:
		rtpmap += SDP_RTPMAP_G726_40;
		break;
	case CODEC_TELEPHONE_EVENT:
		rtpmap += SDP_RTPMAP_TELEPHONE_EV;
		break;
	default:
		assert(false);
	}

	return rtpmap;
}

string sdp_media_direction2str(t_sdp_media_direction d) {
	switch(d) {
	case SDP_INACTIVE:	return "inactive";
	case SDP_SENDONLY:	return "sendonly";
	case SDP_RECVONLY:	return "recvonly";
	case SDP_SENDRECV:	return "sendrecv";
	default:
		assert(false);
	}
}

///////////////////////////////////
// class t_sdp_origin
///////////////////////////////////

t_sdp_origin::t_sdp_origin() {
	network_type = SDP_NTWK_NULL;
	address_type = SDP_ADDR_NULL;
}

t_sdp_origin::t_sdp_origin(string _username, string _session_id,
		           string _session_version, string _address) :
		username(_username),
		session_id(_session_id),
		session_version(_session_version),
		address(_address)
{
	network_type = SDP_NTWK_IN;
	address_type = SDP_ADDR_IP4;
}

string t_sdp_origin::encode(void) const {
	string s;

	s = "o=";
	s += username;
	s += ' ' + session_id;
	s += ' ' + session_version;
	s += ' ' + sdp_ntwk_type2str(network_type);
	s += ' ' + sdp_addr_type2str(address_type);
	s += ' ' + address;
	s += CRLF;

	return s;
}


///////////////////////////////////
// class t_sdp_connection
///////////////////////////////////

t_sdp_connection::t_sdp_connection() {
	network_type = SDP_NTWK_NULL;
}

t_sdp_connection::t_sdp_connection(string _address) :
		address(_address)
{
	network_type = SDP_NTWK_IN;
	address_type = SDP_ADDR_IP4;
}

string t_sdp_connection::encode(void) const {
	string s;

	s = "c=";
	s += sdp_ntwk_type2str(network_type);
	s += ' ' + sdp_addr_type2str(address_type);
	s += ' ' + address;
	s += CRLF;

	return s;
}


///////////////////////////////////
// class t_sdp_attr
///////////////////////////////////

t_sdp_attr::t_sdp_attr(string _name) {
	name = _name;
}

t_sdp_attr::t_sdp_attr(string _name, string _value) {
	name = _name;
	value = _value;
}

string t_sdp_attr::encode(void) const {
	string s;

	s = "a=";
	s += name;

	if (value != "") {
		s += ':' + value;
	}
	
	s += CRLF;

	return s;
}


///////////////////////////////////
// class t_sdp_media
///////////////////////////////////

t_sdp_media::t_sdp_media() {
	port = 0;
	format_dtmf = 0;
}

t_sdp_media::t_sdp_media(t_sdp_media_type _media_type,
			 unsigned short _port, const list<t_audio_codec> &_formats,
			 unsigned short _format_dtmf,
			 const map<t_audio_codec, unsigned short> &ac2format)
{
	media_type = sdp_media_type2str(_media_type);
	port = _port;
	transport = sdp_transport2str(SDP_TRANS_RTP);
	format_dtmf = _format_dtmf;

	for (list<t_audio_codec>::const_iterator i = _formats.begin();
	     i != _formats.end(); i++)
	{
		map<t_audio_codec, unsigned short>::const_iterator it;
		it = ac2format.find(*i);
		assert(it != ac2format.end());
		add_format(it->second, *i);
	}

	if (format_dtmf > 0) {
		add_format(format_dtmf, CODEC_TELEPHONE_EVENT);
	}
}

string t_sdp_media::encode(void) const {
	string s;

	s = "m=";
	s += media_type;
	s += ' ' + int2str(port);
	s += ' ' + transport;

	// Encode media formats. Note that only one of the format lists
	// will be populated depending on the media type.
	
	// Numeric formats.
	for (list<unsigned short>::const_iterator i = formats.begin();
	     i != formats.end(); ++i)
	{
		s += ' ' + int2str(*i);
	}
	
	// Alpha numeric formats.
	for (list<string>::const_iterator i = alpha_num_formats.begin();
	     i != alpha_num_formats.end(); ++i)
	{
		s += ' ' + *i;
	}

	s += CRLF;

	// Connection information.
	if (connection.network_type != SDP_NTWK_NULL) {
		s += connection.encode();
	}

	// Attributes.
	for (list<t_sdp_attr>::const_iterator i = attributes.begin();
	     i != attributes.end(); ++i)
	{
		s += i->encode();
	}

	return s;
}

void t_sdp_media::add_format(unsigned short f, t_audio_codec codec) {
	formats.push_back(f);

	// RFC 3264 5.1
	// All media descriptions SHOULD contain an rtpmap
	string rtpmap = get_rtpmap(f, codec);
	attributes.push_back(t_sdp_attr("rtpmap", rtpmap));

	// RFC 2833 3.9
	// Add fmtp parameter
	if (format_dtmf > 0 && f == format_dtmf) {
		string fmtp = int2str(f);
		fmtp += ' ';
		fmtp += "0-15";
		attributes.push_back(t_sdp_attr("fmtp", fmtp));
	}
}

t_sdp_attr *t_sdp_media::get_attribute(const string &name) {
	for (list<t_sdp_attr>::iterator i = attributes.begin();
	     i != attributes.end(); i++)
	{
		if (cmp_nocase(i->name, name) == 0) return &(*i);
	}

	// Attribute does not exist
	return NULL;
}

list<t_sdp_attr *> t_sdp_media::get_attributes(const string &name) {
	list<t_sdp_attr *> l;

	for (list<t_sdp_attr>::iterator i = attributes.begin();
	     i != attributes.end(); i++)
	{
		if (cmp_nocase(i->name, name) == 0) l.push_back(&(*i));
	}

	return l;
}

t_sdp_media_direction t_sdp_media::get_direction(void) const {
	t_sdp_attr *a;

	t_sdp_media *self = const_cast<t_sdp_media *>(this);

	a = self->get_attribute("inactive");
	if (a) return SDP_INACTIVE;

	a = self->get_attribute("sendonly");
	if (a) return SDP_SENDONLY;

	a = self->get_attribute("recvonly");
	if (a) return SDP_RECVONLY;

	return SDP_SENDRECV;
}

t_sdp_media_type t_sdp_media::get_media_type(void) const {
	return str2sdp_media_type(media_type);
}

t_sdp_transport t_sdp_media::get_transport(void) const {
	return str2sdp_transport(transport);
}

///////////////////////////////////
// class t_sdp
///////////////////////////////////

t_sdp::t_sdp() : t_sip_body(), version(0) 
{}

t_sdp::t_sdp(const string &user, const string &sess_id, const string &sess_version, 
	     const string &user_host, const string &media_host, unsigned short media_port,
	     const list<t_audio_codec> &formats, unsigned short format_dtmf,
	     const map<t_audio_codec, unsigned short> &ac2format) :
	     	t_sip_body(),
	     	version(0),
		origin(user, sess_id, sess_version, user_host),
		connection(media_host)
{
	media.push_back(t_sdp_media(SDP_AUDIO, media_port, formats, format_dtmf,
				ac2format));
}

t_sdp::t_sdp(const string &user, const string &sess_id, const string &sess_version, 
		const string &user_host, const string &media_host) :
			t_sip_body(),
			version(0),
			origin(user, sess_id, sess_version, user_host),
			connection(media_host)
{}

void t_sdp::add_media(const t_sdp_media &m) {
	media.push_back(m);
}

string t_sdp::encode(void) const {
	string s;

	s = "v=" + int2str(version) + CRLF;
	s += origin.encode();

	if (session_name == "") {
		// RFC 3264 5
		// Session name may no be empty. Recommende is '-'
		s += "s=-";
		s += CRLF;
	} else {
		s += "s=" + session_name + CRLF;
	}

	if (connection.network_type != SDP_NTWK_NULL) {
		s += connection.encode();
	}

	// RFC 3264 5
	// Time parameter should be 0 0
	s += "t=0 0";
	s += CRLF;

	for (list<t_sdp_attr>::const_iterator i = attributes.begin();
	     i != attributes.end(); i++)
	{
		s += i->encode();
	}

	for (list<t_sdp_media>::const_iterator i = media.begin();
	     i != media.end(); i++)
	{
		s += i->encode();
	}

	return s;
}

t_sip_body *t_sdp::copy(void) const {
	t_sdp *s = new t_sdp(*this);
	MEMMAN_NEW(s);
	return s;
}

t_body_type t_sdp::get_type(void) const {
	return BODY_SDP;
}

t_media t_sdp::get_media(void) const {
	return t_media("application", "sdp");
}

bool t_sdp::is_supported(int &warn_code, string &warn_text) const {
	warn_text = "";

	if (version != 0) {
		warn_code = W_399_MISCELLANEOUS;
		warn_text = "SDP version ";
		warn_text += int2str(version);
		warn_text += " not supported";
		return false;
	}

	const t_sdp_media *m = get_first_media(SDP_AUDIO);

	// Connection information must be present at the session level
	// and/or the media level
	if (connection.network_type == SDP_NTWK_NULL) {
		if (m == NULL || m->connection.network_type == SDP_NTWK_NULL) {
			warn_code = W_399_MISCELLANEOUS;
			warn_text = "c-line missing";
			return false;
		}
	} else {
		// Only Internet is supported
		if (connection.network_type != SDP_NTWK_IN) {
			warn_code = W_300_INCOMPATIBLE_NWK_PROT;
			return false;
		}

		// Only IPv4 is supported
		if (connection.address_type != SDP_ADDR_IP4) {
			warn_code = W_301_INCOMPATIBLE_ADDR_FORMAT;
			return false;
		}
	}
	
	// There must be at least 1 audio stream with a non-zero port value
	if (m == NULL && !media.empty()) {
		warn_code = W_304_MEDIA_TYPE_NOT_AVAILABLE;
		warn_text = "Valid media stream for audio is missing";
		return false;
	}
	
	// RFC 3264 5, RFC 3725 flow IV
	// There may be 0 media streams
	if (media.empty()) {
		return true;
	}

	// Check connection information on media level
	if (m->connection.network_type != SDP_NTWK_NULL &&
	    m->connection.address_type != SDP_ADDR_IP4) {
		warn_code = W_301_INCOMPATIBLE_ADDR_FORMAT;
		return false;
	}

	if (m->get_transport() != SDP_TRANS_RTP) {
		warn_code = W_302_INCOMPATIBLE_TRANS_PROT;
		return false;
	}

	t_sdp_media *m2 = const_cast<t_sdp_media *>(m);
	const t_sdp_attr *a = m2->get_attribute("ptime");
	if (a) {
		unsigned short p = atoi(a->value.c_str());
		if (p < MIN_PTIME) {
			warn_code = W_306_ATTRIBUTE_NOT_UNDERSTOOD;
			warn_text = "Attribute 'ptime' too small. must be >= ";
			warn_text += int2str(MIN_PTIME);
			return false;
		}

		if (p > MAX_PTIME) {
			warn_code = W_306_ATTRIBUTE_NOT_UNDERSTOOD;
			warn_text = "Attribute 'ptime' too big. must be <= ";
			warn_text += int2str(MAX_PTIME);
			return false;
		}

	}

	return true;
}

string t_sdp::get_rtp_host(t_sdp_media_type media_type) const {
	const t_sdp_media *m = get_first_media(media_type);
	assert(m != NULL);

	// If the media line has its own connection information, then
	// take the host information from there.
	if (m->connection.network_type == SDP_NTWK_IN) {
		return m->connection.address;
	}

	// The host information must be in the session connection info
	assert(connection.network_type == SDP_NTWK_IN);
	return connection.address;
}

unsigned short t_sdp::get_rtp_port(t_sdp_media_type media_type) const {
	const t_sdp_media *m = get_first_media(media_type);
	assert(m != NULL);

	return m->port;
}

list<unsigned short> t_sdp::get_codecs(t_sdp_media_type media_type) const {
	const t_sdp_media *m = get_first_media(media_type);
	assert(m != NULL);

	return m->formats;
}

string t_sdp::get_codec_description(t_sdp_media_type media_type,
		unsigned short codec) const
{
	t_sdp_media *m = const_cast<t_sdp_media *>(get_first_media(media_type));
	assert(m != NULL);

	const list<t_sdp_attr *> attrs = m->get_attributes("rtpmap");
	if (attrs.empty()) return "";

	for (list<t_sdp_attr *>::const_iterator i = attrs.begin();
	     i != attrs.end(); i++)
	{
		vector<string> l = split_ws((*i)->value);
		if (atoi(l.front().c_str()) == codec) {
			return l.back();
		}
	}

	return "";
}

t_audio_codec t_sdp::get_rtpmap_codec(const string &rtpmap) const {
	if (rtpmap.empty()) return CODEC_NULL;
	
	vector<string> rtpmap_elems = split(rtpmap, '/');
	if (rtpmap_elems.size() < 2) {
		// RFC 2327	
		// The rtpmap should at least contain the encoding name
		// and sample rate
		return CODEC_UNSUPPORTED;
	}
		
	string codec_name = trim(rtpmap_elems[0]);
	int sample_rate = atoi(trim(rtpmap_elems[1]).c_str());
	
	if (cmp_nocase(codec_name, SDP_AC_NAME_G711_ULAW) == 0 && sample_rate == 8000) {
		return CODEC_G711_ULAW;
	} else if (cmp_nocase(codec_name, SDP_AC_NAME_G711_ALAW) == 0 && sample_rate == 8000) {
		return CODEC_G711_ALAW;
	} else if (cmp_nocase(codec_name, SDP_AC_NAME_GSM) == 0 && sample_rate == 8000) {
		return CODEC_GSM;
	} else if (cmp_nocase(codec_name, SDP_AC_NAME_SPEEX) == 0 && sample_rate == 8000) {
		return CODEC_SPEEX_NB;
	} else if (cmp_nocase(codec_name, SDP_AC_NAME_SPEEX) == 0 && sample_rate == 16000) {
		return CODEC_SPEEX_WB;
	} else if (cmp_nocase(codec_name, SDP_AC_NAME_SPEEX) == 0 && sample_rate == 32000) {
		return CODEC_SPEEX_UWB;
	} else if (cmp_nocase(codec_name, SDP_AC_NAME_ILBC) == 0 && sample_rate == 8000) {
		return CODEC_ILBC;
	} else if (cmp_nocase(codec_name, SDP_AC_NAME_G726_16) == 0 && sample_rate == 8000) {
		return CODEC_G726_16;
	} else if (cmp_nocase(codec_name, SDP_AC_NAME_G726_24) == 0 && sample_rate == 8000) {
		return CODEC_G726_24;
	} else if (cmp_nocase(codec_name, SDP_AC_NAME_G726_32) == 0 && sample_rate == 8000) {
		return CODEC_G726_32;
	} else if (cmp_nocase(codec_name, SDP_AC_NAME_G726_40) == 0 && sample_rate == 8000) {
		return CODEC_G726_40;
	} else if (cmp_nocase(codec_name, SDP_AC_NAME_TELEPHONE_EV) == 0) {
		return CODEC_TELEPHONE_EVENT;
	}
	
	return CODEC_UNSUPPORTED;
}

t_audio_codec t_sdp::get_codec(t_sdp_media_type media_type,
		unsigned short codec) const
{
	string rtpmap = get_codec_description(media_type, codec);
	
	// If there is no rtpmap description then use the static
	// payload definition as defined by RFC 3551
	if (rtpmap.empty()) {
		switch(codec) {
		case SDP_FORMAT_G711_ULAW:
			return CODEC_G711_ULAW;
		case SDP_FORMAT_G711_ALAW:
			return CODEC_G711_ALAW;
		case SDP_FORMAT_GSM:
			return CODEC_GSM;
		default:
			return CODEC_UNSUPPORTED;
		}
	}
	
	// Use the rtpmap description to map the payload number
	// to a codec
	return get_rtpmap_codec(rtpmap);
}

t_sdp_media_direction t_sdp::get_direction(t_sdp_media_type media_type) const {
	const t_sdp_media *m = get_first_media(media_type);
	assert(m != NULL);

	return m->get_direction();
}

string t_sdp::get_fmtp(t_sdp_media_type media_type, unsigned short codec) const {
	t_sdp_media *m = const_cast<t_sdp_media *>(get_first_media(media_type));
	assert(m != NULL);

	const list<t_sdp_attr *> attrs = m->get_attributes("fmtp");
	if (attrs.empty()) return "";

	for (list<t_sdp_attr *>::const_iterator i = attrs.begin();
	     i != attrs.end(); i++)
	{
		vector<string> l = split_ws((*i)->value);
		if (atoi(l.front().c_str()) == codec) {
			return l.back();
		}
	}

	return "";
}

int t_sdp::get_fmtp_int_param(t_sdp_media_type media_type, unsigned short codec,
			const string param) const
{
	string fmtp = get_fmtp(media_type, codec);
	if (fmtp.empty()) return -1;
	
	int value;
	list<t_parameter> l = str2param_list(fmtp);
	list<t_parameter>::const_iterator it = find(l.begin(), l.end(), t_parameter(param, ""));
	if (it != l.end()) {
		value = atoi(it->value.c_str());
	} else {
		value = -1;
	}
	
	return value;
}

unsigned short t_sdp::get_ptime(t_sdp_media_type media_type) const {
	t_sdp_media *m = const_cast<t_sdp_media *>(get_first_media(media_type));
	assert(m != NULL);

	const t_sdp_attr *a = m->get_attribute("ptime");
	if (!a) return 0;
	return atoi(a->value.c_str());
}

bool t_sdp::get_zrtp_support(t_sdp_media_type media_type) const {
	t_sdp_media *m = const_cast<t_sdp_media *>(get_first_media(media_type));
	assert(m != NULL);
	
	const t_sdp_attr *a = m->get_attribute("zrtp");
	if (!a) return false;
	return true;
}

void t_sdp::set_ptime(t_sdp_media_type media_type, unsigned short ptime) {
	t_sdp_media *m = const_cast<t_sdp_media *>(get_first_media(media_type));
	assert(m != NULL);

	t_sdp_attr a("ptime", int2str(ptime));
	m->attributes.push_back(a);
}

void t_sdp::set_direction(t_sdp_media_type media_type, t_sdp_media_direction direction) {
	t_sdp_media *m = const_cast<t_sdp_media *>(get_first_media(media_type));
	assert(m != NULL);

	t_sdp_attr a(sdp_media_direction2str(direction));
	m->attributes.push_back(a);
}

void t_sdp::set_fmtp(t_sdp_media_type media_type, unsigned short codec, const string &fmtp) {
	t_sdp_media *m = const_cast<t_sdp_media *>(get_first_media(media_type));
	assert(m != NULL);
	
	string s = int2str(codec);
	s += ' ';
	s += fmtp;
	t_sdp_attr a("fmtp", s);
	m->attributes.push_back(a);
}

void t_sdp::set_fmtp_int_param(t_sdp_media_type media_type, unsigned short codec,
			const string &param, int value)
{
	string fmtp(param);
	fmtp += '=';
	fmtp += int2str(value);
	set_fmtp(media_type, codec, fmtp);
}

void t_sdp::set_zrtp_support(t_sdp_media_type media_type) {
	t_sdp_media *m = const_cast<t_sdp_media *>(get_first_media(media_type));
	assert(m != NULL);
	
	t_sdp_attr a("zrtp");
	m->attributes.push_back(a);
}

const t_sdp_media *t_sdp::get_first_media(t_sdp_media_type media_type) const {
	for (list<t_sdp_media>::const_iterator i = media.begin();
	     i != media.end(); i++)
	{
		if (i->get_media_type() == media_type && i->port != 0) {
			return &(*i);
		}
	}

	return NULL;
}

bool t_sdp::local_ip_check(void) const {
	if (origin.address == AUTO_IP4_ADDRESS) return false;
	
	if (connection.address == AUTO_IP4_ADDRESS) return false;
	
	for (list<t_sdp_media>::const_iterator it = media.begin(); it != media.end(); ++it) {
		if (it->connection.address == AUTO_IP4_ADDRESS) return false;
	}
	
	return true;
}
