project(powder)

cmake_minimum_required(VERSION 2.8)

# TODO: mingw windres?
# Force 32/64
# Stable, release flags
# OpenGL

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

file(GLOB powder_SRC
    "src/elements/*.cpp"
    "src/*.cpp"
    "src/*/*.cpp"
    "src/*/*/*.cpp"
    "generated/*.cpp"
)

# TODO: remove this once everything is C++, this is needed because the C++ stuff interacts with stuff in .c files and not all the headers for .c files have extern "C"
set_source_files_properties(${powder_SRC} PROPERTIES LANGUAGE CXX )

include_directories(includes)
include_directories(data)
include_directories(generated)
include_directories(resources)
include_directories(src)
add_executable(powder ${powder_SRC})

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	add_definitions(-D_64BIT)
endif()

option(Renderer "Compiles TPT for site previews rendering" OFF)
if(Renderer)
	add_definitions(-DRENDERER)
else()
	add_definitions(-DUSE_SDL)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	add_definitions(-DLIN)
	add_definitions(-DLINUX)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	add_definitions(-DWIN32)
	add_definitions(-DWINDOWS)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	add_definitions(-DMACOSX)
	#TODO: Include SDLMain.h and SDLMain.m
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")


option(PreferStatic "Prefer linking to static libraries in most cases" ON)
set(CMAKE_FIND_LIBRARY_SUFFIXES_ORIG ${CMAKE_FIND_LIBRARY_SUFFIXES})
set(CMAKE_FIND_LIBRARY_SUFFIXES_ONLY_STATIC .a;)


if(PreferStatic AND (${CMAKE_SYSTEM_NAME} MATCHES "Windows"))
	if(CMAKE_COMPILER_IS_GNUCC)
		set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -static-libstdc++ -static-libgcc")
	endif(CMAKE_COMPILER_IS_GNUCC)
	if(CMAKE_COMPILER_IS_GNUCXX)
		set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -static-libstdc++ -static-libgcc")
	endif(CMAKE_COMPILER_IS_GNUCXX)
endif(PreferStatic AND (${CMAKE_SYSTEM_NAME} MATCHES "Windows"))


if(PreferStatic)
	set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES_ONLY_STATIC})
	find_package(GnuRegex)
	set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES_ORIG})
endif(PreferStatic)
if(NOT GNUREGEX_FOUND)
	find_package(GnuRegex REQUIRED)
endif(NOT GNUREGEX_FOUND)
include_directories(${GNUREGEX_INCLUDE_DIR})
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_link_libraries(powder ${GNUREGEX_LIBRARIES})
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")


find_package(Threads REQUIRED)
target_link_libraries(powder ${CMAKE_THREAD_LIBS_INIT})


option(GravityFFT "Enable FFTs for Newtonian gravity (makes it faster)" ON)
if (GravityFFT)
	if(PreferStatic)
		set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES_ONLY_STATIC})
		find_package(FFTW3F)
		set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES_ORIG})
	endif(PreferStatic)
	if(NOT FFTW3F_FOUND)
		find_package(FFTW3F)
	endif(NOT FFTW3F_FOUND)
	if (FFTW3F_FOUND)
		include_directories(${FFTW3F_INCLUDE_DIR})
		target_link_libraries(powder ${FFTW3F_LIBRARIES})
		add_definitions(-DGRAVFFT)
	else (FFTW3F_FOUND)
		message(STATUS "Package FFTW3F not found, compiling without FFTs for Newtonian gravity")
	endif (FFTW3F_FOUND)
endif (GravityFFT)


# lm

if(PreferStatic)
	set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES_ONLY_STATIC})
	find_package(BZip2)
	set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES_ORIG})
endif(PreferStatic)
if(NOT BZIP2_FOUND)
	find_package(BZip2 REQUIRED)
endif(NOT BZIP2_FOUND)
include_directories(${BZIP2_INCLUDE_DIR})
target_link_libraries(powder ${BZIP2_LIBRARIES})

# zlib

if(PreferStatic)
	set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES_ONLY_STATIC})
	find_package(ZLIB)
	set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES_ORIG})
endif(PreferStatic)
if(NOT ZLIB_FOUND)
	find_package(ZLIB REQUIRED)
endif(NOT ZLIB_FOUND)
include_directories(${ZLIB_INCLUDE_DIR})
target_link_libraries(powder ${ZLIB_LIBRARIES})


option(LuaConsole "Enable Lua console" ON)
if (LuaConsole)
	if(PreferStatic)
		IF(UNIX AND NOT APPLE)
			# For Linux, find maths library first, because otherwise find_package(Lua51) will fail to find the static Lua library if there isn't a static libm
			find_library(LUA_MATH_LIBRARY m)
		ENDIF(UNIX AND NOT APPLE)
		set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES_ONLY_STATIC})
		find_package(Lua51)
		set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES_ORIG})
		if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
			if(LUA51_FOUND)
				FIND_LIBRARY(LUA_EXTRA_LIBS dl)
				if(LUA_EXTRA_LIBS STREQUAL "LUA_EXTRA_LIBS-NOTFOUND")
					message(STATUS "Could not find libdl, trying dynamic lua51")
					unset(LUA51_FOUND)
					unset(LUA_LIBRARY CACHE)
					unset(LUA_LIBRARIES CACHE)
				endif()
			endif(LUA51_FOUND)
		else(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
			set(LUA_EXTRA_LIBS "")
		endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	endif(PreferStatic)
	if(NOT LUA51_FOUND)
		find_package(Lua51)
		set(LUA_EXTRA_LIBS "")
	endif(NOT LUA51_FOUND)
	if (LUA51_FOUND)
		include_directories(${LUA_INCLUDE_DIR})
		target_link_libraries(powder ${LUA_LIBRARIES};${LUA_EXTRA_LIBS})
		add_definitions(-DLUACONSOLE)
	else (LUA51_FOUND)
		message(STATUS "Package lua51 not found, compiling without Lua console")
	endif (LUA51_FOUND)
endif (LuaConsole)


if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	find_package(ClockGettime REQUIRED)
	include_directories(${CLOCK_GETTIME_INCLUDE_DIR})
	target_link_libraries(powder ${CLOCK_GETTIME_LIBRARIES})
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	find_package(X11 REQUIRED)
	include_directories(${X11_X11_INCLUDE_PATH})
	target_link_libraries(powder ${X11_X11_LIB})
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	find_package(Winsock REQUIRED)
	include_directories(${WINSOCK_INCLUDE_DIR})
	target_link_libraries(powder ${WINSOCK_LIBRARIES})
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

# SDL

if(PreferStatic)
	set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES_ONLY_STATIC})
	find_package(SDL)
	if(SDL_FOUND)
		find_package(SDL_static_extra)
		if(NOT SDL_STATIC_EXTRA_LIBRARIES)
			set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES_ORIG})
			find_package(SDL_static_extra REQUIRED)
		endif(NOT SDL_STATIC_EXTRA_LIBRARIES)
		if(SDL_STATIC_EXTRA_LIBRARIES)
			include_directories(${SDL_INCLUDE_DIR})
			target_link_libraries(powder ${SDL_LIBRARY})
			if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
				target_link_libraries(powder ${SDL_STATIC_EXTRA_LIBRARIES})
			endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
		else()
			set(SDL_FOUND false)
		endif()
	endif(SDL_FOUND)
	set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES_ORIG})
endif(PreferStatic)
if(NOT SDL_FOUND)
	find_package(SDL REQUIRED)
	include_directories(${SDL_INCLUDE_DIR})
	target_link_libraries(powder ${SDL_LIBRARY})
endif(NOT SDL_FOUND)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++98 -fkeep-inline-functions")

option(Optimisations "Enable optimisations" ON)
if (Optimisations)
	if(CMAKE_COMPILER_IS_GNUCC)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -ffast-math -ftree-vectorize -funsafe-math-optimizations")
	endif(CMAKE_COMPILER_IS_GNUCC)
	if(CMAKE_COMPILER_IS_GNUCXX)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ffast-math -ftree-vectorize -funsafe-math-optimizations")
	endif(CMAKE_COMPILER_IS_GNUCXX)
endif(Optimisations)

option(Debug "Enable debug symbols" ON)
if (Debug)
	if(CMAKE_COMPILER_IS_GNUCC)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
	endif(CMAKE_COMPILER_IS_GNUCC)
	if(CMAKE_COMPILER_IS_GNUCXX)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
	endif(CMAKE_COMPILER_IS_GNUCXX)
endif(Debug)

option(NoWarnings "Disable compiler warnings" ON)
if (NoWarnings)
	if(CMAKE_COMPILER_IS_GNUCC)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w")
	endif(CMAKE_COMPILER_IS_GNUCC)
	if(CMAKE_COMPILER_IS_GNUCXX)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
	endif(CMAKE_COMPILER_IS_GNUCXX)
else(NoWarnings)
	if(CMAKE_COMPILER_IS_GNUCC)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
	endif(CMAKE_COMPILER_IS_GNUCC)
	if(CMAKE_COMPILER_IS_GNUCXX)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
	endif(CMAKE_COMPILER_IS_GNUCXX)
endif(NoWarnings)
