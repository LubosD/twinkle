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
 * Mime database
 * Conversion between mime types, file content and file extensions.
 */

#ifndef _MIME_DATABASE_H
#define _MIME_DATABASE_H

#include <map>
#include <string>
#include <magic.h>

#include "record_file.h"

using namespace std;

namespace utils {

/**  Record from the mime database. */
class t_mime_db_record : public utils::t_record {
public:
	string	mimetype;	/**< Mimetype, e.g. text/plain */
	string	file_glob;	/**< File glob expression */
	
	virtual bool create_file_record(vector<string> &v) const;
	virtual bool populate_from_file_record(const vector<string> &v);
};

/**
 * The mime database.
 * The default location for the mime database is /usr/share/mime/globs
 */
class t_mime_database : public utils::t_record_file<t_mime_db_record> {
private:
	/** Mapping between mimetypes and file globs. */
	map<string, string>	map_mime2glob_;
	
	/** Handle on the magic number database. */
	magic_t mime_magic_;
	
protected:
	virtual void add_record(const t_mime_db_record &record);
	
public:
	/** Constructor */
	t_mime_database();
	
	/** Destructor */
	~t_mime_database();
	
	/**
	 * Get a glob expression for a mimetype.
	 * @param mimetype [in] The mimetype.
	 * @return Glob expression associated with the mimetype. Empty string
	 *         if no glob expression can be found.
	 */
	string get_glob(const string &mimetype) const;
	
	/**
	 * Get the mimetype of a file.
	 * @param filename [in] Name of the file.
	 * @return The mimetype or empty string if no mimetype can be determined.
	 */
	string get_mimetype(const string &filename) const;
};

}; // namespace

extern utils::t_mime_database *mime_database;

#endif
