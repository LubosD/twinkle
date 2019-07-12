/*
   (This file was initially copied from qBittorrent.)

   Copyright (C) 2019  Vladimir Golovnev <glassez@yandex.ru>
                       Frédéric Brière <fbriere@fbriere.net>

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

#ifndef IDLESESSION_INHIBITOR_H
#define IDLESESSION_INHIBITOR_H

#include <QObject>

// Forward declaration
QT_BEGIN_NAMESPACE
class QDBusPendingCallWatcher;
QT_END_NAMESPACE

// Object tasked with inhibiting idle sessions via D-Bus
class IdleSessionInhibitor : public QObject
{
	Q_OBJECT

public:
	IdleSessionInhibitor(QObject *parent = 0);

	void requestBusy();
	void requestIdle();

private slots:
	// Handle the reply to our (un)inhibit request
	void onAsyncReply(QDBusPendingCallWatcher *call);

private:
	// Internal state of the inhibitor
	enum _state
	{
		error,
		busy,
		idle,
		request_busy,
		request_idle
	};
	// Current state
	enum _state m_state;
	// Most recent idle/busy setting, possibly pending while we're waiting
	// for a reply to our previous D-Bus call
	enum _state m_intended_state;

	// Cookie returned by Inhibit(), to be passed to Uninhibit()
	unsigned int m_cookie;

	// Whether or not we are dealing with GNOME Session
	bool m_use_gsm;

	// Send an (un)inhibit request via D-Bus
	void sendRequest(bool inhibit);
};

#endif // IDLESESSION_INHIBITOR_H
