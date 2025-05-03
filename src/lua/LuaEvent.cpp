#include "LuaScriptInterface.h"
#include "common/VariantIndex.h"
#include "PowderToySDL.h"

static int fregister(lua_State *L)
{
	auto *lsi = GetLSI();
	lsi->AssertInterfaceEvent();
	int eventType = luaL_checkinteger(L, 1);
	luaL_checktype(L, 2, LUA_TFUNCTION);
	if (eventType < 0 || eventType >= int(std::variant_size_v<GameControllerEvent>))
	{
		luaL_error(L, "Invalid event type: %i", lua_tointeger(L, 1));
	}
	lsi->AddEventHandler(eventType, 2);
	lua_pushvalue(L, 2);
	return 1;
}

static int unregister(lua_State *L)
{
	auto *lsi = GetLSI();
	lsi->AssertInterfaceEvent();
	int eventType = luaL_checkinteger(L, 1);
	luaL_checktype(L, 2, LUA_TFUNCTION);
	if (eventType < 0 || eventType >= int(std::variant_size_v<GameControllerEvent>))
	{
		luaL_error(L, "Invalid event type: %i", lua_tointeger(L, 1));
	}
	lsi->RemoveEventHandler(eventType, 2);
	return 0;
}

static int getModifiers(lua_State *L)
{
	GetLSI()->AssertInterfaceEvent();
	lua_pushnumber(L, GetModifiers());
	return 1;
}

void LuaEvent::Open(lua_State *L)
{
	static const luaL_Reg reg[] = {
#define LFUNC(v) { #v, v }
		LFUNC(unregister),
		LFUNC(getModifiers),
#undef LFUNC
		{ "register", fregister },
		{ nullptr, nullptr }
	};
	lua_newtable(L);
	luaL_register(L, nullptr, reg);
#define LVICONST(id, v) lua_pushinteger(L, VariantIndex<GameControllerEvent, id>()); lua_setfield(L, -2, v)
	LVICONST(TextInputEvent    , "TEXTINPUT"    );
	LVICONST(TextEditingEvent  , "TEXTEDITING"  );
	LVICONST(KeyPressEvent     , "KEYPRESS"     );
	LVICONST(KeyReleaseEvent   , "KEYRELEASE"   );
	LVICONST(MouseDownEvent    , "MOUSEDOWN"    );
	LVICONST(MouseUpEvent      , "MOUSEUP"      );
	LVICONST(MouseMoveEvent    , "MOUSEMOVE"    );
	LVICONST(MouseWheelEvent   , "MOUSEWHEEL"   );
	LVICONST(TickEvent         , "TICK"         );
	LVICONST(BlurEvent         , "BLUR"         );
	LVICONST(CloseEvent        , "CLOSE"        );
	LVICONST(BeforeSimEvent    , "BEFORESIM"    );
	LVICONST(AfterSimEvent     , "AFTERSIM"     );
	LVICONST(BeforeSimDrawEvent, "BEFORESIMDRAW");
	LVICONST(AfterSimDrawEvent , "AFTERSIMDRAW" );
#undef LVICONST
	lua_pushvalue(L, -1);
	lua_setglobal(L, "event");
	lua_setglobal(L, "evt");
}
