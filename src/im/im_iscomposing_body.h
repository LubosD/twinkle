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
 * RFC 3994 im-iscomposing+xml body
 */

#ifndef _IM_ISCOMPOSING_BODY_H
#define _IM_ISCOMPOSING_BODY_H

#include <string>
#include <time.h>
#include <libxml/tree.h>
#include "parser/sip_body.h"

using namespace std;

//@{
/** @name Message composition states */
#define IM_ISCOMPOSING_STATE_IDLE	"idle"
#define IM_ISCOMPOSING_STATE_ACTIVE	"active"
//@}

/** RFC 3994 im-iscomposing+xml body */
class t_im_iscomposing_xml_body : public t_sip_body_xml {
private:
	string		state_;		/**< Composition state */
	time_t		refresh_;	/**< Refresh interval in seconds */
	
	/** Extract information elements from the XML document. */
	bool extract_data(void);
	
	/** 
	 * Process the state element.
	 * @param node [in] The state element.
	 */
	bool process_node_state(xmlNode *node);
	
	/** 
	 * Process the refresh element.
	 * @param node [in] The refresh element.
	 */
	void process_node_refresh(xmlNode *node);
	
protected:
	/**
	 * Create a im-iscomposing document from the values stored in the attributes.
	 */
	virtual void create_xml_doc(const string &xml_version = "1.0", const string &charset = "UTF-8");
	
public:
	/** Constructor */
	t_im_iscomposing_xml_body();
	
	virtual t_sip_body *copy(void) const;
	virtual t_body_type get_type(void) const;
	virtual t_media get_media(void) const;
	virtual bool parse(const string &s);
	
	/** @name Getters */
	//@{
	string get_state(void) const;
	time_t get_refresh(void) const;
	//@}
	
	/** @name Setters */
	//@{
	void set_state(const string &state);
	void set_refresh(time_t refresh);
	//@}
};

#endif
