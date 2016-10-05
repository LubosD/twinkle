/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/

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
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _TEXTBROWSERNOAUTOLINK_H
#define _TEXTBROWSERNOAUTOLINK_H

#include <QTextBrowser>

/**
  * A text browser similar to QTextBrowser, but when a user clicks a link
  * the browser will not automatically load the linked document.
  */
class TextBrowserNoAutoLink : public QTextBrowser {
	Q_OBJECT
public:
	TextBrowserNoAutoLink ( QWidget * parent = 0 );
	virtual void setSource(const QUrl& name) override;
};

#endif
