project(libtwinkle-gsm)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../inc)

set(LIBTWINKLE_GSM-SRCS
	add.cpp
	code.cpp
	debug.cpp
	decode.cpp
	gsm_create.cpp
	gsm_decode.cpp
	gsm_destroy.cpp
	gsm_encode.cpp
	gsm_explode.cpp
	gsm_implode.cpp
	gsm_option.cpp
	gsm_print.cpp
	long_term.cpp
	lpc.cpp
	preprocess.cpp
	rpe.cpp
	short_term.cpp
	table.cpp
)

add_library(libtwinkle-gsm OBJECT ${LIBTWINKLE_GSM-SRCS})
