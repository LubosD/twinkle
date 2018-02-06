INCLUDE(CMakePushCheckState)
INCLUDE(CheckCSourceCompiles)

FIND_PATH(G729_INCLUDE_DIR bcg729/decoder.h)
FIND_LIBRARY(G729_LIBRARY NAMES bcg729)

IF(G729_INCLUDE_DIR AND G729_LIBRARY)
	SET(G729_FOUND TRUE)

	# The bcg729 API was changed in 1.0.2 (see #104); since there is no
	# apparent way to determine the actual installed version, this checks
	# whether we are dealing with the old or new API.
	CMAKE_PUSH_CHECK_STATE()
	SET(CMAKE_REQUIRED_INCLUDES "${INCLUDE_DIRECTORIES};${G729_INCLUDE_DIR}")
	SET(CMAKE_REQUIRED_LIBRARIES "${G729_LIBRARY}")
	SET(CMAKE_REQUIRED_QUIET TRUE)
	# Try to compile something using the old (pre-1.0.2) API
	CHECK_C_SOURCE_COMPILES("
		#include <bcg729/encoder.h>
		#include <bcg729/decoder.h>

		int main() {
			/* This function requires an argument since 1.0.2 */
			initBcg729EncoderChannel();
			return 0;
		}
	" G729_OLD_API)
	CMAKE_POP_CHECK_STATE()
ENDIF(G729_INCLUDE_DIR AND G729_LIBRARY)

IF(G729_FOUND)
	IF (NOT G729_FIND_QUIETLY)
		MESSAGE(STATUS "Found bcg729 includes:	${G729_INCLUDE_DIR}/bcg729/decoder.h")
		MESSAGE(STATUS "Found bcg729 library: ${G729_LIBRARY}")
		IF (G729_OLD_API)
			MESSAGE(STATUS "Using the old (pre-1.0.2) bcg729 API")
		ELSE (G729_OLD_API)
			MESSAGE(STATUS "Using the new (post-1.0.2) bcg729 API")
		ENDIF (G729_OLD_API)
	ENDIF (NOT G729_FIND_QUIETLY)
ELSE(G729_FOUND)
	IF (G729_FIND_REQUIRED)
		MESSAGE(FATAL_ERROR "Could NOT find bcg729 development files")
	ENDIF (G729_FIND_REQUIRED)
ENDIF(G729_FOUND)
