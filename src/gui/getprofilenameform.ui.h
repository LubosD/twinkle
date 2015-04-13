/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
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


void GetProfileNameForm::init()
{
	// Letters, digits, underscore, minus
	QRegExp rxFilenameChars("[\\w\\-][\\w\\-@\\.]*");
	
	// Set validators
	// USER
	profileLineEdit->setValidator(new QRegExpValidator(rxFilenameChars, this));
}

void GetProfileNameForm::validate()
{
	if (profileLineEdit->text().isEmpty()) return;
	
	// Find the .twinkle directory in HOME
	QDir d = QDir::home();
	if (!d.cd(USER_DIR)) {
		QMessageBox::critical(this, PRODUCT_NAME,
			tr("Cannot find .twinkle directory in your home directory."));
		reject();
	}
	
	QString filename = profileLineEdit->text();
	filename.append(USER_FILE_EXT);
	QString fullname = d.filePath(filename);
	if (QFile::exists(fullname)) {
		QMessageBox::warning(this, PRODUCT_NAME,
			tr("Profile already exists."));
		return;
	}
	
	accept();
}

QString GetProfileNameForm:: getProfileName()
{
	return profileLineEdit->text();
}

// Execute a dialog to get a name for a new profile
int GetProfileNameForm::execNewName()
{
	profileTextLabel->setText(tr("Enter a name for your profile:"));
	return exec();
}

// Execute this dialog to get a new name for an existing profile
int GetProfileNameForm::execRename(const QString &oldName)
{
	QString s = tr("Rename profile '%1' to:").arg(oldName);
	profileTextLabel->setText(s);
	return exec();
}
