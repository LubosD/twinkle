project(libtwinkle-im)

set(LIBTWINKLE_IM-SRCS
	im_iscomposing_body.cpp
	msg_session.cpp
)

add_library(libtwinkle-im OBJECT ${LIBTWINKLE_IM-SRCS})
