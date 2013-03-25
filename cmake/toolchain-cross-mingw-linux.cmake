# adapted from http://www.cmake.org/Wiki/File:Toolchain-cross-mingw32-linux.cmake

# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)

IF(NOT ("${GNU_HOST}" STREQUAL ""))
    find_program(CC_GNUHOST NAMES ${GNU_HOST}-gcc)
ENDIF()

if(CC_GNUHOST)
	set(COMPILER_PREFIX "${GNU_HOST}")
else()
	find_program(CC_W64 NAMES i686-w64-mingw32-gcc)
	if(CC_W64)
		# for 32 or 64 bits mingw-w64
		# see http://mingw-w64.sourceforge.net/
		set(COMPILER_PREFIX "i686-w64-mingw32")
	else()
		# for classical mingw32
		# see http://www.mingw.org/
		set(COMPILER_PREFIX "i586-mingw32msvc")
	endif()
endif()

#set(COMPILER_PREFIX "x86_64-w64-mingw32"

# which compilers to use for C and C++
find_program(CMAKE_RC_COMPILER NAMES ${COMPILER_PREFIX}-windres)
#SET(CMAKE_RC_COMPILER ${COMPILER_PREFIX}-windres)
find_program(CMAKE_C_COMPILER NAMES ${COMPILER_PREFIX}-gcc)
#SET(CMAKE_C_COMPILER ${COMPILER_PREFIX}-gcc)
find_program(CMAKE_CXX_COMPILER NAMES ${COMPILER_PREFIX}-g++)
#SET(CMAKE_CXX_COMPILER ${COMPILER_PREFIX}-g++)


# here is the target environment located
SET(CMAKE_FIND_ROOT_PATH  /usr/${COMPILER_PREFIX} /usr/${COMPILER_PREFIX}/sys-root/${COMPILER_PREFIX} /usr/${COMPILER_PREFIX}/sys-root/mingw)
SET(CMAKE_SYSTEM_PREFIX_PATH ${CMAKE_FIND_ROOT_PATH}) # needed to make FindSDL find includes

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search 
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
 
