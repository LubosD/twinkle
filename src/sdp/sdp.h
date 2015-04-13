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

// Session description

#ifndef _H_SDP
#define _H_SDP

#include <list>
#include <map>
#include <string>
#include "audio/audio_codecs.h"
#include "parser/sip_body.h"

/** User name to be put in o= line of SDP */
#define SDP_O_USER		"twinkle"

// Audio codec formats
#define SDP_FORMAT_G711_ULAW	0
#define SDP_FORMAT_GSM		3
#define SDP_FORMAT_G711_ALAW	8

// rtpmap values
#define SDP_RTPMAP_G711_ULAW	"PCMU/8000"
#define SDP_RTPMAP_GSM		"GSM/8000"
#define SDP_RTPMAP_G711_ALAW	"PCMA/8000"
#define SDP_RTPMAP_SPEEX_NB	"speex/8000"
#define SDP_RTPMAP_SPEEX_WB	"speex/16000"
#define SDP_RTPMAP_SPEEX_UWB	"speex/32000"
#define SDP_RTPMAP_ILBC		"iLBC/8000"
#define SDP_RTPMAP_G726_16	"G726-16/8000"
#define SDP_RTPMAP_G726_24	"G726-24/8000"
#define SDP_RTPMAP_G726_32	"G726-32/8000"
#define SDP_RTPMAP_G726_40	"G726-40/8000"
#define SDP_RTPMAP_TELEPHONE_EV	"telephone-event/8000"

// Audio codec names
#define SDP_AC_NAME_G711_ULAW		"PCMU"
#define SDP_AC_NAME_G711_ALAW		"PCMA"
#define SDP_AC_NAME_GSM			"GSM"
#define SDP_AC_NAME_SPEEX		"speex"
#define SDP_AC_NAME_ILBC		"iLBC"
#define SDP_AC_NAME_G726_16		"G726-16"
#define SDP_AC_NAME_G726_24		"G726-24"
#define SDP_AC_NAME_G726_32		"G726-32"
#define SDP_AC_NAME_G726_40		"G726-40"
#define SDP_AC_NAME_TELEPHONE_EV	"telephone-event"

// Check on fmtp parameter values
#define VALID_ILBC_MODE(mode) ((mode) == 20 || (mode == 30))

using namespace std;

enum t_sdp_ntwk_type {
	SDP_NTWK_NULL,
	SDP_NTWK_IN
};

string sdp_ntwk_type2str(t_sdp_ntwk_type n);
t_sdp_ntwk_type str2sdp_ntwk_type(string s);


enum t_sdp_addr_type {
	SDP_ADDR_NULL,
	SDP_ADDR_IP4,
	SDP_ADDR_IP6
};

string sdp_addr_type2str(t_sdp_addr_type a);
t_sdp_addr_type str2sdp_addr_type(string s);


/** Transport protocol */
enum t_sdp_transport {
	SDP_TRANS_RTP,		/**< RTP/AVP */
	SDP_TRANS_UDP,		/**< UDP */
	SDP_TRANS_OTHER		/**< Another protocol not yet supported */
};

string sdp_transport2str(t_sdp_transport t);
t_sdp_transport str2sdp_transport(string s);


enum t_sdp_media_direction {
	SDP_INACTIVE,
	SDP_SENDONLY,
	SDP_RECVONLY,
	SDP_SENDRECV
};

string sdp_media_direction2str(t_sdp_media_direction d);


enum t_sdp_media_type {
	SDP_AUDIO,
	SDP_VIDEO,
	SDP_OTHER
};

t_sdp_media_type str2sdp_media_type(string s);
string sdp_media_type2str(t_sdp_media_type m);


class t_sdp_origin {
public:
	string			username;
	string			session_id;
	string			session_version;
	t_sdp_ntwk_type		network_type;
	t_sdp_addr_type		address_type;
	string			address;

	t_sdp_origin();
	t_sdp_origin(string _username, string _session_id,
		     string _session_version, string _address);

	string encode(void) const;
};

class t_sdp_connection {
public:
	t_sdp_ntwk_type		network_type;
	t_sdp_addr_type		address_type;
	string			address;

	t_sdp_connection();
	t_sdp_connection(string _address);
	string encode(void) const;
};

class t_sdp_attr {
public:
	string			name;
	string			value;

	t_sdp_attr(string _name);
	t_sdp_attr(string _name, string _value);
	string encode(void) const;
};

/** 
 * Media definition.
 * The data from an m= line and associated a= lines.
 */
class t_sdp_media {
private:
	/** Dynamic payload type for DTMF */
	unsigned short		format_dtmf;

public:
	/** The media type, e.g. audio, video */
	string			media_type;
	
	/** Port to receive media */
	unsigned short		port;
	
	/** Transport protocol, e.g. RTP/AVP */
	string			transport;
	
	/** 
	 * @name Media formats 
	 * Depending on the media type, formats are in numeric format or
	 * alpha numeric format. Only one of the following formats will
	 * be populated.
	 */
	//@{
	/** Media formats in numeric form, i.e. audio codecs */
	list<unsigned short>	formats;
	
	/** Media formats in alpha numeric form. */
	list<string>		alpha_num_formats;
	//@}
	
	/** Optional connection information if not specified on global level. */
	t_sdp_connection	connection;
	
	/** Attributes (a= lines) */
	list <t_sdp_attr>	attributes;

	t_sdp_media();
	t_sdp_media(t_sdp_media_type _media_type,
		    unsigned short _port, const list<t_audio_codec> &_formats,
	            unsigned short _format_dtmf, 
	            const map<t_audio_codec, unsigned short> &ac2format);

	string encode(void) const;
	void add_format(unsigned short f, t_audio_codec codec);
	t_sdp_attr *get_attribute(const string &name);
	list<t_sdp_attr *>get_attributes(const string &name);
	t_sdp_media_direction get_direction(void) const;
	t_sdp_media_type get_media_type(void) const;
	t_sdp_transport get_transport(void) const;
};

class t_sdp : public t_sip_body {
public:
	unsigned short		version;
	t_sdp_origin		origin;
	string			session_name;
	t_sdp_connection	connection;
	list<t_sdp_attr>	attributes;
	list<t_sdp_media>	media;

	t_sdp();

	// Create SDP with a single audio media stream
	t_sdp(const string &user, const string &sess_id, const string &sess_version, 
	      const string &user_host, const string &media_host, unsigned short media_port,
	      const list<t_audio_codec> &formats, unsigned short format_dtmf,
	      const map<t_audio_codec, unsigned short> &ac2format);

	// Create SDP without media streams
	t_sdp(const string &user, const string &sess_id, const string &sess_version, 
		const string &user_host, const string &media_host);

	// Add media stream
	void add_media(const t_sdp_media &m);

	string encode(void) const;
	t_sip_body *copy(void) const;
	t_body_type get_type(void) const;
	t_media get_media(void) const;

	// Return true if the current SDP is supported:
	// version is 0
	// 1 audio stream RTP
	// IN IP4 addressing
	// connection at session level only
	// If false is returned, then a warning code and text is returned.
	bool is_supported(int &warn_code, string &warn_text) const;

	// Get/set codec/rtp info for first media stream having a non-zero
	// value for the port of the given media type
	string get_rtp_host(t_sdp_media_type media_type) const;
	unsigned short get_rtp_port(t_sdp_media_type media_type) const;
	list<unsigned short> get_codecs(t_sdp_media_type media_type) const;

	// Get codec description from rtpmap
	string get_codec_description(t_sdp_media_type media_type,
			unsigned short codec) const;
	t_audio_codec get_rtpmap_codec(const string &rtpmap) const;
	t_audio_codec get_codec(t_sdp_media_type media_type,
			unsigned short codec) const;
	t_sdp_media_direction get_direction(t_sdp_media_type media_type) const;
	
	// Get ftmp attribute
	string get_fmtp(t_sdp_media_type media_type, unsigned short codec) const;
	
	// Get a specific parameter from fmtp, assuming the fmtp string is a list
	// of paramter=value strings separated by semi-colons
	// Returns -1 on failure
	int get_fmtp_int_param(t_sdp_media_type media_type, unsigned short codec,
			const string param) const;

	// Get ptime. Returns 0 if ptime is not present
	unsigned short get_ptime(t_sdp_media_type media_type) const;
	
	bool get_zrtp_support(t_sdp_media_type media_type) const;
	
	void set_ptime(t_sdp_media_type media_type, unsigned short ptime);
	void set_direction(t_sdp_media_type media_type, t_sdp_media_direction direction);
	void set_fmtp(t_sdp_media_type media_type, unsigned short codec, const string &fmtp);
	void set_fmtp_int_param(t_sdp_media_type media_type, unsigned short codec,
			const string &param, int value);
	void set_zrtp_support(t_sdp_media_type media_type);

	// Returns a pointer to the first media stream in the list of media
	// streams having a non-zero port value for the give media type.
	// Returns NULL if no such media stream can be found.
	const t_sdp_media *get_first_media(t_sdp_media_type media_type) const;
	
	/**
	 * Check if all local IP address are correctly filled in. This
	 * check is an integrity check to help debugging the auto IP
	 * discover feature.
	 */
	virtual bool local_ip_check(void) const;
};

#endif
