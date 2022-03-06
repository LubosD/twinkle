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

#include "getaddressform.h"
#include <QMessageBox>
#include <QRegExp>
#include "sys_settings.h"
#include "gui.h"
#include "address_book.h"
#include "addresstablemodel.h"
#include "addresscardform.h"
#include "audits/memman.h"
#define TAB_AKONADI	0
#define TAB_LOCAL	1

#ifdef HAVE_AKONADI
#include "akonadiaddressbook.h"
#include "kcontactstablemodel.h"

// Column numbers
#define AB_COL_NAME	0
#define AB_COL_PHONE	2
#endif

GetAddressForm::GetAddressForm(QWidget *parent)
    : QDialog(parent)
{
	setupUi(this);

	m_model = new AddressTableModel(this, ab_local->get_address_list());
	localListView->setModel(m_model);
#ifdef HAVE_AKONADI
	k_model = new KContactsTableModel(this);
	addressListView->setModel(k_model);
#endif

	init();

	localListView->sortByColumn(COL_ADDR_NAME, Qt::AscendingOrder);
	addressListView->sortByColumn(COL_ADDR_NAME, Qt::AscendingOrder);

	localListView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	addressListView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

GetAddressForm::~GetAddressForm()
{
	// destroy();
}

void GetAddressForm::init() 
{
#ifdef HAVE_AKONADI
	loadAddresses();
	
	connect(AkonadiAddressBook::self(),
		SIGNAL(addressBookChanged()),
		this, SLOT(loadAddresses()));
	
	sipOnlyCheckBox->setChecked(sys_config->get_ab_show_sip_only());
#else
    addressTabWidget->setTabEnabled(addressTabWidget->indexOf(tabAkonadi), false);
    addressTabWidget->setCurrentIndex(TAB_LOCAL);
#endif
}

void GetAddressForm::reload()
{
#ifdef HAVE_AKONADI
	AkonadiAddressBook::self()->reload();
#endif
}

void GetAddressForm::show()
{
	QDialog::show();
	
#ifdef HAVE_AKONADI
	if (k_model->rowCount() == 0) {
		if (m_model->rowCount() == 0) {
			QMessageBox::information(this, PRODUCT_NAME, tr(
				"<p>"
				"You seem not to have any contacts with a phone number "
				"in <b>Akonadi</b>, KDE's PIM Storage Service. "
				"Twinkle retrieves all contacts with a phone number from "
				"Akonadi. To manage your contacts you have to "
				"use another application such as KAddressBook or Kontact."
				"<p>"
				"As an alternative you may use Twinkle's local address book."
				"</p>"));
		} else {
			addressTabWidget->setCurrentIndex(TAB_LOCAL);
		}
	}
#endif
}

void GetAddressForm::loadAddresses()
{
#ifdef HAVE_AKONADI
	k_model->loadContacts(AkonadiAddressBook::self()->get_contacts(),
			      sys_config->get_ab_show_sip_only());
	
	addressListView->sortByColumn(addressListView->horizontalHeader()->sortIndicatorSection(), addressListView->horizontalHeader()->sortIndicatorOrder());
#endif
}

void GetAddressForm::selectAddress()
{
    if (addressTabWidget->currentIndex() == TAB_AKONADI) {
		selectAkonadiAddress();
	} else {
		selectLocalAddress();
	}
}

void GetAddressForm::selectAkonadiAddress()
{
#ifdef HAVE_AKONADI
	QModelIndexList sel = addressListView->selectionModel()->selectedRows();
	if (!sel.isEmpty())
	{
		t_address_card card = k_model->getAddress(sel[0].row());
		emit address(QString::fromStdString(card.get_display_name()),
				QString::fromStdString(card.sip_address));
		
		// Signal display name and url combined.
		t_display_url du(t_url(card.sip_address), card.get_display_name());
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
		emit address(QString::fromStdString(card.get_display_name()), QString::fromStdString(card.sip_address));

		// Signal display name and url combined.
		t_display_url du(t_url(card.sip_address), card.get_display_name());
		emit address(du.encode().c_str());
	}
	
	accept();
}

void GetAddressForm::toggleSipOnly(bool on)
{
#ifdef HAVE_AKONADI
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

		localListView->sortByColumn(localListView->horizontalHeader()->sortIndicatorSection(), localListView->horizontalHeader()->sortIndicatorOrder());
		
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

	QString card_name = QString::fromStdString(card.get_display_name());
	QString msg = tr("Are you sure you want to delete contact '%1' from the local address book?").arg(card_name);
	QMessageBox *mb = new QMessageBox(tr("Delete contact"), msg,
			QMessageBox::Warning,
			QMessageBox::Yes,
			QMessageBox::No,
			QMessageBox::NoButton,
			this);
	MEMMAN_NEW(mb);

	if (mb->exec() == QMessageBox::Yes) {
		if (ab_local->del_address(card)) {
			m_model->removeAddress(sel[0].row());

			string error_msg;
			if (!ab_local->save(error_msg)) {
				ui->cb_show_msg(error_msg, MSG_CRITICAL);
			}
		}
	}

	MEMMAN_DELETE(mb);
	delete mb;
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

			localListView->sortByColumn(localListView->horizontalHeader()->sortIndicatorSection(), localListView->horizontalHeader()->sortIndicatorOrder());
			
			string error_msg;
			if (!ab_local->save(error_msg)) {
				ui->cb_show_msg(error_msg, MSG_CRITICAL);
			}
		}
	}
}
