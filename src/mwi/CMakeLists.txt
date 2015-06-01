project(libtwinkle-mwi)

set(LIBTWINKLE_MWI-SRCS
	mwi.cpp
	mwi_dialog.cpp
	mwi_subscription.cpp
	simple_msg_sum_body.cpp
)

add_library(libtwinkle-mwi OBJECT ${LIBTWINKLE_MWI-SRCS})
