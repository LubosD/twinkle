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

#include <QVariant>
#include <cassert>
#include <QImage>
#include <QPixmap>

#include <QListWidget>
#include "selectuserform.h"
/*
 *  Constructs a SelectUserForm as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
SelectUserForm::SelectUserForm(QWidget* parent)
    : QDialog(parent)
{
	setupUi(this);

	init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
SelectUserForm::~SelectUserForm()
{
	// no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void SelectUserForm::languageChange()
{
	retranslateUi(this);
}


void SelectUserForm::init()
{
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
    setWindowTitle(title);
	purposeTextLabel->setText(msg_purpose);
	
	// Fill list view
	list<t_user *> user_list = phone->ref_users();
	for (list<t_user *>::reverse_iterator i = user_list.rbegin(); i != user_list.rend(); i++) {
        QListWidgetItem* item = new QListWidgetItem(QString::fromStdString((*i)->get_profile_name()), userListView);

        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
		
		switch (purpose) {
		case SELECT_DND:
            if (phone->ref_service(*i)->is_dnd_active())
                item->setCheckState(Qt::Checked);
			break;
		case SELECT_AUTO_ANSWER:
            if (phone->ref_service(*i)->is_auto_answer_active())
                item->setCheckState(Qt::Checked);
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
	
    for (int i = 0; i < userListView->count(); i++)
    {
        QListWidgetItem *item = userListView->item(i);
        if (item->checkState() == Qt::Checked) {
			selected_list.push_back(phone->
                ref_user_profile(item->text().toStdString()));
		} else {
			not_selected_list.push_back(phone->
                ref_user_profile(item->text().toStdString()));
		}
		i++;
	}
	
	emit (selection(selected_list));
	emit (not_selected(not_selected_list));
	accept();
}

void SelectUserForm::selectAll()
{
    for (int i = 0; i < userListView->count(); i++)
    {
        QListWidgetItem *item = userListView->item(i);
        item->setCheckState(Qt::Checked);
    }
}

void SelectUserForm::clearAll()
{
    for (int i = 0; i < userListView->count(); i++)
    {
        QListWidgetItem *item = userListView->item(i);
        item->setCheckState(Qt::Unchecked);
    }
}

void SelectUserForm::toggle(QModelIndex index)
{
    QListWidgetItem *item = userListView->item(index.row());
    Qt::CheckState state = item->checkState();
    item->setCheckState((state == Qt::Checked) ? Qt::Unchecked : Qt::Checked);
}
