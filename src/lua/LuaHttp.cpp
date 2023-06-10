#include "LuaHttp.h"
#include "client/http/Request.h"
#include "client/Client.h"
#include "json/json.h"
#include "LuaScriptInterface.h"
#include "Format.h"
#include "Config.h"
#include <memory>
#include <iostream>

class RequestHandle
{
public:
	enum RequestType
	{
		normal,
		getAuthToken,
	};

private:
	std::unique_ptr<http::Request> request;
	bool dead = false;
	RequestType type;

	RequestHandle() = default;

	std::pair<int, ByteString> FinishGetAuthToken(ByteString data)
	{
		std::istringstream ss(data);
		Json::Value root;
		try
		{
			ss >> root;
			auto status = root["Status"].asString();
			if (status == "OK")
			{
				return { 200, root["Token"].asString() };
			}
			return { 403, status };
		}
		catch (std::exception &e)
		{
			std::cerr << "bad auth response: " << e.what() << std::endl;
		}
		return { 600, {} };
	}

public:
	static int Make(lua_State *l, const ByteString &uri, bool isPost, const ByteString &verb, RequestType type, const http::PostData &postData, const std::vector<ByteString> &headers)
	{
		auto authUser = Client::Ref().GetAuthUser();
		if (type == getAuthToken && !authUser.UserID)
		{
			lua_pushnil(l);
			lua_pushliteral(l, "not authenticated");
			return 2;
		}
		auto *rh = (RequestHandle *)lua_newuserdata(l, sizeof(RequestHandle));
		if (!rh)
		{
			return 0;
		}
		new(rh) RequestHandle();
		rh->type = type;
		rh->request = std::make_unique<http::Request>(uri);
		if (verb.size())
		{
			rh->request->Verb(verb);
		}
		for (auto &header : headers)
		{
			rh->request->AddHeader(header);
		}
		if (isPost)
		{
			rh->request->AddPostData(postData);
		}
		if (type == getAuthToken)
		{
			rh->request->AuthHeaders(ByteString::Build(authUser.UserID), authUser.SessionID);
		}
		rh->request->Start();
		luaL_newmetatable(l, "HTTPRequest");
		lua_setmetatable(l, -2);
		return 1;
	}

	~RequestHandle()
	{
		if (!Dead())
		{
			Cancel();
		}
	}

	bool Dead() const
	{
		return dead;
	}

	bool Done() const
	{
		return request->CheckDone();
	}

	void Progress(int *total, int *done)
	{
		if (!dead)
		{
			std::tie(*total, *done) = request->CheckProgress();
		}
	}

	void Cancel()
	{
		if (!dead)
		{
			request.reset();
			dead = true;
		}
	}

	std::pair<int, ByteString> Finish(std::vector<ByteString> &headers)
	{
		int status = 0;
		ByteString data;
		if (!dead)
		{
			if (request->CheckDone())
			{
				if (type != getAuthToken)
				{
					headers = request->ResponseHeaders();
				}
				// Get this separately so it's always present.
				status = request->StatusCode();
				try
				{
					data = request->Finish().second;
				}
				catch (const http::RequestError &ex)
				{
					// Nothing, the only way to fail here is to fail in RequestManager, and
					// that means the problem has already been printed to std::cerr.
				}
				request.reset();
				if (type == getAuthToken)
				{
					if (status == 200)
					{
						std::tie(status, data) = FinishGetAuthToken(data);
					}
				}
				dead = true;
			}
		}
		return { status, data };
	}
};

static int http_request_gc(lua_State *l)
{
	auto *rh = (RequestHandle *)luaL_checkudata(l, 1, "HTTPRequest");
	rh->~RequestHandle();
	return 0;
}

static int http_request_status(lua_State *l)
{
	auto *rh = (RequestHandle *)luaL_checkudata(l, 1, "HTTPRequest");
	if (rh->Dead())
	{
		lua_pushliteral(l, "dead");
	}
	else if (rh->Done())
	{
		lua_pushliteral(l, "done");
	}
	else
	{
		lua_pushliteral(l, "running");
	}
	return 1;
}

static int http_request_progress(lua_State *l)
{
	auto *rh = (RequestHandle *)luaL_checkudata(l, 1, "HTTPRequest");
	if (!rh->Dead())
	{
		int total, done;
		rh->Progress(&total, &done);
		lua_pushinteger(l, total);
		lua_pushinteger(l, done);
		return 2;
	}
	return 0;
}

static int http_request_cancel(lua_State *l)
{
	auto *rh = (RequestHandle *)luaL_checkudata(l, 1, "HTTPRequest");
	if (!rh->Dead())
	{
		rh->Cancel();
	}
	return 0;
}

static int http_request_finish(lua_State *l)
{
	auto *rh = (RequestHandle *)luaL_checkudata(l, 1, "HTTPRequest");
	if (!rh->Dead())
	{
		std::vector<ByteString> headers;
		auto [ status, data ] = rh->Finish(headers);
		tpt_lua_pushByteString(l, data);
		lua_pushinteger(l, status);
		lua_newtable(l);
		for (auto i = 0; i < int(headers.size()); ++i)
		{
			lua_pushlstring(l, headers[i].data(), headers[i].size());
			lua_rawseti(l, -2, i + 1);
		}
		return 3;
	}
	return 0;
}

static int http_request(lua_State *l, bool isPost)
{
	ByteString uri = tpt_lua_checkByteString(l, 1);
	http::PostData postData;
	auto headersIndex = 2;
	auto verbIndex = 3;

	if (isPost)
	{
		headersIndex += 1;
		verbIndex += 1;
		if (lua_isstring(l, 2))
		{
			postData = tpt_lua_toByteString(l, 2);
		}
		else if (lua_istable(l, 2))
		{
			postData = http::FormData{};
			auto &formData = std::get<http::FormData>(postData);
			lua_pushnil(l);
			while (lua_next(l, 2))
			{
				lua_pushvalue(l, -2);
				formData.emplace(tpt_lua_toByteString(l, -1), tpt_lua_toByteString(l, -2));
				lua_pop(l, 2);
			}
		}
	}

	std::vector<ByteString> headers;
	if (lua_istable(l, headersIndex))
	{
		auto size = lua_objlen(l, headersIndex);
		if (size)
		{
			for (auto i = 0U; i < size; ++i)
			{
				lua_rawgeti(l, headersIndex, i + 1);
				headers.push_back(tpt_lua_toByteString(l, -1));
				lua_pop(l, 1);
			}
		}
		else
		{
			// old dictionary format
			lua_pushnil(l);
			while (lua_next(l, headersIndex))
			{
				lua_pushvalue(l, -2);
				headers.push_back(tpt_lua_toByteString(l, -1) + ByteString(": ") + tpt_lua_toByteString(l, -2));
				lua_pop(l, 2);
			}
		}
	}

	auto verb = tpt_lua_optByteString(l, verbIndex, "");
	return RequestHandle::Make(l, uri, isPost, verb, RequestHandle::normal, postData, headers);
}

static int http_get_auth_token(lua_State *l)
{
	return RequestHandle::Make(l, ByteString::Build(SCHEME, SERVER, "/ExternalAuth.api?Action=Get&Audience=", format::URLEncode(tpt_lua_checkByteString(l, 1))), false, {}, RequestHandle::getAuthToken, {}, {});
}

static int http_get(lua_State * l)
{
	return http_request(l, false);
}

static int http_post(lua_State * l)
{
	return http_request(l, true);
}

void LuaHttp::Open(lua_State *l)
{
	luaL_newmetatable(l, "HTTPRequest");
	lua_pushcfunction(l, http_request_gc);
	lua_setfield(l, -2, "__gc");
	lua_newtable(l);
	struct luaL_Reg httpRequestIndexMethods[] = {
		{ "status", http_request_status },
		{ "progress", http_request_progress },
		{ "cancel", http_request_cancel },
		{ "finish", http_request_finish },
		{ NULL, NULL }
	};
	luaL_register(l, NULL, httpRequestIndexMethods);
	lua_setfield(l, -2, "__index");
	lua_pop(l, 1);
	lua_newtable(l);
	struct luaL_Reg httpMethods[] = {
		{ "get", http_get },
		{ "post", http_post },
		{ "getAuthToken", http_get_auth_token },
		{ NULL, NULL }
	};
	luaL_register(l, NULL, httpMethods);
	lua_setglobal(l, "http");
}
