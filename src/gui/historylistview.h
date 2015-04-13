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

#ifndef _HISTORYLISTVIEW_H
#define _HISTORYLISTVIEW_H

#include <sys/time.h>
#include "qlistview.h"
#include "qpainter.h"
#include "call_history.h"
#include "user.h"

// Columns of the history list view
#define HISTCOL_TIMESTAMP 	0
#define HISTCOL_DIRECTION	1
#define HISTCOL_FROMTO		2
#define HISTCOL_SUBJECT	3
#define HISTCOL_STATUS		4

class HistoryListViewItem : public QListViewItem {
private:
	t_call_record	call_record;
	time_t		last_viewed;
	
public:
	HistoryListViewItem( QListView * parent, const t_call_record &cr, t_user *user_config,
			     time_t _last_viewed);
	
	void paintCell(QPainter *painter, const QColorGroup &cg, 
				    int column, int width, int align);
	int compare ( QListViewItem * i, int col, bool ascending ) const;
	time_t get_time_start(void) const;
	t_call_record get_call_record(void) const;
};

#endif

