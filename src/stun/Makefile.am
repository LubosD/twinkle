AM_CPPFLAGS = \
	-Wall \
	-I$(top_srcdir)/src \
	$(CCRTP_CFLAGS)\
	$(XML2_CFLAGS)

noinst_LIBRARIES = libstun.a

libstun_a_SOURCES =\
	stun.cxx\
	stun_transaction.cpp\
	udp.cxx\
	stun.h\
	stun_transaction.h\
	udp.h
