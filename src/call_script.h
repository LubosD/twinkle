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
 * Call scripting interface.
 * A call script is called by Twinkle during call processing.
 * Currently only when a call comes in (INVITE received).
 * Twinkle calls the script and based of the output of the script, the
 * call is further handled.
 * 
 * The following environment variables are passed to the script:
 * 
@verbatim
   TWINKLE_USER_PROFILE=<user profile name>
   TWINKLE_TRIGGER=<trigger type>
   TWINKLE_LINE=<line number (starting at 1) associated with the call>
   SIPREQUEST_METHOD=<method>
   SIPREQUEST_URI=<request uri>
   SIPSTATUS_CODE=<status code of a response>
   SIPSTATUS_REASON=<reason phrase of a response>
   SIP_FROM_USER=<user name of From header>
   SIP_FROM_HOST=<host part of From header>
   SIP_TO_USER=<user name of To header>
   SIP_TO_HOST=<host part of To header>
   SIP_<header_name>=<header value>
@endverbatim
 * 
 * The header name is in capitals and dashed are replaced by underscores
 * 
 * The script can return on stdout how the call should be further
 * processed. The following output parameters are recognized:
 * 
@verbatim
   action=[continue|reject|dnd|redirect]
   reason=<reason phrase>, for reject and dnd actions
   contact=<sip uri>, for redirect ation
   ringtone=<name of wav file>, for continue action
   caller_name=<name to override the display name of the caller>
   display_msg=<msg to show in Twinkle's display> (may occur multiple times)
   end This parameter makes Twinkle stop waiting for the script to complete.
@endverbatim
 * 
 * If no action is returned, the "continue" action is performed.
 * Invalid output will be skipped.
 */

#ifndef _H_CALL_SCRIPT
#define _H_CALL_SCRIPT

#include <vector>
#include <string>
#include <cc++/config.h>
#include "user.h"
#include "parser/request.h"

using namespace std;

/** Results of the incoming call script. */
class t_script_result {
public:
	/** Action to perform. */
	enum t_action {
		ACTION_CONTINUE, 	/**< Continue with incoming call */
		ACTION_REJECT, 		/**< Reject incoming call with 603 response */
		ACTION_DND,		/**< Do not disturb, send 480 response */
		ACTION_REDIRECT,	/**< Redirect call (302 response) */
		ACTION_AUTOANSWER,	/**< Auto answer incoming call */
		ACTION_ERROR		/**< Fail call due to error (500 response) */
	};
	
	/** @name Output parameters */
	//@{
	t_action	action;		/**< How to proceed with call */
	string		reason;		/**< Reason if call is not continued */
	string		contact;	/**< Redirect destination for redirect action */
	string		caller_name;	/**< Name of caller (can be used to override display name) */
	string		ringtone;	/**< Wav file for ring tone */
	vector<string>	display_msgs;	/**< Message (multi line) to show on display */
	//@}
	
	/** Constructor. */
	t_script_result();
	
	/**
	 * Convert string representation to an action.
	 * @param action_string [in] String representation of an action.
	 * @return The action.
	 */
	static t_action str2action(const string action_string);
	
	/** Clear the results. */
	void clear(void);
	
	/**
	 * Set output parameter from values read from the result output of a script.
	 * @param parameter [in] Name of the parameter to set,
	 * @param value [in] The value to set.
	 */
	void set_parameter(const string &parameter, const string &value);
};

/** Call script definition. */
class t_call_script {
public:
	/** Trigger type. */
	enum t_trigger {
		TRIGGER_IN_CALL,		/**< Incoming call. */
		TRIGGER_IN_CALL_ANSWERED,	/**< Incoming call answered. */
		TRIGGER_IN_CALL_FAILED,		/**< Incoming call failed. */
		TRIGGER_OUT_CALL,		/**< Outgoing call made. */
		TRIGGER_OUT_CALL_ANSWERED,	/**< Outgoing call answered. */
		TRIGGER_OUT_CALL_FAILED,	/**< Outgoing call failed. */
		TRIGGER_LOCAL_RELEASE,		/**< Call released by local party. */
		TRIGGER_REMOTE_RELEASE		/**< Call released by remotre party. */
	};
	
private:
	t_user		*user_config;		/**< The user profile. */
	string		script_command;		/**< The script to execute. */
	t_trigger	trigger;		/**< Trigger point for this script. */
	
	/**
	 * Number of the line associated with the call causing the trigger.
	 * The line numbers start at 1. For some triggers a line number does not
	 * apply, e.g. incoming call and all lines are busy. In that case the
	 * line number is 0.
	 */
	uint16		line_number;
	
	/**
	 * Convert a trigger type value to a string.
	 * @param t [in] Trigger
	 * @return String representation for the trigger.
	 */
	string trigger2str(t_trigger t) const;
	
	/**
	 * Create environment for the process running the script.
	 * The environment contains the header values of a SIP message.
	 * @param m [in] The SIP message.
	 * @return The environment.
	 * @note This function creates the env array without registering
	 *       the memory allocation to MEMMAN.
	 */
	char **create_env(t_sip_message *m) const;
	
	/**
	 * Create script command argument list.
	 * @return The argument list.
	 * @note This function creates the argv array without registering
	 *       the memory allocation to MEMMAN.
	 */
	char **create_argv(void) const;
	
protected:
	/** Cannot use this constructor. */
	t_call_script() {};
	
public:
	/** 
	 * Constructor. 
	 * @param _user_config [in] User profile associated with the trigger.
	 * @param _trigger [in] The trigger type.
	 * @param _line_number [in] Line associated with the trigger (0 if no line
	 * is associated).
	 */
	t_call_script(t_user *_user_config, t_trigger _trigger, uint16 _line_number);
	
	/**
	 * Execute call script resulting in an action.
	 * @param result [out] Contains the result on return.
	 * @param m [in] The SIP message triggering this call script.
	 */
	void exec_action(t_script_result &result, t_sip_message *m) const;
	
	/**
	 * Execute notification call script.
	 * @param m [in] The SIP message triggering this call script.
	 */
	void exec_notify(t_sip_message *m) const;
};

#endif
