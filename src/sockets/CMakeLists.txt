project(libtwinkle-sockets)

set(LIBTWINKLE_SOCKETS-SRCS
	connection.cpp
	connection_table.cpp
	dnssrv.cpp
        interfaces.cpp
        socket.cpp
        url.cpp
)

add_library(libtwinkle-sockets OBJECT ${LIBTWINKLE_SOCKETS-SRCS})
