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

#include "twinkle_config.h"
#include <QtDebug>
#include <QtGlobal>

#ifdef HAVE_KDE
#include <kapplication.h>
#include <kcmdlineargs.h>
#endif

#include <qapplication.h>
#include <qtranslator.h>
#include <qtextcodec.h>
#include <QSettings>
#include <QDir>

#include "mphoneform.h"

#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <QtDebug>
#include <unistd.h>

#include "address_book.h"
#include "address_finder.h"
#include "call_history.h"
#include "cmd_socket.h"
#include "events.h"
#include "listener.h"
#include "log.h"
#include "protocol.h"
#include "sender.h"
#include "transaction_mgr.h"
#include "twinkleapplication.h"
#include "user.h"
#include "util.h"
#include "phone.h"
#include "gui.h"
#include "qt_translator.h"
#include "command_args.h"
#include "sockets/connection_table.h"
#include "sockets/interfaces.h"
#include "sockets/socket.h"
#include "threads/thread.h"
#include "utils/mime_database.h"
#include "audits/memman.h"
#include <QLibraryInfo>

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

// Initialize random generator
t_init_rand init_rand;

// Language translator for the core of Twinkle
t_translator *translator = NULL;

// Indicates if application is ending (because user pressed Quit)
bool end_app;

// Memory manager for memory leak tracing
t_memman 		*memman;

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

// Event queue that is handled by the UDP sender thread
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
t_mime_database	*mime_database;

/** Command arguments. */
t_command_args g_cmd_args;

// Thread id of main thread
pthread_t		thread_id_main;

// Indicates if LinuxThreads or NPTL is active.
bool			threading_is_LinuxThreads;

QSettings*      g_gui_state;

/**
 * Parse arguments passed to application
 * @param argc [in] Number of arguments
 * @param argv [in] Array of arguments
 * @param cli_mode [out] Indicates if Twinkle must run in CLI mode.
 * @param override_lock_file [out] Indicates if an existing lock file must be overriden.
 * @param config_files [out] User profiles passed on the command line.
 * @param remain_argc [out] The number of arguments not parsed by this function.
 * @param remain_argv [out] The arguments not parsed by this function.
 * 	remain_argv[0] == argv[0]
 */
void parse_main_args(int argc, char **argv, bool &cli_mode, bool &override_lock_file,
		     list<string> &config_files, int &remain_argc, char **&remain_argv) 
{
	cli_mode = false;
	override_lock_file = false;
	config_files.clear();
	
	// Initialize the remaining arguments with the first argument (application name)
	// from the original arguments.
	remain_argv = (char**)malloc(argc * sizeof(char*));
	remain_argv[0] = argv[0];
	remain_argc = 1;

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			// Help
			cout << "Usage: twinkle [options]\n\n";
			cout << "Options:\n";
			cout << " -c";
			cout << "\t\tRun in command line interface (CLI) mode\n";
			cout << endl;
			cout << " --share <dir>";
			cout << "\tSet the share directory.\n";
			cout << endl;
			cout << " -f <profile>";
			cout << "\tStartup with a specific profile. You will not be requested\n";
			cout << "\t\tto choose a profile at startup. The profiles that you created\n";
			cout << "\t\tare the .cfg files in your .twinkle directory.\n";
			cout << "\t\tYou may specify multiple profiles separated by spaces.\n";
			cout << endl;
			cout << " --force";
			cout << "\tIf a lock file is detected at startup, then override it\n";
			cout << "\t\tand startup.\n";
			cout << endl;
#if 0
			// DEPRECATED
			cout << " -i <IP addr>";
			cout << "\tIf you have multiple IP addresses on your computer,\n";
			cout << "\t\tthen you can supply the IP address to use here.\n";
			cout << endl;
#endif
			cout << " --sip-port <port>\n";
			cout << "\t\tPort for SIP signalling.\n";
			cout << "\t\tThis port overrides the port from the system settings.\n";
			cout << endl;
			cout << " --rtp-port <port>\n";
			cout << "\t\tPort for RTP.\n";
			cout << "\t\tThis port overrides the port from the system settings.\n";
			cout << endl;
#if 0
			// DEPRECATED
			cout << " --nic <NIC>";
			cout << "\tIf you have multiple NICs on your computer,\n";
			cout << "\t\tthen you can supply the NIC name to use here (e.g. eth0).\n";
			cout << endl;
#endif
			cout << " --call <address>\n";
			cout << "\t\tInstruct Twinkle to call the address.\n";
			cout << "\t\tWhen Twinkle is already running, this will instruct the running\n";
			cout << "\t\tprocess to call the address.\n";
			cout << "\t\tThe address may be a full or partial SIP URI. A partial SIP URI\n";
			cout << "\t\twill be completed with the information from the user profile.\n";
			cout << endl;
			cout << "\t\tA subject may be passed by appending '?subject=<subject>'\n";
			cout << "\t\tto the address.\n";
			cout << endl;
			cout << "\t\tExamples:\n";
			cout << "\t\ttwinkle --call 123456\n";
			cout << "\t\ttwinkle --call sip:example@example.com?subject=hello\n";
			cout << endl;
			cout << " --cmd <cli command>\n";
			cout << "\t\tInstruct Twinkle to execute the CLI command. You can run\n";
			cout << "\t\tall commands from the command line interface mode.\n";
			cout << "\t\tWhen Twinkle is already running, this will instruct the running\n";
			cout << "\t\tprocess to execute the CLI command.\n";
			cout << endl;
			cout << "\t\tExamples:\n";
			cout << "\t\ttwinkle --cmd answer\n";
			cout << "\t\ttwinkle --cmd mute\n";
			cout << "\t\ttwinkle --cmd 'transfer 12345'\n";
			cout << endl;
			cout << " --immediate";
			cout << "\tThis option can be used in conjunction with --call or --cmd\n";
			cout << "\t\tIt indicates the the command or call is to be performed\n";
			cout << "\t\timmediately without asking the user for any confirmation.\n";
			cout << endl;
			cout << " --set-profile <profile>\n";
			cout << "\t\tMake <profile> the active profile.\n";
			cout << "\t\tWhen using this option in conjunction with --call and --cmd,\n";
			cout << "\t\tthen the profile is activated before executing --call or \n";
			cout << "\t\t--cmd.\n";
			cout << endl;
			cout << " --show";
			cout << "\t\tInstruct a running instance of Twinkle to show the main window\n";
			cout << "\t\tand take focus.\n";
			cout << endl;
			cout << " --hide";
			cout << "\t\tInstruct a running instance of Twinkle to hide in the system tray.\n";
			cout << "\t\tIf no system tray is used, then Twinkle will minimize.\n";
			cout << endl;
			cout << " --help-cli [cli command]\n";
			cout << "\t\tWithout a cli command this option lists all available CLI\n";
			cout << "\t\tcommands. With a CLI command this option prints help on\n";
			cout << "\t\tthe CLI command.\n";
			cout << endl;
			cout << " --version";
			cout << "\tGet version information.\n";
			exit(0);
		} else if (strcmp(argv[i], "--version") == 0) {
			// Get version
			QString s = sys_config->about(false).c_str();
            cout << s.toStdString();
			exit(0);
		} else if (strcmp(argv[i], "-c") == 0) {
			// CLI mode
			cli_mode = true;
		} else if (strcmp(argv[i], "--share") == 0) {
			if (i < argc - 1 && argv[i+1][0] != '-') {
				i++;
				sys_config->set_dir_share(argv[i]);
			} else {
				cout << argv[0] << ": ";
				cout << "Directory missing for option '-share'.\n";
				exit(0);
			}
		} else if (strcmp(argv[i], "-f") == 0) {
			if (i < argc - 1 && argv[i+1][0] != '-') {
				while (i < argc -1 && argv[i+1][0] != '-') {
					i++;
					// Config file name
					QString config_file = argv[i];
					if (!config_file.endsWith(USER_FILE_EXT)) 
					{
						config_file += USER_FILE_EXT;
					}
                    config_files.push_back(config_file.toStdString());
				}
			} else {
				cout << argv[0] << ": ";
				cout << "Config file name missing for option '-f'.\n";
				exit(0);
			}
		} else if (strcmp(argv[i], "--force") == 0) {
			override_lock_file = true;
#if 0
		// DEPRECATED
		} else if (strcmp(argv[i], "-i") == 0) {
			if (i < argc - 1) {
				i++;
				// IP address
				user_host = argv[i];
				if (!exists_interface(user_host)) {
					cout << argv[0] << ": ";
					cout << "There is no interface with IP address ";
					cout << user_host << endl;
					exit(0);
				}
			} else {
				cout << argv[0] << ": ";
				cout << "IP address missing for option '-i'.\n";
				exit(0);
			}
#endif
		} else if (strcmp(argv[i], "--sip-port") == 0) {
			if (i < argc - 1) {
				i++;
				g_cmd_args.override_sip_port = atoi(argv[i]);
			} else {
				cout << argv[0] << ": ";
				cout << "Port missing for option '--sip-port'\n";
			}
		} else if (strcmp(argv[i], "--rtp-port") == 0) {
			if (i < argc - 1) {
				i++;
				g_cmd_args.override_rtp_port = atoi(argv[i]);
			} else {
				cout << argv[0] << ": ";
				cout << "Port missing for option '--rtp-port'\n";
			}
		} else if (strcmp(argv[i], "--call") == 0) {
			if (i < argc - 1) {
				i++;
				// SIP URI
				g_cmd_args.callto_destination = argv[i];
				
				if (g_cmd_args.callto_destination.isEmpty()) {
					cout << argv[0] << ": ";
					cout << "--call argument may not be empty.\n";
					exit(0);
				}
			} else {
				cout << argv[0] << ": ";
				cout << "SIP URI missing for option '--call'.\n";
				exit(0);
			}
		} else if (strcmp(argv[i], "--cmd") == 0) {
			if (i < argc - 1) {
				i++;
				// CLI command
				g_cmd_args.cli_command = argv[i];
				
				if (g_cmd_args.cli_command.isEmpty()) {
					cout << argv[0] << ": ";
					cout << "--cmd argument may not be empty.\n";
					exit(0);
				}
			} else {
				cout << argv[0] << ": ";
				cout << "CLI command missing for option '--cmd'.\n";
				exit(0);
			}
		} else if (strcmp(argv[i], "--immediate") == 0) {
			// Immediate mode
			g_cmd_args.cmd_immediate_mode = true;
		} else if (strcmp(argv[i], "--set-profile") == 0) {
			if (i < argc - 1) {
				i++;
				// Set profile
				g_cmd_args.cmd_set_profile = argv[i];
			} else {
				cout << argv[0] << ": ";
				cout << "Profile missing for option '--set-profile'.\n";
				exit(0);
			}	
		} else if (strcmp(argv[i], "--show") == 0) {
			// Show main window
			g_cmd_args.cmd_show = true;
		} else if (strcmp(argv[i], "--hide") == 0) {
			// Hide main window
			g_cmd_args.cmd_hide = true;
		} else if (strcmp(argv[i], "--help-cli") == 0) {
			string cmd_help("help ");
			if (i < argc -1) {
				i++;
				// Help CLI
				cmd_help += argv[i];
			}
			
			t_phone p;
			t_userintf u(&p);
			u.exec_command(cmd_help);
			exit(0);
		} else {
			// Unknown argument. Assume that it is an Qt/KDE argument.
			remain_argv[remain_argc++] = argv[i];
		}
	}
	
	if (!g_cmd_args.callto_destination.isEmpty() && !g_cmd_args.cli_command.isEmpty()) {
		cout << argv[0] << ": ";
		cout << "--call and --cmd cannot be used at the same time.\n";
		exit(0);
	}
	
	return;
}

bool open_sip_socket(bool cli_mode) {
	QString sock_type;
	
	// Open socket for SIP signaling
	try {
		sock_type = "UDP";
		sip_socket = new t_socket_udp(sys_config->get_sip_port(true));
		MEMMAN_NEW(sip_socket);
		if (sip_socket->enable_icmp()) {
			log_file->write_report("ICMP processing enabled.", "::main");
		} else {
			log_file->write_report("ICMP processing disabled.", "::main");
		}
		
		sock_type = "TCP";
		sip_socket_tcp = new t_socket_tcp(sys_config->get_sip_port());
		MEMMAN_NEW(sip_socket_tcp);	
	} catch (int err) {		
		string msg;
		if (cli_mode) {
			msg = QString("Failed to create a %1 socket (SIP) on port %2")
			   .arg(sock_type)
               .arg(sys_config->get_sip_port()).toStdString();
		} else {
			msg = qApp->translate("GUI", "Failed to create a %1 socket (SIP) on port %2")
			   .arg(sock_type)
               .arg(sys_config->get_sip_port()).toStdString();
		}
		msg += "\n";
		msg += get_error_str(err);
		log_file->write_report(msg, "::main", LOG_NORMAL, LOG_CRITICAL);
		ui->cb_show_msg(msg, MSG_CRITICAL);
		return false;
	}
	
	return true;
}

QApplication *create_user_interface(bool cli_mode, int argc, char **argv, QTranslator *appTranslator, QTranslator *qtTranslator) {
	QApplication *qa = NULL;
	
	if (cli_mode) {
		// CLI mode
		ui = new t_userintf(phone);
		MEMMAN_NEW(ui);
	} else {
		// GUI mode
		
#ifdef HAVE_KDE
		// Store the defualt mime source factory for the embedded icons.
		// This is created by Qt. The KApplication constructor seems to destroy
		// this default.
		Q3MimeSourceFactory *factory_qt = Q3MimeSourceFactory::takeDefaultFactory();
		
		// Initialize the KApplication
		KCmdLineArgs::init(argc, argv, "twinkle", PRODUCT_NAME, "Soft phone",
				   PRODUCT_VERSION, true);
		qa = new t_twinkle_application();
		MEMMAN_NEW(qa);
		
		// Store the KDE mime source factory
		Q3MimeSourceFactory *factory_kde = Q3MimeSourceFactory::takeDefaultFactory();
		
		// Make the Qt factory the default to make the embedded icons work.
		Q3MimeSourceFactory::setDefaultFactory(factory_qt);
		
		// Add the KDE factory
		Q3MimeSourceFactory::addFactory(factory_kde);
#else
		static int tmp = argc;
		qa = new t_twinkle_application(tmp, argv);
		MEMMAN_NEW(qa);
#endif

        g_gui_state = new QSettings(QDir::home().absoluteFilePath(QString("%1/%2").arg(DIR_USER).arg("gui_state.ini")),
                                    QSettings::IniFormat, qa);
		
		// Install Qt translator
		// Do not report to memman as the translator will be deleted
		// automatically when the QApplication is deleted.
		appTranslator = new QTranslator(0);
		qtTranslator = new QTranslator(0);

		QString langName = QLocale::system().name().left(2);

		qDebug() << "Language name:" << langName;
		appTranslator->load(QString("twinkle_") + langName,
			QString(sys_config->get_dir_lang().c_str()));
		qa->installTranslator(appTranslator);
		
		qtTranslator->load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
		qa->installTranslator(qtTranslator);

		qa->setQuitOnLastWindowClosed(false);
		
		// Create translator for translation of strings from the core
		translator = new t_qt_translator(qa);
		MEMMAN_NEW(translator);

		ui = new t_gui(phone);
		MEMMAN_NEW(ui);
	}
	
	return qa;
}

void blockSignals()
{
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
}

int main( int argc, char ** argv )
{
	string error_msg;
	bool cli_mode;
	bool override_lock_file;
	list<string> config_files;
	
	// Initialize globals
	end_app = false;
	
	// Determine threading implementation
	threading_is_LinuxThreads = t_thread::is_LinuxThreads();

	blockSignals();

	QApplication *qa = NULL;
	QTranslator *appTranslator = NULL;
	QTranslator *qtTranslator = NULL;

  // Set phone role
  qputenv("PULSE_PROP_media.role", "phone");
	
	// Store id of main thread
	thread_id_main = t_thread::self();
	
	memman = new t_memman();
	MEMMAN_NEW(memman);
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
	
	// Create system configuration object
	sys_config = new t_sys_settings();
	MEMMAN_NEW(sys_config);
	
	// Parse command line arguments
	int remain_argc = 0;
	char **remain_argv = NULL;
	parse_main_args(argc, argv, cli_mode, override_lock_file, config_files, remain_argc, remain_argv);
	sys_config->set_override_sip_port(g_cmd_args.override_sip_port);
	sys_config->set_override_rtp_port(g_cmd_args.override_rtp_port);
	
	// Checking the environment and creating the lock is done at
	// this early stage to improve performance of the --call parameter.
	// Creation of the QApplication object for the GUI is slow.
	// However for errors, the user interface must be created to give
	// either a message box or text formatted error.
	
	// Check requirements on environment
	// If check fails, then display error after user interface has been
	// created.
	string env_error_msg;
	bool env_check_ok = sys_config->check_environment(env_error_msg);
	
	// Create a lock file to guarantee that the application runs only once.
	bool already_running;
	bool lock_created = false;
	string lock_error_msg;	
	if (env_check_ok &&
	    !(lock_created = sys_config->create_lock_file(false, lock_error_msg, already_running))) 
	{
		bool must_exit = false;
		
		// Show the main window of the running Twinkle process.
		if (already_running && g_cmd_args.cmd_show) {
			cmdsocket::cmd_show();
			must_exit = true;
		}
		
		// Hide the main window of the running Twinkle process.
		if (already_running && g_cmd_args.cmd_hide) {
			cmdsocket::cmd_hide();
			must_exit = true;
		}
		
		// Activate a profile in the running Twinkle process.
		if (already_running && !g_cmd_args.cmd_set_profile.isEmpty()) {
            cmdsocket::cmd_cli(string("user ") + g_cmd_args.cmd_set_profile.toStdString(), true);
			// Do not exit now as this option may be used in conjunction
			// with --call or --cmd
			must_exit = true;
		}
		
		// If Twinkle is running already and the --call parameter
		// is present, then send the call destination to the running
		// Twinkle process.
		if (already_running && !g_cmd_args.callto_destination.isEmpty()) {
            cmdsocket::cmd_call(g_cmd_args.callto_destination.toStdString(),
					    g_cmd_args.cmd_immediate_mode);
			exit(0);
		}
		
		// If the --cmd parameter is present, send the cli command
		// to the running Twinkle process
		if (already_running && !g_cmd_args.cli_command.isEmpty()) {
            cmdsocket::cmd_cli(g_cmd_args.cli_command.toStdString(),
					   g_cmd_args.cmd_immediate_mode);
			exit(0);
		}
		
		// Exit if an instruction for a running instance was given.
		if (must_exit) {
			exit(0);
		}
	}
	
	// Read system configuration
	bool sys_config_read = sys_config->read_config(error_msg);
	qa = create_user_interface(cli_mode, remain_argc, remain_argv, appTranslator, qtTranslator);
	if (!sys_config_read) {
		ui->cb_show_msg(error_msg, MSG_CRITICAL);
		exit(1);
	}

	user_host = AUTO_IP4_ADDRESS;
	local_hostname = get_local_hostname();
	
	if (!env_check_ok) {
		// Environment is not good
		// Call the check_environment once more to get proper translation
		// of the error message. The previous check was done before
		// the QApplication was created.
		(void)sys_config->check_environment(env_error_msg);
		ui->cb_show_msg(env_error_msg, MSG_CRITICAL);
		exit(1);
	}
	
	// Show error if lock file could not be created
	if (!lock_created) {	
		string msg;
		// Call create lock file once more to get proper translation of
		// error message.
		if (!sys_config->create_lock_file(false, msg, already_running)) {
			if (already_running) {
				if (!cli_mode) {
					msg += "\n\n";
					msg += qApp->translate("GUI",
                        "Override lock file and start anyway?").toStdString();
				}
				if (override_lock_file || ui->cb_ask_msg(msg, MSG_WARNING)) {
					sys_config->delete_lock_file();
					if (!sys_config->create_lock_file(true, msg, 
						already_running))
					{
						ui->cb_show_msg(msg, MSG_CRITICAL);
						exit(1);
					}
				} else {
					exit(1);
				}
			} else {
				ui->cb_show_msg(msg, MSG_CRITICAL);
				exit(1);
			}
		}
		// If for some obscure reason the lock file could be
		// created this time, then continue.
	}
	
	// Create log file
	log_file = new t_log();
	MEMMAN_NEW(log_file);
	
	// Write threading implementation to log file. May be useful for debugging.
	if (threading_is_LinuxThreads) {
		log_file->write_report("Threading implementation is LinuxThreads.",
			"::main", LOG_NORMAL, LOG_INFO);
	} else {
		log_file->write_report("Threading implementation is NPTL.",
			"::main", LOG_NORMAL, LOG_INFO);
	}
	
	// Check if the previous Twinkle session was stopped by a system
	// shutdow and now gets restored.
	if (qa && qa->isSessionRestored()) {
		QString msg = "Restore session: " + qa->sessionId();
        log_file->write_report(msg.toStdString(), "::main");
		
        if (sys_config->get_ui_session_id() == qa->sessionId().toStdString()) {
			config_files = sys_config->get_ui_session_active_profiles();
			// Note: the GUI state is restore in t_gui::run()
		} else {
			log_file->write_header("::main", LOG_NORMAL, LOG_WARNING);
			log_file->write_raw("Cannot restore session.\n");
			log_file->write_raw("Stored session id: ");
			log_file->write_raw(sys_config->get_ui_session_id());
			log_file->write_endl();
			log_file->write_footer();
		}
	}
	
	// Get default values from system configuration
	if (config_files.empty()) {
		list<string> start_user_profiles = sys_config->get_start_user_profiles();
		for (list<string>::iterator i = start_user_profiles.begin();
		i != start_user_profiles.end(); i++)
		{
			QString config_file = (*i).c_str();
			config_file += USER_FILE_EXT;
            config_files.push_back(config_file.toStdString());
		}
	}
	
	bool profile_selected = false;
	while(!profile_selected) {
		// Select user profile
		if (config_files.empty()) {
			if (!ui->select_user_config(config_files)) {
				sys_config->delete_lock_file();
				exit(1);
			}
		}
		
		for (list<string>::iterator i = config_files.begin();
		i != config_files.end(); i++)
		{	
			t_user user_config;
			
			// Read user configuration
			if (user_config.read_config(*i, error_msg)) {
				t_user *dup_user;
				if (phone->add_phone_user(
						user_config, &dup_user))
				{
					profile_selected = true;
				} else {
					if (cli_mode) {
                        error_msg = QString("The following profiles are both for user %1").arg(user_config.get_name().c_str()).toStdString();
					} else {
                        error_msg = qApp->translate("GUI", "The following profiles are both for user %1").arg(user_config.get_name().c_str()).toStdString();
					}
					error_msg += ":\n\n";
					error_msg += user_config.get_profile_name();
					error_msg += "\n";
					error_msg += dup_user->get_profile_name();
					error_msg += "\n\n";
					if (cli_mode) {
                        error_msg += QString("You can only run multiple profiles for different users.").toStdString();
						error_msg += "\n";
                        error_msg += QString("If these are users for different domains, then enable the following option in your user profile (SIP protocol):").toStdString();
						error_msg += "\n";
                        error_msg += QString("Use domain name to create a unique contact header").toStdString();
					} else {
                        error_msg += qApp->translate("GUI", "You can only run multiple profiles for different users.").toStdString();
						error_msg += "\n";
                        error_msg += qApp->translate("GUI", "If these are users for different domains, then enable the following option in your user profile (SIP protocol)").toStdString();
						error_msg += ":\n";
                        error_msg += qApp->translate("GUI", "Use domain name to create a unique contact header").toStdString();
					}
					ui->cb_show_msg(error_msg, MSG_CRITICAL);
					profile_selected = false;
					break;
				}
			} else {
				ui->cb_show_msg(error_msg, MSG_CRITICAL);
				profile_selected = false;
				break;
			}
		}
		
		if (profile_selected && !open_sip_socket(cli_mode)) {
			// Opening SIP socket failed. Let user pick a user profile
			// again, so he can make changes in settings to fix the error.
			profile_selected = false;
		}
		
		// In CLI mode the user cannot select another profile.
		if (!profile_selected) {
			if (cli_mode) {
				sys_config->delete_lock_file();
				exit(1);
			}
		}
		
		config_files.clear();
	}
	
	// Initialize RTP port settings.
	phone->init_rtp_ports();
	
	// Create call history
	call_history = new t_call_history();
	MEMMAN_NEW(call_history);
	
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
	
	// Preload the address finder (Akonadi is only available in GUI mode)
	if (!cli_mode) {
		t_address_finder::preload();
	}
	
	// Create mime database
	mime_database = new t_mime_database();
	MEMMAN_NEW(mime_database);
	if (!mime_database->load(error_msg)) {
		log_file->write_report(error_msg, "::main", LOG_NORMAL, LOG_WARNING);
	}
	
	// Discover NAT type if STUN is enabled
	list<t_user *> user_list = phone->ref_users();
	ui->cb_nat_discovery_progress_start(user_list.size());
	list<string> msg_list;
	int progressStep = 0;
	for (list<t_user *>::iterator i = user_list.begin(); i != user_list.end(); i++) {
		ui->cb_nat_discovery_progress_step(progressStep);
		
		if (ui->cb_nat_discovery_cancelled()) {
			log_file->write_report("User aborted NAT discovery.", "::main");
			sys_config->delete_lock_file();
			exit(1);
		}
		
		if (!phone->stun_discover_nat(*i, error_msg)) {
			msg_list.push_back(error_msg);
		}
		
		progressStep++;
	}
	ui->cb_nat_discovery_finished();
	
	for (list<string>::iterator i = msg_list.begin();
	     i != msg_list.end(); i++)
	{
		ui->cb_show_msg(*i, MSG_WARNING);
	}
	
	// Open socket for external commands from the command line
	string cmd_sock_name = sys_config->get_dir_user();
	cmd_sock_name += '/';
	cmd_sock_name += CMD_SOCKNAME;
	t_socket_local *sock_cmd = NULL;
	try {

		
		// The local socket may still exist if Twinkle got killed
		// previously, so remove it if it is still there.
		unlink(cmd_sock_name.c_str());
		
		sock_cmd = new t_socket_local();
		MEMMAN_NEW(sock_cmd);
		sock_cmd->bind(cmd_sock_name);
		sock_cmd->listen(5);
		
		string log_msg = "Created local socket: ";
		log_msg += cmd_sock_name;
		log_file->write_report(log_msg, "::main");
	}
	catch (int e) {
		if (sock_cmd) {
			MEMMAN_DELETE(sock_cmd);
			delete sock_cmd;
			sock_cmd = NULL;
		}
		string log_msg = "Failed to create local socket: ";
		log_msg += cmd_sock_name;
		log_msg += "\n";
		log_msg += get_error_str(e);
		log_msg += "\n";
		log_file->write_report(log_msg, "::main", LOG_NORMAL, LOG_WARNING);
	}
				 
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
	t_thread *thr_listen_cmd = NULL;
	
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
		
		// External command listener thread
		if (sock_cmd) {
			thr_listen_cmd = new t_thread(cmdsocket::listen_cmd, sock_cmd);
			MEMMAN_NEW(thr_listen_cmd);
		}
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

	// Start UI event loop (CLI/QApplication/KApplication)
	try {
		ui->run();
	} catch (string e) {
		string msg = "Exception: ";
		msg += e;
		log_file->write_report(msg, "::main", LOG_NORMAL, LOG_CRITICAL);
		ui->cb_show_msg(msg, MSG_CRITICAL);
		sys_config->delete_lock_file();
		exit(1);
	} catch (int e) {
		string msg = "Error code exception: ";
		msg += e;
		log_file->write_report(msg, "::main", LOG_NORMAL, LOG_CRITICAL);
		ui->cb_show_msg(msg, MSG_CRITICAL);
		sys_config->delete_lock_file();
		exit(1);
	} catch (const std::exception& e) {
		string msg = "std::exception exception: ";
		msg += e.what();
		msg += " (";
		msg += typeid(e).name();
		msg += ")";
		
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
	
	// Terminate threads
	// Kill the threads getting receiving input from the outside world first,
	// so no new inputs come in during termination.
	if (thr_listen_cmd) {
		thr_listen_cmd->cancel();
		thr_listen_cmd->join();
		log_file->write_report("thr_listen_cmd stopped.", "::main", LOG_NORMAL, LOG_DEBUG);
	}
	
	thr_listen_udp->cancel();
	thr_listen_udp->join();
	log_file->write_report("thr_listen_udp stopped.", "::main", LOG_NORMAL, LOG_DEBUG);
	
	thr_listen_conn_tcp->cancel();
	thr_listen_conn_tcp->join();
	log_file->write_report("thr_listen_conn_tcp stopped.", "::main", LOG_NORMAL, LOG_DEBUG);
	
	connection_table->cancel_select();
	thr_listen_data_tcp->join();
	log_file->write_report("thr_listen_data_tcp stopped.", "::main", LOG_NORMAL, LOG_DEBUG);
	thr_conn_timeout_handler->join();
	log_file->write_report("thr_conn_timeout_handler stopped.", "::main", LOG_NORMAL, LOG_DEBUG);
	
	thr_tcp_sender->join();
	log_file->write_report("thr_tcp_sender stopped.", "::main", LOG_NORMAL, LOG_DEBUG);
	
	evq_trans_layer->push_quit();
	thr_phone_uas->join();
	log_file->write_report("thr_phone_uas stopped.", "::main", LOG_NORMAL, LOG_DEBUG);
	
	evq_trans_mgr->push_quit();
	thr_trans_mgr->join();
	
	if (!threading_is_LinuxThreads) {
		try {
			thr_sig_catcher->cancel();
		} catch (int) {
			// Thread terminated already by itself
		}
		thr_sig_catcher->join();
		log_file->write_report("thr_sig_catcher stopped.", "::main", 
				       LOG_NORMAL, LOG_DEBUG);
		
		thr_alarm_catcher->cancel();
		thr_alarm_catcher->join();
		log_file->write_report("thr_alarm_catcher stopped.", "::main", 
				       LOG_NORMAL, LOG_DEBUG);
	}
	
	evq_timekeeper->push_quit();
	thr_timekeeper->join();
	log_file->write_report("thr_timekeeper stopped.", "::main", LOG_NORMAL, LOG_DEBUG);
	
	evq_sender->push_quit();
	thr_sender->join();
	log_file->write_report("thr_sender stopped.", "::main", LOG_NORMAL, LOG_DEBUG);
	
	sys_config->remove_all_tmp_files();
	
	if (thr_listen_cmd) {
		MEMMAN_DELETE(thr_listen_cmd);
		delete thr_listen_cmd;
	}

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
	call_history = NULL;

	MEMMAN_DELETE(ui);
	delete ui;
	ui = NULL;
	
	MEMMAN_DELETE(connection_table);
	delete connection_table;
	MEMMAN_DELETE(sip_socket_tcp);
	delete sip_socket_tcp;
	MEMMAN_DELETE(sip_socket);
	delete sip_socket;
	
	if (sock_cmd) {
		MEMMAN_DELETE(sock_cmd);
		delete sock_cmd;
		unlink(cmd_sock_name.c_str());
	}

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
	
	if (translator) {
		MEMMAN_DELETE(translator);
		delete translator;
		translator = NULL;
	}
	
	if (appTranslator) {
		MEMMAN_DELETE(appTranslator);
		delete(appTranslator);
	}
	
	if (qa) {
		MEMMAN_DELETE(qa);
		delete qa;
	}

	// Report memory leaks
	// Report deletion of log_file and sys_config already to get a correct
	// report.
	MEMMAN_DELETE(sys_config);
	MEMMAN_DELETE(log_file);
	MEMMAN_DELETE(memman);
	MEMMAN_REPORT;
	
	delete log_file;
	delete memman;
	
	sys_config->delete_lock_file();
	delete sys_config;
}
