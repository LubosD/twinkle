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

void HistoryForm::init()
{
	historyListView->setSorting(HISTCOL_TIMESTAMP, false);
	historyListView->setColumnWidthMode(HISTCOL_FROMTO, QListView::Manual);
	historyListView->setColumnWidth(HISTCOL_FROMTO, 200);
	historyListView->setColumnWidthMode(HISTCOL_SUBJECT, QListView::Manual);
	historyListView->setColumnWidth(HISTCOL_SUBJECT, 200);
	
	inCheckBox->setChecked(true);
	outCheckBox->setChecked(true);
	successCheckBox->setChecked(true);
	missedCheckBox->setChecked(true);
	profileCheckBox->setChecked(true);
	
	timeLastViewed = phone->get_startup_time();
	
	QIconSet inviteIcon(QPixmap::fromMimeSource("invite.png"));
	QIconSet deleteIcon(QPixmap::fromMimeSource("editdelete.png"));
	histPopupMenu = new QPopupMenu(this);
	MEMMAN_NEW(histPopupMenu);
	
	itemCall = histPopupMenu->insertItem(inviteIcon, tr("Call..."), this, SLOT(call()));
	histPopupMenu->insertItem(deleteIcon, tr("Delete"), this, SLOT(deleteEntry()));
}

void HistoryForm::destroy()
{
	MEMMAN_DELETE(histPopupMenu);
	delete histPopupMenu;
}

void HistoryForm::loadHistory()
{
	// Create list of all active profile names
	QStringList profile_name_list;
	list<t_user *>user_list = phone->ref_users();
	for (list<t_user *>::iterator i = user_list.begin(); i != user_list.end(); i++) {
		profile_name_list.append((*i)->get_profile_name().c_str());
	}
	
	// Fill the history table
	unsigned long numberOfCalls = 0;
	unsigned long totalCallDuration = 0;
	unsigned long totalConversationDuration = 0;
	historyListView->clear();
	list<t_call_record> history;
	call_history->get_history(history);
	for (list<t_call_record>::iterator i = history.begin(); i != history.end(); i++) {
		if (i->direction == t_call_record::DIR_IN && !inCheckBox->isChecked()) {
			continue;
		}
		if (i->direction == t_call_record::DIR_OUT && !outCheckBox->isChecked()) {
			continue;
		}
		if (i->invite_resp_code < 300 && !successCheckBox->isChecked()) {
			continue;
		}
		if (i->invite_resp_code >= 300 && !missedCheckBox->isChecked()) {
			continue;
		}
		if (!profile_name_list.contains(i->user_profile.c_str()) &&
		    profileCheckBox->isChecked())
		{
			continue;
		}
		
		numberOfCalls++;
		
		// Calculate total duration
		totalCallDuration += i->time_end - i->time_start;
		if (i->time_answer != 0) {
			totalConversationDuration += i->time_end - i->time_answer;
		}
		
		t_user *user_config = phone->ref_user_profile(i->user_profile);
		
		// If the user profile is not active, then use the
		// first user profile for formatting	
		if (!user_config) {
			user_config = phone->ref_users().front();
		}
		
		new HistoryListViewItem(historyListView,
			*i, user_config, timeLastViewed);
	}
	
	numberCallsValueTextLabel->setText(QString().setNum(numberOfCalls));
	
	// Total call duration formatting
	QString durationText = duration2str(totalCallDuration).c_str();
	durationText += " (";
	durationText += tr("conversation");
	durationText += ": ";
	durationText += duration2str(totalConversationDuration).c_str();
	durationText += ")";
	totalDurationValueTextLabel->setText(durationText);
	
	// Make the first entry the selected entry.
	QListViewItem *first = historyListView->firstChild();
	if (first) {
		historyListView->setSelected(first, true);
		showCallDetails(first);
	} else {
		cdrTextEdit->clear();
	}
}

// Update history when triggered by a call back function on the user
// interface.
void HistoryForm::update()
{
	// There is no need to update the history when the window is
	// hidden.
	if (isShown()) loadHistory();
}

void HistoryForm::show()
{
	if (isShown()) {
		raise();
		setActiveWindow();
		return;
	}
	
	loadHistory();
	QDialog::show();
	raise();
}

void HistoryForm::closeEvent( QCloseEvent *e )
{
	struct timeval t;
	
	gettimeofday(&t, NULL);
	timeLastViewed = t.tv_sec;
	
	// If Twinkle is terminated while the history window is
	// shown, then the call_history object is destroyed, before this
	// window is closed.
	if (call_history) {
		call_history->clear_num_missed_calls();
	}
	QDialog::closeEvent(e);
}

void HistoryForm::showCallDetails(QListViewItem *item)
{
	QString s;
	
	t_call_record cr = ((HistoryListViewItem *)item)->get_call_record();
	cdrTextEdit->clear();
	
	t_user *user_config = phone->ref_user_profile(cr.user_profile);
	// If the user profile is not active, then use the
	// first user profile for formatting	
	if (!user_config) {
		user_config = phone->ref_users().front();
	}
	
	s = "<table>";
	
	// Left column: header names
	s += "<tr><td><b>";
	s += tr("Call start:") + "<br>";
	s += tr("Call answer:") + "<br>";
	s += tr("Call end:") + "<br>";
	s += tr("Call duration:") + "<br>";
	s += tr("Direction:") + "<br>";
	s += tr("From:") + "<br>";
	s += tr("To:") + "<br>";
	if (cr.reply_to_uri.is_valid()) s += tr("Reply to:") + "<br>";
	if (cr.referred_by_uri.is_valid()) s += tr("Referred by:") + "<br>";
	s += tr("Subject:") + "<br>";
	s += tr("Released by:") + "<br>";
	s += tr("Status:") + "<br>";
	if (!cr.far_end_device.empty()) s += tr("Far end device:") + "<br>";
	s += tr("User profile:");
	s += "</b></td>";
	
	// Right column: values
	s += "<td>";
	s += time2str(cr.time_start, "%d %b %Y %H:%M:%S").c_str();
	s += "<br>";
	if (cr.time_answer != 0) {
		s += time2str(cr.time_answer,  "%d %b %Y %H:%M:%S").c_str();
	}
	s += "<br>";
	s += time2str(cr.time_end, "%d %b %Y %H:%M:%S").c_str();
	s += "<br>";
	
	s += duration2str((unsigned long)(cr.time_end - cr.time_start)).c_str();
	if (cr.time_answer != 0) {
		s += " (";
		s += tr("conversation");
		s += ": ";
		s += duration2str((unsigned long)(cr.time_end - cr.time_answer)).c_str();
		s += ")";
	}
	s += "<br>";
	
	s += cr.get_direction().c_str();
	s += "<br>";
	s += str2html(ui->format_sip_address(user_config, cr.from_display, cr.from_uri).c_str());
	if (cr.from_organization != "") {
		s += ", ";
		s += str2html(cr.from_organization.c_str());
	}
	s += "<br>";
	s +=  str2html(ui->format_sip_address(user_config, cr.to_display, cr.to_uri).c_str());
	if (cr.to_organization != "") {
		s += ", ";
		s +=  str2html(cr.to_organization.c_str());
	}
	s += "<br>";
	if (cr.reply_to_uri.is_valid()) {
		s +=  str2html(ui->format_sip_address(user_config,
					cr.reply_to_display, cr.reply_to_uri).c_str());
		s += "<br>";
	}
	if (cr.referred_by_uri.is_valid()) {
		s +=  str2html(ui->format_sip_address(user_config,
				cr.referred_by_display, cr.referred_by_uri).c_str());
		s += "<br>";
	}
	s +=  str2html(cr.subject.c_str());
	s += "<br>";
	s += cr.get_rel_cause().c_str();
	s += "<br>";
	s += int2str(cr.invite_resp_code).c_str();
	s += ' ';
	s +=  str2html(cr.invite_resp_reason.c_str());
	s += "<br>";
	if (!cr.far_end_device.empty()) {
		s += str2html(cr.far_end_device.c_str());
		s += "<br>";
	}
	s +=  str2html(cr.user_profile.c_str());
	s += "</td></tr>";
	
	s += "</table>";
	
	cdrTextEdit->setText(s);
}

void HistoryForm::popupMenu(QListViewItem *item, const QPoint &pos)
{
	if (!item) return;
	
	HistoryListViewItem *histItem = dynamic_cast<HistoryListViewItem *>(item);
	if (!histItem) return;
	
	t_call_record cr = histItem->get_call_record();
	
	// An anonymous caller cannot be called
	bool canCall = !(cr.direction == t_call_record::DIR_IN &&
			    cr.from_uri.encode() == ANONYMOUS_URI);
	
	histPopupMenu->setItemEnabled(itemCall, canCall);
	histPopupMenu->popup(pos);
}

void HistoryForm::call(QListViewItem *item)
{
	if (!item) return;
	
	HistoryListViewItem *histItem = (HistoryListViewItem *)item;
	t_call_record cr = histItem->get_call_record();
	
	t_user *user_config = phone->ref_user_profile(cr.user_profile);
	// If the user profile is not active, then use the first profile
	if (!user_config) {
		user_config = phone->ref_users().front();
	}
	
	// Determine subject
	QString subject;
	if (cr.direction == t_call_record::DIR_IN) {
		if (!cr.subject.empty()) {
			if (cr.subject.substr(0, tr("Re:").length()) != tr("Re:").ascii()) {
				subject = tr("Re:").append(" ");
				subject += cr.subject.c_str();
			} else {
				subject = cr.subject.c_str();
			}
		}
	} else {
		subject = cr.subject.c_str();
	}
	
	// Send call signal
	if (cr.direction == t_call_record::DIR_IN && cr.reply_to_uri.is_valid()) {
		// Call to the Reply-To contact
		emit call(user_config,
			ui->format_sip_address(user_config, 
				cr.reply_to_display, cr.reply_to_uri).c_str(), 
			subject, false);
	} else {
		// For incoming calls, call to the From contact
		// For outgoing calls, call to the To contact
		bool hide_user = false;
		if (cr.direction == t_call_record::DIR_OUT && 
		    cr.from_uri.encode() == ANONYMOUS_URI)
		{
			hide_user = true;
		}
		emit call(user_config, item->text(HISTCOL_FROMTO), subject, hide_user);
	}
}

void HistoryForm::call(void)
{
	QListViewItem *item = historyListView->currentItem();
	if (item) call(item);
}

void HistoryForm::deleteEntry(void)
{
	QListViewItem *item = historyListView->currentItem();
	HistoryListViewItem *histItem = dynamic_cast<HistoryListViewItem *>(item);
	if (!histItem) return;
	
	call_history->delete_call_record(histItem->get_call_record().get_id());
}

void HistoryForm::clearHistory()
{
	call_history->clear();
}
