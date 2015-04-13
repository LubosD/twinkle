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
 * Threads communicate by passing events via event queues.
 */

#ifndef _EVENTS_H
#define _EVENTS_H

#include <queue>
#include "protocol.h"
#include "timekeeper.h"
#include "stun/stun.h"
#include "audio/audio_codecs.h"
#include "parser/sip_message.h"
#include "sockets/socket.h"
#include "sockets/url.h"
#include "threads/mutex.h"
#include "threads/sema.h"

using namespace std;

// Forward declarations
class t_userintf;

/** Different types of events. */
enum t_event_type {
	EV_QUIT,		/**< Generic quit event */
	EV_NETWORK,		/**< Network event, eg. SIP message from/to network */
	EV_USER,		/**< User event, eg. SIP message from/to user  */
	EV_TIMEOUT,		/**< Timer expiry */
	EV_FAILURE,		/**< Failure, eg. transport failure */
	EV_START_TIMER,		/**< Start timer */
	EV_STOP_TIMER,		/**< Stop timer */
	EV_ABORT_TRANS,		/**< Abort transaction */
	EV_STUN_REQUEST,	/**< Outgoing STUN request */
	EV_STUN_RESPONSE,	/**< Received STUN response */
	EV_NAT_KEEPALIVE,	/**< Send a NAT keep alive packet */
	EV_ICMP,		/**< ICMP error */
	EV_UI,			/**< User interface event */
	EV_ASYNC_RESPONSE,	/**< Response on an asynchronous question */
	EV_BROKEN_CONNECTION,	/**< Persitent connection to SIP proxy broken */
	EV_TCP_PING,		/**< Send a TCP ping (double CRLF) */
};

/** Abstract parent class for all events */
class t_event {
public:
	virtual ~t_event() {}
	
	/** Get the type of this event. */
	virtual t_event_type get_type(void) const = 0;
};


/** 
 * Generic quit event.
 * The quit event instructs a thread to exit gracefully.
 */
class t_event_quit : public t_event {
public:
	virtual ~t_event_quit();
	virtual t_event_type get_type(void) const;
};


/** 
 * Network events.
 * A network event is a SIP message going from the transaction manager
 * to the network or v.v.
 */
class t_event_network : public t_event {
private:
	/** The SIP message. */
	t_sip_message	*msg;

public:
	unsigned int	src_addr; /**< Source IP address of the SIP message (host order). */
	unsigned short	src_port; /**< Source port of the SIP message (host order). */
	unsigned int	dst_addr; /**< Destination IP address of the SIP message (host order). */
	unsigned short	dst_port; /**< Destination port of the SIP message (host order). */
	string		transport; /**< Transport protocol */

	/**
	 * Constructor.
	 * The event will keep a copy of the SIP message.
	 * @param m [in] The SIP message.
	 */
	t_event_network(t_sip_message *m);
	
	~t_event_network();
	
	t_event_type get_type(void) const;
	
	/** 
	 * Get the SIP message.
	 * @return Pointer to the SIP message inside this event.
	 */
	t_sip_message *get_msg(void) const;
};


/**
 * User events.
 * A user event is a SIP message going from the user to the
 * transaction manager or v.v.
 */
class t_event_user : public t_event {
private:
	t_sip_message	*msg;	/**< The SIP message. */
	unsigned short	tuid;	/**< Transaction user id. */
	unsigned short	tid;	/**< Transaction id. */

	/**
	 * Transaction id that is the target of the CANCEL message.
	 * Only set if tid is a CANCEL transaction and the event
	 * is sent towards the user.
	 */
	unsigned short	tid_cancel_target;
	
	/** User profile of the user sending/receiving the SIP message. */
	t_user		*user_config;

public:
	/** Constructor.
	 * @param u [in] User profile.
	 * @param m [in] SIP message
	 * @param _tuid [in] Transaction user id associated with this message.
	 * @param _tid [in] Transaction id of the transaction for this message.
	 */
	t_event_user(t_user *u, t_sip_message *m, unsigned short _tuid,
		unsigned short _tid);
		
	/** Constructor for CANCEL request towards the user.
	 * @param u [in] User profile.
	 * @param m [in] SIP message.
	 * @param _tuid [in] Transaction user id associated with this message.
	 * @param _tid [in] Transaction id of the transaction for this message.
	 * @param _tid_cancel_target [in] Id of the target transaction of a CANCEL request.
	 */
	t_event_user(t_user *u, t_sip_message *m, unsigned short _tuid,
		unsigned short _tid, unsigned short _tid_cancel_target);
		
	~t_event_user();
	
	t_event_type get_type(void) const;
	
	/** 
	 * Get the SIP message.
	 * @return Pointer to the SIP message inside this event.
	 */
	t_sip_message *get_msg(void) const;
	
	/** Get transaction user id. */
	unsigned short get_tuid(void) const;
	
	/** Get transaction id. */
	unsigned short get_tid(void) const;
	
	/** Get the CANCEL target transaction id. */
	unsigned short get_tid_cancel_target(void) const;
	
	/**
	 * Get the user profile.
	 * @return Pointer to the user profile inside the event.
	 */
	t_user *get_user_config(void) const;
};


/**
 * Time out events.
 * Expiration of a timer is signalled by a time out event.
 */
class t_event_timeout : public t_event {
private:
	/**
	 * The epxired timer.
	 * @note Timer pointer will be deleted upon destruction of the object.
	 */
	t_timer		*timer;

public:
	/**
	 * Constructor.
	 * @param t [in] The expired timer.
	 * @note The event will keep a copy of the timer.
	 */
	t_event_timeout(t_timer *t);
	
	~t_event_timeout();
	
	t_event_type get_type(void) const;
	
	/**
	 * Get the timer from the event.
	 * @return The timer.
	 */
	t_timer *get_timer(void) const;
};

/**
 * Failure events.
 */
class t_event_failure : public t_event {
private:
	t_failure	failure;	/**< Type of failure. */
	
	/**
	 * Indicates if the tid value is populated. If the tid value is not
	 * populated, then the branch and cseq_method are populated.
	 */
	bool		tid_populated;
	
	unsigned short	tid;		/**< Id of transaction that failed. */
	
	string		branch;		/**< Branch parameter of SIP message that failed. */
	t_method	cseq_method;	/**< CSeq method of SIP message that failed. */
public:
	/**
	 * Constructor.
	 * @param f [in] Type of failure.
	 * @param _tid [in] Transaction id.
	 */
	t_event_failure(t_failure f, unsigned short _tid);
	
	/** Constructor */
	t_event_failure(t_failure f, const string &_branch, const t_method &_cseq_method);
	
	t_event_type get_type(void) const;
	
	/**
	 * Get the type of failure.
	 * @return Type of failure.
	 */
	t_failure get_failure(void) const;
	
	/**
	 * Get the transaction id.
	 * @return Transaction id.
	 */
	unsigned short get_tid(void) const;
	
	/** Get branch parameter. */
	string get_branch(void) const;
	
	/** Get CSeq method. */
	t_method get_cseq_method(void) const;
	
	/** Check if tid is populated. */
	bool is_tid_populated(void) const;
};


/**
 * Start timer event.
 * A start timer event instructs the time keeper to start a timer.
 */
class t_event_start_timer : public t_event {
private:
	t_timer		*timer;		/**< The timer to start. */

public:
	/**
	 * Constructor.
	 * @param t [in] The timer to start.
	 */
	t_event_start_timer(t_timer *t);
	
	
	t_event_type get_type(void) const;
	
	/**
	 * Get the timer.
	 * @return Timer.
	 */
	t_timer *get_timer(void) const;
};


/**
 * Stop timer event
 * A stop timer event instructs the time keeper to stop a timer.
 */
class t_event_stop_timer : public t_event {
private:
	/** Id of the timer to stop. */
	unsigned short	timer_id;

public:
	/**
	 * Constructor.
	 * @param id [in] Id of the timer to stop.
	 */
	t_event_stop_timer(unsigned short id);
	
	t_event_type get_type(void) const;
	
	/**
	 * Get the timer id.
	 * @return Timer id.
	 */
	unsigned short get_timer_id(void) const;
};


/**
 * Abort transaction event.
 * With an abort transaction event, the requester asks the transaction
 * manager to abort a pending transaction.
 */
class t_event_abort_trans : public t_event {
private:
	unsigned short	tid; /**< Id of the transaction to abort. */
public:
	/**
	 * Constructor.
	 * @param _tid [in] Transaction id.
	 */
	t_event_abort_trans(unsigned short _tid);
	
	t_event_type get_type(void) const;
	
	/**
	 * Get transaction id.
	 * @return Transaction id.
	 */
	unsigned short get_tid(void) const;
};


/** STUN event types. */
enum t_stun_event_type {
	TYPE_STUN_SIP,		/**< Request to open a port for SIP. */
	TYPE_STUN_MEDIA,	/**< Request to open a port for media. */
};	

/**
 * STUN request event.
 */
class t_event_stun_request : public t_event {
private:
	StunMessage		*msg;		/**< STUN request to send. */
	unsigned short		tuid;		/**< Transaction user id. */
	unsigned short		tid;		/**< Transaction id. */
	t_stun_event_type	stun_event_type; /**< Type of STUN event. */
	t_user			*user_config;	/**< User profile associated with this request. */

public:
	unsigned int	dst_addr;	/**< Destination address of request (host order). */
	unsigned short	dst_port;	/**< Destination port of request (host order). */
	unsigned short	src_port;	/**< Source port for media event type (host order). */

	/** Constructor. */
	t_event_stun_request(t_user *u, StunMessage *m, t_stun_event_type ev_type,
		unsigned short _tuid, unsigned short _tid);
		
	~t_event_stun_request();
	
	t_event_type get_type(void) const;
	
	/** Get STUN message. */
	StunMessage *get_msg(void) const;
	
	/** Get transaction user id. */
	unsigned short get_tuid(void) const;
	
	/** Get transaction id. */
	unsigned short get_tid(void) const;
	
	/** Get STUN event type. */
	t_stun_event_type get_stun_event_type(void) const;
	
	/** Get user profile. */
	t_user *get_user_config(void) const;
};


/**
 * STUN response event.
 */
class t_event_stun_response : public t_event {
private:
	StunMessage	*msg;		/**< STUN request to send. */
	unsigned short	tuid;		/**< Transaction user id. */
	unsigned short	tid;		/**< Transaction id. */

public:
	/** Constructor. */
	t_event_stun_response(StunMessage *m, unsigned short _tuid,
		unsigned short _tid);
		
	~t_event_stun_response();
	
	t_event_type get_type(void) const;
	
	/** Get STUN message. */
	StunMessage *get_msg(void) const;
	
	/** Get transaction user id. */
	unsigned short get_tuid(void) const;
	
	/** Get transaction id. */
	unsigned short get_tid(void) const;
};


/**
 * NAT keep alive event.
 * Request to send a NAT keep alive message.
 */
class t_event_nat_keepalive : public t_event {
public:
	unsigned int	dst_addr;	/**< Destination address for keepalive (host order) */
	unsigned short	dst_port;	/**< Destination port (host order) */
	
	t_event_type get_type(void) const;
};


/**
 * ICMP event.
 * This event signals the reception of an ICMP error.
 */
class t_event_icmp : public t_event {
private:
	t_icmp_msg	icmp;	/**< The received ICMP message. */

public:
	/**
	 * Constructor.
	 * @param m [in] ICMP message.
	 */
	t_event_icmp(const t_icmp_msg &m);
	
	t_event_type get_type(void) const;
	
	/**
	 * Get the ICMP message.
	 * @return ICMP message.
	 */
	t_icmp_msg get_icmp(void) const;
};


/** User interface callback types. */
enum t_ui_event_type {
	TYPE_UI_CB_DISPLAY_MSG,			/**< Display a message */
	TYPE_UI_CB_DTMF_DETECTED,		/**< DTMF tone detected */
	TYPE_UI_CB_SEND_DTMF,			/**< Sending DTMF */
	TYPE_UI_CB_RECV_CODEC_CHANGED,		/**< Codec changed */
	TYPE_UI_CB_LINE_STATE_CHANGED,		/**< Line state changed */
	TYPE_UI_CB_LINE_ENCRYPTED,		/**< Line is now encrypted */
	TYPE_UI_CB_SHOW_ZRTP_SAS,		/**< Show the ZRTP SAS */
	TYPE_UI_CB_ZRTP_CONFIRM_GO_CLEAR,	/**< ZRTP Confirm go-clear */
	TYPE_UI_CB_QUIT				/**< Quit the user interface */
};

/** Display message priorities. */
enum t_msg_priority {
	MSG_NO_PRIO,
	MSG_INFO,
	MSG_WARNING,
	MSG_CRITICAL
};

/**
 * User interface event.
 * Send a user interface callback to the user interface.
 * Most callbacks are called directly as a function call.
 * Sometimes an asynchronous callback is needed. That's where
 * this event is used for.
 */
class t_event_ui : public t_event {
private:
	t_ui_event_type	type;	/**< User interface callback type. */
	
	/** @name Parameters for call back functions */
	//@{
	int		line;		/**< Line number. */
	t_audio_codec	codec;		/**< Audio codec. */
	char		dtmf_event;	/**< DTMF event. */
	bool		encrypted;	/**< Encryption indication. */
	string		cipher_mode;	/**< Cipher mode (algorithm name). */
	string		zrtp_sas;	/**< ZRTP SAS/ */
	t_msg_priority	msg_priority;	/**< Priority of a display message. */
	string		msg;		/**> Message to display. */
	//@}

public:
	/**
	 * Constructor.
	 * @param _type [in] Type of callback.
	 */
	t_event_ui(t_ui_event_type _type);
	
	t_event_type get_type(void) const;
	
	/** @name Set parameters for call back functions */
	//@{
	void set_line(int _line);
	void set_codec(t_audio_codec _codec);
	void set_dtmf_event(char _dtmf_event);
	void set_encrypted(bool on);
	void set_cipher_mode(const string &_cipher_mode);
	void set_zrtp_sas(const string &sas);
	void set_display_msg(const string &_msg, t_msg_priority &_msg_priority);
	//@}
	
	/**
	 * Call the callback function.
	 * @param user_intf [in] The user interface that receives the callback.
	 */
	void exec(t_userintf *user_intf);
};


/**
 * Asynchronous response event.
 * A user interface can open an asynchronous message box to request
 * information from the user. Via this event the user interface signals
 * the response from the user.
 */
class t_event_async_response : public t_event {
public:
	/** Response type */
	enum t_response_type {
		RESP_REFER_PERMISSION	/**< Response on permission to refer question */
	};
	
private:
	t_response_type		response_type;	/**< Response type. */
	bool			bool_response;	/**< Boolean response. */
	
public:
	/**
	 * Constructor.
	 * @param type [in] The response type.
	 */
	t_event_async_response(t_response_type type);
	
	t_event_type get_type(void) const;
	
	/**
	 * Set the boolean response.
	 * @param b [in] The response.
	 */
	void set_bool_response(bool b);
	
	/**
	 * Get response type.
	 * @return Response type.
	 */
	t_response_type get_response_type(void) const;
	
	/**
	 * Get boolean response.
	 * @return The response.
	 */
	bool get_bool_response(void) const;
};

/**
 * Broken connection event.
 * A persistent connection to a SIP proxy is broken. With this event
 * the transport layer signals the transaction layer that a connection
 * is broken.
 */
class t_event_broken_connection : public t_event {
private:
	/** The user URI (AoR) that the connection was associated with. */
	t_url		user_uri_;
	
public:
	/** Constructor */
	t_event_broken_connection(const t_url &url);
	
	t_event_type get_type(void) const;
	
	/**
	 * Get the user URI.
	 * @return The user URI.
	 */
	t_url get_user_uri(void) const;
};

/**
 * TCP ping event.
 * Send a TCP ping (double CRLF).
 */
class t_event_tcp_ping : public t_event {
private:
	/** The user URI (AoR) for which the ping must be sent. */
	t_url		user_uri_;
	
	unsigned int	dst_addr_;	/**< Destination address for ping (host order) */
	unsigned short	dst_port_;	/**< Destination port (host order) */
	
public:
	/** Constructor */
	t_event_tcp_ping(const t_url &url, unsigned int dst_addr, unsigned short dst_port);
	
	t_event_type get_type(void) const;
	
	/** @name Getters */
	//@{
	t_url get_user_uri(void) const;
	unsigned int get_dst_addr(void) const;
	unsigned short get_dst_port(void) const;
	//@}
};


/**
 * Event queue.
 * An event queue is the communication pipe between multiple
 * threads. Multiple threads write events into the queue and
 * one thread reads the events from the queue and processes them
 * Access to the queue is protected by a mutex. A semaphore is
 * used to synchronize the reader with the writers of the queue.
 */
class t_event_queue {
private:
	queue<t_event *>	ev_queue;	/**< Queue of events. */
	t_mutex			mutex_evq;	/**< Mutex to protect access to the queue. */
	t_semaphore		sema_evq;	/**< Semephore counting the number of events. */

	/**
	 * Semaphore to signal an interrupt.
	 * Will be posted when the interrupt method is called.
	 */
	t_semaphore		sema_caught_interrupt;

public:
	/** Constructor. */
	t_event_queue();
	
	~t_event_queue();

	/**
	 * Push an event into the queue.
	 * @param e [in] Event
	 */
	void push(t_event *e);
	
	/** Push a quit event into the queue. */
	void push_quit(void);

	/**
	 * Create a network event and push it into the queue.
	 * @param m [in] SIP message.
	 * @param ipaddr [in] Destination address of the message (host order).
	 * @param port [in] Port of the message (host order).
	 */
	void push_network(t_sip_message *m, const t_ip_port &ip_port);

	/**
	 * Create a user event and push it into the queue.
	 * The user event must be associated with a user profile.
	 * @param user_config [in] The user profile.
	 * @param m [in] SIP message.
	 * @param tuid [in] Transaction user id.
	 * @param tid [in] Transaction id.
	 */
	void push_user(t_user *user_config, t_sip_message *m, unsigned short tuid,
		unsigned short tid);
		
	/**
	 * Create a user event and push it into the queue.
	 * The user event must be unrelated to a particular user profile.
	 * @param m [in] SIP message.
	 * @param tuid [in] Transaction user id.
	 * @param tid [in] Transaction id.
	 */
	void push_user(t_sip_message *m, unsigned short tuid,
		unsigned short tid);

	/**
	 * Create a cancel event for a user.
	 * @param user_config [in] The user profile.
	 * @param m [in] SIP message.
	 * @param tuid [in] Transaction user id.
	 * @param tid [in] Transaction id.
	 */
	void push_user_cancel(t_user *user_config, t_sip_message *m, unsigned short tuid,
		unsigned short tid, unsigned short target_tid);
		
	/**
	 * Create a cancel event for a user.
	 * @param m [in] SIP message.
	 * @param tuid [in] Transaction user id.
	 * @param tid [in] Transaction id.
	 */
	void push_user_cancel(t_sip_message *m, unsigned short tuid,
		unsigned short tid, unsigned short target_tid);

	/**
	 * Create a timeout event and push it into the queue.
	 * @param t [in] The timer that expired.
	 */
	void push_timeout(t_timer *t);

	/**
	 * Create failure event and push it into the queue.
	 * @param f [in] Type of failure.
	 * @param tid [in] Transaction id of failed transaction.
	 */
	void push_failure(t_failure f, unsigned short tid);
	
	/**
	 * Create failure event and push it into the queue.
	 * @param f [in] Type of failure.
	 * @param branch [in] Branch parameter of failed transaction.
	 * @param cseq_method [in] CSeq method of failed transaction.
	 */
	void push_failure(t_failure f, const string &branch, const t_method &cseq_method);

	/**
	 * Create a start timer event.
	 * @param t [in] Timer to start.
	 */
	void push_start_timer(t_timer *t);

	/**
	 * Create a stop timer event.
	 * @param timer_id [in] Timer id of timer to stop.
	 */
	void push_stop_timer(unsigned short timer_id);

	/**
	 * Create an abort transaction event.
	 * @param tid [in] Transaction id of transaction to abort.
	 */
	void push_abort_trans(unsigned short tid);
	
	/**
	 * Create a STUN request event.
	 * @param user_config [in] The user profile associated with the request.
	 * @param m [in] STUN request.
	 * @param ev_type [in] Type of STUN event.
	 * @param tuid [in] Transaction user id.
	 * @param tid [in] Transaction id.
	 * @param ipaddr [in] Destination address (host order)
	 * @param port [in] Destination port (host order)
	 * @param src_port [in] Source port of media. This must only be passed for
	 *   a media STUN event.
	 */
	void push_stun_request(t_user *user_config, StunMessage *m, t_stun_event_type ev_type,
		unsigned short tuid, unsigned short tid,
		unsigned long ipaddr, unsigned short port, unsigned short src_port = 0);
		
	/**
	 * Create a STUN response event.
	 * @param m [in] STUN response.
	 * @param tuid [in] Transaction user id.
	 * @param tid [in] Transaction id.
	 */
	void push_stun_response(StunMessage *m,
		unsigned short tuid, unsigned short tid);
		
	/**
	 * Create a NAT keepalive event.
	 * @param ipaddr [in] Destination address (host order)
	 * @param port [in] Destination port (host order)
	 */
	void push_nat_keepalive(unsigned long ipaddr, unsigned short port);
	
	/**
	 * Create ICMP event.
	 * @param m [in] ICMP message.
	 */
	void push_icmp(const t_icmp_msg &m);
	
	/**
	 * Create a REFER pemission response event.
	 * @param permission [in] Permission allowed?.
	 */
	void push_refer_permission_response(bool permission);
	
	/**
	 * Create a broken connection event.
	 * @param user_uri [in] The user URI (AoR) associated with the connection.
	 */
	void push_broken_connection(const t_url &user_uri);
	
	/**
	 * Create a TCP ping event.
	 * @param user_uri [in] The user URI (AoR) for which the TCP ping must be sent.
	 * @param dst_addr [in] The destination IPv4 address for the ping.
	 * @param dst_port [in] The destination TCP port for the ping.
	 */
	void push_tcp_ping(const t_url &user_uri, unsigned int dst_addr, unsigned short dst_port);

	/**
	 * Pop an event from the queue. 
	 * If the queue is empty then the thread will be blocked until an 
	 * event arrives.
	 * @return The popped event.
	 */
	t_event *pop(void);

	/**
	 * Pop an event from the queue.
	 * Same method as above, but this one can be interrupted by
	 * calling the method interrupt.
	 * @param interrupted [out] When the pop operation is interrupted this
	 * parameter is set to true. Otherwise it is false.
	 * @return NULL, when interrupted.
	 * @return The popped event, otherwise.
	 */
	t_event *pop(bool &interrupted);

	/**
	 * Send an interrupt. 
	 * This will cause the interruptable pop to return.
	 * A non-interruptable pop will ignore the interrupt.
	 * If pop is currently not suspending the thread execution then the
	 * next call to pop will catch the interrupt.
	 */
	void interrupt(void);
};

#endif
