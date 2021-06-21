#ifndef CONFIG_H
#define CONFIG_H

#mesondefine CURL_STATICLIB
#mesondefine ZLIB_WINAPI

#mesondefine LUACONSOLE
#mesondefine NOHTTP
#mesondefine GRAVFFT
#mesondefine RENDERER
#mesondefine FONTEDITOR

#mesondefine BETA
#mesondefine DEBUG
#mesondefine IGNORE_UPDATES
#mesondefine LIN
#mesondefine NATIVE
#mesondefine NO_INSTALL_CHECK
#mesondefine OGLI
#mesondefine OGLR
#mesondefine PIX32OGL
#mesondefine SNAPSHOT
#mesondefine WIN
#mesondefine MACOSX
#mesondefine X86
#mesondefine X86_SSE
#mesondefine X86_SSE2
#mesondefine X86_SSE3
#mesondefine _64BIT
#mesondefine SERVER
#mesondefine STATICSERVER
#mesondefine UPDATESERVER

#ifdef WIN
# define PATH_SEP "\\"
# define PATH_SEP_CHAR '\\'
#else
# define PATH_SEP "/"
# define PATH_SEP_CHAR '/'
#endif

//VersionInfoStart
#define SAVE_VERSION 95
#define MINOR_VERSION 0
#define BUILD_NUM 345
#mesondefine SNAPSHOT_ID
#mesondefine MOD_ID
#define FUTURE_SAVE_VERSION 96
#define FUTURE_MINOR_VERSION 0

#if !(defined(SNAPSHOT) || defined(BETA) || defined(DEBUG) || MOD_ID > 0)
#undef FUTURE_SAVE_VERSION
#undef FUTURE_MINOR_VERSION
#endif
//VersionInfoEnd

#if !(defined(MACOSX) && defined(DEBUG))
#define HIGH_QUALITY_RESAMPLE			//High quality image resampling, slower but much higher quality than my terribad linear interpolation
#endif

#if defined(SNAPSHOT)
#define IDENT_RELTYPE "S"
#elif defined(BETA)
#define IDENT_RELTYPE "B"
#else
#define IDENT_RELTYPE "R"
#endif

#if defined(WIN)
#if defined(_64BIT)
#define IDENT_PLATFORM "WIN64"
#else
#define IDENT_PLATFORM "WIN32"
#endif
#elif defined(LIN)
#if defined(_64BIT)
#define IDENT_PLATFORM "LIN64"
#else
#define IDENT_PLATFORM "LIN32"
#endif
#elif defined(MACOSX)
#define IDENT_PLATFORM "MACOSX"
#else
#define IDENT_PLATFORM "UNKNOWN"
#endif

#if defined(X86_SSE3)
#define IDENT_BUILD "SSE3"
#elif defined(X86_SSE2)
#define IDENT_BUILD "SSE2"
#elif defined(X86_SSE)
#define IDENT_BUILD "SSE"
#else
#define IDENT_BUILD "NO"
#endif

#define MTOS_EXPAND(str) #str
#define MTOS(str) MTOS_EXPAND(str)

#define SCHEME "https://"
#define STATICSCHEME "https://"
#define ENFORCE_HTTPS

#define LOCAL_SAVE_DIR "Saves"

#define STAMPS_DIR "stamps"

#define BRUSH_DIR "Brushes"

#ifndef M_GRAV
#define M_GRAV 6.67300e-1
#endif

#ifdef RENDERER
#define MENUSIZE 0
#define BARSIZE 0
#else
#define MENUSIZE 40
#define BARSIZE 17
#endif
#define XRES	612
#define YRES	384
#define NPART XRES*YRES

#define XCNTR   XRES/2
#define YCNTR   YRES/2

#define WINDOWW (XRES+BARSIZE)
#define WINDOWH (YRES+MENUSIZE)

#define GRAV_DIFF

#define MAXSIGNS 16

//CELL, the size of the pressure, gravity, and wall maps. Larger than 1 to prevent extreme lag
#define CELL	4
#define ISTP	(CELL/2)
#define CFDS	(4.0f/CELL)
#define SIM_MAXVELOCITY 1e4f

//Air constants
#define AIR_TSTEPP 0.3f
#define AIR_TSTEPV 0.4f
#define AIR_VADV 0.3f
#define AIR_VLOSS 0.999f
#define AIR_PLOSS 0.9999f

#define NGOL 24

#define CIRCLE_BRUSH 0
#define SQUARE_BRUSH 1
#define TRI_BRUSH 2
#define BRUSH_NUM 3

//Photon constants
#define SURF_RANGE		10
#define NORMAL_MIN_EST	3
#define NORMAL_INTERP	20
#define NORMAL_FRAC		16

#define REFRACT			0x80000000

/* heavy flint glass, for awesome refraction/dispersion
   this way you can make roof prisms easily */
#define GLASS_IOR		1.9
#define GLASS_DISP		0.07

#define SDEUT

#endif /* CONFIG_H */
