//Added by qt3to4:
#include <QCloseEvent>
#include <QPixmap>
#include <QMenu>

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

#include "call_history.h"
#include "util.h"
#include "gui.h"
#include "q3listview.h"
#include "qicon.h"
#include "audits/memman.h"
#include "historyform.h"

#define HISTCOL_TIMESTAMP 	0
#define HISTCOL_DIRECTION	1
#define HISTCOL_FROMTO		2
#define HISTCOL_SUBJECT	3
#define HISTCOL_STATUS		4

/*
 *  Constructs a HistoryForm as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
HistoryForm::HistoryForm(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
	: QDialog(parent, name, modal, fl)
{
	setupUi(this);

	init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
HistoryForm::~HistoryForm()
{
	destroy();
	// no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void HistoryForm::languageChange()
{
	retranslateUi(this);
}


void HistoryForm::init()
{

    m_model = new QStandardItemModel(historyListView);
    historyListView->setModel(m_model);
    m_model->setColumnCount(5);

    m_model->setHorizontalHeaderLabels(QStringList() << tr("Time") << tr("In/Out") << tr("From/To") << tr("Subject") << tr("Status"));
    historyListView->sortByColumn(HISTCOL_TIMESTAMP, Qt::DescendingOrder);

    historyListView->setColumnWidth(HISTCOL_FROMTO, 200);
    historyListView->setColumnWidth(HISTCOL_SUBJECT, 200);
	
	inCheckBox->setChecked(true);
	outCheckBox->setChecked(true);
	successCheckBox->setChecked(true);
	missedCheckBox->setChecked(true);
	profileCheckBox->setChecked(true);
	
	timeLastViewed = phone->get_startup_time();
	
	QIcon inviteIcon(QPixmap(":/icons/images/invite.png"));
	QIcon deleteIcon(QPixmap(":/icons/images/editdelete.png"));
    histPopupMenu = new QMenu(this);
	
	itemCall = histPopupMenu->insertItem(inviteIcon, tr("Call..."), this, SLOT(call()));
	histPopupMenu->insertItem(deleteIcon, tr("Delete"), this, SLOT(deleteEntry()));

    m_pixmapIn = QPixmap(":/icons/images/1leftarrow-yellow.png");
    m_pixmapOut = QPixmap(":/icons/images/1rightarrow.png");

    m_pixmapOk = QPixmap(":/icons/images/ok.png");
    m_pixmapCancel = QPixmap(":/icons/images/cancel.png");
}

void HistoryForm::destroy()
{
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

    std::list<t_call_record> history;

    call_history->get_history(history);
    m_history = QList<t_call_record>::fromStdList(history);

    for (int x = 0; x < m_history.size(); x++) {
        const t_call_record* cr = &m_history[x];

        if (cr->direction == t_call_record::DIR_IN && !inCheckBox->isChecked()) {
			continue;
		}
        if (cr->direction == t_call_record::DIR_OUT && !outCheckBox->isChecked()) {
			continue;
		}
        if (cr->invite_resp_code < 300 && !successCheckBox->isChecked()) {
			continue;
		}
        if (cr->invite_resp_code >= 300 && !missedCheckBox->isChecked()) {
			continue;
		}
        if (!profile_name_list.contains(cr->user_profile.c_str()) &&
		    profileCheckBox->isChecked())
		{
			continue;
		}
		
		numberOfCalls++;
		
		// Calculate total duration
        totalCallDuration += cr->time_end - cr->time_start;
        if (cr->time_answer != 0) {
            totalConversationDuration += cr->time_end - cr->time_answer;
		}
		
        t_user *user_config = phone->ref_user_profile(cr->user_profile);
		
		// If the user profile is not active, then use the
		// first user profile for formatting	
		if (!user_config) {
			user_config = phone->ref_users().front();
		}
		
        m_model->setRowCount(numberOfCalls);

        for (int j = 0; j < 5; j++)
        {
            QModelIndex index = m_model->index(m_model->rowCount()-1, j);

            m_model->setData(index, QVariant(x), Qt::UserRole);
            switch (j)
            {
                case HISTCOL_TIMESTAMP:
                {
                    QString time = QString::fromStdString(time2str(cr->time_start,  "%d %b %Y %H:%M:%S"));
                    m_model->setData(index, time);
                    break;
                }
                case HISTCOL_DIRECTION:
                {
                    m_model->setData(index, QString::fromStdString(cr->get_direction()));

                    m_model->setData(index, (cr->direction == t_call_record::DIR_IN ?
                                    m_pixmapIn : m_pixmapOut), Qt::DecorationRole);

                    break;
                }
                case HISTCOL_FROMTO:
                {
                    std::string address;

                    address = (cr->direction == t_call_record::DIR_IN ?
                         ui->format_sip_address(user_config,
                          cr->from_display, cr->from_uri) :
                         ui->format_sip_address(user_config,
                          cr->to_display, cr->to_uri));

                    m_model->setData(index, QString::fromStdString(address));

                    m_model->setData(index, (cr->invite_resp_code < 300 ?
                                    m_pixmapOk : m_pixmapCancel), Qt::DecorationRole);
                    break;
                }
                case HISTCOL_SUBJECT:
                {
                    m_model->setData(index, QString::fromStdString(cr->subject));
                    break;
                }
                case HISTCOL_STATUS:
                {
                    m_model->setData(index, QString::fromStdString(cr->invite_resp_reason));
                    break;
                }
            }
        }
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
    historyListView->selectRow(0);

    //	showCallDetails(first);
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

void HistoryForm::showCallDetails(QModelIndex index)
{
	QString s;
	
    int x = m_model->data(index, Qt::UserRole).toInt();
    const t_call_record& cr = m_history[x];
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

void HistoryForm::popupMenu(QPoint pos)
{
    if (!historyListView->selectionModel()->hasSelection())
        return;

    QModelIndex index = historyListView->selectionModel()->currentIndex();
    int x = m_model->data(index, Qt::UserRole).toInt();
	
    const t_call_record& cr = m_history[x];
	
	// An anonymous caller cannot be called
	bool canCall = !(cr.direction == t_call_record::DIR_IN &&
			    cr.from_uri.encode() == ANONYMOUS_URI);
	
	histPopupMenu->setItemEnabled(itemCall, canCall);
	histPopupMenu->popup(pos);
}

void HistoryForm::call(QModelIndex index)
{
    int i = m_model->data(index, Qt::UserRole).toInt();
    const t_call_record& cr = m_history[i];
	
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
        emit call(user_config, m_model->data(m_model->index(index.row(), HISTCOL_FROMTO)).toString(), subject, hide_user);
	}
}

void HistoryForm::call(void)
{
    QModelIndex index = historyListView->selectionModel()->currentIndex();
    if (index.isValid()) call(index);
}

void HistoryForm::deleteEntry(void)
{
    QModelIndex index = historyListView->selectionModel()->currentIndex();
    int i = m_model->data(index, Qt::UserRole).toInt();
    m_model->removeRow(index.row());
	
    call_history->delete_call_record(m_history[i].get_id());
}

void HistoryForm::clearHistory()
{
	call_history->clear();
    m_model->setRowCount(0);
}
