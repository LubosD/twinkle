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

#include "cassert"
#include "rtp_telephone_event.h"
#include <netinet/in.h>

void t_rtp_telephone_event::set_event(unsigned char _event) {
	event = _event;
}

void t_rtp_telephone_event::set_volume(unsigned char _volume) {
	volume = _volume;
}

void t_rtp_telephone_event::set_reserved(bool _reserved) {
	reserved = _reserved;
}

void t_rtp_telephone_event::set_end(bool _end) {
	end = _end;
}

void t_rtp_telephone_event::set_duration(unsigned short _duration) {
	duration = htons(_duration);
}

unsigned char t_rtp_telephone_event::get_event(void) const {
	return event;
}

unsigned char t_rtp_telephone_event::get_volume(void) const {
	return volume;
}

bool t_rtp_telephone_event::get_reserved(void) const {
	return reserved;
}

bool t_rtp_telephone_event::get_end(void) const {
	return end;
}

unsigned short t_rtp_telephone_event::get_duration(void) const {
	return ntohs(duration);
}

unsigned char char2dtmf_ev(char sym) {
	if (sym >= '0' && sym <= '9') return (sym - '0' + TEL_EV_DTMF_0);
	if (sym >= 'A' && sym <= 'D') return (sym - 'A' + TEL_EV_DTMF_A);
	if (sym >= 'a' && sym <= 'd') return (sym-  'a' + TEL_EV_DTMF_A);
	if (sym == '*') return TEL_EV_DTMF_STAR;
	if (sym == '#') return TEL_EV_DTMF_POUND;
	assert(false);
}

char dtmf_ev2char(unsigned char ev) {
	if (ev <= TEL_EV_DTMF_9) {
		return ev + '0' - TEL_EV_DTMF_0;
	}
	if (ev >= TEL_EV_DTMF_A && ev <= TEL_EV_DTMF_D) {
	  	return ev + 'A' - TEL_EV_DTMF_A;
	}
	if (ev == TEL_EV_DTMF_STAR) return '*';
	if (ev == TEL_EV_DTMF_POUND) return '#';
	assert(false);
}

