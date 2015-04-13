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
 * Abstract class for all types of SIP dialogs.
 */

#ifndef _ABSTRACT_DIALOG_H
#define _ABSTRACT_DIALOG_H

#include <string>
#include <list>
#include <set>
#include <queue>
#include "client_request.h"
#include "id_object.h"
#include "protocol.h"
#include "sockets/url.h"
#include "parser/request.h"

using namespace std;

// Forward declaration
class t_phone_user;

/**
 * Abstract class for all types of SIP dialogs.
 * Concrete classes for all SIP dialogs inherit from this class.
 */
class t_abstract_dialog : public t_id_object {
protected:	
	/** 
	 * Phone user for which this dialog is created.
	 * This pointer should never be deleted.
	 */
	t_phone_user	*phone_user;

	string		call_id;	/**< SIP call id. */
	bool		call_id_owner;	/**< Indicates if the call id is generated locally. */
	string		local_tag;	/**< Local tag value. */
	string		remote_tag;	/**< Remote tag value. */
	unsigned long	local_seqnr;	/**< Last local sequence number issued. */
	unsigned long	remote_seqnr;	/**< Last remote sequence number received. */

	/**
	 * The remote_seqnr_set indicates if the remote_seqnr is set by the far-end.
	 * RFC 3261 allows the CSeq sequence number to be 0. So there is no
	 * invalid sequence number.
	 */
	bool		remote_seqnr_set;
	
	t_url		local_uri;		/**< URI of the local party (From/To headers). */
	string		local_display;		/**< Display name of the local party. */
	t_url		remote_uri;		/**< URI of the remote party (From/To headers). */
	string		remote_display;		/**< Display name of the remote party. */
	 
	/** URI of the remote target (Contact header). This is the destination for a request. */
	t_url		remote_target_uri;
	string		remote_target_display;	/**< Display name of the remote target. */
	
	list<t_route>	route_set;		/**< The route set. */
	unsigned long	local_resp_nr;		/**< Last local response number (for 100rel) issued. */
	unsigned long	remote_resp_nr;		/**< Last remote response number (for 100rel) received. */
	set<string>	remote_extensions;      /**< SIP extensions supported by the remote party. */
	
	/** The IP transport/address/port from which the last SIP message was received. */
	t_ip_port	remote_ip_port;

	/**
	 * Remove a client request. Pass one of the client request
	 * pointers to this member. The reference count of the
	 * request will be decremented. If it becomes zero, then
	 * the request object is deleted.
	 * In all cases the passed pointer will be set to NULL.
	 * @param cr [in] The client request.
	 */
	void remove_client_request(t_client_request **cr);

	/**
	 * Create route set from the Record-Route header of a response.
	 * If the response does not have a Record-Route header, then the route
	 * set is cleared.
	 * @param r [in] The response.
	 */
	void create_route_set(t_response *r);

	/**
	 * Create remote target uri and display from the Contact header of a response.
	 * @param r [in] The response.
	 */
	void create_remote_target(t_response *r);
	
	/**
	 * Send a request within the dialog.
	 * Sending a request will create a SIP transaction.
	 * @param r [in] The request.
	 * @param tuid [in] The transaction user id to be assigend to the transaction.
	 */
	virtual void send_request(t_request *r, t_tuid tuid) = 0;

	/**
	 * Resend an existing client request.
	 * A new Via and CSeq header will be put in the request.
	 * Resending is different from retransmitting. Requests are automatically
	 * retransmitted by the transaction layer. Resending creates a new SIP
	 * transaction. Resending is f.i. done when a request must be redirected.
	 * @param cr [in] The client request.
	 */
	virtual void resend_request(t_client_request *cr);
	
	/**
	 * Resend mid-dialog request with an authorization header containing
	 * credentials for the challenge in the response. 
	 * @param cr [in] The request.
	 * @param resp [in] The 401 or 407 response.
	 * @return true, if resending succeeded.
	 * @return false, if credentials could not be determined.
	 *
	 * @pre The response must be a 401 or 407.
	 */
	bool resend_request_auth(t_client_request *cr, t_response *resp);
	
	/**
	 * Redirect mid-dialog request to the next destination.
	 * There are multiple reasons for redirection:
	 *  - A 3XX response was received.
	 *  - The request failed with a non-3XX response. A next contact should be tried.
	 *
	 * @param cr [in] The request.
	 * @param resp [in] The failure response that was received on the request. 
	 * @param contact [out] Contains on succesful return the contact to which the request is sent.
	 * @return true, if the request is sent to a next destination.
	 * @return false, if no next destination exists.
	 */
	bool redirect_request(t_client_request *cr, t_response *resp,
			t_contact_param &contact);
	
	/**
	 * Failover request to the next destination from DNS lookup.
	 * @param cr [in] The request.
	 * @return true, if the request is sent to a next destination.
	 * @return false, if no next destination exists.
	 */
	bool failover_request(t_client_request *cr);

public:
	/**
	 * Constructor.
	 * @param pu [in] Phone user for which the dialog must be created.
	 */
	t_abstract_dialog(t_phone_user *pu);
	
	/**
	 * Destructor.
	 */
	virtual ~t_abstract_dialog();

	/**
	 * Create a request using the stored dialog state information.
	 * @param m [in] Request method.
	 * @return The request.
	 */
	virtual t_request *create_request(t_method m);

	/**
	 * Copy a dialog.
	 * @return A copy of the dialog.
	 */
	virtual t_abstract_dialog *copy(void) = 0;
	
	/**
	 * Get a pointer to the user profile of the user for whom this dialog
	 * was created.
	 * @return The user profile.
	 */
	t_user *get_user(void) const;

	/**
	 * Resend mid-dialog request with an authorization header containing
	 * credentials for the challenge in the response.
	 * @param resp [in] The 401 or 407 response to the request that must be resent.
	 * @return true, if resending succeeded.
	 * @return false, if credentials could not be determined.
	 *
	 * @pre The response must be a 401 or 407.
	 */
	virtual bool resend_request_auth(t_response *resp) = 0;

	/**
	 * Redirect mid-dialog request to the next destination.
	 * @param resp [in] The response to the request that must be resent.
	 * @return true, if the request is sent to a next destination.
	 * @return false, if no next destination exists.
	 */
	virtual bool redirect_request(t_response *resp) = 0;
	
	/**
	 * Failover request to the next destination from DNS lookup.
	 * @param resp [in] The response to the request that must be resent.
	 * @return true, if the request is sent to a next destination.
	 * @return false, if no next destination exists.
	 */
	virtual bool failover_request(t_response *resp) = 0;

	/**
	 * Process a received response.
	 * @param r [in] The received response.
	 * @param tuid [in] The transaction user id of the transaction for the response.
	 * @param tid [in] The transaction id of the transaction for the response.
	 */
	virtual void recvd_response(t_response *r, t_tuid tuid, t_tid tid);
	
	/**
	 * Process a received request.
	 * @param r [in] The received request.
	 * @param tuid [in] The transaction user id of the transaction for the request.
	 * @param tid [in] The transaction id of the transaction for the request.
	 */
	virtual void recvd_request(t_request *r, t_tuid tuid, t_tid tid);

	/**
	 * Match a response with the dialog.
	 * @param r [in] The response.
	 * @param tuid [in] The transaction user id of the transaction for the response.
	 * @return true, if the response matches the dialog.
	 * @return false, otherwise.
	 */
	virtual bool match_response(t_response *r, t_tuid tuid);

	/**
	 * Match a request with the dialog.
	 * @param r [in] The request.
	 * @return true, if the request matches the dialog.
	 * @return false, otherwise.
	 */
	virtual bool match_request(t_request *r);
	
	/**
	 * Partially match a request with the dialog, i.e. do not match remote tag.
	 * @param r [in] The request.
	 * @return true, if the request partially matches the dialog.
	 * @return false, otherwise.
	 */
	virtual bool match_partial_request(t_request *r);
	
	/**
	 * Match call-id and tags with the dialog.
	 * @param _call_id [in] SIP call-id.
	 * @param to_tag [in] SIP to-tag.
	 * @param from_tag [in] SIP from-tag.
	 * @return true, if call-id and tags match the dialog.
	 * @return false, otherwise.
	 */
	virtual bool match(const string &_call_id, const string &to_tag, 
		const string &from_tag) const;
	
	/**
	 * Get the URI of the remote target.
	 * @return remote target URI.
	 * @see remote_target_uri
	 */
	t_url get_remote_target_uri(void) const;
	
	/**
	 * Get the display name of the remote target.
	 * @return display name of remote target.
	 * @see remote_target_display
	 */
	string get_remote_target_display(void) const;
	
	/**
	 * Get the URI of the remote party.
	 * @return URI of remote party.
	 * @see remote_uri
	 */
	t_url get_remote_uri(void) const;
	
	/**
	 * Get the display name of the remote party.
	 * @return display name of the remote party.
	 * @see remote_display
	 */
	string get_remote_display(void) const;
	
	/**
	 * Get the IP transport/address/port from which the last SIP message was received.
	 * @return transport/address/port
	 */
	t_ip_port get_remote_ip_port(void) const;
	
	/**
	 * Get the SIP call id.
	 * @return SIP call id.
	 */
	string get_call_id(void) const;
	
	/**
	 * Get the local tag.
	 * @return local tag.
	 */
	string get_local_tag(void) const;
	
	/**
	 * Get the remote tag.
	 * @return remote tag.
	 */
	string get_remote_tag(void) const;
	
	/**
	 * Check if the remote party supports a particular SIP exentsion.
	 * @param extension [in] Name of the SIP extension.
	 * @return true, if remote party supports the extension.
	 * @return false, otherwise.
	 */
	virtual bool remote_extension_supported(const string &extension) const;

	/**
	 * Check if this dialog is the owner of the call id.
	 * @return true, if this dialog is the owner.
	 * @return false, otherwise.
	 * @see call_id_owner
	 */
	bool is_call_id_owner(void) const;
};

#endif
