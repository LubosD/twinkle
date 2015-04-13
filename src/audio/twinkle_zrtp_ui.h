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

// Author: Werner Dittmann <Werner.Dittmann@t-online.de>, (C) 2006
//         Michel de Boer <michel@twinklephone.com>

/**
 * @file
 * User interface call back functions for libzrtpcpp.
 */

#ifndef __TWINKLEZRTPUI_H_
#define __TWINKLEZRTPUI_H_

#include "twinkle_config.h"

#ifdef HAVE_ZRTP

#include <iostream>
#include <libzrtpcpp/ZrtpQueue.h>
#include <libzrtpcpp/ZrtpUserCallback.h>
#include "audio_session.h"
#include "userintf.h"

using namespace GnuZrtpCodes;

/** User interface for libzrtpcpp. */
class TwinkleZrtpUI : public ZrtpUserCallback {

    public:
    	/**
    	 * Constructor.
    	 * @param session [in] The audio session that is encrypted by ZRTP.
    	 */
    	TwinkleZrtpUI(t_audio_session* session);
    	virtual ~TwinkleZrtpUI() {};
    
    	//@{
        /** @name ZRTP call back functions called from the ZRTP thread */
        virtual void secureOn(std::string cipher);
        virtual void secureOff();
        virtual void showSAS(std::string sas, bool verified); 
        virtual void confirmGoClear();
        virtual void showMessage(MessageSeverity sev, int subCode);
        virtual void zrtpNegotiationFailed(MessageSeverity severity, int subCode);
        virtual void zrtpNotSuppOther();
        //}

    private:
    	/** Audio session associated with this user interface. */
        t_audio_session* audioSession;
        
        //@{
        /** @name Message mappings for libzrtpcpp */
        static map<int32, std::string> infoMap;		/**< Info messages */
        static map<int32, std::string> warningMap;	/**< Warnings */
        static map<int32, std::string> severeMap;	/**< Severe errors */
        static map<int32, std::string> zrtpMap;		/**< ZRTP errors */
	static bool mapsDone;				/**< Flag to indicate that maps are initialized */
	static std::string unknownCode;			/**< Unknown error code */
	//@}
	
	/**
	 * Map a message code returned by libzrtpcpp to a message text.
	 * @param severity [in] The severity of the message.
	 * @param subCode [in] The message code.
	 * @return The message text.
	 */
	const string *const mapCodesToString(MessageSeverity severity, int subCode);

};

#endif // HAVE_ZRTP
#endif // __TWINKLEZRTPUI_H_

