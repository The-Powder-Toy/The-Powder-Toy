#if defined(USE_SDL) && defined(_MSC_VER) && (_MSC_VER >= 1900)

#include <cstdio>

FILE _iob[] = { *stdin, *stdout, *stderr };
extern "C" FILE * __cdecl __iob_func(void)
{
	return _iob;
}

#endif
