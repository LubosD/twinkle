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


void SrvRedirectForm::init()
{
	cfAlwaysGroupBox->setEnabled(false);
	cfBusyGroupBox->setEnabled(false);
	cfNoanswerGroupBox->setEnabled(false);
	
	// Keeps track of which address book tool button is clicked.
	nrAddressBook = 0;
	
	getAddressForm = 0;
	
	// Set toolbutton icons for disabled options.
	QIconSet i;
	i = addrAlways1ToolButton->iconSet();
	i.setPixmap(QPixmap::fromMimeSource("kontact_contacts-disabled.png"), 
		    QIconSet::Automatic, QIconSet::Disabled);
	addrAlways1ToolButton->setIconSet(i);
	addrAlways2ToolButton->setIconSet(i);
	addrAlways3ToolButton->setIconSet(i);
	addrBusy1ToolButton->setIconSet(i);
	addrBusy2ToolButton->setIconSet(i);
	addrBusy3ToolButton->setIconSet(i);
	addrNoanswer1ToolButton->setIconSet(i);
	addrNoanswer2ToolButton->setIconSet(i);
	addrNoanswer3ToolButton->setIconSet(i);
}

void SrvRedirectForm::destroy()
{
	if (getAddressForm) {
		MEMMAN_DELETE(getAddressForm);
		delete getAddressForm;
	}
}

void SrvRedirectForm::show()
{
	current_user_idx = -1;
	((t_gui *)ui)->fill_user_combo(userComboBox);
	userComboBox->setEnabled(userComboBox->count() > 1);
	current_user = phone->ref_users().front();
	current_user_idx = 0;
	populate();
	
	QDialog::show();
}

void SrvRedirectForm::populate()
{
	t_service *srv = phone->ref_service(current_user);
	bool cf_active;
	list<t_display_url> dest_list;
	int field;
	
	// Call forwarding unconditional
	cf_active = srv->get_cf_active(CF_ALWAYS, dest_list);
	cfAlwaysDst1LineEdit->clear();
	cfAlwaysDst2LineEdit->clear();
	cfAlwaysDst3LineEdit->clear();
	cfAlwaysCheckBox->setChecked(cf_active);
	if (cf_active) {
		field = 1;
		for (list<t_display_url>::iterator i = dest_list.begin(); i != dest_list.end(); i++) {
			if (field == 1) cfAlwaysDst1LineEdit->setText(i->encode().c_str());
			if (field == 2) cfAlwaysDst2LineEdit->setText(i->encode().c_str());
			if (field == 3) cfAlwaysDst3LineEdit->setText(i->encode().c_str());
			field++;
		}
	}
	
	// Call forwarding busy
	cf_active = srv->get_cf_active(CF_BUSY, dest_list);
	cfBusyDst1LineEdit->clear();
	cfBusyDst2LineEdit->clear();
	cfBusyDst3LineEdit->clear();
	cfBusyCheckBox->setChecked(cf_active);
	if (cf_active) {
		field = 1;
		for (list<t_display_url>::iterator i = dest_list.begin(); i != dest_list.end(); i++) {
			if (field == 1) cfBusyDst1LineEdit->setText(i->encode().c_str());
			if (field == 2) cfBusyDst2LineEdit->setText(i->encode().c_str());
			if (field == 3) cfBusyDst3LineEdit->setText(i->encode().c_str());
			field++;
		}
	}
	
	// Call forwarding no answer
	cf_active = srv->get_cf_active(CF_NOANSWER, dest_list);
	cfNoanswerDst1LineEdit->clear();
	cfNoanswerDst2LineEdit->clear();
	cfNoanswerDst3LineEdit->clear();
	cfNoanswerCheckBox->setChecked(cf_active);
	if (cf_active) {
		field = 1;
		for (list<t_display_url>::iterator i = dest_list.begin(); i != dest_list.end(); i++) {
			if (field == 1) cfNoanswerDst1LineEdit->setText(i->encode().c_str());
			if (field == 2) cfNoanswerDst2LineEdit->setText(i->encode().c_str());
			if (field == 3) cfNoanswerDst3LineEdit->setText(i->encode().c_str());
			field++;
		}
	}
}
		
void SrvRedirectForm::validate()
{
	if (validateValues()) {
		accept();
	} else {
		((t_gui *)ui)->cb_show_msg(this,
			tr("You have entered an invalid destination.").ascii(),
			MSG_WARNING);
	}
}

bool SrvRedirectForm::validateValues()
{
	list<t_display_url> cfDestAlways, cfDestBusy, cfDestNoanswer;
	bool valid = false;
	
	// Redirect unconditional
	valid = validate(cfAlwaysCheckBox->isChecked(), 
		 cfAlwaysDst1LineEdit, cfAlwaysDst2LineEdit, cfAlwaysDst3LineEdit,
		 cfDestAlways);
	if (!valid) {
		cfTabWidget->setCurrentPage(0);
		return false;
	}
	
	// Redirect busy
	valid = validate(cfBusyCheckBox->isChecked(), 
		 cfBusyDst1LineEdit, cfBusyDst2LineEdit, cfBusyDst3LineEdit,
		 cfDestBusy);
	if (!valid) {
		cfTabWidget->setCurrentPage(1);
		return false;
	}
	
	// Redirect no answer
	valid = validate(cfNoanswerCheckBox->isChecked(), 
		 cfNoanswerDst1LineEdit, cfNoanswerDst2LineEdit, 
		 cfNoanswerDst3LineEdit,
		 cfDestNoanswer);
	if (!valid) {
		cfTabWidget->setCurrentPage(2);
		return false;
	}
	
	emit destinations(current_user, cfDestAlways, cfDestBusy, cfDestNoanswer);	
	return true;
}


// Validate 3 destinations if cf_active is true.
// Returns true when all destinations are valid (first must be set, others may be empty)
// dest_list containst the encoded destinations when valid.
// If cf_active is false then the 3 destinations will be cleared.
bool SrvRedirectForm::validate(bool cf_active,
			       QLineEdit *dst1, QLineEdit *dst2, QLineEdit *dst3,
			       list<t_display_url> &dest_list)
{
	t_display_url destination;
	
	dest_list.clear();
	
	if (!cf_active) {
		dst1->clear();
		dst2->clear();
		dst3->clear();
		return true;
	}
	
	// 1st choice destination
	ui->expand_destination(current_user, dst1->text().stripWhiteSpace().ascii(), destination);
	if (destination.is_valid()) {
		dest_list.push_back(destination);
	} else {
		dst1->selectAll();
		return false;
	}
	
	// 2nd choice destination
	if (!dst2->text().isEmpty()) {
		ui->expand_destination(current_user, 
				       dst2->text().stripWhiteSpace().ascii(), destination);
		if (destination.is_valid()) {
			dest_list.push_back(destination);
		} else {
			dst2->selectAll();
			return false;
		}
	}
	
	// 3rd choice destination
	if (!dst3->text().isEmpty()) {
		ui->expand_destination(current_user,
				       dst3->text().stripWhiteSpace().ascii(), destination);
		if (destination.is_valid()) {
			dest_list.push_back(destination);
		} else {
			dst3->selectAll();
			return false;
		}
	}
	
	return true;
}

void SrvRedirectForm::toggleAlways(bool on)
{
	if (on) {
		cfAlwaysGroupBox->setEnabled(true);
	} else {
		cfAlwaysGroupBox->setEnabled(false);
	}
}

void SrvRedirectForm::toggleBusy(bool on)
{
	if (on) {
		cfBusyGroupBox->setEnabled(true);
	} else {
		cfBusyGroupBox->setEnabled(false);
	}
}

void SrvRedirectForm::toggleNoanswer(bool on)
{
	if (on) {
		cfNoanswerGroupBox->setEnabled(true);
	} else {
		cfNoanswerGroupBox->setEnabled(false);
	}
}

void SrvRedirectForm::changedUser(const QString &user_profile)
{
	if (current_user_idx == -1) {
		// Initializing combo box
		return;
	}
	
	t_user *new_user = phone->ref_user_profile(user_profile.ascii());
	if (!new_user) {
		userComboBox->setCurrentItem(current_user_idx);
		return;
	}
	
	if (!validateValues()) {
		userComboBox->setCurrentItem(current_user_idx);
		((t_gui *)ui)->cb_show_msg(this,
			tr("You have entered an invalid destination.").ascii(),
			MSG_WARNING);
		return;
	}
	
	// Change current user
	current_user_idx = userComboBox->currentItem();
	current_user = new_user;
	populate();
}

void SrvRedirectForm::showAddressBook()
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

void SrvRedirectForm::showAddressBook1()
{
	nrAddressBook = 1;
	showAddressBook();
}

void SrvRedirectForm::showAddressBook2()
{
	nrAddressBook = 2;
	showAddressBook();
}

void SrvRedirectForm::showAddressBook3()
{
	nrAddressBook = 3;
	showAddressBook();
}

void SrvRedirectForm::showAddressBook4()
{
	nrAddressBook = 4;
	showAddressBook();
}

void SrvRedirectForm::showAddressBook5()
{
	nrAddressBook = 5;
	showAddressBook();
}

void SrvRedirectForm::showAddressBook6()
{
	nrAddressBook = 6;
	showAddressBook();
}

void SrvRedirectForm::showAddressBook7()
{
	nrAddressBook = 7;
	showAddressBook();
}

void SrvRedirectForm::showAddressBook8()
{
	nrAddressBook = 8;
	showAddressBook();
}

void SrvRedirectForm::showAddressBook9()
{
	nrAddressBook = 9;
	showAddressBook();
}

void SrvRedirectForm::selectedAddress(const QString &address)
{
	switch(nrAddressBook) {
	case 1:
		cfAlwaysDst1LineEdit->setText(address);
		break;
	case 2:
		cfAlwaysDst2LineEdit->setText(address);
		break;
	case 3:
		cfAlwaysDst3LineEdit->setText(address);
		break;
	case 4:
		cfBusyDst1LineEdit->setText(address);
		break;
	case 5:
		cfBusyDst2LineEdit->setText(address);
		break;
	case 6:
		cfBusyDst3LineEdit->setText(address);
		break;
	case 7:
		cfNoanswerDst1LineEdit->setText(address);
		break;
	case 8:
		cfNoanswerDst2LineEdit->setText(address);
		break;
	case 9:
		cfNoanswerDst3LineEdit->setText(address);
		break;
	}
}
