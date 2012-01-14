#pragma once


/* ***** Platform-ness ***** */

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32_LEAN_AND_MEAN)
# define IEF_PLATFORM_WIN32
# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
# endif
#elif defined(linux) || defined(_linux) || defined(__linux)
# define IEF_PLATFORM_LINUX

#elif defined(__APPLE__) || defined(MACOSX) || defined(macintosh) || defined(Macintosh)
# define IEF_PLATFORM_MACOSX

//#elif defined(__FreeBSD__) || define(__FreeBSD_kernel__)
//# define IEF_PLATFORM_FREEBSD

#else
# error Operating System not supported.
#endif


/* ***** Endian-ness ***** */

#if defined(__m68k__) || defined(mc68000) || defined(_M_M68K) || (defined(__MIPS__) && defined(__MISPEB__)) || defined(__ppc__) || defined(__POWERPC__) || defined(_M_PPC) || defined(__sparc__) || defined(__hppa__)
# define IEF_ENDIAN_BIG

#else
# define IEF_ENDIAN_LITTLE
#endif


/* ***** Debug-ness ***** */

#if !defined(NDEBUG) || defined(_DEBUG)
# define IEF_DEBUG
#endif


/* ***** Primitive Types ***** */

#ifndef NULL
# define NULL 0
#endif

#include <climits>
namespace sys
{

#if UCHAR_MAX == 0xFF //char
 typedef signed char s8;
 typedef unsigned char u8;
#else
# error No 8-Bit Integer supported.
#endif
#if USHRT_MAX == 0xFFFF //short
 typedef signed short s16;
 typedef unsigned short u16;
#elif UINT_MAX == 0xFFFF
 typedef signed int s16;
 typedef unsigned int u16;
#elif ULONG_MAX == 0xFFFF
 typedef signed long s16;
 typedef unsigned long u16;
 #else
 # error No 16-Bit Integer supported.
 #endif
 #if USHRT_MAX == 0xFFFFFFFF //int
 typedef signed short s32;
 typedef unsigned short u32;
#elif UINT_MAX == 0xFFFFFFFF
 typedef signed int s32;
 typedef unsigned int u32;
#elif ULONG_MAX == 0xFFFFFFFF
 typedef signed long s32;
 typedef unsigned long u32;
 #else
 # error No 32-Bit Integer supported.
 #endif
#if UINT_MAX == 0xFFFFFFFFFFFFFFFF //long
 typedef signed int s64;
 typedef unsigned int u64;
#elif ULONG_MAX == 0xFFFFFFFFFFFFFFFF
 typedef signed long s64;
 typedef unsigned long u64;
#elif ULLONG_MAX == 0xFFFFFFFFFFFFFFFF
 typedef signed long long s64;
 typedef unsigned long long u64;
#else
# pragma message("Warning: 64-bit not supported. s64 and u64 defined as 32-bit.")
 typedef s32 s64;
 typedef u32 u64;
#endif
//floating
typedef float f32;
typedef double f64;
//misc
typedef u8 byte;
typedef u8 ubyte;
typedef s8 sbyte;
typedef s64 llong;
typedef s64 sllong;
typedef u64 ullong;
typedef char* cstring;

} //namespace sys
