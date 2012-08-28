/*
 * Config.h
 *
 *  Created on: Jan 5, 2012
 *      Author: Simon
 */

//#ifndef CONFIG_H_
//#define CONFIG_H_


#ifdef WIN
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#endif

//VersionInfoStart
#ifndef SAVE_VERSION
#define SAVE_VERSION 83
#endif

#ifndef MINOR_VERSION
#define MINOR_VERSION 0
#endif

#ifndef BUILD_NUM
#define BUILD_NUM 155
#endif

#ifndef SNAPSHOT_ID
#define SNAPSHOT_ID 0
#endif

#ifndef BETA
#define BETA
#define SNAPSHOT
#endif
//VersionInfoEnd

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

#define IDENT_VERSION "G" //Change this if you're not Simon! It should be a single letter

#define MTOS_EXPAND(str) #str
#define MTOS(str) MTOS_EXPAND(str)

#define SERVER "powdertoy.co.uk"
#define SCRIPTSERVER "powdertoy.co.uk"
#define STATICSERVER "static.powdertoy.co.uk"

#define LOCAL_SAVE_DIR "Saves"

#define STAMPS_DIR "stamps"

#define APPDATA_SUBDIR "\\HardWIRED"

//Number of unique thumbnails to have in cache at one time
#define THUMB_CACHE_SIZE 256

#ifndef M_PI
#define M_PI 3.14159265f
#endif
#ifndef M_GRAV
#define M_GRAV 6.67300e-1
#endif

//Number of asynchronous connections used to retrieve thumnails
#define IMGCONNS 5
//Not sure
#define TIMEOUT 100
//HTTP request timeout in seconds
#define HTTP_TIMEOUT 10

#ifdef RENDERER
#define MENUSIZE 0
#define BARSIZE 0
#else
#define MENUSIZE 40
//#define MENUSIZE 20
//#define BARSIZE 50
#define BARSIZE 17
#endif
#define XRES	612
#define YRES	384
#define NPART XRES*YRES

#define XCNTR   306
#define YCNTR   192

#define MAX_DISTANCE sqrt(pow((float)XRES, 2)+pow((float)YRES, 2))

#define GRAV_DIFF

#define MAXSIGNS 16
#define TAG_MAX 256

#define ZSIZE_D	16
#define ZFACTOR_D	8
extern unsigned char ZFACTOR;
extern unsigned char ZSIZE;

#define CELL    4
#define ISTP    (CELL/2)
#define CFDS	(4.0f/CELL)

#define AIR_TSTEPP 0.3f
#define AIR_TSTEPV 0.4f
#define AIR_VADV 0.3f
#define AIR_VLOSS 0.999f
#define AIR_PLOSS 0.9999f

#define GRID_X 5
#define GRID_Y 4
#define GRID_P 3
#define GRID_S 6
#define GRID_Z 3

#define CATALOGUE_X 4
#define CATALOGUE_Y 3
#define CATALOGUE_S 6
#define CATALOGUE_Z 3

#define STAMP_MAX 240

#define SAVE_OPS

#define NGOL 24
#define NGOLALT 24 //NGOL should be 24, but use this var until I find out why

#define CIRCLE_BRUSH 0
#define SQUARE_BRUSH 1
#define TRI_BRUSH 2
#define BRUSH_NUM 3

#define SURF_RANGE     10
#define NORMAL_MIN_EST 3
#define NORMAL_INTERP  20
#define NORMAL_FRAC    16

#define REFRACT        0x80000000

/* heavy flint glass, for awesome refraction/dispersion
   this way you can make roof prisms easily */
#define GLASS_IOR      1.9
#define GLASS_DISP     0.07

#ifdef WIN
#define strcasecmp stricmp
#endif
#if defined(WIN) && !defined(__GNUC__)
#define fmin min
#define fminf min
#define fmax max
#define fmaxf max
#endif

#if defined(WIN) && !defined(__GNUC__)
#define TPT_INLINE _inline
#else
#define TPT_INLINE inline
#endif

#define SDEUT
//#define REALHEAT

#define DEBUG_PARTS		0x0001
#define DEBUG_PARTCOUNT	0x0002
#define DEBUG_DRAWTOOL	0x0004
#define DEBUG_PERFORMANCE_CALC 0x0008
#define DEBUG_PERFORMANCE_FRAME 0x0010

//#endif /* CONFIG_H_ */
