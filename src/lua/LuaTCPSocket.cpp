#include "LuaTCPSocket.h"

#ifndef NOHTTP
# include "common/String.h"
# include <curl/curl.h>
# include <vector>
# include <stdexcept>
# include <cstring>
#endif
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

void SetupCurlEasyCiphers(CURL *easy);

namespace LuaTCPSocket
{
	static double Now()
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

	static void Timeout(double timeout)
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

#ifndef NOHTTP
	enum Status
	{
		StatusReady,
		StatusConnecting,
		StatusConnected,
		StatusDead,
	};

	struct TCPSocket
	{
		CURL *easy;
		CURLM *multi;
		char errorBuf[CURL_ERROR_SIZE];
		Status status;
		bool timeoutIndefinite;
		bool blocking;
		double timeout;
		std::vector<char> recvBuf;
		size_t stashedLen;
		bool readClosed;
		bool writeClosed;
	};

	static void Reset(TCPSocket *tcps)
	{
		if (tcps->multi)
		{
			curl_multi_remove_handle(tcps->multi, tcps->easy);
			curl_multi_cleanup(tcps->multi);
			tcps->multi = nullptr;
		}
		if (tcps->easy)
		{
			curl_easy_cleanup(tcps->easy);
			tcps->easy = nullptr;
		}
		tcps->status = StatusDead;
	}

	static bool ConnectPerform(TCPSocket *tcps, CURLcode *res)
	{
		while (true)
		{
			int dontcare;
			auto mres = curl_multi_perform(tcps->multi, &dontcare);
			struct CURLMsg *msg;
			while ((msg = curl_multi_info_read(tcps->multi, &dontcare)))
			{
				if (msg->msg == CURLMSG_DONE)
				{
					*res = msg->data.result;
					return true;
				}
			};
			if (mres != CURLM_CALL_MULTI_PERFORM)
			{
				break;
			}
		}
		return false;
	}

	static int New(lua_State *l)
	{
		auto *tcps = (TCPSocket *)lua_newuserdata(l, sizeof(TCPSocket));
		new (tcps) TCPSocket;
		tcps->errorBuf[0] = 0;
		tcps->easy = curl_easy_init();
		tcps->status = StatusReady;
		tcps->timeoutIndefinite = true;
		tcps->blocking = true;
		tcps->stashedLen = 0;
		tcps->readClosed = false;
		tcps->writeClosed = false;
		if (!tcps->easy)
		{
			Reset(tcps);
			return luaL_error(l, "curl_easy_init failed");
		}
		tcps->multi = curl_multi_init();
		if (!tcps->multi)
		{
			Reset(tcps);
			return luaL_error(l, "curl_multi_init failed");
		}
		curl_multi_add_handle(tcps->multi, tcps->easy);
		luaL_newmetatable(l, "TCPSocket");
		lua_setmetatable(l, -2);
		return 1;
	}

	static int GC(lua_State *l)
	{
		auto *tcps = (TCPSocket *)luaL_checkudata(l, 1, "TCPSocket");
		Reset(tcps);
		tcps->~TCPSocket();
		return 0;
	}

	static int Close(lua_State *l)
	{
		auto *tcps = (TCPSocket *)luaL_checkudata(l, 1, "TCPSocket");
		Reset(tcps);
		return 0;
	}

	static int Send(lua_State *l)
	{
		auto *tcps = (TCPSocket *)luaL_checkudata(l, 1, "TCPSocket");
		if (tcps->status != StatusConnected)
		{
			return luaL_error(l, "attempt to send on socket while not connected");
		}
		size_t dlenu;
		auto *data = luaL_checklstring(l, 2, &dlenu);
		auto dlen = int(dlenu);
		auto first = luaL_optinteger(l, 3, 1);
		auto last = luaL_optinteger(l, 4, -1);
		if (first < 0) first += dlen + 1;
		if (last  < 0) last  += dlen + 1;
		if (first <    1) first =    1;
		if (first > dlen) first = dlen;
		if (last  <    1) last  =    1;
		if (last  > dlen) last  = dlen;
		auto begin = first - 1;
		auto end = last;
		data += begin;
		auto len = size_t((end > begin) ? (end - begin) : 0);
		auto startedAt = Now();
		size_t writtenTotal = 0;
		while (true)
		{
			size_t writtenNow = 0;
			CURLcode res = CURLE_OK;
			if (!tcps->writeClosed)
			{
				res = curl_easy_send(tcps->easy, &data[0] + writtenTotal, len - writtenTotal, &writtenNow);
			}
			writtenTotal += writtenNow;
			if (writtenTotal >= len)
			{
				break;
			}
			switch (res)
			{
			case CURLE_OK:
			case CURLE_AGAIN:
				break;

			default:
				tcps->writeClosed = true;
				break;
			}
			if (tcps->writeClosed)
			{
				Reset(tcps);
				lua_pushnil(l);
				lua_pushliteral(l, "closed");
				lua_pushinteger(l, writtenTotal + begin);
				return 3;
			}
			if (res == CURLE_AGAIN)
			{
				if (tcps->blocking && (tcps->timeoutIndefinite || startedAt + tcps->timeout > Now()))
				{
					// * Using a platform-dependent primitive to "wait" on the socket is
					//   not worth the trouble: sockets shouldn't be used with timeouts
					//   anyway because they freeze the game.
					Timeout(0.01);
					continue;
				}
				lua_pushnil(l);
				lua_pushliteral(l, "timeout");
				lua_pushinteger(l, writtenTotal + begin);
				return 3;
			}
		}
		lua_pushinteger(l, writtenTotal + begin);
		return 1;
	}

	static int ReceiveNoPrefix(lua_State *l)
	{
		auto *tcps = (TCPSocket *)luaL_checkudata(l, 1, "TCPSocket");
		if (tcps->status != StatusConnected)
		{
			return luaL_error(l, "attempt to receive on socket while not connected");
		}
		enum
		{
			readN,
			readAll,
			readLine,
		} pattern = readN;
		size_t len = 4096;
		if (tpt_lua_equalsLiteral(l, 2, "*a"))
		{
			pattern = readAll;
		}
		else if (tpt_lua_equalsLiteral(l, 2, "*l"))
		{
			pattern = readLine;
		}
		else
		{
			len = size_t(luaL_checkinteger(l, 2));
		}
		if (pattern == readAll || pattern == readLine)
		{
			len += tcps->stashedLen;
		}
		if (tcps->recvBuf.size() < len)
		{
			tcps->recvBuf.resize(len);
		}
		auto startedAt = Now();
		size_t readTotal = 0;
		size_t returning = 0;
		int retn = 1;
		while (true)
		{
			size_t readNow = 0;
			CURLcode res = CURLE_OK;
			if (tcps->stashedLen)
			{
				readNow = tcps->stashedLen;
				tcps->stashedLen = 0;
			}
			else
			{
				res = curl_easy_recv(tcps->easy, &tcps->recvBuf[0] + readTotal, len - readTotal, &readNow);
			}
			readTotal += readNow;
			returning = readTotal;
			if (pattern == readN && readTotal >= len)
			{
				returning = len;
				break;
			}
			if (pattern == readLine)
			{
				bool foundLF = false;
				for (size_t i = 0; i < readTotal; ++i)
				{
					if (tcps->recvBuf[i] == '\n')
					{
						returning = i + 1;
						foundLF = true;
						break;
					}
				}
				if (foundLF)
				{
					break;
				}
			}
			switch (res)
			{
			case CURLE_OK:
				if (!readNow)
				{
					tcps->readClosed = true;
				}
				break;

			case CURLE_AGAIN:
				break;

			default:
				tcps->readClosed = true;
				break;
			}
			if (tcps->readClosed)
			{
				Reset(tcps);
				if (pattern == readAll)
				{
					// * Closed "*a" patterns don't return an error.
					break;
				}
				lua_pushnil(l);
				lua_pushliteral(l, "closed");
				if (pattern == readLine)
				{
					// * Closed "*l" patterns don't return partial lines.
					returning = 0;
				}
				retn = 3;
				break;
			}
			if (readTotal == len && (pattern == readAll || pattern == readLine))
			{
				len += len;
				if (tcps->recvBuf.size() < len)
				{
					tcps->recvBuf.resize(len);
				}
			}
			if (res == CURLE_AGAIN)
			{
				if (tcps->blocking && (tcps->timeoutIndefinite || startedAt + tcps->timeout > Now()))
				{
					// * Using a platform-dependent primitive to "wait" on the socket is
					//   not worth the trouble: sockets shouldn't be used with timeouts
					//   anyway because they freeze the game.
					Timeout(0.01);
					continue;
				}
				lua_pushnil(l);
				lua_pushliteral(l, "timeout");
				if (pattern == readLine)
				{
					// * Timed-out "*l" patterns don't return partial lines.
					returning = 0;
				}
				retn = 3;
				break;
			}
		}
		tcps->stashedLen = readTotal - returning;
		if (pattern == readLine)
		{
			auto curOut = 0U;
			for (auto curIn = 0U; curIn < returning; ++curIn)
			{
				if (tcps->recvBuf[curIn] != '\r' && tcps->recvBuf[curIn] != '\n')
				{
					tcps->recvBuf[curOut] = tcps->recvBuf[curIn];
					curOut += 1;
				}
			}
			returning = curOut;
		}
		lua_pushlstring(l, &tcps->recvBuf[0], returning);
		// * This copy makes ReceiveNoPrefix quadratic if there's a lot of stuff in
		//   the stash (as a result of a *very* long line being returned by an "*l"
		//   pattern and then whatever was left being stashed) and it's all *very*
		//   short lines (compared to the previous *very* long one, from the point
		//   of view of an "*l" pattern). Handling this edge case in a special,
		//   sub-quadratic way isn't worth the effort.
		std::copy(
			&tcps->recvBuf[0] + readTotal - tcps->stashedLen,
			&tcps->recvBuf[0] + readTotal,
			&tcps->recvBuf[0]
		);
		return retn;
	}

	static int Receive(lua_State *l)
	{
		bool prefix = false;
		if (lua_gettop(l) >= 3)
		{
			prefix = true;
			lua_tostring(l, 3);
		}
		int ret = ReceiveNoPrefix(l);
		if (prefix)
		{
			lua_pushvalue(l, 3);
			lua_insert(l, -2);
			lua_concat(l, 2);
		}
		return ret;
	}

	static int Connect(lua_State *l)
	{
		auto *tcps = (TCPSocket *)luaL_checkudata(l, 1, "TCPSocket");
		if (tcps->status == StatusDead)
		{
			return luaL_error(l, "attempt to connect dead socket");
		}
		if (tcps->status == StatusConnected)
		{
			return luaL_error(l, "attempt to connect connected socket");
		}
		auto startedAt = Now();
		while (true)
		{
			if (tcps->status != StatusConnecting)
			{
				tcps->status = StatusConnecting;
				// * Using CURLPROTO_HTTPS and CURLPROTO_HTTP with CURL_HTTP_VERSION_1_0
				//   because these really don't send anything while connecting if
				//   CURLOPT_CONNECT_ONLY is 1 and there are no proxies involved. The
				//   only ugly bit is that we have to prepend http:// or https:// to
				//   the hostnames.
				curl_easy_setopt(tcps->easy, CURLOPT_ERRORBUFFER, tcps->errorBuf);
				curl_easy_setopt(tcps->easy, CURLOPT_CONNECT_ONLY, 1L);
				ByteString address = tpt_lua_checkByteString(l, 2);
				curl_easy_setopt(tcps->easy, CURLOPT_PORT, long(luaL_checkinteger(l, 3)));
				curl_easy_setopt(tcps->easy, CURLOPT_NOSIGNAL, 1L);
				curl_easy_setopt(tcps->easy, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
				if (lua_toboolean(l, 4))
				{
#if defined(CURL_AT_LEAST_VERSION) && CURL_AT_LEAST_VERSION(7, 85, 0)
					curl_easy_setopt(tcps->easy, CURLOPT_PROTOCOLS_STR, "https");
					curl_easy_setopt(tcps->easy, CURLOPT_REDIR_PROTOCOLS_STR, "https");
#else
					curl_easy_setopt(tcps->easy, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);
					curl_easy_setopt(tcps->easy, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTPS);
#endif
					SetupCurlEasyCiphers(tcps->easy);
					address = "https://" + address;
				}
				else
				{
#if defined(CURL_AT_LEAST_VERSION) && CURL_AT_LEAST_VERSION(7, 85, 0)
					curl_easy_setopt(tcps->easy, CURLOPT_PROTOCOLS_STR, "http");
					curl_easy_setopt(tcps->easy, CURLOPT_REDIR_PROTOCOLS_STR, "http");
#else
					curl_easy_setopt(tcps->easy, CURLOPT_PROTOCOLS, CURLPROTO_HTTP);
					curl_easy_setopt(tcps->easy, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTP);
#endif
					address = "http://" + address;
				}
				curl_easy_setopt(tcps->easy, CURLOPT_URL, address.c_str());
			}
			CURLcode res;
			if (!ConnectPerform(tcps, &res))
			{
				if (tcps->blocking && (tcps->timeoutIndefinite || startedAt + tcps->timeout > Now()))
				{
					// * Using a platform-dependent primitive to "wait" on the socket is
					//   not worth the trouble: sockets shouldn't be used with timeouts
					//   anyway because they freeze the game.
					Timeout(0.01);
					continue;
				}
				lua_pushnil(l);
				lua_pushliteral(l, "timeout");
				return 2;
			}
			if (res != CURLE_OK)
			{
				Reset(tcps);
				lua_pushnil(l);
				lua_pushstring(l, tcps->errorBuf);
				return 2;
			}
			break;
		}
		tcps->status = StatusConnected;
		lua_pushinteger(l, 1);
		return 1;
	}

	static int LastError(lua_State *l)
	{
		auto *tcps = (TCPSocket *)luaL_checkudata(l, 1, "TCPSocket");
		lua_pushstring(l, tcps->errorBuf);
		return 1;
	}

	static int GetStatus(lua_State *l)
	{
		auto *tcps = (TCPSocket *)luaL_checkudata(l, 1, "TCPSocket");
		switch (tcps->status)
		{
		case StatusReady:      lua_pushliteral(l,      "ready"); break;
		case StatusConnecting: lua_pushliteral(l, "connecting"); break;
		case StatusConnected:  lua_pushliteral(l,  "connected"); break;
		case StatusDead:       lua_pushliteral(l,       "dead"); break;
		}
		return 1;
	}

	static int GetPeerName(lua_State *l)
	{
		auto *tcps = (TCPSocket *)luaL_checkudata(l, 1, "TCPSocket");
		if (tcps->status != StatusConnected)
		{
			return luaL_error(l, "attempt to get remote socket info while not connected");
		}
		char *address;
		curl_easy_getinfo(tcps->easy, CURLINFO_PRIMARY_IP, &address);
		lua_pushstring(l, address);
		long port;
		curl_easy_getinfo(tcps->easy, CURLINFO_PRIMARY_PORT, &port);
		lua_pushinteger(l, port);
		return 2;
	}

	static int SetTimeout(lua_State *l)
	{
		auto *tcps = (TCPSocket *)luaL_checkudata(l, 1, "TCPSocket");
		tcps->blocking = true;
		if (lua_isnoneornil(l, 2))
		{
			tcps->timeoutIndefinite = true;
		}
		else
		{
			tcps->timeoutIndefinite = false;
			tcps->timeout = luaL_checknumber(l, 2);
			if (int(tcps->timeout) == 0)
			{
				tcps->blocking = false;
			}
		}
		return 0;
	}

	static int GetSockName(lua_State *l)
	{
		auto *tcps = (TCPSocket *)luaL_checkudata(l, 1, "TCPSocket");
		if (tcps->status != StatusConnected)
		{
			return luaL_error(l, "attempt to get local socket info while not connected");
		}
		char *address;
		curl_easy_getinfo(tcps->easy, CURLINFO_LOCAL_IP, &address);
		lua_pushstring(l, address);
		long port;
		curl_easy_getinfo(tcps->easy, CURLINFO_LOCAL_PORT, &port);
		lua_pushinteger(l, port);
		return 2;
	}

	static int SetOption(lua_State *l)
	{
		auto *tcps = (TCPSocket *)luaL_checkudata(l, 1, "TCPSocket");
		auto option = tpt_lua_checkByteString(l, 2);
		if (byteStringEqualsLiteral(option, "keepalive"))
		{
			curl_easy_setopt(tcps->easy, CURLOPT_TCP_KEEPALIVE, long(lua_toboolean(l, 3)));
			return 0;
		}
		else if (byteStringEqualsLiteral(option, "tcp-nodelay"))
		{
			curl_easy_setopt(tcps->easy, CURLOPT_TCP_NODELAY, long(lua_toboolean(l, 3)));
			return 0;
		}
		else if (byteStringEqualsLiteral(option, "verify-peer"))
		{
			curl_easy_setopt(tcps->easy, CURLOPT_SSL_VERIFYPEER, long(lua_toboolean(l, 3)));
			return 0;
		}
		return luaL_error(l, "unknown option");
	}

	static int Shutdown(lua_State *l)
	{
		auto *tcps = (TCPSocket *)luaL_checkudata(l, 1, "TCPSocket");
		auto direction = tpt_lua_optByteString(l, 2, "both");
		if (byteStringEqualsLiteral(direction, "receive"))
		{
			tcps->readClosed = true;
			return 0;
		}
		else if (byteStringEqualsLiteral(direction, "send"))
		{
			tcps->writeClosed = true;
			return 0;
		}
		else if (byteStringEqualsLiteral(direction, "both"))
		{
			tcps->readClosed = true;
			tcps->writeClosed = true;
			return 0;
		}
		return luaL_error(l, "unknown direction");
	}
#endif

	void Open(lua_State *l)
	{
#ifndef NOHTTP
		luaL_newmetatable(l, "TCPSocket");
		lua_pushcfunction(l, LuaTCPSocket::GC);
		lua_setfield(l, -2, "__gc");
		lua_newtable(l);
		struct luaL_Reg tcpSocketIndexMethods[] = {
			{     "connect", LuaTCPSocket::Connect     },
			{       "close", LuaTCPSocket::Close       },
			{        "send", LuaTCPSocket::Send        },
			{     "receive", LuaTCPSocket::Receive     },
			{   "lasterror", LuaTCPSocket::LastError   },
			{      "status", LuaTCPSocket::GetStatus   },
			{ "getpeername", LuaTCPSocket::GetPeerName },
			{ "getsockname", LuaTCPSocket::GetSockName },
			{  "settimeout", LuaTCPSocket::SetTimeout  },
			{   "setoption", LuaTCPSocket::SetOption   },
			{    "shutdown", LuaTCPSocket::Shutdown    },
			{          NULL, NULL                      },
		};
		luaL_register(l, NULL, tcpSocketIndexMethods);
		lua_setfield(l, -2, "__index");
		lua_pop(l, 1);
#endif
		lua_newtable(l);
		struct luaL_Reg socketMethods[] = {
#ifndef NOHTTP
			{     "tcp", LuaTCPSocket::New     },
#endif
			{   "sleep", LuaTCPSocket::Sleep   },
			{ "gettime", LuaTCPSocket::GetTime },
			{      NULL, NULL                  },
		};
		luaL_register(l, NULL, socketMethods);
		lua_setglobal(l, "socket");
	}
}
