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
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifdef HAVE_KDE
#include <kfiledialog.h>
#endif

void SendFileForm::init()
{
	setWFlags(getWFlags() | Qt::WDestructiveClose);
	_chooseFileDialog = NULL;
}

void SendFileForm::destroy()
{
	// Auto destruct window
	MEMMAN_DELETE(this);
	
	if (_chooseFileDialog) {
		MEMMAN_DELETE(_chooseFileDialog);
		delete _chooseFileDialog;
	}
}

/** Signal the selected information to an observer. */
void SendFileForm::signalSelectedInfo() 
{
	if (!QFile::exists(fileLineEdit->text())) {
		((t_gui *)ui)->cb_show_msg(this,  tr("File does not exist.").ascii(), MSG_WARNING);
		return;
	}
	
	emit selected(fileLineEdit->text(), subjectLineEdit->text());
	accept();
}

/** Choose a file from a file dialog. */
void SendFileForm::chooseFile()
{
#ifdef HAVE_KDE
	KFileDialog *d = new KFileDialog(QString::null, QString::null, this, 0, true);
	MEMMAN_NEW(d);
	
	d->setOperationMode(KFileDialog::Other);
	connect(d, SIGNAL(okClicked()), this, SLOT(setFilename()));
#else
	QFileDialog *d = new QFileDialog(QString::null, QString::null, this, 0, true);
	MEMMAN_NEW(d);
	
	connect(d, SIGNAL(fileSelected(const QString &)), this, SLOT(setFilename()));
#endif
	d->setCaption(tr("Send file..."));
	
	if (_chooseFileDialog) {
		MEMMAN_DELETE(_chooseFileDialog);
		delete _chooseFileDialog;
	}
	_chooseFileDialog = d;
	
	d->show();
}

/**
 * Set the filename value.
 * @param filename [in] The value to set.
 */
void SendFileForm::setFilename()
{
	QString filename;
#ifdef HAVE_KDE
	KFileDialog *d = dynamic_cast<KFileDialog *>(_chooseFileDialog);
	filename = d->selectedFile();
#else
	QFileDialog *d = dynamic_cast<QFileDialog *>(_chooseFileDialog);
	filename = d->selectedFile();
#endif
	
	fileLineEdit->setText(filename);
}
