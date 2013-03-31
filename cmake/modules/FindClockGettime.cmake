
include(CheckFunctionExists)

if(NOT CLOCK_GETTIME_FOUND)
    check_function_exists(clock_gettime HAVE_CLOCK_GETTIME)
    if(HAVE_CLOCK_GETTIME)
        set(CLOCK_GETTIME_FOUND TRUE)
        set(CLOCK_GETTIME_LIBRARIES "")
    else()
        message(STATUS "clock_gettime() not found, searching for library")
    endif()

    if(NOT CLOCK_GETTIME_FOUND)
		find_path(CLOCK_GETTIME_INCLUDE_DIR
			NAMES
				time.h
			PATHS
				${LIBRT_PREFIX}/include/
		)

		find_library(
			CLOCK_GETTIME_LIBRARIES rt
			PATHS
				${LIBRT_PREFIX}/lib/
				/usr/local/lib64/
				/usr/local/lib/
				/usr/lib/i386-linux-gnu/
				/usr/lib/x86_64-linux-gnu/
				/usr/lib64/
				/usr/lib/
		)

        find_package_handle_standard_args (clock_gettime DEFAULT_MSG CLOCK_GETTIME_LIBRARIES CLOCK_GETTIME_INCLUDE_DIR)

		mark_as_advanced(CLOCK_GETTIME_LIBRARIES CLOCK_GETTIME_INCLUDE_DIR)
    endif()
endif()
