AM_CPPFLAGS = \
	-I$(srcdir)/inc \
	-DSASR -DNeedFunctionPrototypes=1

noinst_LIBRARIES = libgsm.a

libgsm_a_SOURCES =\
	src/add.cpp\
	src/code.cpp\
	src/debug.cpp\
	src/decode.cpp\
	src/long_term.cpp\
	src/lpc.cpp\
	src/preprocess.cpp\
	src/rpe.cpp\
	src/gsm_destroy.cpp\
	src/gsm_decode.cpp\
	src/gsm_encode.cpp\
	src/gsm_explode.cpp\
	src/gsm_implode.cpp\
	src/gsm_create.cpp\
	src/gsm_print.cpp\
	src/gsm_option.cpp\
	src/short_term.cpp\
	src/table.cpp\
	inc/gsm.h\
	inc/proto.h\
	inc/config.h\
	inc/private.h\
	inc/unproto.h

EXTRA_DIST = COPYRIGHT MACHINES
