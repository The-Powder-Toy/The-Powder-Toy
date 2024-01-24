#include "LuaScriptInterface.h"
#include "gui/game/GameController.h"
#include "gui/game/GameModel.h"
#include "graphics/Renderer.h"
#include "simulation/ElementGraphics.h"

static int renderModes(lua_State *L)
{
	auto *lsi = GetLSI();
	int args = lua_gettop(L);
	if(args)
	{
		int size = 0;
		luaL_checktype(L, 1, LUA_TTABLE);
		size = lua_objlen(L, 1);

		std::vector<unsigned int> renderModes;
		for(int i = 1; i <= size; i++)
		{
			lua_rawgeti(L, 1, i);
			renderModes.push_back(lua_tointeger(L, -1));
			lua_pop(L, 1);
		}
		lsi->ren->SetRenderMode(renderModes);
		return 0;
	}
	else
	{
		lua_newtable(L);
		std::vector<unsigned int> renderModes = lsi->ren->GetRenderMode();
		int i = 1;
		for(std::vector<unsigned int>::iterator iter = renderModes.begin(), end = renderModes.end(); iter != end; ++iter)
		{
			lua_pushinteger(L, *iter);
			lua_rawseti(L, -2, i++);
		}
		return 1;
	}
}

static int hud(lua_State *L)
{
	auto *lsi = GetLSI();
	int acount = lua_gettop(L);
	if (acount == 0)
	{
		lua_pushboolean(L, lsi->gameController->GetHudEnable());
		return 1;
	}
	auto hudstate = lua_toboolean(L, 1);
	lsi->gameController->SetHudEnable(hudstate);
	return 0;
}

static int debugHud(lua_State *L)
{
	auto *lsi = GetLSI();
	int acount = lua_gettop(L);
	if (acount == 0)
	{
		lua_pushboolean(L, lsi->gameController->GetDebugHUD());
		return 1;
	}
	auto debug = lua_toboolean(L, 1);
	lsi->gameController->SetDebugHUD(debug);
	return 0;
}

static int useDisplayPreset(lua_State *L)
{
	auto *lsi = GetLSI();
	int cmode = luaL_optint(L, 1, 3)+1;
	if (cmode == 11)
		cmode = 0;
	if (cmode >= 0 && cmode <= 10)
		lsi->gameController->LoadRenderPreset(cmode);
	else
		return luaL_error(L, "Invalid display mode");
	return 0;
}

static int fireSize(lua_State *L)
{
	auto *lsi = GetLSI();
	if (lua_gettop(L) < 1)
	{
		lua_pushnumber(L, lsi->gameModel->GetRenderer()->GetFireIntensity());
		return 1;
	}
	float fireintensity = float(luaL_checknumber(L, 1));
	lsi->gameModel->GetRenderer()->prepare_alpha(CELL, fireintensity);
	return 0;
}

static int displayModes(lua_State *L)
{
	auto *lsi = GetLSI();
	int args = lua_gettop(L);
	if(args)
	{
		int size = 0;
		luaL_checktype(L, 1, LUA_TTABLE);
		size = lua_objlen(L, 1);

		std::vector<unsigned int> displayModes;
		for(int i = 1; i <= size; i++)
		{
			lua_rawgeti(L, 1, i);
			displayModes.push_back(lua_tointeger(L, -1));
			lua_pop(L, 1);
		}
		lsi->ren->SetDisplayMode(displayModes);
		return 0;
	}
	else
	{
		lua_newtable(L);
		std::vector<unsigned int> displayModes = lsi->ren->GetDisplayMode();
		int i = 1;
		for(std::vector<unsigned int>::iterator iter = displayModes.begin(), end = displayModes.end(); iter != end; ++iter)
		{
			lua_pushinteger(L, *iter);
			lua_rawseti(L, -2, i++);
		}
		return 1;
	}
}

static int colorMode(lua_State *L)
{
	auto *lsi = GetLSI();
	int args = lua_gettop(L);
	if(args)
	{
		luaL_checktype(L, 1, LUA_TNUMBER);
		lsi->ren->SetColourMode(lua_tointeger(L, 1));
		return 0;
	}
	else
	{
		lua_pushinteger(L, lsi->ren->GetColourMode());
		return 1;
	}
}

static int decorations(lua_State *L)
{
	auto *lsi = GetLSI();
	int acount = lua_gettop(L);
	if (acount == 0)
	{
		lua_pushboolean(L, lsi->gameModel->GetDecoration());
		return 1;
	}
	int decostate = lua_toboolean(L, 1);
	lsi->gameModel->SetDecoration(decostate);
	lsi->gameModel->UpdateQuickOptions();
	return 0;
}

static int grid(lua_State *L)
{
	auto *lsi = GetLSI();
	int acount = lua_gettop(L);
	if (acount == 0)
	{
		lua_pushnumber(L, lsi->ren->GetGridSize());
		return 1;
	}
	int grid = luaL_optint(L, 1, -1);
	lsi->ren->SetGridSize(grid);
	return 0;
}

static int showBrush(lua_State *L)
{
	auto *lsi = GetLSI();
	int acount = lua_gettop(L);
	if (acount == 0)
	{
		lua_pushnumber(L, lsi->gameController->GetBrushEnable());
		return 1;
	}
	int brush = luaL_optint(L, 1, -1);
	lsi->gameController->SetBrushEnable(brush);
	return 0;
}

static int depth3d(lua_State *L)
{
	return luaL_error(L, "This feature is no longer supported");
}

static int zoomEnabled(lua_State *L)
{
	auto *lsi = GetLSI();
	if (lua_gettop(L) == 0)
	{
		lua_pushboolean(L, lsi->ren->zoomEnabled);
		return 1;
	}
	else
	{
		luaL_checktype(L, -1, LUA_TBOOLEAN);
		lsi->ren->zoomEnabled = lua_toboolean(L, -1);
		return 0;
	}
}

static int zoomWindow(lua_State *L)
{
	auto *lsi = GetLSI();
	auto *ren = lsi->ren;
	if (lua_gettop(L) == 0)
	{
		ui::Point location = ren->zoomWindowPosition;
		lua_pushnumber(L, location.X);
		lua_pushnumber(L, location.Y);
		lua_pushnumber(L, ren->ZFACTOR);
		lua_pushnumber(L, ren->zoomScopeSize * ren->ZFACTOR);
		return 4;
	}
	int x = luaL_optint(L, 1, 0);
	int y = luaL_optint(L, 2, 0);
	int f = luaL_optint(L, 3, 0);
	if (f <= 0)
		return luaL_error(L, "Zoom factor must be greater than 0");

	// To prevent crash when zoom window is outside screen
	if (x < 0 || y < 0 || ren->zoomScopeSize * f + x > XRES || ren->zoomScopeSize * f + y > YRES)
		return luaL_error(L, "Zoom window outside of bounds");

	ren->zoomWindowPosition = ui::Point(x, y);
	ren->ZFACTOR = f;
	return 0;
}

static int zoomScope(lua_State *L)
{
	auto *lsi = GetLSI();
	auto *ren = lsi->ren;
	if (lua_gettop(L) == 0)
	{
		ui::Point location = ren->zoomScopePosition;
		lua_pushnumber(L, location.X);
		lua_pushnumber(L, location.Y);
		lua_pushnumber(L, ren->zoomScopeSize);
		return 3;
	}
	int x = luaL_optint(L, 1, 0);
	int y = luaL_optint(L, 2, 0);
	int s = luaL_optint(L, 3, 0);
	if (s <= 0)
		return luaL_error(L, "Zoom scope size must be greater than 0");

	// To prevent crash when zoom or scope window is outside screen
	int windowEdgeRight = ren->ZFACTOR * s + ren->zoomWindowPosition.X;
	int windowEdgeBottom = ren->ZFACTOR * s + ren->zoomWindowPosition.Y;
	if (x < 0 || y < 0 || x + s > XRES || y + s > YRES)
		return luaL_error(L, "Zoom scope outside of bounds");
	if (windowEdgeRight > XRES || windowEdgeBottom > YRES)
		return luaL_error(L, "Zoom window outside of bounds");

	ren->zoomScopePosition = ui::Point(x, y);
	ren->zoomScopeSize = s;
	return 0;
}

void LuaRenderer::Open(lua_State *L)
{
	static const luaL_Reg reg[] = {
#define LFUNC(v) { #v, v }
		LFUNC(renderModes),
		LFUNC(displayModes),
		LFUNC(colorMode),
		LFUNC(decorations),
		LFUNC(grid),
		LFUNC(debugHud),
		LFUNC(hud),
		LFUNC(showBrush),
		LFUNC(depth3d),
		LFUNC(zoomEnabled),
		LFUNC(zoomWindow),
		LFUNC(zoomScope),
		LFUNC(fireSize),
		LFUNC(useDisplayPreset),
#undef LFUNC
		{ NULL, NULL }
	};
	lua_newtable(L);
	luaL_register(L, NULL, reg);
#define LCONST(v) lua_pushinteger(L, int(v)); lua_setfield(L, -2, #v)
	LCONST(PMODE);
	LCONST(PMODE_NONE);
	LCONST(PMODE_FLAT);
	LCONST(PMODE_BLOB);
	LCONST(PMODE_BLUR);
	LCONST(PMODE_GLOW);
	LCONST(PMODE_SPARK);
	LCONST(PMODE_FLARE);
	LCONST(PMODE_LFLARE);
	LCONST(PMODE_ADD);
	LCONST(PMODE_BLEND);
	LCONST(PSPEC_STICKMAN);
	LCONST(OPTIONS);
	LCONST(NO_DECO);
	LCONST(DECO_FIRE);
	LCONST(FIREMODE);
	LCONST(FIRE_ADD);
	LCONST(FIRE_BLEND);
	LCONST(FIRE_SPARK);
	LCONST(EFFECT);
	LCONST(EFFECT_GRAVIN);
	LCONST(EFFECT_GRAVOUT);
	LCONST(EFFECT_LINES);
	LCONST(EFFECT_DBGLINES);
	LCONST(RENDER_EFFE);
	LCONST(RENDER_FIRE);
	LCONST(RENDER_GLOW);
	LCONST(RENDER_BLUR);
	LCONST(RENDER_BLOB);
	LCONST(RENDER_BASC);
	LCONST(RENDER_NONE);
	LCONST(COLOUR_HEAT);
	LCONST(COLOUR_LIFE);
	LCONST(COLOUR_GRAD);
	LCONST(COLOUR_BASC);
	LCONST(COLOUR_DEFAULT);
	LCONST(DISPLAY_AIRC);
	LCONST(DISPLAY_AIRP);
	LCONST(DISPLAY_AIRV);
	LCONST(DISPLAY_AIRH);
	LCONST(DISPLAY_AIR);
	LCONST(DISPLAY_WARP);
	LCONST(DISPLAY_PERS);
	LCONST(DISPLAY_EFFE);
#undef LCONST
	lua_pushvalue(L, -1);
	lua_setglobal(L, "renderer");
	lua_setglobal(L, "ren");
}
