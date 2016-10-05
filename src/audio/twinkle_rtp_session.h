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
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef TWINKLE_RTP_SESSION_H
#define TWINKLE_RTP_SESSION_H

#include "twinkle_config.h"

#include <string>

#ifdef HAVE_ZRTP
#include <libzrtpcpp/zrtpccrtp.h>
#else
#include <ccrtp/rtp.h>
#endif

using namespace std;
using namespace ost;

#ifdef HAVE_ZRTP
class t_twinkle_rtp_session : public SymmetricZRTPSession {
private:
	bool zrtp_initialized;
	void init_zrtp(void);
public:
	bool is_zrtp_initialized(void) const;
#else
class t_twinkle_rtp_session : public SymmetricRTPSession {
#endif
public:
	virtual ~t_twinkle_rtp_session();
	 
	t_twinkle_rtp_session(const InetHostAddress &host);
	t_twinkle_rtp_session(const InetHostAddress &host, unsigned short port);
	uint32 getLastTimestamp(const SyncSource *src=NULL) const;
};

#endif
