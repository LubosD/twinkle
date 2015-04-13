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
 * File to store data records.
 */
 
#ifndef _RECORD_FILE_H
#define _RECORD_FILE_H

#include <list>
#include <string>
#include <vector>
#include <cstdlib>
#include <sys/stat.h>
#include <iostream>
#include <fstream>

#include "translator.h"
#include "util.h"
#include "threads/mutex.h"


using namespace std;

namespace utils {

/** A single record in a file. */
class t_record {
public:
	virtual ~t_record() {};
	
	/**
	 * Create a record to write to a file.
	 * @param v [out] Vector of fields of record.
	 * @return true, if record is succesfully created.
	 * @return false, otherwise.
	 */
	virtual bool create_file_record(vector<string> &v) const = 0;
	
	/**
	 * Populate from a file record.
	 * @param v [in] Vector containing the fields of the record.
	 * @return true, if record is succesfully populated.
	 * @return false, if file record could not be parsed.
	 */
	virtual bool populate_from_file_record(const vector<string> &v) = 0;	
};

/** 
 * A file containing records with a fixed number of fields. 
 * @param R Subclass of @ref t_record
 */
template< class R >
class t_record_file {
private:
	/** Separator to separate fields in a file record. */
	char	field_separator;
	
	/** Header string to write as comment at start of file. */
	string	header;
	
	/** Name of the file containing the records. */
	string	filename;
	
	/**
	 * Split a record into separate fields.
	 * @param record [in] A complete record.
	 * @param v [out] Vector of fields.
	 */
	void split_record(const string &record, vector<string> &v) const;
	
	/**
	 * Join fields of a record into a string.
	 * Separator and comment symbols will be escaped.
	 * @param v [in] Vector of fields.
	 * @return Joined fields.
	 */
	string join_fields(const vector<string> &v) const;
	
protected:
	/** Mutex to protect concurrent access/ */
	mutable t_recursive_mutex	mtx_records;
	
	/** Records in the file. */
	list<R> records;
	
	/**
	 * Add a record to the file.
	 * @param record [in] Record to add.
	 */
	virtual void add_record(const R &record);

public:
	/** Constructor. */
	t_record_file();
	
	/** Constructor. */
	t_record_file(const string &_header, char _field_separator, const string &_filename);
	
	/** Destructor. */
	virtual ~t_record_file() {};
	
	/** @name Setters */
	//@{
	void set_header(const string &_header);
	void set_separator(char _separator);
	void set_filename(const string &_filename);
	//@}
	
	/** @name Getters */
	//@{
	list<R> *get_records();
	//@}

	/** 
	 * Load records from file. 
	 * @param error_msg [out] Error message on failure return.
	 * @return true, if file was read succesfully.
	 * @return false, if it fails. error_msg is an error to be given to
	 * the user.
	 */
	virtual bool load(string &error_msg);
	
	/** 
	 * Save records to file.
	 * @param error_msg [out] Error message on failure return.
	 * @return true, if file was saved succesfully.
	 * @return false, if it fails. error_msg is an error to be given to
	 * the user.
	 */
	virtual bool save(string &error_msg) const;
	
	typedef typename list<R>::const_iterator record_const_iterator;
	typedef typename list<R>::iterator record_iterator;
};

#include "record_file.hpp"

}; // namespace

#endif
