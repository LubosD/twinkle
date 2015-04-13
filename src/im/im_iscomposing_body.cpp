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

#include "im_iscomposing_body.h"

#include <cassert>
#include <libxml/parser.h>

#include "log.h"
#include "util.h"
#include "audits/memman.h"

#define IM_ISCOMPOSING_NAMESPACE		"urn:ietf:params:xml:ns:im-iscomposing"

#define IS_IM_ISCOMPOSING_TAG(node, tag)	IS_XML_TAG(node, tag, IM_ISCOMPOSING_NAMESPACE)
				
#define IS_IM_ISCOMPOSING_ATTR(attr, attr_name) IS_XML_ATTR(attr, attr_name, IM_ISCOMPOSING_NAMESPACE)

bool t_im_iscomposing_xml_body::extract_data(void) {
	assert(xml_doc);
	
	state_.clear();
	refresh_ = 0;

	xmlNode *root_element = NULL;
	
	// Get root
	root_element = xmlDocGetRootElement(xml_doc);
	if (!root_element) {
		log_file->write_report("im-iscomposing document has no root element.",
			"t_im_iscomposing_xml_body::extract_data",
			LOG_NORMAL, LOG_WARNING);
		return false;
	}
	
	// Check if root is <isComposing>
	if (!IS_IM_ISCOMPOSING_TAG(root_element, "isComposing")) {
		log_file->write_report("im-iscomposing document has invalid root element.",
			"t_im_iscomposing_xml_body::extract_data",
			LOG_NORMAL, LOG_WARNING);
		return false;
	}
	
	xmlNode *child = root_element->children;
	
	// Process children of root.
	bool state_present = false;
	
	for (xmlNode *cur_node = child; cur_node; cur_node = cur_node->next) {
		if (IS_IM_ISCOMPOSING_TAG(cur_node, "state")) {
			state_present = process_node_state(cur_node);
		} else if (IS_IM_ISCOMPOSING_TAG(cur_node, "refresh")) {
			process_node_refresh(cur_node);
		}
	}
	
	// The state node is mandatory, so return only true if it is present.
	return state_present;
}

bool t_im_iscomposing_xml_body::process_node_state(xmlNode *node) {
	assert(node);
	
	xmlNode *child = node->children;
	if (child && child->type == XML_TEXT_NODE) {
		state_ = tolower((char*)child->content);
	} else {
		log_file->write_report("<state> element has no content.",
			"t_im_iscomposing_xml_body::process_node_state",
			LOG_NORMAL, LOG_WARNING);
			
		return false;
	}
	
	return true;
}

void t_im_iscomposing_xml_body::process_node_refresh(xmlNode *node) {
	assert(node);
	
	xmlNode *child = node->children;
	if (child && child->type == XML_TEXT_NODE) {
		refresh_ = atoi((char*)child->content);
	} else {
		log_file->write_report("<refresh> element has no content.",
			"t_im_iscomposing_xml_body::process_node_refresh",
			LOG_NORMAL, LOG_WARNING);
	}
}

void t_im_iscomposing_xml_body::create_xml_doc(
		const string &xml_version, 
		const string &charset) 
{
	t_sip_body_xml::create_xml_doc(xml_version, charset);
	
	// isComposing
	xmlNode *node_iscomposing = xmlNewNode(NULL, BAD_CAST "isComposing");
	xmlNs *ns_im_iscomposing = xmlNewNs(node_iscomposing, BAD_CAST IM_ISCOMPOSING_NAMESPACE, NULL);
	xmlDocSetRootElement(xml_doc, node_iscomposing);
	
	// state
	xmlNewChild(node_iscomposing, ns_im_iscomposing, 
			BAD_CAST "state", 
			BAD_CAST state_.c_str());
			
	// refresh
	if (refresh_ > 0) {
		xmlNewChild(node_iscomposing, ns_im_iscomposing,
				BAD_CAST "refresh",
				BAD_CAST int2str(refresh_).c_str());
	}
}

t_im_iscomposing_xml_body::t_im_iscomposing_xml_body() : t_sip_body_xml (),
	state_(IM_ISCOMPOSING_STATE_IDLE),
	refresh_(0)
{}

t_sip_body *t_im_iscomposing_xml_body::copy(void) const {
	t_im_iscomposing_xml_body *body = new t_im_iscomposing_xml_body(*this);
	MEMMAN_NEW(body);
	
	// Clear the xml_doc pointer in the new body, as a copy of the
	// XML document must be copied to the body.
	body->xml_doc = NULL;
	
	copy_xml_doc(body);
	
	return body;
}

t_body_type t_im_iscomposing_xml_body::get_type(void) const {
	return BODY_IM_ISCOMPOSING_XML;
}

t_media t_im_iscomposing_xml_body::get_media(void) const {
	return t_media("application", "im-iscomposing+xml");
}

bool t_im_iscomposing_xml_body::parse(const string &s) {
	if (t_sip_body_xml::parse(s)) {
		if (!extract_data()) {
			MEMMAN_DELETE(xml_doc);
			xmlFreeDoc(xml_doc);
			xml_doc = NULL;
		}
	}
	
	return (xml_doc != NULL);
}

string t_im_iscomposing_xml_body::get_state(void) const {
	return state_;
}

time_t t_im_iscomposing_xml_body::get_refresh(void) const {
	return refresh_;
}

void t_im_iscomposing_xml_body::set_state(const string &state) {
	state_ = state;
}

void t_im_iscomposing_xml_body::set_refresh(time_t refresh) {
	refresh_ = refresh;
}
