# - Try to find libmagic header and library
#
# Usage of this module as follows:
#
#     find_package(LibMagic)
#
# Variables used by this module, they can change the default behaviour and need
# to be set before calling find_package:
#
#  LibMagic_ROOT_DIR         Set this variable to the root installation of
#                            libmagic if the module has problems finding the
#                            proper installation path.
#
# Variables defined by this module:
#
#  LIBMAGIC_FOUND              System has libmagic, magic.h, and file
#  LibMagic_FILE_EXE           Path to the 'file' command
#  LibMagic_VERSION            Version of libmagic
#  LibMagic_LIBRARY            The libmagic library
#  LibMagic_INCLUDE_DIR        The location of magic.h

find_path(LibMagic_ROOT_DIR
    NAMES include/magic.h
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    # the static version of the library is preferred on OS X for the
    # purposes of making packages (libmagic doesn't ship w/ OS X)
    set(libmagic_names libmagic.a magic)
else ()
    set(libmagic_names magic)
endif ()

find_file(LibMagic_FILE_EXE
    NAMES file
    HINTS ${LibMagic_ROOT_DIR}/bin
)

find_library(LibMagic_LIBRARY
    NAMES ${libmagic_names}
    HINTS ${LibMagic_ROOT_DIR}/lib
)

find_path(LibMagic_INCLUDE_DIR
    NAMES magic.h
    HINTS ${LibMagic_ROOT_DIR}/include
)

if (LibMagic_FILE_EXE)
    execute_process(COMMAND "${LibMagic_FILE_EXE}" --version
                    ERROR_VARIABLE  LibMagic_VERSION
                    OUTPUT_VARIABLE LibMagic_VERSION)
    string(REGEX REPLACE "^file-([0-9.]+).*$" "\\1"
           LibMagic_VERSION "${LibMagic_VERSION}")
    message(STATUS "libmagic version: ${LibMagic_VERSION}")
else ()
    set(LibMagic_VERSION NOTFOUND)
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibMagic DEFAULT_MSG
    LibMagic_LIBRARY
    LibMagic_INCLUDE_DIR
    LibMagic_FILE_EXE
    LibMagic_VERSION
)

mark_as_advanced(
    LibMagic_ROOT_DIR
    LibMagic_FILE_EXE
    LibMagic_VERSION
    LibMagic_LIBRARY
    LibMagic_INCLUDE_DIR
)
