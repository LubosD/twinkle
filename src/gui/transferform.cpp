//Added by qt3to4:
#include <QPixmap>
#include <QCloseEvent>
#include <QVariant>
#include <QImage>
#include <QPixmap>
#include "gui.h"
#include "audits/memman.h"
#include "transferform.h"
/*
 *  Constructs a TransferForm as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
TransferForm::TransferForm(QWidget* parent)
    : QDialog(parent)
{
	setupUi(this);

	init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
TransferForm::~TransferForm()
{
	destroy();
	// no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void TransferForm::languageChange()
{
	retranslateUi(this);
}


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

void TransferForm::init()
{
	getAddressForm = 0;
	
	// Set toolbutton icons for disabled options.
	QIcon i;
    i = addressToolButton->icon();
    i.addPixmap(QPixmap(":/icons/images/kontact_contacts-disabled.png"), QIcon::Disabled);
    addressToolButton->setIcon(i);
}

void TransferForm::destroy()
{
	if (getAddressForm) {
		MEMMAN_DELETE(getAddressForm);
		delete getAddressForm;
	}
}

void TransferForm::initTransferOptions() 
{
	// Show possible transfer type options
	// Basic transfer is always possible.
	// If a line is idle, then a transfer with consultation is possible.
	// The line will be seized, so an incoming call cannot occupy it.
	// If both lines are busy, then the active line can be transferred
	// to the other line.
	unsigned short idle_line;
	if (phone->get_idle_line(idle_line)) {
		consult_line = (int)idle_line;
		phone->pub_seize(consult_line);	
		consultRadioButton->show();
		consultRadioButton->setChecked(true);
		otherLineRadioButton->hide();
	} else {
		consult_line = -1;
		consultRadioButton->hide();
		otherLineRadioButton->show();
		otherLineRadioButton->setChecked(true);
	}
}

void TransferForm::show(t_user *user)
{
	user_config = user;
	initTransferOptions();
	QDialog::show();
}

void TransferForm::show(t_user *user, const string &dest, t_transfer_type transfer_type)
{
	user_config = user;
	initTransferOptions();
	toLineEdit->setText(dest.c_str());
	
	switch (transfer_type) {
	case TRANSFER_CONSULT:
		consultRadioButton->setChecked(true);
		break;
	case TRANSFER_OTHER_LINE:
		otherLineRadioButton->setChecked(true);
		break;
	default:
		basicRadioButton->setChecked(true);
		break;
	}
	
	QDialog::show();
}

void TransferForm::hide()
{
	if (consult_line > -1) {
		phone->pub_unseize(consult_line);
	}
	
	QDialog::hide();
}

void TransferForm::reject()
{
	if (user_config->get_referrer_hold()) {
		((t_gui *)ui)->action_retrieve();
	}
	
	if (consult_line > -1) {
		phone->pub_unseize(consult_line);
	}
	
	QDialog::reject();
}

void TransferForm::validate()
{
	t_display_url dest;
    ui->expand_destination(user_config, toLineEdit->text().trimmed().toStdString(), dest);
	
	t_transfer_type transfer_type;
    if (consultRadioButton->isChecked()) {
		transfer_type = TRANSFER_CONSULT;
    } else if (otherLineRadioButton->isChecked()) {
		transfer_type = TRANSFER_OTHER_LINE;
	} else {
		transfer_type = TRANSFER_BASIC;
	}
	
	
	if (transfer_type == TRANSFER_OTHER_LINE || dest.is_valid()) {
		if (consult_line > -1) {
			phone->pub_unseize(consult_line);
		}	
		emit destination(dest, transfer_type);
		accept();
	} else {
		toLineEdit->selectAll();
	}
}

void TransferForm::closeEvent(QCloseEvent *)
{
	reject();
}

void TransferForm::showAddressBook()
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

void TransferForm::selectedAddress(const QString &address)
{
	toLineEdit->setText(address);
}

void TransferForm::setOtherLineAddress(bool on)
{
	if (on) {
		previousAddress = toLineEdit->text();
		unsigned short active_line = phone->get_active_line();
		unsigned short other_line = (active_line == 0 ? 1 : 0);
		QString address = ui->format_sip_address(user_config,
			phone->get_remote_display(other_line),
			phone->get_remote_uri(other_line)).c_str();
		toLineEdit->setText(address);
		toLineEdit->setEnabled(false);
		toLabel->setEnabled(false);
#ifdef HAVE_KDE
		addressToolButton->setEnabled(false);
#endif
	} else {
		toLineEdit->setText(previousAddress);
		toLineEdit->setEnabled(true);
		toLabel->setEnabled(true);
#ifdef HAVE_KDE
		addressToolButton->setEnabled(true);
#endif
	}
}
