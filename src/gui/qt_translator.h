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

#ifndef _QT_TRANSLATOR_H
#define _QT_TRANSLATOR_H

#include <qapplication.h>
#include "translator.h"

// This class provides the translation service from Qt to the
// core of Twinkle.
class t_qt_translator : public t_translator {
public:
	t_qt_translator(QApplication *qa) : _qa(qa) {};
	
	virtual string translate(const string &s) {
		return _qa->translate("TwinkleCore", s.c_str()).ascii(); 
	};
	
	virtual string translate2(const string &context, const string &s) {
		return _qa->translate(context.c_str(), s.c_str()).ascii();
	};

private:
	QApplication *_qa;
};

#endif
