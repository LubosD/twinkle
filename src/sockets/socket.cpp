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

#include <cstdio>
#include <cerrno>
#include <cstring>
#include <sys/un.h>
#include "twinkle_config.h"
#include "socket.h"
#include "audits/memman.h"
#include "user.h"

#ifdef HAVE_GNUTLS
#	include <gnutls/x509.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_LINUX_TYPES_H
#include <linux/types.h>
#endif

#ifdef HAVE_LINUX_ERRQUEUE_H
#include <linux/errqueue.h>
#endif

/////////////////
// t_icmp_msg
/////////////////

t_icmp_msg::t_icmp_msg(short _type, short _code, unsigned long _icmp_src_ipaddr,
	unsigned long _ipaddr, unsigned short _port) :
		type(_type), code(_code), icmp_src_ipaddr(_icmp_src_ipaddr),
		ipaddr(_ipaddr), port(_port)
{}

/////////////////
// t_socket
/////////////////

t_socket::~t_socket() {
	close(sd);
}

t_socket::t_socket() : sd(0)
{}

t_socket::t_socket(int _sd) : sd(_sd)
{}

int t_socket::get_descriptor(void) const {
	return sd;
}

int t_socket::getsockopt(int level, int optname, void *optval, socklen_t *optlen) {
	return ::getsockopt(sd, level, optname, optval, optlen);
}

int t_socket::setsockopt(int level, int optname, const void *optval, socklen_t optlen) {
	return ::setsockopt(sd, level, optname, optval, optlen);
}

/////////////////
// t_socket_udp
/////////////////


t_socket_udp::t_socket_udp() {
	struct sockaddr_in addr;
	int ret;

	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sd < 0) throw errno;

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(0);
	ret = bind(sd, (struct sockaddr *)&addr, sizeof(addr));
	if (ret < 0) throw errno;
}

t_socket_udp::t_socket_udp(unsigned short port) {
	struct sockaddr_in addr;
	int ret;

	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sd < 0) throw errno;

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);
	ret = bind(sd, (struct sockaddr *)&addr, sizeof(addr));
	if (ret < 0) throw errno;
}

int t_socket_udp::connect(unsigned long dest_addr, unsigned short dest_port) {
	struct sockaddr_in addr;
	int ret;

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(dest_addr);
	addr.sin_port = htons(dest_port);
	ret = ::connect(sd, (struct sockaddr *)&addr, sizeof(addr));
	if (ret < 0) throw errno;
	
	return ret;
}

int t_socket_udp::sendto(unsigned long dest_addr, unsigned short dest_port,
	           const char *data, int data_size) {
	struct sockaddr_in addr;
	int ret;

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(dest_addr);
	addr.sin_port = htons(dest_port);
	ret = ::sendto(sd, data, data_size, 0,
		     (struct sockaddr *)&addr, sizeof(addr));
	if (ret < 0) throw errno;

	return ret;
}

ssize_t t_socket_udp::send(const void *data, int data_size) {
	int ret = ::send(sd, data, data_size, 0);
	if (ret < 0) throw errno;

	return ret;
}

int t_socket_udp::recvfrom(unsigned long &src_addr, unsigned short &src_port,
		     char *buf, int buf_size) {
	struct sockaddr_in addr;
	int ret, len_addr;

	len_addr = sizeof(addr);
	memset(buf, 0, buf_size);
	ret = ::recvfrom(sd, buf, buf_size - 1, 0,
		       (struct sockaddr *)&addr, (socklen_t *)&len_addr);
	if (ret < 0) throw errno;

	src_addr = ntohl(addr.sin_addr.s_addr);
	src_port = ntohs(addr.sin_port);

	return ret;
}

ssize_t t_socket_udp::recv(void *buf, int buf_size) {
	int ret;

	memset(buf, 0, buf_size);
	ret = ::recv(sd, buf, buf_size - 1, 0);
	if (ret < 0) throw errno;

	return ret;
}

bool t_socket_udp::select_read(unsigned long timeout) {
	fd_set fds;
	struct timeval t;
	
	FD_ZERO(&fds);
	FD_SET(sd, &fds);

	t.tv_sec = timeout / 1000;
	t.tv_usec = (timeout % 1000) * 1000;
	
	int ret = select(sd + 1, &fds, NULL, NULL, &t);
	
	if (ret < 0) throw errno;
	if (ret == 0) return false;
	return true;
}

bool t_socket_udp::enable_icmp(void) {
#ifdef HAVE_LINUX_ERRQUEUE_H
	int enable = 1;
	int ret = setsockopt(SOL_IP, IP_RECVERR, &enable, sizeof(int));
	if (ret < 0) return false;
	return true;
#else
	return false;
#endif
}

bool t_socket_udp::get_icmp(t_icmp_msg &icmp) {
#ifdef HAVE_LINUX_ERRQUEUE_H
	int ret;
	char buf[256];
	
	// The destination address of the packet causing the ICMP
	struct sockaddr dest_addr;
	
	struct msghdr msgh;
	struct cmsghdr *cmsg;
	
	// Initialize message header to receive the ancillary data for
	// an ICMP message.
	memset(&msgh, 0, sizeof(struct msghdr));
	msgh.msg_control = buf;
	msgh.msg_controllen = 256;
	msgh.msg_name = &dest_addr;
	msgh.msg_namelen = sizeof(struct sockaddr);
	
	// Get error from the socket error queue
	ret = recvmsg(sd, &msgh, MSG_ERRQUEUE);
	if (ret < 0) return false;
	
	// Find ICMP message in returned controll messages
	for (cmsg = CMSG_FIRSTHDR(&msgh); cmsg != NULL; 
	     cmsg = CMSG_NXTHDR(&msgh, cmsg))
	{
		if (cmsg->cmsg_level == SOL_IP &&
		    cmsg->cmsg_type == IP_RECVERR)
		{
			// ICMP message found
			sock_extended_err *err = (sock_extended_err *)CMSG_DATA(cmsg);
			icmp.type = err->ee_type;
			icmp.code = err->ee_code;
			
			// Get IP address of host that has sent the ICMP error
			sockaddr *sa_src_icmp = SO_EE_OFFENDER(err);
			if (sa_src_icmp->sa_family == AF_INET) {
				sockaddr_in *addr = (sockaddr_in *)sa_src_icmp;
				icmp.icmp_src_ipaddr = ntohl(addr->sin_addr.s_addr);
			} else {
				// Non supported address type
				icmp.icmp_src_ipaddr = 0;
			}
			
			// Get destinnation address/port of packet causing the error.
			if (dest_addr.sa_family == AF_INET) {
				sockaddr_in *addr = (sockaddr_in *)&dest_addr;
				icmp.ipaddr = ntohl(addr->sin_addr.s_addr);
				icmp.port = ntohs(addr->sin_port);
				return true;
			} else {
				// Non supported address type
				continue;
			}
		}
	}
#endif
	return false;
}

string h_ip2str(unsigned long ipaddr) {
	char buf[16];
	unsigned long x = htonl(ipaddr);
	unsigned char *ipbuf = (unsigned char *)&x;

	snprintf(buf, 16, "%u.%u.%u.%u", ipbuf[0], ipbuf[1], ipbuf[2],
		ipbuf[3]);

	return string(buf);
}

/////////////////
// t_socket_tcp
/////////////////

t_socket_tcp::t_socket_tcp() {
	sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd < 0) throw errno;
}

t_socket_tcp::t_socket_tcp(unsigned short port) {
	struct sockaddr_in addr;
	int ret;

	sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd < 0) throw errno;
	
	int enable = 1;
	
	// Allow server to connect to the socket immediately (disable TIME_WAIT)
	(void)setsockopt(SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
	
	enable = 1;
	
	// Disable Nagle algorithm
	(void)setsockopt(IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(enable));

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);
	ret = bind(sd, (struct sockaddr *)&addr, sizeof(addr));
	if (ret < 0) throw errno;
}

t_socket_tcp::t_socket_tcp(int _sd) : t_socket(_sd)
{}

void t_socket_tcp::listen(int backlog) {
	int ret = ::listen(sd, backlog);
	if (ret < 0) throw errno;
}

t_socket_tcp *t_socket_tcp::accept(unsigned long &src_addr, unsigned short &src_port) {
	struct sockaddr_in addr;
	socklen_t socklen = sizeof(addr);
	int ret = ::accept(sd, (struct sockaddr *)&addr, &socklen);
	if (ret < 0) throw errno;
	
	src_addr = ntohl(addr.sin_addr.s_addr);
	src_port = ntohs(addr.sin_port);
	
	t_socket_tcp *sock = new t_socket_tcp(ret);
	MEMMAN_NEW(sock);
	return sock;
}

void t_socket_tcp::connect(unsigned long dest_addr, unsigned short dest_port) {
	struct sockaddr_in addr;
	int ret;

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(dest_addr);
	addr.sin_port = htons(dest_port);
	ret = ::connect(sd, (struct sockaddr *)&addr, sizeof(addr));
	if (ret < 0) throw errno;
}

ssize_t t_socket_tcp::send(const void *data, int data_size) {
	ssize_t ret = ::send(sd, data, data_size, 0);
	if (ret < 0) throw errno;

	return ret;
}

ssize_t t_socket_tcp::recv(void *buf, int buf_size) {
	ssize_t ret;

	ret = ::recv(sd, buf, buf_size, 0);
	if (ret < 0) throw errno;

	return ret;
}

void t_socket_tcp::get_remote_address(unsigned long &remote_addr, unsigned short &remote_port) {
	struct sockaddr_in addr;
	socklen_t namelen = sizeof(addr);
	
	int ret = getpeername(sd, (struct sockaddr *)&addr, &namelen);
	if (ret < 0) throw errno;
	if (addr.sin_family != AF_INET) throw EBADF;
	
	remote_addr = ntohl(addr.sin_addr.s_addr);
	remote_port = ntohs(addr.sin_port);
};

#ifdef HAVE_GNUTLS
/////////////////
// t_socket_tcp_tls
/////////////////

t_socket_tcp_tls::t_socket_tcp_tls(t_user* user, std::string hostname)
    : m_user(user), m_hostname(hostname), m_custom_ca(false)
{
    std::string pem;

	gnutls_certificate_allocate_credentials(&m_xcred);
	gnutls_init(&m_session, GNUTLS_CLIENT);

	gnutls_certificate_set_verify_function(m_xcred, vertify_certificate_callback);

    pem = user->get_tls_ca_cert();

    if (pem.empty())
    {
        gnutls_certificate_set_x509_system_trust(m_xcred);
    }
    else
    {
        gnutls_x509_crt_t cert;
        gnutls_datum_t data;
        int error;

        gnutls_x509_crt_init(&cert);

        data.data = (unsigned char*) pem.c_str();
        data.size = pem.length();

        error = gnutls_x509_crt_import(cert, &data, GNUTLS_X509_FMT_PEM);

        if (error == GNUTLS_E_SUCCESS)
        {
            gnutls_certificate_set_x509_trust(m_xcred, &cert, 1);
            m_custom_ca = true;
        }

        gnutls_x509_crt_deinit(cert);
    }

	gnutls_session_set_ptr(m_session, this);
}

t_socket_tcp_tls::~t_socket_tcp_tls()
{
	gnutls_deinit(m_session);
	gnutls_certificate_free_credentials(m_xcred);
}

void t_socket_tcp_tls::connect(unsigned long dest_addr, unsigned short dest_port)
{
	int ret;

	gnutls_set_default_priority(m_session);
	gnutls_credentials_set(m_session, GNUTLS_CRD_CERTIFICATE, m_xcred);

	t_socket_tcp::connect(dest_addr, dest_port);

	gnutls_transport_set_int(m_session, get_descriptor());
	gnutls_handshake_set_timeout(m_session, GNUTLS_DEFAULT_HANDSHAKE_TIMEOUT);

	do
	{
		ret = gnutls_handshake(m_session);
	}
	while (ret < 0 && gnutls_error_is_fatal(ret) == 0);

	if (ret < 0)
		throw ret;
}

/** Send data */
ssize_t t_socket_tcp_tls::send(const void *data, int data_size)
{
	return gnutls_record_send(m_session, data, data_size);
}


/** Receive data */
ssize_t t_socket_tcp_tls::recv(void *buf, int buf_size)
{
	return gnutls_record_recv(m_session, buf, buf_size);
}

int t_socket_tcp_tls::vertify_certificate_callback(gnutls_session_t session)
{
	int ret;
	gnutls_certificate_type_t type;
	gnutls_datum_t out;
	unsigned int status;
	gnutls_typed_vdata_st data[2];
	t_socket_tcp_tls* This;

	This = static_cast<t_socket_tcp_tls*>(gnutls_session_get_ptr(session));

	std::memset(data, 0, sizeof(data));

    if (!This->m_custom_ca)
    {
        data[1].type = GNUTLS_DT_DNS_HOSTNAME;
        data[1].data = (unsigned char*) This->m_hostname.c_str();
    }

    data[0].type = GNUTLS_DT_KEY_PURPOSE_OID;
    data[0].data = (unsigned char*) GNUTLS_KP_ANY;

    ret = gnutls_certificate_verify_peers(session, data, This->m_custom_ca ? 1 : 2, &status);

	type = gnutls_certificate_type_get(session);
	gnutls_certificate_verification_status_print(status, type, &out, 0);

	printf("(for hostname %s): %s\n", This->m_hostname.c_str(), out.data);
	gnutls_free(out.data);

    if (This->m_custom_ca)
    {
        // If the user specified a custom CA, then we don't prompt the user
        // to accept the certificate. We rely solely on the verification result.
        return ret;
    }
    else if (ret != GNUTLS_E_SUCCESS)
    {
        // TODO: Ask user to accept/remember/reject certificate.
        return 0; // accept all for now (insecure)
    }
    else
        return 0;
}
#endif

/////////////////
// t_socket_local
/////////////////

t_socket_local::t_socket_local() {
	sd = socket(PF_LOCAL, SOCK_STREAM, 0);
	if (sd < 0) throw errno;
}

t_socket_local::t_socket_local(int _sd) {
	sd = _sd;
}

void t_socket_local::bind(const string &name) {
	int ret;
	struct sockaddr_un sockname;
	
	// A name for a local socket can be at most 108 characters
	// including NULL at end of string.
	if (name.size() > 107) {
		throw ENAMETOOLONG;
	}
	
	sockname.sun_family = AF_LOCAL;
	strcpy(sockname.sun_path, name.c_str());
	ret = ::bind(sd, (struct sockaddr *)&sockname, SUN_LEN(&sockname));
	if (ret < 0) throw errno;
}

void t_socket_local::listen(int backlog) {
	int ret;
	ret = ::listen(sd, backlog);
	if (ret < 0) throw errno;
}

int t_socket_local::accept(void) {
	int ret;
	ret = ::accept(sd, NULL, 0);
	if (ret < 0) throw errno;
	return ret;
}

void t_socket_local::connect(const string &name) {
	int ret;
	struct sockaddr_un sockname;

	// A name for a local socket can be at most 108 characters
	// including NULL at end of string.
	if (name.size() > 107) {
		throw ENAMETOOLONG;
	}
	
	sockname.sun_family = AF_LOCAL;
	strcpy(sockname.sun_path, name.c_str());
	ret = ::connect(sd, (struct sockaddr *)&sockname, SUN_LEN(&sockname));
	if (ret < 0) throw errno;
}

int t_socket_local::read(void *buf, int count) {
	int ret;
	
	ret = ::read(sd, buf, count);
	if (ret < 0) throw errno;
	return ret;
}

ssize_t t_socket_local::recv(void *buf, int buf_size) {
	return read(buf, buf_size);
}

int t_socket_local::write(const void *buf, int count) {
	int ret;
	
	ret = ::write(sd, buf, count);
	if (ret < 0) throw errno;
	return ret;	
}

ssize_t t_socket_local::send(const void *buf, int count) {
	return write(buf, count);
}
