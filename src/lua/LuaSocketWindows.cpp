#include "LuaScriptInterface.h"
#include "Misc.h"
#include <stdint.h>
#include <algorithm>
#include <windows.h>
#include <float.h>

namespace LuaSocket
{
	double Now()
	{
		FILETIME rt;
		GetSystemTimeAsFileTime(&rt);
		return (rt.dwLowDateTime + (uint64_t(rt.dwHighDateTime) << 32) - uint64_t(116444736000000000ULL)) / 1e7;
	}

	void Timeout(double timeout)
	{
		if (timeout <              0.0) timeout  =     0.0;
		if (timeout < DBL_MAX / 1000.0) timeout *=  1000.0;
		if (timeout >          INT_MAX) timeout  = INT_MAX;
		::Sleep(int(timeout));
	}
}
