project(libtwinkle-utils)

set(LIBTWINKLE_UTILS-SRCS
	file_utils.cpp
	mime_database.cpp
)

add_library(libtwinkle-utils OBJECT ${LIBTWINKLE_UTILS-SRCS})
