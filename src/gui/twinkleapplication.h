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

#ifndef _TWINKLEAPPLICATION_H
#define _TWINKLEAPPLICATION_H

#include "twinkle_config.h"

#ifdef HAVE_KDE
#include <kapplication.h>
#else
#include <qapplication.h>
#endif

#ifdef HAVE_KDE
class t_twinkle_application : public KApplication {
#else
class t_twinkle_application : public QApplication {
#endif
public:
#ifdef HAVE_KDE
	t_twinkle_application();
#else
	t_twinkle_application(int &argc, char **argv);
#endif
	virtual void commitData ( QSessionManager &sm );
};

#endif
