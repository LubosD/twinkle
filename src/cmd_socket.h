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

/**
 * @file
 * Twinkle listens on a local socket for external commands.
 */

#ifndef _H_CMD_SOCKET
#define _H_CMD_SOCKET

#include <string>

/** Name of the local socket. */
#define CMD_SOCKNAME	".cmdsock"

using namespace std;

namespace cmdsocket {

/**
 * Listen on local socket for commands.
 * @param arg A local socket (@ref t_socket_local)
 */
void *listen_cmd(void *arg);

/**
 * Send call command to the local socket.
 * @param destination The SIP destination to call.
 * @param immediate Indicates if the call should be made immediately
 * without asking the user for confirmation.
 */
void cmd_call(const string &destination, bool immediate);

/**
 * Send a CLI command to the local socket.
 * @param  cli_command The CLI command to send.
 * @param immediate Indicates if the call should be made immediately
 * without asking the user for confirmation.
 */
void cmd_cli(const string &cli_command, bool immediate);

/** Send show command to the local socket. */
void cmd_show(void);

/** Send hide command to the local socket. */
void cmd_hide(void);

}

#endif
