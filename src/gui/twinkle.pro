TEMPLATE	= app
LANGUAGE	= C++

CONFIG	+= qt warn_on debug thread

LIBS	+= ../libtwinkle.a ../parser/libsipparser.a ../sdp/libsdpparser.a ../sockets/libsocket.a ../threads/libthread.a ../audio/libaudio.a ../audits/libaudits.a ../stun/libstun.a ../mwi/libmwi.a ../im/libim.a ../patterns/libpatterns.a ../presence/libpresence.a ../utils/libutils.a -lsndfile -lmagic -lncurses -lreadline -lX11

DEFINES	+= QT_NO_STL

INCLUDEPATH	+= ..

HEADERS	+= gui.h \
	dtmfform.h \
	deregisterform.h \
	logviewform.h \
	numberconversionform.h \
	getprofilenameform.h \
	historylistview.h \
	freedesksystray.h \
	twinklesystray.h \
	address_finder.h \
	qt_translator.h \
	core_strings.h \
	addresslistviewitem.h \
	yesnodialog.h \
	command_args.h \
	messageformview.h \
	buddylistview.h \
	textbrowsernoautolink.h \
	twinkleapplication.h

SOURCES	+= main.cpp \
	gui.cpp \
	dtmfform.cpp \
	deregisterform.cpp \
	logviewform.cpp \
	numberconversionform.cpp \
	getprofilenameform.cpp \
	historylistview.cpp \
	freedesksystray.cpp \
	twinklesystray.cpp \
	address_finder.cpp \
	addresslistviewitem.cpp \
	yesnodialog.cpp \
	messageformview.cpp \
	buddylistview.cpp \
	twinkleapplication.cpp

FORMS	= dtmfform.ui \
	deregisterform.ui \
	logviewform.ui \
	numberconversionform.ui \
	getprofilenameform.ui

#The following line was changed from FORMS to FORMS3 by qt3to4
FORMS3	= mphoneform.ui \
	inviteform.ui \
	redirectform.ui \
	termcapform.ui \
	selectnicform.ui \
	srvredirectform.ui \
	authenticationform.ui \
	userprofileform.ui \
	selectprofileform.ui \
	transferform.ui \
	syssettingsform.ui \
	wizardform.ui \
	getaddressform.ui \
	historyform.ui \
	selectuserform.ui \
	addresscardform.ui \
	messageform.ui \
	buddyform.ui \
	sendfileform.ui \
	diamondcardprofileform.ui

RESOURCES += icons.qrc

IMAGES	= images/filenew \
	images/filesave \
	images/print \
	images/undo \
	images/redo \
	images/editcut \
	images/editcopy \
	images/editpaste \
	images/searchfind \
	images/invite.png \
	images/answer.png \
	images/bye.png \
	images/reject.png \
	images/redirect.png \
	images/hold.png \
	images/dtmf.png \
	images/bye-disabled.png \
	images/redial.png \
	images/redial-disabled.png \
	images/invite-disabled.png \
	images/answer-disabled.png \
	images/reject-disabled.png \
	images/redirect-disabled.png \
	images/hold-disabled.png \
	images/dtmf-disabled.png \
	images/penguin.png \
	images/package_network.png \
	images/kmix.png \
	images/package_system.png \
	images/yast_babelfish.png \
	images/clock.png \
	images/yast_PhoneTTOffhook.png \
	images/penguin_big.png \
	images/password.png \
	images/kcmpci.png \
	images/penguin-small.png \
	images/conf.png \
	images/conf-disabled.png \
	images/mute.png \
	images/mute-disabled.png \
	images/twinkle16.png \
	images/twinkle48.png \
	images/twinkle32.png \
	images/transfer-disabled.png \
	images/transfer.png \
	images/log.png \
	images/dtmf-2.png \
	images/dtmf-3.png \
	images/dtmf-5.png \
	images/dtmf-6.png \
	images/dtmf-7.png \
	images/dtmf-8.png \
	images/dtmf-9.png \
	images/dtmf-4.png \
	images/dtmf-1.png \
	images/dtmf-0.png \
	images/dtmf-star.png \
	images/dtmf-pound.png \
	images/dtmf-a.png \
	images/dtmf-b.png \
	images/dtmf-c.png \
	images/dtmf-d.png \
	images/twinkle24.png \
	images/exit.png \
	images/kontact_contacts.png \
	images/ok.png \
	images/cancel.png \
	images/1rightarrow.png \
	images/1leftarrow-yellow.png \
	images/editdelete.png \
	images/kcmpci16.png \
	images/kontact_contacts-disabled.png \
	images/sys_auto_ans.png \
	images/sys_auto_ans_dis.png \
	images/sys_busy_estab.png \
	images/sys_busy_estab_dis.png \
	images/sys_busy_trans.png \
	images/sys_busy_trans_dis.png \
	images/sys_dnd.png \
	images/sys_dnd_dis.png \
	images/sys_idle.png \
	images/sys_idle_dis.png \
	images/sys_redir.png \
	images/sys_redir_dis.png \
	images/sys_services.png \
	images/sys_services_dis.png \
	images/sys_hold.png \
	images/sys_hold_dis.png \
	images/sys_mute.png \
	images/sys_mute_dis.png \
	images/network.png \
	images/knotify.png \
	images/fileopen.png \
	images/fileopen-disabled.png \
	images/cf.png \
	images/auto_answer.png \
	images/auto_answer-disabled.png \
	images/cancel-disabled.png \
	images/cf-disabled.png \
	images/missed-disabled.png \
	images/missed.png \
	images/sys_missed.png \
	images/sys_missed_dis.png \
	images/twinkle16-disabled.png \
	images/gear.png \
	images/reg_failed-disabled.png \
	images/reg_failed.png \
	images/no-indication.png \
	images/contexthelp.png \
	images/settings.png \
	images/reg-query.png \
	images/log_small.png \
	images/qt-logo.png \
	images/1leftarrow.png \
	images/1uparrow.png \
	images/1downarrow.png \
	images/kontact_contacts32.png \
	images/encrypted.png \
	images/sys_encrypted.png \
	images/sys_encrypted_dis.png \
	images/encrypted32.png \
	images/encrypted-disabled.png \
	images/stat_conference.png \
	images/stat_established.png \
	images/stat_outgoing.png \
	images/stat_ringing.png \
	images/stat_mute.png \
	images/stat_established_nomedia.png \
	images/encrypted_verified.png \
	images/sys_encrypted_verified.png \
	images/sys_encrypted_verified_dis.png \
	images/consult-xfer.png \
	images/mwi_new16.png \
	images/mwi_none16.png \
	images/mwi_none16_dis.png \
	images/sys_mwi.png \
	images/sys_mwi_dis.png \
	images/mwi_none.png \
	images/mwi_failure16.png \
	images/presence_offline.png \
	images/presence_online.png \
	images/presence_failed.png \
	images/presence_rejected.png \
	images/presence_unknown.png \
	images/edit16.png \
	images/message.png \
	images/edit.png \
	images/buddy.png \
	images/message32.png \
	images/presence.png \
	images/save_as.png \
	images/attach.png \
	images/mime_application.png \
	images/mime_audio.png \
	images/mime_image.png \
	images/mime_text.png \
	images/mime_video.png

TRANSLATIONS	= lang/twinkle_nl.ts \
	lang/twinkle_de.ts \
	lang/twinkle_cs.ts \
	lang/twinkle_fr.ts \
	lang/twinkle_ru.ts \
	lang/twinkle_sv.ts \
	lang/twinkle_xx.ts

unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}

include( ../../qtccxxincl.pro )
#The following line was inserted by qt3to4
QT +=  qt3support
#The following line was inserted by qt3to4
CONFIG += uic3

OTHER_FILES += \
    images/yast_PhoneTTOffhook.png \
    images/yast_babelfish.png \
    images/undo \
    images/twinkle48.png \
    images/twinkle32.png \
    images/twinkle24.png \
    images/twinkle16.png \
    images/twinkle16-disabled.png \
    images/transfer.png \
    images/transfer-disabled.png \
    images/telephone-hook.png \
    images/sys_services_dis.png \
    images/sys_services.png \
    images/sys_redir_dis.png \
    images/sys_redir.png \
    images/sys_mwi_dis.png \
    images/sys_mwi.png \
    images/sys_mute_dis.png \
    images/sys_mute.png \
    images/sys_missed_dis.png \
    images/sys_missed.png \
    images/sys_idle_dis.png \
    images/sys_idle.png \
    images/sys_hold_dis.png \
    images/sys_hold.png \
    images/sys_encrypted_verified_dis.png \
    images/sys_encrypted_verified.png \
    images/sys_encrypted_dis.png \
    images/sys_encrypted.png \
    images/sys_dnd_dis.png \
    images/sys_dnd.png \
    images/sys_busy_trans_dis.png \
    images/sys_busy_trans.png \
    images/sys_busy_estab_dis.png \
    images/sys_busy_estab.png \
    images/sys_auto_ans_dis.png \
    images/sys_auto_ans.png \
    images/stat_ringing.png \
    images/stat_outgoing.png \
    images/stat_mute.png \
    images/stat_established_nomedia.png \
    images/stat_established.png \
    images/stat_conference.png \
    images/settings.png \
    images/searchfind \
    images/save_as.png \
    images/reject.png \
    images/reject-disabled.png \
    images/reg_failed.png \
    images/reg_failed-disabled.png \
    images/reg-query.png \
    images/redo \
    images/redirect.png \
    images/redirect-disabled.png \
    images/redial.png \
    images/redial-disabled.png \
    images/qt-logo.png \
    images/print \
    images/presence_unknown.png \
    images/presence_rejected.png \
    images/presence_online.png \
    images/presence_offline.png \
    images/presence_failed.png \
    images/presence.png \
    images/penguin_big.png \
    images/penguin.png \
    images/penguin-small.png \
    images/password.png \
    images/package_system.png \
    images/package_network.png \
    images/ok.png \
    images/no-indication.png \
    images/network.png \
    images/mwi_none16_dis.png \
    images/mwi_none16.png \
    images/mwi_none.png \
    images/mwi_new16.png \
    images/mwi_failure16.png \
    images/mute.png \
    images/mute-disabled.png \
    images/missed.png \
    images/missed-disabled.png \
    images/mime_video.png \
    images/mime_text.png \
    images/mime_image.png \
    images/mime_audio.png \
    images/mime_application.png \
    images/message32.png \
    images/message.png \
    images/log_small.png \
    images/log.png \
    images/kontact_contacts32.png \
    images/kontact_contacts.png \
    images/kontact_contacts-disabled.png \
    images/knotify.png \
    images/kmix.png \
    images/kcmpci16.png \
    images/kcmpci.png \
    images/invite.png \
    images/invite-disabled.png \
    images/hold.png \
    images/hold-disabled.png \
    images/gear.png \
    images/filesave \
    images/fileopen.png \
    images/fileopen-disabled.png \
    images/filenew \
    images/favorites.png \
    images/exit.png \
    images/encrypted_verified.png \
    images/encrypted32.png \
    images/encrypted.png \
    images/encrypted-disabled.png \
    images/editpaste \
    images/editdelete.png \
    images/editcut \
    images/editcopy \
    images/edit16.png \
    images/edit.png \
    images/dtmf.png \
    images/dtmf-star.png \
    images/dtmf-pound.png \
    images/dtmf-disabled.png \
    images/dtmf-d.png \
    images/dtmf-c.png \
    images/dtmf-b.png \
    images/dtmf-a.png \
    images/dtmf-9.png \
    images/dtmf-8.png \
    images/dtmf-7.png \
    images/dtmf-6.png \
    images/dtmf-5.png \
    images/dtmf-4.png \
    images/dtmf-3.png \
    images/dtmf-2.png \
    images/dtmf-1.png \
    images/dtmf-0.png \
    images/contexthelp.png \
    images/consult-xfer.png \
    images/conference.png \
    images/conference-disabled.png \
    images/conf.png \
    images/conf-disabled.png \
    images/clock.png \
    images/cf.png \
    images/cf-disabled.png \
    images/cancel.png \
    images/cancel-disabled.png \
    images/bye.png \
    images/bye-disabled.png \
    images/buddy.png \
    images/auto_answer.png \
    images/auto_answer-disabled.png \
    images/attach.png \
    images/answer.png \
    images/answer-disabled.png \
    images/1uparrow.png \
    images/1rightarrow.png \
    images/1leftarrow.png \
    images/1leftarrow-yellow.png \
    images/1downarrow.png
