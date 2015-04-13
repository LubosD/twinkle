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

void SelectNicForm::init()
{
	idxDefault = -1;
}

void SelectNicForm::setAsDefault(bool setIp)
{
#if 0
	// DEPRECATED
	// Only show the information when the default button is
	// pressed for the first time.
	if (idxDefault == -1) {
		QMessageBox::information(this, PRODUCT_NAME, tr(
			"If you want to remove or "
			"change the default at a later time, you can do that "
			"via the system settings."));
	}
	
	// Store current index as the changeItem method also changes
	// the current index as a side effect.
	int idxNewDefault = nicListBox->currentItem();
	
	// Restore pixmap of the old default
	if (idxDefault != -1) {
		nicListBox->changeItem(
			QPixmap::fromMimeSource("kcmpci16.png"),
			nicListBox->text(idxDefault),
			idxDefault);
	}
	
	// Set pixmap of the default
	idxDefault = idxNewDefault;
	nicListBox->changeItem(
		QPixmap::fromMimeSource("twinkle16.png"),
		nicListBox->text(idxDefault),
		idxDefault);	
	
	// Write default to system settings
	int pos = nicListBox->currentText().findRev(':');
	if (setIp) {
		sys_config->set_start_user_host(nicListBox->currentText().mid(pos + 1).ascii());
		sys_config->set_start_user_nic("");
	} else {
		sys_config->set_start_user_nic(nicListBox->currentText().left(pos).ascii());
		sys_config->set_start_user_host("");
	}
	string error_msg;
	if (!sys_config->write_config(error_msg)) {
		// Failed to write config file
		((t_gui *)ui)->cb_show_msg(this, error_msg, MSG_CRITICAL);
	}
#endif
}

void SelectNicForm::setAsDefaultIp()
{
	setAsDefault(true);
}

void SelectNicForm::setAsDefaultNic()
{
	setAsDefault(false);
}
