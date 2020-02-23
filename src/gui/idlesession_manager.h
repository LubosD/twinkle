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

#ifndef IDLESESSION_MANAGER_H
#define IDLESESSION_MANAGER_H

#include <QObject>

// Forward declaration
class IdleSessionInhibitor;

// Object responsible for keeping track of this session's activity, whether
// D-Bus is supported or not
class IdleSessionManager : public QObject
{
	Q_OBJECT

public:
	IdleSessionManager(QObject *parent = 0);

	// Enable/disable inhibition of idle session
	void setEnabled(bool enabled);

	// Declare the session as busy/idle
	void setActivityState(bool busy);
	void setBusy();
	void setIdle();

private:
	bool m_enabled;
	bool m_busy;

	IdleSessionInhibitor *m_inhibitor;
};

#endif // IDLE_SESSION_MANAGER_H
