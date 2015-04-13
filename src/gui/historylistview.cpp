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

#include "historylistview.h"
#include "util.h"
#include "userintf.h"

#include "qpixmap.h"

HistoryListViewItem::HistoryListViewItem( QListView * parent, const t_call_record &cr, t_user *user_config, time_t _last_viewed) :
		QListViewItem(parent,
			      time2str(cr.time_start,  "%d %b %Y %H:%M:%S").c_str(),
			      cr.get_direction().c_str(),
			      (cr.direction == t_call_record::DIR_IN ?
			       ui->format_sip_address(user_config, 
					cr.from_display, cr.from_uri).c_str() :
			       ui->format_sip_address(user_config,
					cr.to_display, cr.to_uri).c_str()),
			      cr.subject.c_str(),
			      cr.invite_resp_reason.c_str())
{
	call_record = cr;
	last_viewed = _last_viewed;
	
	// Set direction icon
	setPixmap(HISTCOL_DIRECTION, (cr.direction == t_call_record::DIR_IN ?
			    QPixmap::fromMimeSource("1leftarrow-yellow.png") :
			    QPixmap::fromMimeSource("1rightarrow.png")));
		
	// Set status icon
	setPixmap(HISTCOL_STATUS, (cr.invite_resp_code < 300 ?
			    QPixmap::fromMimeSource("ok.png") :
			    QPixmap::fromMimeSource("cancel.png")));
}

void HistoryListViewItem::paintCell(QPainter *painter, const QColorGroup &cg, 
				    int column, int width, int align)
{
	painter->save();
	QColorGroup grp(cg);
	if (call_record.time_start > last_viewed &&
	    call_record.rel_cause == t_call_record::CS_FAILURE &&
	    call_record.direction == t_call_record::DIR_IN) 
	{
		// Highlight missed calls since last view
		grp.setColor(QColorGroup::Base, QColor("yellow"));
	}
	QListViewItem::paintCell(painter, grp, column, width, align);
	painter->restore();
}

int HistoryListViewItem::compare ( QListViewItem * i, int col, bool ascending ) const
{
	if (col != HISTCOL_TIMESTAMP) {
		return QListViewItem::compare(i, col, ascending);
	}
	if (call_record.time_start < ((HistoryListViewItem *)i)->get_time_start()) {
		return -1;
	}
	if (call_record.time_start == ((HistoryListViewItem *)i)->get_time_start()) {
		return 0;
	}
	return 1;
}

time_t HistoryListViewItem::get_time_start(void) const
{
	return call_record.time_start;
}

t_call_record HistoryListViewItem::get_call_record(void) const
{
	return call_record;
}
