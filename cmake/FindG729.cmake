INCLUDE(CMakePushCheckState)
INCLUDE(CheckCSourceCompiles)

FIND_PATH(G729_INCLUDE_DIR bcg729/decoder.h)
FIND_LIBRARY(G729_LIBRARY NAMES bcg729)

IF(G729_INCLUDE_DIR AND G729_LIBRARY)
	SET(G729_FOUND TRUE)

	# The bcg729 API was changed in 1.0.2 to add support for G.729 Annex B.
	# This checks whether we are dealing with the old or new API.
	CMAKE_PUSH_CHECK_STATE()
	SET(CMAKE_REQUIRED_INCLUDES "${INCLUDE_DIRECTORIES}" "${G729_INCLUDE_DIR}")
	SET(CMAKE_REQUIRED_LIBRARIES "${G729_LIBRARY}")
	SET(CMAKE_REQUIRED_QUIET TRUE)
	# Try to compile something using the old (pre-1.0.2) API.
	#
	# We cannot do it the other way around, as initBcg729EncoderChannel()
	# did not have a prototype before 1.0.2, thus compilation would not fail
	# when passing it an extra argument.
	CHECK_C_SOURCE_COMPILES("
		#include <bcg729/encoder.h>

		int main() {
			/* This function requires an argument since 1.0.2 */
			initBcg729EncoderChannel();
			return 0;
		}
	" G729_OLD_API)
	CMAKE_POP_CHECK_STATE()

	IF (G729_OLD_API)
		SET(G729_ANNEX_B FALSE)
	ELSE (G729_OLD_API)
		SET(G729_ANNEX_B TRUE)
	ENDIF (G729_OLD_API)
ENDIF(G729_INCLUDE_DIR AND G729_LIBRARY)

IF(G729_FOUND)
	IF (NOT G729_FIND_QUIETLY)
		MESSAGE(STATUS "Found bcg729 includes:	${G729_INCLUDE_DIR}/bcg729/decoder.h")
		MESSAGE(STATUS "Found bcg729 library: ${G729_LIBRARY}")
		IF (G729_ANNEX_B)
			MESSAGE(STATUS "bcg729 supports Annex B; using the new (1.0.2) API")
		ELSE (G729_ANNEX_B)
			MESSAGE(STATUS "bcg729 does not support Annex B; using the old (pre-1.0.2) API")
		ENDIF (G729_ANNEX_B)
	ENDIF (NOT G729_FIND_QUIETLY)
ELSE(G729_FOUND)
	IF (G729_FIND_REQUIRED)
		MESSAGE(FATAL_ERROR "Could NOT find bcg729 development files")
	ENDIF (G729_FIND_REQUIRED)
ENDIF(G729_FOUND)
