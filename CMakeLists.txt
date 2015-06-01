project(twinkle)

cmake_minimum_required(VERSION 2.6.0 FATAL_ERROR)

set(PRODUCT_VERSION "1.9.0")
set(PRODUCT_DATE    "June 1, 2015")

OPTION(WITH_ZRTP	"Enable ZRTP encrypted calls" OFF)
OPTION(WITH_SPEEX	"Enable the Speex codec" OFF)
OPTION(WITH_ILBC	"Enable the iLBC codec" OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include (CheckIncludeFile)
find_package(Qt4 REQUIRED COMPONENTS QTGUI QT3SUPPORT)
find_package(LibXml2 REQUIRED)
find_package(ALSA REQUIRED)
find_package(LibMagic REQUIRED)
find_package(LibSndfile REQUIRED)
find_package(Readline REQUIRED)
find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)
find_package(Boost REQUIRED COMPONENTS regex)

include_directories(${LIBXML2_INCLUDE_DIR} ${QT_QT3SUPPORT_INCLUDE_DIR}
	${QT_QTCORE_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR} ${QT_INCLUDE_DIR})

if (WITH_ZRTP)
	find_package(Zrtpcpp)
	
	if (ZRTPCPP_FOUND)
		message(STATUS "libzrtpcpp OK")
		
		include_directories(${ZRTPCPP_INCLUDE_DIR})
	else (ZRTPCPP_FOUND)
		message(FATAL_ERROR "libzrtpcpp not found!")
	endif (ZRTPCPP_FOUND)
	
	#####
	
	find_package(Ccrtp)
	
	if (CCRTP_FOUND)
		message(STATUS "ccrtp OK")
		
		include_directories(${CCRTP_INCLUDE_DIR})
	else (CCRTP_FOUND)
		message(FATAL_ERROR "ccrtp not found!")
	endif (CCRTP_FOUND)
	
	#####
	
	find_package(Commoncpp)
	
	if (COMMONCPP_FOUND)
		message(STATUS "commoncpp OK")
		
		include_directories(${COMMONCPP_INCLUDE_DIR})
	else (COMMONCPP_FOUND)
		message(FATAL_ERROR "commoncpp not found!")
	endif (COMMONCPP_FOUND)
	
	#####
	
	find_package(Ucommon)
	
	if (UCOMMON_FOUND)
		message(STATUS "ucommon OK")
		
		include_directories(${UCOMMON_INCLUDE_DIR})
	else (UCOMMON_FOUND)
		message(FATAL_ERROR "ucommon not found!")
	endif (UCOMMON_FOUND)
endif (WITH_ZRTP)

if (WITH_SPEEX)
	find_package(Speex)
	
	if (SPEEX_FOUND)
		message(STATUS "Speex OK")
		
		include_directories(${SPEEX_INCLUDE_DIR})
	else (SPEEX_FOUND)
		message(FATAL_ERROR "Speex not found!")
	endif (SPEEX_FOUND)
endif (WITH_SPEEX)

if (WITH_ILBC)
	find_package(Ilbc)
	
	if (ILBC_FOUND)
		message(STATUS "iLBC OK")
		
		include_directories(${ILBC_INCLUDE_DIR})
	else (ILBC_FOUND)
		message(FATAL_ERROR "iLBC not found!")
	endif (ILBC_FOUND)
endif (WITH_ILBC)

check_include_file(unistd.h HAVE_UNISTD_H)
check_include_file(linux/types.h HAVE_LINUX_TYPES_H)
check_include_file(linux/errqueue.h HAVE_LINUX_ERRQUEUE_H)

configure_file(twinkle_config.h.in twinkle_config.h)
include_directories("${CMAKE_BINARY_DIR}")

install(FILES
	${CMAKE_CURRENT_SOURCE_DIR}/data/providers.csv
	${CMAKE_CURRENT_SOURCE_DIR}/data/ringtone.wav
	${CMAKE_CURRENT_SOURCE_DIR}/data/ringback.wav
	${CMAKE_CURRENT_SOURCE_DIR}/src/gui/images/twinkle16.png
	${CMAKE_CURRENT_SOURCE_DIR}/src/gui/images/twinkle32.png
	${CMAKE_CURRENT_SOURCE_DIR}/src/gui/images/twinkle48.png
	DESTINATION share/twinkle)
install(FILES
	${CMAKE_CURRENT_SOURCE_DIR}/src/gui/images/twinkle48.png
	DESTINATION share/pixmaps
	RENAME twinkle.png)

add_subdirectory(src)
