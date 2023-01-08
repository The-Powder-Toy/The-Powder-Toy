#include "LuaSocket.h"

#include <stdint.h>
#include <algorithm>
#ifdef WIN
# include <windows.h>
# include <float.h>
#else
# include <sys/time.h>
# include <time.h>
#endif

#include "LuaScriptInterface.h"
#include "Misc.h"

namespace LuaSocket
{
	double Now()
	{
#ifdef WIN
		FILETIME rt;
		GetSystemTimeAsFileTime(&rt);
		return (rt.dwLowDateTime + (uint64_t(rt.dwHighDateTime) << 32) - uint64_t(116444736000000000ULL)) / 1e7;
#else
		struct timeval rt;
		gettimeofday(&rt, (struct timezone *)NULL);
		return rt.tv_sec + rt.tv_usec / 1e6;
#endif
	}

	static int GetTime(lua_State *l)
	{
		lua_pushnumber(l, Now());
		return 1;
	}

	void Timeout(double timeout)
	{
#ifdef WIN
		if (timeout <              0.0) timeout  =     0.0;
		if (timeout < DBL_MAX / 1000.0) timeout *=  1000.0;
		if (timeout >          INT_MAX) timeout  = INT_MAX;
		::Sleep(int(timeout));
#else
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
#endif
	}

	static int Sleep(lua_State *l)
	{
		Timeout(luaL_checknumber(l, 1));
		return 0;
	}

	void Open(lua_State *l)
	{
		lua_newtable(l);
		struct luaL_Reg socketMethods[] = {
			{   "sleep", LuaSocket::Sleep   },
			{ "gettime", LuaSocket::GetTime },
			{      NULL, NULL                  },
		};
		luaL_register(l, NULL, socketMethods);
		lua_setglobal(l, "socket");
		OpenTCP(l);
	}
}
