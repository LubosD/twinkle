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

// RFC 2833
// RTP payload format for DTMF telephone events

#ifndef _RTP_TELEPHONE_EVENT_H
#define _RTP_TELEPHONE_EVENT_H


// RFC 2833 3.10
// DTMF events
#define	TEL_EV_DTMF_0		0
#define	TEL_EV_DTMF_1		1
#define	TEL_EV_DTMF_2		2
#define	TEL_EV_DTMF_3		3
#define	TEL_EV_DTMF_4		4
#define	TEL_EV_DTMF_5		5
#define	TEL_EV_DTMF_6		6
#define	TEL_EV_DTMF_7		7
#define	TEL_EV_DTMF_8		8
#define	TEL_EV_DTMF_9		9
#define	TEL_EV_DTMF_STAR	10
#define	TEL_EV_DTMF_POUND	11
#define	TEL_EV_DTMF_A		12
#define	TEL_EV_DTMF_B		13
#define	TEL_EV_DTMF_C		14
#define	TEL_EV_DTMF_D		15

#define VALID_DTMF_EV(ev)	( (ev) <= TEL_EV_DTMF_D )
#define VALID_DTMF_SYM(s)	( ((s) >= '0' && (s) <= '9') || \
				  ((s) >= 'a' && (s) <= 'd') || \
				  ((s) >= 'A' && (s) <= 'D') || \
				  (s) == '*' || (s) == '#' )

// RFC 2833 3.5
// Payload format (in network order!!)
struct t_rtp_telephone_event {
private:
	unsigned char	event : 8;
	unsigned char	volume : 6;
	bool		reserved : 1;
	bool		end : 1;
	unsigned short	duration : 16;

public:
	// Values set/get are in host order
	void set_event(unsigned char _event);
	void set_volume(unsigned char _volume);
	void set_reserved(bool _reserved);
	void set_end(bool _end);
	void set_duration(unsigned short _duration);

	unsigned char get_event(void) const;
	unsigned char get_volume(void) const;
	bool get_reserved(void) const;
	bool get_end(void) const;
	unsigned short get_duration(void) const;
};

unsigned char char2dtmf_ev(char sym);
char dtmf_ev2char(unsigned char ev);

#endif
