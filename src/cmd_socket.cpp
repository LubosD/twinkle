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

#include <cstdlib>
#include <cstring>
#include <iostream>
#include "cmd_socket.h"
#include "log.h"
#include "sys_settings.h"
#include "userintf.h"
#include "util.h"
#include "audits/memman.h"
#include "sockets/socket.h"

namespace cmdsocket {

/** Command opcodes */
enum t_cmd_code {
	CMD_CALL,	/**< Call */
	CMD_CLI,	/**< Any CLI command */
	CMD_SHOW,	/**< Show Twinkle */
	CMD_HIDE	/**< Hide Twinkle */
};

string cmd_code2str(t_cmd_code opcode) {
	switch (opcode) {
	case CMD_CALL:
		return "CALL";
	case CMD_CLI:
		return "CLI";
	case CMD_SHOW:
		return "SHOW";
	case CMD_HIDE:
		return "HIDE";
	default:
		return "UNKNOWN";
	}
}

void exec_cmd(t_socket_local &sock_client) {
	t_cmd_code opcode;
	bool immediate;
	int len;
	string log_msg;

	try {
		if (sock_client.read(&opcode, sizeof(opcode)) != sizeof(opcode)) {
			log_file->write_report("Failed to read opcode from socket.",
				"cmdsocket::exec_cmd", LOG_NORMAL, LOG_WARNING);
			return;
		}
		
		if (sock_client.read(&immediate, sizeof(immediate)) != sizeof(immediate)) {
			log_file->write_report("Failed to read immediate mode from socket.",
				"cmdsocket::exec_cmd", LOG_NORMAL, LOG_WARNING);
			return;
		}
	
		if (sock_client.read(&len, sizeof(len)) != sizeof(len)) {
			log_file->write_report("Failed to read length from socket.",
				"cmdsocket::exec_cmd", LOG_NORMAL, LOG_WARNING);
			return;
		}
		
		char args[len];
		
		if (sock_client.read(args, len) != len) {
			log_file->write_report("Failed to read arguments from socket.",
				"cmdsocket::exec_cmd", LOG_NORMAL, LOG_WARNING);
			return;
		}
		
		log_file->write_header("cmdsocket::exec_cmd", LOG_NORMAL, LOG_DEBUG);
		log_file->write_raw("External command received:\n");
		log_file->write_raw("Opcode: ");
		log_file->write_raw(cmd_code2str(opcode));
		log_file->write_raw("\nImmediate: ");
		log_file->write_raw(bool2yesno(immediate));
		log_file->write_raw("\nArguments: ");
		log_file->write_raw(args);
		log_file->write_endl();
		log_file->write_footer();
		
		switch (opcode) {
		case CMD_CALL:
			ui->cmd_call(args, immediate);
			break;
		case CMD_CLI:
			ui->cmd_cli(args, immediate);
			break;
		case CMD_SHOW:
			ui->cmd_show();
			break;
		case CMD_HIDE:
			ui->cmd_hide();
			break;
		default:
			// Discard unknown commands
			log_file->write_header("cmdsocket::exec_cmd", LOG_NORMAL, LOG_WARNING);
			log_file->write_raw("Unknown external command received:\n");
			log_file->write_raw("Opcode: ");
			log_file->write_raw(cmd_code2str(opcode));
			log_file->write_raw("\nImmediate: ");
			log_file->write_raw(bool2yesno(immediate));
			log_file->write_raw("\nArguments: ");
			log_file->write_raw(args);
			log_file->write_endl();
			log_file->write_footer();
			break;
		}
	}
	catch (int e) {
		log_msg = "Failed to read from socket.\n";
		log_msg += get_error_str(e);
		log_msg += "\n";
		log_file->write_report(log_msg, "cmdsocket::exec_cmd", LOG_NORMAL, LOG_WARNING);
	}
}

void *listen_cmd(void *arg) {
	t_socket_local *sock_cmd = (t_socket_local *)arg;
	string log_msg;
	
	while (true) {
		try {
			int fd = sock_cmd->accept();
			t_socket_local sock_client(fd);
			exec_cmd(sock_client);
		}
		catch (int e) {
			log_msg = "Accept failed on socket.\n";
			log_msg += get_error_str(e);
			log_msg += "\n";
			log_file->write_report(log_msg, "cmdsocket::listen_cmd", LOG_NORMAL, 
				LOG_WARNING);
			return NULL;
		}
	}
}

void write_cmd_to_socket(t_cmd_code opcode, bool immediate, const string &args) {
	string name = sys_config->get_dir_user();
	name += '/';
	name += CMD_SOCKNAME;

	try {
		t_socket_local sock_cmd;
		sock_cmd.connect(name);
		sock_cmd.write(&opcode, sizeof(opcode));
		sock_cmd.write(&immediate, sizeof(immediate));
		int len = args.size() + 1;
		sock_cmd.write(&len, sizeof(len));
		char *buf = strdup(args.c_str());
		MEMMAN_NEW(buf);
		sock_cmd.write(buf, len);
		MEMMAN_DELETE(buf);
		free(buf);
	}
	catch (int e) {
		// This function will be called from Twinkle when it
		// notices another Twinkle is already running. In that
		// case this process does not have a log file. So write
		// errors to stderr
		cerr << "Failed to send " << cmd_code2str(opcode) << " command to " << name << endl;
		cerr << get_error_str(e) << endl;
	}
}

void cmd_call(const string &destination, bool immediate) {
	write_cmd_to_socket(CMD_CALL, immediate, destination);
}

void cmd_cli(const string &cli_command, bool immediate) {
	write_cmd_to_socket(CMD_CLI, immediate, cli_command);
}

void cmd_show(void) {
	write_cmd_to_socket(CMD_SHOW, true, "");
}

void cmd_hide(void) {
	write_cmd_to_socket(CMD_HIDE, true, "");
}

}
