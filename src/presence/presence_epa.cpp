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

#include "presence_epa.h"

#include "pidf_body.h"
#include "audits/memman.h"
#include "parser/hdr_event.h"

t_presence_epa::t_presence_epa(t_phone_user *pu) :
	t_epa(pu, SIP_EVENT_PRESENCE, t_url(pu->get_user_profile()->create_user_uri(false))),
	basic_state(t_presence_state::ST_BASIC_CLOSED),
	tuple_id(NEW_PIDF_TUPLE_ID)
{}

t_presence_state::t_basic_state t_presence_epa::get_basic_state(void) const {
	return basic_state;
}

bool t_presence_epa::recv_response(t_response *r, t_tuid tuid, t_tid tid) {
	t_epa::recv_response(r, tuid, tid);
	
	// Notify observers so they can get the latest publication state.
	notify();
	
	return true;
}

void t_presence_epa::publish_presence(t_presence_state::t_basic_state _basic_state) {
	if (_basic_state != t_presence_state::ST_BASIC_CLOSED &&
	    _basic_state != t_presence_state::ST_BASIC_OPEN)
	{
		// Cannot publish internal states.
		return;
	}
	
	t_user *user_config = phone_user->get_user_profile();
	basic_state = _basic_state;
	
	// Create PIDF document
	t_pidf_xml_body *pidf = new t_pidf_xml_body();
	MEMMAN_NEW(pidf);
	pidf->set_pres_entity(user_config->create_user_uri(false));
	pidf->set_tuple_id(tuple_id);
	pidf->set_basic_status(t_presence_state::basic_state2pidf_str(_basic_state));
	
	publish(user_config->get_pres_publication_time(), pidf);
	
	// NOTE: the observers will be notified of the state change, when the
	//       PUBLISH response is received.
}

void t_presence_epa::clear(void) {
	t_epa::clear();
	basic_state = t_presence_state::ST_BASIC_CLOSED;
}
