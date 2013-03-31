# - Find GnuRegex
# Find the native GnuRegex includes and library
#
#  GNUREGEX_INCLUDE_DIR    - where to find regex.h
#  GNUREGEX_LIBRARIES   - List of libraries when using GnuRegex.
#  GNUREGEX_FOUND       - True if GnuRegex found.

find_path (GNUREGEX_INCLUDE_DIR regex.h)
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	# Windows, library needed
	find_library(GNUREGEX_LIBRARIES NAMES regex gnurx)
else(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	# library not needed
	set(GNUREGEX_LIBRARIES TRUE)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

# handle the QUIETLY and REQUIRED arguments and set GNUREGEX_FOUND to TRUE if
# all listed variables are TRUE
include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (GnuRegex DEFAULT_MSG GNUREGEX_LIBRARIES GNUREGEX_INCLUDE_DIR)

mark_as_advanced (GNUREGEX_LIBRARIES GNUREGEX_INCLUDE_DIR)
