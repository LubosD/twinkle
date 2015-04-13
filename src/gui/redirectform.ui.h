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

void RedirectForm::init()
{
	// Keeps track of which address book tool button is clicked.
	nrAddressBook = 0;
	
	getAddressForm = 0;
	
	// Set toolbutton icons for disabled options.
	QIconSet i;
	i = address1ToolButton->iconSet();
	i.setPixmap(QPixmap::fromMimeSource("kontact_contacts-disabled.png"), 
		    QIconSet::Automatic, QIconSet::Disabled);
	address1ToolButton->setIconSet(i);
	address2ToolButton->setIconSet(i);
	address3ToolButton->setIconSet(i);
}

void RedirectForm::destroy()
{
	if (getAddressForm) {
		MEMMAN_DELETE(getAddressForm);
		delete getAddressForm;
	}
}

void RedirectForm::show(t_user *user, const list<string> &contacts)
{
	user_config = user;
	
	int num = 0;
	for (list<string>::const_iterator i = contacts.begin();
	i != contacts.end(); i++, num++)
	{
		if (num == 0) contact1LineEdit->setText(i->c_str());
		if (num == 1) contact2LineEdit->setText(i->c_str());
		if (num == 2) contact3LineEdit->setText(i->c_str());
	}
	
	QDialog::show();
}

void RedirectForm::validate()
{
	t_display_url destination;
	list<t_display_url> dest_list;
	
	// 1st choice destination
	ui->expand_destination(user_config, contact1LineEdit->text().stripWhiteSpace().ascii(),
			       destination);
	if (destination.is_valid()) {
		dest_list.push_back(destination);
	} else {
		contact1LineEdit->selectAll();
		return;
	}
	
	// 2nd choice destination
	if (!contact2LineEdit->text().isEmpty()) {
		ui->expand_destination(user_config,
			contact2LineEdit->text().stripWhiteSpace().ascii(), destination);
		if (destination.is_valid()) {
			dest_list.push_back(destination);
		} else {
			contact2LineEdit->selectAll();
			return;
		}
	}
	
	// 3rd choice destination
	if (!contact3LineEdit->text().isEmpty()) {
		ui->expand_destination(user_config,
			contact3LineEdit->text().stripWhiteSpace().ascii(), destination);
		if (destination.is_valid()) {
			dest_list.push_back(destination);
		} else {
			contact3LineEdit->selectAll();
			return;
		}
	}
	
	emit destinations(dest_list);
	accept();
}

void RedirectForm::showAddressBook()
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

void RedirectForm::showAddressBook1()
{
	nrAddressBook = 1;
	showAddressBook();
}

void RedirectForm::showAddressBook2()
{
	nrAddressBook = 2;
	showAddressBook();
}

void RedirectForm::showAddressBook3()
{
	nrAddressBook = 3;
	showAddressBook();
}

void RedirectForm::selectedAddress(const QString &address)
{
	switch(nrAddressBook) {
	case 1:
		contact1LineEdit->setText(address);
		break;
	case 2:
		contact2LineEdit->setText(address);
		break;
	case 3:
		contact3LineEdit->setText(address);
		break;
	}
}
