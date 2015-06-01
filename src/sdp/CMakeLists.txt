project(libtwinkle-sdp)

#BISON_TARGET(MyParser migcom.tproj/parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.c)
#FLEX_TARGET(MyScanner migcom.tproj/lexxer.l ${CMAKE_CURRENT_BINARY_DIR}/lexer.c)
#ADD_FLEX_BISON_DEPENDENCY(MyScanner MyParser)

set(LIBTWINKLE_SDP-SRCS
	sdp.cpp
        sdp_parse_ctrl.cpp
        sdp_scanner.cxx
)

add_library(libtwinkle-sdp OBJECT ${LIBTWINKLE_SDP-SRCS})
