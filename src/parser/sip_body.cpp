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

#include "sip_body.h"

#include <list>
#include <cassert>
#include <cstdlib>
#include "log.h"
#include "protocol.h"
#include "sip_message.h"
#include "util.h"
#include "audits/memman.h"
#include "audio/rtp_telephone_event.h"

////////////////////////////////////
// class t_sip_body
////////////////////////////////////

t_sip_body::t_sip_body() {
	invalid = false;
}

bool t_sip_body::local_ip_check(void) const {
	return true;
}

size_t t_sip_body::get_encoded_size(void) const {
	return encode().size();
}

////////////////////////////////////
// class t_sip_xml_body
////////////////////////////////////

void t_sip_body_xml::create_xml_doc(const string &xml_version, const string &charset) {
	clear_xml_doc();
	
	// XML doc
	xml_doc = xmlNewDoc(BAD_CAST xml_version.c_str());
	MEMMAN_NEW(xml_doc);
	xml_doc->encoding = xmlCharStrdup(charset.c_str());
}

void t_sip_body_xml::clear_xml_doc(void) {
	if (xml_doc) {
		MEMMAN_DELETE(xml_doc);
		xmlFreeDoc(xml_doc);
		xml_doc = NULL;
	}
}

void t_sip_body_xml::copy_xml_doc(t_sip_body_xml *to_body) const {
	if (to_body->xml_doc) {
		to_body->clear_xml_doc();
	}
	
	if (xml_doc) {
		to_body->xml_doc = xmlCopyDoc(xml_doc, 1);
		if (!to_body->xml_doc) {
			log_file->write_report("Failed to copy xml document.",
				"t_sip_body_xml::copy",
				LOG_NORMAL, LOG_CRITICAL);
		} else {
			MEMMAN_NEW(to_body->xml_doc);
		}
	}
}

t_sip_body_xml::t_sip_body_xml() : t_sip_body(),
	xml_doc(NULL)
{}

t_sip_body_xml::~t_sip_body_xml() {
	clear_xml_doc();
}

string t_sip_body_xml::encode(void) const {
	if (!xml_doc) {
		t_sip_body_xml *self = const_cast<t_sip_body_xml *>(this);
		self->create_xml_doc();
	}
	assert(xml_doc);
	
	xmlChar *buf;
	int buf_size;
	
	xmlDocDumpMemory(xml_doc, &buf, &buf_size);
	string result((char*)buf);
	xmlFree(buf);
	
	return result;
}

bool t_sip_body_xml::parse(const string &s) {
	assert(xml_doc == NULL);

	xml_doc = xmlReadMemory(s.c_str(), s.size(), "noname.xml", NULL, 
		XML_PARSE_NOBLANKS | XML_PARSE_NOERROR | XML_PARSE_NOWARNING);
	if (!xml_doc) {
		log_file->write_report("Failed to parse xml document.",
			"t_sip_body_xml::parse",
			LOG_NORMAL, LOG_WARNING);
	} else {
		MEMMAN_NEW(xml_doc);
	}
	
	return (xml_doc != NULL);
}

////////////////////////////////////
// class t_sip_body_opaque
////////////////////////////////////

t_sip_body_opaque::t_sip_body_opaque() : t_sip_body()
{}

t_sip_body_opaque::t_sip_body_opaque(string s) : 
	t_sip_body(),
	opaque(s)
{}

string t_sip_body_opaque::encode(void) const {
	return opaque;
}

t_sip_body *t_sip_body_opaque::copy(void) const {
	t_sip_body_opaque *sb =  new t_sip_body_opaque(*this);
	MEMMAN_NEW(sb);
	return sb;
}

t_body_type t_sip_body_opaque::get_type(void) const {
	return BODY_OPAQUE;
}

t_media t_sip_body_opaque::get_media(void) const {
	return t_media("application", "octet-stream");
}

size_t t_sip_body_opaque::get_encoded_size(void) const {
	return opaque.size();
}

////////////////////////////////////
// class t_sip_body_sipfrag
////////////////////////////////////

t_sip_body_sipfrag::t_sip_body_sipfrag(t_sip_message *m) : t_sip_body() {
	sipfrag = m->copy();
}

t_sip_body_sipfrag::~t_sip_body_sipfrag() {
	MEMMAN_DELETE(sipfrag);
	delete sipfrag;
}

string t_sip_body_sipfrag::encode(void) const {
	return sipfrag->encode(false);
}

t_sip_body *t_sip_body_sipfrag::copy(void) const {
	t_sip_body_sipfrag *sb = new t_sip_body_sipfrag(sipfrag);
	MEMMAN_NEW(sb);
	return sb;
}

t_body_type t_sip_body_sipfrag::get_type(void) const {
	return BODY_SIPFRAG;
}

t_media t_sip_body_sipfrag::get_media(void) const {
	return t_media("message", "sipfrag");
}

////////////////////////////////////
// class t_sip_body_dtmf_relay
////////////////////////////////////

t_sip_body_dtmf_relay::t_sip_body_dtmf_relay() : t_sip_body() {
	signal = '0';
	duration = 250;
}

t_sip_body_dtmf_relay::t_sip_body_dtmf_relay(char _signal, uint16 _duration) :
	signal(_signal), duration(_duration)
{}

string t_sip_body_dtmf_relay::encode(void) const {
	string s = "Signal=";
	s += signal;
	s += CRLF;
	
	s += "Duration=";
	s += int2str(duration);
	s += CRLF;
	
	return s;
}

t_sip_body *t_sip_body_dtmf_relay::copy(void) const {
	t_sip_body_dtmf_relay *sb = new t_sip_body_dtmf_relay(*this);
	MEMMAN_NEW(sb);
	return sb;	
}

t_body_type t_sip_body_dtmf_relay::get_type(void) const {
	return BODY_DTMF_RELAY;
}

t_media t_sip_body_dtmf_relay::get_media(void) const {
	return t_media("application", "dtmf-relay");
}

bool t_sip_body_dtmf_relay::parse(const string &s) {
	signal = 0;
	duration = 250;
	
	bool valid = false;
	vector<string> lines = split_linebreak(s);
	
	for (vector<string>::iterator i = lines.begin(); i != lines.end(); i++) {
		string line = trim(*i);
		if (line.empty()) continue;
		
		vector<string> l = split_on_first(line, '=');
		if (l.size() != 2) continue;
		
		string parameter = tolower(trim(l[0]));
		string value = tolower(trim(l[1]));
		
		if (value.empty()) continue;
		
		if (parameter == "signal") {
			if (!VALID_DTMF_SYM(value[0])) return false;
			signal = value[0];
			valid = true;
		} else if (parameter == "duration") {
			duration = atoi(value.c_str());
			if (duration == 0) return false;
		}
	}
	
	return valid;
}

////////////////////////////////////
// class t_sip_body_plain_text
////////////////////////////////////

t_sip_body_plain_text::t_sip_body_plain_text() :
	t_sip_body()
{}

t_sip_body_plain_text::t_sip_body_plain_text(const string &_text) :
	t_sip_body(),
	text(_text)
{}

string t_sip_body_plain_text::encode(void) const {
	return text;
}

t_sip_body *t_sip_body_plain_text::copy(void) const {
	t_sip_body *sb = new t_sip_body_plain_text(*this);
	MEMMAN_NEW(sb);
	return sb;
}

t_body_type t_sip_body_plain_text::get_type(void) const {
	return BODY_PLAIN_TEXT;
}

t_media t_sip_body_plain_text::get_media(void) const {
	return t_media("text", "plain");
}

size_t t_sip_body_plain_text::get_encoded_size(void) const {
	return text.size();
}

////////////////////////////////////
// class t_sip_body_html_text
////////////////////////////////////

t_sip_body_html_text::t_sip_body_html_text(const string &_text) :
	t_sip_body(),
	text(_text)
{}

string t_sip_body_html_text::encode(void) const {
	return text;
}

t_sip_body *t_sip_body_html_text::copy(void) const {
	t_sip_body *sb = new t_sip_body_html_text(*this);
	MEMMAN_NEW(sb);
	return sb;
}

t_body_type t_sip_body_html_text::get_type(void) const {
	return BODY_HTML_TEXT;
}

t_media t_sip_body_html_text::get_media(void) const {
	return t_media("text", "html");
}

size_t t_sip_body_html_text::get_encoded_size(void) const {
	return text.size();
}
