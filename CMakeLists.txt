project(twinkle)

cmake_minimum_required(VERSION 2.6.0 FATAL_ERROR)

OPTION(WITH_ZRTP	"Enable ZRTP encrypted calls" OFF)
OPTION(WITH_SPEEX	"Enable the Speex codec" OFF)
OPTION(WITH_ILBC	"Enable the iLBC codec" OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
find_package(Qt4 REQUIRED)

if (WITH_ZRTP)
	find_package(Zrtpcpp)
	
	if (ZRTPCPP_FOUND)
		message(STATUS "libzrtpcpp OK")
	else (ZRTPCPP_FOUND)
		message(FATAL_ERROR "libzrtpcpp not found!")
	endif (ZRTPCPP_FOUND)
endif (WITH_ZRTP)

if (WITH_SPEEX)
	find_package(Speex)
	
	if (SPEEX_FOUND)
		message(STATUS "Speex OK")
	else (SPEEX_FOUND)
		message(FATAL_ERROR "Speex not found!")
	endif (SPEEX_FOUND)
endif (WITH_SPEEX)

if (WITH_ILBC)
	find_package(Ilbc)
	
	if (ILBC_FOUND)
		message(STATUS "iLBC OK")
	else (ILBC_FOUND)
		message(FATAL_ERROR "iLBC not found!")
	endif (ILBC_FOUND)
endif (WITH_ILBC)
