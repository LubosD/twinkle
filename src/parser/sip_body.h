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

// SIP bodies
#ifndef _H_SIP_BODY
#define _H_SIP_BODY

#include <cc++/config.h>
#include <string>
#include <libxml/tree.h>

#include "media_type.h"

//@{
/** @name Utilies for XML body parsing */
/**
 * Check the tag name of an XML node.
 * @param node [in] (xmlNode *) The XML node to check.
 * @param tag [in] (const char *) The tag name.
 * @param namespace [in] (const char *) The namespace of the tag.
 * @return true if the node has the tag name within the name space.
 */
#define IS_XML_TAG(node, tag, namespace)\
				((node)->type == XML_ELEMENT_NODE &&\
				(node)->ns &&\
				xmlStrEqual((node)->ns->href, BAD_CAST (namespace)) &&\
				xmlStrEqual((node)->name, BAD_CAST (tag)))
				
/**
 * Check the attribute name of an XML attribute.
 */
#define IS_XML_ATTR(attr, attr_name, namespace)\
				 ((attr)->type == XML_ATTRIBUTE_NODE &&\
				 (attr)->ns &&\
				 xmlStrEqual((attr)->ns->href, BAD_CAST (namespace)) &&\
				 xmlStrEqual((attr)->name, BAD_CAST (attr_name)))
//@}

class t_sip_message;

using namespace std;

/** Body type. */
enum t_body_type {
	BODY_OPAQUE,		/**< Opaque body. */
	BODY_SDP,		/**< SDP */
	BODY_SIPFRAG,		/**< message/sipfrag RFC 3420 */
	BODY_DTMF_RELAY,	/**< DTMF relay as defined by Cisco */
	BODY_SIMPLE_MSG_SUM,	/**< Simple message summary RFC 3842 */
	BODY_PLAIN_TEXT,	/**< Plain text for messaging */
	BODY_HTML_TEXT,		/**< HTML text for messaging */
	BODY_PIDF_XML,		/**< pidf+xml RFC 3863 */
	BODY_IM_ISCOMPOSING_XML	/**< im-iscomposing+xml RFC 3994 */
};

/** Abstract base class for SIP bodies. */
class t_sip_body {
public:
	/**
	 * Indicates if the body content is invalid.
	 * This will be set by the body parser.
	 */
	bool 	invalid;

	/** Constructor. */
	t_sip_body();
	
	virtual ~t_sip_body() {}

	/**
	 * Encode the body.
	 * @return Text encoded body.
	 */
	virtual string encode(void) const = 0;

	/**
	 * Create a copy of the body.
	 * @return Copy of the body.
	 */
	virtual t_sip_body *copy(void) const = 0;

	/** 
	 * Get type of body.
	 * @return body type.
	 */
	virtual t_body_type get_type(void) const = 0;
	
	/**
	 * Get content type for this type of body.
	 * @return Content type.
	 */
	virtual t_media get_media(void) const = 0;
	
	/**
	 * Check if all local IP address are correctly filled in. This
	 * check is an integrity check to help debugging the auto IP
	 * discover feature.
	 */
	virtual bool local_ip_check(void) const;
	
	/**
	 * Return the size of the encoded body. This method encodes the body
	 * to calculate the size. When a more efficient algorithm is available
	 * a sub class may override this method.
	 * @return The size of the encoded body in bytes.
	 */
	virtual size_t get_encoded_size(void) const;
};

/** Abstract base class for XML formatted bodies. */
class t_sip_body_xml : public t_sip_body {
protected:
	xmlDoc		*xml_doc;	/**< XML document */
	
	/**
	 * Create an empty XML document.
	 * Override this method to create the specific XML document.
	 * @param xml_version [in] The XML version of the document.
	 * @param charset [in] The character set of the document.
	 */
	virtual void create_xml_doc(const string &xml_version = "1.0", const string &charset = "UTF-8");
	
	/** Remove the XML document */
	virtual void clear_xml_doc(void);
	
	/** 
	 * Copy the XML document from this body to another body.
	 * @param to_body [in] The body to copy the XML body to.
	 */
	virtual void copy_xml_doc(t_sip_body_xml *to_body) const;
	
public:
	/** Constructor */
	t_sip_body_xml();
	
	/** Destructor */
	virtual ~t_sip_body_xml();
	
	virtual string encode(void) const;
	
	/**
	 * Parse a text representation of the body.
	 * The result is stored in @ref xml_doc
	 * @param s [in] Text to parse.
	 * @return True if parsing and state extracting succeeded, false otherwise.
	 * @pre xml_doc == NULL
	 * @post If parsing succeeds then xml_doc != NULL
	 */
	virtual bool parse(const string &s);
};


/**
 * This body can contain any type of body. The contents are
 * unparsed and thus opaque.
 */
class t_sip_body_opaque : public t_sip_body {
public:
	string	opaque; /**< The body contents. */

	/** Construct body with empty content. */
	t_sip_body_opaque();

	/**
	 * Construct a body with opaque content.
	 * @param s [in] The content.
	 */
	t_sip_body_opaque(string s);
	
	string encode(void) const;
	t_sip_body *copy(void) const;
	t_body_type get_type(void) const;
	t_media get_media(void) const;
	virtual size_t get_encoded_size(void) const;
};

// RFC 3420
// sipfrag body
class t_sip_body_sipfrag : public t_sip_body {
public:
	t_sip_message	*sipfrag;

	t_sip_body_sipfrag(t_sip_message *m);
	~t_sip_body_sipfrag();
	string encode(void) const;
	t_sip_body *copy(void) const;
	t_body_type get_type(void) const;
	t_media get_media(void) const;
};

// application/dtmf-relay body
class t_sip_body_dtmf_relay : public t_sip_body {
public:
	char	signal;
	uint16	duration; // ms
	
	t_sip_body_dtmf_relay();
	t_sip_body_dtmf_relay(char _signal, uint16 _duration);
	string encode(void) const;
	t_sip_body *copy(void) const;
	t_body_type get_type(void) const;
	t_media get_media(void) const;
	bool parse(const string &s);
};

/** Plain text body. */
class t_sip_body_plain_text : public t_sip_body {
public:
	string	text;	/**< The text */
	
	/** Construct a body with empty text. */
	t_sip_body_plain_text();
	
	/**
	 * Constructor.
	 * @param _text [in] The body text.
	 */
	t_sip_body_plain_text(const string &_text);
	
	string encode(void) const;
	t_sip_body *copy(void) const;
	t_body_type get_type(void) const;
	t_media get_media(void) const;
	virtual size_t get_encoded_size(void) const;
};

/** Html text body. */
class t_sip_body_html_text : public t_sip_body {
public:
	string	text;	/**< The text */
	
	/**
	 * Constructor.
	 * @param _text [in] The body text.
	 */
	t_sip_body_html_text(const string &_text);
	
	string encode(void) const;
	t_sip_body *copy(void) const;
	t_body_type get_type(void) const;
	t_media get_media(void) const;
	virtual size_t get_encoded_size(void) const;
};

#endif
