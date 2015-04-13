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

#ifndef _COMMAND_ARGS_H
#define _COMMAND_ARGS_H

#include "twinkle_config.h"

/** Command arguments. */
struct t_command_args {
	/** SIP URI to be called passed via the --call command line parameter. */
	QString			callto_destination;
	
	/** CLI command passed via the --cmd command line parameter. */
	QString			cli_command;
	
	/** Indicates if the --call or --cmd must be performed immediately. */
	bool			cmd_immediate_mode;
	
	/** Indicates the profile that should be made active before performing
	 * --call or --cmd
	 */
	QString			cmd_set_profile;
	
	/** Indicates if the --show option was given. */
	bool			cmd_show;
	
	/** Indicates if the --hide option was given. */
	bool			cmd_hide;
	
	/** If a port number is passed by the user on the command line, then
	 * that port number overrides the port from the system settings.
	 */
	unsigned short		override_sip_port;
	unsigned short		override_rtp_port;
	
	t_command_args() :
			cmd_immediate_mode(false),
			cmd_show(false),
			cmd_hide(false),
			override_sip_port(0),
			override_rtp_port(0)
	{}
};

#endif
