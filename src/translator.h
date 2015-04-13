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

#ifndef _TRANSLATOR_H
#define _TRANSLATOR_H

#include <string>

#define TRANSLATE(s)		(translator ? translator->translate(s) : s)
#define TRANSLATE2(c, s)	(translator ? translator->translate2(c, s) : s)

using namespace std;

// This class provides an interface for languague translations.
// The default implementation does not perform any translation.
// The class may be subclassed to provide translation services.

class t_translator {
public:
	virtual ~t_translator() {};
	
	// The default implementation simply returns the passed
	// string. A subclass should reimplement this method to
	// provide translation.
	virtual string translate(const string &s) { return s; };
	
	// The name of the context parameter is in comments to avoid
	// unused argument warnings.
	virtual string translate2(const string &/*context*/, const string &s) { return s; };
};

extern t_translator *translator;

#endif
