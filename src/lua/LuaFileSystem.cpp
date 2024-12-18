#include "LuaScriptInterface.h"
#include "common/platform/Platform.h"

static int list(lua_State *L)
{
	auto directoryName = tpt_lua_checkByteString(L, 1);
	lua_newtable(L);
	int index = 0;
	for (auto &name : Platform::DirectorySearch(directoryName, "", {}))
	{
		if (name != "." && name != "..")
		{
			index += 1;
			tpt_lua_pushByteString(L, name);
			lua_rawseti(L, -2, index);
		}
	}
	return 1;
}

static int exists(lua_State *L)
{
	auto filename = tpt_lua_checkByteString(L, 1);
	bool ret = Platform::Stat(filename);
	lua_pushboolean(L, ret);
	return 1;
}

static int isFile(lua_State *L)
{
	auto filename = tpt_lua_checkByteString(L, 1);
	bool ret = Platform::FileExists(filename);
	lua_pushboolean(L, ret);
	return 1;
}

static int isDirectory(lua_State *L)
{
	auto dirname = tpt_lua_checkByteString(L, 1);
	bool ret = Platform::DirectoryExists(dirname);
	lua_pushboolean(L, ret);
	return 1;
}

static int isLink(lua_State *L)
{
	auto dirname = tpt_lua_checkByteString(L, 1);
	bool ret = Platform::IsLink(dirname);
	lua_pushboolean(L, ret);
	return 1;
}

static int makeDirectory(lua_State *L)
{
	auto dirname = tpt_lua_checkByteString(L, 1);

	int ret = 0;
	ret = Platform::MakeDirectory(dirname);
	lua_pushboolean(L, ret);
	return 1;
}

static int removeDirectory(lua_State *L)
{
	auto directory = tpt_lua_checkByteString(L, 1);

	bool ret = Platform::DeleteDirectory(directory);
	lua_pushboolean(L, ret);
	return 1;
}

static int removeFile(lua_State *L)
{
	auto filename = tpt_lua_checkByteString(L, 1);
	lua_pushboolean(L, Platform::RemoveFile(filename));
	return 1;
}

static int move(lua_State *L)
{
	auto filename = tpt_lua_checkByteString(L, 1);
	auto newFilename = tpt_lua_checkByteString(L, 2);
	bool replace = lua_toboolean(L, 3);
	lua_pushboolean(L, Platform::RenameFile(filename, newFilename, replace));
	return 1;
}

static int copy(lua_State *L)
{
	auto filename = tpt_lua_checkByteString(L, 1);
	auto newFilename = tpt_lua_checkByteString(L, 2);
	std::vector<char> fileData;
	lua_pushboolean(L, Platform::ReadFile(fileData, filename) && Platform::WriteFile(fileData, newFilename));
	return 1;
}

void LuaFileSystem::Open(lua_State *L)
{
	static const luaL_Reg reg[] = {
#define LFUNC(v) { #v, v }
		LFUNC(list),
		LFUNC(exists),
		LFUNC(isFile),
		LFUNC(isDirectory),
		LFUNC(isLink),
		LFUNC(makeDirectory),
		LFUNC(removeDirectory),
		LFUNC(removeFile),
		LFUNC(move),
		LFUNC(copy),
#undef LFUNC
		{ nullptr, nullptr }
	};
	lua_newtable(L);
	luaL_register(L, nullptr, reg);
	lua_pushvalue(L, -1);
	lua_setglobal(L, "fileSystem");
	lua_setglobal(L, "fs");
}
