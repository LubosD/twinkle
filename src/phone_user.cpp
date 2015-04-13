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

#include "phone_user.h"
#include "log.h"
#include "userintf.h"
#include "util.h"
#include "audits/memman.h"
#include "im/im_iscomposing_body.h"
#include "presence/presence_epa.h"

extern t_phone 		*phone;
extern t_event_queue	*evq_timekeeper;
extern t_event_queue	*evq_sender;
extern string		user_host;
extern string		local_hostname;

void t_phone_user::cleanup_mwi_dialog(void) {
	if (mwi_dialog && mwi_dialog->get_subscription_state() == SS_TERMINATED) {
		string reason_termination = mwi_dialog->get_reason_termination();
		bool may_resubscribe = mwi_dialog->get_may_resubscribe();
		unsigned long dur_resubscribe = mwi_dialog->get_resubscribe_after();
		
		MEMMAN_DELETE(mwi_dialog);
		delete mwi_dialog;
		mwi_dialog = NULL;
		stun_binding_inuse_mwi = false;
		cleanup_stun_data();
		cleanup_nat_keepalive();
		
		if (mwi_auto_resubscribe) {
			if (may_resubscribe) {
				if (dur_resubscribe > 0) {
					start_resubscribe_mwi_timer(dur_resubscribe * 1000);
				} else {
					subscribe_mwi();
				}
			} else if (reason_termination.empty()) {
				start_resubscribe_mwi_timer(DUR_MWI_FAILURE * 1000);
			}
		}
	}
}

void t_phone_user::cleanup_stun_data(void) {
	if (!use_stun) return;
	
	if (!stun_binding_inuse_registration &&
	    !stun_binding_inuse_mwi && stun_binding_inuse_presence == 0)
	{
		stun_public_ip_sip = 0;
		stun_public_port_sip = 0;
	}
}

void t_phone_user::cleanup_nat_keepalive(void) {
	if (register_ip_port.ipaddr == 0 && register_ip_port.port == 0 &&
	    !mwi_dialog)
	{
		if (id_nat_keepalive) phone->stop_timer(PTMR_NAT_KEEPALIVE, this);
	}
}

void t_phone_user::sync_nat_keepalive(void) {
	if (user_config->get_enable_nat_keepalive() && !id_nat_keepalive) {
		send_nat_keepalive();
		phone->start_timer(PTMR_NAT_KEEPALIVE, this);
	}
}

void t_phone_user::cleanup_tcp_ping(void) {
	if (register_ip_port.ipaddr == 0 && register_ip_port.port == 0) {
		if (id_tcp_ping) phone->stop_timer(PTMR_TCP_PING, this);
	}
}

void t_phone_user::cleanup_registration_data(void) {
	register_ip_port.ipaddr = 0;
	register_ip_port.port = 0;
	stun_binding_inuse_registration = false;
	cleanup_stun_data();
	cleanup_nat_keepalive();
	cleanup_tcp_ping();
}

t_phone_user::t_phone_user(const t_user &profile)
{
	user_config = profile.copy();
	
	service = new t_service(user_config);
	MEMMAN_NEW(service);
	
	buddy_list = new t_buddy_list(this);
	MEMMAN_NEW(buddy_list);
	
	presence_epa = new t_presence_epa(this);
	MEMMAN_NEW(presence_epa);
	
	string err_msg;
	if (buddy_list->load(err_msg)) {
		log_file->write_header("t_phone_user::t_phone_user");
		log_file->write_raw(user_config->get_profile_name());
		log_file->write_raw(": buddy list loaded.\n");
		log_file->write_footer();
	} else {
		log_file->write_header("t_phone_user::t_phone_user", LOG_NORMAL, LOG_CRITICAL);
		log_file->write_raw(user_config->get_profile_name());
		log_file->write_raw(": falied to load buddy list.\n");
		log_file->write_raw(err_msg);
		log_file->write_endl();
		log_file->write_footer();
	}
	
	active = true;

	r_options = NULL;
	r_register = NULL;
	r_deregister = NULL;
	r_query_register = NULL;
	r_message = NULL;
	r_stun = NULL;
	
	// Initialize registration data
	// Call-ID cannot be set here as user_host is not determined yet.
	register_seqnr = NEW_SEQNR;
	is_registered = false;
	register_ip_port.ipaddr = 0L;
	register_ip_port.port = 0;
	last_reg_failed = false;
	
	// Initialize STUN data
	stun_public_ip_sip = 0L;
	stun_public_port_sip = 0;
	stun_binding_inuse_registration = false;
	stun_binding_inuse_mwi = false;
	stun_binding_inuse_presence = 0;
	register_after_stun = false;
	mwi_subscribe_after_stun = false;
	presence_subscribe_after_stun = false;
	use_stun = false;
	use_nat_keepalive = false;
	
	// Timers
	id_registration = 0;
	id_nat_keepalive = 0;
	id_tcp_ping = 0;
	id_resubscribe_mwi = 0;
	
	// MWI
	mwi_dialog = NULL;
	mwi_auto_resubscribe = false;
}

t_phone_user::~t_phone_user() {
	// Stop timers
	if (id_registration) phone->stop_timer(PTMR_REGISTRATION, this);
	if (id_nat_keepalive) phone->stop_timer(PTMR_NAT_KEEPALIVE, this);
	if (id_tcp_ping) phone->stop_timer(PTMR_TCP_PING, this);

	// Delete pointers
	if (r_options) {
		MEMMAN_DELETE(r_options);
		delete r_options;
	}
	if (r_register) {
		MEMMAN_DELETE(r_register);
		delete r_register;
	}
	if (r_deregister) {
		MEMMAN_DELETE(r_deregister);
		delete r_deregister;
	}
	if (r_query_register) {
		MEMMAN_DELETE(r_query_register);
		delete r_query_register;
	}
	if (r_message) {
		MEMMAN_DELETE(r_message);
		delete r_message;
	}
	if (r_stun) {
		MEMMAN_DELETE(r_stun);
		delete r_stun;
	}
	
	for (list<t_request *>::iterator it = pending_messages.begin();
	     it != pending_messages.end(); ++it)
	{
		MEMMAN_DELETE(*it);
		delete *it;
	}
	
	if (mwi_dialog) {
		MEMMAN_DELETE(mwi_dialog);
		delete mwi_dialog;
	}
	
	MEMMAN_DELETE(service);
	delete service;
	MEMMAN_DELETE(presence_epa);
	delete presence_epa;
	MEMMAN_DELETE(buddy_list);
	delete buddy_list;
	buddy_list = NULL;
	MEMMAN_DELETE(user_config);
	delete user_config;
}

t_user *t_phone_user::get_user_profile(void) {
	return user_config;
}

t_buddy_list *t_phone_user::get_buddy_list(void) {
	return buddy_list;
}

t_presence_epa *t_phone_user::get_presence_epa(void) {
	return presence_epa;
}

void t_phone_user::registration(t_register_type register_type, bool re_register,
		unsigned long expires)
{
	// If STUN is enabled, then do a STUN query before registering to
	// determine the public IP address.
	if (register_type == REG_REGISTER && use_stun) {
		if (stun_public_ip_sip == 0) {
			send_stun_request();
			register_after_stun = true;
			registration_time = expires;
			return;
		}
		
		stun_binding_inuse_registration = true;
	}

	// Stop registration timer for non-query request
	if (register_type != REG_QUERY) {
		phone->stop_timer(PTMR_REGISTRATION, this);
	}

	// Create call-id if no call-id is created yet
	if (register_call_id == "") {
		register_call_id = NEW_CALL_ID(user_config);
	}

	// RFC 3261 10.2
	// Construct REGISTER request

	t_request *req = create_request(REGISTER, 
			t_url(string(USER_SCHEME) + ":" + user_config->get_domain()));

	// To
	req->hdr_to.set_uri(user_config->create_user_uri(false));
	req->hdr_to.set_display(user_config->get_display(false));

	//Call-ID
	req->hdr_call_id.set_call_id(register_call_id);

	// CSeq
	req->hdr_cseq.set_method(REGISTER);
	req->hdr_cseq.set_seqnr(++register_seqnr);

	// Contact
        t_contact_param contact;

        switch (register_type) {
        case REG_REGISTER:
        	// URI
                contact.uri.set_url(user_config->create_user_contact(false,
                		h_ip2str(req->get_local_ip())));
                
                // Expires
                if (expires > 0) {
			if (user_config->get_registration_time_in_contact()) {
				contact.set_expires(expires);
			} else {
				req->hdr_expires.set_time(expires);
			}
		}
		
		// q-value
		if (user_config->get_reg_add_qvalue()) {
			contact.set_qvalue(user_config->get_reg_qvalue());
		}

                req->hdr_contact.add_contact(contact);
                break;
        case REG_DEREGISTER:
                contact.uri.set_url(user_config->create_user_contact(false,
                		h_ip2str(req->get_local_ip())));
 		if (user_config->get_registration_time_in_contact()) {
			contact.set_expires(0);
		} else {
			req->hdr_expires.set_time(0);
		}
                req->hdr_contact.add_contact(contact);
                break;
        case REG_DEREGISTER_ALL:
                req->hdr_contact.set_any();
                req->hdr_expires.set_time(0);
                break;
        default:
                break;
        }

	// Allow
	SET_HDR_ALLOW(req->hdr_allow, user_config);

	// Store request in the proper place
	t_tuid tuid;

        switch(register_type) {
        case REG_REGISTER:
		// Delete a possible pending registration request
		if (r_register) {
			MEMMAN_DELETE(r_register);
			delete r_register;
		}
                r_register = new t_client_request(user_config, req, 0);
		MEMMAN_NEW(r_register);
                tuid = r_register->get_tuid();

                // Store expiration time for re-registration.
                registration_time = expires;
                break;
        case REG_QUERY:
		// Delete a possible pending query registration request
		if (r_query_register) {
			MEMMAN_DELETE(r_query_register);
			delete r_query_register;
		}
                r_query_register = new t_client_request(user_config, req, 0);
		MEMMAN_NEW(r_query_register);
                tuid = r_query_register->get_tuid();
                break;
        case REG_DEREGISTER:
        case REG_DEREGISTER_ALL:
		// Delete a possible pending de-registration request
		if (r_deregister) {
			MEMMAN_DELETE(r_deregister);
			delete r_deregister;
		}
                r_deregister = new t_client_request(user_config, req, 0);
		MEMMAN_NEW(r_deregister);
                tuid = r_deregister->get_tuid();
                break;
        default:
                assert(false);
        }

        // Send REGISTER
        authorizor.set_re_register(re_register);
	ui->cb_register_inprog(user_config, register_type);
        phone->send_request(user_config, req, tuid);
	MEMMAN_DELETE(req);
        delete req;
}

void t_phone_user::options(const t_url &to_uri, const string &to_display) {
	// RFC 3261 11.1
	// Construct OPTIONS request

	t_request *req = create_request(OPTIONS, to_uri);

	// To
	req->hdr_to.set_uri(to_uri);
	req->hdr_to.set_display(to_display);

	// Call-ID
	req->hdr_call_id.set_call_id(NEW_CALL_ID(user_config));

	// CSeq
	req->hdr_cseq.set_method(OPTIONS);
	req->hdr_cseq.set_seqnr(NEW_SEQNR);

	// Accept
	req->hdr_accept.add_media(t_media("application","sdp"));

	// Store and send request
	// Delete a possible pending options request
	if (r_options) {
		MEMMAN_DELETE(r_options);
		delete r_options;
	}
	r_options = new t_client_request(user_config, req, 0);
	MEMMAN_NEW(r_options);
	phone->send_request(user_config, req, r_options->get_tuid());
	MEMMAN_DELETE(req);
	delete req;
}

void t_phone_user::handle_response_out_of_dialog(t_response *r, t_tuid tuid, t_tid tid) {
	t_client_request **current_cr;
	t_request *req;
	bool is_register = false;
	t_buddy *buddy;
	
	if (r_register && r_register->get_tuid() == tuid) {
		current_cr = &r_register;
		is_register = true;
	} else if (r_deregister && r_deregister->get_tuid() == tuid) {
		current_cr = &r_deregister;
		is_register = true;
	} else if (r_query_register && r_query_register->get_tuid() == tuid) {
		current_cr = &r_query_register;
		is_register = true;
	} else if (r_options && r_options->get_tuid() == tuid) {
		current_cr = &r_options;
	} else if (r_message && r_message->get_tuid() == tuid) {
		current_cr = &r_message;
	} else if (mwi_dialog && mwi_dialog->match_response(r, tuid)) {
		mwi_dialog->recvd_response(r, tuid, tid);
		cleanup_mwi_dialog();
		return;
	} else if (presence_epa && presence_epa->match_response(r, tuid)) {
		presence_epa->recv_response(r, tuid, tid);
		return;
	} else if (buddy_list->match_response(r, tuid, &buddy)) {
		buddy->recvd_response(r, tuid, tid);
		if (buddy->must_delete_now()) buddy_list->del_buddy(*buddy);
		return;
	} else {
		// Response does not match any pending request.
		log_file->write_report("Response does not match any pending request.",
			"t_phone_user::handle_response_out_of_dialog",
			LOG_NORMAL, LOG_WARNING);
		return;
	}

	req = (*current_cr)->get_request();

	// Authentication
	if (r->must_authenticate()) {
		if (authorize(req, r)) {
			resend_request(req, is_register, *current_cr);
			return;
		}

		// Authentication failed
		// Handle the 401/407 as a normal failure response
	}
	
	// RFC 3263 4.3
	// Failover
	if (r->code == R_503_SERVICE_UNAVAILABLE) {
		if (req->next_destination()) {
			log_file->write_report("Failover to next destination.",
				"t_phone_user::handle_response_out_of_dialog");
			resend_request(req, is_register, *current_cr);
			return;
		}			
	}

	// Redirect failed request if there is another destination
	if (r->get_class() > R_2XX && user_config->get_allow_redirection()) {
		// If the response is a 3XX response then add redirection
		// contacts
		if (r->get_class() == R_3XX  &&
		    r->hdr_contact.is_populated())
		{
			(*current_cr)->redirector.add_contacts(
					r->hdr_contact.contact_list);
		}

		// Get next destination
		t_contact_param contact;
		if ((*current_cr)->redirector.get_next_contact(contact)) {
			// Ask user for permission to redirect if indicated
			// by user config
			bool permission = true;
			if (user_config->get_ask_user_to_redirect()) {
				permission = ui->cb_ask_user_to_redirect_request(
							user_config,
							contact.uri, contact.display,
							r->hdr_cseq.method);
			}

			if (permission) {
				req->uri = contact.uri;
				req->calc_destinations(*user_config);
				ui->cb_redirecting_request(user_config, contact);
				resend_request(req, is_register, *current_cr);
				return;
			}
		}
	}

	// REGISTER (register)
	if (r_register && r_register->get_tuid() == tuid) {
		bool re_register;
		handle_response_register(r, re_register);
		MEMMAN_DELETE(r_register);
		delete r_register;
		r_register = NULL;
		if (re_register) registration(REG_REGISTER, authorizor.get_re_register(), 
				registration_time);
		return;
	}

	// REGISTER (de-register)
	if (r_deregister && r_deregister->get_tuid() == tuid) {
		handle_response_deregister(r);
		MEMMAN_DELETE(r_deregister);
		delete r_deregister;
		r_deregister = NULL;
		return;
	}

	// REGISTER (query)
	if (r_query_register && r_query_register->get_tuid() == tuid) {
		handle_response_query_register(r);
		MEMMAN_DELETE(r_query_register);
		delete r_query_register;
		r_query_register = NULL;
		return;
	}


	// OPTIONS
	if (r_options && r_options->get_tuid() == tuid) {
		handle_response_options(r);
		MEMMAN_DELETE(r_options);
		delete r_options;
		r_options = NULL;
		return;
	}
	
	// MESSAGE
	if (r_message && r_message->get_tuid() == tuid) {
		handle_response_message(r);
		MEMMAN_DELETE(r_message);
		delete r_message;
		r_message = NULL;
		
		// Send next pending MESSAGE
		if (!pending_messages.empty()) {
			t_request *req = pending_messages.front();
			pending_messages.pop_front();
			r_message = new t_client_request(user_config, req, 0);
			MEMMAN_NEW(r_message);
			phone->send_request(user_config, req, r_message->get_tuid());
			MEMMAN_DELETE(req);
			delete req;			
		}
		
		return;
	}

	// Response does not match any pending request. Do nothing.
}

void t_phone_user::resend_request(t_request *req, bool is_register, t_client_request *cr) {
	// A new sequence number must be assigned
	if (is_register) {
		req->hdr_cseq.set_seqnr(++register_seqnr);
	} else {
		req->hdr_cseq.seqnr++;
	}

	// Create a new via-header. Otherwise the
	// request will be seen as a retransmission
	unsigned long local_ip = req->get_local_ip();
	req->hdr_via.via_list.clear();
	t_via via(USER_HOST(user_config, h_ip2str(local_ip)), PUBLIC_SIP_PORT(user_config));
	req->hdr_via.add_via(via);

	cr->renew(0);
	phone->send_request(user_config, req, cr->get_tuid());
}

void t_phone_user::handle_response_out_of_dialog(StunMessage *r, t_tuid tuid) {
	if (!r_stun || r_stun->get_tuid() != tuid) {
		// Response does not match pending STUN request
		return;
	}
	
	if (r->msgHdr.msgType == BindResponseMsg && r->hasMappedAddress) {
		// The STUN response contains the public IP.
		stun_public_ip_sip = r->mappedAddress.ipv4.addr;
		stun_public_port_sip = r->mappedAddress.ipv4.port;
                MEMMAN_DELETE(r_stun);
                delete r_stun;
                r_stun = NULL;
                
                if (register_after_stun) {
                	register_after_stun = false;
                	registration(REG_REGISTER, false, registration_time);
                }
                
                if (mwi_subscribe_after_stun) {
                	mwi_subscribe_after_stun = false;
                	subscribe_mwi();
                }
                
                if (presence_subscribe_after_stun) {
                	presence_subscribe_after_stun = false;
                	buddy_list->stun_completed();
                }
                
                return;
	}
	
	if (r->msgHdr.msgType == BindErrorResponseMsg && r->hasErrorCode) {
		// STUN request failed.
                ui->cb_stun_failed(user_config, r->errorCode.errorClass * 100 +
                	r->errorCode.number, r->errorCode.reason);
	} else {	
		// No satisfying STUN response was received.
 	       ui->cb_stun_failed(user_config);
	}
	
        MEMMAN_DELETE(r_stun);
        delete r_stun;
        r_stun = NULL;
	
        if (register_after_stun) {
		// Retry registration later.
		bool first_failure = !last_reg_failed;
		last_reg_failed = true;
		is_registered = false;
		ui->cb_register_stun_failed(user_config, first_failure);
		phone->start_set_timer(PTMR_REGISTRATION, DUR_REG_FAILURE * 1000, this);
		register_after_stun = false;
        }
        
        if (mwi_subscribe_after_stun) {
        	// Retry MWI subscription later
        	start_resubscribe_mwi_timer(DUR_MWI_FAILURE * 1000);
        	mwi_subscribe_after_stun = false;
        }
        
        if (presence_subscribe_after_stun) {
        	buddy_list->stun_failed();
        	presence_subscribe_after_stun = false;
        }
}

void t_phone_user::handle_response_register(t_response *r, bool &re_register) {
	t_contact_param *c;
	unsigned long expires;
	unsigned long e;
	bool first_failure, first_success;

	re_register = false;
	
	// Store the destination IP address/port of the REGISTER message.
	// To this destination the NAT keep alive packets will be sent.
	t_request *req = r_register->get_request();
	req->get_destination(register_ip_port, *user_config);

        switch(r->get_class()) {
        case R_2XX:
                last_reg_failed = false;

                // Stop registration timer if one was running
                phone->stop_timer(PTMR_REGISTRATION, this);

                c = r->hdr_contact.find_contact(req->hdr_contact.contact_list.front().uri);
                if (!c) {               	
	               	if (!user_config->get_allow_missing_contact_reg()) {
				is_registered = false;

	              		log_file->write_report(
        	       			"Contact header is missing.",
               				"t_phone_user::handle_response_register",
               				LOG_NORMAL, LOG_WARNING);
				
				ui->cb_invalid_reg_resp(user_config,
					r, "Contact header missing.");
				cleanup_registration_data();
				return;
                        } else {
                        	log_file->write_report(
                        		"Cannot find matching contact header.",
                        		"t_phone_user::handle_response_register",
                        		LOG_NORMAL, LOG_DEBUG);
                        }
                }

                if (c && c->is_expires_present() && c->get_expires() != 0) {
                        expires = c->get_expires();
                }
                else if (r->hdr_expires.is_populated() &&
                         r->hdr_expires.time != 0)
                {
                        expires = r->hdr_expires.time;
                }
                else {	
               		if (!user_config->get_allow_missing_contact_reg()) {
				is_registered = false;
				
               			log_file->write_report(
               				"Expires parameter/header mising.",
               				"t_phone_user::handle_response_register",
               				LOG_NORMAL, LOG_WARNING);
				
				ui->cb_invalid_reg_resp(user_config,
					r, "Expires parameter/header mising.");
				cleanup_registration_data();
				return;
                        }
                        
                        expires = user_config->get_registration_time();
                        
                        // Assume a default expiration of 3600 sec if no expiry
                        // time was returned.
                        if (expires == 0) expires = 3600;
                }

                // Start new registration timer
                // The maximum value of the timer can be 2^32-1 s
                // The maximum timer that we can handle however is 2^31-1 ms
                e = (expires > 2147483 ? 2147483 : expires);
                phone->start_set_timer(PTMR_REGISTRATION, e * 1000, this);
		// Save the Service-Route if present the response contains any

		// RFC 3608 6
		// Collect the service route to route later initial requests.
		if (r->hdr_service_route.is_populated()) {
			service_route = r->hdr_service_route.route_list;
			log_file->write_header("t_phone_user::handle_response_register");
			log_file->write_raw("Store service route:\n");
			for (list<t_route>::const_iterator it = service_route.begin();
			     it != service_route.end(); ++it)
			{
				log_file->write_raw(it->encode());
				log_file->write_endl();
			}
			log_file->write_footer();
		} else {
			if (!service_route.empty())
			{
				log_file->write_report("Clear service route.",
					"t_phone_user::handle_response_register");
				service_route.clear();
			}
		}

		first_success = !is_registered;
                is_registered = true;
		ui->cb_register_success(user_config, r, expires, first_success);
		
		// Start sending NAT keepalive packets when STUN is used
		// (or in case of symmetric firewall)
		if (use_nat_keepalive && id_nat_keepalive == 0) {
			// Just start the NAT keepalive timer. The REGISTER
			// message itself created the NAT binding. So there is
			// no need to send a NAT keep alive packet now.
			phone->start_timer(PTMR_NAT_KEEPALIVE, this);
		}
		
		// Start sending TCP ping packets on persistent TCP connections.
		if (user_config->get_persistent_tcp() && id_tcp_ping == 0) {
			phone->start_timer(PTMR_TCP_PING, this);
		}
		
		// Registration succeeded. If sollicited MWI is provisioned
		// and no MWI subscription is established yet, then subscribe
		// to MWI.
		if (user_config->get_mwi_sollicited() && !mwi_auto_resubscribe) {
			subscribe_mwi();
		}
		
		// Publish presence state if not yet published.
		if (user_config->get_pres_publish_startup() && 
		    presence_epa->get_epa_state() == t_epa::EPA_UNPUBLISHED)
		{
			publish_presence(t_presence_state::ST_BASIC_OPEN);
		}
		
		// Subscribe to buddy list presence if not done so.
		if (!buddy_list->get_is_subscribed()) {
			subscribe_presence();
		}

                break;
        case R_4XX:
                is_registered = false;

                // RFC 3261 10.3
                if (r->code == R_423_INTERVAL_TOO_BRIEF) {
                        if (!r->hdr_min_expires.is_populated()) {
                                // Violation of RFC 3261 10.3 item 7
				log_file->write_report("Min-Expires header missing from 423 response.",
					"t_phone_user::handle_response_register",
					LOG_NORMAL, LOG_WARNING);
                                ui->cb_invalid_reg_resp(user_config, r,
                                        "Min-Expires header missing.");
				cleanup_registration_data();
                                return;
                        }

                        if (r->hdr_min_expires.time <= registration_time) {
                                // Wrong Min-Expires time
                                string s = "Min-Expires (";
                                s += ulong2str(r->hdr_min_expires.time);
                                s += ") is smaller than the requested ";
                                s += "time (";
                                s += ulong2str(registration_time);
                                s += ")";
                                log_file->write_report(s, "t_phone_user::handle_response_register",
                                	LOG_NORMAL, LOG_WARNING);
                                ui->cb_invalid_reg_resp(user_config, r, s);
				cleanup_registration_data();
                                return;
                        }

                        // Automatic re-register with Min-Expires time
                        registration_time = r->hdr_min_expires.time;
                        re_register = true;
                        // No need to cleanup STUN data as a new REGISTER will be
                        // sent immediately.
                        return;
                }

		// If authorization failed, then do not start the continuous
		// re-attempts. When authorization fails the user is asked
		// for credentials (in GUI). So the user cancelled these
		// questions and should not be bothered with the same question
		// again every 30 seconds. The user does not have the
		// credentials.
		if (r->code == R_401_UNAUTHORIZED ||
		    r->code == R_407_PROXY_AUTH_REQUIRED)
		{
			last_reg_failed = true;
			ui->cb_register_failed(user_config, r, true);			
	
			cleanup_registration_data();
			return;
		}

                // fall thru
        default:
		first_failure = !last_reg_failed;
                last_reg_failed = true;
                is_registered = false;
                authorizor.remove_from_cache(""); // Clear credentials cache
		ui->cb_register_failed(user_config, r, first_failure);
                phone->start_set_timer(PTMR_REGISTRATION, DUR_REG_FAILURE * 1000, this);

		cleanup_registration_data();
        }
}

void t_phone_user::handle_response_deregister(t_response *r) {
	is_registered = false;
	last_reg_failed = false;

	if (r->is_success()) {
		ui->cb_deregister_success(user_config, r);
	} else {
		ui->cb_deregister_failed(user_config, r);
	}
	
	cleanup_registration_data();
}

void t_phone_user::handle_response_query_register(t_response *r) {
	if (r->is_success()) {
		ui->cb_fetch_reg_result(user_config, r);
	} else {
		ui->cb_fetch_reg_failed(user_config, r);
	}
}

void t_phone_user::handle_response_options(t_response *r) {
	ui->cb_options_response(r);
}

void t_phone_user::handle_response_message(t_response *r) {
	t_request *req = r_message->get_request();
	
	if (req->body && req->body->get_type() == BODY_IM_ISCOMPOSING_XML) {
		// Response on message composing indication.
		if (r->code == R_415_UNSUPPORTED_MEDIA_TYPE) {
			// RFC 3994 4
			// In SIP-based IM, The composer MUST cease transmitting 
			// status messages if the receiver returned a 415 status 
			// code (Unsupported Media Type) in response to MESSAGE 
			// request containing the status indication.
			ui->cb_im_iscomposing_not_supported(user_config, r);
		}
	} else {
		// Response on instant message
		ui->cb_message_response(user_config, r, req);
	}
}

void t_phone_user::subscribe_mwi(void) {
	mwi_auto_resubscribe = true;
	
	if (mwi_dialog) {
		// This situation may occur, when an unsubscription is still
		// in progress. The subscibe will be retried after the unsubscription
		// is finished. Note that mwi_auto_resubscribe has been set to true
		// to trigger an automatic subscription.
		log_file->write_header("t_phone_user::subscribe_mwi", LOG_NORMAL, LOG_DEBUG);
		log_file->write_raw("MWI dialog already exists.\n");
		log_file->write_raw("Subscription state: ");
		log_file->write_raw(t_subscription_state2str(mwi_dialog->get_subscription_state()));
		log_file->write_endl();
		log_file->write_footer();

		return;
	}
	
	// If STUN is enabled, then do a STUN query before registering to
	// determine the public IP address.
	if (use_stun) {
		if (stun_public_ip_sip == 0)
		{
			send_stun_request();
			mwi_subscribe_after_stun = true;
			return;
		}
		stun_binding_inuse_mwi = true;
	}
	
	mwi_dialog = new t_mwi_dialog(this);
	MEMMAN_NEW(mwi_dialog);
	
	// RFC 3842 4.1
	// The example flow shows:
	// Request-URI = mail_user@mailbox_server
	// To = user@domain
	mwi_dialog->subscribe(DUR_MWI(user_config), user_config->get_mwi_uri(),
		user_config->create_user_uri(false), user_config->get_display(false));
		
	// Start sending NAT keepalive packets when STUN is used
	// (or in case of symmetric firewall)
	if (use_nat_keepalive && id_nat_keepalive == 0) {
		// Just start the NAT keepalive timer. The SUBSCRIBE
		// message will create the NAT binding. So there is
		// no need to send a NAT keep alive packet now.
		phone->start_timer(PTMR_NAT_KEEPALIVE, this);
	}
		
	cleanup_mwi_dialog();
}

void t_phone_user::unsubscribe_mwi(void) {
	mwi_auto_resubscribe = false;
	stop_resubscribe_mwi_timer();
	mwi.set_status(t_mwi::MWI_UNKNOWN);
	
	if (mwi_dialog) {
		mwi_dialog->unsubscribe();
		cleanup_mwi_dialog();
	}
	
	ui->cb_update_mwi();
}

bool t_phone_user::is_mwi_subscribed(void) const {
	if (mwi_dialog) {
		return mwi_dialog->get_subscription_state() == SS_ESTABLISHED;
	}
	
	return false;
}

bool t_phone_user::is_mwi_terminated(void) const {
	return mwi_dialog == NULL;
}

void t_phone_user::handle_mwi_unsollicited(t_request *r, t_tid tid) {
	if (user_config->get_mwi_sollicited()) {
		// Unsollicited MWI is not supported
		t_response *resp = r->create_response(R_403_FORBIDDEN);
		phone->send_response(resp, 0, tid);
		MEMMAN_DELETE(resp);
		delete resp;
		return;
	}
	
	if (r->body && r->body->get_type() == BODY_SIMPLE_MSG_SUM) {
		t_simple_msg_sum_body *body = dynamic_cast<t_simple_msg_sum_body *>(r->body);
		mwi.set_msg_waiting(body->get_msg_waiting());
		
		t_msg_summary summary;
		if (body->get_msg_summary(MSG_CONTEXT_VOICE, summary)) {
			mwi.set_voice_msg_summary(summary);
		}
		
		mwi.set_status(t_mwi::MWI_KNOWN);
	}
	
	t_response *resp = r->create_response(R_200_OK);
	phone->send_response(resp, 0, tid);
	MEMMAN_DELETE(resp);
	delete resp;
	
	ui->cb_update_mwi();
}

void t_phone_user::subscribe_presence(void) {
	assert(buddy_list);
	buddy_list->subscribe_presence();
}

void t_phone_user::unsubscribe_presence(void) {
	assert(buddy_list);
	buddy_list->unsubscribe_presence();
}

void t_phone_user::publish_presence(t_presence_state::t_basic_state basic_state) {
	assert(presence_epa);
	presence_epa->publish_presence(basic_state);
}

void t_phone_user::unpublish_presence(void) {
	assert(presence_epa);
	presence_epa->unpublish();
}

bool t_phone_user::is_presence_terminated(void) const {
	assert(buddy_list);
	return buddy_list->is_presence_terminated();
}

bool t_phone_user::send_message(const t_url &to_uri, const string &to_display, 
		const t_msg &msg)
{
	t_request *req = create_request(MESSAGE, to_uri);
	
	// To
	req->hdr_to.set_uri(to_uri);
	req->hdr_to.set_display(to_display);

	// Call-ID
	req->hdr_call_id.set_call_id(NEW_CALL_ID(user_config));

	// CSeq
	req->hdr_cseq.set_method(MESSAGE);
	req->hdr_cseq.set_seqnr(NEW_SEQNR);
	
	// Subject
	if (!msg.subject.empty()) {
		req->hdr_subject.set_subject(msg.subject);
	}
	
	// Body and Content-Type
	if (!msg.has_attachment) {
		// A message without an attachment is a text message.
		req->set_body_plain_text(msg.message, MSG_TEXT_CHARSET);
	} else {
		// Send message with file attachment
		if (!req->set_body_from_file(msg.attachment_filename, msg.attachment_media)) {
			log_file->write_header("t_phone_user::send_message", LOG_NORMAL, LOG_WARNING);
			log_file->write_raw("Could not read file ");
			log_file->write_raw(msg.attachment_filename);
			log_file->write_endl();
			log_file->write_footer();
			
			MEMMAN_DELETE(req);
			delete req;
			
			return false;
		}
		
		// Content-Disposition
		req->hdr_content_disp.set_type(DISPOSITION_ATTACHMENT);
		req->hdr_content_disp.set_filename(msg.attachment_save_as_name);
	}
	
	// Store and send request
	// Delete a possible pending options request
	if (r_message) {
		// RFC 3428 8
		// Send only 1 message at a time.
		// Store the message. It will be sent if the previous
		// message transaction is finished.
		pending_messages.push_back(req);
	} else {
		r_message = new t_client_request(user_config, req, 0);
		MEMMAN_NEW(r_message);
		phone->send_request(user_config, req, r_message->get_tuid());
		MEMMAN_DELETE(req);
		delete req;
	}
	
	return true;
}

bool t_phone_user::send_im_iscomposing(const t_url &to_uri, const string &to_display, 
			const string &state, time_t refresh)
{
	t_request *req = create_request(MESSAGE, to_uri);
	
	// To
	req->hdr_to.set_uri(to_uri);
	req->hdr_to.set_display(to_display);

	// Call-ID
	req->hdr_call_id.set_call_id(NEW_CALL_ID(user_config));

	// CSeq
	req->hdr_cseq.set_method(MESSAGE);
	req->hdr_cseq.set_seqnr(NEW_SEQNR);
	
	// Body and Content-Type
	t_im_iscomposing_xml_body *body = new t_im_iscomposing_xml_body;
	MEMMAN_NEW(body);
	
	body->set_state(state);
	body->set_refresh(refresh);
	
	req->body = body;
	req->hdr_content_type.set_media(body->get_media());
	
	// Store and send request
	// Delete a possible pending options request
	if (r_message) {
		// RFC 3428 8
		// Send only 1 message at a time.
		// Store the message. It will be sent if the previous
		// message transaction is finished.
		pending_messages.push_back(req);
	} else {
		r_message = new t_client_request(user_config, req, 0);
		MEMMAN_NEW(r_message);
		phone->send_request(user_config, req, r_message->get_tuid());
		MEMMAN_DELETE(req);
		delete req;
	}
	
	return true;
}

void t_phone_user::recvd_message(t_request *r, t_tid tid) {
	t_response *resp;
	
	if (!r->body || !MESSAGE_CONTENT_TYPE_SUPPORTED(*r)) {
		resp = r->create_response(R_415_UNSUPPORTED_MEDIA_TYPE);
		// RFC 3261 21.4.13
		SET_MESSAGE_HDR_ACCEPT(resp->hdr_accept);
		phone->send_response(resp, 0, tid);
		MEMMAN_DELETE(resp);
		delete resp;
		
		return;
	}
	
	if (r->body && r->body->get_type() == BODY_IM_ISCOMPOSING_XML) {
		// Message composing indication
		t_im_iscomposing_xml_body *sb = dynamic_cast<t_im_iscomposing_xml_body *>(r->body);
		im::t_composing_state state = im::string2composing_state(sb->get_state());
		time_t refresh = sb->get_refresh();
		
		ui->cb_im_iscomposing_request(user_config, r, state, refresh);
		resp = r->create_response(R_200_OK);
	} else {
		bool accepted = ui->cb_message_request(user_config, r);
		if (accepted) {
			resp = r->create_response(R_200_OK);
		} else {
			if (user_config->get_im_max_sessions() == 0) {
				resp = r->create_response(R_603_DECLINE);
			} else {
				resp = r->create_response(R_486_BUSY_HERE);
			}
		}
	}
	
	phone->send_response(resp, 0, tid);
	MEMMAN_DELETE(resp);
	delete resp;
}

void t_phone_user::recvd_notify(t_request *r, t_tid tid) {
	bool partial_match = false;
	
	if (r->hdr_to.tag.empty()) {
		// Unsollicited NOTIFY
		handle_mwi_unsollicited(r, tid);
		return;
	}
	
	if (mwi_dialog && mwi_dialog->match_request(r, partial_match)) {
		// Sollicited NOTIFY
		mwi_dialog->recvd_request(r, 0, tid);
		cleanup_mwi_dialog();
		return;
	}
	
	// A NOTIFY may be received before a 2XX on SUBSCRIBE.
	// In this case the NOTIFY will establish the dialog.
	if (partial_match && mwi_dialog->get_remote_tag().empty()) {
		mwi_dialog->recvd_request(r, 0, tid);
		cleanup_mwi_dialog();
		return;
	}
	
	t_buddy *buddy;
	if (buddy_list->match_request(r, &buddy)) {
		buddy->recvd_request(r, 0, tid);
		if (buddy->must_delete_now()) buddy_list->del_buddy(*buddy);
		return;
	}
	
	// RFC 3265 4.4.9
	// A SUBSCRIBE request may have forked. So multiple NOTIFY's
	// can be received. Twinkle simply rejects additional NOTIFY's with
	// a 481. This should terminate the forked dialog, such that only
	// one dialog will remain.
	t_response *resp = r->create_response(R_481_TRANSACTION_NOT_EXIST);
	phone->send_response(resp, 0, tid);
	MEMMAN_DELETE(resp);
	delete resp;
}

void t_phone_user::send_stun_request(void) {
	if (r_stun) {
		log_file->write_report("STUN request already in progress.",
			"t_phone_user::send_stun_request", LOG_NORMAL, LOG_DEBUG);
		return;
	}

	StunMessage req;
	StunAtrString username;
	username.sizeValue = 0;
	stunBuildReqSimple(&req, username, false, false);
	r_stun = new t_client_request(user_config, &req, 0);
	MEMMAN_NEW(r_stun);
	phone->send_request(user_config, &req, r_stun->get_tuid());
	return;
}

// NOTE: The term "NAT keep alive" does not cover all uses. The keep alives will
//       also be sent when there is a symmetric firewall without NAT.
void t_phone_user::send_nat_keepalive(void) {
	// Send keep-alive to registrar/proxy
	if (register_ip_port.ipaddr != 0 && register_ip_port.port != 0 &&
	    register_ip_port.transport == "udp") 
	{
		evq_sender->push_nat_keepalive(register_ip_port.ipaddr, register_ip_port.port);
	}
	
	// Send keep-alive to MWI mailbox if different from registrar/proxy
	if (mwi_dialog) {
		t_ip_port mwi_ip_port = mwi_dialog->get_remote_ip_port();
		    
		if (!mwi_ip_port.is_null() && mwi_ip_port != register_ip_port &&
		    mwi_ip_port.transport == "udp")
		{
			evq_sender->push_nat_keepalive(mwi_ip_port.ipaddr, mwi_ip_port.port);
		}
	}
}

void t_phone_user::send_tcp_ping(void) {
	if (register_ip_port.ipaddr != 0 && register_ip_port.port != 0 &&
	    register_ip_port.transport == "tcp") 
	{
		evq_sender->push_tcp_ping(user_config->create_user_uri(false),
				register_ip_port.ipaddr, register_ip_port.port);
	}
}

void t_phone_user::timeout(t_phone_timer timer) {
	switch (timer) {
	case PTMR_REGISTRATION:
		id_registration = 0;
		
		// Registration expired. Re-register.
		if (is_registered || last_reg_failed) {
			// Re-register if no register is pending
			if (!r_register) {
				registration(REG_REGISTER, true, registration_time);
			}
		}
		break;
	case PTMR_NAT_KEEPALIVE:
		id_nat_keepalive = 0;
		
		// Send a new NAT keepalive packet
		if (use_nat_keepalive) {
			send_nat_keepalive();
			phone->start_timer(PTMR_NAT_KEEPALIVE, this);
		}
		break;
	case PTMR_TCP_PING:
		id_tcp_ping = 0;
		
		// Send a TCP ping;
		if (user_config->get_persistent_tcp()) {
			send_tcp_ping();
			phone->start_timer(PTMR_TCP_PING, this);
		}
		break;
	default:
		assert(false);
	}
}

void t_phone_user::timeout_sub(t_subscribe_timer timer, t_object_id id_timer) 
{
	t_buddy *buddy;
	
	switch (timer) {
	case STMR_SUBSCRIPTION:
		if (mwi_dialog && mwi_dialog->match_timer(timer, id_timer)) {
			mwi_dialog->timeout(timer);
		} else if (buddy_list->match_timer(timer, id_timer, &buddy)) {
			buddy->timeout(timer, id_timer);
			if (buddy->must_delete_now()) buddy_list->del_buddy(*buddy);
		} else if (id_timer == id_resubscribe_mwi) {
			// Try to subscribe to MWI
			id_resubscribe_mwi = 0;
			subscribe_mwi();
		}
		break;
	default:
		assert(false);
	}
}

void t_phone_user::timeout_publish(t_publish_timer timer, t_object_id id_timer) {
	switch (timer) {
	case PUBLISH_TMR_PUBLICATION:
		if (presence_epa->match_timer(timer, id_timer)) {
			presence_epa->timeout(timer);
		}
		break;
	default:
		assert(false);
	}
}

void t_phone_user::handle_broken_connection(void) {
	log_file->write_header("t_phone_user::handle_broken_connection", LOG_NORMAL, LOG_DEBUG);
	log_file->write_raw("Handle broken connection for ");
	log_file->write_raw(user_config->get_profile_name());
	log_file->write_endl();
	log_file->write_footer();

	// A persistent connection has been broken. The connection must be re-established
	// by registering again. This is only needed when the user was registered already.
	// If no registration was present, then the persistent connection should not have
	// been established.
	if (is_registered) {
		// Re-register if no register is pending
		if (!r_register) {
			log_file->write_header("t_phone_user::handle_broken_connection", 
					LOG_NORMAL, LOG_DEBUG);
			log_file->write_raw("Re-establish broken connection for ");
			log_file->write_raw(user_config->get_profile_name());
			log_file->write_endl();
			log_file->write_footer();
			
			registration(REG_REGISTER, true, registration_time);
		}
	}
}

bool t_phone_user::match_subscribe_timer(t_subscribe_timer timer, t_object_id id_timer) const 
{
	t_buddy *buddy;
	
	if (mwi_dialog && mwi_dialog->match_timer(timer, id_timer)) {
		return true;
	}
	
	t_phone_user *self = const_cast<t_phone_user *>(this);
	if (self->buddy_list->match_timer(timer, id_timer, &buddy)) {
		return true;
	}
	
	return id_timer == id_resubscribe_mwi;
}

bool t_phone_user::match_publish_timer(t_publish_timer timer, t_object_id id_timer) const 
{
	assert(presence_epa);
	return presence_epa->match_timer(timer, id_timer);
}

void t_phone_user::start_resubscribe_mwi_timer(unsigned long duration) {
	t_tmr_subscribe	*t;
	t = new t_tmr_subscribe(duration, STMR_SUBSCRIPTION, 0, 0, SIP_EVENT_MSG_SUMMARY, "");
	MEMMAN_NEW(t);
	id_resubscribe_mwi = t->get_object_id();
	
	evq_timekeeper->push_start_timer(t);
	MEMMAN_DELETE(t);
	delete t;
}

void t_phone_user::stop_resubscribe_mwi_timer(void) {
	if (id_resubscribe_mwi != 0) {
		evq_timekeeper->push_stop_timer(id_resubscribe_mwi);
		id_resubscribe_mwi = 0;
	}
}

t_request *t_phone_user::create_request(t_method m, const t_url &request_uri) const {
	t_request *req = new t_request(m);
	MEMMAN_NEW(req);

	// From
	req->hdr_from.set_uri(user_config->create_user_uri(false));
	req->hdr_from.set_display(user_config->get_display(false));
	req->hdr_from.set_tag(NEW_TAG);

	// Max-Forwards header (mandatory)
	req->hdr_max_forwards.set_max_forwards(MAX_FORWARDS);

	// User-Agent
	SET_HDR_USER_AGENT(req->hdr_user_agent);
	
	// Set request URI and calculate destinations. By calculating
	// destinations now, the request can be resend to a next destination
	// if failover is needed.
	if (m == REGISTER) {
		// For a REGISTER do not use the service route for routing.
		req->uri = request_uri;
	} else {
		// RFC 3608
		// For all other requests, use the service route set for routing.
		req->set_route(request_uri, service_route);
	}

	req->calc_destinations(*user_config);
	
        // The Via header can only be created after the destinations
        // are calculated, because the destination deterimines which
        // local IP address should be used.
	
	// Via
	unsigned long local_ip = req->get_local_ip();
	t_via via(USER_HOST(user_config, h_ip2str(local_ip)), PUBLIC_SIP_PORT(user_config));
	req->hdr_via.add_via(via);

	return req;
}

t_response *t_phone_user::create_options_response(t_request *r,
		bool in_dialog) const
{
	t_response *resp;

	// RFC 3261 11.2
	switch(phone->get_state()) {
	case PS_IDLE:
		if (!in_dialog && service->is_dnd_active()) {
			resp = r->create_response(R_480_TEMP_NOT_AVAILABLE);
		} else {
			resp = r->create_response(R_200_OK);
		}
		break;
	case PS_BUSY:
		if (in_dialog) {
			resp = r->create_response(R_200_OK);
		} else {
			resp = r->create_response(R_486_BUSY_HERE);
		}
		break;
	default:
		assert(false);
	}

	SET_HDR_ALLOW(resp->hdr_allow, user_config);
	SET_HDR_ACCEPT(resp->hdr_accept);
	SET_HDR_ACCEPT_ENCODING(resp->hdr_accept_encoding);
	SET_HDR_ACCEPT_LANGUAGE(resp->hdr_accept_language);
	SET_HDR_SUPPORTED(resp->hdr_supported, user_config);

	if (user_config->get_ext_100rel() != EXT_DISABLED) {
		resp->hdr_supported.add_feature(EXT_100REL);
	}

	// TODO: include SDP body if requested (optional)

	return resp;
}

bool t_phone_user::get_is_registered(void) const {
	return is_registered;
}

bool t_phone_user::get_last_reg_failed(void) const {
	return last_reg_failed;
}

string t_phone_user::get_ip_sip(const string &auto_ip) const {
	if (stun_public_ip_sip) return h_ip2str(stun_public_ip_sip);
	if (user_config->get_use_nat_public_ip()) return user_config->get_nat_public_ip();
	if (LOCAL_IP == AUTO_IP4_ADDRESS) return auto_ip;
	return LOCAL_IP;
}

unsigned short t_phone_user::get_public_port_sip(void) const {
	if (stun_public_port_sip) return stun_public_port_sip;
	return sys_config->get_sip_port();
}

list<t_route> t_phone_user::get_service_route(void) const {
	return service_route;
}

bool t_phone_user::match(t_response *r, t_tuid tuid) const {
	t_buddy *dummy;

	if (r_register && r_register->get_tuid() == tuid) {
		return true;
	} else if (r_deregister && r_deregister->get_tuid() == tuid) {
		return true;
	} else if (r_query_register && r_query_register->get_tuid() == tuid) {
		return true;
	} else if (r_options && r_options->get_tuid() == tuid) {
		return true;
	} else if (r_message && r_message->get_tuid() == tuid) {
		return true;
	} else if (mwi_dialog && mwi_dialog->match_response(r, tuid)) {
		return true;
	} else if (presence_epa && presence_epa->match_response(r, tuid)) {
		return true;
	} else if (buddy_list && buddy_list->match_response(r, tuid, &dummy)) {
		return true;
	} else {
		// Response does not match any pending request.
		return false;
	}
}

bool t_phone_user::match(t_request *r) const {
	if (!r->hdr_to.tag.empty()) {
		// Match in-dialog requests
		if (mwi_dialog) {
			bool partial_match = false;
			if (mwi_dialog->match_request(r, partial_match)) return true;
			if (partial_match) return true;
		} else if (buddy_list) {
			t_buddy *dummy;
			if (buddy_list->match_request(r, &dummy)) return true;
		} else {
			return false;
		}
	}

	// Match on contact URI
	// NOTE: the host-part is not matched with the IP address to avoid
	//       NAT traversal problems. Some providers, using hosted NAT
	//       traversal, send an INVITE to username@<public_ip>. Twinkle
	//       only knows the <private_ip> in this case though. This is a
	//       fault on the provider side.
	if (r->uri.get_user() == user_config->get_contact_name()) {
		return true;
	}
	
	// Match on user URI
	if (r->uri.get_user() == user_config->get_name() &&
	    r->uri.get_host() == user_config->get_domain())
	{
		return true;
	}
	
	return false;
}

bool t_phone_user::match(StunMessage *r, t_tuid tuid) const {
	if (r_stun && r_stun->get_tuid() == tuid) return true;
	return false;
}

bool t_phone_user::authorize(t_request *r, t_response *resp) {
	if (authorizor.authorize(user_config, r, resp)) {
		return true;
	}
	return false;
}

void t_phone_user::resend_request(t_request *req, t_client_request *cr) {
	return resend_request(req, false, cr);
}

void t_phone_user::remove_cached_credentials(const string &realm) {
	authorizor.remove_from_cache(realm);
}

bool t_phone_user::is_active(void) const {
	return active;
}

void t_phone_user::activate(const t_user &user) {
	// Replace old user config with the new passed user config, because
	// the user config might have been edited while this phone user was
	// inactive.
	delete user_config;
	MEMMAN_DELETE(user_config);
	user_config = user.copy();
	
	// Initialize registration data
	register_seqnr = NEW_SEQNR;
	is_registered = false;
	register_ip_port.ipaddr = 0L;
	register_ip_port.port = 0;
	last_reg_failed = false;
	
	// Initialize STUN data
	stun_public_ip_sip = 0L;
	stun_public_port_sip = 0;
	use_stun = false;
	use_nat_keepalive = false;
	
	active = true;
}

void t_phone_user::deactivate(void) {
	// Stop timers
	if (id_registration) phone->stop_timer(PTMR_REGISTRATION, this);
	if (id_nat_keepalive) phone->stop_timer(PTMR_NAT_KEEPALIVE, this);
	if (id_tcp_ping) phone->stop_timer(PTMR_TCP_PING, this);
	if (id_resubscribe_mwi) stop_resubscribe_mwi_timer();

	// Clear MWI
	if (mwi_dialog) {
		MEMMAN_DELETE(mwi_dialog);
		delete mwi_dialog;
		mwi_dialog = NULL;
	}
	mwi.set_status(t_mwi::MWI_UNKNOWN);
	mwi_auto_resubscribe = false;
	
	// Clear presence state
	// presence_epa->clear();
	buddy_list->clear_presence();
	
	// Clear STUN
	stun_binding_inuse_registration = false;
	stun_binding_inuse_mwi = false;
	stun_binding_inuse_presence = 0;
	cleanup_registration_data();
	cleanup_stun_data();
	
	active = false;
}
