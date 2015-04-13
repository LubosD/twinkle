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

#ifndef _GUI_H
#define _GUI_H

#include "twinkle_config.h"

#include "phone.h"
#include "userintf.h"
#include "im/msg_session.h"

#include "messageform.h"

#include "qaction.h"
#include "qcombobox.h"
#include "qlabel.h"
#include "qlineedit.h"
#include "qprogressdialog.h"
#include "qtimer.h"
#include "qtoolbutton.h"
#include "qwidget.h"

#ifdef HAVE_KDE
#include <kpassivepopup.h>
#endif

using namespace std;

// Forward declaration
class MphoneForm;

// Length of redial list in combo boxes
#define SIZE_REDIAL_LIST 10

// Selection purpose for select user form
enum t_select_purpose {
	SELECT_REGISTER,
	SELECT_DEREGISTER,
	SELECT_DEREGISTER_ALL,
	SELECT_DND,
	SELECT_AUTO_ANSWER
};

QString str2html(const QString &s);

void setDisabledIcon(QAction *action, const QString &icon);
void setDisabledIcon(QToolButton *toolButton, const QString &icon);

class t_gui : public QObject, public t_userintf {
	Q_OBJECT
private:
	MphoneForm	*mainWindow;
	
	// List of active instant messaging session.
	list<im::t_msg_session *> messageSessions;
	
	// Timer to schedule updating of message sessions every second.
	QTimer *timerUpdateMessageSessions;
	
	// Progress dialog for FW/NAT discovery progress bar
	QProgressDialog	*natDiscoveryProgressDialog;
	
	// Pointers to line information fields to display information
	QLineEdit	*fromLabel;
	QLineEdit	*toLabel;
	QLineEdit	*subjectLabel;
	QLabel		*codecLabel;
	QLabel		*photoLabel;
	
	// Timers to auto show main window on incoming call
	QTimer		autoShowTimer[NUM_USER_LINES];
	
#ifdef HAVE_KDE
	// Popup window on system tray for incoming call notification
	KPassivePopup	*sys_tray_popup;
	unsigned short	line_sys_tray_popup; // lineno for popup
#endif
	
	// Last dir path browsed by the user with a file dialog
	QString		lastFileBrowsePath;
	
	// Set the line information field pointers to the fields for 'line'
	void setLineFields(int line);
	
	// Set text inf from, to and subject fields
	void displayTo(const QString &s);
	void displayFrom(const QString &s);
	void displaySubject(const QString &s);
	
	// Display the codecs in use for the line
	void displayCodecInfo(int line);
	
	// Display a photo
	void displayPhoto(const QImage &photo);
	
protected:
	// The do_* methods perform the commands parsed by the exec_* methods.
	virtual bool do_invite(const string &destination, const string &display, 
			const string &subject, bool immediate,
			bool anonymous);
	virtual void do_redial(void);
	virtual void do_answer(void);
	virtual void do_answerbye(void);
	virtual void do_reject(void);
	virtual void do_redirect(bool show_status, bool type_present, t_cf_type cf_type, 
		bool action_present, bool enable, int num_redirections,
		const list<string> &dest_strlist, bool immediate);
	virtual void do_dnd(bool show_status, bool toggle, bool enable);
	virtual void do_auto_answer(bool show_status, bool toggle, bool enable);
	virtual void do_bye(void);
	virtual void do_hold(void);
	virtual void do_retrieve(void);
	virtual bool do_refer(const string &destination, t_transfer_type transfer_type,
		bool immediate);
	virtual void do_conference(void);
	virtual void do_mute(bool show_status, bool toggle, bool enable);
	virtual void do_dtmf(const string &digits);
	virtual void do_register(bool reg_all_profiles);
	virtual void do_deregister(bool dereg_all_profiles, bool dereg_all_devices);
	virtual void do_fetch_registrations(void);
	virtual bool do_options(bool dest_set, const string &destination, bool immediate);
	virtual void do_line(int line);
	virtual void do_user(const string &profile_name);
	virtual void do_zrtp(t_zrtp_cmd zrtp_cmd);
	virtual bool do_message(const string &destination, const string &display,
				const im::t_msg &msg);
	virtual void do_presence(t_presence_state::t_basic_state basic_state);
	virtual void do_quit(void);
	virtual void do_help(const list<t_command_arg> &al);
	
public:
	t_gui(t_phone *_phone);
	virtual ~t_gui();
	
	// Start the GUI
	void run(void);
	
	// Save user interface state to system settings
	void save_state(void);
	
	// Restore user interface state from system settings
	void restore_state(void);
	
	/** Save state to restore a UI session. */
	void save_session_state(void);
	
	/** Restore UI session state. */
	void restore_session_state(void);
	
	// Lock the user interface to synchornize output
	void lock(void);
	void unlock(void);
	
	// Select network interface to use.
	string select_network_intf(void);
	
	// Select a user configuration file. Returns false if selection failed.
	bool select_user_config(list<string> &config_files);
	
	// Clear the contents of the line information fields. After clearing
	// the field pointers point to the fields for 'line'
	void clearLineFields(int line);
	
	// Call back functions
	void cb_incoming_call(t_user *user_config, int line, const t_request *r);
	void cb_call_cancelled(int line);
	void cb_far_end_hung_up(int line);
	void cb_answer_timeout(int line);
	void cb_sdp_answer_not_supported(int line, const string &reason);
	void cb_sdp_answer_missing(int line);
	void cb_unsupported_content_type(int line, const t_sip_message *r);
	void cb_ack_timeout(int line);
	void cb_100rel_timeout(int line);
	void cb_prack_failed(int line, const t_response *r);
	void cb_provisional_resp_invite(int line, const t_response *r);
	void cb_cancel_failed(int line, const t_response *r);
	void cb_call_answered(t_user *user_config, int line, const t_response *r);
	void cb_call_failed(t_user *user_config, int line, const t_response *r);
	void cb_stun_failed_call_ended(int line);
	void cb_call_ended(int line);
	void cb_call_established(int line);
	void cb_options_response(const t_response *r);
	void cb_reinvite_success(int line, const t_response *r);
	void cb_reinvite_failed(int line, const t_response *r);
	void cb_retrieve_failed(int line, const t_response *r);
	void cb_invalid_reg_resp(t_user *user_config, const t_response *r, const string &reason);
	void cb_register_success(t_user *user_config, const t_response *r, unsigned long expires,
				 bool first_success);
	void cb_register_failed(t_user *user_config, const t_response *r, bool first_failure);
	void cb_register_stun_failed(t_user *user_config, bool first_failure);
	void cb_deregister_success(t_user *user_config, const t_response *r);
	void cb_deregister_failed(t_user *user_config, const t_response *r);
	void cb_fetch_reg_failed(t_user *user_config, const t_response *r);
	void cb_fetch_reg_result(t_user *user_config, const t_response *r);
	void cb_register_inprog(t_user *user_config, t_register_type register_type);
	void cb_redirecting_request(t_user *user_config, int line, const t_contact_param &contact);
	void cb_redirecting_request(t_user *user_config, const t_contact_param &contact);
	void cb_notify_call(int line, const QString &from_party, const QString &organization,
			   const QImage &photo, const QString &subject, QString &referred_by_party);
	void cb_stop_call_notification(int line);
	void cb_dtmf_detected(int line, char dtmf_event);
	void cb_send_dtmf(int line, char dtmf_event);
	void cb_dtmf_not_supported(int line);
	void cb_dtmf_supported(int line);
	void cb_line_state_changed(void);
	void cb_send_codec_changed(int line, t_audio_codec codec);
	void cb_recv_codec_changed(int line, t_audio_codec codec);
	void cb_notify_recvd(int line, const t_request *r);
	void cb_refer_failed(int line, const t_response *r);
	void cb_refer_result_success(int line);
	void cb_refer_result_failed(int line);
	void cb_refer_result_inprog(int line);
	
	// A call is being referred by the far end. r must be the REFER request.
	void cb_call_referred(t_user *user_config, int line, t_request *r);

	// The reference failed. Call to referrer is retrieved.
	void cb_retrieve_referrer(t_user *user_config, int line);
	
	// A consulation call for a call transfer is being setup.
	void cb_consultation_call_setup(t_user *user_config, int line);
	
	// STUN errors
	void cb_stun_failed(t_user *user_config, int err_code, const string &err_reason);
	void cb_stun_failed(t_user *user_config);
	
	// Interactive call back functions
	bool cb_ask_user_to_redirect_invite(t_user *user_config, const t_url &destination,
			const string &display);
	bool cb_ask_user_to_redirect_request(t_user *user_config, const t_url &destination,
			const string &display, t_method method);
	bool cb_ask_credentials(t_user *user_config, const string &realm, string &username,
			string &password);
	
	// Ask questions asynchronously.
	void cb_ask_user_to_refer(t_user *user_config, const t_url &refer_to_uri,
			const string &refer_to_display,
			const t_url &referred_by_uri,
			const string &referred_by_display);
	
	// Show an error message to the user. Depending on the interface mode
	// the user has to acknowledge the error before processing continues.
	void cb_show_msg(const string &msg, t_msg_priority prio = MSG_INFO);
	void cb_show_msg(QWidget *parent, const string &msg, t_msg_priority prio = MSG_INFO);
	
	// Ask a yes/no question to the user.
	// Returns true for yes and false for no.
	bool cb_ask_msg(const string &msg, t_msg_priority prio = MSG_INFO);
	bool cb_ask_msg(QWidget *parent, const string &msg, t_msg_priority prio = MSG_INFO);
	
	// Display an error message.
	void cb_display_msg(const string &msg, t_msg_priority prio = MSG_INFO);
	
	// Log file has been updated
	void cb_log_updated(bool log_zapped = false);
	
	// Call history has been updated
	void cb_call_history_updated(void);
	void cb_missed_call(int num_missed_calls);
	
	// Show firewall/NAT discovery progress
	void cb_nat_discovery_progress_start(int num_steps);
	void cb_nat_discovery_progress_step(int step);
	void cb_nat_discovery_finished(void);
	bool cb_nat_discovery_cancelled(void);
	
	// ZRTP
	void cb_line_encrypted(int line, bool encrypted, const string &cipher_mode = "");
	void cb_show_zrtp_sas(int line, const string &sas);
	void cb_zrtp_confirm_go_clear(int line);
	void cb_zrtp_sas_confirmed(int line);
	void cb_zrtp_sas_confirmation_reset(int line);
	
	// MWI
	void cb_update_mwi(void);
	void cb_mwi_subscribe_failed(t_user *user_config, t_response *r, bool first_failure);
	void cb_mwi_terminated(t_user *user_config, const string &reason);
	
	// Instant messaging
	bool cb_message_request(t_user *user_config, t_request *r);
	void cb_message_response(t_user *user_config, t_response *r, t_request *req);
	void cb_im_iscomposing_request(t_user *user_config, t_request *r,
			im::t_composing_state state, time_t refresh);
	void cb_im_iscomposing_not_supported(t_user *user_config, t_response *r);
	
	// Execute external commands
	void cmd_call(const string &destination, bool immediate);
	void cmd_quit(void);
	void cmd_show(void);
	void cmd_hide(void);
	
	// Lookup a URL in the address book
	string get_name_from_abook(t_user *user_config, const t_url &u);
	
	// Actions
	void action_register(list<t_user *> user_list);
	void action_deregister(list<t_user *> user_list, bool dereg_all);
	void action_show_registrations(list<t_user *> user_list);
	void action_invite(t_user *user_config, 
			   const t_url &destination, const string &display, 
			   const string &subject, bool anonymous);
	void action_answer(void);
	void action_bye(void);
	void action_reject(void);
	void action_reject(unsigned short line);
	void action_redirect(const list<t_display_url> &contacts);
	void action_refer(const t_url &destination, const string &display);
	void action_refer(unsigned short line_from, unsigned short line_to);
	void action_setup_consultation_call(const t_url &destination, const string &display);
	void action_hold(void);
	void action_retrieve(void);
	void action_conference(void);
	void action_mute(bool on);
	void action_options(void);
	void action_options(t_user *user_config, const t_url &contact);
	void action_dtmf(const string &digits);
	void action_activate_line(unsigned short line);
	bool action_seize(void);
	void action_unseize(void);
	void action_confirm_zrtp_sas(int line);
	void action_confirm_zrtp_sas();
	void action_reset_zrtp_sas_confirmation(int line);
	void action_reset_zrtp_sas_confirmation();
	void action_enable_zrtp(void);
	void action_zrtp_request_go_clear(void);
	void action_zrtp_go_clear_ok(unsigned short line);
	
	// Service (de)activation
	void srv_dnd(list<t_user *> user_list, bool on);
	void srv_enable_cf(t_user *user_config,
		t_cf_type cf_type, const list<t_display_url> &cf_dest);
	void srv_disable_cf(t_user *user_config, t_cf_type cf_type);
	void srv_auto_answer(list<t_user *> user_list, bool on);
	
	// Fill a combo box with user names (display, uri) of active users
	void fill_user_combo(QComboBox *cb);
	
	// Get/set last dir path for a file dialog browse session
	QString get_last_file_browse_path(void) const;
	void set_last_file_browse_path(QString path);
	
#ifdef HAVE_KDE
	// Get the line associated with the sys tray popup
	unsigned short get_line_sys_tray_popup(void) const;
#endif
	
	// Get the message session for a dialog between the user
	// and the remote url. If the display name was not known
	// to the session yet, it is set to the passed display.
	// Returns NULL if no form exists.
	im::t_msg_session *getMessageSession(t_user *user_config,
				    const t_url &remote_url,
				    const string &display) const;
	
	void addMessageSession(im::t_msg_session *s);
	void removeMessageSession(im::t_msg_session *s);
	void destroyAllMessageSessions(void);
	
	/**
	  * Convert a mime type to a file extension.
	  * @param media [in] The mime type.
	  * @return file extension as glob expression.
	  */
	string mime2file_extension(t_media media);
	
	/** 
             * Open a URL in an external web browser.
	  * @param url [in] URL to open.
             */
	void open_url_in_browser(const QString &url);
	
private slots:
	/** 
            * Update timers associated with message sessions. This
	 * function should be called every second.
	 */
	void updateTimersMessageSessions();
};

#endif
