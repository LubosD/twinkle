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

#include "twinkleapplication.h"

#include "phone.h"
#include "sys_settings.h"
#include "user.h"
#include "gui.h"

extern t_phone *phone;

#ifdef HAVE_KDE
t_twinkle_application::t_twinkle_application() : KApplication() {}
#else
t_twinkle_application::t_twinkle_application(int &argc, char **argv) :
		QApplication(argc,argv)
{}
#endif
void t_twinkle_application::commitData (QSessionManager &sm) {
	sys_config->set_ui_session_id(sessionId().ascii());
	
	// Create list of active profile file names
	list<t_user *> user_list = phone->ref_users();
	list<string> profile_filenames;
	for (list<t_user *>::const_iterator it = user_list.begin(); it != user_list.end(); ++it) {
		profile_filenames.push_back((*it)->get_filename());
	}
	
	sys_config->set_ui_session_active_profiles(profile_filenames);
	((t_gui*)ui)->save_session_state();
}
