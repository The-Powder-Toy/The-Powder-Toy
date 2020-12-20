#pragma once

#ifdef DEBUG
# undef DEBUG
# define DEBUG 1
#else
# define DEBUG 0
#endif
#include <mach-o/dyld.h>
#include <ApplicationServices/ApplicationServices.h>
