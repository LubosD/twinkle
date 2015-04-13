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

void BuddyForm::init()
{
	getAddressForm = 0;
}

void BuddyForm::destroy()
{
	if (getAddressForm) {
		MEMMAN_DELETE(getAddressForm);
		delete getAddressForm;
	}
}

void BuddyForm::showNew(t_buddy_list &_buddy_list, QListViewItem *_profileItem) 
{
	user_config = _buddy_list.get_user_profile();
	edit_mode = false;
	buddy_list = &_buddy_list;
	profileItem = _profileItem;
	
	QDialog::show();
}

void BuddyForm::showEdit(t_buddy &buddy) 
{
	user_config = buddy.get_user_profile();
	edit_mode = true;
	edit_buddy = &buddy;
	buddy_list = edit_buddy->get_buddy_list();
	
	nameLineEdit->setText(buddy.get_name().c_str());
	phoneLineEdit->setText(buddy.get_sip_address().c_str());
	subscribeCheckBox->setChecked(buddy.get_may_subscribe_presence());
	
	phoneLineEdit->setEnabled(false);
	phoneTextLabel->setEnabled(false);
	addressToolButton->hide();
	
	QDialog::show();
}

void BuddyForm::validate()
{
	QString name = nameLineEdit->text().stripWhiteSpace();
	QString address = phoneLineEdit->text().stripWhiteSpace();
	
	if (name.isEmpty()) {
		((t_gui *)ui)->cb_show_msg(this,  
			tr("You must fill in a name.").ascii(), MSG_CRITICAL);
		nameLineEdit->setFocus();
		return;
	}
	
	string dest = ui->expand_destination(user_config, address.ascii());
	t_url dest_url(dest);
	if (!dest_url.is_valid()) {
		((t_gui *)ui)->cb_show_msg(this,  
			tr("Invalid phone.").ascii(), MSG_CRITICAL);
		phoneLineEdit->setFocus();
		return;
	}
	
	if (edit_mode) {
		// Edit existing buddy
		bool must_subscribe = false;
		bool must_unsubscribe = false;

		if (edit_buddy->get_may_subscribe_presence() != subscribeCheckBox->isChecked()) 
		{
			if (subscribeCheckBox->isChecked()) {
				must_subscribe = true;;
			} else {
				must_unsubscribe = true;
			}
		}
		
		edit_buddy->set_name(nameLineEdit->text().stripWhiteSpace().ascii());
		edit_buddy->set_sip_address(phoneLineEdit->text().stripWhiteSpace().ascii());
		edit_buddy->set_may_subscribe_presence(subscribeCheckBox->isChecked());
		
		if (must_subscribe) edit_buddy->subscribe_presence();
		if (must_unsubscribe) edit_buddy->unsubscribe_presence();
	} else {
		// Add a new buddy
		t_buddy buddy;
		buddy.set_name(nameLineEdit->text().stripWhiteSpace().ascii());
		buddy.set_sip_address(phoneLineEdit->text().stripWhiteSpace().ascii());
		buddy.set_may_subscribe_presence(subscribeCheckBox->isChecked());
		
		t_buddy *new_buddy = buddy_list->add_buddy(buddy);
		new BuddyListViewItem(profileItem, new_buddy);
		new_buddy->subscribe_presence();
	}
	
	string err_msg;
	if (!buddy_list->save(err_msg)) {
		QString msg = tr("Failed to save buddy list: %1").arg(err_msg.c_str());
		((t_gui *)ui)->cb_show_msg(this, msg.ascii(), MSG_CRITICAL);
	}
	
	accept();
}

void BuddyForm::showAddressBook()
{
	if (!getAddressForm) {
		getAddressForm = new GetAddressForm(
				this, "select address", true);
		MEMMAN_NEW(getAddressForm);
	}
	
	connect(getAddressForm, 
		SIGNAL(address(const QString &, const QString &)),
		this, SLOT(selectedAddress(const QString &, const QString &)));
	
	getAddressForm->show();
}

void BuddyForm::selectedAddress(const QString &name, const QString &phone)
{
	nameLineEdit->setText(name);
	phoneLineEdit->setText(phone);
}
