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

#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include "address_book.h"
#include "call_history.h"
#include "events.h"
#include "line.h"
#include "listener.h"
#include "log.h"
#include "phone.h"
#include "protocol.h"
#include "sender.h"
#include "sys_settings.h"
#include "transaction_mgr.h"
#include "translator.h"
#include "user.h"
#include "userintf.h"
#include "util.h"
#include "sockets/connection_table.h"
#include "sockets/interfaces.h"
#include "sockets/socket.h"
#include "threads/thread.h"
#include "utils/mime_database.h"
#include "audits/memman.h"

using namespace std;
using namespace utils;

// Class to initialize the random generator before objects of
// other classes are created. Initializing just from the main function
// is too late.
class t_init_rand {
public:
	t_init_rand();
};

t_init_rand::t_init_rand() { srand(time(NULL)); }

// Memory manager for memory leak tracing
t_memman 		*memman;

// Initialize random generator
t_init_rand init_rand;

// Indicates if application is ending (because user pressed Quit)
bool end_app;

// Language translator
t_translator *translator = NULL;

// IP address on which the phone is running
string user_host;

// Local host name
string local_hostname;

// SIP UDP socket for sending and receiving signaling
t_socket_udp *sip_socket;

// SIP TCP socket for sending and receiving signaling
t_socket_tcp *sip_socket_tcp;

// SIP connection table for connection oriented transport
t_connection_table *connection_table;

// Event queue that is handled by the transaction manager thread
// The following threads write to this queue
// - UDP listener
// - transaction layer
// - timekeeper
t_event_queue		*evq_trans_mgr;

// Event queue that is handled by the sender thread
// The following threads write to this queue:
// - phone UAS
// - phone UAC
// - transaction manager
t_event_queue		*evq_sender;

// Event queue that is handled by the transaction layer thread
// The following threads write to this queue
// - transaction manager
// - timekeeper
t_event_queue		*evq_trans_layer;

// Event queue that is handled by the phone timekeeper thread
// The following threads write into this queue
// - phone UAS
// - phone UAC
// - transaction manager
t_event_queue		*evq_timekeeper;

// The timekeeper
t_timekeeper		*timekeeper;

// The transaction manager
t_transaction_mgr	*transaction_mgr;

// The phone
t_phone			*phone;

// User interface
t_userintf		*ui;

// Log file
t_log			*log_file;

// System config
t_sys_settings		*sys_config;

// Call history
t_call_history		*call_history;

// Local address book
t_address_book		*ab_local;

// Mime database
t_mime_database		*mime_database;

// If a port number is passed by the user on the command line, then
// that port number overrides the port from the system settings.
unsigned short		g_override_sip_port = 0;
unsigned short		g_override_rtp_port = 0;

// Indicates if LinuxThreads or NPTL is active.
bool			threading_is_LinuxThreads;


int main(int argc, char *argv[]) {
	string error_msg;
	
	end_app = false;

	memman = new t_memman();
	MEMMAN_NEW(memman);
	translator = new t_translator();
	MEMMAN_NEW(translator);
	connection_table = new t_connection_table();
	MEMMAN_NEW(connection_table);
	evq_trans_mgr = new t_event_queue();
	MEMMAN_NEW(evq_trans_mgr);
	evq_sender = new t_event_queue();
	MEMMAN_NEW(evq_sender);
	evq_trans_layer = new t_event_queue();
	MEMMAN_NEW(evq_trans_layer);
	evq_timekeeper = new t_event_queue();
	MEMMAN_NEW(evq_timekeeper);
	timekeeper = new t_timekeeper();
	MEMMAN_NEW(timekeeper);
	transaction_mgr = new t_transaction_mgr();
	MEMMAN_NEW(transaction_mgr);
	phone = new t_phone();
	MEMMAN_NEW(phone);

	sys_config = new t_sys_settings();
	MEMMAN_NEW(sys_config);
	ui = new t_userintf(phone);
	MEMMAN_NEW(ui);

	// Check requirements on environment
	if (!sys_config->check_environment(error_msg)) {
		// Environment is not good
		ui->cb_show_msg(error_msg, MSG_CRITICAL);
		exit(1);
	}
	
	// Read system configuration
	if (!sys_config->read_config(error_msg)) {
		ui->cb_show_msg(error_msg, MSG_CRITICAL);
		exit(1);
	}
	
	// Get default values from system configuration
	list<string> config_files;
	list<string> start_user_profiles = sys_config->get_start_user_profiles();
	for (list<string>::iterator i = start_user_profiles.begin();
	     i != start_user_profiles.end(); i++)
	{
		string config_file = *i;
		config_file += USER_FILE_EXT;
		config_files.push_back(config_file);
	}

#if 0
	// DEPRECATED
	if (user_host.empty()) {
                string ip;
		if (exists_interface(sys_config->get_start_user_host())) {
			user_host = sys_config->get_start_user_host();
		}
		else if (exists_interface_dev(sys_config->get_start_user_nic(), ip)) {
			user_host = ip;
		}
	}
#endif
	user_host = AUTO_IP4_ADDRESS;
	local_hostname = get_local_hostname();

	// Create a lock file to guarantee that the application
	// runs only once.
	bool already_running;
	if (!sys_config->create_lock_file(false, error_msg, already_running)) {
		ui->cb_show_msg(error_msg, MSG_CRITICAL);
		exit(1);
	}

	log_file = new t_log();
	MEMMAN_NEW(log_file);
	
	call_history = new t_call_history();
	MEMMAN_NEW(call_history);

	// Determine threading implementation
	threading_is_LinuxThreads = t_thread::is_LinuxThreads();
	if (threading_is_LinuxThreads) {
		log_file->write_report("Threading implementation is LinuxThreads.",
			"::main", LOG_NORMAL, LOG_INFO);
	} else {
		log_file->write_report("Threading implementation is NPTL.",
			"::main", LOG_NORMAL, LOG_INFO);
	}

	// Take default user profile if there are is no default is sys settings
	if (config_files.empty()) config_files.push_back(USER_CONFIG_FILE);

	// Read user configurations.
	if (argc >= 2) {
		config_files.clear();
		for (int i = 1; i < argc; i++) {
			config_files.push_back(argv[i]);
		}
	}

	// Activate users
	for (list<string>::iterator i = config_files.begin();
		i != config_files.end(); i++)
	{		
		t_user *user_config = new t_user();
		MEMMAN_NEW(user_config);
		if (!user_config->read_config(*i, error_msg)) {
			ui->cb_show_msg(error_msg, MSG_CRITICAL);
			sys_config->delete_lock_file();
			exit(1);
		}
		
		t_user *dup_user;
		if(!phone->add_phone_user(*user_config, &dup_user)) {
			error_msg = "The following profiles are both for user ";
			error_msg += user_config->get_name();
			error_msg += '@';
			error_msg += user_config->get_domain();
			error_msg += ":\n\n";
			error_msg += user_config->get_profile_name();
			error_msg += "\n";
			error_msg += dup_user->get_profile_name();
			error_msg += "\n\n";
			error_msg += "You can only run multiple profiles ";
			error_msg += "for different users.";
			ui->cb_show_msg(error_msg, MSG_CRITICAL);
			sys_config->delete_lock_file();
			exit(1);
		}
			
		MEMMAN_DELETE(user_config);
		delete user_config;
	}
	
	// Read call history
	if (!call_history->load(error_msg)) {
		log_file->write_report(error_msg, "::main", LOG_NORMAL, LOG_WARNING);
	}
	
	// Create local address book
	ab_local = new t_address_book();
	MEMMAN_NEW(ab_local);
	
	// Read local address book
	if (!ab_local->load(error_msg)) {
		log_file->write_report(error_msg, "::main", LOG_NORMAL, LOG_WARNING);
		ui->cb_show_msg(error_msg, MSG_WARNING);
	}
	
	// Create mime database
	mime_database = new t_mime_database();
	MEMMAN_NEW(mime_database);
	if (!mime_database->load(error_msg)) {
		log_file->write_report(error_msg, "::main", LOG_NORMAL, LOG_WARNING);
	}

	// Initialize RTP port settings.
	phone->init_rtp_ports();

	// Open UDP socket for SIP signaling
	try {
		sip_socket = new t_socket_udp(sys_config->get_sip_port());
		MEMMAN_NEW(sip_socket);
		if (sip_socket->enable_icmp()) {
			log_file->write_report("ICMP processing enabled.", "::main");
		} else {
			log_file->write_report("ICMP processing disabled.", "::main");
		}
	} catch (int err) {
		string msg("Failed to create a UDP socket (SIP) on port ");
		msg += int2str(sys_config->get_sip_port());
		msg += "\n";
		msg += get_error_str(err);
		log_file->write_report(msg, "::main", LOG_NORMAL, LOG_CRITICAL);
		ui->cb_show_msg(msg, MSG_CRITICAL);
		sys_config->delete_lock_file();
		exit(1);
	}
	
	// Open TCP socket for SIP signaling
	try {
		sip_socket_tcp = new t_socket_tcp(sys_config->get_sip_port());
		MEMMAN_NEW(sip_socket_tcp);		
	} catch (int err) {
		string msg("Failed to create a TCP socket (SIP) on port ");
		msg += int2str(sys_config->get_sip_port());
		msg += "\n";
		msg += get_error_str(err);
		log_file->write_report(msg, "::main", LOG_NORMAL, LOG_CRITICAL);
		ui->cb_show_msg(msg, MSG_CRITICAL);
		sys_config->delete_lock_file();
		exit(1);
	}
	
#if 0
	// DEPRECATED
	// Pick network interface
	if (user_host.empty()) {
		user_host = ui->select_network_intf();
		if (user_host.empty()) {
			sys_config->delete_lock_file();
			exit(1);
		}
	}
#endif
	
	// Discover NAT type if STUN is enabled
	list<string> msg_list;
	if (!phone->stun_discover_nat(msg_list)) {
		for (list<string>::iterator i = msg_list.begin();
		     i != msg_list.end(); i++)
		{
			ui->cb_show_msg(*i, MSG_WARNING);
		}
	}
	
	// Dedicated thread will catch SIGALRM, SIGINT, SIGTERM, SIGCHLD signals, 
	// therefore all threads must block these signals. Block now, then all
	// created threads will inherit the signal mask.
	// In LinuxThreads the sigwait does not work very well, so
	// in LinuxThreads a signal handler is used instead.
	if (!threading_is_LinuxThreads) {
		sigset_t sigset;
		sigemptyset(&sigset);
		sigaddset(&sigset, SIGALRM);
		sigaddset(&sigset, SIGINT);
		sigaddset(&sigset, SIGTERM);
		sigaddset(&sigset, SIGCHLD);
		sigprocmask(SIG_BLOCK, &sigset, NULL);
	} else {
		if (!phone->set_sighandler()) {
			string msg = "Failed to register signal handler.";
			log_file->write_report(msg, "::main", LOG_NORMAL, LOG_CRITICAL);
			ui->cb_show_msg(msg, MSG_CRITICAL);
			sys_config->delete_lock_file();
			exit(1);
		}
	}

	// Ignore SIGPIPE so read from broken sockets will not cause
	// the process to terminate.
	(void)signal(SIGPIPE, SIG_IGN);

	// Create threads
	t_thread *thr_sender;
	t_thread *thr_tcp_sender;
	t_thread *thr_listen_udp;
	t_thread *thr_listen_data_tcp;
	t_thread *thr_listen_conn_tcp;
	t_thread *thr_conn_timeout_handler;
	t_thread *thr_timekeeper;
	t_thread *thr_alarm_catcher = NULL;
	t_thread *thr_sig_catcher = NULL;
	t_thread *thr_trans_mgr;
	t_thread *thr_phone_uas;

	try {
		// SIP sender thread
		thr_sender = new t_thread(sender_loop, NULL);
		MEMMAN_NEW(thr_sender);
		
		// SIP TCP sender thread
		thr_tcp_sender = new t_thread(tcp_sender_loop, NULL);
		MEMMAN_NEW(thr_tcp_sender);

		// UDP listener thread
		thr_listen_udp = new t_thread(listen_udp, NULL);
		MEMMAN_NEW(thr_listen_udp);
		
		// TCP data listener thread
		thr_listen_data_tcp = new t_thread(listen_for_data_tcp, NULL);
		MEMMAN_NEW(thr_listen_data_tcp);
		
		// TCP connection listener thread
		thr_listen_conn_tcp = new t_thread(listen_for_conn_requests_tcp, NULL);
		MEMMAN_NEW(thr_listen_conn_tcp);
		
		// Connection timeout handler thread
		thr_conn_timeout_handler = new t_thread(connection_timeout_main, NULL);
		MEMMAN_NEW(thr_conn_timeout_handler);

		// Timekeeper thread
		thr_timekeeper = new t_thread(timekeeper_main, NULL);
		MEMMAN_NEW(thr_timekeeper);
		
		if (!threading_is_LinuxThreads) {
			// Alarm catcher thread
			thr_alarm_catcher = new t_thread(timekeeper_sigwait, NULL);
			MEMMAN_NEW(thr_alarm_catcher);

			// Signal catcher thread
			thr_sig_catcher = new t_thread(phone_sigwait, NULL);
			MEMMAN_NEW(thr_sig_catcher);
		}

		// Transaction manager thread
		thr_trans_mgr = new t_thread(transaction_mgr_main, NULL);
		MEMMAN_NEW(thr_trans_mgr);

		// Phone thread (UAS)
		thr_phone_uas = new t_thread(phone_uas_main, NULL);
		MEMMAN_NEW(thr_phone_uas);
	} catch (int) {
		string msg = "Failed to create threads.";
		log_file->write_report(msg, "::main", LOG_NORMAL, LOG_CRITICAL);
		ui->cb_show_msg(msg, MSG_CRITICAL);
		sys_config->delete_lock_file();
		exit(1);
	}
	
	// Validate sound devices
	if (!sys_config->exec_audio_validation(true, true, true, error_msg)) {
		ui->cb_show_msg(error_msg, MSG_WARNING);
	}

	try {
		ui->run();
	} catch (string e) {
		string msg = "Exception: ";
		msg += e;
		log_file->write_report(msg, "::main", LOG_NORMAL, LOG_CRITICAL);
		ui->cb_show_msg(msg, MSG_CRITICAL);
		sys_config->delete_lock_file();
		exit(1);
	} catch (...) {
		string msg = "Unknown exception";
		log_file->write_report(msg, "::main", LOG_NORMAL, LOG_CRITICAL);
		ui->cb_show_msg(msg, MSG_CRITICAL);
		sys_config->delete_lock_file();
		exit(1);
	}
	
	// Application is ending
	end_app = true;
	
	// Kill the threads getting receiving input from the outside world first,
	// so no new inputs come in during termination.
	thr_listen_udp->cancel();
	thr_listen_udp->join();
	
	thr_listen_conn_tcp->cancel();
	thr_listen_conn_tcp->join();
	
	connection_table->cancel_select();
	thr_listen_data_tcp->join();
	thr_conn_timeout_handler->join();
	thr_tcp_sender->join();
	
	evq_trans_layer->push_quit();
	thr_phone_uas->join();
	
	evq_trans_mgr->push_quit();
	thr_trans_mgr->join();
	
	if (!threading_is_LinuxThreads) {
		try {
			thr_sig_catcher->cancel();
		} catch (int) {
			// Thread terminated already by itself
		}
		thr_sig_catcher->join();
		
		thr_alarm_catcher->cancel();
		thr_alarm_catcher->join();
	}
	
	evq_timekeeper->push_quit();
	thr_timekeeper->join();
	
	evq_sender->push_quit();
	thr_sender->join();
	
	sys_config->remove_all_tmp_files();

	MEMMAN_DELETE(thr_phone_uas);
	delete thr_phone_uas;
	MEMMAN_DELETE(thr_trans_mgr);
	delete thr_trans_mgr;
	MEMMAN_DELETE(thr_timekeeper);
	delete thr_timekeeper;
	MEMMAN_DELETE(thr_conn_timeout_handler);
	delete thr_conn_timeout_handler;

	if (!threading_is_LinuxThreads) {
		MEMMAN_DELETE(thr_sig_catcher);
		delete thr_sig_catcher;
		MEMMAN_DELETE(thr_alarm_catcher);
		delete thr_alarm_catcher;
	}

	MEMMAN_DELETE(thr_listen_udp);
	delete thr_listen_udp;
	MEMMAN_DELETE(thr_sender);
	delete thr_sender;
	MEMMAN_DELETE(thr_tcp_sender);
	delete thr_tcp_sender;
	
	
	MEMMAN_DELETE(thr_listen_data_tcp);
	delete thr_listen_data_tcp;
	MEMMAN_DELETE(thr_listen_conn_tcp);
	delete thr_listen_conn_tcp;

	MEMMAN_DELETE(mime_database);
	delete mime_database;
	MEMMAN_DELETE(ab_local);
	delete ab_local;
	MEMMAN_DELETE(call_history);
	delete call_history;

	MEMMAN_DELETE(ui);
	delete ui;
	ui = NULL;
	
	MEMMAN_DELETE(connection_table);
	delete connection_table;
	MEMMAN_DELETE(sip_socket_tcp);
	delete sip_socket_tcp;
	MEMMAN_DELETE(sip_socket);
	delete sip_socket;

	MEMMAN_DELETE(phone);
	delete phone;
	MEMMAN_DELETE(transaction_mgr);
	delete transaction_mgr;
	MEMMAN_DELETE(timekeeper);
	delete timekeeper;
	MEMMAN_DELETE(evq_trans_mgr);
	delete evq_trans_mgr;
	MEMMAN_DELETE(evq_sender);
	delete evq_sender;
	MEMMAN_DELETE(evq_trans_layer);
	delete evq_trans_layer;
	MEMMAN_DELETE(evq_timekeeper);
	delete evq_timekeeper;
	
	MEMMAN_DELETE(translator);
	delete translator;
	translator = NULL;

	// Report memory leaks
	// Report deletion of log_file and sys_config already to get
	// a correct report.
	MEMMAN_DELETE(sys_config);
	MEMMAN_DELETE(log_file);
	MEMMAN_DELETE(memman);
	MEMMAN_REPORT;

	delete log_file;
	delete memman;

	sys_config->delete_lock_file();
	delete sys_config;
}
