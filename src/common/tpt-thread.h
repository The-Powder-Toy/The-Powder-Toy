#ifndef TPT_THREAD_H
#define TPT_THREAD_H

#if defined(WIN) && defined(__GNUC__)
#define TH_ENTRY_POINT __attribute__((force_align_arg_pointer))
#define _TIMESPEC_DEFINED
#else
#define TH_ENTRY_POINT
#endif

// fix 'timespec' error in VS 2015
#if defined(_MSC_VER) && (_MSC_VER >= 1900)
#define _TIMESPEC_DEFINED 1
#endif

#include <pthread.h>
#undef GetUserName

// Fix deprecation warnings with recent pthread versions on Windows
#if defined(_PTW32_STATIC_LIB) && defined(WIN)
#if PTW32_VERSION <= 2, 8, 0, 0
#define PTW32_STATIC_LIB
#endif
#endif

#endif
