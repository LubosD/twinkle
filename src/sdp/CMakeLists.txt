project(libtwinkle-sdp)

BISON_TARGET(MyParser sdp_parser.yxx ${CMAKE_CURRENT_BINARY_DIR}/sdp_parser.cxx COMPILE_FLAGS "-p yysdp")
FLEX_TARGET(MyScanner sdp_scanner.lxx ${CMAKE_CURRENT_BINARY_DIR}/sdp_scanner.cxx COMPILE_FLAGS "-Pyysdp")
ADD_FLEX_BISON_DEPENDENCY(MyScanner MyParser)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(LIBTWINKLE_SDP-SRCS
	sdp.cpp
	sdp_parse_ctrl.cpp
	${CMAKE_CURRENT_BINARY_DIR}/sdp_parser.cxx
	${CMAKE_CURRENT_BINARY_DIR}/sdp_scanner.cxx
	sdp_scanner.cxx
)

add_library(libtwinkle-sdp OBJECT ${LIBTWINKLE_SDP-SRCS})
