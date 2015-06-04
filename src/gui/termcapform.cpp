#include "termcapform.h"

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

#include <qvariant.h>
#include <qimage.h>
#include <qpixmap.h>

#include "gui.h"
#include "audits/memman.h"
#include "termcapform.h"
/*
 *  Constructs a TermCapForm as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
TermCapForm::TermCapForm(QWidget* parent)
    : QDialog(parent)
{
	setupUi(this);

	init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
TermCapForm::~TermCapForm()
{
	destroy();
	// no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void TermCapForm::languageChange()
{
	retranslateUi(this);
}


void TermCapForm::init()
{
	getAddressForm = 0;
	
	// Set toolbutton icons for disabled options.
	setDisabledIcon(addressToolButton, ":/icons/images/kontact_contacts-disabled.png");
}

void TermCapForm::show(t_user *user_config, const QString &dest)
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
                fromComboBox->currentText().toStdString());
	
	ui->expand_destination(from_user, 
                   partyLineEdit->text().trimmed().toStdString(),
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
        getAddressForm = new GetAddressForm(this);
        getAddressForm->setModal(true);
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
