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

/** @file
 * Bind request with TU and transaction
 */

#ifndef _CLIENT_REQUEST_H
#define _CLIENT_REQUEST_H

#include "protocol.h"
#include "redirect.h"
#include "user.h"
#include "transaction_layer.h"
#include "threads/mutex.h"
#include "parser/request.h"
#include "stun/stun.h"

using namespace std;

/** Object for storing a request together with its Transaction User id and transaction id. */
class t_client_request {
private:
	static t_mutex	mtx_next_tuid; 	/**< Protect updates on @ref next_tuid */
	static t_tuid	next_tuid;     	/**< Next transaction user id to handout. */

	// A client request is either a SIP or a STUN request
	t_request	*request;	/**< SIP request. */
	StunMessage	*stun_request;	/**< STUN request. */
	
	t_tuid		tuid;		/**< Transaction user id. */
	t_tid		tid;		/**< Transaction id. */

	/** Number of references to this object (#dialogs). */
	int		ref_count;

public:
	/** Redirector for 3XX redirections. */
	t_redirector	redirector;

	/**
	 * Constructor.
	 * A copy of the request is stored in the client_request object.
	 * @param user The user profile of the user sending the request.
	 * @param r SIP request.
	 * @param _tid Transaction id.
	 */
	t_client_request(t_user *user, t_request *r, const t_tid _tid);
	
	/**
	 * Constructor.
	 * A copy of the request is stored in the client_request object.
	 * @param user The user profile of the user sending the request.
	 * @param r STUN request.
	 * @param _tid Transaction id.
	 */	
	t_client_request(t_user *user, StunMessage *r, const t_tid _tid);
	
	/** Destructor. */
	~t_client_request();

	/**
	 * Create a copy of the client request.
	 * @return Copy of the client request.
	 * @note: The request inside the client request is copied.
	 */
	t_client_request *copy(void);

	/**
	 * Get a pointer to the SIP request.
	 * @return Pointer to the SIP request.
	 */
	t_request *get_request(void) const;
	
	/**
	 * Get a pointer to the STUN request.
	 * @return Pointer to the STUN request.
	 */
	StunMessage *get_stun_request(void) const;

	/** Get the transaction user id. */
	t_tuid get_tuid(void) const;
	
	/** Get the transaction id. */
	t_tid get_tid(void) const;
	
	/** Set the transaction id. */
	void set_tid(t_tid _tid);

	/** 
	 * Create a new tuid and set tid.
	 * @param _tid The new tid to set.
	 */
	void renew(t_tid _tid);

	/** Get the reference count. */
	int get_ref_count(void) const;

	/**
	 * Increment reference count. 
	 * @return The reference count after increment.
	 */
	int inc_ref_count(void);

	/**
	 * Decrement reference count. 
	 * @returns The reference count after decrement.
	 */
	int dec_ref_count(void);
};

#endif
