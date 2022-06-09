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

#ifndef _AKONADIADDRESSBOOK_H
#define _AKONADIADDRESSBOOK_H

#include "twinkle_config.h"

#include "threads/mutex.h"

#include <QObject>
#include <QMap>

#include <akonadi_version.h>
#if AKONADI_VERSION >= QT_VERSION_CHECK(5, 18, 41)
#include <Akonadi/Collection>
#include <Akonadi/Item>
#include <Akonadi/Monitor>
#else
#include <AkonadiCore/Collection>
#include <AkonadiCore/Item>
#include <AkonadiCore/Monitor>
#endif

#include <KContacts/Addressee>
#include <KContacts/AddresseeList>

// Forward declaration -- we cast this pointer to Akonadi::Job anyway
class KJob;


class AkonadiAddressBook : public QObject
{
private:
	Q_OBJECT

	t_mutex	mtx_abook;

	QMap<Akonadi::Item::Id, KContacts::Addressee> contacts;

	Akonadi::Monitor monitor;

	AkonadiAddressBook();

	void insertItem(const Akonadi::Item &item);
	void removeItem(const Akonadi::Item &item);

public:
	static AkonadiAddressBook *self();

	// Disable copy constructor and assignment operator
	AkonadiAddressBook(AkonadiAddressBook const&) = delete;
	void operator=(AkonadiAddressBook const&) = delete;

	void reload();

	KContacts::AddresseeList get_contacts();

private slots:
	void itemFetchJobFinished(KJob *job);

	void itemAdded(const Akonadi::Item &item,
			const Akonadi::Collection &collection);
	void itemChanged(const Akonadi::Item &item,
			const QSet<QByteArray> &partIdentifiers);
	void itemsRemoved(const Akonadi::Item::List &items);

signals:
	void addressBookChanged();
};

#endif
