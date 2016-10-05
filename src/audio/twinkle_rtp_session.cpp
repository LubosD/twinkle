
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

#include <unistd.h>
#include "twinkle_rtp_session.h"
#include "log.h"
#include "sys_settings.h"

#define TWINKLE_ZID_FILE 	".twinkle.zid"

t_twinkle_rtp_session::~t_twinkle_rtp_session() {}

#ifdef HAVE_ZRTP
void t_twinkle_rtp_session::init_zrtp(void) {
	string zid_filename = sys_config->get_dir_user();
	zid_filename += '/';
	zid_filename += TWINKLE_ZID_FILE;
	
	if (initialize(zid_filename.c_str()) >=0) {
		zrtp_initialized = true;
		return;
	}
	
	// ZID file initialization failed. Maybe the ZID file
	// is corrupt. Try to remove it
	if (unlink(zid_filename.c_str()) < 0) {
		string msg = "Failed to remove ";
		msg += zid_filename;
		log_file->write_report(msg,
			"t_twinkle_rtp_session::init_zrtp",
			LOG_NORMAL, LOG_CRITICAL);
		return;
	}
	
	// Try to initialize once more
	if (initialize(zid_filename.c_str()) >= 0) {
		zrtp_initialized = true;
	} else {
		string msg = "Failed to initialize ZRTP - ";
		msg += zid_filename;
		log_file->write_report(msg,
			"t_twinkle_rtp_session::init_zrtp",
			LOG_NORMAL, LOG_CRITICAL);
	}
}

bool t_twinkle_rtp_session::is_zrtp_initialized(void) const {
	return zrtp_initialized;
}

t_twinkle_rtp_session::t_twinkle_rtp_session(const InetHostAddress &host) :
	SymmetricZRTPSession(host),
	zrtp_initialized(false) 
{
	init_zrtp();
}

t_twinkle_rtp_session::t_twinkle_rtp_session(const InetHostAddress &host, unsigned short port) : 
	SymmetricZRTPSession(host, port) ,
	zrtp_initialized(false)
{
	init_zrtp();
}
#else
t_twinkle_rtp_session::t_twinkle_rtp_session(const InetHostAddress &host) :
	SymmetricRTPSession(host) 
{
}

t_twinkle_rtp_session::t_twinkle_rtp_session(const InetHostAddress &host, unsigned short port) : 
	SymmetricRTPSession(host, port) 
{
}
#endif

uint32 t_twinkle_rtp_session::getLastTimestamp(const SyncSource *src) const {
	if ( src && !isMine(*src) ) return 0L;
	
	recvLock.readLock();

	uint32 ts = 0;	
	if (src != NULL) {
		SyncSourceLink* srcm = getLink(*src);
		IncomingRTPPktLink* l = srcm->getFirst();
		
		while (l) {
			ts = l->getTimestamp();
			l = l->getSrcNext();
		}
	} else {
		IncomingRTPPktLink* l = recvFirst;
		
		while (l) {
			ts = l->getTimestamp();
			l = l->getNext();
		}
	}
	
	recvLock.unlock();
	return ts;
}
