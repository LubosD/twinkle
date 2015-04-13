/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/

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

void SelectUserForm::init()
{
	// Disable sorting
	userListView->setSorting(-1);
}

void SelectUserForm::show(t_select_purpose purpose)
{
	QString title, msg_purpose;
	
	// Set dialog caption and purpose
	title = PRODUCT_NAME;
	title += " - ";
	switch (purpose) {
	case SELECT_REGISTER:
		title.append(tr("Register"));
		msg_purpose = tr("Select users that you want to register.");
		break;
	case SELECT_DEREGISTER:
		title.append(tr("Deregister"));
		msg_purpose = tr("Select users that you want to deregister.");
		break;
	case SELECT_DEREGISTER_ALL:
		title.append(tr("Deregister all devices"));
		msg_purpose = tr("Select users for which you want to deregister all devices.");
		break;
	case SELECT_DND:
		title.append(tr("Do not disturb"));
		msg_purpose = tr("Select users for which you want to enable 'do not disturb'.");
		break;
	case SELECT_AUTO_ANSWER:
		title.append(tr("Auto answer"));
		msg_purpose = tr("Select users for which you want to enable 'auto answer'.");
		break;
	default:
		assert(false);
	}
	setCaption(title);
	purposeTextLabel->setText(msg_purpose);
	
	// Fill list view
	list<t_user *> user_list = phone->ref_users();
	for (list<t_user *>::reverse_iterator i = user_list.rbegin(); i != user_list.rend(); i++) {
		QCheckListItem *item = new QCheckListItem(userListView,
			(*i)->get_profile_name().c_str(), QCheckListItem::CheckBox);
		
		switch (purpose) {
		case SELECT_DND:
			item->setOn(phone->ref_service(*i)->is_dnd_active());
			break;
		case SELECT_AUTO_ANSWER:
			item->setOn(phone->ref_service(*i)->is_auto_answer_active());
			break;
		default:
			break;
		}
	}
	
	QDialog::show();
}

void SelectUserForm::validate()
{
	list<t_user *> selected_list, not_selected_list;
	
	QListViewItemIterator i(userListView);
	while (i.current()) {
		QCheckListItem *item = (QCheckListItem *)(i.current());
		if (item->isOn()) {
			selected_list.push_back(phone->
				ref_user_profile(item->text().ascii()));
		} else {
			not_selected_list.push_back(phone->
				ref_user_profile(item->text().ascii()));
		}
		i++;
	}
	
	emit (selection(selected_list));
	emit (not_selected(not_selected_list));
	accept();
}

void SelectUserForm::selectAll()
{
	QListViewItemIterator i(userListView);
	while (i.current()) {
		QCheckListItem *item = (QCheckListItem *)(i.current());
		item->setOn(true);
		i++;
	}
}

void SelectUserForm::clearAll()
{
	QListViewItemIterator i(userListView);
	while (i.current()) {
		QCheckListItem *item = (QCheckListItem *)(i.current());
		item->setOn(false);
		i++;
	}
}

void SelectUserForm::toggle(QListViewItem *item) 
{
	QCheckListItem *checkItem = (QCheckListItem *)item;
	checkItem->setOn(!checkItem->isOn());
}
