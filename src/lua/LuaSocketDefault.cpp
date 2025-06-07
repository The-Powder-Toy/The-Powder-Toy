#include "LuaScriptInterface.h"
#include "Misc.h"
#include <stdint.h>
#include <algorithm>
#include <climits>
#include <sys/time.h>
#include <time.h>

namespace LuaSocket
{
	double Now()
	{
		struct timeval rt;
		gettimeofday(&rt, (struct timezone *)nullptr);
		return rt.tv_sec + rt.tv_usec / 1e6;
	}

	void Timeout(double timeout)
	{
		struct timespec req, rem;
		if (timeout <     0.0) timeout =     0.0;
		if (timeout > INT_MAX) timeout = INT_MAX;
		req.tv_sec = int(timeout);
		req.tv_nsec = int((timeout - req.tv_sec) * 1000000000);
		if (req.tv_nsec > 999999999) req.tv_nsec = 999999999;
		while (nanosleep(&req, &rem))
		{
			req.tv_sec = rem.tv_sec;
			req.tv_nsec = rem.tv_nsec;
		}
	}
}
