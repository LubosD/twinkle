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


void TermCapForm::init()
{
	getAddressForm = 0;
	
	// Set toolbutton icons for disabled options.
	setDisabledIcon(addressToolButton, "kontact_contacts-disabled.png");
}

void TermCapForm::show(t_user *user_config, const QString &dest)
{
	((t_gui *)ui)->fill_user_combo(fromComboBox);
	
	// Select from user
	if (user_config) {
		for (int i = 0; i < fromComboBox->count(); i++) {
			if (fromComboBox->text(i) == 
			    user_config->get_profile_name().c_str())
			{
				fromComboBox->setCurrentItem(i);
				break;
			}
		}
	}
	
	partyLineEdit->setText(dest);
	QDialog::show();
}

void TermCapForm::destroy()
{
	if (getAddressForm) {
		MEMMAN_DELETE(getAddressForm);
		delete getAddressForm;
	}
}

void TermCapForm::validate()
{
	string display, dest_str;
	t_user *from_user = phone->ref_user_profile(
				fromComboBox->currentText().ascii());
	
	ui->expand_destination(from_user, 
			       partyLineEdit->text().stripWhiteSpace().ascii(), 
			       display, dest_str);
	t_url dest(dest_str);
	
	if (dest.is_valid()) {
		emit destination(from_user, dest);
		accept();
	} else {
		partyLineEdit->selectAll();
	}
}

void TermCapForm::showAddressBook()
{
	if (!getAddressForm) {
		getAddressForm = new GetAddressForm(
				this, "select address", true);
		MEMMAN_NEW(getAddressForm);
	}
	
	connect(getAddressForm, 
		SIGNAL(address(const QString &)),
		this, SLOT(selectedAddress(const QString &)));
	
	getAddressForm->show();
}

void TermCapForm::selectedAddress(const QString &address)
{
	partyLineEdit->setText(address);
}
