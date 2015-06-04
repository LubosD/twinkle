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

#include "gui.h"
#include "addresscardform.h"

/*
 *  Constructs a AddressCardForm as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
AddressCardForm::AddressCardForm(QWidget* parent)
    : QDialog(parent)
{
	setupUi(this);

}

/*
 *  Destroys the object and frees any allocated resources
 */
AddressCardForm::~AddressCardForm()
{
	// no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void AddressCardForm::languageChange()
{
	retranslateUi(this);
}


int AddressCardForm::exec(t_address_card &card) {
	firstNameLineEdit->setText(card.name_first.c_str());
	infixNameLineEdit->setText(card.name_infix.c_str());
	lastNameLineEdit->setText(card.name_last.c_str());
	phoneLineEdit->setText(card.sip_address.c_str());
	remarkLineEdit->setText(card.remark.c_str());
	
	int retval = QDialog::exec();
	
	if (retval == QDialog::Accepted) {
        card.name_first = firstNameLineEdit->text().trimmed().toStdString();
        card.name_infix = infixNameLineEdit->text().trimmed().toStdString();
        card.name_last = lastNameLineEdit->text().trimmed().toStdString();
        card.sip_address = phoneLineEdit->text().trimmed().toStdString();
        card.remark = remarkLineEdit->text().trimmed().toStdString();
	}
	
	return retval;
}

void AddressCardForm::validate()
{
    QString firstName = firstNameLineEdit->text().trimmed();
    QString infixName = infixNameLineEdit->text().trimmed();
    QString lastName = lastNameLineEdit->text().trimmed();
    QString phone = phoneLineEdit->text().trimmed();
	
	if (firstName.isEmpty() && infixName.isEmpty() && lastName.isEmpty()) {
		((t_gui *)ui)->cb_show_msg(this,  
            tr("You must fill in a name.").toStdString(), MSG_CRITICAL);
		firstNameLineEdit->setFocus();
		return;
	}
	
	if (phone.isEmpty()) {
		((t_gui *)ui)->cb_show_msg(this,  
            tr("You must fill in a phone number or SIP address.").toStdString(), MSG_CRITICAL);
		phoneLineEdit->setFocus();
		return;
	}
	
	accept();
}
