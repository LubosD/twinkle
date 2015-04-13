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

int AddressCardForm::exec(t_address_card &card) {
	firstNameLineEdit->setText(card.name_first.c_str());
	infixNameLineEdit->setText(card.name_infix.c_str());
	lastNameLineEdit->setText(card.name_last.c_str());
	phoneLineEdit->setText(card.sip_address.c_str());
	remarkLineEdit->setText(card.remark.c_str());
	
	int retval = QDialog::exec();
	
	if (retval == QDialog::Accepted) {
		card.name_first = firstNameLineEdit->text().stripWhiteSpace().ascii();
		card.name_infix = infixNameLineEdit->text().stripWhiteSpace().ascii();
		card.name_last = lastNameLineEdit->text().stripWhiteSpace().ascii();
		card.sip_address = phoneLineEdit->text().stripWhiteSpace().ascii();
		card.remark = remarkLineEdit->text().stripWhiteSpace().ascii();
	}
	
	return retval;
}

void AddressCardForm::validate()
{
	QString firstName = firstNameLineEdit->text().stripWhiteSpace();
	QString infixName = infixNameLineEdit->text().stripWhiteSpace();
	QString lastName = lastNameLineEdit->text().stripWhiteSpace();
	QString phone = phoneLineEdit->text().stripWhiteSpace();
	
	if (firstName.isEmpty() && infixName.isEmpty() && lastName.isEmpty()) {
		((t_gui *)ui)->cb_show_msg(this,  
			tr("You must fill in a name.").ascii(), MSG_CRITICAL);
		firstNameLineEdit->setFocus();
		return;
	}
	
	if (phone.isEmpty()) {
		((t_gui *)ui)->cb_show_msg(this,  
			tr("You must fill in a phone number or SIP address.").ascii(), MSG_CRITICAL);
		phoneLineEdit->setFocus();
		return;
	}
	
	accept();
}
