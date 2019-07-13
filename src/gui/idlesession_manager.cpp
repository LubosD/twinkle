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

#include "twinkle_config.h"

#include "idlesession_manager.h"
#ifdef HAVE_DBUS
#include "idlesession_inhibitor.h"
#endif

#include "log.h"

#include <assert.h>

IdleSessionManager::IdleSessionManager(QObject *parent)
	: QObject(parent)
{
	// Creation of the inhibitor will only take place in setEnabled(),
	// to prevent issueing a warning if no management service is found
	// when the option has not been enabled in the first place.

	m_busy = false;
	m_inhibitor = nullptr;
}

void IdleSessionManager::setEnabled(bool enabled)
{
	// Make sure logging has been made available at this point
	assert(log_file);

#ifdef HAVE_DBUS
	// Create our inhibitor if enabling for the first time
	if (enabled && !m_inhibitor)
		m_inhibitor = new IdleSessionInhibitor(this);

	// Changing this setting while busy requires special handling
	if ((enabled != m_enabled) && m_busy) {
		// We need to directly call request*() methods, as the current
		// values of m_enabled and m_busy would screw up set*()
		if (enabled)
			// Forward the current state to the new inhibitor
			m_inhibitor->requestBusy();
		else
			// Switch back to idle before going silent
			m_inhibitor->requestIdle();
	}
#endif

	m_enabled = enabled;
}

void IdleSessionManager::setActivityState(bool busy)
{
	if (busy)
		setBusy();
	else
		setIdle();
}

void IdleSessionManager::setBusy()
{
	if (m_busy)
		return;
	m_busy = true;

#ifdef HAVE_DBUS
	if (m_enabled && m_inhibitor)
		m_inhibitor->requestBusy();
#endif
}

void IdleSessionManager::setIdle()
{
	if (!m_busy)
		return;
	m_busy = false;

#ifdef HAVE_DBUS
	if (m_enabled && m_inhibitor)
		m_inhibitor->requestIdle();
#endif
}
