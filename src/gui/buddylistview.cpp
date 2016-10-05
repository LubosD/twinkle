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
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "buddylistview.h"

#include "gui.h"

#include "qapplication.h"
#include "qfont.h"
#include "qpixmap.h"
#include "qrect.h"
#include "qsize.h"
#include <QTextDocument>

void AbstractBLVItem::set_icon(t_presence_state::t_basic_state state) {
	switch (state) {
	case t_presence_state::ST_BASIC_UNKNOWN:
        setData(0, Qt::DecorationRole, QPixmap(":/icons/images/presence_unknown.png"));
		break;
	case t_presence_state::ST_BASIC_CLOSED:
        setData(0, Qt::DecorationRole, QPixmap(":/icons/images/presence_offline.png"));
		break;
	case t_presence_state::ST_BASIC_OPEN:
        setData(0, Qt::DecorationRole, QPixmap(":/icons/images/presence_online.png"));
		break;
	case t_presence_state::ST_BASIC_FAILED:
        setData(0, Qt::DecorationRole, QPixmap(":/icons/images/presence_failed.png"));
		break;
	case t_presence_state::ST_BASIC_REJECTED:
        setData(0, Qt::DecorationRole, QPixmap(":/icons/images/presence_rejected.png"));
		break;
	default:
        setData(0, Qt::DecorationRole, QPixmap(":/icons/images/presence_unknown.png"));
		break;
	}
}

AbstractBLVItem::AbstractBLVItem(QTreeWidgetItem *parent, const QString &text) :
        QTreeWidgetItem(parent, QStringList(text))
{}
		
AbstractBLVItem::AbstractBLVItem(QTreeWidget *parent, const QString &text) :
        QTreeWidgetItem(parent, QStringList(text))
{}

AbstractBLVItem::~AbstractBLVItem() {}

QString AbstractBLVItem::get_tip(void) {
	return tip;
}


void BuddyListViewItem::set_icon(void) {
	t_user *user_config = buddy->get_user_profile();
	string url_str = ui->expand_destination(user_config, buddy->get_sip_address());
	QString address = QString::fromStdString(ui->format_sip_address(user_config, buddy->get_name(), t_url(url_str)));
	
	tip = "<html>";
#if QT_VERSION >= 0x050000
	tip += address.toHtmlEscaped().replace(' ', "&nbsp;");
#else
	tip += Qt::escape(address).replace(' ', "&nbsp;");
#endif
	
	if (!buddy->get_may_subscribe_presence()) {
        setData(0, Qt::DecorationRole, QPixmap(":/icons/images/buddy.png"));
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
            failure = QString::fromStdString(buddy->get_presence_state()->get_failure_msg());
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

BuddyListViewItem::BuddyListViewItem(QTreeWidgetItem *parent, t_buddy *_buddy) :
        AbstractBLVItem(parent, QString::fromStdString(_buddy->get_name())),
		buddy(_buddy)
{
	set_icon();
	buddy->attach(this);
	QObject::connect(this, SIGNAL(update_signal()), this, SLOT(update_slot()));
}

BuddyListViewItem::~BuddyListViewItem() {
	buddy->detach(this);
}

void BuddyListViewItem::update_slot(void) {
	// This method is called directly from the core, so lock the GUI
	ui->lock();
	set_icon();
	
	if (buddy->get_name().c_str() != text(0)) {
        setText(0, QString::fromStdString(buddy->get_name()));
        QTreeWidgetItem::treeWidget()->sortItems(0, Qt::AscendingOrder);
	}
	ui->unlock();
}

void BuddyListViewItem::update(void) {
	emit update_signal();
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
    QString profile_name = QString::fromStdString(presence_epa->get_user_profile()->get_profile_name());
	
	tip = "<html>";
#if QT_VERSION >= 0x050000
	tip += profile_name.toHtmlEscaped();
#else
    tip += Qt::escape(profile_name);
#endif
	tip += "<br>";
	tip += "<b>";
	tip += qApp->translate("BuddyList", "Availability");
	tip += ":&nbsp;</b>";
	
	switch (presence_epa->get_epa_state()) {
	case t_presence_epa::EPA_UNPUBLISHED:
		tip += qApp->translate("BuddyList", "not published");
        setData(0, Qt::DecorationRole, QPixmap(":/icons/images/penguin-small.png"));
		break;
	case t_presence_epa::EPA_FAILED:
		tip += qApp->translate("BuddyList", "failed to publish");
		failure = presence_epa->get_failure_msg().c_str();
		if (!failure.isEmpty()) {
				tip += QString(" (%1)").arg(failure);
			}
        setData(0, Qt::DecorationRole, QPixmap(":/icons/images/presence_failed.png"));
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

BLViewUserItem::BLViewUserItem(QTreeWidget *parent, t_presence_epa *_presence_epa) :
        AbstractBLVItem(parent, QString::fromStdString(_presence_epa->get_user_profile()->get_profile_name())),
		presence_epa(_presence_epa)
{
	set_icon();
	presence_epa->attach(this);
	QObject::connect(this, SIGNAL(update_signal()), this, SLOT(update_slot()));

    QFont font = this->font(0);
    font.setBold(true);
    this->setFont(0, font);
}

BLViewUserItem::~BLViewUserItem() {
	presence_epa->detach(this);
}

void BLViewUserItem::update_slot(void) {
	// This method is called directly from the core, so lock the GUI
	ui->lock();
	set_icon();
	
	if (presence_epa->get_user_profile()->get_profile_name().c_str() == text(0)) {
        setText(0, QString::fromStdString(presence_epa->get_user_profile()->get_profile_name()));
        QTreeWidgetItem::treeWidget()->sortItems(0, Qt::AscendingOrder);
	}
	ui->unlock();
}

void BLViewUserItem::update(void) {
	emit update_signal();
}

void BLViewUserItem::subject_destroyed(void) {
	delete this;
}

t_presence_epa *BLViewUserItem::get_presence_epa(void) {
	return presence_epa;
}

