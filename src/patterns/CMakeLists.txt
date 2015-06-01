project(libtwinkle-patterns)

set(LIBTWINKLE_PATTERNS-SRCS
	observer.cpp
)

add_library(libtwinkle-patterns OBJECT ${LIBTWINKLE_PATTERNS-SRCS})
