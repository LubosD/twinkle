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

#ifndef _KCONTACTSTABLEMODEL_H
#define _KCONTACTSTABLEMODEL_H

#include <KContacts/Addressee>
#include <KContacts/AddresseeList>

#include "addresstablemodel.h"


// Variation of AddressTableModel, slightly tweaked for a list of KContacts:
//
//  - The list is optional at creation time
//  - The list can be reloaded
//  - The "Remark" column holds the typeLabel property of the phone number

class KContactsTableModel : public AddressTableModel
{
public:
	KContactsTableModel(QObject *parent, const list<t_address_card>& data = list<t_address_card>());
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

	void loadContacts(const KContacts::Addressee::List& data, bool sip_only = false);

private:
	static t_address_card addressCard(const KContacts::Addressee& contact, const KContacts::PhoneNumber& phone_number);
};

#endif
