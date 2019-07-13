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

#include "idlesession_inhibitor.h"

#include "log.h"
#include "userintf.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

// There are various standards for session/power management out there.
// Fortunately, most of them are either obsolete or redundant; the following
// two should cover most, if not all, cases.

// freedesktop.org Power Management Specification  (KDE, Xfce)
#define FDO_SERVICE          "org.freedesktop.PowerManagement"
#define FDO_PATH             "/org/freedesktop/PowerManagement/Inhibit"
#define FDO_INTERFACE        "org.freedesktop.PowerManagement.Inhibit"
#define FDO_METHOD_INHIBIT   "Inhibit"
#define FDO_METHOD_UNINHIBIT "UnInhibit"

// GNOME Session  (GNOME, MATE)
#define GSM_SERVICE          "org.gnome.SessionManager"
#define GSM_PATH             "/org/gnome/SessionManager"
#define GSM_INTERFACE        "org.gnome.SessionManager"
#define GSM_METHOD_INHIBIT   "Inhibit"
#define GSM_METHOD_UNINHIBIT "Uninhibit"
// Additional arguments required by GNOME's Inhibit()
// - toplevel_xid: The toplevel X window identifier
//                 (No idea what this does; everyone just uses 0)
#define GSM_ARG_TOPLEVEL_XID 0u
// - flags: Flags that spefify what should be inhibited
//          (8: Inhibit the session being marked as idle)
#define GSM_ARG_INHIBIT_FLAG 8u

// Common arguments to Inhibit()
#define INHIBIT_REASON       "Call in progress"
// (Included for PRODUCT_NAME)
#include "protocol.h"

// How long (in ms) to wait for a reply to our D-Bus (un)inhibit requests
#define DBUS_CALL_TIMEOUT    1000


IdleSessionInhibitor::IdleSessionInhibitor(QObject *parent)
	: QObject(parent)
{
	if (!QDBusConnection::sessionBus().isConnected()) {
		issueWarning(tr("D-Bus: Could not connect to session bus"));
		m_state = error;
		return;
	}

	auto interface = QDBusConnection::sessionBus().interface();
	if (interface->isServiceRegistered(FDO_SERVICE)) {
		m_use_gsm = false;
	} else if (interface->isServiceRegistered(GSM_SERVICE)) {
		m_use_gsm = true;
	} else {
		issueWarning(tr("D-Bus: No supported session/power management service found"));
		m_state = error;
		return;
	}

	m_state = idle;
	m_intended_state = idle;
	m_cookie = 0;
}

void IdleSessionInhibitor::sendRequest(bool inhibit)
{
	QDBusMessage call;
	if (!m_use_gsm)
		call = QDBusMessage::createMethodCall(
				FDO_SERVICE,
				FDO_PATH,
				FDO_INTERFACE,
				inhibit ? FDO_METHOD_INHIBIT : FDO_METHOD_UNINHIBIT);
	else
		call = QDBusMessage::createMethodCall(
				GSM_SERVICE,
				GSM_PATH,
				GSM_INTERFACE,
				inhibit ? GSM_METHOD_INHIBIT : GSM_METHOD_UNINHIBIT);

	QList<QVariant> args;
	if (inhibit) {
		args << PRODUCT_NAME;
		if (m_use_gsm)
			args << GSM_ARG_TOPLEVEL_XID;
		args << INHIBIT_REASON;
		if (m_use_gsm)
			args << GSM_ARG_INHIBIT_FLAG;
	} else {
		args << m_cookie;
	}
	call.setArguments(args);

	QDBusPendingCall pcall = QDBusConnection::sessionBus().asyncCall(call, DBUS_CALL_TIMEOUT);
	QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pcall, this);
	connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
			this, SLOT(onAsyncReply(QDBusPendingCallWatcher*)));
}

void IdleSessionInhibitor::requestBusy()
{
	m_intended_state = busy;
	if (m_state != idle)
		return;

	m_state = request_busy;

	sendRequest(true);
}

void IdleSessionInhibitor::requestIdle()
{
	m_intended_state = idle;
	if (m_state != busy)
		return;

	m_state = request_idle;

	sendRequest(false);
}

void IdleSessionInhibitor::onAsyncReply(QDBusPendingCallWatcher *call)
{
	switch (m_state) {
	case request_idle: {
		// Reply to "Uninhibit" has no return value
		QDBusPendingReply<> reply = *call;

		if (reply.isError()) {
			issueWarning(tr("D-Bus: Reply: Error: %1").arg(reply.error().message()));
			m_state = error;
		} else {
			m_state = idle;
			// Process any pending requestBusy() call
			if (m_intended_state == busy)
				requestBusy();
		}
		break;
	}
	case request_busy: {
		// Reply to "Inhibit" has a cookie as return value
		QDBusPendingReply<uint> reply = *call;

		if (reply.isError()) {
			issueWarning(tr("D-Bus: Reply: Error: %1").arg(reply.error().message()));
			m_state = error;
		} else {
			m_state = busy;
			m_cookie = reply.value();
			// Process any pending requestIdle() call
			if (m_intended_state == idle)
				requestIdle();
		}
		break;
	}
	default:
		issueWarning(tr("D-Bus: Unexpected reply in state %1").arg(m_state));
		m_state = error;
	}

	call->deleteLater();
}

void IdleSessionInhibitor::issueWarning(const QString &msg) const
{
	log_file->write_report(msg.toStdString(), "IdleSessionInhibitor",
			LOG_NORMAL, LOG_WARNING);
	ui->cb_display_msg(msg.toStdString(), MSG_WARNING);
}
