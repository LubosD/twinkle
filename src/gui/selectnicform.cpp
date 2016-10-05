//Added by qt3to4:
#include <QPixmap>
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

#include "gui.h"
#include <QMessageBox>
#include "sys_settings.h"
#include "selectnicform.h"
/*
 *  Constructs a SelectNicForm as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
SelectNicForm::SelectNicForm(QWidget* parent)
    : QDialog(parent)
{
	setupUi(this);

	init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
SelectNicForm::~SelectNicForm()
{
	// no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void SelectNicForm::languageChange()
{
	retranslateUi(this);
}


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
			QPixmap(":/icons/images/kcmpci16.png"),
			nicListBox->text(idxDefault),
			idxDefault);
	}
	
	// Set pixmap of the default
	idxDefault = idxNewDefault;
	nicListBox->changeItem(
		QPixmap(":/icons/images/twinkle16.png"),
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
