/* 
   This software is copyrighted (c) 2002 Rick van Rein, the Netherlands.

   This software has been modified by Michel de Boer. 2005
*/ 
 
#include "dnssrv.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <errno.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <string.h>
#include <signal.h>


/* Common offsets into an SRV RR */
#define SRV_COST    (RRFIXEDSZ+0)
#define SRV_WEIGHT  (RRFIXEDSZ+2)
#define SRV_PORT    (RRFIXEDSZ+4)
#define SRV_SERVER  (RRFIXEDSZ+6)
#define SRV_FIXEDSZ (RRFIXEDSZ+6)


/* Data structures */
typedef struct {
	unsigned char buf [PACKETSZ];
	int len;
} iobuf;
typedef char name [MAXDNAME];
#define MAXNUM_SRV PACKETSZ

/* Local variable for SRV options */
static unsigned long int srv_flags = 0L;


/* Setup the SRV options when initialising -- invocation optional */
void insrv_init (unsigned long flags) {
#ifdef HAVE_RES_INIT
	srv_flags = flags;
	res_init ();
#endif
}


/* Test the given SRV options to see if all are set */
int srv_testflag (unsigned long flags) {
	return ((srv_flags & flags) == flags) ? 1 : 0;
}


/* Compare two SRV records by priority and by (scattered) weight */
int srvcmp (const void *left, const void *right) {
	int lcost = ntohs (((unsigned short **) left ) [0][5]);
	int rcost = ntohs (((unsigned short **) right) [0][5]);
	if (lcost == rcost) {
		lcost = -ntohs (((unsigned short **) left ) [0][6]);
		rcost = -ntohs (((unsigned short **) right) [0][6]);
	}
	if (lcost < rcost) {
		return -1;
	} else if (lcost > rcost) {
		return +1;
	} else {
		return  0;
	}
}


/* Setup a client socket for the named service over the given protocol under
 * the given domain name.
 */
int insrv_lookup (const char *service, const char *proto, const char *domain, 
	list<t_dns_result> &result) 
{
	// 1. convert service/proto to svcnm
	// 2. construct SRV query for _service._proto.domain

	iobuf names;
	name svcnm;
	int ctr;
	int rnd;
	HEADER *nameshdr;
	unsigned char *here, *srv[MAXNUM_SRV];
	int num_srv=0;
	// Storage for fallback SRV list, constructed when DNS gives no SRV
	//unsigned char fallbacksrv [2*(MAXCDNAME+SRV_FIXEDSZ+MAXCDNAME)];

	// srv_flags &= ~SRV_GOT_MASK;
	// srv_flags |=  SRV_GOT_SRV;

	strcpy (svcnm, "_");
	strcat (svcnm, service);
	strcat (svcnm, "._");
	strcat (svcnm, proto);

	// Note that SRV records are only defined for class IN
	if (domain) {
		names.len=res_querydomain (svcnm, domain,
				C_IN, T_SRV,
				names.buf, PACKETSZ);
	} else {
		names.len=res_query (svcnm,
				C_IN, T_SRV,
				names.buf, PACKETSZ);
	}
	if (names.len < 0) {
		return -ENOENT;
	}
	nameshdr=(HEADER *) names.buf;
	here=names.buf + HFIXEDSZ;
	rnd=nameshdr->id; 	// Heck, gimme one reason why not!

	if ((names.len < HFIXEDSZ) || nameshdr->tc) {
		return -EMSGSIZE;
	}
	switch (nameshdr->rcode) {
		case 1:
			return -EFAULT;
		case 2:
			return -EAGAIN;
		case 3:
			return -ENOENT;
		case 4:
			return -ENOSYS;
		case 5:
			return -EPERM;
		default:
			break;
	}
	if (ntohs (nameshdr->ancount) == 0) {
		return -ENOENT;
	}
	if (ntohs (nameshdr->ancount) > MAXNUM_SRV) {
		return -ERANGE;
	}
	for (ctr=ntohs (nameshdr->qdcount); ctr>0; ctr--) {
		int strlen=dn_skipname (here, names.buf+names.len);
		here += strlen + QFIXEDSZ;
	}
	for (ctr=ntohs (nameshdr->ancount); ctr>0; ctr--) {
		int strlen=dn_skipname (here, names.buf+names.len);
		here += strlen;
		srv [num_srv++] = here;
		here += SRV_FIXEDSZ;
		here += dn_skipname (here, names.buf+names.len);
	}
	
	// Overwrite weight with rnd-spread version to divide load over weights
	for (ctr=0; ctr<num_srv; ctr++) {
		*(unsigned short *) (srv [ctr]+SRV_WEIGHT)
			= htons(rnd % (1+ns_get16 (srv [ctr]+SRV_WEIGHT)));
	}
	qsort (srv, num_srv, sizeof (*srv), srvcmp);
	
	result.clear();
	for (ctr=0; ctr<num_srv; ctr++) {
		name srvname;
		if (ns_name_ntop (srv [ctr]+SRV_SERVER, srvname, MAXDNAME) < 0) {
			return -errno;
		}
		
		t_dns_result r;
		r.hostname = srvname;
		r.port = ns_get16(srv [ctr]+SRV_PORT);
		result.push_back(r);
	}
	
	return 0;
}
