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
 * Message Waiting Indication information.
 */

#ifndef _MWI_HH
#define _MWI_HH

#include "simple_msg_sum_body.h"
#include "threads/mutex.h"

/** MWI information */
class t_mwi {
public:
	/** Status of MWI information */
	enum t_status {
		MWI_UNKNOWN, 	/**< The status is unknown */
		MWI_KNOWN,   	/**< MWI properly received */
		MWI_FAILED  	/**< MWI subscription failed */ 
	};
		
private:
	/** Mutex for exclusive access to MWI information */
	mutable t_mutex	mtx_mwi;

	/** MWI status */
	t_status	status;

	/** Indication if messages are waiting */
	bool		msg_waiting;

	/** Summary of voice messages waiting */
	t_msg_summary	voice_msg_summary;
	
public:
	t_mwi();
	
	t_status get_status(void) const;
	bool get_msg_waiting(void) const;
	t_msg_summary get_voice_msg_summary(void) const;
	
	void set_status(t_status _status);
	void set_msg_waiting(bool _msg_waiting);
	void set_voice_msg_summary(const t_msg_summary &summary);

	/** Set all counters to zero */
	void clear_voice_msg_summary(void);
};

#endif
