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

#include "mwi.h"

t_mwi::t_mwi() :
	status(MWI_UNKNOWN)
{}

t_mwi::t_status t_mwi::get_status(void) const {
	t_status result;
	mtx_mwi.lock();
	result = status;
	mtx_mwi.unlock();
	
	return result;
}

bool t_mwi::get_msg_waiting(void) const {
	bool result;
	mtx_mwi.lock();
	result = msg_waiting;
	mtx_mwi.unlock();
	
	return result;
}

t_msg_summary t_mwi::get_voice_msg_summary(void) const {
	t_msg_summary result;
	mtx_mwi.lock();
	result = voice_msg_summary;
	mtx_mwi.unlock();
	
	return result;
}

void t_mwi::set_status(t_status _status) {
	mtx_mwi.lock();
	status = _status;
	mtx_mwi.unlock();
}

void t_mwi::set_msg_waiting(bool _msg_waiting) {
	mtx_mwi.lock();
	msg_waiting = _msg_waiting;
	mtx_mwi.unlock();
}

void t_mwi::set_voice_msg_summary(const t_msg_summary &summary) {
	mtx_mwi.lock();
	voice_msg_summary = summary;
	mtx_mwi.unlock();
}

void t_mwi::clear_voice_msg_summary(void) {
	mtx_mwi.lock();
	voice_msg_summary.clear();
	mtx_mwi.unlock();
}
