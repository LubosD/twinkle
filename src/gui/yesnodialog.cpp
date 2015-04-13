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

#include "yesnodialog.h"

#include "qlabel.h"
#include "qlayout.h"

#include "userintf.h"

// class YesNoDialog

void YesNoDialog::actionYes() {
	QDialog::accept();
}

void YesNoDialog::actionNo() {
	QDialog::reject();
}

YesNoDialog::YesNoDialog() {
	new QDialog(NULL, NULL, true, Qt::WDestructiveClose);
}

YesNoDialog::YesNoDialog(QWidget *parent, const QString &caption, const QString &text) :
		QDialog(parent, NULL, true, Qt::WDestructiveClose)
{
	setCaption(caption);
	QBoxLayout *vb = new QVBoxLayout(this, 11, 6);
	QLabel *lblQuestion = new QLabel(text, this);
	vb->addWidget(lblQuestion);
	QHBoxLayout *hb = new QHBoxLayout(NULL, 0, 6);
	QSpacerItem *spacer1 = new QSpacerItem(1, 1, QSizePolicy::Expanding, 
					       QSizePolicy::Minimum );
	hb->addItem(spacer1);
	pbYes = new QPushButton(tr("&Yes"), this);
	hb->addWidget(pbYes);
	pbNo = new QPushButton(tr("&No"), this);
	hb->addWidget(pbNo);
	QSpacerItem *spacer2 = new QSpacerItem(1, 1, QSizePolicy::Expanding, 
					       QSizePolicy::Minimum );
	hb->addItem(spacer2);
	vb->addLayout(hb);
	
	connect(pbYes, SIGNAL(clicked()), this, SLOT(actionYes()));
	connect(pbNo, SIGNAL(clicked()), this, SLOT(actionNo()));
}

YesNoDialog::~YesNoDialog() {}

void YesNoDialog::reject() {
	pbNo->animateClick();
}


// class ReferPermissionDialog

void ReferPermissionDialog::actionYes() {
	ui->send_refer_permission(true);
	YesNoDialog::actionYes();
}

void ReferPermissionDialog::actionNo() {
	ui->send_refer_permission(false);
	YesNoDialog::actionNo();
}

ReferPermissionDialog::ReferPermissionDialog(QWidget *parent, const QString &caption, 
					     const QString &text) :
		YesNoDialog(parent, caption, text)
{}
