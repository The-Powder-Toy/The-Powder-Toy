#include "LuaScriptInterface.h"
#include "bzip2/bz2wrap.h"

static int compress(lua_State *L)
{
	auto src = tpt_lua_checkByteString(L, 1);
	auto maxSize = size_t(luaL_optinteger(L, 2, 0));
	std::vector<char> dest;
	auto result = BZ2WCompress(dest, src.data(), src.size(), maxSize);
#define RETURN_ERR(str) lua_pushnil(L); lua_pushinteger(L, int(result)); lua_pushliteral(L, str); return 3
	switch (result)
	{
	case BZ2WCompressOk: break;
	case BZ2WCompressNomem: RETURN_ERR("out of memory");
	case BZ2WCompressLimit: RETURN_ERR("size limit exceeded");
	}
#undef RETURN_ERR
	tpt_lua_pushByteString(L, ByteString(dest.begin(), dest.end()));
	return 1;
}

static int decompress(lua_State *L)
{
	auto src = tpt_lua_checkByteString(L, 1);
	auto maxSize = size_t(luaL_optinteger(L, 2, 0));
	std::vector<char> dest;
	auto result = BZ2WDecompress(dest, src.data(), src.size(), maxSize);
#define RETURN_ERR(str) lua_pushnil(L); lua_pushinteger(L, int(result)); lua_pushliteral(L, str); return 3
	switch (result)
	{
	case BZ2WDecompressOk: break;
	case BZ2WDecompressNomem: RETURN_ERR("out of memory");
	case BZ2WDecompressLimit: RETURN_ERR("size limit exceeded");
	case BZ2WDecompressType:
	case BZ2WDecompressBad:
	case BZ2WDecompressEof: RETURN_ERR("corrupted stream");
	}
#undef RETURN_ERR
	tpt_lua_pushByteString(L, ByteString(dest.begin(), dest.end()));
	return 1;
}

void LuaBz2::Open(lua_State *L)
{
	static const luaL_Reg reg[] = {
#define LFUNC(v) { #v, v }
		LFUNC(compress),
		LFUNC(decompress),
#undef LFUNC
		{ NULL, NULL }
	};
	lua_newtable(L);
	luaL_register(L, NULL, reg);
#define LCONSTAS(k, v) lua_pushinteger(L, int(v)); lua_setfield(L, -2, k)
	LCONSTAS("COMPRESS_NOMEM"  , BZ2WCompressNomem  );
	LCONSTAS("COMPRESS_LIMIT"  , BZ2WCompressLimit  );
	LCONSTAS("DECOMPRESS_NOMEM", BZ2WDecompressNomem);
	LCONSTAS("DECOMPRESS_LIMIT", BZ2WDecompressLimit);
	LCONSTAS("DECOMPRESS_TYPE" , BZ2WDecompressType );
	LCONSTAS("DECOMPRESS_BAD"  , BZ2WDecompressBad  );
	LCONSTAS("DECOMPRESS_EOF"  , BZ2WDecompressEof  );
#undef LCONSTAS
	lua_setglobal(L, "bz2");
}
