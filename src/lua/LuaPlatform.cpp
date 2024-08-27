#include "LuaScriptInterface.h"
#include "common/platform/Platform.h"
#include "Config.h"
#include "PowderToySDL.h"

static int platform(lua_State *L)
{
	tpt_lua_pushByteString(L, IDENT_PLATFORM);
	return 1;
}

static int ident(lua_State *L)
{
	tpt_lua_pushByteString(L, IDENT);
	return 1;
}

static int releaseType(lua_State *L)
{
	tpt_lua_pushByteString(L, ByteString(1, IDENT_RELTYPE));
	return 1;
}

static int exeName(lua_State *L)
{
	ByteString name = Platform::ExecutableName();
	if (name.length())
		tpt_lua_pushByteString(L, name);
	else
		luaL_error(L, "Error, could not get executable name");
	return 1;
}

static int restart(lua_State *L)
{
	Platform::DoRestart();
	return 0;
}

static int openLink(lua_State *L)
{
	auto uri = tpt_lua_checkByteString(L, 1);
	Platform::OpenURI(uri);
	return 0;
}

static int clipboardCopy(lua_State *L)
{
	tpt_lua_pushByteString(L, ClipboardPull());
	return 1;
}

static int clipboardPaste(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TSTRING);
	ClipboardPush(tpt_lua_optByteString(L, 1, ""));
	return 0;
}

void LuaPlatform::Open(lua_State *L)
{
	static const luaL_Reg reg[] = {
#define LFUNC(v) { #v, v }
		LFUNC(platform),
		LFUNC(ident),
		LFUNC(releaseType),
		LFUNC(exeName),
		LFUNC(restart),
		LFUNC(openLink),
		LFUNC(clipboardCopy),
		LFUNC(clipboardPaste),
#undef LFUNC
		{ NULL, NULL }
	};
	lua_newtable(L);
	luaL_register(L, NULL, reg);
	lua_pushvalue(L, -1);
	lua_setglobal(L, "platform");
	lua_setglobal(L, "plat");
}
