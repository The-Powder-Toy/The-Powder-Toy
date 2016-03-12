#ifndef TPT_THREAD_H
#define TPT_THREAD_H

#if defined(WIN) && defined(__GNUC__)
#define TH_ENTRY_POINT __attribute__((force_align_arg_pointer)) 
#define _TIMESPEC_DEFINED
#else
#define TH_ENTRY_POINT
#endif

#include <pthread.h>
#undef GetUserName

#endif