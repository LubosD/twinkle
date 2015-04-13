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

#ifndef _TWINKLESYSTRAY_H
#define _TWINKLESYSTRAY_H

#include "twinkle_config.h"

#ifdef HAVE_KDE
#include <ksystemtray.h>
#include <kpopupmenu.h>
#else
#include "freedesksystray.h"
#endif

#include "qpopupmenu.h"
#include "qwidget.h"

#ifdef HAVE_KDE
class t_twinkle_sys_tray : public KSystemTray {
#else
class t_twinkle_sys_tray : public FreeDeskSysTray {
#endif
	
public:
	t_twinkle_sys_tray(QWidget *pParent = 0, const char *pszName = 0);
	~t_twinkle_sys_tray();
	
	void dock();
};

#endif
