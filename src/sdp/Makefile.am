AM_CPPFLAGS = \
	-Wall \
	-I$(top_srcdir)/src\
	$(XML2_CFLAGS)

AM_YFLAGS = -p yysdp -d
AM_LFLAGS = -Pyysdp -olex.yy.c -i

# This target is only for testing the parser in isolation
# noinst_PROGRAMS = sdpparse

# sdpparse_SOURCES = main.cpp

# sdpparse_LDADD = $(top_builddir)/src/util.o\
# 	$(top_builddir)/src/sdp/libsdpparser.a\
# 	$(top_builddir)/src/parser/sip_body.o

noinst_LIBRARIES = libsdpparser.a


libsdpparser_a_SOURCES =\
        sdp.cpp\
        sdp_parse_ctrl.cpp\
	sdp_parser.yxx\
	sdp_scanner.lxx\
        sdp.h\
        sdp_parse_ctrl.h
