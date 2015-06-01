project(libtwinkle-threads)

set(LIBTWINKLE_THREADS-SRCS
	thread.cpp
	mutex.cpp
	sema.cpp
)

add_library(libtwinkle-threads OBJECT ${LIBTWINKLE_THREADS-SRCS})
