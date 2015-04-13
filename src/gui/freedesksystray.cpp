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

#include "freedesksystray.h"

FreeDeskSysTray::FreeDeskSysTray ( QWidget *pParent , const char *pszName )
    : QLabel(pParent, pszName, WMouseNoMask | WRepaintNoErase | WType_TopLevel | WStyle_Customize | WStyle_NoBorder | WStyle_StaysOnTop)
{
  mainWindow = pParent;
  trayMenu = new QPopupMenu(this);
}

void FreeDeskSysTray::dock ()
{
  trayMenu->insertSeparator();
  trayMenu->insertItem(tr("Show/Hide"), this, SLOT(slotMenuItemShow())) ;
  
  QIconSet quitIcon(QPixmap::fromMimeSource("exit.png"));
  trayMenu->insertItem(quitIcon, tr("Quit"), this, SLOT(slotMenuItemQuit())) ;
  
  Display *dpy = QPaintDevice::x11AppDisplay();
  WId trayWin  = winId();

  // System Tray Protocol Specification from freedesktop.org
  Screen *screen = XDefaultScreenOfDisplay(dpy);
  int iScreen = XScreenNumberOfScreen(screen);
  char szAtom[32];
  snprintf(szAtom, sizeof(szAtom), "_NET_SYSTEM_TRAY_S%d", iScreen);
  Atom selectionAtom = XInternAtom(dpy, szAtom, false);
  XGrabServer(dpy);
  Window managerWin = XGetSelectionOwner(dpy, selectionAtom);
  XSelectInput(dpy, managerWin, StructureNotifyMask);
  XUngrabServer(dpy);
  XFlush(dpy);
  XEvent ev;
  memset(&ev, 0, sizeof(ev));
  ev.xclient.type = ClientMessage;
  ev.xclient.window = managerWin;
  ev.xclient.message_type = XInternAtom(dpy, "_NET_SYSTEM_TRAY_OPCODE", true);
  ev.xclient.format = 32;
  ev.xclient.data.l[0] = CurrentTime;
  ev.xclient.data.l[1] = SYSTEM_TRAY_REQUEST_DOCK;
  ev.xclient.data.l[2] = trayWin;
  ev.xclient.data.l[3] = 0;
  ev.xclient.data.l[4] = 0;
  XSendEvent(dpy, managerWin, false, NoEventMask, &ev);
  XSync(dpy, false);

  Atom trayAtom;
  // KDE 3
  WId forWin = mainWindow ? mainWindow->topLevelWidget()->winId() : qt_xrootwin();
  trayAtom = XInternAtom(dpy, "_KDE_NET_WM_SYSTEM_TRAY_WINDOW_FOR", false);
  XChangeProperty(dpy, trayWin, trayAtom, XA_WINDOW, 32, PropModeReplace, (unsigned char *) &forWin, 1);
  
  setMinimumSize(22, 22);
  setBackgroundMode(Qt::X11ParentRelative);
  
  // because of GNOME - needs a wait of at least 50-100 ms, otherwise width=1
  // KDocker solves the problem so (bug?)
  QTimer::singleShot(500, this, SLOT(show()));
  
}

void FreeDeskSysTray::undock ()
{
  XUnmapWindow(QPaintDevice::x11AppDisplay(), winId());
  hide();
}

FreeDeskSysTray::~FreeDeskSysTray ()
{}

void FreeDeskSysTray::mousePressEvent ( QMouseEvent *pMouseEvent )
{
  if (!QLabel::rect().contains(pMouseEvent->pos()))
    return;

  switch (pMouseEvent->button())
  {

  case LeftButton:
    slotMenuItemShow();
    break;

  case RightButton:
    showContextMenu(pMouseEvent->globalPos());
    break;

  default:
    break;
  }
}

void FreeDeskSysTray::setPixmapOverlay ( const QPixmap& pmOverlay )
{
  QWidget *pParent = parentWidget();
  if (pParent == 0)
    return;

  // Get base pixmap from parent widget.
  QPixmap pm;
  pm.convertFromImage(pParent->icon()->convertToImage().smoothScale(22, 22), 0);

  // Merge with the overlay pixmap.
  QBitmap bmMask(*pm.mask());
  bitBlt(&bmMask, 0, 0, pmOverlay.mask(), 0, 0, -1, -1, Qt::OrROP);
  pm.setMask(bmMask);
  bitBlt(&pm, 0, 0, &pmOverlay);

  QLabel::setPixmap(pm);
}

QPopupMenu *FreeDeskSysTray::contextMenu()
{
	return trayMenu;
}

 void FreeDeskSysTray::setPixmap(const QPixmap& pixmap)
{
	 QLabel::setPixmap(pixmap);
	 repaint(true);
}

void FreeDeskSysTray::showContextMenu(const QPoint& position)
{
   trayMenu->popup(position,0);
}

void FreeDeskSysTray::slotMenuItemShow() {
   
//     mainWindowGeometry = mainWindow->geometry();
//     windowPos = mainWindow->frameGeometry().topLeft();
   
   if (mainWindow->isVisible()) {
     //mainWindow->setGeometry(mainWindowGeometry);
     mainWindow->close();
   } else {
//     mainWindow->move( windowPos );          // restore position
     mainWindow->show();
   }
       
}
  
void FreeDeskSysTray::slotMenuItemQuit() {
   emit quitSelected();
}
