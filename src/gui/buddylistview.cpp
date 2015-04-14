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

#include "buddylistview.h"

#include "gui.h"

#include "qapplication.h"
#include "qfont.h"
#include "q3header.h"
#include "qpixmap.h"
#include "qrect.h"
#include "qsize.h"
#include "q3stylesheet.h"
//Added by qt3to4:
#include <q3mimefactory.h>

void AbstractBLVItem::set_icon(t_presence_state::t_basic_state state) {
	switch (state) {
	case t_presence_state::ST_BASIC_UNKNOWN:
		setPixmap(0, qPixmapFromMimeSource("presence_unknown.png"));
		break;
	case t_presence_state::ST_BASIC_CLOSED:
		setPixmap(0, qPixmapFromMimeSource("presence_offline.png"));
		break;
	case t_presence_state::ST_BASIC_OPEN:
		setPixmap(0, qPixmapFromMimeSource("presence_online.png"));
		break;
	case t_presence_state::ST_BASIC_FAILED:
		setPixmap(0, qPixmapFromMimeSource("presence_failed.png"));
		break;
	case t_presence_state::ST_BASIC_REJECTED:
		setPixmap(0, qPixmapFromMimeSource("presence_rejected.png"));
		break;
	default:
		setPixmap(0, qPixmapFromMimeSource("presence_unknown.png"));
		break;
	}
}

AbstractBLVItem::AbstractBLVItem(Q3ListViewItem *parent, const QString &text) :
		Q3ListViewItem(parent, text)
{}
		
AbstractBLVItem::AbstractBLVItem(Q3ListView *parent, const QString &text) :
		Q3ListViewItem(parent, text)
{}

AbstractBLVItem::~AbstractBLVItem() {}

QString AbstractBLVItem::get_tip(void) {
	return tip;
}


void BuddyListViewItem::set_icon(void) {
	t_user *user_config = buddy->get_user_profile();
	string url_str = ui->expand_destination(user_config, buddy->get_sip_address());
	
	tip = "<html>";
	tip += Q3StyleSheet::escape(ui->format_sip_address(user_config, buddy->get_name(), t_url(url_str)).c_str()).replace(' ', "&nbsp;");
	
	if (!buddy->get_may_subscribe_presence()) {
		setPixmap(0, qPixmapFromMimeSource("buddy.png"));
	} else {
		QString failure;
		t_presence_state::t_basic_state basic_state = buddy->
					get_presence_state()->get_basic_state();
		AbstractBLVItem::set_icon(basic_state);
		
		tip += "<br>";
		tip += "<b>";
		tip += qApp->translate("BuddyList", "Availability");
		tip += ":&nbsp;</b>";
		
		switch (basic_state) {
		case t_presence_state::ST_BASIC_UNKNOWN:
			tip += qApp->translate("BuddyList", "unknown");
			break;
		case t_presence_state::ST_BASIC_CLOSED:
			tip += qApp->translate("BuddyList", "offline");
			break;
		case t_presence_state::ST_BASIC_OPEN:
			tip += qApp->translate("BuddyList", "online");
			break;
		case t_presence_state::ST_BASIC_FAILED:
			tip += qApp->translate("BuddyList", "request failed");
			failure = buddy->get_presence_state()->get_failure_msg().c_str();
			if (!failure.isEmpty()) {
				tip += QString(" (%1)").arg(failure);
			}
			break;
		case t_presence_state::ST_BASIC_REJECTED:
			tip += qApp->translate("BuddyList", "request rejected");
			break;
		default:
			tip += qApp->translate("BuddyList", "unknown");
			break;
		}
	}
	
	tip += "</html>";
	tip = tip.replace(' ', "&nbsp;");
}

BuddyListViewItem::BuddyListViewItem(Q3ListViewItem *parent, t_buddy *_buddy) :
		AbstractBLVItem(parent, _buddy->get_name().c_str()),
		buddy(_buddy)
{
	set_icon();
	buddy->attach(this);
}

BuddyListViewItem::~BuddyListViewItem() {
	buddy->detach(this);
}

void BuddyListViewItem::update(void) {
	// This method is called directly from the core, so lock the GUI
	ui->lock();
	set_icon();
	
	if (buddy->get_name().c_str() != text(0)) {
		setText(0, buddy->get_name().c_str());
		Q3ListViewItem::parent()->sort();
	}
	ui->unlock();
}

void BuddyListViewItem::subject_destroyed(void) {
	delete this;
}

t_buddy *BuddyListViewItem::get_buddy(void) {
	return buddy;
}


void BLViewUserItem::set_icon(void) {
	t_presence_state::t_basic_state basic_state;
	QString failure;
	QString profile_name = presence_epa->get_user_profile()->get_profile_name().c_str();
	
	tip = "<html>";
	tip += Q3StyleSheet::escape(profile_name);
	tip += "<br>";
	tip += "<b>";
	tip += qApp->translate("BuddyList", "Availability");
	tip += ":&nbsp;</b>";
	
	switch (presence_epa->get_epa_state()) {
	case t_presence_epa::EPA_UNPUBLISHED:
		tip += qApp->translate("BuddyList", "not published");
		setPixmap(0, qPixmapFromMimeSource("penguin-small.png"));
		break;
	case t_presence_epa::EPA_FAILED:
		tip += qApp->translate("BuddyList", "failed to publish");
		failure = presence_epa->get_failure_msg().c_str();
		if (!failure.isEmpty()) {
				tip += QString(" (%1)").arg(failure);
			}
		setPixmap(0, qPixmapFromMimeSource("presence_failed.png"));
		break;
	case t_presence_epa::EPA_PUBLISHED:
		basic_state = presence_epa->get_basic_state();
		AbstractBLVItem::set_icon(basic_state);
		
		switch (presence_epa->get_basic_state()) {
		case t_presence_state::ST_BASIC_CLOSED:
			tip += qApp->translate("BuddyList", "offline");
			break;
		case t_presence_state::ST_BASIC_OPEN:
			tip += qApp->translate("BuddyList", "online");
			break;
		default:
			tip += qApp->translate("BuddyList", "unknown");
			break;
		}
		break;
	default:
		tip += qApp->translate("BuddyList", "unknown");
		break;
	}
	
	tip += "<p><i>";
	tip += qApp->translate("BuddyList", "Click right to add a buddy.");
	tip += "<i></html>";
	tip = tip.replace(' ', "&nbsp;");
}

BLViewUserItem::BLViewUserItem(Q3ListView *parent, t_presence_epa *_presence_epa) :
		AbstractBLVItem(parent, _presence_epa->get_user_profile()->get_profile_name().c_str()),
		presence_epa(_presence_epa)
{
	set_icon();
	presence_epa->attach(this);
}

BLViewUserItem::~BLViewUserItem() {
	presence_epa->detach(this);
}

void BLViewUserItem::paintCell(QPainter *painter, const QColorGroup &cg, 
				    int column, int width, int align)
{
	painter->save();
	QFont font = painter->font();
	font.setBold(true);
	painter->setFont(font);
	Q3ListViewItem::paintCell(painter, cg, column, width, align);
	painter->restore();
}

void BLViewUserItem::update(void) {
	// This method is called directly from the core, so lock the GUI
	ui->lock();
	set_icon();
	
	if (presence_epa->get_user_profile()->get_profile_name().c_str() == text(0)) {
		setText(0, presence_epa->get_user_profile()->get_profile_name().c_str());
		Q3ListViewItem::listView()->sort();
	}
	ui->unlock();
}

void BLViewUserItem::subject_destroyed(void) {
	delete this;
}

t_presence_epa *BLViewUserItem::get_presence_epa(void) {
	return presence_epa;
}

