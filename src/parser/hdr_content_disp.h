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

// Content-Disposition header

#ifndef _HDR_CONTENT_DISP
#define _HDR_CONTENT_DISP

#include <string>
#include <list>
#include "header.h"
#include "parameter.h"

using namespace std;

//@{
/** @name Disposition types */
#define DISPOSITION_ATTACHMENT	"attachment"
//@}

class t_hdr_content_disp : public t_header {
public:
	string			type;
	string			filename;
	list<t_parameter>	params;

	t_hdr_content_disp();

	void set_type(const string &t);
	void set_filename(const string &name);
	void add_param(const t_parameter &p);
	void set_params(const list<t_parameter> &l);
	string encode_value(void) const;
};

#endif
