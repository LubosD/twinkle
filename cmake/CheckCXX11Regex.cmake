# Check if C++11 regular expressions are available and actually work.
#
# libstdc++ 4.8 shipped with a buggy prototype for C++11 regular expressions,
# which barely supports the simplest cases, and will throw an exception at
# pretty much anything else.  Unfortunately, this does not cause the build to
# fail (since all the symbols are properly exported), and results instead in
# mysterious runtime errors.  (See issue #31 for such an example.)

include(CMakePushCheckState)
include(CheckCXXSourceRuns)

function(check_cxx11_regex result_var)
	# When cross-compiling, check_cxx_source_runs() below will cause the
	# build to fail.  Since libstdc++ 4.8 is pretty much obsolete by now,
	# we may as well assume that everything is working fine.
	if(CMAKE_CROSSCOMPILING)
		message(STATUS "Skipping C++11 regular expressions check due to cross-compilation")
		# Returning a true value that we could recognize if needed
		set(${result_var} 42 PARENT_SCOPE)
		return()
	endif()

	cmake_push_check_state()
	set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -std=c++11")

	# With libstdc++ 4.8, this will compile but crash at runtime
	check_cxx_source_runs("
	    #include <regex>

	    // Regex copied from Apple's SwiftCheckCXXNativeRegex.cmake
	    const std::regex re(\"([a]+)\");

	    int main() {
	        return 0;
	    }
	" ${result_var})
	set(${result_var} ${${result_var}} PARENT_SCOPE)

	cmake_pop_check_state()
endfunction()
