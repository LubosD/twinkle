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

#ifndef BUDDYLISTVIEW_H
#define BUDDYLISTVIEW_H

#include "qlistview.h"
#include "qpainter.h"
#include "qtooltip.h"
#include "presence/buddy.h"
#include "presence/presence_epa.h"
#include "patterns/observer.h"

class AbstractBLVItem : public QListViewItem {
protected:
	// Text to show as a tool tip.
	QString		tip;
	
	// Set the presence icon to reflect the presence state
	virtual void set_icon(t_presence_state::t_basic_state state);
	
public:
	AbstractBLVItem(QListViewItem *parent, const QString &text);
	AbstractBLVItem(QListView *parent, const QString &text);
	virtual ~AbstractBLVItem();
	virtual QString get_tip(void);
};

// List view item representing a buddy.
class BuddyListViewItem : public AbstractBLVItem, public patterns::t_observer {
private:
	t_buddy		*buddy;
	
	// Set the presence icon to reflect the buddy's presence
	void set_icon(void);
	
public:
	BuddyListViewItem(QListViewItem *parent, t_buddy *_buddy);
	virtual ~BuddyListViewItem();
	
	virtual void update(void);
	virtual void subject_destroyed(void);
	
	t_buddy *get_buddy(void);
};

// List view item representing a user
class BLViewUserItem : public AbstractBLVItem, public patterns::t_observer {
private:
	t_presence_epa *presence_epa;
	
	void set_icon(void);
	
public:
	BLViewUserItem(QListView *parent, t_presence_epa *_presence_epa);
	virtual ~BLViewUserItem();
	
	void paintCell(QPainter *painter, const QColorGroup &cg, 
				    int column, int width, int align);
	
	virtual void update(void);
	virtual void subject_destroyed(void);
	
	t_presence_epa *get_presence_epa(void);
};

class BuddyListViewTip : public QToolTip {
private:
	QListView *parentListView;
	
public:
	BuddyListViewTip(QListView *parent);
	virtual ~BuddyListViewTip() {};
	void maybeTip ( const QPoint & p );
};

#endif
