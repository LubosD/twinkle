/***************************************************************************
 *   Copyright (C) 2004 by Emil Stoyanov                                   *
 *   emosto@users.sourceforge.net                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

// 2006 Modified by Michel de Boer

#ifndef FREEDESKSYSTEMTRAY_H
#define FREEDESKTSYSTEMTRAY_H

#include <qlabel.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qwidget.h>
#include <qapplication.h>
#include <qpoint.h>
#include <qsize.h>
#include <qtooltip.h>
#include <qbitmap.h>
#include <qimage.h>
#include <qtimer.h>
#include <qrect.h>

#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <X11/Xatom.h>
#include <X11/Xlib.h>


// System Tray Protocol Specification opcodes.
#define SYSTEM_TRAY_REQUEST_DOCK    0
#define SYSTEM_TRAY_BEGIN_MESSAGE   1
#define SYSTEM_TRAY_CANCEL_MESSAGE  2


class FreeDeskSysTray : public QLabel
{
    Q_OBJECT

public:

    FreeDeskSysTray(QWidget *pParent = 0, const char *pszName = 0);
    ~FreeDeskSysTray();

    void setPixmapOverlay(const QPixmap& pmOverlay);
    void showContextMenu(const QPoint& position);
    void dock ();
    void undock ();
    QPopupMenu *contextMenu();
    void setPixmap(const QPixmap& pixmap);

public:

    QWidget * mainWindow;
    QPopupMenu * trayMenu;
    
protected:

    void mousePressEvent(QMouseEvent *);
    
public slots:

    void slotMenuItemShow();
    void slotMenuItemQuit();    
    
signals:

    void quitSelected();
    
private:
    QRect mainWindowGeometry;    
    QPoint windowPos;
};

// ifndef FREEDESKSYSTEMTRAY_H
#endif
