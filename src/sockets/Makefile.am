AM_CPPFLAGS = \
	-Wall \
	-I$(top_srcdir)/src\
	$(XML2_CFLAGS)

noinst_LIBRARIES = libsocket.a

#noinst_PROGRAMS = srvinfo urlinfo
#srvinfo_SOURCES = srvinfo.cpp
#srvinfo_LDADD = $(top_builddir)/src/util.o\
#	$(top_builddir)/src/sockets/libsocket.a\
#	-lresolv
#urlinfo_SOURCES = urlinfo.cpp
#urlinfo_LDADD = $(top_builddir)/src/util.o\
#	$(top_builddir)/src/sockets/libsocket.a\
#	-lresolv

libsocket_a_SOURCES =\
	connection.cpp\
	connection_table.cpp\
	dnssrv.cpp\
        interfaces.cpp\
        socket.cpp\
        url.cpp\
        connection.h\
        connection_table.h\
        dnssrv.h\
        interfaces.h\
        socket.h\
        url.h
