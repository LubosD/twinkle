AM_LDFLAGS = -L $(libdir)

AM_CPPFLAGS = -Wall $(CCRTP_CFLAGS) $(XML2_CFLAGS) -DDATADIR=\"$(pkgdatadir)\"

noinst_PROGRAMS = twinkle

noinst_LIBRARIES = libtwinkle.a

twinkle_SOURCES = main.cpp

twinkle_LDADD = libtwinkle.a\
	$(top_builddir)/src/parser/libsipparser.a\
        $(top_builddir)/src/sdp/libsdpparser.a\
        $(top_builddir)/src/sockets/libsocket.a\
        $(top_builddir)/src/threads/libthread.a\
        $(top_builddir)/src/audio/libaudio.a\
        $(top_builddir)/src/audits/libaudits.a\
        $(top_builddir)/src/stun/libstun.a\
        $(top_builddir)/src/mwi/libmwi.a\
        $(top_builddir)/src/im/libim.a\
        $(top_builddir)/src/presence/libpresence.a\
        $(top_builddir)/src/patterns/libpatterns.a\
        $(top_builddir)/src/utils/libutils.a\
	$(GSM_LIBS)\
	$(CCRTP_LIBS)\
	$(XML2_LIBS)\
	-lncurses \
	-lreadline \
	-lsndfile\
	-lmagic

libtwinkle_a_SOURCES =\
	abstract_dialog.cpp\
	address_book.cpp\
        auth.cpp\
        call_history.cpp\
        call_script.cpp\
        client_request.cpp\
        cmd_socket.cpp\
        dialog.cpp\
        diamondcard.cpp\
        epa.cpp\
        events.cpp\
        id_object.cpp\
        line.cpp\
        listener.cpp\
        log.cpp\
        phone.cpp\
        phone_user.cpp\
        prohibit_thread.cpp\
        redirect.cpp\
        sender.cpp\
        service.cpp\
        session.cpp\
	sub_refer.cpp\
	subscription.cpp\
	subscription_dialog.cpp\
        sys_settings.cpp\
        timekeeper.cpp\
        transaction.cpp\
        transaction_layer.cpp\
        transaction_mgr.cpp\
        user.cpp\
        userintf.cpp\
        util.cpp\
        abstract_dialog.h\
        address_book.h\
        auth.h\
        call_history.h\
        call_script.h\
        client_request.h\
        cmd_socket.h\
        dialog.h\
        diamondcard.h\
        epa.h\
        events.h\
	exceptions.h\
	id_object.h\
        line.h\
        listener.h\
        log.h\
        phone.h\
        phone_user.h\
        prohibit_thread.h\
	protocol.h\
        redirect.h\
        sender.h\
	sequence_number.h\
        service.h\
        session.h\
	sub_refer.h\
	subscription.h\
	subscription_dialog.h\
        sys_settings.h\
        timekeeper.h\
        transaction.h\
        transaction_layer.h\
        transaction_mgr.h\
        translator.h\
        user.h\
        userintf.h\
        util.h

SUBDIRS = audio audits im mwi parser patterns presence sdp sockets stun threads utils . gui gui/lang
