project(libtwinkle-presence)

set(LIBTWINKLE_PRESENCE-SRCS
	buddy.cpp
	pidf_body.cpp
	presence_dialog.cpp
	presence_epa.cpp
	presence_state.cpp
	presence_subscription.cpp
)

add_library(libtwinkle-presence OBJECT ${LIBTWINKLE_PRESENCE-SRCS})
