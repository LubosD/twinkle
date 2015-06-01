project(libtwinkle-stun)

set(LIBTWINKLE_STUN-SRCS
	stun.cxx
	stun_transaction.cpp
	udp.cxx
)

add_library(libtwinkle-stun OBJECT ${LIBTWINKLE_STUN-SRCS})
