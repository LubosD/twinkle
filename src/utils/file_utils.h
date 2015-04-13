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
 * File utilities
 */

#ifndef _FILE_UTILS_H
#define _FILE_UTILS_H

#include <string>

using namespace std;

namespace utils {
 
/** Separator to split parts in a file path. */
#define PATH_SEPARATOR	'/'

/**
 * Copy a file.
 * @param from [in] Absolute path of file to copy.
 * @param to [in] Absolute path of destination file.
 * @return true if copy succeeded, otherwise false.
 */
bool filecopy(const string &from, const string &to);

/**
 * Strip the path to a file from an absolute file name.
 * @return The remaining file name without the full path.
 */
string strip_path_from_filename(const string &filename);

/**
 * Get path to a file from an absolute file name.
 * @return The path name.
 */
string get_path_from_filename(const string &filename);

/**
 * Get the extension from a file name.
 * @return The extension (without the initial dot).
 * @return Empty string if the file name does not have an extension.
 */
string get_extension_from_filename(const string &filename);

/**
 * Apply a glob expression to a filename.
 * E.g. /tmp/twinkle with glob *.txt gives /tmp/twinkle.txt
 *      /tmp/twinkle with README* give /tmp/READMEtwinkle.txt
 * @param filename [in] The filename.
 * @param glob [in] The glob expression to apply.
 * @return The modified filename.
 */
string apply_glob_to_filename(const string &filename, const string &glob);

/**
 * Get the absolute path of the current working directory.
 * @return The absolute path of the current working directory.
 * @return Empty string if the current working directory cannot be determined.
 */
string get_working_dir(void);

}; // namespace

#endif
