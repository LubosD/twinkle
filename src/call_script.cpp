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

#include <cstdio>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include "call_script.h"
#include "log.h"
#include "userintf.h"
#include "util.h"

// Maximum length of the reason value
#define MAX_LEN_REASON		50

// Script result fields
#define SCR_ACTION		"action"
#define SCR_REASON		"reason"
#define SCR_CONTACT		"contact"
#define SCR_CALLER_NAME		"caller_name"
#define SCR_RINGTONE		"ringtone"
#define SCR_DISPLAY_MSG		"display_msg"
#define SCR_INTERNAL_ERROR	"internal_error"

// Script triggers
#define SCR_TRIGGER_IN_CALL		"in_call"
#define SCR_TRIGGER_IN_CALL_ANSWERED	"in_call_answered"
#define SCR_TRIGGER_IN_CALL_FAILED	"in_call_failed"
#define SCR_TRIGGER_OUT_CALL		"out_call"
#define SCR_TRIGGER_OUT_CALL_ANSWERED	"out_call_answered"
#define SCR_TRIGGER_OUT_CALL_FAILED	"out_call_failed"
#define SCR_TRIGGER_LOCAL_RELEASE	"local_release"
#define SCR_TRIGGER_REMOTE_RELEASE	"remote_release"

/////////////////////////
// class t_script_result
/////////////////////////

t_script_result::t_script_result() {
	clear();
}

t_script_result::t_action t_script_result::str2action(const string action_string) {
	string s = tolower(action_string);
	
	t_action result;	
	if (s == "continue") {
		result = ACTION_CONTINUE;
	} else if (s == "reject") {
		result = ACTION_REJECT;
	} else if (s == "dnd") {
		result = ACTION_DND;
	} else if (s == "redirect") {
		result = ACTION_REDIRECT;
	} else if (s == "autoanswer") {
		result = ACTION_AUTOANSWER;
	} else {
		// Unknown action
		result = ACTION_ERROR;
	}
	
	return result;
}

void t_script_result::clear(void) {
	action = ACTION_CONTINUE;
	reason.clear();
	contact.clear();
	caller_name.clear();
	ringtone.clear();
	display_msgs.clear();
}

void t_script_result::set_parameter(const string &parameter, const string &value) {
	if (parameter == SCR_ACTION) {
		action = str2action(value);
	} else if (parameter == SCR_REASON) {
		if (value.size() <= MAX_LEN_REASON) {
			reason = value;
		} else {
			reason = value.substr(0, MAX_LEN_REASON);
		}
	} else if (parameter == SCR_CONTACT) {
		contact = value;
	} else if (parameter == SCR_CALLER_NAME) {
		caller_name = value;
	} else if (parameter == SCR_RINGTONE) {
		ringtone = value;
	} else if (parameter == SCR_DISPLAY_MSG) {
		display_msgs.push_back(value);
	}
	// Unknown parameters are ignored
}

/////////////////////////
// class t_call_script
/////////////////////////

string t_call_script::trigger2str(t_trigger t) const {
	switch (t) {
	case TRIGGER_IN_CALL:
		return SCR_TRIGGER_IN_CALL;
	case TRIGGER_IN_CALL_ANSWERED:
		return SCR_TRIGGER_IN_CALL_ANSWERED;
	case TRIGGER_IN_CALL_FAILED:
		return SCR_TRIGGER_IN_CALL_FAILED;
	case TRIGGER_OUT_CALL:
		return SCR_TRIGGER_OUT_CALL;
	case TRIGGER_OUT_CALL_ANSWERED:
		return SCR_TRIGGER_OUT_CALL_ANSWERED;
	case TRIGGER_OUT_CALL_FAILED:
		return SCR_TRIGGER_OUT_CALL_FAILED;
	case TRIGGER_LOCAL_RELEASE:
		return SCR_TRIGGER_LOCAL_RELEASE;
	case TRIGGER_REMOTE_RELEASE:
		return SCR_TRIGGER_REMOTE_RELEASE;
	default:
		return "unknown";
	}
}

char **t_call_script::create_env(t_sip_message *m) const {
		string var_twinkle;

		// Number of existing environment variables
		int environ_size = 0;
		for (int i = 0; environ[i] != NULL; i++) {
			environ_size++;
		}
		
		// Number of SIP environment variables
		int start_sip_env = environ_size; // Position of SIP variables
		list<string> l = m->encode_env();
		
		var_twinkle = "SIP_FROM_USER=";
		var_twinkle += m->hdr_from.uri.get_user();
		l.push_back(var_twinkle);
		
		var_twinkle = "SIP_FROM_HOST=";
		var_twinkle += m->hdr_from.uri.get_host();
		l.push_back(var_twinkle);
		
		var_twinkle = "SIP_TO_USER=";
		var_twinkle += m->hdr_to.uri.get_user();
		l.push_back(var_twinkle);
		
		var_twinkle = "SIP_TO_HOST=";
		var_twinkle += m->hdr_to.uri.get_host();
		l.push_back(var_twinkle);
		
		environ_size += l.size();
		
		// Number of Twinkle environment variables
		int start_twinkle_env = environ_size; // Position of Twinkle variables
		environ_size += 3;
		
		// MEMMAN not called on purpose
		char **env = new char *[environ_size + 1];
		
		// Copy current environment to child
		for (int i = 0; environ[i] != NULL; i++) {
			env[i] = strdup(environ[i]);
		}
		
		// Add environment variables for SIP request
		int j = start_sip_env;
		for (list<string>::iterator i = l.begin(); i != l.end(); i++, j++) {
			env[j] = strdup(i->c_str());
		}
		
		// Add Twinkle specific environment variables
		var_twinkle = "TWINKLE_USER_PROFILE=";
		var_twinkle += user_config->get_profile_name();
		env[start_twinkle_env] = strdup(var_twinkle.c_str());
		
		var_twinkle = "TWINKLE_TRIGGER=";
		var_twinkle += trigger2str(trigger);
		env[start_twinkle_env + 1] = strdup(var_twinkle.c_str());
		
		var_twinkle = "TWINKLE_LINE=";
		var_twinkle += ulong2str(line_number);
		env[start_twinkle_env + 2] = strdup(var_twinkle.c_str());
		
		// Terminate array with NULL
		env[environ_size] = NULL;
		
		return env;
}

char **t_call_script::create_argv(void) const {
		// Determine script agument list
		vector<string> arg_list = split_ws(script_command, true);
		
		// MEMMAN not called on purpose
		char **argv = new char *[arg_list.size() + 1];
		
		int idx = 0;
		for (vector<string>::iterator i = arg_list.begin(); 
		     i != arg_list.end(); i++, idx++) 
		{
			argv[idx] = strdup(i->c_str());
		}
		argv[arg_list.size()] = NULL;
		
		return argv;
}

t_call_script::t_call_script(t_user *_user_config, t_trigger _trigger, uint16 _line_number) :
	user_config(_user_config),
	trigger(_trigger),
	line_number(_line_number)
{
	switch (trigger) {
	case TRIGGER_IN_CALL:
		script_command = user_config->get_script_incoming_call();
		break;
	case TRIGGER_IN_CALL_ANSWERED:
		script_command = user_config->get_script_in_call_answered();
		break;
	case TRIGGER_IN_CALL_FAILED:
		script_command = user_config->get_script_in_call_failed();
		break;
	case TRIGGER_OUT_CALL:
		script_command = user_config->get_script_outgoing_call();
		break;
	case TRIGGER_OUT_CALL_ANSWERED:
		script_command = user_config->get_script_out_call_answered();
		break;
	case TRIGGER_OUT_CALL_FAILED:
		script_command = user_config->get_script_out_call_failed();
		break;
	case TRIGGER_LOCAL_RELEASE:
		script_command = user_config->get_script_local_release();
		break;
	case TRIGGER_REMOTE_RELEASE:
		script_command = user_config->get_script_remote_release();
		break;
	default:
		script_command.clear();
		break;
	}
}

void t_call_script::exec_action(t_script_result &result, t_sip_message *m) const 
{
	result.clear();
	
	if (script_command.empty()) return;
	
	log_file->write_header("t_call_script::exec_action");
	log_file->write_raw("Execute script: ");
	log_file->write_raw(script_command);
	log_file->write_raw("\nTrigger: ");
	log_file->write_raw(trigger2str(trigger));
	log_file->write_raw("\nLine: ");
	log_file->write_raw(line_number);
	log_file->write_endl();
	log_file->write_footer();
	
	// Create pipe for communication with child process
	int fds[2];
	if (pipe(fds) == -1) {
		// Failed to create pipe
		log_file->write_header("t_call_script::exec_action",
			LOG_NORMAL, LOG_WARNING);
		log_file->write_raw("Failed to create pipe: ");
		log_file->write_raw(get_error_str(errno));
		log_file->write_endl();
		log_file->write_footer();
		return;
	}
	
	// Fork child process
	pid_t pid = fork();
	if (pid == -1) {
		// Failed to fork child process
		log_file->write_header("t_call_script::exec_action",
			LOG_NORMAL, LOG_WARNING);
		log_file->write_raw("Failed to fork child process: ");
		log_file->write_raw(get_error_str(errno));
		log_file->write_endl();
		log_file->write_footer();
		
		close(fds[0]);
		close(fds[1]);
		return;
	} else if (pid == 0) {
		// Child process
		
		// Close the read end of the pipe
		close(fds[0]);
		
		// Redirect stdout to the write end of the pipe
		dup2(fds[1], STDOUT_FILENO);
		
		// NOTE: MEMMAN audits are not called as all pointers will be deleted
		//       automatically when the child process dies
		//	 Also, the child process has a copy of the MEMMAN object
		char **argv = create_argv();
		
		// Determine environment
		char **env = create_env(m);
		
		// Replace the child process by the script
		if (execve(argv[0], argv, env) == -1) {
			// Failed to execute script. Report error to parent.
			string err_msg;
			err_msg = get_error_str(errno);
			err_msg += ": ";
			err_msg += argv[0];
			cout << SCR_INTERNAL_ERROR << '=' << err_msg << endl;
			exit(0);
		}
	} else {
		// Parent process
		log_file->write_header("t_call_script::exec_action");
		log_file->write_raw("Child process spawned, pid = ");
		log_file->write_raw((int)pid);
		log_file->write_endl();
		log_file->write_footer();
		
		// Close the write end of the pipe
		close(fds[1]);
		
		// Read the script results
		FILE *fp_result = fdopen(fds[0], "r");
		if (!fp_result) {
			log_file->write_header("t_call_script::exec_action",
				LOG_NORMAL, LOG_WARNING);
			log_file->write_raw("Failed to open pipe to child: ");
			log_file->write_raw(get_error_str(errno));
			log_file->write_endl();
			log_file->write_footer();
			
			// Child will be cleaned up by phone_sigwait

			close(fds[0]);
			return;
		}
		
		char *line_buf = NULL;
		size_t line_buf_len = 0;
		ssize_t num_read;
		
		// Read and parse script results.
		while ((num_read = getline(&line_buf, &line_buf_len, fp_result)) != -1) {
			// Strip newline if present
			if (line_buf[num_read - 1] == '\n') {
				line_buf[num_read - 1] = 0;
			}

			// Convert the read line to a C++ string
			string line(line_buf);	
			line = trim(line);
			
			// Stop reading on end command
			if (line == "end") break;
	
			// Skip empty lines
			if (line.empty()) continue;
	
			// Skip comment lines
			if (line[0] == '#') continue;
	
			vector<string> v = split_on_first(line, '=');
			
			// SKip invalid lines
			if (v.size() != 2) continue;
	
			string parameter = trim(v[0]);
			string value = trim(v[1]);
			
			if (parameter == SCR_INTERNAL_ERROR) {
				log_file->write_report(value,
					"t_call_script::exec_action",
					LOG_NORMAL, LOG_WARNING);
				ui->cb_display_msg(value, MSG_WARNING);
				result.clear();
				break;
			}
			
			result.set_parameter(parameter, value);
		}
		
		if (line_buf) free(line_buf);
		fclose(fp_result);
		close(fds[0]);
		
		// Child will be cleaned up by phone_sigwait
	}
}

void t_call_script::exec_notify(t_sip_message *m) const 
{
	if (script_command.empty()) return;
	
	log_file->write_header("t_call_script::exec_notify");
	log_file->write_raw("Execute script: ");
	log_file->write_raw(script_command);
	log_file->write_raw("\nTrigger: ");
	log_file->write_raw(trigger2str(trigger));
	log_file->write_endl();
	log_file->write_footer();
	
	// Fork child process
	pid_t pid = fork();
	if (pid == -1) {
		// Failed to fork child process
		log_file->write_header("t_call_script::exec_notify",
			LOG_NORMAL, LOG_WARNING);
		log_file->write_raw("Failed to fork child process: ");
		log_file->write_raw(get_error_str(errno));
		log_file->write_endl();
		log_file->write_footer();

		return;
	} else if (pid == 0) {
		// Child process
			
		// NOTE: MEMMAN audits are not called as all pointers will be deleted
		//       automatically when the child process dies
		//	 Also, the child process has a copy of the MEMMAN object
		char **argv = create_argv();
		
		// Determine environment
		char **env = create_env(m);
		
		// Replace the child process by the script
		if (execve(argv[0], argv, env) == -1) {
			// Failed to execute script.
			exit(0);
		}
	} else {
		// Parent process
		log_file->write_header("t_call_script::exec_notify");
		log_file->write_raw("Child process spawned, pid = ");
		log_file->write_raw((int)pid);
		log_file->write_endl();
		log_file->write_footer();
		
		// No interaction with child needed.
		// Child will be cleaned up by phone_sigwait
	}
}
