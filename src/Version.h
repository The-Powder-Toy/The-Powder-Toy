#ifndef VERSION_H
#define VERSION_H

//VersionInfoStart
#ifndef SAVE_VERSION
#define SAVE_VERSION 85
#endif

#ifndef MINOR_VERSION
#define MINOR_VERSION 0
#endif

#ifndef BUILD_NUM
#define BUILD_NUM 254
#endif

#ifndef SNAPSHOT_ID
#define SNAPSHOT_ID 0
#endif

#ifndef STABLE
#ifndef BETA
#define BETA
#define SNAPSHOT
#endif
#endif
//VersionInfoEnd

//#define IGNORE_UPDATES //uncomment this for mods, to not get any update notifications

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

#define IDENT_VERSION "G"

#endif
