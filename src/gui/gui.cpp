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

#include <iostream>
#include <cstdlib>
#include <qapplication.h>

#ifdef HAVE_KDE
#include <kmimetype.h>
#include <krun.h>
#include <kservice.h>
#include <ktrader.h>
#endif

// This include is needed to avoid build error when building Twinkle
// without Qt. One of the other includes below seems to include qdir.h
// indirectly. But by that time it probably conflicts with a macro causing
// compilation errors reported in qdir.h Include qdir.h here avoids the
// conflict.
#include "qdir.h"

#include "gui.h"
#include "line.h"
#include "log.h"
#include "sys_settings.h"
#include "user.h"
#include "cmd_socket.h"
#include "audio/rtp_telephone_event.h"
#include "sockets/interfaces.h"
#include "threads/thread.h"
#include "audits/memman.h"
#include "authenticationform.h"
#include "mphoneform.h"
#include "selectnicform.h"
#include "selectprofileform.h"
#include "messageformview.h"
#include "twinklesystray.h"
#include "util.h"
#include "address_finder.h"
#include "yesnodialog.h"
#include "command_args.h"
#include "im/msg_session.h"

#include "qcombobox.h"
#include "qhbox.h"
#include "qlabel.h"
#include "qlayout.h"
#include "qlistbox.h"
#include "qmessagebox.h"
#include "qpixmap.h"
#include "qprocess.h"
#include "qpushbutton.h"
#include "qsize.h"
#include "qsizepolicy.h"
#include "qstring.h"
#include "qtextcodec.h"
#include "qtextedit.h"
#include "qtoolbar.h"
#include "qtooltip.h"
#include "qvbox.h"

extern string user_host;
extern pthread_t thread_id_main;

// External command arguments
extern t_command_args g_cmd_args;

QString str2html(const QString &s)
{
	QString result(s);
	
	result.replace('&', "&amp;");
	result.replace('<', "&lt;");
	result.replace('>', "&gt;");
	
	return result;
}

void setDisabledIcon(QAction *action, const QString &icon) {
	QIconSet i = action->iconSet();
	i.setPixmap(QPixmap::fromMimeSource(icon), 
		    QIconSet::Automatic, QIconSet::Disabled);
	action->setIconSet(i);
}

void setDisabledIcon(QToolButton *toolButton, const QString &icon) {
	QIconSet i = toolButton->iconSet();
	i.setPixmap(QPixmap::fromMimeSource(icon), 
		    QIconSet::Automatic, QIconSet::Disabled);
	toolButton->setIconSet(i);
}

/////////////////////////////////////////////////
// PRIVATE
/////////////////////////////////////////////////

void t_gui::setLineFields(int line) {
	if (line == 0) {
		fromLabel = mainWindow->from1Label;
		toLabel = mainWindow->to1Label;
		subjectLabel = mainWindow->subject1Label;
		codecLabel = mainWindow->codec1TextLabel;
		photoLabel = mainWindow->photo1Label;
	} else {
		fromLabel = mainWindow->from2Label;
		toLabel = mainWindow->to2Label;
		subjectLabel = mainWindow->subject2Label;
		codecLabel = mainWindow->codec2TextLabel;
		photoLabel = mainWindow->photo2Label;
	}
}

void t_gui::clearLineFields(int line) {
	if (line >= NUM_USER_LINES) return;
	
	setLineFields(line);
	fromLabel->clear();
	QToolTip::remove(fromLabel);
	toLabel->clear();
	QToolTip::remove(toLabel);
	subjectLabel->clear();
	QToolTip::remove(subjectLabel);
	codecLabel->clear();
	photoLabel->clear();
	photoLabel->hide();
}

void t_gui::displayTo(const QString &s) {
	toLabel->setText(s);
	toLabel->setCursorPosition(0);
	QToolTip::add(toLabel, s);
}

void t_gui::displayFrom(const QString &s) {
	fromLabel->setText(s);
	fromLabel->setCursorPosition(0);
	QToolTip::add(fromLabel, s);
}

void t_gui::displaySubject(const QString &s) {
	subjectLabel->setText(s);
	subjectLabel->setCursorPosition(0);
	QToolTip::add(subjectLabel, s);
}

void t_gui::displayCodecInfo(int line) {
	if (line > NUM_USER_LINES) return;
	
	setLineFields(line);
	codecLabel->clear();
	
	t_call_info call_info = phone->get_call_info(line);
	
	if (call_info.send_codec == CODEC_NULL && call_info.recv_codec == CODEC_NULL) {
		return;
	}
	
	if (call_info.send_codec == CODEC_NULL) {
		codecLabel->setText(format_codec(call_info.recv_codec).c_str());
		return;
	}
	
	if (call_info.recv_codec == CODEC_NULL) {
		codecLabel->setText(format_codec(call_info.send_codec).c_str());
		return;
	}
	
	if (call_info.send_codec == call_info.recv_codec) {
		codecLabel->setText(format_codec(call_info.send_codec).c_str());
		return;
	}
	
	QString s = format_codec(call_info.send_codec).c_str();
	s.append('/').append(format_codec(call_info.recv_codec).c_str());
	codecLabel->setText(s);
}

void t_gui::displayPhoto(const QImage &photo) {
	if (mainWindow->getViewCompactLineStatus()) {
		// In compact line status mode, no photo can be shown
		return;
	}
	
	if (photo.isNull()) {
		photoLabel->hide();
	} else {
		QPixmap pm;
		pm.convertFromImage(photo.smoothScale(
			photoLabel->width(), photoLabel->height(), QImage::ScaleMin));
		photoLabel->setPixmap(pm);
		photoLabel->show();
	}
}

/////////////////////////////////////////////////
// PROTECTED
/////////////////////////////////////////////////
bool t_gui::do_invite(const string &destination, const string &display, 
			const string &subject, bool immediate, bool anonymous)
{
	lock();
	if (mainWindow->callInvite->isEnabled()) {
		if (immediate) {
			t_user *user = phone->ref_user_profile(
				mainWindow->userComboBox->currentText().ascii());
			t_url dst_url(expand_destination(user, destination));
			if (dst_url.is_valid()) {
				mainWindow->do_phoneInvite(user, 
						display.c_str(), dst_url, subject.c_str(),
						anonymous);
			}
		} else {
			t_url dest_url(destination);
			t_display_url du(dest_url, display);
			mainWindow->phoneInvite(du.encode().c_str(), subject.c_str(),
						anonymous);
		}
	}
	unlock();
	
	return true;
}

void t_gui::do_redial(void) {
	lock();
	if (mainWindow->callRedial->isEnabled()) {
		mainWindow->phoneRedial();
	}
	unlock();
}

void t_gui::do_answer(void) {
	lock();
	if (mainWindow->callAnswer->isEnabled()) {
		mainWindow->phoneAnswer();
	}
	unlock();
}

void t_gui::do_answerbye(void) {
	lock();
	if (mainWindow->callAnswer->isEnabled()) {
		mainWindow->phoneAnswer();
	} else if (mainWindow->callBye->isEnabled()) {
		mainWindow->phoneBye();
	}
	unlock();
}

void t_gui::do_reject(void) {
	lock();
	if (mainWindow->callReject->isEnabled()) {
		mainWindow->phoneReject();
	}
	unlock();
}

void t_gui::do_redirect(bool show_status, bool type_present, t_cf_type cf_type, 
		bool action_present, bool enable, int num_redirections,
		const list<string> &dest_strlist, bool immediate)
{
	if (show_status) {
		// Show status not supported in GUI
		return;
	}
	
	t_user *user = phone->ref_user_profile(mainWindow->
				userComboBox->currentText().ascii());
	
	list<t_display_url> dest_list;
	for (list<string>::const_iterator i = dest_strlist.begin();
	     i != dest_strlist.end(); i++)
	{
		t_display_url du;
		du.url = expand_destination(user, *i);
		du.display.clear();
		if (!du.is_valid()) return;
		dest_list.push_back(du);
	}
	
	// Enable/disable permanent redirections
	if (type_present) {
		lock();
		if (enable) {
			phone->ref_service(user)->enable_cf(cf_type, dest_list);
		} else {
			phone->ref_service(user)->disable_cf(cf_type);
		}
		mainWindow->updateServicesStatus();
		unlock();
		
		return;
	} else {
		if (action_present) {
			if (!enable) {
				lock();
				phone->ref_service(user)->disable_cf(CF_ALWAYS);
				phone->ref_service(user)->disable_cf(CF_BUSY);
				phone->ref_service(user)->disable_cf(CF_NOANSWER);
				mainWindow->updateServicesStatus();
				unlock();
			}
			
			return;
		}
	}
	
	lock();
	if (mainWindow->callRedirect->isEnabled()) {
		if (immediate) {
			mainWindow->do_phoneRedirect(dest_list);
		} else {
			mainWindow->phoneRedirect(dest_strlist);
		}
	}
	unlock();
	
	return;
}

void t_gui::do_dnd(bool show_status, bool toggle, bool enable) {
	if (show_status) {
		// Show status not supported in GUI
		return;
	}
	
	lock();
	if (phone->ref_users().size() == 1) {
		if (toggle) {
			enable = !mainWindow->serviceDnd->isOn();
		}
		mainWindow->srvDnd(enable);
		mainWindow->serviceDnd->setOn(enable);
	} else {
		t_user *user = phone->ref_user_profile(mainWindow->
				userComboBox->currentText().ascii());
		list<t_user *> l;
		l.push_back(user);
		if (toggle) {
			enable = !phone->ref_service(user)->is_dnd_active();
		}
		
		if (enable) {
			mainWindow->do_srvDnd_enable(l);
		} else {
			mainWindow->do_srvDnd_disable(l);
		}
	}
	unlock();
}

void t_gui::do_auto_answer(bool show_status, bool toggle, bool enable) {
	if (show_status) {
		// Show status not supported in GUI
		return;
	}
	
	lock();
	if (phone->ref_users().size() == 1) {
		if (toggle) {
			enable = !mainWindow->serviceAutoAnswer->isOn();
		}
		mainWindow->srvAutoAnswer(enable);
		mainWindow->serviceAutoAnswer->setOn(enable);
	} else {
		t_user *user = phone->ref_user_profile(mainWindow->
				userComboBox->currentText().ascii());
		list<t_user *> l;
		l.push_back(user);
		if (toggle) {
			enable = !phone->ref_service(user)->
				 is_auto_answer_active();
		}

		if (enable) {
			mainWindow->do_srvAutoAnswer_enable(l);
		} else {
			mainWindow->do_srvAutoAnswer_disable(l);
		}
	}
	unlock();
}

void t_gui::do_bye(void) {
	lock();
	if (mainWindow->callBye->isEnabled()) {
		mainWindow->phoneBye();
	}
	unlock();
}

void t_gui::do_hold(void) {
	lock();
	if (mainWindow->callHold->isEnabled() && !mainWindow->callHold->isOn()) {
		mainWindow->phoneHold(true);
	}
	unlock();
}

void t_gui::do_retrieve(void) {
	lock();
	if (mainWindow->callHold->isEnabled() && mainWindow->callHold->isOn()) {
		mainWindow->phoneHold(false);
	}
	unlock();
}

bool t_gui::do_refer(const string &destination, t_transfer_type transfer_type, bool immediate) {
	lock();
	if (mainWindow->callTransfer->isEnabled() &&
	    !mainWindow->callTransfer->isOn()) 
	{
		if (immediate) {
			t_display_url du;
			t_user *user = phone->ref_user_profile(mainWindow->
				userComboBox->currentText().ascii());
			du.url = expand_destination(user, destination);
			
			if (du.is_valid() || transfer_type == TRANSFER_OTHER_LINE) {
				mainWindow->do_phoneTransfer(du, transfer_type);
			}
		} else {
			mainWindow->phoneTransfer(destination, transfer_type);
		}
	} else if(mainWindow->callTransfer->isEnabled() &&
		  mainWindow->callTransfer->isOn())
	{
		if (transfer_type != TRANSFER_CONSULT) {
			mainWindow->do_phoneTransferLine();
		}
	}
	unlock();
	
	return true;
}

void t_gui::do_conference(void) {
	lock();
	if (mainWindow->callConference->isEnabled()) {
		mainWindow->phoneConference();
	}
	unlock();
}

void t_gui::do_mute(bool show_status, bool toggle, bool enable) {
	if (show_status) {
		// Show status not supported in GUI
		return;
	}
	
	lock();
	if (mainWindow->callMute->isEnabled()) {
		if (toggle) enable = !phone->is_line_muted(phone->get_active_line());
		mainWindow->phoneMute(enable);
	}
	unlock();
}

void t_gui::do_dtmf(const string &digits) {
	lock();
	if (mainWindow->callDTMF->isEnabled()) {
		mainWindow->sendDTMF(digits.c_str());
	}
	unlock();
}

void t_gui::do_register(bool reg_all_profiles) {
	lock();
	list<t_user *> l;
	
	if (reg_all_profiles) {
		l = phone->ref_users();
	} else {
		t_user *user = phone->ref_user_profile(mainWindow->
			userComboBox->currentText().ascii());
		l.push_back(user);
	}
	
	mainWindow->do_phoneRegister(l);
	unlock();
}

void t_gui::do_deregister(bool dereg_all_profiles, bool dereg_all_devices) {
	lock();
	list<t_user *> l;
	
	if (dereg_all_profiles) {
		l = phone->ref_users();
	} else {
		t_user *user = phone->ref_user_profile(mainWindow->
			userComboBox->currentText().ascii());
		l.push_back(user);
	}
	
	if (dereg_all_devices) {
		mainWindow->do_phoneDeregisterAll(l);
	} else {
		mainWindow->do_phoneDeregister(l);
	}
	unlock();
}

void t_gui::do_fetch_registrations(void) {
	lock();
	mainWindow->phoneShowRegistrations();
	unlock();
}

bool t_gui::do_options(bool dest_set, const string &destination, bool immediate) {
	lock();
	// In-dialog OPTIONS request
	int line = phone->get_active_line();
	if (phone->get_line_substate(line) == LSSUB_ESTABLISHED) {
		((t_gui *)ui)->action_options();
		return true;
	}
	
	if (immediate) {
		t_user *user = phone->ref_user_profile(mainWindow->
			userComboBox->currentText().ascii());
		t_url dst_url(expand_destination(user, destination));
		
		if (dst_url.is_valid()) {
			mainWindow->do_phoneTermCap(user, dst_url);
		}
	} else {
		mainWindow->phoneTermCap(destination.c_str());
	}
	unlock();
	
	return true;
}

void t_gui::do_line(int line) {
	// Cannot get current line number via CLI interface on GUI.
	// So return in this case.
	if (line == 0) return;
	
	phone->pub_activate_line(line - 1);
}

void t_gui::do_user(const string &profile_name) {
	lock();
	for (int i = 0; i < mainWindow->userComboBox->count(); i++) {
		if (mainWindow->userComboBox->text(i) == profile_name.c_str()) 
		{
			mainWindow->userComboBox->setCurrentItem(i);
		}
	}
	unlock();
}

bool t_gui::do_message(const string &destination, const string &display,
		       const im::t_msg &msg)
{
	t_user *user = phone->ref_user_profile(mainWindow->
			userComboBox->currentText().ascii());
	
	t_url dest_url(expand_destination(user, destination));
	
	if (dest_url.is_valid()) {
		phone->pub_send_message(user, dest_url, display, msg);
	}
	
	return true;
}

void t_gui::do_presence(t_presence_state::t_basic_state basic_state) {
	t_user *user = phone->ref_user_profile(mainWindow->
			userComboBox->currentText().ascii());
	
	phone->pub_publish_presence(user, basic_state);
}

void t_gui::do_zrtp(t_zrtp_cmd zrtp_cmd) {
	lock();
	
	switch (zrtp_cmd) {
	case ZRTP_ENCRYPT:
		mainWindow->phoneEnableZrtp(true);
		break;
	case ZRTP_GO_CLEAR:
		mainWindow->phoneEnableZrtp(false);
		break;
	case ZRTP_CONFIRM_SAS:
		mainWindow->phoneConfirmZrtpSas();
		break;
	case ZRTP_RESET_SAS:
		mainWindow->phoneResetZrtpSasConfirmation();
		break;
	default:
		assert(false);
	}
	
	unlock();
}



void t_gui::do_quit(void) {
	lock();
	mainWindow->fileExit();
	unlock();
}

void t_gui::do_help(const list<t_command_arg> &al) {
	// Nothing to do in GUI mode
	return;
}


/////////////////////////////////////////////////
// PUBLIC
/////////////////////////////////////////////////

t_gui::t_gui(t_phone *_phone) : t_userintf(_phone), timerUpdateMessageSessions(NULL)
{
	use_stdout = false;
	lastFileBrowsePath = DIR_HOME;
	
	mainWindow = new MphoneForm(0, 0, Qt::WType_TopLevel | Qt::WStyle_ContextHelp);
#ifdef HAVE_KDE
	sys_tray_popup = NULL;
#endif
	
	for (int i = 0; i < NUM_USER_LINES; i++) {
		QObject::connect(&autoShowTimer[i], SIGNAL(timeout()),
			mainWindow, SLOT(show()));
	}
	
	MEMMAN_NEW(mainWindow);
	qApp->setMainWidget(mainWindow);
}

t_gui::~t_gui() {
	destroyAllMessageSessions();
	
	MEMMAN_DELETE(mainWindow);
	delete mainWindow;
}

void t_gui::run(void) {
	// Start asynchronous event processor
	thr_process_events = new t_thread(process_events_main, NULL);
	MEMMAN_NEW(thr_process_events);
	
	QString s;
	list<t_user *> user_list = phone->ref_users();
	
	// The Qt event loop is not running yet. Explicitly take the Qt lock
	// to avoid race conditions with other threads that may call GUI call
	// backs.
	// NOTE: the t_gui::lock() method cannot be used as this method
	// will not lock from the main thread and we are running in the
	// main thread (a bit of a kludge).
	qApp->lock();
	
	// Set configuration file name in titlebar
	s = PRODUCT_NAME;
	mainWindow->setCaption(s);
	
	// Set user combo box
	mainWindow->updateUserComboBox();
	
	// Display product information
	s = PRODUCT_NAME;
	s.append(' ').append(PRODUCT_VERSION).append(", ");
	s.append(sys_config->get_product_date().c_str());
	mainWindow->display(s);
	s = "Copyright (C) 2005-2009  ";
	s.append(PRODUCT_AUTHOR);
	mainWindow->display(s);
	
	// Restore user interface state from previous session
	restore_state();
	
	// Initialize phone functions
	phone->init();
	
	// Set controls in correct status
	mainWindow->updateState();
	mainWindow->updateRegStatus();
	mainWindow->updateMwi();
	mainWindow->updateServicesStatus();
	mainWindow->updateMissedCallStatus(0);
	mainWindow->updateMenuStatus();
	
	// Clear line field info fields
	clearLineFields(0);
	clearLineFields(1);
	
	// Populate buddy list
	mainWindow->populateBuddyList();
	
	// Set width of window to width of tool bar
	int widthToolBar = mainWindow->callToolbar->width();
	QSize sizeMainWin = mainWindow->size();
	sizeMainWin.setWidth(widthToolBar);
	mainWindow->resize(sizeMainWin);
	
	// Start QApplication/KApplication
	if (qApp->isSessionRestored() && 
	    sys_config->get_ui_session_id() == qApp->sessionId().ascii())
	{
		// Restore previous session
		restore_session_state();
	} else {
		if ((sys_config->get_start_hidden() && !g_cmd_args.cmd_show) ||
		    g_cmd_args.cmd_hide)
		{
			mainWindow->hide();
		} else {
			mainWindow->show();
		}
	}
	
	// Activate a profile if the --set-profile option was given on the command
	// line.
	if (!g_cmd_args.cmd_set_profile.isEmpty()) {
		cmdsocket::cmd_cli(string("user ") + 
				   g_cmd_args.cmd_set_profile.ascii(), true);
	}
	
	// Execute the call command if a callto destination was specified on the
	// command line
	if (!g_cmd_args.callto_destination.isEmpty()) {
		cmdsocket::cmd_call(g_cmd_args.callto_destination.ascii(), 
			 g_cmd_args.cmd_immediate_mode);
	}
	
	// Execute a CLI command if one was given on the command line
	if (!g_cmd_args.cli_command.isEmpty()) {
		cmdsocket::cmd_cli(g_cmd_args.cli_command.ascii(), 
			g_cmd_args.cmd_immediate_mode);
	}
	
	qApp->unlock();
	
	// Start Qt application
	qApp->exec();
	
	// Terminate phone functions
	phone->terminate();
	
	// Make user interface state persistent
	save_state();
}

void t_gui::save_state(void) {
	lock();
	
	sys_config->set_last_used_profile(
			mainWindow->userComboBox->currentText().ascii());

	list<string> history;
	for (int i = 0; i < mainWindow->callComboBox->count(); i++) {
		history.push_back(mainWindow->callComboBox->text(i).ascii());
	}
	sys_config->set_dial_history(history);
	
	sys_config->set_show_display(mainWindow->getViewDisplay());
	sys_config->set_compact_line_status(mainWindow->getViewCompactLineStatus());
	sys_config->set_show_buddy_list(mainWindow->getViewBuddyList());
	
	t_userintf::save_state();
	
	unlock();
}

void t_gui::restore_state(void) {
	lock();
	
	// The last used profile is selected when the userComboBox is
	// filled by MphoneForm::updateUserComboBox
	
	mainWindow->callComboBox->clear();
	list<string> dial_history = sys_config->get_dial_history();
	for (list<string>::reverse_iterator i = dial_history.rbegin(); i != dial_history.rend(); i++)
	{
		mainWindow->addToCallComboBox(i->c_str());
	}
	
	mainWindow->showDisplay(sys_config->get_show_display());
	mainWindow->showCompactLineStatus(sys_config->get_compact_line_status());
	mainWindow->showBuddyList(sys_config->get_show_buddy_list());
	
	t_userintf::restore_state();
	
	unlock();
}

void t_gui::save_session_state(void) {
	lock();
	
	t_win_geometry geometry(mainWindow->x(), mainWindow->y(),
				mainWindow->width(), mainWindow->height());
	sys_config->set_ui_session_main_geometry(geometry);
	sys_config->set_ui_session_main_state(mainWindow->windowState());
	sys_config->set_ui_session_main_hidden(mainWindow->isHidden());
	
	unlock();
}

void t_gui::restore_session_state(void) {
	lock();
	
	t_win_geometry geometry = sys_config->get_ui_session_main_geometry();
	mainWindow->setGeometry(geometry.x, geometry.y, geometry.width, geometry.height);
	mainWindow->setWindowState(sys_config->get_ui_session_main_state());
	mainWindow->setHidden(sys_config->get_ui_session_main_hidden());
	
	unlock();
}

void t_gui::lock(void) {
	// To synchronize actions on the Qt widget the application lock
	// is used. The main thread running the Qt event loop takes the
	// application lock itself already. So take the lock if this is not the
	// main thread.
	// If the Qt event loop has not been started yet, then the lock
	// should also be taken from the main thread.
	t_userintf::lock();
	if (!t_thread::is_self(thread_id_main)) {
		qApp->lock();
	}
}

void t_gui::unlock(void) {
	t_userintf::lock();
	if (!t_thread::is_self(thread_id_main)) {
		qApp->unlock();
	}
}

string t_gui::select_network_intf(void) {
	string ip;
	list<t_interface> *l = get_interfaces();
	// The socket routines are not under control of MEMMAN so report
	// the allocation here.
	MEMMAN_NEW(l);
	if (l->size() == 0) {
		cb_show_msg(qApp->translate("GUI",
			    "Cannot find a network interface. Twinkle will use "
			    "127.0.0.1 as the local IP address. When you connect to "
			    "the network you have to restart Twinkle to use the correct "
			    "IP address.").ascii(),
			    MSG_WARNING);
		
		MEMMAN_DELETE(l);
		delete l;
		return "127.0.0.1";
	}
	
	if (l->size() == 1) {
		// There is only 1 interface
		ip = l->front().get_ip_addr();
	} else {
		// There are multiple interfaces
		SelectNicForm *sf = new SelectNicForm(NULL, "nic", true);
		MEMMAN_NEW(sf);
		QString item;
		for (list<t_interface>::iterator i = l->begin(); i != l->end(); i++) {
			item = i->name.c_str();
			item.append(':').append(i->get_ip_addr().c_str());
			sf->nicListBox->insertItem(
					QPixmap::fromMimeSource("kcmpci16.png"), 
					item);
		}
		
		sf->nicListBox->setCurrentItem(0);
		sf->exec();
		
		int selection = sf->nicListBox->currentItem();
		int num = 0;
		for (list<t_interface>::iterator i = l->begin(); i != l->end(); i++) {
			if (num == selection) {
				ip = i->get_ip_addr();
				break;
			}
			num++;
		}
		
		MEMMAN_DELETE(sf);
		delete sf;		
	}
	
	MEMMAN_DELETE(l);
	delete l;
	return ip;
}

bool t_gui::select_user_config(list<string> &config_files) {
	SelectProfileForm f(0, "select user profile", true);
	
	if (f.execForm()) {
		config_files = f.selectedProfiles;
		return true;
	}
	
	return false;
}

// GUI call back functions

void t_gui::cb_incoming_call(t_user *user_config, int line, const t_request *r) {
	if (line >= NUM_USER_LINES) return;
	
	lock();
	QString s;
	
	setLineFields(line);
	
	// Incoming call for to-header
	mainWindow->displayHeader();
	s = qApp->translate("GUI", "Line %1: incoming call for %2").arg(line + 1).arg(
			format_sip_address(user_config, r->hdr_to.display, r->hdr_to.uri).c_str());
	mainWindow->display(s);
	
	// Is this a transferred call?
	QString referredByParty;
	if (r->hdr_referred_by.is_populated()) {
		referredByParty = format_sip_address(user_config, 
				r->hdr_referred_by.display, r->hdr_referred_by.uri).c_str();
		s = "Call transferred by ";
		s = qApp->translate("GUI", "Call transferred by %1").arg(referredByParty);
		mainWindow->display(s);
	}
	
	// From
	QString fromParty = format_sip_address(user_config,
				r->hdr_from.get_display_presentation(), r->hdr_from.uri).c_str();
	s = fromParty;
	QString organization("");
	if (r->hdr_organization.is_populated()) {
		organization = r->hdr_organization.name.c_str();
		s.append(", ").append(organization);
	}
	displayFrom(s);
	
	// Display photo
	QImage fromPhoto;
	
	if (sys_config->get_ab_lookup_photo()) {
		t_address_finder *af = t_address_finder::get_instance();
		fromPhoto = af->find_photo(user_config, r->hdr_from.uri);
	}
	
	displayPhoto(fromPhoto);
	
	// To
	s = "";
	s.append(format_sip_address(user_config, r->hdr_to.display, r->hdr_to.uri).c_str());
	displayTo(s);
	
	// Subject
	QString subject("");
	if (r->hdr_subject.is_populated()) {
		subject = r->hdr_subject.subject.c_str();
	}
	displaySubject(subject);
	
	cb_notify_call(line, fromParty, organization, fromPhoto, subject, referredByParty);
	
	unlock();
}

void t_gui::cb_call_cancelled(int line) {
	if (line >= NUM_USER_LINES) return;
	
	lock();
	QString s;
	
	mainWindow->displayHeader();
	s = qApp->translate("GUI", "Line %1: far end cancelled call.").arg(line + 1);
	mainWindow->display(s);
	
	cb_stop_call_notification(line);
	
	unlock();
}

void t_gui::cb_far_end_hung_up(int line) {
	if (line >= NUM_USER_LINES) return;
	
	lock();
	QString s;
	
	mainWindow->displayHeader();
	s = qApp->translate("GUI", "Line %1: far end released call.").arg(line + 1);
	mainWindow->display(s);

	cb_stop_call_notification(line);
	
	unlock();
}

void t_gui::cb_answer_timeout(int line) {
	if (line >= NUM_USER_LINES) return;
	
	lock();
	
	cb_stop_call_notification(line);
	
	unlock();
}

void t_gui::cb_sdp_answer_not_supported(int line, const string &reason) {
	if (line >= NUM_USER_LINES) return;
	
	lock();
	QString s;
	
	mainWindow->displayHeader();
	s = qApp->translate("GUI", "Line %1: SDP answer from far end not supported.").arg(line + 1);
	mainWindow->display(s);
	
	s = reason.c_str();
	mainWindow->display(s);
	
	cb_stop_call_notification(line);
	
	unlock();
}

void t_gui::cb_sdp_answer_missing(int line) {
	if (line >= NUM_USER_LINES) return;
	
	lock();
	QString s;

	
	mainWindow->displayHeader();
	s = qApp->translate("GUI", "Line %1: SDP answer from far end missing.").arg(line + 1);
	mainWindow->display(s);

	cb_stop_call_notification(line);
	
	unlock();
}

void t_gui::cb_unsupported_content_type(int line, const t_sip_message *r) {
	if (line >= NUM_USER_LINES) return;
	
	lock();
	QString s;
	
	mainWindow->displayHeader();
	s = qApp->translate("GUI", "Line %1: Unsupported content type in answer from far end.").arg(line + 1);
	mainWindow->display(s);
	
	s = r->hdr_content_type.media.type.c_str();
	s.append("/").append(r->hdr_content_type.media.subtype.c_str());
	mainWindow->display(s);
	
	cb_stop_call_notification(line);
	
	unlock();
}

void t_gui::cb_ack_timeout(int line) {
	if (line >= NUM_USER_LINES) return;
	
	lock();
	QString s;
	
	mainWindow->displayHeader();
	s = qApp->translate("GUI", "Line %1: no ACK received, call will be terminated.").arg(line + 1);
	mainWindow->display(s);

	cb_stop_call_notification(line);
	
	unlock();
}

void t_gui::cb_100rel_timeout(int line) {
	if (line >= NUM_USER_LINES) return;
	
	lock();
	QString s;
	
	mainWindow->displayHeader();
	s = qApp->translate("GUI", "Line %1: no PRACK received, call will be terminated.").arg(line + 1);
	mainWindow->display(s);

	cb_stop_call_notification(line);
	
	unlock();
}

void t_gui::cb_prack_failed(int line, const t_response *r) {
	if (line >= NUM_USER_LINES) return;
	
	lock();
	QString s;
	
	mainWindow->displayHeader();
	s = qApp->translate("GUI", "Line %1: PRACK failed.").arg(line + 1);
	mainWindow->display(s);
	
	s = QString().setNum(r->code);
	s.append(' ').append(r->reason.c_str());
	mainWindow->display(s);

	cb_stop_call_notification(line);
	
	unlock();
}

void t_gui::cb_provisional_resp_invite(int line, const t_response *r) {
	if (line >= NUM_USER_LINES) return;
	
	lock();
	mainWindow->updateState();
	unlock();
}

void t_gui::cb_cancel_failed(int line, const t_response *r) {
	if (line >= NUM_USER_LINES) return;
	
	lock();
	QString s;
	
	mainWindow->displayHeader();
	s = qApp->translate("GUI", "Line %1: failed to cancel call.").arg(line + 1);
	mainWindow->display(s);
	
	s = QString().setNum(r->code);
	s.append(' ').append(r->reason.c_str());
	mainWindow->display(s);
	
	unlock();
}

void t_gui::cb_call_answered(t_user *user_config, int line, const t_response *r) {
	if (line >= NUM_USER_LINES) return;
	
	lock();
	QString s;
	
	setLineFields(line);
	
	mainWindow->displayHeader();
	s = qApp->translate("GUI", "Line %1: far end answered call.").arg(line + 1);
	mainWindow->display(s);
	
	// Put far-end party in line to-field
	s = "";
	s.append(format_sip_address(user_config, r->hdr_to.display, r->hdr_to.uri).c_str());
	if (r->hdr_organization.is_populated()) {
		s.append(", ").append(r->hdr_organization.name.c_str());
	}
	displayTo(s);
	
	unlock();
}

void t_gui::cb_call_failed(t_user *user_config, int line, const t_response *r) {
	if (line >= NUM_USER_LINES) return;
	
	lock();
	QString s;
	
	mainWindow->displayHeader();
	s = qApp->translate("GUI", "Line %1: call failed.").arg(line + 1);
	mainWindow->display(s);
	
	s = QString().setNum(r->code);
	s.append(' ').append(r->reason.c_str());
	mainWindow->display(s);
	
	// Warnings
	if (r->hdr_warning.is_populated()) {
		list<string> l = format_warnings(r->hdr_warning);
		for (list<string>::iterator i = l.begin(); i != l.end(); i++) {
			mainWindow->display(i->c_str());
		}
	}
	
	// Redirect response
	if (r->get_class() == R_3XX && r->hdr_contact.is_populated()) {
		list<t_contact_param> l = r->hdr_contact.contact_list;
		l.sort();
		mainWindow->display(qApp->translate("GUI",
				"The call can be redirected to:"));
		for (list<t_contact_param>::iterator i = l.begin();
		i != l.end(); i++)
		{
			s = format_sip_address(user_config,
					i->display, i->uri).c_str();
			mainWindow->display(s);
		}
	}
	
	// Unsupported extensions
	if (r->code == R_420_BAD_EXTENSION) {
		mainWindow->display(r->hdr_unsupported.encode().c_str());
	}

	unlock();
}

void t_gui::cb_stun_failed_call_ended(int line) {
	if (line >= NUM_USER_LINES) return;
	
	lock();
	QString s;
	
	mainWindow->displayHeader();
	s = qApp->translate("GUI", "Line %1: call failed.").arg(line + 1);
	mainWindow->display(s);
	
	unlock();
}

void t_gui::cb_call_ended(int line) {
	if (line >= NUM_USER_LINES) return;
	
	lock();
	QString s;
	
	mainWindow->displayHeader();
	s = qApp->translate("GUI", "Line %1: call released.").arg(line + 1);
	mainWindow->display(s);

	unlock();
}

void t_gui::cb_call_established(int line) {
	if (line >= NUM_USER_LINES) return;
	
	lock();
	QString s;
	
	mainWindow->displayHeader();
	s = qApp->translate("GUI", "Line %1: call established.").arg(line + 1);
	mainWindow->display(s);
	
	unlock();
}

void t_gui::cb_options_response(const t_response *r) {
	lock();
	QString s;
	
	mainWindow->displayHeader();
	s = qApp->translate("GUI", "Response on terminal capability request: %1 %2")
	    .arg(r->code).arg(r->reason.c_str());
	mainWindow->display(s);
	
	if (r->code == R_408_REQUEST_TIMEOUT) {
		// The request timed out, so no capabilities are known.
		unlock();
		return;
	}
	
	s = qApp->translate("GUI", "Terminal capabilities of %1").arg(r->hdr_to.uri.encode().c_str());
	mainWindow->display(s);
	
	s = qApp->translate("GUI", "Accepted body types:").append(" ");
	if (r->hdr_accept.is_populated()) {
		s.append(r->hdr_accept.get_value().c_str());
	} else {
		s.append(qApp->translate("GUI", "unknown"));
	}
	mainWindow->display(s);
	
	s = qApp->translate("GUI", "Accepted encodings:").append(" ");
	if (r->hdr_accept_encoding.is_populated()) {
		s.append(r->hdr_accept_encoding.get_value().c_str());
	} else {
		s.append(qApp->translate("GUI", "unknown"));
	}
	mainWindow->display(s);
	
	s = qApp->translate("GUI", "Accepted languages:").append(" ");
	if (r->hdr_accept_language.is_populated()) {
		s.append(r->hdr_accept_language.get_value().c_str());
	} else {
		s.append(qApp->translate("GUI", "unknown"));
	}
	mainWindow->display(s);
	
	s = qApp->translate("GUI", "Allowed requests:").append(" ");
	if (r->hdr_allow.is_populated()) {
		s.append(r->hdr_allow.get_value().c_str());
	} else {
		s.append(qApp->translate("GUI", "unknown"));
	}
	mainWindow->display(s);
	
	s = qApp->translate("GUI", "Supported extensions:").append(" ");
	if (r->hdr_supported.is_populated()) {
		if (r->hdr_supported.features.empty()) {
			s.append(qApp->translate("GUI", "none"));
		} else {
			s.append(r->hdr_supported.get_value().c_str());
		}
	} else {
		s.append(qApp->translate("GUI", "unknown"));
	}
	mainWindow->display(s);
	
	s = qApp->translate("GUI", "End point type:").append(" ");
	if (r->hdr_server.is_populated()) {
		s.append(r->hdr_server.get_value().c_str());
	} else if (r->hdr_user_agent.is_populated()) {
		// Some end-points put a User-Agent header in the response
		// instead of a Server header.
		s.append(r->hdr_user_agent.get_value().c_str());
	} else {
		s.append(qApp->translate("GUI", "unknown"));
	}
	mainWindow->display(s);
	
	unlock();
}

void t_gui::cb_reinvite_success(int line, const t_response *r) {
	// Do not bother GUI user.
	return;
}

void t_gui::cb_reinvite_failed(int line, const t_response *r) {
	// Do not bother GUI user.
	return;
}

void t_gui::cb_retrieve_failed(int line, const t_response *r) {
	if (line >= NUM_USER_LINES) return;
	
	lock();
	QString s;
	
	mainWindow->displayHeader();
	s = qApp->translate("GUI", "Line %1: call retrieve failed.").arg(line + 1);
	mainWindow->display(s);
	
	s = QString().setNum(r->code);
	s.append(' ').append(r->reason.c_str());
	mainWindow->display(s);
	
	unlock();
}


void  t_gui::cb_invalid_reg_resp(t_user *user_config, const t_response *r, const string &reason) {
	lock();
	QString s;
	
	mainWindow->displayHeader();
	qApp->translate("GUI", "%1, registration failed: %2 %3")
			.arg(user_config->get_profile_name().c_str())
			.arg(r->code)
			.arg(r->reason.c_str());
	mainWindow->display(s);
	mainWindow->display(reason.c_str());
	
	mainWindow->updateRegStatus();
	unlock();
}

void t_gui::cb_register_success(t_user *user_config, const t_response *r, unsigned long expires,
				bool first_success) 
{
	lock();
	QString s;
	
	if (first_success) {
		mainWindow->displayHeader();
		s = qApp->translate("GUI", "%1, registration succeeded (expires = %2 seconds)")
		    .arg(user_config->get_profile_name().c_str())
		    .arg(expires);
		mainWindow->display(s);
	}
	
	mainWindow->updateRegStatus();
	unlock();
}

void t_gui::cb_register_failed(t_user *user_config, const t_response *r, bool first_failure) {
	lock();
	QString s;
	
	if (first_failure) {
		mainWindow->displayHeader();
		s = qApp->translate("GUI", "%1, registration failed: %2 %3")
		    .arg(user_config->get_profile_name().c_str())
		    .arg(r->code)
		    .arg(r->reason.c_str());
		mainWindow->display(s);
	}
	
	mainWindow->updateRegStatus();
	unlock();
}

void t_gui::cb_register_stun_failed(t_user *user_config, bool first_failure) {
	lock();
	QString s;
	
	if (first_failure) {
		mainWindow->displayHeader();
		s = qApp->translate("GUI", "%1, registration failed: STUN failure")
		    .arg(user_config->get_profile_name().c_str());
		mainWindow->display(s);
	}
	
	mainWindow->updateRegStatus();
	unlock();
}

void t_gui::cb_deregister_success(t_user *user_config, const t_response *r) {
	lock();
	QString s;
	
	mainWindow->displayHeader();
	s = qApp->translate("GUI", "%1, de-registration succeeded: %2 %3")
	    .arg(user_config->get_profile_name().c_str())
	    .arg(r->code)
	    .arg(r->reason.c_str());
	mainWindow->display(s);
	
	mainWindow->updateRegStatus();
	unlock();
}

void t_gui::cb_deregister_failed(t_user *user_config, const t_response *r) {
	lock();
	QString s;
	
	mainWindow->displayHeader();
	s = qApp->translate("GUI", "%1, de-registration failed: %2 %3")
	    .arg(user_config->get_profile_name().c_str())
	    .arg(r->code)
	    .arg(r->reason.c_str());
	mainWindow->display(s);
	
	mainWindow->updateRegStatus();
	unlock();
}

void t_gui::cb_fetch_reg_failed(t_user *user_config, const t_response *r) {
	lock();
	QString s;
	
	mainWindow->displayHeader();
	s = qApp->translate("GUI", "%1, fetching registrations failed: %2 %3")
	    .arg(user_config->get_profile_name().c_str())
	    .arg(r->code)
	    .arg(r->reason.c_str());
	mainWindow->display(s);
	
	unlock();
}

void t_gui::cb_fetch_reg_result(t_user *user_config, const t_response *r) {
	lock();
	QString s;
	
	mainWindow->displayHeader();
	
	s = user_config->get_profile_name().c_str();
	const list<t_contact_param> &l = r->hdr_contact.contact_list;
	if (l.size() == 0) {
		s += qApp->translate("GUI", ": you are not registered");
		mainWindow->display(s);
	} else {
		s += qApp->translate("GUI", ": you have the following registrations");
		mainWindow->display(s);
		for (list<t_contact_param>::const_iterator i = l.begin();
		i != l.end(); i++)
		{
			mainWindow->display(i->encode().c_str());
		}
	}
	
	unlock();
}

void t_gui::cb_register_inprog(t_user *user_config, t_register_type register_type) {
	QString s;
	
	lock();
	
	switch(register_type) {
	case REG_REGISTER:
		// Do not report registration refreshments
		if (phone->get_is_registered(user_config)) break;
		mainWindow->statRegLabel->setPixmap(
				QPixmap::fromMimeSource("gear.png"));
		break;
	case REG_DEREGISTER:
	case REG_DEREGISTER_ALL:
		mainWindow->statRegLabel->setPixmap(
				QPixmap::fromMimeSource("gear.png"));
		break;
	case REG_QUERY:
		mainWindow->displayHeader();
		s = user_config->get_profile_name().c_str();
		s += qApp->translate("GUI", ": fetching registrations...");
		mainWindow->display(s);
		break;
	}
	
	unlock();
}

void t_gui::cb_redirecting_request(t_user *user_config, int line, const t_contact_param &contact) {
	if (line >= NUM_USER_LINES) return;
	
	lock();
	QString s;
	
	mainWindow->displayHeader();
	s = qApp->translate("GUI", "Line %1: redirecting request to").arg(line + 1);
	mainWindow->display(s);
	
	s = format_sip_address(user_config, contact.display, contact.uri).c_str();
	mainWindow->display(s);
	
	unlock();
}

void t_gui::cb_redirecting_request(t_user *user_config, const t_contact_param &contact) {
	lock();
	QString s;
	
	mainWindow->displayHeader();
	s = qApp->translate("GUI", "Redirecting request to: %1").arg(
			format_sip_address(user_config, contact.display, contact.uri).c_str());
	mainWindow->display(s);
	
	unlock();
}

void t_gui::cb_notify_call(int line, const QString &from_party, const QString &organization,
			   const QImage &photo, const QString &subject, QString &referred_by_party)
{
	if (line >= NUM_USER_LINES) return;
	
	lock();
	
	// Play ringtone if the call is received on the active line.
	if (line == phone->get_active_line() &&
	    !phone->is_line_auto_answered(line))
	{
		cb_play_ringtone(line);
	}
	
	// Pop up sys tray balloon
#ifdef HAVE_KDE
	t_twinkle_sys_tray *tray = mainWindow->getSysTray();
	if (tray && !sys_tray_popup &&  !phone->is_line_auto_answered(line)) {
		QString presFromParty("");
		if (!from_party.isEmpty()) {
			presFromParty = dotted_truncate(from_party.ascii(), 50).c_str();
		}
		QString presOrganization("");
		if (!organization.isEmpty()) {
			presOrganization = dotted_truncate(organization.ascii(), 50).c_str();
		}
		QString presSubject("");
		if (!subject.isEmpty()) {
			presSubject = dotted_truncate(subject.ascii(), 50).c_str();
		}
		QString presReferredByParty("");
		if (!referred_by_party.isEmpty()) {
			presReferredByParty = qApp->translate("GUI", "Transferred by: %1").arg(
					dotted_truncate(referred_by_party.ascii(), 40).c_str());
		}
		
		// Create photo pixmap. If no photo is available, then use
		// the Twinkle icon.
		QPixmap pm;
		QFrame::Shape photoFrameShape = QFrame::NoFrame;
		if (photo.isNull()) {
			pm = QPixmap::fromMimeSource("twinkle32.png");
		} else {
			pm.convertFromImage(photo);
			photoFrameShape = QFrame::Box;
		}
		
		// Create the popup view.
		sys_tray_popup = new KPassivePopup(tray);
		MEMMAN_NEW(sys_tray_popup);
		sys_tray_popup->setAutoDelete(false);
		sys_tray_popup->setTimeout(0);
		QVBox *popup_view = new QVBox(sys_tray_popup);
		QHBox *hb = new QHBox(popup_view);
		hb->setSpacing(5);
		QLabel *lblPhoto = new QLabel(hb);
		lblPhoto->setPixmap(pm);
		lblPhoto->setFrameShape(photoFrameShape);
		QVBox *vb = new QVBox(hb);
		QString captionText("<H2>");
		captionText += qApp->translate("SysTrayPopup", "Incoming Call");
		captionText += "</H2>";
		QLabel *lblCaption = new QLabel(captionText, vb);
		lblCaption->setAlignment(Qt::AlignTop | Qt::AlignLeft);
		lblCaption->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
		QLabel *lblFrom = new QLabel(presFromParty, vb);
		lblFrom->setAlignment(Qt::AlignTop | Qt::AlignLeft);
		lblFrom->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
		QLabel *lastLabel = lblFrom;
		if (!presOrganization.isEmpty()) {
			QLabel *lblOrganization = new QLabel(presOrganization, vb);
			lblOrganization->setAlignment(Qt::AlignTop | Qt::AlignLeft);
			lblOrganization->setSizePolicy(QSizePolicy::Expanding, 
						       QSizePolicy::Minimum);
			lastLabel = lblOrganization;
		}
		if (!presReferredByParty.isEmpty()) {
			QLabel *lblReferredBy = new QLabel(presReferredByParty, vb);
			lblReferredBy->setAlignment(Qt::AlignTop | Qt::AlignLeft);
			lblReferredBy->setSizePolicy(QSizePolicy::Expanding, 
						       QSizePolicy::Minimum);
			lastLabel = lblReferredBy;
		}
		if (!presSubject.isEmpty()) {
			QLabel *lblSubject = new QLabel(presSubject, vb);
			lblSubject->setAlignment(Qt::AlignTop | Qt::AlignLeft);
			lblSubject->setSizePolicy(QSizePolicy::Expanding, 
						       QSizePolicy::Minimum);
			lastLabel = lblSubject;
		}
		lastLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		
		// Answer and reject buttons
		
		QHBox *buttonBox = new QHBox(vb);
		QIconSet iconAnswer(QPixmap::fromMimeSource("answer.png"));
		QPushButton *pbAnswer = new QPushButton(iconAnswer, 
			qApp->translate("SysTrayPopup", "Answer"), buttonBox);
		QObject::connect(pbAnswer, SIGNAL(clicked()), 
				 mainWindow, SLOT(phoneAnswerFromSystrayPopup()));
		QIconSet iconReject(QPixmap::fromMimeSource("reject.png"));
		QPushButton *pbReject = new QPushButton(iconReject, 
			qApp->translate("SysTrayPopup", "Reject"), buttonBox);
		QObject::connect(pbReject, SIGNAL(clicked()), 
				 mainWindow, SLOT(phoneRejectFromSystrayPopup()));
		
		sys_tray_popup->setView(popup_view);
		
		// Show the popup
		line_sys_tray_popup = line;
		sys_tray_popup->show();
		QObject::connect(sys_tray_popup, SIGNAL(clicked()),
			sys_tray_popup, SLOT(hide()));
	}
#endif
	
	// Show main window after a few seconds
	if (sys_config->get_gui_auto_show_incoming()) {
		autoShowTimer[line].start(
			sys_config->get_gui_auto_show_timeout() * 1000, true);
	}
	
	unlock();
}

void t_gui::cb_stop_call_notification(int line) {
	if (line >= NUM_USER_LINES) return;
	
	lock();
	cb_stop_tone(line);
	autoShowTimer[line].stop();
	
#ifdef HAVE_KDE
	if (sys_tray_popup && line_sys_tray_popup == line) {
		sys_tray_popup->hide();
		MEMMAN_DELETE(sys_tray_popup);
		delete sys_tray_popup;
		sys_tray_popup = NULL;
	}
#endif
	unlock();
}

void t_gui::cb_dtmf_detected(int line, char dtmf_event) {
	if (line >= NUM_USER_LINES) return;
	
	lock();
	QString s;
	
	mainWindow->displayHeader();
	s = qApp->translate("GUI", "Line %1: DTMF detected:").arg(line + 1).append(" ");
	
	if (VALID_DTMF_EV(dtmf_event)) {
		s.append(dtmf_ev2char(dtmf_event));
	} else {
		s.append(qApp->translate("GUI", "invalid DTMF telephone event (%1)").arg(
				(int)dtmf_event));
	}
	
	mainWindow->display(s);
	
	unlock();
}

void t_gui::cb_send_dtmf(int line, char dtmf_event) {
	if (line >= NUM_USER_LINES) return;
	
	lock();
	QString s;
	
	if (!VALID_DTMF_EV(dtmf_event)) return;
	
	mainWindow->displayHeader();
	s = qApp->translate("GUI", "Line %1: send DTMF %2").arg(line + 1).arg(
			dtmf_ev2char(dtmf_event));
	mainWindow->display(s);
	
	unlock();
}

void t_gui::cb_dtmf_not_supported(int line) {
	if (line >= NUM_USER_LINES) return;
	
	QString s;
	
	if (throttle_dtmf_not_supported) return;
	
	lock();
	
	mainWindow->displayHeader();
	s = qApp->translate("GUI", "Line %1: far end does not support DTMF telephone events.").arg(line + 1);
	mainWindow->display(s);
	
	// Throttle subsequent call backs
	throttle_dtmf_not_supported = true;
	
	unlock();
}

void t_gui::cb_dtmf_supported(int line) {
	if (line >= NUM_USER_LINES) return;
	
	lock();
	mainWindow->updateState();
	unlock();
}

void t_gui::cb_line_state_changed(void) {
	lock();
	mainWindow->updateState();
	unlock();
}

void t_gui::cb_send_codec_changed(int line, t_audio_codec codec) {
	if (line >= NUM_USER_LINES) return;
	
	lock();
	displayCodecInfo(line);
	unlock();
}

void t_gui::cb_recv_codec_changed(int line, t_audio_codec codec) {
	if (line >= NUM_USER_LINES) return;
	
	lock();
	displayCodecInfo(line);
	unlock();
}

void t_gui::cb_notify_recvd(int line, const t_request *r) {
	if (line >= NUM_USER_LINES) return;
	
	lock();
	QString s;
	
	mainWindow->displayHeader();
	s = qApp->translate("GUI", "Line %1: received notification.").arg(line+1);
	mainWindow->display(s);
	
	s = qApp->translate("GUI", "Event: %1").arg(r->hdr_event.event_type.c_str());
	mainWindow->display(s);
	
	s = qApp->translate("GUI", "State: %1").arg(r->hdr_subscription_state.substate.c_str());
	mainWindow->display(s);
	
	if (r->hdr_subscription_state.substate == SUBSTATE_TERMINATED) {
		s = qApp->translate("GUI", "Reason: %1").arg(r->hdr_subscription_state.reason.c_str());
		mainWindow->display(s);
	}
	
	t_response *sipfrag = (t_response *)((t_sip_body_sipfrag *)r->body)->sipfrag;
	s = qApp->translate("GUI", "Progress: %1 %2").arg(sipfrag->code).arg(sipfrag->reason.c_str());
	mainWindow->display(s);
	
	unlock();
}

void t_gui::cb_refer_failed(int line, const t_response *r) {
	if (line >= NUM_USER_LINES) return;
	
	lock();
	QString s;
	
	mainWindow->displayHeader();
	s = qApp->translate("GUI", "Line %1: call transfer failed.").arg(line + 1);
	mainWindow->display(s);
	
	s = QString().setNum(r->code);
	s.append(' ').append(r->reason.c_str());
	mainWindow->display(s);
	
	// The refer state has changed, so update the main window.
	mainWindow->updateState();
	
	unlock();
}

void t_gui::cb_refer_result_success(int line) {
	if (line >= NUM_USER_LINES) return;
	
	lock();
	QString s;
	
	mainWindow->displayHeader();
	s = qApp->translate("GUI", "Line %1: call succesfully transferred.").arg(line + 1);
	mainWindow->display(s);
	
	// The refer state has changed, so update the main window.
	mainWindow->updateState();
	
	unlock();
}

void t_gui::cb_refer_result_failed(int line) {
	if (line >= NUM_USER_LINES) return;
	
	lock();
	QString s;
	
	mainWindow->displayHeader();
	s = qApp->translate("GUI", "Line %1: call transfer failed.").arg(line + 1);
	mainWindow->display(s);
	
	// The refer state has changed, so update the main window.
	mainWindow->updateState();
	
	unlock();
}

void t_gui::cb_refer_result_inprog(int line) {
	if (line >= NUM_USER_LINES) return;
	
	lock();
	QString s;
	
	mainWindow->displayHeader();
	s = qApp->translate("GUI", "Line %1: call transfer still in progress.").arg(line + 1);
	mainWindow->display(s);
	
	s = qApp->translate("GUI", "No further notifications will be received.");
	mainWindow->display(s);
	
	// The refer state has changed, so update the main window.
	mainWindow->updateState();
	
	unlock();
}

void t_gui::cb_call_referred(t_user *user_config, int line, t_request *r) {
	if (line >= NUM_USER_LINES) return;
	
	QString s;
	
	lock();
	
	mainWindow->displayHeader();
	s = qApp->translate("GUI", "Line %1: transferring call to %2").arg(line +1).arg(
			format_sip_address(user_config,
			r->hdr_refer_to.display, r->hdr_refer_to.uri).c_str());
	mainWindow->display(s);
	
	if (r->hdr_referred_by.is_populated()) {
		s = qApp->translate("GUI", "Transfer requested by %1").arg(
				format_sip_address(user_config,
					    r->hdr_referred_by.display, 
					    r->hdr_referred_by.uri).c_str());
		mainWindow->display(s);
	}
	
	setLineFields(line);
	s = format_sip_address(user_config, 
			       user_config->get_display(false),  user_config->create_user_uri(false)).c_str();
	displayFrom(s);
	photoLabel->hide();
	
	s = format_sip_address(user_config,
			       r->hdr_refer_to.display, r->hdr_refer_to.uri).c_str();
	displayTo(s);
	
	subjectLabel->clear();
	codecLabel->clear();
	
	unlock();
}

void t_gui::cb_retrieve_referrer(t_user *user_config, int line) {
	if (line >= NUM_USER_LINES) return;
	
	QString s;
	
	lock();
	
	mainWindow->displayHeader();
	s = qApp->translate("GUI", "Line %1: Call transfer failed. Retrieving original call.").arg(line + 1);
	mainWindow->display(s);
	
	setLineFields(line);
	const t_call_info call_info = phone->get_call_info(line);
	
	s = format_sip_address(user_config, call_info.get_from_display_presentation(), 
			       call_info.from_uri).c_str();
	if (!call_info.from_organization.empty()) {
		s += ", ";
		s += call_info.from_organization.c_str();
	}
	displayFrom(s);
	
	// Display photo
	QImage fromPhoto;
	
	if (sys_config->get_ab_lookup_photo()) {
		t_address_finder *af = t_address_finder::get_instance();
		fromPhoto = af->find_photo(user_config, call_info.from_uri);
	}
	
	displayPhoto(fromPhoto);	
	
	s = format_sip_address(user_config, call_info.to_display, call_info.to_uri).c_str();
	if (!call_info.to_organization.empty()) {
		s += ", ";
		s += call_info.to_organization.c_str();
	}
	displayTo(s);
	
	displaySubject(call_info.subject.c_str());
	codecLabel->clear();
	
	unlock();
}

void t_gui::cb_consultation_call_setup(t_user *user_config, int line) {
	if (line >= NUM_USER_LINES) return;
	
	QString s;
	
	lock();
	
	setLineFields(line);
	const t_call_info call_info = phone->get_call_info(line);
	
	s = format_sip_address(user_config, call_info.get_from_display_presentation(), 
			       call_info.from_uri).c_str();
	if (!call_info.from_organization.empty()) {
		s += ", ";
		s += call_info.from_organization.c_str();
	}
	displayFrom(s);
	
	photoLabel->hide();
	
	s = format_sip_address(user_config, call_info.to_display, call_info.to_uri).c_str();
	if (!call_info.to_organization.empty()) {
		s += ", ";
		s += call_info.to_organization.c_str();
	}
	displayTo(s);
	
	displaySubject(call_info.subject.c_str());
	codecLabel->clear();
	
	unlock();
}

void t_gui::cb_stun_failed(t_user *user_config, int err_code, const string &err_reason) {
	lock();
	QString s;
	
	mainWindow->displayHeader();
	s = qApp->translate("GUI", "%1, STUN request failed: %2 %3")
	    .arg(user_config->get_profile_name().c_str())
	    .arg(err_code).arg(err_reason.c_str());
	mainWindow->display(s);

	unlock();
}

void t_gui::cb_stun_failed(t_user *user_config) {
	lock();
	QString s;
	
	mainWindow->displayHeader();
	s = qApp->translate("GUI", "%1, STUN request failed.")
	    .arg(user_config->get_profile_name().c_str());
	mainWindow->display(s);

	unlock();
}

bool t_gui::cb_ask_user_to_redirect_invite(t_user *user_config, const t_url &destination,
					   const string &display)
{
	QString s;
	QString title;
	
	lock();
	
	title = PRODUCT_NAME;
	title.append(" - ").append(qApp->translate("GUI", "Redirecting call"));
	
	s = qApp->translate("GUI", "User profile:").append(" <b>");
	s.append(user_config->get_profile_name().c_str());
	s.append("</b><br>").append(qApp->translate("GUI", "User:")).append(" <b>");
	s.append(str2html(user_config->get_display_uri().c_str()));
	s.append("</b><br><br>");
	
	s.append(qApp->translate("GUI", "Do you allow the call to be redirected to the following destination?"));
	s.append("<br><br>");
	s.append(str2html(ui->format_sip_address(user_config, display, destination).c_str()));
	s.append("<br><br>");
	s.append(qApp->translate("GUI", 
		"If you don't want to be asked this anymore, then you must change "
		"the settings in the SIP protocol section of the user profile."));
	QMessageBox *mb = new QMessageBox(title, s,
					  QMessageBox::Warning,
					  QMessageBox::Yes,
					  QMessageBox::No,
					  QMessageBox::NoButton,
					  mainWindow);
	MEMMAN_NEW(mb);
	bool permission = (mb->exec() == QMessageBox::Yes);
	MEMMAN_DELETE(mb);
	delete mb;
	
	unlock();
	
	return permission;
}

bool t_gui::cb_ask_user_to_redirect_request(t_user *user_config,
					    const t_url &destination,
					    const string &display, t_method method)
{
	QString s;
	QString title;
	
	lock();
	
	title = PRODUCT_NAME;
	title.append(" - ").append(qApp->translate("GUI", "Redirecting request"));
	
	s = qApp->translate("GUI", "User profile:").append(" <b>");
	s.append(user_config->get_profile_name().c_str());
	s.append("</b><br>").append(qApp->translate("GUI", "User:")).append(" <b>");
	s.append(str2html(user_config->get_display_uri().c_str()));
	s.append("</b><br><br>");
	
	s.append(qApp->translate("GUI", 
		"Do you allow the %1 request to be redirected to the following destination?").arg(
		method2str(method).c_str()));
	s.append("<br><br>");
	s.append(str2html(ui->format_sip_address(user_config, display, destination).c_str()));
	s.append("<br><br>");
	s.append(qApp->translate("GUI",
		"If you don't want to be asked this anymore, then you must change "
		"the settings in the SIP protocol section of the user profile."));
	QMessageBox *mb = new QMessageBox(title, s,
					  QMessageBox::Warning,
					  QMessageBox::Yes,
					  QMessageBox::No,
					  QMessageBox::NoButton,
					  mainWindow);
	MEMMAN_NEW(mb);
	bool permission = (mb->exec() == QMessageBox::Yes);
	MEMMAN_DELETE(mb);
	delete mb;
	
	unlock();
	
	return permission;
}

bool t_gui::cb_ask_credentials(t_user *user_config, const string &realm, string &username,
			       string &password)
{
	QString user(username.c_str());
	QString passwd(password.c_str());
	
	lock();
	
	AuthenticationForm *af = new AuthenticationForm(mainWindow, "authentication",
							true);
	MEMMAN_NEW(af);
	if (!af->exec(user_config, QString(realm.c_str()), user, passwd)) {
		MEMMAN_DELETE(af);
		delete af;
		unlock();
		return false;
	}
	
	username = user.ascii();
	password = passwd.ascii();
	MEMMAN_DELETE(af);
	delete af;
	
	unlock();
	return true;
}

void t_gui::cb_ask_user_to_refer(t_user *user_config, const t_url &refer_to_uri,
				 const string &refer_to_display,
				 const t_url &referred_by_uri,
				 const string &referred_by_display)
{
	QString s;
	QString title;
	
	lock();
	
	title = PRODUCT_NAME;
	title.append(" - ").append(qApp->translate("GUI", "Transferring call"));
	
	s = qApp->translate("GUI", "User profile:").append(" <b>");
	s.append(user_config->get_profile_name().c_str());
	s.append("</b><br>").append(qApp->translate("GUI", "User:")).append(" <b>");
	s.append(str2html(user_config->get_display_uri().c_str()));
	s.append("</b><br><br>");
	
	if (referred_by_uri.is_valid()) {
		s.append(qApp->translate("GUI","Request to transfer call received from:"));
		s.append("<br>");
		s.append(str2html(format_sip_address(user_config, referred_by_display,
					    referred_by_uri).c_str()));
		s.append("<br>");
	} else {
		s.append(qApp->translate("GUI", "Request to transfer call received."));
		s.append("<br>");
	}
	s.append("<br>");
	
	s.append(qApp->translate("GUI",
		"Do you allow the call to be transferred to the following destination?"));
	s.append("<br><br>");
	s.append(str2html(ui->format_sip_address(user_config, refer_to_display, 
					refer_to_uri).c_str()));
	s.append("<br><br>");
	s.append(qApp->translate("GUI",
		"If you don't want to be asked this anymore, then you must change "
		"the settings in the SIP protocol section of the user profile."));
	
	ReferPermissionDialog *dialog = new ReferPermissionDialog(mainWindow, title, s);
	// Do not report to MEMMAN as Qt will auto destruct this dialog on close.
	dialog->show();
	
	unlock();
}

void t_gui::cb_show_msg(const string &msg, t_msg_priority prio) {
	cb_show_msg(NULL, msg, prio);
}

void t_gui::cb_show_msg(QWidget *parent, const string &msg, t_msg_priority prio) {
	lock();
	
	switch (prio) {
	case MSG_INFO:
		QMessageBox::information(parent, PRODUCT_NAME, msg.c_str());
		break;
	case MSG_WARNING:
		QMessageBox::warning(parent, PRODUCT_NAME, msg.c_str());
		break;
	case MSG_CRITICAL:
	default:
		QMessageBox::critical(parent, PRODUCT_NAME, msg.c_str());
		break;
	}
	
	unlock();
}

bool t_gui::cb_ask_msg(const string &msg, t_msg_priority prio) {
	return cb_ask_msg(NULL, msg, prio);
}

bool t_gui::cb_ask_msg(QWidget *parent, const string &msg, t_msg_priority prio) {
	lock();
	
	int button = QMessageBox::No;
	switch (prio) {
	case MSG_INFO:
		button = QMessageBox::information(parent, PRODUCT_NAME, msg.c_str(),
			QMessageBox::Yes,
			QMessageBox::No | QMessageBox::Escape | QMessageBox::Default);
		break;
	case MSG_WARNING:
		button = QMessageBox::warning(parent, PRODUCT_NAME, msg.c_str(),
			QMessageBox::Yes,
			QMessageBox::No | QMessageBox::Escape | QMessageBox::Default);
		break;
	case MSG_CRITICAL:
	default:
		button = QMessageBox::critical(parent, PRODUCT_NAME, msg.c_str(),
			QMessageBox::Yes,
			QMessageBox::No | QMessageBox::Escape | QMessageBox::Default);
		break;
	}
	
	unlock();
	
	return (button == QMessageBox::Yes);
}

void t_gui::cb_display_msg(const string &msg, t_msg_priority prio) {
	// If this thread may not lock the UI, then push the display message on
	// the UI event queue. The message will be display asynchronously.
	if (is_prohibited_thread()) {
		cb_async_display_msg(msg, prio);
		return;
	}
	
	QString s;
	
	lock();
	
	switch (prio) {
	case MSG_NO_PRIO:
		break;
	case MSG_INFO:
		s = qApp->translate("GUI", "Info:");
		break;
	case MSG_WARNING:
		s = qApp->translate("GUI", "Warning:");
		break;
	case MSG_CRITICAL:
	default:
		s = qApp->translate("GUI", "Critical:");
		break;
	}	
	
	if (prio == MSG_NO_PRIO) {
		s = msg.c_str();
	} else {
		s.append(" ").append(msg.c_str());
	}
	mainWindow->displayHeader();
	mainWindow->display(s);
	
	unlock();
}

void t_gui::cb_log_updated(bool log_zapped) {
	lock();
	mainWindow->updateLog(log_zapped);
	unlock();
}

void t_gui::cb_call_history_updated(void) {
	lock();
	mainWindow->updateCallHistory();
	unlock();
}

void  t_gui::cb_missed_call(int num_missed_calls) {
	lock();
	mainWindow->updateMissedCallStatus(num_missed_calls);
	unlock();
}

void t_gui::cb_nat_discovery_progress_start(int num_steps) {
	natDiscoveryProgressDialog = new QProgressDialog(
			qApp->translate("GUI", "Firewall / NAT discovery..."), 
			qApp->translate("GUI", "Abort"), 
			num_steps, NULL,
			"nat discovery progress", true);
	MEMMAN_NEW(natDiscoveryProgressDialog);
	natDiscoveryProgressDialog->setCaption(PRODUCT_NAME);
	natDiscoveryProgressDialog->setMinimumDuration(200);
}

void t_gui::cb_nat_discovery_progress_step(int step) {
	natDiscoveryProgressDialog->setProgress(step);
	qApp->processEvents();
}

void t_gui::cb_nat_discovery_finished(void) {
	MEMMAN_DELETE(natDiscoveryProgressDialog);
	delete natDiscoveryProgressDialog;
}

bool t_gui::cb_nat_discovery_cancelled(void) {
	return natDiscoveryProgressDialog->wasCancelled();
}

void t_gui::cb_line_encrypted(int line, bool encrypted, const string &cipher_mode) {
	// Nothing todo in GUI
	// Encryption state is shown by the line state updata methods on
	// MphoneForm
}

void t_gui::cb_show_zrtp_sas(int line, const string &sas) {
		if (line >= NUM_USER_LINES) return;
	
	lock();
	QString s;
	
	setLineFields(line);
	
	mainWindow->displayHeader();
	s = qApp->translate("GUI", "Line %1").arg(line + 1);
	s.append(": SAS = ").append(sas.c_str());
	mainWindow->display(s);
	s = qApp->translate("GUI", "Click the padlock to confirm a correct SAS.");
	mainWindow->display(s);
	
	unlock();
}

void t_gui::cb_zrtp_confirm_go_clear(int line) {
	t_user *user_config = phone->get_line_user(line);
	if (!user_config) return;
	
	QString msg(qApp->translate("GUI", "The remote user on line %1 disabled the encryption.")
			.arg(line + 1));
	if (user_config->get_zrtp_goclear_warning()) {
		cb_show_msg(msg.ascii(), MSG_WARNING);
	} else {
		cb_display_msg(msg.ascii(), MSG_WARNING);
	}
	
	action_zrtp_go_clear_ok(line);
}

void t_gui::cb_zrtp_sas_confirmed(int line) {
	lock();
	QString s;
	
	setLineFields(line);
	
	mainWindow->displayHeader();
	s = qApp->translate("GUI", "Line %1: SAS confirmed.").arg(line + 1);
	mainWindow->display(s);
	
	unlock();
}

void t_gui::cb_zrtp_sas_confirmation_reset(int line) {
	lock();
	QString s;
	
	setLineFields(line);
	
	mainWindow->displayHeader();
	s = qApp->translate("GUI", "Line %1: SAS confirmation reset.").arg(line + 1);
	mainWindow->display(s);
	
	unlock();
}

void t_gui::cb_update_mwi(void) {
	lock();
	mainWindow->updateMwi();
	unlock();
}

void t_gui::cb_mwi_subscribe_failed(t_user *user_config, t_response *r, bool first_failure) {
	lock();
	QString s;
	
	if (first_failure) {
		mainWindow->displayHeader();
		s = qApp->translate("GUI", "%1, voice mail status failure.")
		    .arg(user_config->get_profile_name().c_str());
		mainWindow->display(s);
	}
	
	unlock();
}

void t_gui::cb_mwi_terminated(t_user *user_config, const string &reason) {
	lock();
	QString s;
	
	if (reason == "EV_REASON_REJECTED") {
		s = qApp->translate("GUI", "%1, voice mail status rejected.");
	} else if (reason == "EV_REASON_NORESOURCE") {
		s = qApp->translate("GUI", "%1, voice mailbox does not exist.");
	} else {
		s = qApp->translate("GUI", "%1, voice mail status terminated.");
	}

	mainWindow->displayHeader();
	mainWindow->display(s.arg(user_config->get_profile_name().c_str()));

	unlock();
}

bool t_gui::cb_message_request(t_user *user_config, t_request *r) {	
	string text;
	im::t_text_format text_format = im::TXT_PLAIN;
	bool attachment = false;
	bool failed_to_save_attachment = false;
	string attachment_error_msg;
	string attachment_saved_name;
	string attachment_save_as_name;
	
	if (!r->body) {
		log_file->write_report("Missing body.", "t_gui::cb_message_request",
				       LOG_NORMAL, LOG_DEBUG);
		return true;
	}
	
	// Determine if message should be shown inline or as an attachment
	if ((r->hdr_content_disp.is_populated() && r->hdr_content_disp.type == "attachment") ||
	    (r->body->get_type() == BODY_OPAQUE) ||
	    (r->hdr_content_length.is_populated() && r->hdr_content_length.length > im::MAX_INLINE_TEXT_LEN) ||
	    (r->body->encode().size() > im::MAX_INLINE_TEXT_LEN))
	{
		attachment = true;
		
		string suggested_file_extension = mime2file_extension(r->hdr_content_type.media);
		
		if (!sys_config->save_sip_body(*r, suggested_file_extension,
					       attachment_saved_name, 
					       attachment_save_as_name, 
					       attachment_error_msg)) 
		{
			failed_to_save_attachment = true;
		}
	} else if (r->body->get_type() == BODY_PLAIN_TEXT) {
		t_sip_body_plain_text *sb = dynamic_cast<t_sip_body_plain_text *>(r->body);
		text = sb->text;
		text_format = im::TXT_PLAIN;
	} else if (r->body->get_type() == BODY_HTML_TEXT) {
		t_sip_body_html_text *sb = dynamic_cast<t_sip_body_html_text *>(r->body);
		text = sb->text;
		text_format = im::TXT_HTML;
	} else {
		log_file->write_header("t_gui::cb_message_request",
			LOG_NORMAL, LOG_CRITICAL);
		log_file->write_raw("Unsupported content type: ");
		log_file->write_raw(r->body->get_type());
		log_file->write_endl();
		log_file->write_footer();
		return true;
	}
	
	if (!text.empty()) {
		string charset = r->hdr_content_type.media.charset;
		if (!charset.empty() && cmp_nocase(charset, "utf-8") != 0) {
			// Try to decode the text
			QTextCodec *c = QTextCodec::codecForName(charset.c_str());
			if (c) {
				text = c->toUnicode(text.c_str());
			} else {
				log_file->write_header(
						"t_gui::cb_message_request",
						LOG_NORMAL, LOG_WARNING);
				log_file->write_raw("Cannot decode charset: ");
				log_file->write_raw(charset);
				log_file->write_endl();
				log_file->write_footer();
			}
		}
	}
	
	lock();
	
	// Find an existing session
	im::t_msg_session *session = getMessageSession(user_config, r->hdr_from.uri,
				r->hdr_from.get_display_presentation());
	if (!session) {
		// There is no session yet.
		if (messageSessions.size() >= user_config->get_im_max_sessions()) {
			log_file->write_report(
				"Maximum number of message sessions reached. Reject message",
				"t_gui::cb_message_request");
			unlock();
			return false;
		}
		
		// Create a new session.
		session = new im::t_msg_session(user_config, t_display_url(r->hdr_from.uri, 
				r->hdr_from.get_display_presentation()));
		MEMMAN_NEW(session);
		addMessageSession(session);
		MessageFormView *view = new MessageFormView(NULL, session);
		MEMMAN_NEW(view);
		view->show();
		view->raise();
	}
	
	// Construct message
	im::t_msg msg;
	
	msg.direction = im::MSG_DIR_IN;
	
	if (r->hdr_subject.is_populated()) {
		msg.subject = r->hdr_subject.subject;
	}
	
	if (!text.empty()) {
		msg.message = text;
		msg.format = text_format;
	}
	
	if (attachment && !failed_to_save_attachment) {
		msg.has_attachment = true;
		msg.attachment_filename = attachment_saved_name;
		msg.attachment_save_as_name = attachment_save_as_name;
		msg.attachment_media = r->hdr_content_type.media;
	}
	
	session->recv_msg(msg);
	
	// Set error message if attachment could not be saved
	if (failed_to_save_attachment) {
		QString s = qApp->translate("GUI", "Failed to save message attachment: %1");
		s.arg(attachment_error_msg.c_str());
		session->set_error(s.ascii());
	}
	
	unlock();
	return true;
}

void t_gui::cb_message_response(t_user *user_config, t_response *r, t_request *req) {
	lock();
	
	// Find session associated with the response
	im::t_msg_session *session = getMessageSession(user_config, r->hdr_to.uri,
				r->hdr_to.display);
	
	if (session) {
		if (!r->is_success()) {
			string s = int2str(r->code);
			s += ' ';
			s += r->reason;
			session->set_error(s);
		} else {
			if (r->code == R_202_ACCEPTED) {
				string s;
				if (r->reason == REASON_202) {
					s = qApp->translate("GUI", "Accepted by network").ascii();
				} else { 
					s = r->reason;
				}
				session->set_delivery_notification(s);
			}
		}
		
		session->set_msg_in_flight(false);
	}
	// If there is no session anymore, then discard the response
	
	unlock();
}

void t_gui::cb_im_iscomposing_request(t_user *user_config, t_request *r,
			im::t_composing_state state, time_t refresh)
{
	lock();
	
	// Find an existing session
	im::t_msg_session *session = getMessageSession(user_config, r->hdr_from.uri,
				r->hdr_from.get_display_presentation());
	if (!session) {
		// A composing indication does not initiate a new message session.
		log_file->write_report(
			"Received composing indication for unknown session.",
			"t_gui::cb_im_iscomposing_request");
	} else {
		session->set_remote_composing_state(state, refresh);
	}
	
	unlock();
}

void t_gui::cb_im_iscomposing_not_supported(t_user *user_config, t_response *r) {
	lock();
	
	// Find an existing session
	im::t_msg_session *session = getMessageSession(user_config, r->hdr_to.uri,
				r->hdr_to.display);
	
	if (session) session->set_send_composing_state(false);
	
	unlock();
}

void t_gui::cmd_call(const string &destination, bool immediate) {
	string subject;
	string dst_no_headers;
	t_display_url du;
	
	t_user *user = phone->ref_user_profile(
			mainWindow->userComboBox->currentText().ascii());
	expand_destination(user, destination, du, subject, dst_no_headers);
	if (!du.is_valid()) return;
	
	lock();
	if (immediate) {
		mainWindow->do_phoneInvite(user, du.display.c_str(), du.url, 
					   subject.c_str(), false);
	} else {
		mainWindow->phoneInvite(dst_no_headers.c_str(), subject.c_str(), false);
	}
	unlock();
}

void t_gui::cmd_quit(void) {
	lock();
	mainWindow->fileExit();
	unlock();
}

void t_gui::cmd_show(void) {
	lock();
	if (mainWindow->isMinimized()) {
		mainWindow->setWindowState(mainWindow->windowState() & ~Qt::WindowMinimized | Qt::WindowActive);
		mainWindow->raise();
	} else {
		mainWindow->show();
		mainWindow->raise();
		mainWindow->setActiveWindow();
	}
	unlock();
}

void t_gui::cmd_hide(void) {
	lock();
	if (sys_config->get_gui_use_systray()) {
		mainWindow->hide();
	} else {
		mainWindow->setWindowState(mainWindow->windowState() | Qt::WindowMinimized);
	}
	unlock();
}

string t_gui::get_name_from_abook(t_user *user_config, const t_url &u) {
	string name;
	
	lock();
	// Search local address book first
	name = t_userintf::get_name_from_abook(user_config, u);
	
	// Search KAddressBook
	if (name.empty()) {
		t_address_finder *af = t_address_finder::get_instance();
		name = af->find_name(user_config, u);
	}
	unlock();
	
	return name;
}

// User invoked actions on the phone object

void t_gui::action_register(list<t_user *> user_list) {
	for (list<t_user *>::iterator i = user_list.begin(); i != user_list.end(); i++) {
		phone->pub_registration(*i, REG_REGISTER, 
			DUR_REGISTRATION(*i));
	}
}

void t_gui::action_deregister(list<t_user *> user_list, bool dereg_all) {
	for (list<t_user *>::iterator i = user_list.begin(); i != user_list.end(); i++) {
		if (dereg_all) {
			phone->pub_registration(*i, REG_DEREGISTER_ALL);
		} else {
			phone->pub_registration(*i, REG_DEREGISTER);
		}
	}
}

void t_gui::action_show_registrations(list<t_user *> user_list) {
	for (list<t_user *>::iterator i = user_list.begin(); i != user_list.end(); i++) {
		phone->pub_registration(*i, REG_QUERY);
	}
}

void t_gui::action_invite(t_user *user_config, const t_url &destination,
			  const string &display, const string &subject, bool anonymous) 
{
	QString s;
	
	// Call can only be made if line is idle
	int line = phone->get_active_line();
	if (phone->get_line_state(line) == LS_BUSY) return;
	
	t_url vm_url(expand_destination(user_config, user_config->get_mwi_vm_address()));
	if (destination != vm_url) {
		// Store call info for redial
		last_called_url = destination;
		last_called_display = display;
		last_called_subject = subject;
		last_called_profile = user_config->get_profile_name();
		last_called_hide_user = anonymous;
	}
	
	setLineFields(line);
	
	// Set party and subject line fields
	s = "";
	s.append(format_sip_address(user_config, display, destination).c_str());
	displayTo(s);
	
	s = "";
	s.append(format_sip_address(user_config, user_config->get_display(false), 
				    user_config->create_user_uri(false)).c_str());
	displayFrom(s);
	
	displaySubject(subject.c_str());
	
	phone->pub_invite(user_config, destination, display, subject.c_str(), anonymous);
}

void t_gui::action_answer(void) {
	cb_stop_call_notification(phone->get_active_line());
	phone->pub_answer();
}

void t_gui::action_bye(void) {
	phone->pub_end_call();
}

void t_gui::action_reject(void) {
	QString s;
	
	cb_stop_call_notification(phone->get_active_line());
	phone->pub_reject();
	
	int line = phone->get_active_line();
	mainWindow->displayHeader();
	s = qApp->translate("GUI", "Line %1: call rejected.").arg(line + 1);
	mainWindow->display(s);
}

void t_gui::action_reject(unsigned short line) {
	QString s;
	
	cb_stop_call_notification(line);
	phone->pub_reject(line);
	
	mainWindow->displayHeader();
	s = qApp->translate("GUI", "Line %1: call rejected.").arg(line + 1);
	mainWindow->display(s);
}

void t_gui::action_redirect(const list<t_display_url> &contacts) {
	QString s;
	
	cb_stop_call_notification(phone->get_active_line());
	phone->pub_redirect(contacts, 302);
	
	int line = phone->get_active_line();
	mainWindow->displayHeader();
	s = qApp->translate("GUI", "Line %1: call redirected.").arg(line + 1);
	mainWindow->display(s);
}

void t_gui::action_refer(const t_url &destination, const string &display) {
	phone->pub_refer(destination, display);
}

void t_gui::action_refer(unsigned short line_from, unsigned short line_to) {
	phone->pub_refer(line_from, line_to);
}

void t_gui::action_setup_consultation_call(const t_url &destination, const string &display) {
	phone->pub_setup_consultation_call(destination, display);
}

void t_gui::action_hold(void) {
	phone->pub_hold();
}

void t_gui::action_retrieve(void) {
	phone->pub_retrieve();
}

void t_gui::action_conference(void) {
	if (!phone->join_3way(0, 1)) {
		mainWindow->display(qApp->translate("GUI", "Failed to start conference."));
	}
}

void t_gui::action_mute(bool on) {
	phone->mute(on);
}

void t_gui::action_options(void) {
	phone->pub_options();
}

void t_gui::action_options(t_user *user_config, const t_url &contact) {
	phone->pub_options(user_config, contact);
}

void t_gui::action_dtmf(const string &digits) {
	const t_call_info call_info = phone->get_call_info(phone->get_active_line());
	throttle_dtmf_not_supported = false;
	
	if (!call_info.dtmf_supported) return;
	
	for (string::const_iterator i = digits.begin(); i != digits.end(); i++) {
		if (VALID_DTMF_SYM(*i)) {
			phone->pub_send_dtmf(*i, call_info.dtmf_inband, call_info.dtmf_info);
		}
	}
}

void t_gui::action_activate_line(unsigned short line) {
	phone->pub_activate_line(line);
}

bool t_gui::action_seize(void) {
	return phone->pub_seize();
}

void t_gui::action_unseize(void) {
	phone->pub_unseize();
}

void t_gui::action_confirm_zrtp_sas(int line) {
	phone->pub_confirm_zrtp_sas(line);
}

void t_gui::action_confirm_zrtp_sas() {
	phone->pub_confirm_zrtp_sas();
}

void t_gui::action_reset_zrtp_sas_confirmation(int line) {
	phone->pub_reset_zrtp_sas_confirmation(line);
}

void t_gui::action_reset_zrtp_sas_confirmation() {
	phone->pub_reset_zrtp_sas_confirmation();
}

void  t_gui::action_enable_zrtp(void) {
	phone->pub_enable_zrtp();
}

void  t_gui::action_zrtp_request_go_clear(void) {
	phone->pub_zrtp_request_go_clear();
}

void  t_gui::action_zrtp_go_clear_ok(unsigned short line) {
	phone->pub_zrtp_go_clear_ok(line);
}

void t_gui::srv_dnd(list<t_user *> user_list, bool on) {
	for (list<t_user *>::iterator i = user_list.begin(); i != user_list.end(); i++) {
		if (on) {
			phone->ref_service(*i)->enable_dnd();
		} else {
			phone->ref_service(*i)->disable_dnd();
		}
	}
}

void t_gui::srv_enable_cf(t_user *user_config,
		t_cf_type cf_type, const list<t_display_url> &cf_dest) 
{
	phone->ref_service(user_config)->enable_cf(cf_type, cf_dest);
}

void t_gui::srv_disable_cf(t_user *user_config, t_cf_type cf_type) {
	phone->ref_service(user_config)->disable_cf(cf_type);
}

void t_gui::srv_auto_answer(list<t_user *> user_list, bool on) {
	for (list<t_user *>::iterator i = user_list.begin(); i != user_list.end(); i++) {
		phone->ref_service(*i)->enable_auto_answer(on);
	}
}

void t_gui::fill_user_combo(QComboBox *cb) {
	cb->clear();
	
	list<t_user *> user_list = phone->ref_users();
	for (list<t_user *>::iterator i = user_list.begin(); i != user_list.end(); i++) {
		// OLD code: used display uri
		// cb->insertItem((*i)->get_display_uri().c_str());
		cb->insertItem((*i)->get_profile_name().c_str());
	}
	
	cb->setCurrentItem(0);
}

QString t_gui::get_last_file_browse_path(void) const {
	return lastFileBrowsePath;
}

void t_gui::set_last_file_browse_path(QString path) {
	lock();
	lastFileBrowsePath = path;
	unlock();
}

#ifdef HAVE_KDE
unsigned short t_gui::get_line_sys_tray_popup(void) const {
	return line_sys_tray_popup;
}
#endif

im::t_msg_session *t_gui::getMessageSession(t_user *user_config, 
			const t_url &remote_url, const string &display) const 
{
	for (list<im::t_msg_session *>::const_iterator it = messageSessions.begin();
	it != messageSessions.end(); ++it)
	{
		if ((*it)->match(user_config, remote_url)) {
			(*it)->set_display_if_empty(display);
			return *it;
		}
	}
	
	return NULL;
}
	
void t_gui::addMessageSession(im::t_msg_session *s) {
	messageSessions.push_back(s);
	
	if (!timerUpdateMessageSessions) {
		timerUpdateMessageSessions = new QTimer();
		MEMMAN_NEW(timerUpdateMessageSessions);
		
		connect(timerUpdateMessageSessions, SIGNAL(timeout()),
				 this, SLOT(updateTimersMessageSessions()));
		timerUpdateMessageSessions->start(1000);
	}
}

void t_gui::removeMessageSession(im::t_msg_session *s) {
	messageSessions.remove(s);
	
	if (messageSessions.empty()) {
		timerUpdateMessageSessions->stop();
		
		MEMMAN_DELETE(timerUpdateMessageSessions);
		delete timerUpdateMessageSessions;
		timerUpdateMessageSessions = NULL;
	}
}

void t_gui::destroyAllMessageSessions(void) {
	if (timerUpdateMessageSessions) {
		MEMMAN_DELETE(timerUpdateMessageSessions);
		delete timerUpdateMessageSessions;
		timerUpdateMessageSessions = NULL;
	}
	
	for (list<im::t_msg_session *>::iterator it = messageSessions.begin();
	it != messageSessions.end(); ++it)
	{
		MEMMAN_DELETE(*it);
		delete *it;
	}
	
	messageSessions.clear();
}

void t_gui::updateTimersMessageSessions() {
	for (list<im::t_msg_session *>::iterator it = messageSessions.begin();
	it != messageSessions.end(); ++it)
	{
		(*it)->dec_local_composing_timeout();
		(*it)->dec_remote_composing_timeout();
	}
}

string t_gui::mime2file_extension(t_media media) {
	string extension;
	
#ifdef HAVE_KDE
	// If KDE is available then use KDE to retrieve the proper file
	// extension so we nicely integrate with the desktop settings.
	string mime = media.type + "/" + media.subtype;
	KMimeType::Ptr pMime = KMimeType::mimeType(mime.c_str());
	const QStringList &patterns = pMime->patterns();
	
	if (!patterns.empty()) {
		extension = patterns.front().ascii();
	} else {
		log_file->write_header("t_gui::mime2file_extension", LOG_NORMAL, LOG_DEBUG);
		log_file->write_raw("Cannot find file extension for mime type ");
		log_file->write_raw(mime);
		log_file->write_footer();
	}
#endif
	return extension;
}

void t_gui::open_url_in_browser(const QString &url) {
	string sys_browser = sys_config->get_gui_browser_cmd();
#ifdef HAVE_KDE
	if (sys_browser.empty())
	{
		KTrader::OfferList offers = KTrader::self()->query("text/html", "Type == 'Application'");
		if (!offers.empty()) {
			KService::Ptr ptr = offers.first();
			KURL::List lst;
			lst.append(url);
			KRun::run(*ptr, lst);
		
			return;
		}
	}
#endif
	QProcess process;
	bool process_started = false;
	
	QStringList browsers;
	
	if (sys_browser.empty()) {
		browsers << "firefox" << "mozilla" << "netscape" << "opera";
		browsers << "galeon" << "epiphany" << "konqueror";
	} else {
		browsers << sys_browser.c_str();
	}
	
	for (QStringList::Iterator it = browsers.begin(); it != browsers.end(); ++it)
	{
		process.setArguments(QStringList() << *it << url);
		process_started = process.start();
		if (process_started) break;
	}
	
	if (!process_started) {
		QString msg = qApp->translate("GUI", "Cannot open web browser: %1").arg(url);
		msg += "\n\n";
		msg += qApp->translate("GUI", "Configure your web browser in the system settings.");
		cb_show_msg(msg.ascii(), MSG_CRITICAL);
	}
}
