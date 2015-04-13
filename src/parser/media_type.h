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
 * Media MIME type definition.
 */

#ifndef _MEDIA_TYPE_H
#define _MEDIA_TYPE_H

#include <list>
#include <string>
#include "parameter.h"

using namespace std;

/** Media MIME type definition. */
class t_media {
public:
	string	type;		/**< main type */
	string	subtype;	/**< subtype */
	string	charset;	/**< Character set */
	float	q;		/**< quality factor */
	list<t_parameter> media_param_list; 	 /**< media paramters */
	list<t_parameter> accept_extension_list; /**< accept parameters */

	/** Constructor */
	t_media();

	/** 
	 * Constructor. 
	 * Construct object with a specic type and subtype.
	 * @param t [in] type
	 * @param s [in] subtype
	 */
	t_media(const string &t, const string &s);
	
	/**
	 * Constructor.
	 * Construct a media object from a mime type name
	 * @param mime_type [in] The mime type name, e.g. "text/plain"
	 */
	t_media(const string &mime_type);

	/**
	 * Add a parameter list.
	 * Method for parser to add the parsed parameter list l.
	 * l should start with optional media parameters followed
	 * by the q-paramter followed by accept parameters.
	 * @param l [in] The parameter list.
	 */
	void add_params(const list<t_parameter> &l);

	/**
	 * Encode as string.
	 * @return The encoded media type.
	 */
	string encode(void) const;
	
	/**
	 * Get the glob for a file name containing this MIME type.
	 * E.g. <wildcard>.txt for text/plain
	 * @return The file name extension.
	 */
	string get_file_glob(void) const;
};

#endif
