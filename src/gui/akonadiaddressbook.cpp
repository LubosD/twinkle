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

#include "akonadiaddressbook.h"

#include <akonadi_version.h>
#if AKONADI_VERSION >= QT_VERSION_CHECK(5, 18, 41)
#include <Akonadi/ItemFetchScope>
#include <Akonadi/RecursiveItemFetchJob>
#else
#include <AkonadiCore/ItemFetchScope>
#include <AkonadiCore/RecursiveItemFetchJob>
#endif

#include <KContacts/Addressee>

#include "events.h"
#include "log.h"
#include "userintf.h"


AkonadiAddressBook::AkonadiAddressBook()
{
	// Monitor all collections
	monitor.setCollectionMonitored(Akonadi::Collection::root());
	monitor.setMimeTypeMonitored(KContacts::Addressee::mimeType());
	monitor.itemFetchScope().fetchFullPayload();

	connect(&monitor, &Akonadi::Monitor::itemAdded,
			this, &AkonadiAddressBook::itemAdded);
	connect(&monitor, &Akonadi::Monitor::itemChanged,
			this, &AkonadiAddressBook::itemChanged);
	connect(&monitor, &Akonadi::Monitor::itemsRemoved,
			this, &AkonadiAddressBook::itemsRemoved);

	// Load the list of contacts asynchronously
	reload();
}

AkonadiAddressBook *AkonadiAddressBook::self()
{
	// This is thread-safe in C++11: https://stackoverflow.com/a/449823
	static AkonadiAddressBook instance;

	return &instance;
}


// The following two methods were mostly copied from KAddressBook
void AkonadiAddressBook::reload()
{
	Akonadi::RecursiveItemFetchJob *job =
		new Akonadi::RecursiveItemFetchJob(
				Akonadi::Collection::root(),
				QStringList() << KContacts::Addressee::mimeType());
	job->fetchScope().fetchFullPayload();

	connect(job, &Akonadi::RecursiveItemFetchJob::result,
			this, &AkonadiAddressBook::itemFetchJobFinished);

	// Despite what the documentation claims, this is *not* done
	// automatically.  (For most jobs, this is a no-op, so it does not
	// matter anyway.  But this is not the case for *this* type of job.)
	job->start();
}

void AkonadiAddressBook::itemFetchJobFinished(KJob *job)
{
	// Unfortunately, RecursiveItemFetchJob does not implement the nicer
	// itemsReceived(Item::List), so we have to unwrap the job ourselves.
	if (job->error()) {
		log_file->write_report(job->errorString().toStdString(),
				"AkonadiAddressBook::reload",
				LOG_NORMAL, LOG_WARNING);
                ui->cb_show_msg(job->errorString().toStdString(), MSG_WARNING);
		return;
	}

	Akonadi::RecursiveItemFetchJob *fetchJob =
		qobject_cast<Akonadi::RecursiveItemFetchJob*>(job);

	mtx_abook.lock();

	contacts.clear();
	for (const Akonadi::Item &item : fetchJob->items())
		insertItem(item);

	mtx_abook.unlock();

	emit addressBookChanged();
}

KContacts::AddresseeList AkonadiAddressBook::get_contacts()
{
	mtx_abook.lock();
	// This QMap->QList->QVector conversion is probably inefficient, but
	// the list of contacts is probably not that large to matter anyway.
	KContacts::AddresseeList ret = KContacts::AddresseeList::fromList(contacts.values());
	mtx_abook.unlock();
	return ret;
}


// Slots connected to the Monitor signals
//
// Note that Monitor will emit at least one signal (often several, for some
// reason) per item, causing us to emit addressBookChanged(), and thus
// reloading the model, more often than necessary.  If this ever becomes a
// problem, we should probably setup a one-shot timer to merge our signals.

void AkonadiAddressBook::itemAdded(const Akonadi::Item &item, const Akonadi::Collection &collection)
{
	mtx_abook.lock();
	insertItem(item);
	mtx_abook.unlock();

	emit addressBookChanged();
}

void AkonadiAddressBook::itemChanged(const Akonadi::Item &item, const QSet<QByteArray> &partIdentifiers)
{
	mtx_abook.lock();
	insertItem(item);
	mtx_abook.unlock();

	emit addressBookChanged();
}

void AkonadiAddressBook::itemsRemoved(const Akonadi::Item::List &items)
{
	mtx_abook.lock();
	for (const Akonadi::Item &item : items)
		removeItem(item);
	mtx_abook.unlock();

	emit addressBookChanged();
}


// Private methods to insert/remove items (mtx_abook must be locked beforehand)

void AkonadiAddressBook::insertItem(const Akonadi::Item &item) {
	if (item.isValid() && item.hasPayload<KContacts::Addressee>())
		contacts[item.id()] = item.payload<KContacts::Addressee>();
}

void AkonadiAddressBook::removeItem(const Akonadi::Item &item) {
	contacts.remove(item.id());
}
