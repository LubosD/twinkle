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

#ifndef _YESNODIALOG_H
#define _YESNODIALOG_H

#include "qdialog.h"
#include "qpushbutton.h"
#include "qstring.h"

class YesNoDialog : public QDialog {
private:
	Q_OBJECT
	QPushButton *pbYes;
	QPushButton *pbNo;
	
protected slots:
	virtual void actionYes();
	virtual void actionNo();
	
public:
	YesNoDialog();
	YesNoDialog(QWidget *parent, const QString &caption, const QString &text);
	virtual ~YesNoDialog();
	
	void reject();
};

class ReferPermissionDialog : public YesNoDialog {
private:
	Q_OBJECT

protected slots:
	virtual void actionYes();
	virtual void actionNo();

public:
	ReferPermissionDialog(QWidget *parent, const QString &caption, const QString &text);
};

#endif
