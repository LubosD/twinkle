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

#include <iostream>
#include "events.h"
#include "log.h"
#include "userintf.h"
#include "util.h"
#include "audits/memman.h"

string event_type2str(t_event_type t) {
	switch(t) {
	case EV_QUIT:		return "EV_QUIT";
	case EV_NETWORK: 	return "EV_NETWORK";
	case EV_USER: 		return "EV_USER";
	case EV_TIMEOUT: 	return "EV_TIMEOUT";
	case EV_FAILURE: 	return "EV_FAILURE";
	case EV_START_TIMER: 	return "EV_START_TIMER";
	case EV_STOP_TIMER: 	return "EV_STOP_TIMER";
	case EV_ABORT_TRANS:	return "EV_ABORT_TRANS";
	case EV_STUN_REQUEST:	return "EV_STUN_REQUEST";
	case EV_STUN_RESPONSE:	return "EV_STUN_RESPONSE";
	case EV_NAT_KEEPALIVE:	return "EV_NAT_KEEPALIVE";
	case EV_ICMP:		return "EV_ICMP";
	case EV_UI:		return "EV_UI";
	case EV_ASYNC_RESPONSE:	return "EV_ASYNC_RESPONSE";
	case EV_BROKEN_CONNECTION: return "EV_BROKEN_CONNECTION";
	case EV_TCP_PING:	return "EV_TCP_PING";
	}

	return "UNKNOWN";
}

///////////////////////////////////////////////////////////
// class t_event_network
///////////////////////////////////////////////////////////

t_event_network::t_event_network(t_sip_message *m) : t_event() {
	msg = m->copy();
	src_addr = 0;
	src_port = 0;
	dst_addr = 0;
	dst_port = 0;
	transport.clear();
}

t_event_network::~t_event_network() {
	MEMMAN_DELETE(msg);
	delete msg;
}

t_event_type t_event_network::get_type(void) const {
	return EV_NETWORK;
}

t_sip_message *t_event_network::get_msg(void) const {
	return msg;
}

///////////////////////////////////////////////////////////
// class t_event_quit
///////////////////////////////////////////////////////////

t_event_quit::~t_event_quit() {}

t_event_type t_event_quit::get_type(void) const {
	return EV_QUIT;
}

///////////////////////////////////////////////////////////
// class t_event_user
///////////////////////////////////////////////////////////

t_event_user::t_event_user(t_user *u, t_sip_message *m, unsigned short _tuid,
		unsigned short _tid) : t_event()
{
	msg = m->copy();
	tuid = _tuid;
	tid = _tid;
	tid_cancel_target = 0;
	if (u) {
		user_config = u->copy();
	} else {
		user_config = NULL;
	}
}

t_event_user::t_event_user(t_user *u, t_sip_message *m, unsigned short _tuid,
		unsigned short _tid, unsigned short _tid_cancel_target) :
			t_event()
{
	msg = m->copy();
	tuid = _tuid;
	tid = _tid;
	tid_cancel_target = _tid_cancel_target;
	if (u) {
		user_config = u->copy();
	} else {
		user_config = NULL;
	}
}

t_event_user::~t_event_user() {
	MEMMAN_DELETE(msg);
	delete msg;
	
	if (user_config) {
		MEMMAN_DELETE(user_config);
		delete user_config;
	}
}

t_event_type t_event_user::get_type(void) const {
	return EV_USER;
}

t_sip_message *t_event_user::get_msg(void) const {
	return msg;
}

unsigned short t_event_user::get_tuid(void) const {
	return tuid;
}

unsigned short t_event_user::get_tid(void) const {
	return tid;
}

unsigned short t_event_user::get_tid_cancel_target(void) const {
	return tid_cancel_target;
}

t_user *t_event_user::get_user_config(void) const {
	return user_config;
}

///////////////////////////////////////////////////////////
// class t_event_timeout
///////////////////////////////////////////////////////////

t_event_timeout::t_event_timeout(t_timer *t) : t_event() {
	timer = t->copy();
}

t_event_timeout::~t_event_timeout() {
	MEMMAN_DELETE(timer);
	delete timer;
}

t_event_type t_event_timeout::get_type(void) const {
	return EV_TIMEOUT;
}

t_timer *t_event_timeout::get_timer(void) const {
	return timer;
}

///////////////////////////////////////////////////////////
// class t_event_failure
///////////////////////////////////////////////////////////
t_event_failure::t_event_failure(t_failure f, unsigned short _tid) :
	t_event(),
	failure(f),
	tid_populated(true),
	tid(_tid)
{}

t_event_failure::t_event_failure(t_failure f, const string &_branch, const t_method &_cseq_method) :
		failure(f),
		tid_populated(false),
		branch(_branch),
		cseq_method(_cseq_method)
{}

t_event_type t_event_failure::get_type(void) const {
	return EV_FAILURE;
}

t_failure t_event_failure::get_failure(void) const {
	return failure;
}

unsigned short t_event_failure::get_tid(void) const {
	return tid;
}

string t_event_failure::get_branch(void) const {
	return branch;
}

t_method t_event_failure::get_cseq_method(void) const {
	return cseq_method;
}

bool t_event_failure::is_tid_populated(void) const {
	return tid_populated;
}

///////////////////////////////////////////////////////////
// class t_event_start_timer
///////////////////////////////////////////////////////////
t_event_start_timer::t_event_start_timer(t_timer *t) :
	t_event()
{
	timer = t->copy();
}

t_event_type t_event_start_timer::get_type(void) const {
	return EV_START_TIMER;
}

t_timer *t_event_start_timer::get_timer(void) const {
	return timer;
}

///////////////////////////////////////////////////////////
// class t_event_stop_timer
///////////////////////////////////////////////////////////
t_event_stop_timer::t_event_stop_timer(unsigned short id) :
	t_event()
{
	timer_id = id;
}

t_event_type t_event_stop_timer::get_type(void) const {
	return EV_STOP_TIMER;
}

unsigned short t_event_stop_timer::get_timer_id(void) const {
	return timer_id;
}

///////////////////////////////////////////////////////////
// class t_event_abort_trans
///////////////////////////////////////////////////////////
t_event_abort_trans::t_event_abort_trans(unsigned short _tid) :
	t_event()
{
	tid = _tid;
}

t_event_type t_event_abort_trans::get_type(void) const {
	return EV_ABORT_TRANS;
}

unsigned short t_event_abort_trans::get_tid(void) const {
	return tid;
}

///////////////////////////////////////////////////////////
// class t_event_stun_request
///////////////////////////////////////////////////////////

t_event_stun_request::t_event_stun_request(t_user *u,
		StunMessage *m, t_stun_event_type ev_type,
		unsigned short _tuid, unsigned short _tid) : t_event()
{
	msg = new StunMessage(*m);
	MEMMAN_NEW(msg);
	stun_event_type = ev_type;
	tuid = _tuid;
	tid = _tid;
	dst_addr = 0;
	dst_port = 0;
	user_config = u->copy();
}

t_event_stun_request::~t_event_stun_request() {
	MEMMAN_DELETE(msg);
	delete msg;
	MEMMAN_DELETE(user_config);
	delete user_config;
}

t_event_type t_event_stun_request::get_type(void) const {
	return EV_STUN_REQUEST;
}
	
StunMessage *t_event_stun_request::get_msg(void) const {
	return msg;
}

unsigned short t_event_stun_request::get_tuid(void) const {
	return tuid;
}
unsigned short t_event_stun_request::get_tid(void) const {
	return tid;
}

t_stun_event_type t_event_stun_request::get_stun_event_type(void) const {
	return stun_event_type;
}

t_user *t_event_stun_request::get_user_config(void) const {
	return user_config;
}

///////////////////////////////////////////////////////////
// class t_event_stun_response
///////////////////////////////////////////////////////////

t_event_stun_response::t_event_stun_response(StunMessage *m, unsigned short _tuid,
		unsigned short _tid) : t_event()
{
	msg = new StunMessage(*m);
	MEMMAN_NEW(msg);
	tuid = _tuid;
	tid = _tid;
}

t_event_stun_response::~t_event_stun_response() {
	MEMMAN_DELETE(msg);
	delete(msg);
}

t_event_type t_event_stun_response::get_type(void) const {
	return EV_STUN_RESPONSE;
}

StunMessage *t_event_stun_response::get_msg(void) const {
	return msg;
}

unsigned short t_event_stun_response::get_tuid(void) const {
	return tuid;
}

unsigned short t_event_stun_response::get_tid(void) const {
	return tid;
}

///////////////////////////////////////////////////////////
// class t_event_nat_keepalive
///////////////////////////////////////////////////////////
t_event_type t_event_nat_keepalive::get_type(void) const {
	return EV_NAT_KEEPALIVE;
}

///////////////////////////////////////////////////////////
// class t_event_icmp
///////////////////////////////////////////////////////////
t_event_icmp::t_event_icmp(const t_icmp_msg &m) : icmp(m) {}

t_event_type t_event_icmp::get_type(void) const {
	return EV_ICMP;
}

t_icmp_msg t_event_icmp::get_icmp(void) const {
	return icmp;
}

///////////////////////////////////////////////////////////
// class t_event_ui
///////////////////////////////////////////////////////////
t_event_ui::t_event_ui(t_ui_event_type _type) : 
	t_event(),
	type(_type) 
{}

t_event_type t_event_ui::get_type(void) const {
	return EV_UI;
}

void t_event_ui::set_line(int _line) {
	line = _line;
}

void t_event_ui::set_codec(t_audio_codec _codec) {
	codec = _codec;
}

void t_event_ui::set_dtmf_event(char _dtmf_event) {
	dtmf_event = _dtmf_event;
}

void t_event_ui::set_encrypted(bool on) {
	encrypted = on;
}

void t_event_ui::set_cipher_mode(const string &_cipher_mode) {
	cipher_mode = _cipher_mode;
}

void t_event_ui::set_zrtp_sas(const string &sas) {
	zrtp_sas = sas;
}

void t_event_ui::set_display_msg(const string &_msg, t_msg_priority &_msg_priority) {
	msg = _msg;
	msg_priority = _msg_priority;
}

void t_event_ui::exec(t_userintf *user_intf) {
	switch (type) {
	case TYPE_UI_CB_DTMF_DETECTED:
		ui->cb_dtmf_detected(line, dtmf_event);
		break;
	case TYPE_UI_CB_SEND_DTMF:
		ui->cb_send_dtmf(line, dtmf_event);
		break;
	case TYPE_UI_CB_RECV_CODEC_CHANGED:
		ui->cb_recv_codec_changed(line, codec);
		break;
	case TYPE_UI_CB_LINE_STATE_CHANGED:
		ui->cb_line_state_changed();
		break;
	case TYPE_UI_CB_LINE_ENCRYPTED:
		ui->cb_line_encrypted(line, encrypted, cipher_mode);
		break;
	case TYPE_UI_CB_SHOW_ZRTP_SAS:
		ui->cb_show_zrtp_sas(line, zrtp_sas);
		break;
	case TYPE_UI_CB_ZRTP_CONFIRM_GO_CLEAR:
		ui->cb_zrtp_confirm_go_clear(line);
		break;
	case TYPE_UI_CB_QUIT:
		ui->cmd_quit();
		break;
	default:
		assert(false);
	}
}

///////////////////////////////////////////////////////////
// class t_event_async_response
///////////////////////////////////////////////////////////

t_event_async_response::t_event_async_response(t_response_type type) :
	t_event(),
	response_type(type)
{}

t_event_type t_event_async_response::get_type(void) const {
	return EV_ASYNC_RESPONSE;
}

void t_event_async_response::set_bool_response(bool b) {
	bool_response = b;
}

t_event_async_response::t_response_type t_event_async_response::get_response_type(void) const {
	return response_type;
}

bool t_event_async_response::get_bool_response(void) const {
	return bool_response;
}

///////////////////////////////////////////////////////////
// class t_event_broken_connection
///////////////////////////////////////////////////////////

t_event_broken_connection::t_event_broken_connection(const t_url &url) :
	t_event(),
	user_uri_(url)
{}

t_event_type t_event_broken_connection::get_type(void) const {
	return EV_BROKEN_CONNECTION;
}

t_url t_event_broken_connection::get_user_uri(void) const {
	return user_uri_;
}

///////////////////////////////////////////////////////////
// class t_event_tcp_ping
///////////////////////////////////////////////////////////

t_event_tcp_ping::t_event_tcp_ping(const t_url &url, unsigned int dst_addr, unsigned short dst_port) :
	t_event(),
	user_uri_(url),
	dst_addr_(dst_addr),
	dst_port_(dst_port)
{}

t_event_type t_event_tcp_ping::get_type(void) const {
	return EV_TCP_PING;
}

t_url t_event_tcp_ping::get_user_uri(void) const {
	return user_uri_;
}

unsigned int t_event_tcp_ping::get_dst_addr(void) const {
	return dst_addr_;
}

unsigned short t_event_tcp_ping::get_dst_port(void) const {
	return dst_port_;
}

///////////////////////////////////////////////////////////
// class t_event_queue
///////////////////////////////////////////////////////////

t_event_queue::t_event_queue() : sema_evq(0), sema_caught_interrupt(0) {}

t_event_queue::~t_event_queue() {
	log_file->write_header("t_event_queue::~t_event_queue", LOG_NORMAL, LOG_INFO);
	log_file->write_raw("Clean up event queue.\n");

	while (!ev_queue.empty())
	{
		t_event *e = ev_queue.front();
		ev_queue.pop();
		log_file->write_raw("\nDeleting unprocessed event: \n");
		log_file->write_raw("Type: ");
		log_file->write_raw(event_type2str(e->get_type()));
		log_file->write_raw(", Pointer: ");
		log_file->write_raw(ptr2str(e));
		log_file->write_endl();
		MEMMAN_DELETE(e);
		delete e;
	}

	log_file->write_footer();
}

void t_event_queue::push(t_event *e) {
	mutex_evq.lock();
	ev_queue.push(e);
	mutex_evq.unlock();
	sema_evq.up();
}

void t_event_queue::push_quit(void) {
	t_event_quit *event = new t_event_quit();
	MEMMAN_NEW(event);
	push(event);
}

void t_event_queue::push_network(t_sip_message *m, const t_ip_port &ip_port) {
	t_event_network	*event = new t_event_network(m);
	MEMMAN_NEW(event);
	event->dst_addr = ip_port.ipaddr;
	event->dst_port = ip_port.port;
	event->transport = ip_port.transport;
	push(event);
}

void t_event_queue::push_user(t_user *user_config, t_sip_message *m, unsigned short tuid,
		unsigned short tid)
{
	t_event_user *event = new t_event_user(user_config, m, tuid, tid);
	MEMMAN_NEW(event);
	push(event);
}

void t_event_queue::push_user(t_sip_message *m, unsigned short tuid,
		unsigned short tid)
{
	push_user(NULL, m, tuid, tid);
}

void t_event_queue::push_user_cancel(t_user *user_config, t_sip_message *m, unsigned short tuid,
		unsigned short tid, unsigned short target_tid)
{
	t_event_user *event = new t_event_user(user_config, m, tuid, tid, target_tid);
	MEMMAN_NEW(event);
	push(event);
}

void t_event_queue::push_user_cancel(t_sip_message *m, unsigned short tuid,
		unsigned short tid, unsigned short target_tid)
{
	push_user_cancel(NULL, m, tuid, tid, target_tid);
}

void t_event_queue::push_timeout(t_timer *t) {
	t_event_timeout *event = new t_event_timeout(t);
	MEMMAN_NEW(event);
	push(event);
}

void t_event_queue::push_failure(t_failure f, unsigned short tid) {
	t_event_failure *event = new t_event_failure(f, tid);
	MEMMAN_NEW(event);
	push(event);
}

void t_event_queue::push_failure(t_failure f, const string &branch, const t_method &cseq_method) {
	t_event_failure *event = new t_event_failure(f, branch, cseq_method);
	MEMMAN_NEW(event);
	push(event);
}

void t_event_queue::push_start_timer(t_timer *t) {
	t_event_start_timer *event = new t_event_start_timer(t);
	MEMMAN_NEW(event);
	push(event);
}

void t_event_queue::push_stop_timer(unsigned short timer_id) {
	t_event_stop_timer *event = new t_event_stop_timer(timer_id);
	MEMMAN_NEW(event);
	push(event);
}

void t_event_queue::push_abort_trans(unsigned short tid) {
	t_event_abort_trans *event = new t_event_abort_trans(tid);
	MEMMAN_NEW(event);
	push(event);
}

void t_event_queue::push_stun_request(t_user *user_config, 
		StunMessage *m, t_stun_event_type ev_type,
		unsigned short tuid, unsigned short tid,
		unsigned long ipaddr, unsigned short port, unsigned short src_port)
{
	t_event_stun_request *event = new t_event_stun_request(user_config, 
		m, ev_type, tuid, tid);
	MEMMAN_NEW(event);
	event->dst_addr = ipaddr;
	event->dst_port = port;
	event->src_port = src_port;

	push(event);
}

void t_event_queue::push_stun_response(StunMessage *m,
		unsigned short tuid, unsigned short tid)
{
	t_event_stun_response *event = new t_event_stun_response(m, tuid, tid);
	MEMMAN_NEW(event);
	push(event);
}

void t_event_queue::push_nat_keepalive(unsigned long ipaddr, unsigned short port) {
	t_event_nat_keepalive *event = new t_event_nat_keepalive();
	MEMMAN_NEW(event);
	event->dst_addr = ipaddr;
	event->dst_port = port;

	push(event);
}

void t_event_queue::push_icmp(const t_icmp_msg &m) {
	t_event_icmp *event = new t_event_icmp(m);
	MEMMAN_NEW(event);
	push(event);
}

void t_event_queue::push_refer_permission_response(bool permission) {
	t_event_async_response *event = new t_event_async_response(
			t_event_async_response::RESP_REFER_PERMISSION);
	MEMMAN_NEW(event);
	event->set_bool_response(permission);
	push(event);
}

void t_event_queue::push_broken_connection(const t_url &user_uri) {
	t_event_broken_connection *event = new t_event_broken_connection(user_uri);
	MEMMAN_NEW(event);
	push(event);
}

void t_event_queue::push_tcp_ping(const t_url &user_uri, unsigned int dst_addr, unsigned short dst_port) 
{
	t_event_tcp_ping *event = new t_event_tcp_ping(user_uri, dst_addr, dst_port);
	MEMMAN_NEW(event);
	push(event);
}

t_event *t_event_queue::pop(void) {
	t_event *e;
	bool interrupt;

	do {
		interrupt = false;
		sema_evq.down();
		mutex_evq.lock();

		if (sema_caught_interrupt.try_down()) {
			// This pop is non-interruptable, so ignore the interrupt
			interrupt = true;
		} else {
			e = ev_queue.front();
			ev_queue.pop();
		}

		mutex_evq.unlock();
	} while (interrupt);

	return e;
}

t_event *t_event_queue::pop(bool &interrupted) {
	t_event *e;

	sema_evq.down();
	mutex_evq.lock();

	if (sema_caught_interrupt.try_down()) {
		interrupted = true;
		e = NULL;
	} else {
		interrupted = false;
		e = ev_queue.front();
		ev_queue.pop();
	}

	mutex_evq.unlock();

	return e;
}

void t_event_queue::interrupt(void) {
	sema_caught_interrupt.up();
	sema_evq.up();
}
