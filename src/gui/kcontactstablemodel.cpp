/*
    Copyright (C) 2018  Frédéric Brière <fbriere@fbriere.net>

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

#include "twinkle_config.h"

#include "kcontactstablemodel.h"


// The signature of this constructor makes `data` an optional argument
KContactsTableModel::KContactsTableModel(QObject *parent, const list<t_address_card>& data)
	: AddressTableModel(parent, data)
{
}

// Display the appropriate header for the "Remark" column
QVariant KContactsTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole && section == COL_ADDR_REMARK) {
		return tr("Type");
	} else {
		return AddressTableModel::headerData(section, orientation, role);
	}
}

// Replace the current list with a new one
void KContactsTableModel::loadContacts(const KContacts::Addressee::List& data, bool sip_only)
{
	beginResetModel();
	m_data.clear();
	for (const KContacts::Addressee& contact : data)
	{
		for (const KContacts::PhoneNumber& phone_number : contact.phoneNumbers())
		{
			if (!sip_only || phone_number.number().startsWith("sip:"))
				m_data.append(addressCard(contact, phone_number));
		}
	}
	endResetModel();
}

// Convert a KContacts Addressee+PhoneNumber into a t_address_card
t_address_card KContactsTableModel::addressCard(const KContacts::Addressee& contact, const KContacts::PhoneNumber& phone_number)
{
	t_address_card address_card;
	address_card.name_last = contact.realName().toStdString();
	address_card.sip_address = phone_number.number().toStdString();
	address_card.remark = phone_number.typeLabel().toStdString();
	return address_card;
}
