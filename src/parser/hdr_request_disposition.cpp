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

#include "hdr_request_disposition.h"

#include <vector>
#include "util.h"

t_hdr_request_disposition::t_hdr_request_disposition() : 
	t_header("Request-Disposition", "d"),
	proxy_directive(PROXY_NULL),
	cancel_directive(CANCEL_NULL),
	fork_directive(FORK_NULL),
	recurse_directive(RECURSE_NULL),
	parallel_directive(PARALLEL_NULL),
	queue_directive(QUEUE_NULL)
{}

void t_hdr_request_disposition::set_proxy_directive(t_proxy_directive directive) {
	populated = true;
	proxy_directive = directive;
}

void t_hdr_request_disposition::set_cancel_directive(t_cancel_directive directive) {
	populated = true;
	cancel_directive = directive;
}

void t_hdr_request_disposition::set_fork_directive(t_fork_directive directive) {
	populated = true;
	fork_directive = directive;
}

void t_hdr_request_disposition::set_recurse_directive(t_recurse_directive directive) {
	populated = true;
	recurse_directive = directive;
}

void t_hdr_request_disposition::set_parallel_directive(t_parallel_directive directive) {
	populated = true;
	parallel_directive = directive;
}

void t_hdr_request_disposition::set_queue_directive(t_queue_directive directive) {
	populated = true;
	queue_directive = directive;
}

bool t_hdr_request_disposition::set_directive(const string &s) {
	if (s == REQDIS_PROXY) {
		if (proxy_directive == REDIRECT) return false;
		set_proxy_directive(PROXY);
		return true;
	}
	
	if (s == REQDIS_REDIRECT) {
		if (proxy_directive == PROXY) return false;
		set_proxy_directive(REDIRECT);
		return true;
	}
	
	if (s == REQDIS_CANCEL) {
		if (cancel_directive == NO_CANCEL) return false;
		set_cancel_directive(CANCEL);
		return true;
	}
	
	if (s == REQDIS_NO_CANCEL) {
		if (cancel_directive == CANCEL) return false;
		set_cancel_directive(NO_CANCEL);
		return true;
	}
	
	if (s == REQDIS_FORK) {
		if (fork_directive == NO_FORK) return false;
		set_fork_directive(FORK);
		return true;
	}
	
	if (s == REQDIS_NO_FORK) {
		if (fork_directive == FORK) return false;
		set_fork_directive(NO_FORK);
		return true;
	}
	
	if (s == REQDIS_RECURSE) {
		if (recurse_directive == NO_RECURSE) return false;
		set_recurse_directive(RECURSE);
		return true;
	}
	
	if (s == REQDIS_NO_RECURSE) {
		if (recurse_directive == RECURSE) return false;
		set_recurse_directive(NO_RECURSE);
		return true;
	}
	
	if (s == REQDIS_PARALLEL) {
		if (parallel_directive == SEQUENTIAL) return false;
		set_parallel_directive(PARALLEL);
		return true;
	}
	
	if (s == REQDIS_SEQUENTIAL) {
		if (parallel_directive == PARALLEL) return false;
		set_parallel_directive(SEQUENTIAL);
		return true;
	}
	
	if (s == REQDIS_QUEUE) {
		if (queue_directive == NO_QUEUE) return false;
		set_queue_directive(QUEUE);
		return true;
	}
	
	if (s == REQDIS_NO_QUEUE) {
		if (queue_directive == QUEUE) return false;
		set_queue_directive(NO_QUEUE);
		return true;
	}
	
	return false;
}

string t_hdr_request_disposition::encode_value(void) const {
	if (!populated) return "";
	
	vector<string> v;
	
	switch (proxy_directive) {
	case PROXY:
		v.push_back(REQDIS_PROXY);
		break;
	case REDIRECT:
		v.push_back(REQDIS_REDIRECT);
		break;
	default:
		break;
	}
	
	switch (cancel_directive) {
	case CANCEL:
		v.push_back(REQDIS_CANCEL);
		break;
	case NO_CANCEL:
		v.push_back(REQDIS_NO_CANCEL);
		break;
	default:
		break;
	}
	
	switch (fork_directive) {
	case FORK:
		v.push_back(REQDIS_FORK);
		break;
	case NO_FORK:
		v.push_back(REQDIS_NO_FORK);
		break;
	default:
		break;
	}
	
	switch (recurse_directive) {
	case RECURSE:
		v.push_back(REQDIS_RECURSE);
		break;
	case NO_RECURSE:
		v.push_back(REQDIS_NO_RECURSE);
		break;
	default:
		break;
	}
	
	switch (parallel_directive) {
	case PARALLEL:
		v.push_back(REQDIS_PARALLEL);
		break;
	case SEQUENTIAL:
		v.push_back(REQDIS_SEQUENTIAL);
		break;
	default:
		break;
	}
	
	switch (queue_directive) {
	case QUEUE:
		v.push_back(REQDIS_QUEUE);
		break;
	case NO_QUEUE:
		v.push_back(REQDIS_NO_QUEUE);
		break;
	default:
		break;
	}
	
	string s = join_strings(v, ",");
	return s;
}
