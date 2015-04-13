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

#include "twinklesystray.h"
#include "audits/memman.h"

t_twinkle_sys_tray::t_twinkle_sys_tray(QWidget *pParent, const char *pszName) :
#ifdef HAVE_KDE
		KSystemTray(pParent, pszName)
#else
		FreeDeskSysTray(pParent, pszName)
#endif
{}

t_twinkle_sys_tray::~t_twinkle_sys_tray()
{}

void t_twinkle_sys_tray::dock()
{
#ifndef HAVE_KDE
	FreeDeskSysTray::dock();
#endif	
}
