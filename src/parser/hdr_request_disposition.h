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
 * Request-Disposition header (RFC 3841)
 */
 
#ifndef _H_HDR_REQUEST_DISPOSITION
#define _H_HDR_REQUEST_DISPOSITION

#include <string>
#include "header.h"

using namespace std;

#define REQDIS_PROXY		"proxy"
#define REQDIS_REDIRECT		"redirect"
#define REQDIS_CANCEL		"cancel"
#define REQDIS_NO_CANCEL	"no-cancel"
#define REQDIS_FORK		"fork"
#define REQDIS_NO_FORK		"no-fork"
#define REQDIS_RECURSE		"recurse"
#define REQDIS_NO_RECURSE	"no-recurse"
#define REQDIS_PARALLEL		"parallel"
#define REQDIS_SEQUENTIAL	"sequential"
#define REQDIS_QUEUE		"queue"
#define REQDIS_NO_QUEUE		"no-queue"

/** Request-Disposition header (RFC 3841) */
class t_hdr_request_disposition : public t_header {
public:
	enum t_proxy_directive {
		PROXY_NULL,
		PROXY,
		REDIRECT
	};
	
	enum t_cancel_directive {
		CANCEL_NULL,
		CANCEL,
		NO_CANCEL
	};
	
	enum t_fork_directive {
		FORK_NULL,
		FORK,
		NO_FORK
	};
	
	enum t_recurse_directive {
		RECURSE_NULL,
		RECURSE,
		NO_RECURSE
	};
	
	enum t_parallel_directive {
		PARALLEL_NULL,
		PARALLEL,
		SEQUENTIAL
	};
	
	enum t_queue_directive {
		QUEUE_NULL,
		QUEUE,
		NO_QUEUE
	};
	
	t_proxy_directive proxy_directive;
	t_cancel_directive cancel_directive;
	t_fork_directive fork_directive;
	t_recurse_directive recurse_directive;
	t_parallel_directive parallel_directive;
	t_queue_directive queue_directive;
	
	t_hdr_request_disposition();
	
	void set_proxy_directive(t_proxy_directive directive);
	void set_cancel_directive(t_cancel_directive directive);
	void set_fork_directive(t_fork_directive directive);
	void set_recurse_directive(t_recurse_directive directive);
	void set_parallel_directive(t_parallel_directive directive);
	void set_queue_directive(t_queue_directive directive);
	
	/**
	 * Set a directive using one of the tokens define in RFC 3841
	 * @param s [in] Directive token.
	 * @return True if directive set. False if directive is invalid or
	 *         conflicts with exisiting directives.
	 */
	bool set_directive(const string &s);
	
	string encode_value(void) const;
};

#endif
