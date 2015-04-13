/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, pres:erving your code. Ceate an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
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

#include "twinkle_config.h"
#include "twinklesystray.h"

// Time (s) that the conversation timer of a line should stay visible after
// a call has ended
#define HIDE_LINE_TIMER_AFTER	5

void MphoneForm::init()
{
	// Forms
	dtmfForm = 0;
	inviteForm = 0;
	redirectForm = 0;
	transferForm = 0;
	termCapForm = 0;
	srvRedirectForm = 0;
	userProfileForm = 0;
	sysSettingsForm = 0;
	logViewForm = 0;
	historyForm = 0;
	selectUserForm = 0;
	selectProfileForm = 0;
	getAddressForm = 0;
	sysTray = 0;
	
	// Popup menu for a single buddy
	QIconSet inviteIcon(QPixmap::fromMimeSource("invite.png"));
	QIconSet messageIcon(QPixmap::fromMimeSource("message.png"));
	QIconSet editIcon(QPixmap::fromMimeSource("edit16.png"));
	QIconSet deleteIcon(QPixmap::fromMimeSource("editdelete.png"));
	buddyPopupMenu = new QPopupMenu(this);
	MEMMAN_NEW(buddyPopupMenu);
	buddyPopupMenu->insertItem(inviteIcon, tr("&Call..."), this, SLOT(doCallBuddy()));
	buddyPopupMenu->insertItem(messageIcon, tr("Instant &message..."), this, SLOT(doMessageBuddy()));
	buddyPopupMenu->insertItem(editIcon, tr("&Edit..."), this, SLOT(doEditBuddy()));
	buddyPopupMenu->insertItem(deleteIcon, tr("&Delete"), this, SLOT(doDeleteBuddy()));
	
	// Change availibility sub popup menu
	changeAvailabilityPopupMenu = new QPopupMenu(this);
	MEMMAN_NEW(changeAvailabilityPopupMenu);
	QIconSet availOnlineIcon(QPixmap::fromMimeSource("presence_online.png"));
	QIconSet availOfflineIcon(QPixmap::fromMimeSource("presence_offline.png"));
	changeAvailabilityPopupMenu->insertItem(availOfflineIcon, tr("O&ffline"), this, 
						SLOT(doAvailabilityOffline()));
	changeAvailabilityPopupMenu->insertItem(availOnlineIcon, tr("&Online"), this, 
						SLOT(doAvailabilityOnline()));
	
	// Popup menu for a buddy list (click on profile name)
	QIconSet changeAvailabilityIcon(QPixmap::fromMimeSource("presence_online.png"));
	QIconSet addIcon(QPixmap::fromMimeSource("buddy.png"));
	buddyListPopupMenu = new QPopupMenu(this);
	MEMMAN_NEW(buddyListPopupMenu);
	buddyListPopupMenu->insertItem(changeAvailabilityIcon, tr("&Change availability"), 
				       changeAvailabilityPopupMenu);
	buddyListPopupMenu->insertItem(addIcon, tr("&Add buddy..."), this, SLOT(doAddBuddy()));
	
	// Tool tip for buddy list
	buddyToolTip = new BuddyListViewTip(buddyListView);
	MEMMAN_NEW(buddyToolTip);
	
	// Line timers
	lineTimer1 = 0;
	lineTimer2 = 0;
	timer1TextLabel->hide();
	timer2TextLabel->hide();
	
	// Timer to hide the conversation timer after a conversation has ended.
	hideLineTimer1 = new QTimer(this);
	MEMMAN_NEW(hideLineTimer1);
	hideLineTimer2 = new QTimer(this);
	MEMMAN_NEW(hideLineTimer2);
	connect(hideLineTimer1, SIGNAL(timeout()), timer1TextLabel, SLOT(hide()));
	connect(hideLineTimer2, SIGNAL(timeout()), timer2TextLabel, SLOT(hide()));
	
	// Attach the MWI flash slot to the MWI flash timer
	connect(&tmrFlashMWI, SIGNAL(timeout()), this, SLOT(flashMWI()));
	
	// Set toolbar icons for disabled options.
	setDisabledIcon(callInvite, "invite-disabled.png");
	setDisabledIcon(callAnswer, "answer-disabled.png");
	setDisabledIcon(callBye, "bye-disabled.png");
	setDisabledIcon(callReject, "reject-disabled.png");
	setDisabledIcon(callRedirect, "redirect-disabled.png");
	setDisabledIcon(callTransfer, "transfer-disabled.png");
	setDisabledIcon(callHold, "hold-disabled.png");
	setDisabledIcon(callConference, "conf-disabled.png");
	setDisabledIcon(callMute, "mute-disabled.png");
	setDisabledIcon(callDTMF, "dtmf-disabled.png");
	setDisabledIcon(callRedial, "redial-disabled.png");
	
	// Set tool button icons for disabled options
	setDisabledIcon(addressToolButton, "kontact_contacts-disabled.png");
	
	// Some text labels on the main window are implemented as QLineEdit
	// objects as these do not automatically resize when a text set with setText
	// does not fit. The background of a QLineEdit is static however, it does not
	// automatically take a background color passed by the -bg parameter.
	// Set the background color of these QLineEdit objects here.
	from1Label->setPaletteBackgroundColor(paletteBackgroundColor());
	to1Label->setPaletteBackgroundColor(paletteBackgroundColor());
	subject1Label->setPaletteBackgroundColor(paletteBackgroundColor());
	from2Label->setPaletteBackgroundColor(paletteBackgroundColor());
	to2Label->setPaletteBackgroundColor(paletteBackgroundColor());
	subject2Label->setPaletteBackgroundColor(paletteBackgroundColor());
	
	// A QComboBox accepts a new line through copy/paste.
	QRegExp rxNoNewLine("[^\\n\\r]*");
	callComboBox->setValidator(new QRegExpValidator(rxNoNewLine, this));
	
	if (sys_config->get_gui_use_systray()) {
		// Create system tray icon
		sysTray = new t_twinkle_sys_tray(this, "twinkle_sys_tray");
		MEMMAN_NEW(sysTray);
		sysTray->setPixmap(
				QPixmap::fromMimeSource("sys_idle_dis.png"));
		sysTray->setCaption(PRODUCT_NAME);
		QToolTip::add(sysTray, PRODUCT_NAME);
		
		// Add items to the system tray menu
#ifdef HAVE_KDE
		KPopupMenu *menu;
#else
		QPopupMenu *menu;
#endif
		menu = sysTray->contextMenu();
		
		// Call menu
		callInvite->addTo(menu);
		callAnswer->addTo(menu);
		callBye->addTo(menu);
		callReject->addTo(menu);
		callRedirect->addTo(menu);
		callTransfer->addTo(menu);
		callHold->addTo(menu);
		callConference->addTo(menu);
		callMute->addTo(menu);
		callDTMF->addTo(menu);
		callRedial->addTo(menu);
		
		menu->insertSeparator();
		
		// Messaging
		actionSendMsg->addTo(menu);
		
		menu->insertSeparator();
		
		// Line activation
		actgrActivateLine->addTo(menu);
		
		menu->insertSeparator();
		
		// Service menu
		serviceDnd->addTo(menu);
		serviceRedirection->addTo(menu);
		serviceAutoAnswer->addTo(menu);
		servicesVoice_mailAction->addTo(menu);
		
		menu->insertSeparator();
		
		// View menu
		viewCall_HistoryAction->addTo(menu);
		
		menu->insertSeparator();
		
		// Diamondcard menu
		menu->insertItem("Diamondcard", Diamondcard);
		
		// Exit application when user selects Quit from the tray menu
		connect(sysTray, SIGNAL(quitSelected()),
			this, SLOT(fileExit()));
		
		sysTray->dock();
		sysTray->show();
	}
}

void MphoneForm::destroy()
{
	if (dtmfForm) {
		MEMMAN_DELETE(dtmfForm);
		delete dtmfForm;
	}
	if (inviteForm) {
		MEMMAN_DELETE(inviteForm);
		delete inviteForm;
	}
	if (redirectForm) {
		MEMMAN_DELETE(redirectForm);
		delete redirectForm;
	}
	if (termCapForm) {
		MEMMAN_DELETE(termCapForm);
		delete termCapForm;
	}
	if (srvRedirectForm) {
		MEMMAN_DELETE(srvRedirectForm);
		delete srvRedirectForm;
	}
	if (userProfileForm) {
		MEMMAN_DELETE(userProfileForm);
		delete userProfileForm;
	}
	if (transferForm) {
		MEMMAN_DELETE(transferForm);
		delete transferForm;
	}
	if (sysSettingsForm) {
		MEMMAN_DELETE(sysSettingsForm);
		delete sysSettingsForm;
	}
	if (logViewForm) {
		if (logViewForm->isShown()) logViewForm->close();
		MEMMAN_DELETE(logViewForm);
		delete logViewForm;
	}
	if (historyForm) {
		if (historyForm->isShown()) historyForm->close();
		MEMMAN_DELETE(historyForm);
		delete historyForm;
	}
	if (selectUserForm) {
		MEMMAN_DELETE(selectUserForm);
		delete selectUserForm;
	}
	if (selectProfileForm) {
		MEMMAN_DELETE(selectProfileForm);
		delete selectProfileForm;
	}
	if (getAddressForm) {
		MEMMAN_DELETE(getAddressForm);
		delete getAddressForm;
	}
	if (sysTray) {
		MEMMAN_DELETE(sysTray);
		delete sysTray;
	}
	
	if (lineTimer1) {
		MEMMAN_DELETE(lineTimer1);
		delete lineTimer1;
	}
	if (lineTimer2) {
		MEMMAN_DELETE(lineTimer2);
		delete lineTimer2;
	}
	MEMMAN_DELETE(hideLineTimer1);
	delete hideLineTimer1;
	MEMMAN_DELETE(hideLineTimer2);
	delete hideLineTimer2;
	MEMMAN_DELETE(buddyPopupMenu);
	delete buddyPopupMenu;
	MEMMAN_DELETE(changeAvailabilityPopupMenu);
	delete changeAvailabilityPopupMenu;
	MEMMAN_DELETE(buddyListPopupMenu);
	delete buddyListPopupMenu;
	MEMMAN_DELETE(buddyToolTip);
	delete buddyToolTip;
}

QString MphoneForm::lineSubstate2str( int line) {
	QString reason;
	
	t_call_info call_info = phone->get_call_info(line);
	
	switch(phone->get_line_substate(line)) {
	case LSSUB_IDLE:	
		return tr("idle");
	case LSSUB_SEIZED:
		return tr("dialing");
	case LSSUB_OUTGOING_PROGRESS:
		reason = call_info.last_provisional_reason.c_str();
		if (reason == "") {
			return tr("attempting call, please wait");
		}
		return reason;
	case LSSUB_INCOMING_PROGRESS:
		return QString("<font color=red>") + tr("incoming call") + "</font>";
	case LSSUB_ANSWERING:
		return tr("establishing call, please wait");
	case LSSUB_ESTABLISHED:
		if (phone->has_line_media(line)) {
			return tr("established");
		} else {
			return tr("established (waiting for media)");
		}
		break;
	case LSSUB_RELEASING:
		return tr("releasing call, please wait");
	default:
		return tr("unknown state");
	}
}

void MphoneForm::closeEvent( QCloseEvent *e )
{
	if (sysTray && sys_config->get_gui_hide_on_close()) {
		hide();
	} else {
		fileExit();
	}
}

void MphoneForm::fileExit()
{
	hide();
	QApplication::exit(0);
}

// Append a string to the display window
void MphoneForm::display( const QString &s )
{
	displayContents.push_back(s);
	if (displayContents.size() > 100) {
		displayContents.pop_front();
	}
	
	displayTextEdit->setText(displayContents.join("\n"));
	
	// Set cursor position at the end of text
	displayTextEdit->setCursorPosition(displayTextEdit->paragraphs() - 1, 0);
}

// Print message header on display
void MphoneForm::displayHeader()
{
	display("");
	display(current_time2str("%a %H:%M:%S").c_str());
}

// Update the conversation timer
void MphoneForm::showLineTimer(int line)
{
	struct timeval t;
	gettimeofday(&t, NULL);
	
	QLabel *timerLabel;
	
	if (line == 0) {
		timerLabel = timer1TextLabel;
	} else {
		timerLabel = timer2TextLabel;
	}
	
	// Calculate duration of call
	t_call_record cr = phone->get_call_hist(line);
	unsigned long duration = t.tv_sec - cr.time_answer;

	timerLabel->setText(timer2str(duration).c_str());
}

void MphoneForm::showLineTimer1()
{
	showLineTimer(0);
}

void MphoneForm::showLineTimer2()
{
	showLineTimer(1);
}

// Update visibility of the conversation timer for a line
// Initialize the timer for a new established call.
void MphoneForm::updateLineTimer(int line)
{
	QLabel *timerLabel;
	QTimer **timer;
	QTimer *hideLineTimer;
	
	if (line == 0) {
		timerLabel = timer1TextLabel;
		timer = &lineTimer1;
		hideLineTimer = hideLineTimer1;
	} else {
		timerLabel = timer2TextLabel;
		timer = &lineTimer2;
		hideLineTimer = hideLineTimer2;
	}
	
	t_line_substate line_substate = phone->get_line_substate(line);
	
	// Stop hide timer if necessary
	switch(line_substate) {
	case LSSUB_IDLE:
	case LSSUB_RELEASING:
		// Timer can be shown as long as line is idle or releasing.
		break;
	default:
		// The timer showing the call duration should only stay
		// for a few seconds as long as the line is idle or being
		// released.
		// If a new call arrives on the line, the hide timer should
		// be stopped, otherwise the timer of the new call will
		// automatically disappear.
		if (hideLineTimer->isActive()) {
			hideLineTimer->stop();
			if (*timer == NULL) timerLabel->hide();
		}
		break;
	}
	
	switch(line_substate) {
	case LSSUB_ESTABLISHED:
		// Initialize and show call duration timer
		if (*timer == NULL) {
			timerLabel->setText(timer2str(0).c_str());
			timerLabel->show();
			*timer = new QTimer(this);
			MEMMAN_NEW(*timer);
			
			if (line == 0) {
				connect(*timer, SIGNAL(timeout()), this, 
					SLOT(showLineTimer1()));
			} else {
				connect(*timer, SIGNAL(timeout()), this, 
					SLOT(showLineTimer2()));
			}
			
			// Update timer every 1s
			(*timer)->start(1000, false);
		}
		break;
	default:
		// Hide call duration timer
		if (*timer != NULL) {
			// Hide the timer after a few seconds
			hideLineTimer->start(HIDE_LINE_TIMER_AFTER * 1000, true);
			(*timer)->stop();
			MEMMAN_DELETE(*timer);
			*timer = NULL;
		}
		break;
	}
}

void MphoneForm::updateLineEncryptionState(int line)
{
	QLabel *cryptLabel, *sasLabel;
	if (line == 0) {
		cryptLabel = crypt1Label;
		sasLabel = line1SasLabel;
	} else {
		cryptLabel = crypt2Label;
		sasLabel = line2SasLabel;
	}
	
	t_audio_session *as = phone->get_line(line)->get_audio_session();
	if (as && phone->is_line_encrypted(line)) {
		string zrtp_sas = as->get_zrtp_sas();
		bool zrtp_sas_confirmed = as->get_zrtp_sas_confirmed();
		string srtp_cipher_mode = as->get_srtp_cipher_mode();
		
		QToolTip::remove(cryptLabel);
		QString toolTip = tr("Voice is encrypted") + " (";
		toolTip.append(srtp_cipher_mode.c_str()).append(")");
		
		if (!zrtp_sas.empty()) {
			// Set tool tip on encryption icon
			toolTip.append("\nSAS = ");
			toolTip.append(zrtp_sas.c_str());
			
			// Show SAS
			sasLabel->setText(zrtp_sas.c_str());
			sasLabel->show();
		} else {
			sasLabel->hide();
		}
			
		if (!zrtp_sas_confirmed) {
			toolTip.append("\n").append(tr("Click to confirm SAS."));
			cryptLabel->setFrameStyle(QFrame::Panel | QFrame::Raised);
			cryptLabel->setPixmap(
				QPixmap::fromMimeSource("encrypted.png"));
		} else {
			toolTip.append("\n").append(tr("Click to clear SAS verification."));
			cryptLabel->setFrameStyle(QFrame::NoFrame);
			cryptLabel->setPixmap(
				QPixmap::fromMimeSource("encrypted_verified.png"));
		}

		QToolTip::add(cryptLabel, toolTip);
		cryptLabel->show();
	} else {
		cryptLabel->hide();
		sasLabel->hide();
	}
}

void MphoneForm::updateLineStatus(int line)
{
	QString state;
	bool on_hold; // indicates if a line is put on-hold
	bool in_conference; // indicates if a line is in a conference
	bool is_muted; // indicates is a line is muted
	t_refer_state refer_state; // indicates if a call transfer is in progress
	bool is_transfer_consult; // indicates if the call is a consultation
	bool to_be_transferred; // indicates if the line is to be transferred after consultation
	t_call_info call_info;
	unsigned short dummy;
	
	QLabel *statLabel, *holdLabel, *muteLabel, *confLabel, *referLabel, *statusTextLabel;
	
	if (line == 0) {
		statLabel = line1StatLabel;
		holdLabel = line1HoldLabel;
		muteLabel = line1MuteLabel;
		confLabel = line1ConfLabel;
		referLabel = line1ReferLabel;
		statusTextLabel = status1TextLabel;
	} else {
		statLabel = line2StatLabel;
		holdLabel = line2HoldLabel;
		muteLabel = line2MuteLabel;
		confLabel = line2ConfLabel;
		referLabel = line2ReferLabel;
		statusTextLabel = status2TextLabel;
	}
	
	state = lineSubstate2str(line);
	on_hold = phone->is_line_on_hold(line);
	if (on_hold) {
		holdLabel->show();
	} else {
		holdLabel->hide();
	}
	in_conference = phone->part_of_3way(line);
	if (in_conference) {
		confLabel->show();
	} else {
		confLabel->hide();
	}
	is_muted = phone->is_line_muted(line);
	if (is_muted) {
		muteLabel->show();
	} else {
		muteLabel->hide();
	}
	refer_state = phone->get_line_refer_state(line);
	is_transfer_consult = phone->is_line_transfer_consult(line, dummy);
	to_be_transferred = phone->line_to_be_transferred(line, dummy);
	if (refer_state != REFST_NULL || is_transfer_consult || to_be_transferred) {
		QString toolTip;
		QToolTip::remove(referLabel);
		referLabel->show();
		if (is_transfer_consult) {
			referLabel->setPixmap(
				QPixmap::fromMimeSource("consult-xfer.png"));
			toolTip = tr("Transfer consultation");
		} else {
			referLabel->setPixmap(
				QPixmap::fromMimeSource("cf.png"));
			toolTip = tr("Transferring call");
		}
		QToolTip::add(referLabel, toolTip);
	} else {
		referLabel->hide();
	}
	
	statusTextLabel->setText(state);
	
	t_line_substate line_substate;
	line_substate = phone->get_line_substate(line);
	switch (line_substate) {
	case LSSUB_IDLE:
		((t_gui *)ui)->clearLineFields(line);
		statLabel->hide();
		break;
	case LSSUB_SEIZED:
	case LSSUB_OUTGOING_PROGRESS:
		statLabel->setPixmap(QPixmap::fromMimeSource("stat_outgoing.png"));
		statLabel->show();
		break;
	case LSSUB_INCOMING_PROGRESS:
		statLabel->setPixmap(QPixmap::fromMimeSource("stat_ringing.png"));
		statLabel->show();
		break;
	case LSSUB_ANSWERING:
		statLabel->setPixmap(QPixmap::fromMimeSource("gear.png"));
		statLabel->show();
		break;
	case LSSUB_ESTABLISHED:
		if (phone->has_line_media(line)) {
			statLabel->setPixmap(QPixmap::fromMimeSource(
					"stat_established.png"));
		} else {
			statLabel->setPixmap(QPixmap::fromMimeSource(
					"stat_established_nomedia.png"));
		}
		statLabel->show();
		break;
	case LSSUB_RELEASING:
		statLabel->setPixmap(QPixmap::fromMimeSource("gear.png"));
		statLabel->show();
		break;
	default:
		statLabel->hide();
		break;
	}
	
	updateLineEncryptionState(line);
	updateLineTimer(line);
}

// Update line state and enable/disable buttons depending on state
void MphoneForm::updateState()
{
	QString state;
	int line, other_line;
	bool on_hold; // indicates if a line is put on-hold
	bool in_conference; // indicates if a line is in a conference
	bool is_muted; // indicates is a line is muted
	t_refer_state refer_state; // indicates if a call transfer is in progress
	bool is_transfer_consult; // indicates if the call is a consultation
	bool to_be_transferred; // indicates if the line is to be transferred after consultation
	bool has_media; // indicates if a media stream is present
	t_call_info call_info;
	unsigned short dummy;
	
	// Update status of line 1
	updateLineStatus(0);
	
	// Update status of line 2
	updateLineStatus(1);
	
	// Disable/enable controls depending on the active line state
	t_line_substate line_substate;
	line = phone->get_active_line();
	line_substate = phone->get_line_substate(line);
	on_hold = phone->is_line_on_hold(line);
	in_conference = phone->part_of_3way(line);
	is_muted = phone->is_line_muted(line);
	refer_state = phone->get_line_refer_state(line);
	is_transfer_consult = phone->is_line_transfer_consult(line, dummy);
	to_be_transferred = phone->line_to_be_transferred(line, dummy);
	has_media = phone->has_line_media(line);
	other_line = (line == 0 ? 1 : 0);
	call_info = phone->get_call_info(line);
	t_user *user_config = phone->get_line_user(line);
	
	// The active line may change when one of the parties in a conference
	// releases the call. If this happens, then update the state of the
	// line radio buttons.
	if (line == 0 && line2RadioButton->isOn()) 
	{
		line1RadioButton->setChecked(true);
	} else if (line == 1 && line1RadioButton->isOn())
	{
		line2RadioButton->setChecked(true);
	}
	
	// Same logic for the activate line menu items
	if (line == 0 && actionLine2->isOn()) 
	{
		actionLine1->setOn(true);
	} else if (line == 1 && actionLine1->isOn())
	{
		actionLine2->setOn(true);
	}
	
	switch(line_substate) {
	case LSSUB_IDLE:
		enableCallOptions(true);
		callAnswer->setEnabled(false);
		callBye->setEnabled(false);
		callReject->setEnabled(false);
		callRedirect->setEnabled(false);
		callTransfer->setEnabled(false);
		callHold->setEnabled(false);
		callConference->setEnabled(false);
		callMute->setEnabled(false);
		callDTMF->setEnabled(false);
		callRedial->setEnabled(ui->can_redial());
		break;
	case LSSUB_OUTGOING_PROGRESS:
		enableCallOptions(false);
		callAnswer->setEnabled(false);
		callBye->setEnabled(true);
		callReject->setEnabled(false);
		callRedirect->setEnabled(false);
		
		if (is_transfer_consult && user_config->get_allow_transfer_consultation_inprog()) {
			callTransfer->setEnabled(true);
		} else {
			callTransfer->setEnabled(false);
		}
		
		callHold->setEnabled(false);
		callConference->setEnabled(false);
		callMute->setEnabled(false);
		callDTMF->setEnabled(call_info.dtmf_supported);
		callRedial->setEnabled(false);
		break;
	case LSSUB_INCOMING_PROGRESS:
		enableCallOptions(false);
		callAnswer->setEnabled(true);
		callBye->setEnabled(false);
		callReject->setEnabled(true);
		callRedirect->setEnabled(true);
		callTransfer->setEnabled(false);
		callHold->setEnabled(false);
		callConference->setEnabled(false);
		callMute->setEnabled(false);
		callDTMF->setEnabled(call_info.dtmf_supported);
		callRedial->setEnabled(false);
		break;
	case LSSUB_ESTABLISHED:
		enableCallOptions(false);
		callInvite->setEnabled(false);
		callAnswer->setEnabled(false);
		callBye->setEnabled(true);
		callReject->setEnabled(false);
		callRedirect->setEnabled(false);
		
		if (in_conference) {
			callTransfer->setEnabled(false);
			callHold->setEnabled(false);
			callConference->setEnabled(false);
			callDTMF->setEnabled(false);
		} else {
			callTransfer->setEnabled(has_media && 
						 call_info.refer_supported &&
						 refer_state == REFST_NULL &&
						 !to_be_transferred);
			callHold->setEnabled(has_media);
			callDTMF->setEnabled(call_info.dtmf_supported);
			
			if (phone->get_line_substate(other_line) == 
			    LSSUB_ESTABLISHED)
			{
				// If one of the lines is transferring a call, then a
				// conference cannot be setup.
				if (refer_state != REFST_NULL ||
				    phone->get_line_refer_state(other_line) != REFST_NULL)
				{
					callConference->setEnabled(false);
				} else {
					callConference->setEnabled(has_media);
				}
			} else {
				callConference->setEnabled(false);
			}
		}
		
		callMute->setEnabled(true);
		callRedial->setEnabled(false);
		break;
	case LSSUB_SEIZED:
	case LSSUB_ANSWERING:
	case LSSUB_RELEASING:
		// During dialing, answering and call release no other actions are 
		// possible
		enableCallOptions(false);
		callAnswer->setEnabled(false);
		callBye->setEnabled(false);
		callReject->setEnabled(false);
		callRedirect->setEnabled(false);
		callTransfer->setEnabled(false);
		callHold->setEnabled(false);
		callConference->setEnabled(false);
		callMute->setEnabled(false);
		callDTMF->setEnabled(false);
		callRedial->setEnabled(false);
		break;
	default:
		enableCallOptions(true);
		callAnswer->setEnabled(true);
		callBye->setEnabled(true);
		callReject->setEnabled(true);
		callRedirect->setEnabled(true);
		callTransfer->setEnabled(true);
		callHold->setEnabled(true);
		callConference->setEnabled(false);
		callMute->setEnabled(true);
		callDTMF->setEnabled(true);
		callRedial->setEnabled(ui->can_redial());
	}
	
	// Set hold action in correct state
	callHold->setOn(on_hold);
	
	// Set mute action in correct state
	callMute->setOn(is_muted);
	
	// Set transfer action in correct state
	callTransfer->setOn(is_transfer_consult);
	
	// Hide redirect form if it is still visible, but not applicable anymore
	if (!callRedirect->isEnabled() && redirectForm && 
	    redirectForm->isVisible()) 
	{
		redirectForm->hide();
	}
	
	// Hide transfer form if it is still visible, but not applicable anymore
	if (!callTransfer->isEnabled() && transferForm && 
	    transferForm->isVisible()) 
	{
		transferForm->hide();
	}
	
	// Hide DTMF form if it is still visible, but not applicable anymore
	if (!callDTMF->isEnabled() && dtmfForm && 
	    dtmfForm->isVisible()) 
	{
		dtmfForm->hide();
	}
	
	// Set last called address in the redial tool tip
	t_url last_url;
	string last_display;
	string last_subject;
	t_user *last_user;
	bool hide_user;
	if (callRedial->isEnabled() && 
	    ui->get_last_call_info(last_url, last_display, last_subject, &last_user, hide_user))
	{
		QString s = "<b>";
		s += tr("Repeat last call");
		s += "</b><br>";
		s += "<table>";
		
		s += "<tr><td>";
		s += tr("User:").append("</td><td>");
		s += last_user->get_profile_name().c_str();
		s.append("</td></tr><tr><td>").append(tr("Call:")).append("</td><td>");
		s += ui->format_sip_address(last_user,
					last_display, last_url).c_str();
		s += "</td></tr>";

		if (!last_subject.empty()) {
			s.append("<tr><td>").append(tr("Subject:")).append("</td><td>");
			s += last_subject.c_str();
			s += "</td></tr>";
		}
		
		if (hide_user) {
			s.append("<tr><td colspan=2>").append(tr("Hide identity"));
			s += "</td></tr>";
		}
		
		s += "</table>";
		callRedial->setToolTip(s);
	} else {
		callRedial->setToolTip(tr("Repeat last call"));
	}
	callRedial->setStatusTip(tr("Repeat last call"));
	
	updateSysTrayStatus();
}

// Update registration status
void MphoneForm::updateRegStatus()
{
	size_t num_registered = 0;
	size_t num_failed = 0;
	QString toolTip = "<b>";
	toolTip.append(tr("Registration status:"));
	toolTip.append("</b><br>");
	toolTip.append("<table>");
	
	// Count number of succesful and failed registrations.
	// Determine tool tip showing registration details for all users.
	list<t_user *>user_list = phone->ref_users();
	for (list<t_user *>::iterator i = user_list.begin(); i != user_list.end(); i++) {
		toolTip.append("<tr><td>");
		toolTip.append((*i)->get_profile_name().c_str());
		toolTip.append("</td><td>");
		if (phone->get_is_registered(*i)) {
			num_registered++;
			toolTip.append(tr("Registered"));
		} else if (phone->get_last_reg_failed(*i)) {
			num_failed++;
			toolTip.append(tr("Failed"));
		} else {
			toolTip.append(tr("Not registered").replace(' ', "&nbsp;"));
		}
		toolTip.append("</td></tr>");
	}
	toolTip.append("</table><br>");
	toolTip.append("<i>");
	toolTip.append(tr("Click to show registrations.").replace(' ', "&nbsp;"));
	toolTip.append("</i>");
	
	// Set registration status
	if (num_registered == user_list.size()) {
		// All users are registered
		statRegLabel->setPixmap(QPixmap::fromMimeSource("twinkle16.png"));
	} else if (num_failed == user_list.size()) {
		// All users failed to register
		statRegLabel->setPixmap(QPixmap::fromMimeSource("reg_failed.png"));
	} else if (num_registered > 0) {
		// Some users are registered
		statRegLabel->setPixmap(QPixmap::fromMimeSource(
				"twinkle16.png"));
	} else if (num_failed > 0) {
		// Some users failed, none are registered
		statRegLabel->setPixmap(QPixmap::fromMimeSource("reg_failed.png"));
	} else {
		// No users are registered, no users failed
		statRegLabel->setPixmap(QPixmap::fromMimeSource("twinkle16-disabled.png"));
	}
	
	// Set tool tip with detailed info.
	QToolTip::remove(statRegLabel);
	
	if (num_registered > 0 || num_failed > 0) {
		QToolTip::add(statRegLabel, toolTip);
	} else {
		QToolTip::add(statRegLabel, tr("No users are registered."));
	}
	
	updateSysTrayStatus();
}

// Create a status message based on the number of waiting messages.
// On return, msg_waiting will indicate if the MWI indicator should show
// waiting messages.
QString MphoneForm::getMWIStatus(const t_mwi &mwi, bool &msg_waiting) const
{
	QString status;
	msg_waiting = false;
	t_msg_summary summary = mwi.get_voice_msg_summary();
		
	if (summary.newmsgs > 0 && summary.oldmsgs > 0) {
		if (summary.oldmsgs == 1) {
			status = tr("%1 new, 1 old message").
				 arg(summary.newmsgs);
		} else {
			status = tr("%1 new, %2 old messages").
				 arg(summary.newmsgs).
				 arg(summary.oldmsgs);
		}
		msg_waiting = true;
	} else if (summary.newmsgs > 0 && summary.oldmsgs == 0) {
		if (summary.newmsgs == 1) {
			status = tr("1 new message");
		} else {
			status = tr("%1 new messages").
				 arg(summary.newmsgs);
		}
		msg_waiting = true;
	} else if (summary.oldmsgs > 0) {
		if (summary.oldmsgs == 1) {
			status = tr("1 old message");
		} else {
			status = tr("%1 old messages").
				 arg(summary.oldmsgs);
		}
	} else {
		if (mwi.get_msg_waiting()) {
			status = tr("Messages waiting");
			msg_waiting = true;
		} else {
			status = tr("No messages");
		}
	}

	return status.replace(' ', "&nbsp;");
}

// Flash the MWI icon
void MphoneForm::flashMWI()
{
	if (mwiFlashStatus) {
		mwiFlashStatus = false;
		statMWILabel->setPixmap(QPixmap::fromMimeSource(
				"mwi_none16.png"));
	} else {
		mwiFlashStatus = true;
		statMWILabel->setPixmap(QPixmap::fromMimeSource(
				"mwi_new16.png"));
	}
}

// Update MWI
void MphoneForm::updateMwi()
{
	bool mwi_known = false;
	bool mwi_new_msgs = false;
	bool mwi_failure = false;

	// Determine tool tip
	QString toolTip = tr("<b>Voice mail status:</b>").append("\n");
	toolTip.append("<br><table>");
	list<t_user *>user_list = phone->ref_users();
	for (list<t_user *>::iterator i = user_list.begin(); i != user_list.end(); i++) {
		toolTip.append("<tr><td>");
		toolTip.append((*i)->get_profile_name().c_str());
		t_mwi mwi = phone->get_mwi(*i);
		toolTip.append("</td><td>");
		if (phone->is_mwi_subscribed(*i)) {
			if (mwi.get_status() == t_mwi::MWI_KNOWN) {
				bool new_msgs;
				QString status = getMWIStatus(mwi, new_msgs);
				toolTip.append(status);
				mwi_known = true;
				mwi_new_msgs |= new_msgs;
			} else if (mwi.get_status() == t_mwi::MWI_FAILED) {
				toolTip.append(tr("Failure"));
				mwi_failure = true;
			} else {
				toolTip.append(tr("Unknown"));
			}
		} else {
			if ((*i)->get_mwi_sollicited()) {
				if (mwi.get_status() == t_mwi::MWI_FAILED) {
					toolTip.append(tr("Failure"));
					mwi_failure = true;
				} else {
					toolTip.append(tr("Unknown"));
				}
			} else {
				// Unsollicited MWI				
				if (mwi.get_status() == t_mwi::MWI_KNOWN) {
					bool new_msgs;
					QString status = getMWIStatus(mwi, new_msgs);
					toolTip.append(status);
					mwi_known = true;
					mwi_new_msgs |= new_msgs;
				} else {
					toolTip.append(tr("Unknown"));
				}
			}
		}
		toolTip.append("</td></tr>");
	}
	
	toolTip.append("</table><br>");
	toolTip.append("<i>");
	toolTip.append(tr("Click to access voice mail.").replace(' ', "&nbsp;"));
	toolTip.append("</i>");
	
	// Set MWI icon
	if (mwi_new_msgs) {
		statMWILabel->setPixmap(QPixmap::fromMimeSource(
			"mwi_new16.png"));
		mwiFlashStatus = true;
		
		// Start the flash MWI timer to flash the indicator
		tmrFlashMWI.start(1000);
	} else if (mwi_failure) {
		tmrFlashMWI.stop();
		statMWILabel->setPixmap(QPixmap::fromMimeSource(
			"mwi_failure16.png"));
	} else if (mwi_known) {
		tmrFlashMWI.stop();
		statMWILabel->setPixmap(QPixmap::fromMimeSource(
			"mwi_none16.png"));
	} else {
		tmrFlashMWI.stop();
		statMWILabel->setPixmap(QPixmap::fromMimeSource(
			"mwi_none16_dis.png"));
	}
	
	// Set tool tip
	QToolTip::remove(statMWILabel);
	QToolTip::add(statMWILabel, toolTip);
	
	updateSysTrayStatus();
}

// Update active services status
void MphoneForm::updateServicesStatus()
{	
	size_t num_dnd = 0;
	size_t num_cf = 0;
	size_t num_auto_answer = 0;
	QString tipDnd = "<b>";
	tipDnd += tr("Do not disturb active for:").replace(' ', "&nbsp;");
	tipDnd += "</b><br>\n<table>";
	QString tipCf = "<b>";
	tipCf += tr("Redirection active for:").replace(' ', "&nbsp;");
	tipCf +=  "</b><br>\n<table>";
	QString tipAa = "<b>";
	tipAa += tr("Auto answer active for:").replace(' ', "&nbsp;");
	tipAa += "</b><br>\n<table>";
	
	// Calculate number of services active.
	// Determine tool tips with detailed service status for all users.
	list<t_user *>user_list = phone->ref_users();
	for (list<t_user *>::iterator i = user_list.begin(); i != user_list.end(); i++) {
		if (phone->ref_service(*i)->is_dnd_active()) {
			num_dnd++;
			tipDnd.append("<tr><td>");
			tipDnd.append((*i)->get_profile_name().c_str());
			tipDnd.append("</td></tr>");
		}
		if (phone->ref_service(*i)->is_cf_active()) {
			num_cf++;
			tipCf.append("<tr><td>");
			tipCf.append((*i)->get_profile_name().c_str());
			tipCf.append("</td></tr>");
		}
		if (phone->ref_service(*i)->is_auto_answer_active()) {
			num_auto_answer++;
			tipAa.append("<tr><td>");
			tipAa.append((*i)->get_profile_name().c_str());
			tipAa.append("</td></tr>");
		}
	}
	
	QString footer = "<i>";
	footer += tr("Click to activate/deactivate").replace(' ', "&nbsp;");
	footer += "</i>";
	
	tipDnd.append("</table><br>");
	tipDnd.append(footer);
	tipCf.append("</table><br>");
	tipCf.append(footer);
	tipAa.append("</table><br>");
	tipAa.append(footer);
	
	// Set service status
	if (num_dnd == user_list.size()) {
		// All users enabled dnd
		statDndLabel->setPixmap(QPixmap::fromMimeSource("cancel.png"));
	} else if (num_dnd > 0) {
		// Some users enabled dnd
		statDndLabel->setPixmap(QPixmap::fromMimeSource("cancel.png"));
	} else {
		// No users enabeld dnd
		statDndLabel->setPixmap(QPixmap::fromMimeSource("cancel-disabled.png"));
	}
	
	if (num_cf == user_list.size()) {
		// All users enabled redirecton
		statCfLabel->setPixmap(QPixmap::fromMimeSource("cf.png"));
	} else if (num_cf > 0) {
		// Some users enabled redirection
		statCfLabel->setPixmap(QPixmap::fromMimeSource("cf.png"));
	} else {
		// No users enabled redirection
		statCfLabel->setPixmap(QPixmap::fromMimeSource("cf-disabled.png"));
	}
	
	if (num_auto_answer == user_list.size()) {
		// All users enabled auto answer
		statAaLabel->setPixmap(QPixmap::fromMimeSource("auto_answer.png"));
	} else if (num_auto_answer > 0) {
		// Some users enabled auto answer
		statAaLabel->setPixmap(QPixmap::fromMimeSource(
				"auto_answer.png"));
	} else {
		// No users enabeld auto answer
		statAaLabel->setPixmap(QPixmap::fromMimeSource(
				"auto_answer-disabled.png"));
	}
	
	// Set tool tip with detailed info for multiple users.
	QToolTip::remove(statDndLabel);
	QToolTip::remove(statCfLabel);
	QToolTip::remove(statAaLabel);

	QString clickToActivate("<i>");
	clickToActivate += tr("Click to activate").replace(' ', "&nbsp;");
	clickToActivate += "</i>";
	if (num_dnd > 0) {
		QToolTip::add(statDndLabel, tipDnd);
	} else {
		QString status("<p>");
		status += tr("Do not disturb is not active.").replace(' ', "&nbsp;");
		status += "</p>";
		status += clickToActivate;
		QToolTip::add(statDndLabel, status);
	}		
	
	if (num_cf > 0) {
		QToolTip::add(statCfLabel, tipCf);
	} else {
		QString status("<p>");
		status += tr("Redirection is not active.").replace(' ', "&nbsp;");
		status += "</p>";
		status += clickToActivate;
		QToolTip::add(statCfLabel, status);
	}
	
	if (num_auto_answer > 0) {
		QToolTip::add(statAaLabel, tipAa);
	} else {
		QString status("<p>");
		status += tr("Auto answer is not active.").replace(' ', "&nbsp;");
		status += "</p>";
		status += clickToActivate;
		QToolTip::add(statAaLabel, status);
	}
	
	updateSysTrayStatus();
}

void MphoneForm::updateMissedCallStatus(int num_missed_calls)
{
	QToolTip::remove(statMissedLabel);
	
	QString clickDetails("<i>");
	clickDetails += tr("Click to see call history for details.").replace(' ', "&nbsp;");
	clickDetails += "</i>";
	if (num_missed_calls == 0) {
		statMissedLabel->setPixmap(QPixmap::fromMimeSource("missed-disabled.png"));
		QString status("<p>");
		status += tr("You have no missed calls.").replace(' ', "&nbsp;");
		status += "</p>";
		status += clickDetails;
		QToolTip::add(statMissedLabel, status);
	} else {
		statMissedLabel->setPixmap(
			QPixmap::fromMimeSource("missed.png"));
		
		QString tip("<p>");
		if (num_missed_calls == 1) {
			tip += tr("You missed 1 call.").replace(' ', "&nbsp;");
		} else {
			tip += tr("You missed %1 calls.").arg(num_missed_calls).
			       replace(' ', "&nbsp;");
		}
		tip += "</p>";
		tip += clickDetails;
		QToolTip::add(statMissedLabel, tip);
	}
	
	updateSysTrayStatus();
}

// Update system tray status
void MphoneForm::updateSysTrayStatus()
{
	QString icon_name;
	bool cf_active = false;
	bool dnd_active = false;
	bool auto_answer_active = false;
	bool multi_services = false;
	int num_services;
	bool msg_waiting = false;
	
	if (!sysTray) return;
	
	// Get status of active line
	int line = phone->get_active_line();
	t_line_substate line_substate = phone->get_line_substate(line);
	
	list<t_user *> user_list = phone->ref_users();
	
	switch(line_substate) {
	case LSSUB_IDLE:
	case LSSUB_SEIZED:
		// Determine MWI and service status
		user_list = phone->ref_users();
		for (list<t_user *>::iterator i = user_list.begin(); i != user_list.end(); i++) {
			t_mwi mwi = phone->get_mwi(*i);
			if (mwi.get_status() == t_mwi::MWI_KNOWN &&
			    mwi.get_msg_waiting() &&
			    mwi.get_voice_msg_summary().newmsgs > 0)
			{
				msg_waiting = true;
			} else if (phone->ref_service(*i)->multiple_services_active()) {
				multi_services = true;
			} else {
				if (phone->ref_service(*i)->is_dnd_active())  {
					dnd_active = true;
				}
				if (phone->ref_service(*i)->is_cf_active()) {
					cf_active = true;
				}
				if (phone->ref_service(*i)->is_auto_answer_active()) {
					auto_answer_active = true;
				}
			}
		}
		
		// If there are messages waiting, then show MWI icon
		if (msg_waiting) {
			icon_name = "sys_mwi";
			break;
		}
		
		// If there are missed calls, then show the missed call icon
		if (call_history->get_num_missed_calls() > 0) {
			icon_name = "sys_missed";
			break;
		}
		
		// If a service is active, then show the service icon
		num_services = (dnd_active ? 1 : 0) + (cf_active ? 1 : 0) + 
			       (auto_answer_active ? 1 : 0);
		
		if (multi_services || num_services > 1) {
			icon_name = "sys_services";
		} else if (dnd_active) {
			icon_name = "sys_dnd";
		} else if (cf_active) {
			icon_name = "sys_redir";
		} else if (auto_answer_active) {
			icon_name = "sys_auto_ans";
		} else {
			// No service is active, show the idle icon
			if (icon_name.isEmpty()) icon_name = "sys_idle";
		}

		break;
	case LSSUB_ESTABLISHED:
		if (phone->is_line_on_hold(line)) {
			icon_name = "sys_hold";
		} else if (phone->is_line_muted(line)) {
			icon_name = "sys_mute";
		} else if (phone->is_line_encrypted(line)) {
			t_audio_session *as = phone->get_line(line)->get_audio_session();
			if (as && as->get_zrtp_sas_confirmed()) {
				icon_name = "sys_encrypted_verified";
			} else {
				icon_name = "sys_encrypted";
			}
		} else {
			icon_name = "sys_busy_estab";
		}
		break;
	default:
		// Line is in a busy transient state
		icon_name = "sys_busy_trans";
	}
	
	// Based on the registration status use the active or disabled version
	// of the icon.
	bool registered = false;
	for (list<t_user *>::iterator i = user_list.begin(); i != user_list.end(); i++) {
		if (phone->get_is_registered(*i)) {
			registered = true;
			break;
		}
	}
	
	if (registered) {
		icon_name += ".png";
	} else {
		icon_name += "_dis.png";
	}
	
	sysTray->setPixmap(QPixmap::fromMimeSource(icon_name));
}

// Update menu status based on the number of active users
void MphoneForm::updateMenuStatus()
{
	// Some menu options should be toggle actions when there is only
	// 1 user active, but they should be normal actions when there are
	// multiple users.
	disconnect(serviceDnd, 0, 0, 0);
	disconnect(serviceAutoAnswer, 0, 0, 0);
	if (phone->ref_users().size() == 1) {
		t_service *srv = phone->ref_service(phone->ref_users().front());
		
		serviceDnd->setToggleAction(true);
		serviceDnd->setOn(srv->is_dnd_active());
		connect(serviceDnd, SIGNAL(toggled(bool)),
			this, SLOT(srvDnd(bool)));
		
		serviceAutoAnswer->setToggleAction(true);
		serviceAutoAnswer->setOn(srv->is_auto_answer_active());
		connect(serviceAutoAnswer, SIGNAL(toggled(bool)),
			this, SLOT(srvAutoAnswer(bool)));
	} else {
		serviceDnd->setOn(false);
		serviceDnd->setToggleAction(false);
		connect(serviceDnd, SIGNAL(activated()),
			this, SLOT(srvDnd()));
		
		serviceAutoAnswer->setOn(false);
		serviceAutoAnswer->setToggleAction(false);
		connect(serviceAutoAnswer, SIGNAL(activated()),
			this, SLOT(srvAutoAnswer()));
	}
	
	updateDiamondcardMenu();
}

void MphoneForm::updateDiamondcardMenu()
{
	// If one Diamondcard user is active, then create actions in the Diamondcard
	// main menu for recharging, call history, etc. These actions will show the
	// Diamondcard web page.
	// If multiple Diamondcard users are active then create a submenu of each
	// Diamondcard action. In each submenu create an item for each user.
	// When a user item is clicked, the web page for the action and that user is
	// shown.
	list<t_user *> diamondcard_users = diamondcard_get_users(phone);
	
	// Menu item identifiers
	static int rechargeId = -1;
	static int balanceHistoryId = -1;
	static int callHistoryId = -1;
	static int adminCenterId = -1;
	
	// Sub menu's
	static QPopupMenu *rechargeMenu = NULL;
	static QPopupMenu *balanceHistoryMenu = NULL;
	static QPopupMenu *callHistoryMenu = NULL;
	static QPopupMenu *adminCenterMenu = NULL;
	
	// Clear old menu
	removeDiamondcardAction(rechargeId);
	removeDiamondcardAction(balanceHistoryId);
	removeDiamondcardAction(callHistoryId);
	removeDiamondcardAction(adminCenterId);
	removeDiamondcardMenu(rechargeMenu);
	removeDiamondcardMenu(balanceHistoryMenu);
	removeDiamondcardMenu(callHistoryMenu);
	removeDiamondcardMenu(adminCenterMenu);
	
	if (diamondcard_users.size() <= 1)
	{
		rechargeId = Diamondcard->insertItem(tr("Recharge..."), this, SLOT(DiamondcardRecharge(int)));
		Diamondcard->setItemParameter(rechargeId, 0);
		balanceHistoryId = Diamondcard->insertItem(tr("Balance history..."), this, SLOT(DiamondcardBalanceHistory(int)));
		Diamondcard->setItemParameter(balanceHistoryId, 0);
		callHistoryId = Diamondcard->insertItem(tr("Call history..."), this, SLOT(DiamondcardCallHistory(int)));
		Diamondcard->setItemParameter(callHistoryId, 0);
		adminCenterId = Diamondcard->insertItem(tr("Admin center..."), this, SLOT(DiamondcardAdminCenter(int)));
		Diamondcard->setItemParameter(adminCenterId, 0);
		
		// Disable actions as there is no active Diamondcard users.
		if (diamondcard_users.empty()) {
			Diamondcard->setItemEnabled(rechargeId, false);
			Diamondcard->setItemEnabled(balanceHistoryId, false);
			Diamondcard->setItemEnabled(callHistoryId, false);
			Diamondcard->setItemEnabled(adminCenterId, false);
		}
	}
	else
	{
		rechargeMenu = new QPopupMenu(this);
		balanceHistoryMenu = new QPopupMenu(this);
		callHistoryMenu = new QPopupMenu(this);
		adminCenterMenu = new QPopupMenu(this);
		// No MEMMAN registration as the popup menu may be automatically
		// deleted by Qt on application close down. This would show up as
		// a memory leak in MEMMAN.
		
		// Insert a menu item for each Diamondcard user.
		int idx = 0;
		for (list<t_user *>::const_iterator it = diamondcard_users.begin(); 
		       it != diamondcard_users.end(); ++it)
		{
			int menuId;
			t_user *user = *it;
			
			// Set the index in the user list as parameter to the menu item.
			// When the menu item gets clicked, then the receiver of the signal
			// received this parameter and can use it as an index in the user list
			// to find the user.
			
			menuId = rechargeMenu->insertItem(user->get_profile_name().c_str(), this,
							      SLOT(DiamondcardRecharge(int)));
			rechargeMenu->setItemParameter(menuId, idx);
			menuId = balanceHistoryMenu->insertItem(user->get_profile_name().c_str(), this,
							      SLOT(DiamondcardBalanceHistory(int)));
			balanceHistoryMenu->setItemParameter(menuId, idx);
			menuId = callHistoryMenu->insertItem(user->get_profile_name().c_str(), this,
							      SLOT(DiamondcardCallHistory(int)));
			callHistoryMenu->setItemParameter(menuId, idx);
			menuId = adminCenterMenu->insertItem(user->get_profile_name().c_str(), this,
							      SLOT(DiamondcardAdminCenter(int)));
			adminCenterMenu->setItemParameter(menuId, idx);
			
			++idx;
		}
		
		// Add the Diamondcard popup menus to the main Diamondcard menu.
		Diamondcard->insertItem(tr("Recharge"), rechargeMenu);
		Diamondcard->insertItem(tr("Balance history"), balanceHistoryMenu);
		Diamondcard->insertItem(tr("Call history"), callHistoryMenu);
		Diamondcard->insertItem(tr("Admin center"), adminCenterMenu);
	}
}

void MphoneForm::removeDiamondcardAction(int &id)
{
	if (id != -1) {
		Diamondcard->removeItem(id);
		id = -1;
	}
}

void MphoneForm::removeDiamondcardMenu(QPopupMenu* &menu)
{
	if (menu) {
		delete menu;
		menu = NULL;
	}
}

void MphoneForm::phoneRegister()
{
	t_gui *gui = (t_gui *)ui;
	list<t_user *> user_list = phone->ref_users();
	
	if (user_list.size() > 1) {
		if (selectUserForm) {
			MEMMAN_DELETE(selectUserForm);
			delete (selectUserForm);
		}
		
		selectUserForm = new SelectUserForm(this, "register", true);
		MEMMAN_NEW(selectUserForm);
		
		connect(selectUserForm, SIGNAL(selection(list<t_user *>)), this, 
			SLOT(do_phoneRegister(list<t_user *>)));
		selectUserForm->show(SELECT_REGISTER);
	} else {
		gui->action_register(user_list);
	}
}

void MphoneForm::do_phoneRegister(list<t_user *> user_list)
{
	((t_gui *)ui)->action_register(user_list);
}

void MphoneForm::phoneDeregister()
{
	t_gui *gui = (t_gui *)ui;
	list<t_user *> user_list = phone->ref_users();
	
	if (user_list.size() > 1) {
		if (selectUserForm) {
			MEMMAN_DELETE(selectUserForm);
			delete (selectUserForm);
		}
		
		selectUserForm = new SelectUserForm(this, "deregister", true);
		MEMMAN_NEW(selectUserForm);
		
		connect(selectUserForm, SIGNAL(selection(list<t_user *>)), this, 
			SLOT(do_phoneDeregister(list<t_user *>)));
		selectUserForm->show(SELECT_DEREGISTER);
	} else {
		gui->action_deregister(user_list, false);
	}
}

void MphoneForm::do_phoneDeregister(list<t_user *> user_list)
{
	((t_gui *)ui)->action_deregister(user_list, false);
}

void MphoneForm::phoneDeregisterAll()
{
	t_gui *gui = (t_gui *)ui;
	list<t_user *> user_list = phone->ref_users();
	
	if (user_list.size() > 1) {
		if (selectUserForm) {
			MEMMAN_DELETE(selectUserForm);
			delete (selectUserForm);
		}
		
		selectUserForm = new SelectUserForm(this, "deregister all", true);
		MEMMAN_NEW(selectUserForm);
	
		connect(selectUserForm, SIGNAL(selection(list<t_user *>)), this, 
			SLOT(do_phoneDeregisterAll(list<t_user *>)));
		selectUserForm->show(SELECT_DEREGISTER_ALL);
	} else {
		gui->action_deregister(user_list, true);
	}
}

void MphoneForm::do_phoneDeregisterAll(list<t_user *> user_list)
{
	((t_gui *)ui)->action_deregister(user_list, true);
}

void MphoneForm::phoneShowRegistrations()
{
	list<t_user *> user_list = phone->ref_users();
	((t_gui *)ui)->action_show_registrations(user_list);
}


// Show the semi-modal invite window
void MphoneForm::phoneInvite(t_user * user_config, 
		const QString &dest, const QString &subject, bool anonymous)
{
	// Seize the line, so no incoming call can take the line
	if (!((t_gui *)ui)->action_seize()) return;
	
	if (inviteForm) {
		inviteForm->clear();
	} else {
		inviteForm = new InviteForm(this, "invite", true);
		MEMMAN_NEW(inviteForm);
		
		// Initialize the destination history list
		for (int i = callComboBox->count() - 1; i >= 0; i--) {
			inviteForm->addToInviteComboBox(callComboBox->text(i));
		}
		
		connect(inviteForm, 
			SIGNAL(destination(t_user *, const QString &, const t_url &, 
					   const QString &, bool)),
			this, 
			SLOT(do_phoneInvite(t_user *, const QString &, 
				    const t_url &, const QString &, bool)));
		
		connect(inviteForm, SIGNAL(raw_destination(const QString &)), 
			this, SLOT(addToCallComboBox(const QString &)));
	}
	
	inviteForm->show(user_config, dest, subject, anonymous);
	updateState();
}

void MphoneForm::phoneInvite(const QString &dest, const QString &subject, bool anonymous)
{
	t_user *user = phone->ref_user_profile(userComboBox->currentText().ascii());
	if (!user) {
		log_file->write_report("Cannot find user profile.",
			       "MphoneForm::phoneInvite", 
			       LOG_NORMAL, LOG_CRITICAL);
		return;
	}
	phoneInvite(user, dest, subject, anonymous);
}

void MphoneForm::phoneInvite()
{
	t_user *user = phone->ref_user_profile(userComboBox->currentText().ascii());
	if (!user) {
		log_file->write_report("Cannot find user profile.",
			       "MphoneForm::phoneInvite", 
			       LOG_NORMAL, LOG_CRITICAL);
		return;
	}
	phoneInvite(user, "", "", false);
}

// Execute the invite action. This slot is connected to the destination
// signal of the invite window.
void MphoneForm::do_phoneInvite(t_user *user_config, const QString &display, 
			const t_url &destination, const QString &subject,
			bool anonymous)
{
	((t_gui *)ui)->action_invite(user_config, destination, display.ascii(), subject.ascii(),
				     anonymous);
	updateState();
}

// Redial last call
void MphoneForm::phoneRedial(void)
{
	t_url url;
	string display, subject;
	t_user *user_config;
	bool hide_user;
	
	if (!ui->get_last_call_info(url, display, subject, &user_config, hide_user)) return;
	((t_gui *)ui)->action_invite(user_config, url, display, subject, hide_user);
	updateState();
}


void MphoneForm::phoneAnswer()
{
	((t_gui *)ui)->action_answer();
	updateState();
}

// A call can be answered from the systray popup. The user may have
// switched lines, the systray popup answer button should answer the
// correct line.
void MphoneForm::phoneAnswerFromSystrayPopup()
{
#ifdef HAVE_KDE
	unsigned short line = ((t_gui *)ui)->get_line_sys_tray_popup();
	unsigned short active_line = phone->get_active_line();
	
	if (line != active_line) {
		((t_gui *)ui)->action_activate_line(line);
	}
	
	((t_gui *)ui)->action_answer();
	updateState();
#endif
}

void MphoneForm::phoneBye()
{
	((t_gui *)ui)->action_bye();
	updateState();
}


void MphoneForm::phoneReject()
{
	((t_gui *)ui)->action_reject();
	updateState();
}

// A call can be rejected from the systray popup. The user may have
// switched lines, the systray popup reject button should answer the
// correct line.
void MphoneForm::phoneRejectFromSystrayPopup()
{
#ifdef HAVE_KDE
	unsigned short line = ((t_gui *)ui)->get_line_sys_tray_popup();
	((t_gui *)ui)->action_reject(line);
	updateState();
#endif
}


// Show the semi-modal redirect form
void MphoneForm::phoneRedirect(const list<string> &contacts)
{
	int active_line = phone->get_active_line();
	t_user *user_config = phone->get_line_user(active_line);
	
	if (redirectForm) {
		MEMMAN_DELETE(redirectForm);
		delete (redirectForm);
	}
	
	redirectForm = new RedirectForm(this, "redirect", true);
	MEMMAN_NEW(redirectForm);
	connect(redirectForm, SIGNAL(destinations(const list<t_display_url> &)),
		this, SLOT(do_phoneRedirect(const list<t_display_url> &)));
	
	redirectForm->show(user_config, contacts);
}

void MphoneForm::phoneRedirect()
{
	const list<string> l;
	phoneRedirect(l);
}

// Execute the redirect action.
void MphoneForm::do_phoneRedirect(const list<t_display_url> &destinations)
{
	((t_gui *)ui)->action_redirect(destinations);
	updateState();
}

// Show the semi-modal call transfer window
void MphoneForm::phoneTransfer(const string &dest, t_transfer_type transfer_type)
{
	int active_line = phone->get_active_line();
	t_user *user_config = phone->get_line_user(active_line);
	
	// Hold the call if setting in user profile indicates call hold
	if (user_config->get_referrer_hold()) {
		phoneHold(true);
	}
	
	if (transferForm) {
		MEMMAN_DELETE(transferForm);
		delete transferForm;
	}
	
	transferForm = new TransferForm(this, "transfer", true);
	MEMMAN_NEW(transferForm);
	connect(transferForm, SIGNAL(destination(const t_display_url &, t_transfer_type)),
		this, SLOT(do_phoneTransfer(const t_display_url &, t_transfer_type)));
	
	if (dest.empty() && transfer_type == TRANSFER_BASIC) {
		// Let form pick a default transfer type based on the current
		// call status.
		transferForm->show(user_config);
	} else {
		// Set passed destination and transfer type in form
		transferForm->show(user_config, dest, transfer_type);
	}
	updateState();
}

void MphoneForm::phoneTransfer()
{
	unsigned short active_line = phone->get_active_line();
	unsigned short dummy;
	
	if (phone->is_line_transfer_consult(active_line, dummy)) {
		do_phoneTransferLine();
	} else {
		phoneTransfer("", TRANSFER_BASIC);
	}
}

// Execute the transfer action. This slot is connected to the destination
// signal of the transfer window.
void MphoneForm::do_phoneTransfer(const t_display_url &destination, 
				  t_transfer_type transfer_type)
{
	unsigned short active_line;
	unsigned short other_line;
		
	switch (transfer_type) {
	case TRANSFER_BASIC:
		((t_gui *)ui)->action_refer(destination.url, destination.display);
		break;
	case TRANSFER_CONSULT:
		((t_gui *)ui)->action_setup_consultation_call(
				destination.url, destination.display);
		break;
	case TRANSFER_OTHER_LINE:
		active_line = phone->get_active_line();
		other_line = (active_line == 0 ? 1 : 0);
		
		if (phone->get_line_substate(other_line) == LSSUB_ESTABLISHED) {
			((t_gui *)ui)->action_refer(active_line, other_line);
		} else {
			// The other line was released while the user was entering
			// the refer-target.
			t_user *user_config = phone->get_line_user(active_line);
			if (user_config->get_referrer_hold()) {
				phoneHold(false);
			}
		}
		break;
	default:
		assert(false);
	}
	
	updateState();
}

// Transfer the remote party on the held line to the remote party on the
// active line.
void MphoneForm::do_phoneTransferLine()
{
	unsigned short active_line = phone->get_active_line();
	unsigned short line_to_be_transferred;
	
	if (!phone->is_line_transfer_consult(active_line, line_to_be_transferred)) {
		// Somehow the line is not a consultation call.
		updateState();
		return;
	}
	
	((t_gui *)ui)->action_refer(line_to_be_transferred, active_line);
	updateState();
}

void MphoneForm::phoneHold(bool on)
{
	if (on) {
		((t_gui *)ui)->action_hold();
	} else {
		((t_gui *)ui)->action_retrieve();
	}
	
	updateState();
}

void MphoneForm::phoneConference()
{
	((t_gui *)ui)->action_conference();
	updateState();
}

void MphoneForm::phoneMute(bool on)
{
	((t_gui *)ui)->action_mute(on);
	updateState();
}

void MphoneForm::phoneTermCap(const QString &dest)
{
	// In-dialog OPTIONS request
	int line = phone->get_active_line();
	if (phone->get_line_substate(line) == LSSUB_ESTABLISHED) {
		((t_gui *)ui)->action_options();
		return;
	}
	
	// Out-of-dialog OPTIONS request
	if (termCapForm) {
		MEMMAN_DELETE(termCapForm);
		delete (termCapForm);
	}
	
	termCapForm = new TermCapForm(this, "termcap", true);
	MEMMAN_NEW(termCapForm);
	connect(termCapForm, SIGNAL(destination(t_user *, const t_url &)),
		this, SLOT(do_phoneTermCap(t_user *, const t_url &)));
	
	t_user *user = phone->ref_user_profile(userComboBox->currentText().ascii());
	if (!user) {
		log_file->write_report("Cannot find user profile.",
			       "MphoneForm::phoneTermcap", 
			       LOG_NORMAL, LOG_CRITICAL);
		return;
	}
	termCapForm->show(user, dest);
}

void MphoneForm::phoneTermCap()
{
	phoneTermCap("");
}

void MphoneForm::do_phoneTermCap(t_user *user_config, const t_url &destination)
{
	((t_gui *)ui)->action_options(user_config, destination);
}

void MphoneForm::phoneDTMF()
{
	if (!dtmfForm) {
		dtmfForm = new DtmfForm(this);
		MEMMAN_NEW(dtmfForm);
		connect(dtmfForm, SIGNAL(digits(const QString &)),
			this, SLOT(sendDTMF(const QString &)));
	}
	
	dtmfForm->show();
}

void MphoneForm::sendDTMF(const QString &digits)
{
	((t_gui *)ui)->action_dtmf(digits.ascii());	
}

void MphoneForm::startMessageSession(void)
{
	t_user *user = phone->ref_user_profile(userComboBox->currentText().ascii());
	if (!user) {
		log_file->write_report("Cannot find user profile.",
			       "MphoneForm::startMessageSession", 
			       LOG_NORMAL, LOG_CRITICAL);
		return;
	}
	
	im::t_msg_session *session = new im::t_msg_session(user);
	MEMMAN_NEW(session);
	((t_gui  *)ui)->addMessageSession(session);
	MessageFormView *messageFormView = new MessageFormView(NULL, session);
	MEMMAN_NEW(messageFormView);
	messageFormView->show();	
}

void MphoneForm::startMessageSession(t_buddy *buddy)
{
	t_user *user_config = buddy->get_user_profile();
	t_url dest_url(ui->expand_destination(user_config, buddy->get_sip_address()));
	if (!dest_url.is_valid()) return;
	string display = buddy->get_name();
	
	// Find an existing session
	im::t_msg_session *session = ((t_gui *)ui)->getMessageSession(user_config, dest_url, display);
	if (!session) {
		// There is no session yet, create one.
		session = new im::t_msg_session(user_config, t_display_url(dest_url, display));
		MEMMAN_NEW(session);
		((t_gui *)ui)->addMessageSession(session);
		MessageFormView *view = new MessageFormView(NULL, session);
		MEMMAN_NEW(view);
		view->show();
	}
}

void MphoneForm::phoneConfirmZrtpSas(int line)
{
	((t_gui *)ui)->action_confirm_zrtp_sas(line);
	updateState();
}

void MphoneForm::phoneConfirmZrtpSas()
{
	((t_gui *)ui)->action_confirm_zrtp_sas();
	updateState();
}

void MphoneForm::phoneResetZrtpSasConfirmation(int line)
{
	((t_gui *)ui)->action_reset_zrtp_sas_confirmation(line);
	updateState();	
}

void MphoneForm::phoneResetZrtpSasConfirmation()
{
	((t_gui *)ui)->action_reset_zrtp_sas_confirmation();
	updateState();	
}

void MphoneForm::phoneEnableZrtp(bool on)
{
	if (on) {
		((t_gui *)ui)->action_enable_zrtp();
	} else {
		((t_gui *)ui)->action_zrtp_request_go_clear();
	}
	
	updateState();
}

void MphoneForm::phoneZrtpGoClearOk(unsigned short line)
{
	((t_gui *)ui)->action_zrtp_go_clear_ok(line);
	updateState();
}

// Radio button for line 1 changed state
void MphoneForm::line1rbChangedState( bool on )
{
	// If the radio button is switched off, then return, the toggle
	// on the other line will handle the action
	if (!on) return;
	
	((t_gui *)ui)->action_activate_line(0);
}

void MphoneForm::line2rbChangedState( bool on )
{
	// If the radio button is switched off, then return, the toggle
	// on the other line will handle the action
	if (!on) return;
	
	((t_gui *)ui)->action_activate_line(1);
}

void MphoneForm::actionLine1Toggled( bool on)
{
	if (!on) return;
	((t_gui *)ui)->action_activate_line(0);
}

void MphoneForm::actionLine2Toggled( bool on)
{
	if (!on) return;
	((t_gui *)ui)->action_activate_line(1);
}

// Enable/disable dnd when there is 1 user active
void MphoneForm::srvDnd( bool on ) 
{
	((t_gui *)ui)->srv_dnd(phone->ref_users(), on);
	updateServicesStatus();
}

// Enable/disable dnd when there are multiple users active
void MphoneForm::srvDnd()
{
	if (selectUserForm) {
		MEMMAN_DELETE(selectUserForm);
		delete (selectUserForm);
	}
		
	selectUserForm = new SelectUserForm(this, "dnd", true);
	MEMMAN_NEW(selectUserForm);
		
	connect(selectUserForm, SIGNAL(selection(list<t_user *>)), this, 
			SLOT(do_srvDnd_enable(list<t_user *>)));
	connect(selectUserForm, SIGNAL(not_selected(list<t_user *>)), this, 
			SLOT(do_srvDnd_disable(list<t_user *>)));
			
	selectUserForm->show(SELECT_DND);
}

void MphoneForm::do_srvDnd_enable(list<t_user *> user_list) {
	((t_gui *)ui)->srv_dnd(user_list, true);
	updateServicesStatus();
}

void MphoneForm::do_srvDnd_disable(list<t_user *> user_list) {
	((t_gui *)ui)->srv_dnd(user_list, false);
	updateServicesStatus();
}

// Enable/disable auto answer when there is 1 user active
void MphoneForm::srvAutoAnswer( bool on ) 
{
	((t_gui *)ui)->srv_auto_answer(phone->ref_users(), on);
	updateServicesStatus();
}

// Enable/disable auto answer when there are multiple users active
void MphoneForm::srvAutoAnswer()
{
	if (selectUserForm) {
		MEMMAN_DELETE(selectUserForm);
		delete (selectUserForm);
	}
		
	selectUserForm = new SelectUserForm(this, "auto answer", true);
	MEMMAN_NEW(selectUserForm);
		
	connect(selectUserForm, SIGNAL(selection(list<t_user *>)), this, 
			SLOT(do_srvAutoAnswer_enable(list<t_user *>)));
	connect(selectUserForm, SIGNAL(not_selected(list<t_user *>)), this, 
			SLOT(do_srvAutoAnswer_disable(list<t_user *>)));
			
	selectUserForm->show(SELECT_AUTO_ANSWER);
}

void MphoneForm::do_srvAutoAnswer_enable(list<t_user *> user_list) {
	((t_gui *)ui)->srv_auto_answer(user_list, true);
	updateServicesStatus();
}

void MphoneForm::do_srvAutoAnswer_disable(list<t_user *> user_list) {
	((t_gui *)ui)->srv_auto_answer(user_list, false);
	updateServicesStatus();
}

void MphoneForm::srvRedirect()
{
	if (!srvRedirectForm) {
		srvRedirectForm = new SrvRedirectForm(this, "call redirection", true);
		MEMMAN_NEW(srvRedirectForm);
		connect(srvRedirectForm, 
			SIGNAL(destinations(t_user *,
					    const list<t_display_url> &,
					    const list<t_display_url> &,
					    const list<t_display_url> &)),
			this, 
			SLOT(do_srvRedirect(t_user *,
					    const list<t_display_url> &,
					    const list<t_display_url> &,
					    const list<t_display_url> &)));
	}
	
	srvRedirectForm->show();
}

void MphoneForm::do_srvRedirect(t_user *user_config,
				const list<t_display_url> &always, 
				const list<t_display_url> &busy,
				const list<t_display_url> &noanswer)
{
	// Redirection always
	if (always.empty()) {
		((t_gui *)ui)->srv_disable_cf(user_config, CF_ALWAYS);
	} else {
		((t_gui *)ui)->srv_enable_cf(user_config, CF_ALWAYS, always);
	}
	
	// Redirection busy
	if (busy.empty()) {
		((t_gui *)ui)->srv_disable_cf(user_config, CF_BUSY);
	} else {
		((t_gui *)ui)->srv_enable_cf(user_config, CF_BUSY, busy);
	}
	
	// Redirection no answer
	if (noanswer.empty()) {
		((t_gui *)ui)->srv_disable_cf(user_config, CF_NOANSWER);
	} else {
		((t_gui *)ui)->srv_enable_cf(user_config, CF_NOANSWER, noanswer);
	}
	
	updateServicesStatus();
}


void MphoneForm::about()
{
	QString s = sys_config->about(true).c_str();
	
	QMessageBox mbAbout(PRODUCT_NAME, s.replace(' ', "&nbsp;"), 
		    QMessageBox::Information, 
		    QMessageBox::Ok | QMessageBox::Default,
		    QMessageBox::NoButton, QMessageBox::NoButton);
	mbAbout.setIconPixmap(QPixmap::fromMimeSource("twinkle48.png"));
	mbAbout.exec();
}

void MphoneForm::aboutQt()
{
	QMessageBox::aboutQt(this, PRODUCT_NAME);
}

void MphoneForm::manual()
{
	((t_gui *)ui)->open_url_in_browser("http://www.twinklephone.com");
}

void MphoneForm::editUserProfile()
{
	if (!userProfileForm) {
		userProfileForm = new UserProfileForm(this, "user profile", true);
		MEMMAN_NEW(userProfileForm);
	
		connect(userProfileForm,
			SIGNAL(authCredentialsChanged(t_user *, const string&)),
			this,
			SLOT(updateAuthCache(t_user *, const string&)));
		
		connect(userProfileForm,
			SIGNAL(stunServerChanged(t_user *)),
			this,
			SLOT(updateStunSettings(t_user *)));
		
		// MWI settings change triggers an unsubscribe
		connect(userProfileForm,
			SIGNAL(mwiChangeUnsubscribe(t_user *)),
			this,
			SLOT(unsubscribeMWI(t_user *)));
		
		// MWI settings change triggers a subscribe
		connect(userProfileForm,
			SIGNAL(mwiChangeSubscribe(t_user *)),
			this,
			SLOT(subscribeMWI(t_user *)));
	}
	
	userProfileForm->show(phone->ref_users(), 
			      userComboBox->currentText());
}

void MphoneForm::editSysSettings()
{
	if (!sysSettingsForm) {
		sysSettingsForm = new SysSettingsForm(this, "system settings", true);
		MEMMAN_NEW(sysSettingsForm);
		connect(sysSettingsForm, SIGNAL(sipUdpPortChanged()),
			this, SLOT(updateSipUdpPort()));
		connect(sysSettingsForm, SIGNAL(rtpPortChanged()),
			this, SLOT(updateRtpPorts()));
	}
	
	sysSettingsForm->show();
}

void MphoneForm::selectProfile()
{
	if (!selectProfileForm) {
		selectProfileForm = new SelectProfileForm(this, "select profile", true);
		MEMMAN_NEW(selectProfileForm);
		connect(selectProfileForm, SIGNAL(selection(const list<string> &)),
			this, SLOT(newUsers(const list<string> &)));
		connect(selectProfileForm, SIGNAL(profileRenamed()),
			this, SLOT(updateUserComboBox()));
		connect(selectProfileForm, SIGNAL(profileRenamed()),
			this, SLOT(populateBuddyList()));
	}
	
	selectProfileForm->showForm(this);
}

// A new set of users has been selected.
// Remove users from the current user set that are not in the selection.
// Add users from the selection that are not in the current set of users.
void MphoneForm::newUsers(const list<string> &profiles)
{
	string error_msg;
	
	// NOTE: First users must be removed. It could be that a
	// user profile of an active was renamed. In this case, the user 
	// with the old profile name is first removed and then added again.
	
	list<t_user *> user_list = phone->ref_users();
	
	// Remove current users that are not selected anymore.
	for (list<t_user *>::iterator i = user_list.begin(); i != user_list.end(); i++) {
		if (std::find(profiles.begin(), profiles.end(), 
			      (*i)->get_filename().c_str()) == profiles.end())
		{
			// User is not selected anymore.
			// Unsubscribe MWI
			if (phone->is_mwi_subscribed(*i)) {
				phone->pub_unsubscribe_mwi(*i);
			}
			
			// Unpublish presence of user
			phone->pub_unpublish_presence(*i);
			
			// Unsubscribe presence
			phone->pub_unsubscribe_presence(*i);
			
			// Deregister user
			if (phone->get_is_registered(*i)) {
				phone->pub_registration(*i, REG_DEREGISTER);
			}
			
			log_file->write_header("MphoneForm::newUsers");
			log_file->write_raw("Stop user profile: ");
			log_file->write_raw((*i)->get_profile_name());
			log_file->write_endl();
			log_file->write_footer();
			phone->remove_phone_user(*(*i));
		}
	}
	
	// Determine which users to add
	list<string> add_profile_list;
	for (list<string>::const_iterator i = profiles.begin(); i != profiles.end(); i++) {
		QString profile = (*i).c_str();
		// Strip off the .cfg extension
		profile.truncate(profile.length() - 4);
		
		if (!phone->ref_user_profile(profile.ascii())) {
			add_profile_list.push_back(*i);
		}
	}
	
	// Add new phone users
	QProgressDialog progress(tr("Starting user profiles..."), "Abort", add_profile_list.size(), this,
				 "starting user profiles", true);
	progress.setCaption(PRODUCT_NAME);
	progress.setMinimumDuration(200);
	int progressStep = 0;
	for (list<string>::iterator i = add_profile_list.begin(); i != add_profile_list.end(); i++) {
		progress.setProgress(progressStep);
		qApp->processEvents();
		
		if (progress.wasCancelled()) {
			log_file->write_report("User aborted startup of new users.", 
					       "MphoneForm::newUsers");
			break;
		}
		
		t_user user_config;
		
		// Read user configuration
		if (user_config.read_config(*i, error_msg)) {
			t_user *dup_user;
			
			log_file->write_header("MphoneForm::newUsers");
			log_file->write_raw("Run user profile: ");
			log_file->write_raw(user_config.get_profile_name());
			log_file->write_endl();
			log_file->write_footer();
			
			if (phone->add_phone_user(user_config, &dup_user))
			{
				// NAT discovery
				if (!phone->stun_discover_nat(&user_config, error_msg)) 
				{
					// Warn user that the STUN settings will not work.
					((t_gui *)ui)->cb_show_msg(this, error_msg, 
							MSG_WARNING);
				}
				
				// Register at startup
				if (user_config.get_register_at_startup()) {
					phone->pub_registration(&user_config,
						REG_REGISTER,
						DUR_REGISTRATION(&user_config));
				} else {
					// No registration needed, initialize extensions now.
					phone->init_extensions(&user_config);
				}
				
				// Extension initialization will be done after 
				// registration succeeded.
			} else {
				error_msg = tr("The following profiles are both for user %1").arg(user_config.get_name().c_str()).ascii();
				error_msg += '@';
				error_msg += user_config.get_domain();
				error_msg += ":\n\n";
				error_msg += user_config.get_profile_name();
				error_msg += "\n";
				error_msg += dup_user->get_profile_name();
				error_msg += "\n\n";
				error_msg += tr("You can only run multiple profiles for different users.");
				
				log_file->write_report(error_msg,
					"MphoneForm::newUsers", 
					LOG_NORMAL, LOG_WARNING);
				ui->cb_display_msg(error_msg, MSG_WARNING);
			}
		} else {
			log_file->write_report(error_msg,
					"MphoneForm::newUsers", 
					LOG_NORMAL, LOG_CRITICAL);
			ui->cb_display_msg(error_msg, MSG_CRITICAL);
		}
		
		progressStep++;
	}
	progress.setProgress(add_profile_list.size());
	
	populateBuddyList();
	updateUserComboBox();
	updateRegStatus();
	updateMwi();
	updateServicesStatus();
	updateSysTrayStatus();
	updateMenuStatus();
	updateState();
	
	call_history->clear_num_missed_calls();
}

void MphoneForm::updateUserComboBox()
{
	QString current_user;
	
	if (userComboBox->count() == 0) {
		// The last used profile
		current_user = sys_config->get_last_used_profile().c_str();
	} else {
		// Keep the current active profile
		current_user = userComboBox->currentText();
	}
	
	((t_gui *)ui)->fill_user_combo(userComboBox);
	
	// If previous selected user is still active, make it the current user
	for (int i = 0; i < userComboBox->count(); i++) {
		if (userComboBox->text(i) == current_user) {
			userComboBox->setCurrentItem(i);
		}
	}
}

void MphoneForm::updateSipUdpPort()
{
	((t_gui *)ui)->cb_show_msg(sysSettingsForm,
			tr("You have changed the SIP UDP port. This setting will only become "\
			"active when you restart Twinkle.").ascii(),
			MSG_INFO);
}

void MphoneForm::updateRtpPorts()
{
	phone->init_rtp_ports();
}

void MphoneForm::updateStunSettings(t_user *user_config)
{
	string s;
	if (!phone->stun_discover_nat(user_config, s)) {
		// Warn user that the STUN settings will not work.
		((t_gui *)ui)->cb_show_msg(this, s, MSG_WARNING);
	}
	
	if (!user_config->get_use_stun()) {
		// Disable STUN
		phone->disable_stun(user_config);
	}
	
	// Synchronize the sending of NAT keep alives with the user profile settings.
	phone->sync_nat_keepalive(user_config);
}

void MphoneForm::updateAuthCache(t_user *user_config, const string &realm)
{
	phone->remove_cached_credentials(user_config, realm);
}

void MphoneForm::unsubscribeMWI(t_user *user_config)
{
	phone->pub_unsubscribe_mwi(user_config);
}

void MphoneForm::subscribeMWI(t_user *user_config)
{
	phone->pub_subscribe_mwi(user_config);
}

void MphoneForm::viewLog()
{
	if (!logViewForm) {
		logViewForm = new LogViewForm(NULL);
		MEMMAN_NEW(logViewForm);
	}
	
	logViewForm->show();
}

void MphoneForm::updateLog(bool log_zapped)
{
	if (logViewForm) logViewForm->update(log_zapped);
}

void MphoneForm::viewHistory()
{
	if (!historyForm) {
		historyForm = new HistoryForm(NULL);
		MEMMAN_NEW(historyForm);
	}
	
	connect(historyForm, 
		SIGNAL(call(t_user *, const QString &, const QString &, bool)), this,  
		SLOT(phoneInvite(t_user *, const QString &, const QString &, bool)));
	
	historyForm->show();
}

void MphoneForm::updateCallHistory()
{
	if (historyForm) historyForm->update();
}

t_twinkle_sys_tray *MphoneForm::getSysTray()
{
	return sysTray;
}

// Execute call directly from the main window (press call button)
void MphoneForm::quickCall()
{
	string display, dest_str;
	
	t_user *from_user = phone->ref_user_profile(
				userComboBox->currentText().ascii());
	if (!from_user) {
		log_file->write_report("Cannot find user profile.",
			       "MphoneForm::quickCall", 
			       LOG_NORMAL, LOG_CRITICAL);
		return;
	}
	
	ui->expand_destination(from_user, 
			       callComboBox->currentText().stripWhiteSpace().ascii(), 
			       display, dest_str);
	t_url dest(dest_str);
	
	if (dest.is_valid()) {
		QString destination = callComboBox->currentText();
		addToCallComboBox(destination);
		if (inviteForm) inviteForm->addToInviteComboBox(destination);
		callComboBox->setFocus();
		do_phoneInvite(from_user, display.c_str(), dest, "", false);
	}
}

// Add a destination to the list of callComboBox
void MphoneForm::addToCallComboBox(const QString &destination)
{
	// Remove duplicate entries
	for (int i = callComboBox->count() - 1; i >= 0; i--) {
		if (callComboBox->text(i) == destination) {
			callComboBox->removeItem(i);
		}
	}
	
	// Add entry
	callComboBox->insertItem(destination, 0);
	callComboBox->setCurrentItem(0);
	
	// Remove last entry is list exceeds maximum size
	if (callComboBox->count() > SIZE_REDIAL_LIST) {
		callComboBox->removeItem(callComboBox->count() - 1);
	}
	
	// Clearing the edit line must be done here as this function is
	// also called when a call is made through the inviteForm.
	// The insertItem puts the text also in the edit field. So it must
	// be cleared here.
	callComboBox->clearEdit();
}

void MphoneForm::showAddressBook()
{
	if (!getAddressForm) {
		getAddressForm = new GetAddressForm(
				this, "select address", true);
		MEMMAN_NEW(getAddressForm);
	}
	
	connect(getAddressForm, 
		SIGNAL(address(const QString &)),
		this, SLOT(selectedAddress(const QString &)));
	
	getAddressForm->show();
}

void MphoneForm::selectedAddress(const QString &address)
{
	callComboBox->setEditText(address);
}

// Enable/disable the various call widgets
void MphoneForm::enableCallOptions(bool enable)
{
	// Enable/disable widgets
	callInvite->setEnabled(enable);
	callPushButton->setEnabled(enable);
	callComboBox->setEnabled(enable);
	addressToolButton->setEnabled(enable);
	
	// Set focus on callComboBox
	if (enable) {
		callComboBox->setFocus();
	}
}

void MphoneForm::keyPressEvent(QKeyEvent *e)
{
	if (callPushButton->isEnabled()) {
		// Quick dial
		switch (e->key()) {
		case Qt::Key_Return:
		case Qt::Key_Enter:
			quickCall();
			break;
		default:
			e->ignore();
		}
	} else if (callDTMF->isEnabled()) {
		// DTMF keys
		switch (e->key()) {
		case Qt::Key_1:
			sendDTMF("1");
			break;
		case Qt::Key_2:
		case Qt::Key_A:
		case Qt::Key_B:
		case Qt::Key_C:
			sendDTMF("2");
			break;
		case Qt::Key_3:
		case Qt::Key_D:
		case Qt::Key_E:
		case Qt::Key_F:
			sendDTMF("3");
			break;
		case Qt::Key_4:
		case Qt::Key_G:
		case Qt::Key_H:
		case Qt::Key_I:
			sendDTMF("4");
			break;
		case Qt::Key_5:
		case Qt::Key_J:
		case Qt::Key_K:
		case Qt::Key_L:
			sendDTMF("5");
			break;
		case Qt::Key_6:
		case Qt::Key_M:
		case Qt::Key_N:
		case Qt::Key_O:
			sendDTMF("6");
			break;
		case Qt::Key_7:
		case Qt::Key_P:
		case Qt::Key_Q:
		case Qt::Key_R:
		case Qt::Key_S:
			sendDTMF("7");
			break;
		case Qt::Key_8:
		case Qt::Key_T:
		case Qt::Key_U:
		case Qt::Key_V:
			sendDTMF("8");
			break;
		case Qt::Key_9:
		case Qt::Key_W:
		case Qt::Key_X:
		case Qt::Key_Y:
		case Qt::Key_Z:
			sendDTMF("9");
			break;
		case Qt::Key_0:
		case Qt::Key_Space:
			sendDTMF("0");
			break;
		case Qt::Key_Asterisk:
			sendDTMF("*");
			break;
		case Qt::Key_NumberSign:
			sendDTMF("#");
			break;
		default:
			e->ignore();
		}
	} else {
		e->ignore();
	}
}

// QLabels do not have mouse click events. I want the status labels
// to be clickable however. Explicitly check here if a status label has
// been clicked.
void MphoneForm::mouseReleaseEvent(QMouseEvent *e)
{
	if (e->button() == Qt::LeftButton && e->type() == QEvent::MouseButtonRelease) {
		processLeftMouseButtonRelease(e);
	} else if (e->button() == Qt::RightButton && e->type() == QEvent::MouseButtonRelease) {
		processRightMouseButtonRelease(e);
	} else {
		e->ignore();
	}
}

void MphoneForm::processLeftMouseButtonRelease(QMouseEvent *e)
{
	if (statAaLabel->hasMouse()) {
		if (phone->ref_users().size() == 1) {
			bool enable = !serviceAutoAnswer->isOn();
			srvAutoAnswer(enable);
			serviceAutoAnswer->setOn(enable);
		} else {
			srvAutoAnswer();
		}
	} else if (statDndLabel->hasMouse()) {
		if (phone->ref_users().size() == 1) {
			bool enable = !serviceDnd->isOn();
			srvDnd(enable);
			serviceDnd->setOn(enable);
		} else {
			srvDnd();
		}
	} else if (statCfLabel->hasMouse()) {
		srvRedirect();
	} else if (statMWILabel->hasMouse()) {
		popupMenuVoiceMail(e->globalPos());
	} else if (statMissedLabel->hasMouse()) {
		// Open the history form, when the user clicks on the 
		// missed calls indication.
		viewHistory();
	} else if (statRegLabel->hasMouse()) {
		// Fetch registration status
		phoneShowRegistrations();
	} else if (crypt1Label->hasMouse()) {
		processCryptLabelClick(0);
	} else if (crypt2Label->hasMouse()) {
		processCryptLabelClick(1);
	} else {
		e->ignore();
	}
}

void MphoneForm::processRightMouseButtonRelease(QMouseEvent *e)
{
	e->ignore();
}

void MphoneForm::processCryptLabelClick(int line) 
{
	t_audio_session *as = phone->get_line(line)->get_audio_session();
	if (!as) return;
	
	if (as->get_zrtp_sas_confirmed()) {
		phoneResetZrtpSasConfirmation(line);
	} else {
		phoneConfirmZrtpSas(line);
	}
}

// Show popup menu to access voice mail
void MphoneForm::popupMenuVoiceMail(const QPoint &pos)
{
	QPopupMenu menu(this);
	QIconSet vmIcon(QPixmap::fromMimeSource("mwi_none16.png"));
	vmIcon.setPixmap(QPixmap::fromMimeSource("mwi_none16_dis.png"),
		       QIconSet::Automatic, QIconSet::Disabled);
	
	list<t_user *>user_list = phone->ref_users();
	map<int, t_user *> vm;
	for (list<t_user *>::iterator i = user_list.begin(); i != user_list.end(); ++i) {
		QString address = (*i)->get_mwi_vm_address().c_str();
		QString entry		= (*i)->get_profile_name().c_str();
		entry += " - ";		
		if (address.isEmpty()) {
			entry += tr("not provisioned");
		} else {
			entry += address;
		}		
		
		int id = menu.insertItem(vmIcon, entry);
		if (address.isEmpty()) {
			menu.setItemEnabled(id, false);
		}
		
		vm.insert(make_pair(id, *i));
	}
	
	int selected;
	
	// If multiple profiles are active, then show the popup menu.
	// If one profile is active, then call voice mail immediately.
	if (user_list.size() > 1) {
		selected = menu.exec(pos);
		if (selected == -1) return;
	} else {
		if (vm.begin()->second->get_mwi_vm_address().empty()) {
			ui->cb_show_msg(
				tr("You must provision your voice mail address in your "
				   "user profile, before you can access it.").ascii(), 
				MSG_INFO);
			return;
		}
		selected = vm.begin()->first;
	}
	
	// Call can only be made if line is idle
	int line = phone->get_active_line();
	if (phone->get_line_state(line) == LS_BUSY) {
		ui->cb_show_msg(tr("The line is busy. Cannot access voice mail.").ascii(), 
				MSG_WARNING);
		return;
	}
	
	t_user *selectedUser = vm[selected];
	string display, dest_str;
	ui->expand_destination(selectedUser, 
		       selectedUser->get_mwi_vm_address(), 
		       display, dest_str);
	t_url dest(dest_str);
	
	if (dest.is_valid()) {
		QString destination = selectedUser->get_mwi_vm_address().c_str();
		addToCallComboBox(destination);
		if (inviteForm) inviteForm->addToInviteComboBox(destination);
		callComboBox->setFocus();
		do_phoneInvite(selectedUser, display.c_str(), dest, "", false);
	} else {
		QString msg(tr("The voice mail address %1 is an invalid address. "
			       "Please provision a valid address in your user profile."));
		ui->cb_show_msg(msg.arg(selectedUser->get_mwi_vm_address().c_str()).ascii(),
				MSG_CRITICAL);
	}
}

void MphoneForm::popupMenuVoiceMail(void)
{
	popupMenuVoiceMail(QCursor::pos());
}

void MphoneForm::showDisplay(bool on)
{
	if (on) {
		displayGroupBox->show();
	} else {
		int hDisplay = displayGroupBox->height();
		displayGroupBox->hide();
		
		if (hDisplay < minimumHeight()) {
			setMinimumHeight(minimumHeight() - hDisplay);
		}
		resize(width(), minimumHeight());
	}
	
	viewDisplay = on;
	viewDisplayAction->setOn(on);
}

void MphoneForm::showBuddyList(bool on)
{
	if (on) {
		buddyListView->show();
	} else {
		buddyListView->hide();
	}
	
	viewBuddyList = on;
	viewBuddyListAction->setOn(on);
}

void MphoneForm::showCompactLineStatus(bool on)
{
	if (on) {
		int hLabels = fromhead1Label->height() +
			      tohead1Label->height() +
			      subjecthead1Label->height() +
			      fromhead2Label->height() +
			      tohead2Label->height() +
			      subjecthead2Label->height();
		
		fromhead1Label->hide();
		tohead1Label->hide();
		subjecthead1Label->hide();
		from1Label->hide();
		to1Label->hide();
		subject1Label->hide();
		photo1Label->hide();
		fromhead2Label->hide();
		tohead2Label->hide();
		subjecthead2Label->hide();
		from2Label->hide();
		to2Label->hide();
		subject2Label->hide();
		photo2Label->hide();
		
		if (hLabels < minimumHeight()) {
			setMinimumHeight(minimumHeight() - hLabels);
		}
		resize(width(), minimumHeight());
	} else {
		fromhead1Label->show();
		tohead1Label->show();
		subjecthead1Label->show();
		from1Label->show();
		to1Label->show();
		subject1Label->show();
		fromhead2Label->show();
		tohead2Label->show();
		subjecthead2Label->show();
		from2Label->show();
		to2Label->show();
		subject2Label->show();
	}
	
	viewCompactLineStatus = on;
	//viewCompactLineStatusAction->setOn(on);
}

bool MphoneForm::getViewDisplay()
{
	return viewDisplay;
}

bool MphoneForm::getViewBuddyList()
{
	return viewBuddyList;
}

bool MphoneForm::getViewCompactLineStatus()
{
	return viewCompactLineStatus;
}

void MphoneForm::populateBuddyList()
{
	buddyListView->clear();

	list<t_user *> user_list = phone->ref_users();
	for (list<t_user *>::iterator i = user_list.begin(); i != user_list.end(); ++i) {
		t_presence_epa *epa = phone->ref_presence_epa(*i);
		if (!epa) continue;
		
		BLViewUserItem *profileItem = new BLViewUserItem(buddyListView, epa);
		t_buddy_list *buddy_list = phone->ref_buddy_list(*i);
		
		list<t_buddy> *buddies = buddy_list->get_records();
		for (list<t_buddy>::iterator bit = buddies->begin(); bit != buddies->end(); ++bit) {
			QString name = bit->get_name().c_str();
			new BuddyListViewItem(profileItem, &(*bit));
		}
		
		profileItem->setOpen(true);
	}
}

void MphoneForm::showBuddyListPopupMenu(QListViewItem *item, const QPoint &pos)
{
	if (!item) return;
	
	BuddyListViewItem *buddyItem = dynamic_cast<BuddyListViewItem *>(item);
	if (buddyItem) {
		buddyPopupMenu->popup(pos);
	} else {
		buddyListPopupMenu->popup(pos);
	}
}

void MphoneForm::doCallBuddy()
{
	QListViewItem *qitem = buddyListView->currentItem();
	BuddyListViewItem *item = dynamic_cast<BuddyListViewItem *>(qitem);
	if (!item) return;
	
	t_buddy *buddy = item->get_buddy();
	t_user *user_config = buddy->get_user_profile();
	
	phoneInvite(user_config, buddy->get_sip_address().c_str(), "", false);
}

void MphoneForm::doMessageBuddy(QListViewItem *qitem)
{	
	BuddyListViewItem *item = dynamic_cast<BuddyListViewItem *>(qitem);
	if (!item) return;
	
	t_buddy *buddy = item->get_buddy();
	
	startMessageSession(buddy);
}

void MphoneForm::doMessageBuddy()
{
	QListViewItem *item = buddyListView->currentItem();
	doMessageBuddy(item);
}

void MphoneForm::doEditBuddy()
{
	QListViewItem *qitem = buddyListView->currentItem();
	BuddyListViewItem *item = dynamic_cast<BuddyListViewItem *>(qitem);
	if (!item) return;
	
	t_buddy *buddy = item->get_buddy();
	
	BuddyForm *form = new BuddyForm(this, "new_buddy", true, Qt::WDestructiveClose);
	// Do not call MEMMAN as this form will be deleted automatically.	
	form->showEdit(*buddy);
}

void MphoneForm::doDeleteBuddy()
{
	QListViewItem *qitem = buddyListView->currentItem();
	BuddyListViewItem *item = dynamic_cast<BuddyListViewItem *>(qitem);
	if (!item) return;
	
	t_buddy *buddy = item->get_buddy();
	t_buddy_list *buddy_list = buddy->get_buddy_list();
	
	// Delete the list item before deleting the buddy as
	// deleting the item will detach the item from the buddy.
	delete item;
		
	if (buddy->is_presence_terminated()) {
		buddy_list->del_buddy(*buddy);
	} else {
		buddy->unsubscribe_presence(true);
	}
		
	string err_msg;
	if (!buddy_list->save(err_msg)) {
		QString msg = tr("Failed to save buddy list: %1").arg(err_msg.c_str());
		((t_gui *)ui)->cb_show_msg(this, msg.ascii(), MSG_CRITICAL);
	}
}

void MphoneForm::doAddBuddy()
{
	QListViewItem *qitem = buddyListView->currentItem();
	BLViewUserItem *item = dynamic_cast<BLViewUserItem *>(qitem);
	if (!item) return;
	
	t_phone_user *pu = item->get_presence_epa()->get_phone_user();
	if (!pu) return;
	t_buddy_list *buddy_list = pu->get_buddy_list();
	if (!buddy_list) return;
	
	BuddyForm *form = new BuddyForm(this, "new_buddy", true, Qt::WDestructiveClose);
	// Do not call MEMMAN as this form will be deleted automatically.
	form->showNew(*buddy_list, item);
}

void MphoneForm::doAvailabilityOffline()
{
	QListViewItem *qitem = buddyListView->currentItem();
	BLViewUserItem *item = dynamic_cast<BLViewUserItem *>(qitem);
	if (!item) return;
	
	t_phone_user *pu = item->get_presence_epa()->get_phone_user();
	if (!pu) return;
	
	pu->publish_presence(t_presence_state::ST_BASIC_CLOSED);
}

void MphoneForm::doAvailabilityOnline()
{
	QListViewItem *qitem = buddyListView->currentItem();
	BLViewUserItem *item = dynamic_cast<BLViewUserItem *>(qitem);
	if (!item) return;
	
	t_phone_user *pu = item->get_presence_epa()->get_phone_user();
	if (!pu) return;
	
	pu->publish_presence(t_presence_state::ST_BASIC_OPEN);
}

void MphoneForm::DiamondcardSignUp()
{
	DiamondcardProfileForm *f = new DiamondcardProfileForm(this, "select profile", true, 
							       Qt::WDestructiveClose);
	
	connect(f, SIGNAL(newDiamondcardProfile(const QString&)),
			this, SLOT(newDiamondcardUser(const QString &)));
	
	f->show(NULL);
}

void MphoneForm::newDiamondcardUser(const QString &filename)
{
	list<string> profileFilenames;
	list<t_user *> users = phone->ref_users();
	
	for (list<t_user *>::const_iterator it = users.begin(); it != users.end(); ++it) {
		t_user *user = *it;
		profileFilenames.push_back(user->get_filename());
	}
	
	profileFilenames.push_back(filename.ascii());
	newUsers(profileFilenames);
}

void MphoneForm::DiamondcardAction(t_dc_action action, int userIdx)
{
	list<t_user *> diamondcard_users = diamondcard_get_users(phone);
	vector<t_user *> v(diamondcard_users.begin(), diamondcard_users.end());
	
	if (userIdx < 0 || (unsigned int)userIdx >= v.size()) return;
	
	t_user *user = v[userIdx];
	QString url(diamondcard_url(action, user->get_name(), user->get_auth_pass()).c_str());
	((t_gui *)ui)->open_url_in_browser(url);
}

void MphoneForm::DiamondcardRecharge(int userIdx)
{
	DiamondcardAction(DC_ACT_RECHARGE, userIdx);
}

void MphoneForm::DiamondcardBalanceHistory(int userIdx)
{
	DiamondcardAction(DC_ACT_BALANCE_HISTORY, userIdx);
}

void MphoneForm::DiamondcardCallHistory(int userIdx)
{
	DiamondcardAction(DC_ACT_CALL_HISTORY, userIdx);
}

void MphoneForm::DiamondcardAdminCenter(int userIdx)
{
	DiamondcardAction(DC_ACT_ADMIN_CENTER, userIdx);
}
