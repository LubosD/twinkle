#include "inviteform.h"
//Added by qt3to4:
#include <QCloseEvent>
#include "gui.h"
#include "util.h"
#include "audits/memman.h"
#include "sys_settings.h"
#include <QRegExp>
#include <QValidator>
#include <QRegExpValidator>

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

InviteForm::InviteForm(QWidget *parent)
    : QDialog(parent)
{
	setupUi(this);
	init();
}

InviteForm::~InviteForm()
{
	destroy();
}

void InviteForm::init()
{
	getAddressForm = 0;
	
	// Set toolbutton icons for disabled options.
	setDisabledIcon(addressToolButton, ":/icons/images/kontact_contacts-disabled.png");
	
	// A QComboBox accepts a new line through copy/paste.
	QRegExp rxNoNewLine("[^\\n\\r]*");
	inviteComboBox->setValidator(new QRegExpValidator(rxNoNewLine, this));
}

void InviteForm::destroy()
{
	if (getAddressForm) {
		MEMMAN_DELETE(getAddressForm);
		delete getAddressForm;
	}
}

void InviteForm::clear()
{
    inviteComboBox->clearEditText();
	subjectLineEdit->clear();
	hideUserCheckBox->setChecked(false);
	inviteComboBox->setFocus();
}

void InviteForm::show(t_user *user_config, const QString &dest, const QString &subject,
		      bool anonymous)
{
	((t_gui *)ui)->fill_user_combo(fromComboBox);
	
	// Select from user
	if (user_config) {
		for (int i = 0; i < fromComboBox->count(); i++) {
            if (fromComboBox->itemText(i) ==
			    user_config->get_profile_name().c_str())
			{
                fromComboBox->setCurrentIndex(i);
				break;
			}
		}
	}
	
	inviteComboBox->setEditText(dest);
	subjectLineEdit->setText(subject);
	hideUserCheckBox->setChecked(anonymous);
	QDialog::show();
}

void InviteForm::validate()
{
	string display, dest_str;
	t_user *from_user = phone->ref_user_profile(
                fromComboBox->currentText().toStdString());
	
	ui->expand_destination(from_user, 
                   inviteComboBox->currentText().trimmed().toStdString(),
			       display, dest_str);
	t_url dest(dest_str);
	
	if (dest.is_valid()) {
		addToInviteComboBox(inviteComboBox->currentText());
		emit raw_destination(inviteComboBox->currentText());
		emit destination(from_user, display.c_str(), dest, subjectLineEdit->text(),
				 hideUserCheckBox->isChecked());
		accept();
	} else {
		inviteComboBox->setFocus();
		inviteComboBox->lineEdit()->selectAll();
	}
}

// Add a destination to the history list of inviteComboBox
void InviteForm::addToInviteComboBox(const QString &destination)
{
    inviteComboBox->insertItem(0, destination);
	if (inviteComboBox->count() > SIZE_REDIAL_LIST) {
		inviteComboBox->removeItem(inviteComboBox->count() - 1);
	}
}


void InviteForm::reject()
{
	// Unseize the line
	((t_gui *)ui)->action_unseize();
	QDialog::reject();
}

void InviteForm::closeEvent(QCloseEvent *)
{
	reject();
}

void InviteForm::showAddressBook()
{
	if (!getAddressForm) {
        getAddressForm = new GetAddressForm(this);
        getAddressForm->setModal(true);
		MEMMAN_NEW(getAddressForm);
	}
	
	connect(getAddressForm, 
		SIGNAL(address(const QString &)),
		this, SLOT(selectedAddress(const QString &)));
	
	getAddressForm->show();
}

void InviteForm::selectedAddress(const QString &address)
{
	inviteComboBox->setEditText(address);
}

void InviteForm::warnHideUser(void) {
	// Warn only once
	if (!sys_config->get_warn_hide_user()) return;
	
	QString msg = tr("Not all SIP providers support identity hiding. Make sure your SIP provider "
			 "supports it if you really need it.");
    ((t_gui *)ui)->cb_show_msg(this, msg.toStdString(), MSG_WARNING);
	
	// Do not warn again
	sys_config->set_warn_hide_user(false);
}
