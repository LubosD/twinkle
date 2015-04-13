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
 * Event Publication Agent (EPA) [RFC 3903]
 */
 
#ifndef _EPA_H
#define _EPA_H

#include <queue>
#include <string>

#include "id_object.h"
#include "phone_user.h"
#include "sockets/url.h"
#include "parser/sip_body.h"
#include "protocol.h"

using namespace std;


/** Event Publication Agent (EPA) [RFC 3903] */
class t_epa {
public:
	/** State of the EPA */
	enum t_epa_state {
		EPA_UNPUBLISHED, /**< The event has not been published. */
		EPA_PUBLISHED,	 /**< The event has been published. */
		EPA_FAILED,	 /**< Failed to publish the event. */
	};
	
private:
	/**
	 * Queue of pending outgoing PUBLISH requests. A next PUBLISH
	 * will only be sent after the previous PUBLISH has been
	 * answered.
	 */
	queue<t_request *>	queue_publish;
	
	/**
	 * Enqueue a request.
	 * @param r [in] Request to enqueue.
	 */
	void		enqueue_request(t_request *r);

protected:
	/** Phone user for whom publications are issued. */
	t_phone_user	*phone_user;
	
	/** EPA state. */
	t_epa_state	epa_state;
	
	/** Detailed failure message when @ref epa_state == @ref EPA_FAILED */
	string		failure_msg;

	/** 
	 * Entity tag associated with the publication.
	 * For an initial publication there is no entity tag yet.
	 */
	string		etag;

	/** Event for which the event state is published. */
	string		event_type;
	
	/** Request-URI for the publish request. */
	t_url		request_uri;
	
	/** Timer indicating when a publication must be refreshed. */
	t_object_id	id_publication_timeout;
	
	/** Expiry duration (sec) of a publication. */
	unsigned long	publication_expiry;
	
	/** Default duration for a publication/ */
	unsigned long	default_duration;
	
	/** Indicates if an unpublish is in progress. */
	bool		is_unpublishing;
	
	/** Cached body of last publication. */
	t_sip_body	*cached_body;
	
	/** Log the publication details */
	void log_publication(void) const;
	
	/**
	 * Remove a pending request. Pass one of the client request pointers.
	 * @param cr [in] Client request to remove.
	 */
	void remove_client_request(t_client_request **cr);
	
	/**
	 * Create a PUBLISH request.
	 * @param expires [in] Expiry time in seconds.
	 * @param body [in] Body for the request. The body will be destroyed when
	 * the request will be destroyed.
	 */
	virtual t_request *create_publish(unsigned long expires, t_sip_body *body) const;

	/**
	 * Send request.
	 * @param r [in] Request to send.
	 * @param tuid [in] Transaction user id.
	 */
	void send_request(t_request *r, t_tuid tuid) const;
	
	/**
	 * Send the next PUBLISH request from the queue.
	 * If the queue is empty, then this method does nothing.
	 */
	void send_publish_from_queue(void);

	/** 
	 * Start a publication timer.
	 * @param timer [in] Type of publication timer.
	 * @param duration [in] Duration of timer in ms
	 */
	void start_timer(t_publish_timer timer, long duration);
	
	/**
	 * Stop a publication timer.
	 * @param timer [in] Type of publication timer.
	 */
	void stop_timer(t_publish_timer timer);
	
public:
	/** Pending request */
	t_client_request	*req_out;

	/** Constructor. */
	t_epa(t_phone_user *pu, const string &_event_type, const t_url _request_uri);
	
	/** Destructor. */
	virtual ~t_epa();
	
	/** @name Getters */
	//@{
	t_epa_state get_epa_state(void) const;
	string get_failure_msg(void) const;
	t_phone_user *get_phone_user(void) const;
	//@}
	
	/**
	 * Get the user profile of the user.
	 * @return The user profile.
	 */
	t_user *get_user_profile(void) const;
	
	/**
	 * Receive PUBLISH response.
	 * @param r [in] Received response.
	 * @param tuid [in] Transaction user id.
	 * @param tid [in] Transaction id.
	 * @return The return value indicates if processing is finished.
	 */
	virtual bool recv_response(t_response *r, t_tuid tuid, t_tid tid);
	
	/**
	 * Match response with a pending publish.
	 * @param r [in] The response.
	 * @param tuid [in] Transaction user id.
	 * @return True if the response matches, otherwise false.
	 */
	virtual bool match_response(t_response *r, t_tuid tuid) const;
	
	/**
	 * Process timeouts
	 * @param timer [in] Type of publication timer.
	 * @return The return value indicates if processing is finished.
	 */
	virtual bool timeout(t_publish_timer timer);
	
	/**
	 * Match timer id with a running timer.
	 * @param timer [in] Type of publication timer.
	 * @return True, if id matches, otherwise false.
	 */
	virtual bool match_timer(t_publish_timer timer, t_object_id id_timer) const;
	
	/**
	 * Publish event state.
	 * @param expired [in] Duration of publication in seconds.
	 * @param body [in] Body for PUBLISH request.
	 * @note The body will be deleted when the PUBLISH has been sent.
	 * The caller of this method should *not* delete the body.
	 */
	virtual void publish(unsigned long expires, t_sip_body *body);
	
	/** Terminate publication. */
	virtual void unpublish(void);
	
	/** Refresh publication. */
	virtual void refresh_publication(void);
	
	/** Clear all state */
	virtual void clear(void);
};

#endif
