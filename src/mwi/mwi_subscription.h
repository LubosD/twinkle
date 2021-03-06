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
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

// RFC 3842
// message-summary subscription

#ifndef _MWI_SUBSCRIPTION_H
#define _MWI_SUBSCRIPTION_H

#include "mwi.h"
#include "mwi_dialog.h"
#include "subscription.h"

class t_mwi_subscription : public t_subscription {
private:
	t_mwi *mwi;
	
protected:
	virtual t_request *create_subscribe(unsigned long expires) const;

public:
	t_mwi_subscription(t_mwi_dialog *_dialog, t_mwi *_mwi);
	
	virtual bool recv_notify(t_request *r, t_tuid tuid, t_tid tid);
	virtual bool recv_subscribe_response(t_response *r, t_tuid tuid, t_tid tid);
};

#endif
