AM_CPPFLAGS = \
	-Wall \
	-I$(top_srcdir)/src \
	$(CCRTP_CFLAGS) \
	$(XML2_CFLAGS)

#noinst_PROGRAMS = mwitest

#mwitest_SOURCES = mwitest.cpp

#mwitest_LDADD = $(top_builddir)/src/util.o\
#	$(top_builddir)/src/mwi/libmwi.a\
#	$(top_builddir)/src/sockets/libsocket.a\
#	$(top_builddir)/src/parser/libsipparser.a\
#	$(top_builddir)/src/sdp/libsdpparser.a\
#	-lresolv\
#	-lboost_regex

noinst_LIBRARIES = libmwi.a

libmwi_a_SOURCES =\
	mwi.cpp\
	mwi_dialog.cpp\
	mwi_subscription.cpp\
	simple_msg_sum_body.cpp\
	mwi.h\
	mwi_dialog.h\
	mwi_subscription.h\
	simple_msg_sum_body.h
