project(libtwinkle-audits)

set(LIBTWINKLE_AUDITS-SRCS
	memman.cpp
)

add_library(libtwinkle-audits OBJECT ${LIBTWINKLE_AUDITS-SRCS})
