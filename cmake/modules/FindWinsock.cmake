# - Find Winsock
# Find the Windows socket includes and library
#
#  WINSOCK_INCLUDE_DIR    - where to find regex.h
#  WINSOCK_LIBRARIES   - List of libraries when using Winsock.
#  WINSOCK_FOUND       - True if Winsock found.

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	find_path(WINSOCK_INCLUDE_DIR winsock2.h
		HINTS
		C:/MinGW/include/)
	find_library(WINSOCK_LIBRARIES NAMES ws2_32
		HINTS
		C:/MinGW/lib/)
else(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	# Do nothing if not on Windows
	set(WINSOCK_INCLUDE_DIR TRUE)
	set(WINSOCK_LIBRARIES TRUE)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args (Winsock DEFAULT_MSG WINSOCK_LIBRARIES WINSOCK_INCLUDE_DIR)

mark_as_advanced(WINSOCK_LIBRARIES WINSOCK_INCLUDE_DIR)
