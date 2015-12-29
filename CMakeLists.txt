project(twinkle)

cmake_minimum_required(VERSION 2.6.0 FATAL_ERROR)

set(PRODUCT_VERSION "1.9.0")
set(PRODUCT_DATE    "July 10, 2015")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

OPTION(WITH_ZRTP		"Enable ZRTP encrypted calls" OFF)
OPTION(WITH_SPEEX		"Enable the Speex codec" OFF)
OPTION(WITH_ILBC		"Enable the iLBC codec" OFF)
OPTION(WITH_ALSA		"Enable ALSA support" ON)
OPTION(WITH_DIAMONDCARD	"Enable Diamondcard integration" OFF)
OPTION(WITH_QT4			"Enable Qt 4 GUI" OFF)
OPTION(WITH_QT5			"Enable Qt 5 GUI" OFF)
OPTION(WITH_G729		"Enable G.729A support" OFF)
OPTION(WITH_GSM			"Use external GSM library" OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

if (WITH_QT4 AND WITH_QT5)
		message(FATAL_ERROR "Qt 4 and Qt 5 cannot be enabled at the same time!")
endif (WITH_QT4 AND WITH_QT5)

include (CheckIncludeFile)

find_package(LibXml2 REQUIRED)
find_package(LibMagic REQUIRED)
find_package(LibSndfile REQUIRED)
find_package(Readline REQUIRED)
find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)
find_package(Commoncpp REQUIRED)
find_package(Ccrtp REQUIRED)
find_package(Ucommon REQUIRED)

if (WITH_QT4)
	find_package(Qt4 REQUIRED COMPONENTS QTGUI QTDECLARATIVE)
	include_directories(${QT_QT3SUPPORT_INCLUDE_DIR}
		${QT_QTCORE_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR}
		${QT_QTDECLARATIVE_INCLUDE_DIR} ${QT_INCLUDE_DIR})

elseif (WITH_QT5)
	find_package(Qt5Widgets REQUIRED)
	find_package(Qt5LinguistTools REQUIRED)
	find_package(Qt5Declarative REQUIRED)
	find_package(Qt5Quick REQUIRED)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS} ${Qt5Declarative_EXECUTABLE_COMPILE_FLAGS}")
	include_directories(${Qt5Widgets_INCLUDES} ${Qt5Declarative_INCLUDES})
	add_definitions(${Qt5Widgets_DEFINITIONS} ${Qt5Declarative_DEFINITIONS})
endif (WITH_QT4)

include_directories(${LIBXML2_INCLUDE_DIR})

if (WITH_ALSA)
	find_package(ALSA)

	if (ALSA_FOUND)
		message(STATUS "libasound OK")
		set(HAVE_LIBASOUND TRUE)
	else (ALSA_FOUND)
		message(FATAL_ERROR "libasound not found!")
	endif (ALSA_FOUND)
endif (WITH_ALSA)

if (WITH_ZRTP)
	find_package(Zrtpcpp)
	
	if (ZRTPCPP_FOUND)
		message(STATUS "libzrtpcpp OK")
		set(HAVE_ZRTP TRUE)
		
		include_directories(${ZRTPCPP_INCLUDE_DIR})
	else (ZRTPCPP_FOUND)
		message(FATAL_ERROR "libzrtpcpp not found!")
	endif (ZRTPCPP_FOUND)
endif (WITH_ZRTP)

if (WITH_SPEEX)
	find_package(Speex)
	
	if (SPEEX_FOUND)
		message(STATUS "Speex OK")
		set(HAVE_SPEEX TRUE)
		
		include_directories(${SPEEX_INCLUDE_DIR})
	else (SPEEX_FOUND)
		message(FATAL_ERROR "Speex not found!")
	endif (SPEEX_FOUND)
endif (WITH_SPEEX)

if (WITH_ILBC)
	find_package(Ilbc)
	
	if (ILBC_FOUND)
		message(STATUS "iLBC OK")
		set(HAVE_ILBC TRUE)
		
		include_directories(${ILBC_INCLUDE_DIR})
	else (ILBC_FOUND)
		message(FATAL_ERROR "iLBC not found!")
	endif (ILBC_FOUND)
endif (WITH_ILBC)

if (WITH_G729)
	find_package(G729)
	
	if (G729_FOUND)
		message(STATUS "bcg729 OK")
		set(HAVE_BCG729 TRUE)
		
		include_directories(${G729_INCLUDE_DIR})
	else (G729_FOUND)
		message(FATAL_ERROR "bcg729 not found!")
	endif (G729_FOUND)
endif (WITH_G729)

if (WITH_GSM)
	find_package(Gsm)

	if (GSM_FOUND)
		message(STATUS "gsm OK")
		set(HAVE_GSM TRUE)

		include_directories(${GSM_INCLUDE_DIR})
	else (GSM_FOUND)
		message(FATAL_ERROR "gsm not found!")
	endif (GSM_FOUND)
endif (WITH_GSM)

check_include_file(unistd.h HAVE_UNISTD_H)
check_include_file(linux/types.h HAVE_LINUX_TYPES_H)
check_include_file(linux/errqueue.h HAVE_LINUX_ERRQUEUE_H)

set(datadir "${CMAKE_INSTALL_PREFIX}/share/twinkle")
configure_file(twinkle_config.h.in twinkle_config.h)
configure_file(twinkle.desktop.in twinkle.desktop)

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
install(FILES
	${CMAKE_CURRENT_BINARY_DIR}/twinkle.desktop
	DESTINATION share/applications)

add_subdirectory(src)
