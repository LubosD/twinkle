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

/**
 * @file
 * RFC 3863 pidf+xml body
 */

#ifndef _PIDF_BODY_H
#define _PIDF_BODY_H

#include <string>
#include <libxml/tree.h>
#include "parser/sip_body.h"

#define PIDF_STATUS_BASIC_OPEN		"open"
#define PIDF_STATUS_BASIC_CLOSED	"closed"

/** RFC 3863 pidf+xml body */
class t_pidf_xml_body : public t_sip_body_xml {
private:
	string		pres_entity;		/**< Presence entity */
	string		tuple_id;	/**< Id of tuple containing the basic status. */
	string		basic_status;	/**< Value of basic-tag */
	
	/**
	 * Extract the status information from a PIDF document.
	 * This will populate the state attributes.
	 * @return True if PIDF document is valid, otherwise false.
	 * @pre The @ref pidf_doc should contain a valid PIDF document.
	 */
	bool extract_status(void);
	
	/**
	 * Process tuple element.
	 * @param tuple [in] tuple element.
	 */
	void process_pidf_tuple(xmlNode *tuple);
	
	/**
	 * Process status element.
	 * @param status [in] status element.
	 */
	void process_pidf_status(xmlNode *status);
	
	/**
	 * Process basic element.
	 * @param basic [in] basic element.
	 */
	void process_pidf_basic(xmlNode *basic);
	
protected:
	/**
	 * Create a pidf document from the values stored in the attributes.
	 */
	virtual void create_xml_doc(const string &xml_version = "1.0", const string &charset = "UTF-8");
	
public:
	/** Constructor */
	t_pidf_xml_body();
	
	virtual t_sip_body *copy(void) const;
	virtual t_body_type get_type(void) const;
	virtual t_media get_media(void) const;
	
	/** @name Getters */
	//@{
	string get_pres_entity(void) const;
	string get_tuple_id(void) const;
	string get_basic_status(void) const;
	//@}
	
	/** @name Setters */
	//@{
	void set_pres_entity(const string &_pres_entity);
	void set_tuple_id(const string &_tuple_id);
	void set_basic_status(const string &_basic_status);;
	//@}
	
	/**
	 * Parse a text representation of the body.
	 * If parsing succeeds, then the state is extracted.
	 * @param s [in] Text to parse.
	 * @return True if parsing and state extracting succeeded, false otherwise.
	 */
	virtual bool parse(const string &s);
};

#endif
