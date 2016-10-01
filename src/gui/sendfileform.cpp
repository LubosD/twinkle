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

#ifdef HAVE_KDE
#include <kfiledialog.h>
#endif

#include "audits/memman.h"
#include "gui.h"
#include <QFile>
#include <QFileDialog>
#include "sendfileform.h"
/*
 *  Constructs a SendFileForm as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
SendFileForm::SendFileForm(QWidget* parent)
    : QDialog(parent)
{
	setupUi(this);

	init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
SendFileForm::~SendFileForm()
{
	destroy();
	// no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void SendFileForm::languageChange()
{
	retranslateUi(this);
}


void SendFileForm::init()
{
    setAttribute(Qt::WA_DeleteOnClose);
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
        ((t_gui *)ui)->cb_show_msg(this,  tr("File does not exist.").toStdString(), MSG_WARNING);
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
	QFileDialog *d = new QFileDialog(this);
	MEMMAN_NEW(d);
	
	connect(d, SIGNAL(fileSelected(const QString &)), this, SLOT(setFilename()));
#endif
    d->setWindowTitle(tr("Send file..."));
	
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
    QStringList files = d->selectedFiles();

    if (!files.empty())
        filename = files[0];
#endif
	
	fileLineEdit->setText(filename);
}
