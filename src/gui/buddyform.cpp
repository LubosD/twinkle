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
#include "sockets/url.h"
#include "buddylistview.h"
#include "audits/memman.h"
#include "buddyform.h"
/*
 *  Constructs a BuddyForm as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
BuddyForm::BuddyForm(QWidget* parent)
    : QDialog(parent)
{
	setupUi(this);

	init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
BuddyForm::~BuddyForm()
{
	destroy();
	// no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void BuddyForm::languageChange()
{
	retranslateUi(this);
}


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

void BuddyForm::showNew(t_buddy_list &_buddy_list, QTreeWidgetItem *_profileItem)
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
    QString name = nameLineEdit->text().trimmed();
    QString address = phoneLineEdit->text().trimmed();
	
	if (name.isEmpty()) {
		((t_gui *)ui)->cb_show_msg(this,  
            tr("You must fill in a name.").toStdString(), MSG_CRITICAL);
		nameLineEdit->setFocus();
		return;
	}
	
    string dest = ui->expand_destination(user_config, address.toStdString());
	t_url dest_url(dest);
	if (!dest_url.is_valid()) {
		((t_gui *)ui)->cb_show_msg(this,  
            tr("Invalid phone.").toStdString(), MSG_CRITICAL);
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
		
        edit_buddy->set_name(nameLineEdit->text().trimmed().toStdString());
        edit_buddy->set_sip_address(phoneLineEdit->text().trimmed().toStdString());
		edit_buddy->set_may_subscribe_presence(subscribeCheckBox->isChecked());
		
		if (must_subscribe) edit_buddy->subscribe_presence();
		if (must_unsubscribe) edit_buddy->unsubscribe_presence();
	} else {
		// Add a new buddy
		t_buddy buddy;
        buddy.set_name(nameLineEdit->text().trimmed().toStdString());
        buddy.set_sip_address(phoneLineEdit->text().trimmed().toStdString());
		buddy.set_may_subscribe_presence(subscribeCheckBox->isChecked());
		
		t_buddy *new_buddy = buddy_list->add_buddy(buddy);
		new BuddyListViewItem(profileItem, new_buddy);
		new_buddy->subscribe_presence();
	}
	
	string err_msg;
	if (!buddy_list->save(err_msg)) {
		QString msg = tr("Failed to save buddy list: %1").arg(err_msg.c_str());
        ((t_gui *)ui)->cb_show_msg(this, msg.toStdString(), MSG_CRITICAL);
	}
	
	accept();
}

void BuddyForm::showAddressBook()
{
	if (!getAddressForm) {
        getAddressForm = new GetAddressForm(this);
        getAddressForm->setModal(true);
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
