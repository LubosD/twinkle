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

#include "getaddressform.h"
#include <QRegExp>
#include "sys_settings.h"
#include "gui.h"
#include "address_book.h"
#include "addresstablemodel.h"
#include "addresscardform.h"
#define TAB_KABC	0
#define TAB_LOCAL	1

#ifdef HAVE_KDE
#include <kabc/addressbook.h>
#include <kabc/addressee.h>
#include <kabc/addresseelist.h>
#include <kabc/phonenumber.h>
#include <kabc/stdaddressbook.h>

#define ABOOK	((KABC::AddressBook *)addrBook)

// Column numbers
#define AB_COL_NAME	0
#define AB_COL_PHONE	2
#endif

GetAddressForm::GetAddressForm(QWidget *parent)
    : QDialog(parent)
{
	setupUi(this);
	init();

	m_model = new AddressTableModel(this, ab_local->get_address_list());
	localListView->setModel(m_model);
}

GetAddressForm::~GetAddressForm()
{
	// destroy();
}

void GetAddressForm::init() 
{
#ifdef HAVE_KDE
	addrBook = (void *)KABC::StdAddressBook::self(false);
	loadAddresses();
	
	connect(ABOOK, 
		SIGNAL(addressBookChanged(AddressBook *)),
		this, SLOT(loadAddresses()));
	
	sipOnlyCheckBox->setChecked(sys_config->get_ab_show_sip_only());
#else
    addressTabWidget->setTabEnabled(addressTabWidget->indexOf(tabKABC), false);
    addressTabWidget->setCurrentIndex(TAB_LOCAL);
#endif
}

void GetAddressForm::reload()
{
#ifdef HAVE_KDE
	ABOOK->disconnect();
	KABC::StdAddressBook::close();
	addrBook = (void *)KABC::StdAddressBook::self(false);
	loadAddresses();
	
	connect(ABOOK, 
		SIGNAL(addressBookChanged(AddressBook *)),
		this, SLOT(loadAddresses()));
#endif
}

void GetAddressForm::show()
{
	QDialog::show();
	
#ifdef HAVE_KDE
	if (addressListView->childCount() == 0) {
		if (localListView->childCount() == 0) {
			QMessageBox::information(this, PRODUCT_NAME, tr(
				"<p>"
				"You seem not to have any contacts with a phone number "
				"in <b>KAddressBook</b>, KDE's address book application. "
				"Twinkle retrieves all contacts with a phone number from "
				"KAddressBook. To manage your contacts you have to "
				"use KAddressBook."
				"<p>"
				"As an alternative you may use Twinkle's local address book."
				"</p>"));
		} else {
			addressTabWidget->setCurrentPage(TAB_LOCAL);
		}
	}
#endif
}

void GetAddressForm::loadAddresses()
{
#ifdef HAVE_KDE
	// Explicit loading of address book is not needed as it is 
	// automatically loaded.
	// if (!ABOOK->load()) return;
	
	addressListView->clear();
	for (KABC::AddressBook::Iterator i = ABOOK->begin(); i != ABOOK->end(); i++)
	{
		KABC::PhoneNumber::List phoneNrs = i->phoneNumbers();
		for (KABC::PhoneNumber::List::iterator j = phoneNrs.begin();
		j != phoneNrs.end(); j++)
		{
			QString phone = (*j).number();
			if (!sys_config->get_ab_show_sip_only() ||
			    phone.startsWith("sip:"))
			{
				new Q3ListViewItem(addressListView, i->realName(),
					  (*j).typeLabel(), phone);
			}
		}
	}
	
	Q3ListViewItem *first = addressListView->firstChild();
	if (first) addressListView->setSelected(first, true);
#endif
}

void GetAddressForm::selectAddress()
{
    if (addressTabWidget->currentIndex() == TAB_KABC) {
		selectKABCAddress();
	} else {
		selectLocalAddress();
	}
}

void GetAddressForm::selectKABCAddress()
{
#ifdef HAVE_KDE
	Q3ListViewItem *item = addressListView->selectedItem();
	if (item) {
		QString name(item->text(AB_COL_NAME));
		QString phone(item->text(AB_COL_PHONE));
		phone = phone.stripWhiteSpace();
			
		emit address(name, phone);
		
		// Signal display name and url combined.
		t_display_url du(t_url(phone.ascii()), name.ascii());
		emit address(du.encode().c_str());
	}
	
	accept();
#endif
}

void GetAddressForm::selectLocalAddress()
{
	QModelIndexList sel = localListView->selectionModel()->selectedRows();
	if (!sel.isEmpty())
	{
		t_address_card card = m_model->getAddress(sel[0].row());
		emit(QString::fromStdString(card.get_display_name()), QString::fromStdString(card.sip_address));

		// Signal display name and url combined.
		t_display_url du(t_url(card.sip_address), card.get_display_name());
		emit address(du.encode().c_str());
	}
	
	accept();
}

void GetAddressForm::toggleSipOnly(bool on)
{
#ifdef HAVE_KDE
	string msg;
	
	sys_config->set_ab_show_sip_only(on);
	
	// Ignore write failures. If for some reason the system config
	// could not be written, then this settings is lost after exiting Twinkle.
	// No need to bother the user at this point.
	(void)sys_config->write_config(msg);
	
	loadAddresses();
#endif
}

void GetAddressForm::addLocalAddress()
{
	t_address_card card;
	AddressCardForm f(this);
	if (f.exec(card)) {
		ab_local->add_address(card);

		m_model->appendAddress(card);
		
		string error_msg;
		if (!ab_local->save(error_msg)) {
			ui->cb_show_msg(error_msg, MSG_CRITICAL);
		}
	}
}

void GetAddressForm::deleteLocalAddress()
{
	QModelIndexList sel = localListView->selectionModel()->selectedRows();
	if (sel.isEmpty())
		return;

	t_address_card card = m_model->getAddress(sel[0].row());

	if (ab_local->del_address(card)) {
		m_model->removeAddress(sel[0].row());

		string error_msg;
		if (!ab_local->save(error_msg)) {
			ui->cb_show_msg(error_msg, MSG_CRITICAL);
		}
	}
}

void GetAddressForm::editLocalAddress()
{
	QModelIndexList sel = localListView->selectionModel()->selectedRows();
	if (sel.isEmpty())
		return;
	
	t_address_card oldCard = m_model->getAddress(sel[0].row());
	t_address_card newCard = oldCard;
	AddressCardForm f(this);
	if (f.exec(newCard)) {
		if (ab_local->update_address(oldCard, newCard)) {
			m_model->modifyAddress(sel[0].row(), newCard);
			
			string error_msg;
			if (!ab_local->save(error_msg)) {
				ui->cb_show_msg(error_msg, MSG_CRITICAL);
			}
		}
	}
}
