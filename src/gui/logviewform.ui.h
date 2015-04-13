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

void LogViewForm::show()
{
	if (isShown()) {
		raise();
		return;
	}
	
	QString fname = log_file->get_filename().c_str();
	logfile = new QFile(fname);
	MEMMAN_NEW(logfile);
	logstream = NULL;
	if (logfile->open(IO_ReadOnly)) {
		logstream = new QTextStream(logfile);
		MEMMAN_NEW(logstream);
		logTextEdit->setText(logstream->read());
		
		// Set cursor position at the end of text
		logTextEdit->scrollToBottom();
	}
	
	log_file->enable_inform_user(true);
	
	QDialog::show();
	raise();
}

void LogViewForm::closeEvent( QCloseEvent *ev )
{
	log_file->enable_inform_user(false);
	logTextEdit->clear();
	
	if (logstream) {
		MEMMAN_DELETE(logstream);
		delete logstream;
		logstream = NULL;
	}
	
	logfile->close();
	MEMMAN_DELETE(logfile);
	delete logfile;
	logfile = NULL;
	
	QDialog::closeEvent(ev);
}

void LogViewForm::update(bool log_zapped)
{
	if (!isShown()) return;
	
	if (log_zapped) {
		close();
		show();
		return;
	}
	
	if (logstream) {
		QString s = logstream->read();
		if (!s.isNull() && !s.isEmpty()) {
			logTextEdit->append(s);
		}
	}
}

void LogViewForm::clear()
{
	logTextEdit->clear();
}

