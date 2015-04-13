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
#include "twinkle_zrtp_ui.h"

#ifdef HAVE_ZRTP

#include "phone.h"
#include "line.h"
#include "log.h"
#include "util.h"

using namespace std;
using namespace GnuZrtpCodes;

extern t_phone *phone;

// Initialize static data
map<int32, std::string>TwinkleZrtpUI::infoMap;
map<int32, std::string>TwinkleZrtpUI::warningMap;
map<int32, std::string>TwinkleZrtpUI::severeMap;
map<int32, std::string>TwinkleZrtpUI::zrtpMap;

bool TwinkleZrtpUI::mapsDone = false;
std::string TwinkleZrtpUI::unknownCode = "Unknown error code";

TwinkleZrtpUI::TwinkleZrtpUI(t_audio_session* session) :
                             audioSession(session) 
{
        if (mapsDone) {
            return;
        }
        
        // Initialize error mapping
        infoMap.insert(pair<int32, std::string>(InfoHelloReceived,
						 string("Hello received, preparing a Commit")));
        infoMap.insert(pair<int32, std::string>(InfoCommitDHGenerated, 
						 string("Commit: Generated a public DH key")));
        infoMap.insert(pair<int32, std::string>(InfoRespCommitReceived, 
						 string("Responder: Commit received, preparing DHPart1")));
        infoMap.insert(pair<int32, std::string>(InfoDH1DHGenerated, 
						 string("DH1Part: Generated a public DH key")));
        infoMap.insert(pair<int32, std::string>(InfoInitDH1Received,
						 string("Initiator: DHPart1 received, preparing DHPart2")));
        infoMap.insert(pair<int32, std::string>(InfoRespDH2Received,
						 string("Responder: DHPart2 received, preparing Confirm1")));
        infoMap.insert(pair<int32, std::string>(InfoInitConf1Received,
						 string("Initiator: Confirm1 received, preparing Confirm2")));
        infoMap.insert(pair<int32, std::string>(InfoRespConf2Received,
						 string("Responder: Confirm2 received, preparing Conf2Ack")));
        infoMap.insert(pair<int32, std::string>(InfoRSMatchFound,
						 string("At least one retained secrets matches - security OK")));
        infoMap.insert(pair<int32, std::string>(InfoSecureStateOn,
						 string("Entered secure state")));
        infoMap.insert(pair<int32, std::string>(InfoSecureStateOff,
						 string("No more security for this session")));

        warningMap.insert(pair<int32, std::string>(WarningDHAESmismatch,
                          string("Commit contains an AES256 cipher but does not offer a Diffie-Helman 4096")));
        warningMap.insert(pair<int32, std::string>(WarningGoClearReceived,
						    string("Received a GoClear message")));
        warningMap.insert(pair<int32, std::string>(WarningDHShort,
                          string("Hello offers an AES256 cipher but does not offer a Diffie-Helman 4096")));
        warningMap.insert(pair<int32, std::string>(WarningNoRSMatch,
						    string("No retained shared secrets available - must verify SAS")));
        warningMap.insert(pair<int32, std::string>(WarningCRCmismatch,
						    string("Internal ZRTP packet checksum mismatch - packet dropped")));
        warningMap.insert(pair<int32, std::string>(WarningSRTPauthError,
						    string("Dropping packet because SRTP authentication failed!")));
        warningMap.insert(pair<int32, std::string>(WarningSRTPreplayError,
						    string("Dropping packet because SRTP replay check failed!")));
	warningMap.insert(pair<int32, std::string>(WarningNoExpectedRSMatch,
						    string("Valid retained shared secrets availabe but no matches found - must verify SAS")));

        severeMap.insert(pair<int32, std::string>(SevereHelloHMACFailed,
						   string("Hash HMAC check of Hello failed!")));
        severeMap.insert(pair<int32, std::string>(SevereCommitHMACFailed,
						   string("Hash HMAC check of Commit failed!")));
        severeMap.insert(pair<int32, std::string>(SevereDH1HMACFailed,
						   string("Hash HMAC check of DHPart1 failed!")));
        severeMap.insert(pair<int32, std::string>(SevereDH2HMACFailed,
						   string("Hash HMAC check of DHPart2 failed!")));
        severeMap.insert(pair<int32, std::string>(SevereCannotSend,
						   string("Cannot send data - connection or peer down?")));
        severeMap.insert(pair<int32, std::string>(SevereProtocolError,
						   string("Internal protocol error occured!")));
        severeMap.insert(pair<int32, std::string>(SevereNoTimer, 
						   string("Cannot start a timer - internal resources exhausted?")));
        severeMap.insert(pair<int32, std::string>(SevereTooMuchRetries,
					 string("Too much retries during ZRTP negotiation - connection or peer down?")));

        zrtpMap.insert(pair<int32, std::string>(MalformedPacket,
						 string("Malformed packet (CRC OK, but wrong structure)")));
        zrtpMap.insert(pair<int32, std::string>(CriticalSWError, 
						 string("Critical software error")));
        zrtpMap.insert(pair<int32, std::string>(UnsuppZRTPVersion, 
						 string("Unsupported ZRTP version")));
        zrtpMap.insert(pair<int32, std::string>(HelloCompMismatch, 
						 string("Hello components mismatch")));
        zrtpMap.insert(pair<int32, std::string>(UnsuppHashType, 
						 string("Hash type not supported")));
        zrtpMap.insert(pair<int32, std::string>(UnsuppCiphertype, 
						 string("Cipher type not supported")));
        zrtpMap.insert(pair<int32, std::string>(UnsuppPKExchange, 
						 string("Public key exchange not supported")));
        zrtpMap.insert(pair<int32, std::string>(UnsuppSRTPAuthTag, 
						 string("SRTP auth. tag not supported")));
        zrtpMap.insert(pair<int32, std::string>(UnsuppSASScheme, 
						 string("SAS scheme not supported")));
        zrtpMap.insert(pair<int32, std::string>(NoSharedSecret, 
						 string("No shared secret available, DH mode required")));
        zrtpMap.insert(pair<int32, std::string>(DHErrorWrongPV, 
						 string("DH Error: bad pvi or pvr ( == 1, 0, or p-1)")));
        zrtpMap.insert(pair<int32, std::string>(DHErrorWrongHVI, 
						 string("DH Error: hvi != hashed data")));
        zrtpMap.insert(pair<int32, std::string>(SASuntrustedMiTM, 
						 string("Received relayed SAS from untrusted MiTM")));
        zrtpMap.insert(pair<int32, std::string>(ConfirmHMACWrong, 
						 string("Auth. Error: Bad Confirm pkt HMAC")));
        zrtpMap.insert(pair<int32, std::string>(NonceReused, 
						 string("Nonce reuse")));
        zrtpMap.insert(pair<int32, std::string>(EqualZIDHello, 
						 string("Equal ZIDs in Hello")));
        zrtpMap.insert(pair<int32, std::string>(GoCleatNotAllowed, 
						string("GoClear packet received, but not allowed")));

        mapsDone = true;

}

void TwinkleZrtpUI::secureOn(std::string cipher) {
	audioSession->set_is_encrypted(true);
	audioSession->set_srtp_cipher_mode(cipher);
	
	t_line *line = audioSession->get_line();
	int lineno = line->get_line_number();

	log_file->write_header("TwinkleZrtpUI::secureOn");
	log_file->write_raw("Line ");
	log_file->write_raw(lineno + 1);
	log_file->write_raw(": audio encryption enabled: ");
	log_file->write_raw(cipher);
	log_file->write_endl();
	log_file->write_footer();
	
	ui->cb_async_line_encrypted(lineno, true);
	ui->cb_async_line_state_changed();
}

void TwinkleZrtpUI::secureOff() {
	audioSession->set_is_encrypted(false);
	
	t_line *line = audioSession->get_line();
	int lineno = line->get_line_number();

	log_file->write_header("TwinkleZrtpUI::secureOff");
	log_file->write_raw("Line ");
	log_file->write_raw(lineno + 1);
	log_file->write_raw(": audio encryption disabled.\n");
	log_file->write_footer();
	
	ui->cb_async_line_encrypted(lineno, false);
	ui->cb_async_line_state_changed();
}

void TwinkleZrtpUI::showSAS(std::string sas, bool verified) {
	audioSession->set_zrtp_sas(sas);
	audioSession->set_zrtp_sas_confirmed(verified);
	
	t_line *line = audioSession->get_line();
	int lineno = line->get_line_number();

	log_file->write_header("TwinkleZrtpUI::showSAS");
	log_file->write_raw("Line ");
	log_file->write_raw(lineno + 1);
	log_file->write_raw(": SAS =");
	log_file->write_raw(sas);
	log_file->write_endl();
	log_file->write_footer();

	if (!verified) {
	  ui->cb_async_show_zrtp_sas(lineno, sas);
	}
	ui->cb_async_line_state_changed();
}

void TwinkleZrtpUI::confirmGoClear() {
	t_line *line = audioSession->get_line();
	int lineno = line->get_line_number();
	
	ui->cb_async_zrtp_confirm_go_clear(lineno);
}

void TwinkleZrtpUI::showMessage(MessageSeverity sev, int subCode) {
	t_line *line = audioSession->get_line();
	int lineno = line->get_line_number();
	
	string msg = "Line ";
	msg += int2str(lineno + 1);
	msg += ": ";
	msg += *mapCodesToString(sev, subCode);
	
	switch (sev) {
	case Info:
		log_file->write_report(msg, "TwinkleZrtpUI::showMessage", LOG_NORMAL,
			LOG_INFO);
		break;
	case Warning:
		log_file->write_report(msg, "TwinkleZrtpUI::showMessage", LOG_NORMAL,
			LOG_WARNING);
		break;
	default:
		log_file->write_report(msg, "TwinkleZrtpUI::showMessage", LOG_NORMAL,
			LOG_CRITICAL);
	}
}

void TwinkleZrtpUI::zrtpNegotiationFailed(MessageSeverity severity, int subCode) {
	t_line *line = audioSession->get_line();
	int lineno = line->get_line_number();
	
	string m = "Line ";
	m += int2str(lineno + 1);
	m += ": ZRTP negotiation failed.\n";
	m += *mapCodesToString(severity, subCode);
	
	switch (severity) {
	case Info:
		log_file->write_report(m, "TwinkleZrtpUI::zrtpNegotiationFailed", LOG_NORMAL,
			LOG_INFO);
		break;
	case Warning:
		log_file->write_report(m, "TwinkleZrtpUI::zrtpNegotiationFailed", LOG_NORMAL,
			LOG_WARNING);
		break;
	default:
		log_file->write_report(m, "TwinkleZrtpUI::zrtpNegotiationFailed", LOG_NORMAL,
			LOG_CRITICAL);
	}
}

void TwinkleZrtpUI::zrtpNotSuppOther() {
	t_line *line = audioSession->get_line();
	int lineno = line->get_line_number();
	
	string msg = "Line ";
	msg += int2str(lineno + 1);
	msg += ": remote party does not support ZRTP.";
	log_file->write_report(msg, "TwinkleZrtpUI::zrtpNotSuppOther");
}

const string *const TwinkleZrtpUI::mapCodesToString(MessageSeverity severity, int subCode) {
  	string *m = &unknownCode;

	switch (severity) {
	case Info:
		m = &infoMap[subCode];
		break;
	case Warning:
		m = &warningMap[subCode];
		break;
	case Severe:
		m = &severeMap[subCode];
		break;
	case ZrtpError:
		if (subCode < 0) {
			subCode *= -1;
		}
		m = &zrtpMap[subCode];
		break;
	default:
		break;
	}
	
	return m;
}

#endif

