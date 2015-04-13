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

#include "client_request.h"
#include "audits/memman.h"

t_mutex t_client_request::mtx_next_tuid;
t_tuid t_client_request::next_tuid = 1;

t_client_request::t_client_request(t_user *user, t_request *r, const t_tid _tid) :
		redirector(r->uri, user->get_max_redirections())
{
	request = (t_request *)r->copy();
	stun_request = NULL;
	tid = _tid;
	ref_count = 1;

	mtx_next_tuid.lock();
	tuid = next_tuid++;
	if (next_tuid == 65535) next_tuid = 1;
	mtx_next_tuid.unlock();
}

t_client_request::t_client_request(t_user *user, StunMessage *r, const t_tid _tid) :
		redirector(t_url(), user->get_max_redirections())
{
	request = NULL;
	stun_request = new StunMessage(*r);
	MEMMAN_NEW(stun_request);
	tid = _tid;
	ref_count = 1;

	mtx_next_tuid.lock();
	tuid = next_tuid++;
	if (next_tuid == 65535) next_tuid = 1;
	mtx_next_tuid.unlock();
}

t_client_request::~t_client_request() {
	if (request) {
		MEMMAN_DELETE(request);
		delete request;
	}
	
	if (stun_request) {
		MEMMAN_DELETE(stun_request);
		delete stun_request;
	}
}

t_client_request *t_client_request::copy(void) {
	t_client_request *cr = new t_client_request(*this);
	MEMMAN_NEW(cr);
	
	if (request) {
		cr->request = (t_request *)request->copy();
	}
	
	if (stun_request) {
		cr->stun_request = new StunMessage(*stun_request);
		MEMMAN_NEW(cr->stun_request);
	}
	
	cr->ref_count = 1;
	return cr;
}

t_request *t_client_request::get_request(void) const {
	return request;
}

StunMessage *t_client_request::get_stun_request(void) const {
	return stun_request;
}

t_tuid t_client_request::get_tuid(void) const {
	return tuid;
}

t_tid t_client_request::get_tid(void) const {
	return tid;
}

void t_client_request::set_tid(t_tid _tid) {
	tid = _tid;
}

void t_client_request::renew(t_tid _tid) {
	mtx_next_tuid.lock();
	tuid = next_tuid++;
	if (next_tuid == 65535) next_tuid = 1;
	mtx_next_tuid.unlock();

	tid = _tid;
}

int t_client_request::get_ref_count(void) const {
	return ref_count;
}

int t_client_request::inc_ref_count(void) {
	ref_count++;
	return ref_count;
}

int t_client_request::dec_ref_count(void) {
	ref_count--;
	return ref_count;
}
