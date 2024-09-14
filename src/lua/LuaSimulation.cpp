#include "LuaScriptInterface.h"
#include "client/Client.h"
#include "client/GameSave.h"
#include "client/SaveFile.h"
#include "client/SaveInfo.h"
#include "Format.h"
#include "gui/game/GameController.h"
#include "gui/game/GameModel.h"
#include "gui/game/GameView.h"
#include "gui/game/Brush.h"
#include "gui/game/tool/Tool.h"
#include "simulation/Air.h"
#include "simulation/ElementCommon.h"
#include "simulation/GOLString.h"
#include "simulation/gravity/Gravity.h"
#include "simulation/Snapshot.h"
#include "simulation/ToolClasses.h"
#include <type_traits>

static int ambientHeatSim(lua_State *L)
{
	auto *lsi = GetLSI();
	int acount = lua_gettop(L);
	if (acount == 0)
	{
		lua_pushboolean(L, lsi->sim->aheat_enable);
		return 1;
	}
	auto aheatstate = lua_toboolean(L, 1);
	lsi->sim->aheat_enable = aheatstate;
	lsi->gameModel->UpdateQuickOptions();

	return 0;
}

static int heatSim(lua_State *L)
{
	auto *lsi = GetLSI();
	int acount = lua_gettop(L);
	if (acount == 0)
	{
		lua_pushboolean(L, !lsi->sim->legacy_enable);
		return 1;
	}
	auto heatstate = lua_toboolean(L, 1);
	lsi->sim->legacy_enable = !heatstate;
	return 0;
}

static int newtonianGravity(lua_State *L)
{
	auto *lsi = GetLSI();
	int acount = lua_gettop(L);
	if (acount == 0)
	{
		lua_pushboolean(L, bool(lsi->sim->grav));
		return 1;
	}
	lsi->sim->EnableNewtonianGravity(lua_toboolean(L, 1));
	lsi->gameModel->UpdateQuickOptions();
	return 0;
}

static int paused(lua_State *L)
{
	auto *lsi = GetLSI();
	int acount = lua_gettop(L);
	if (acount == 0)
	{
		lua_pushboolean(L, lsi->gameModel->GetPaused());
		return 1;
	}
	auto pausestate = lua_toboolean(L, 1);
	lsi->gameModel->SetPaused(pausestate);
	return 0;
}

static int partCount(lua_State *L)
{
	auto *lsi = GetLSI();
	lua_pushinteger(L, lsi->sim->NUM_PARTS);
	return 1;
}

static int decoSpace(lua_State *L)
{
	auto *lsi = GetLSI();
	if (lua_gettop(L) < 1)
	{
		lua_pushnumber(L, lsi->gameModel->GetDecoSpace());
		return 1;
	}
	auto index = luaL_checkint(L, 1);
	if (index < 0 || index >= NUM_DECOSPACES)
	{
		return luaL_error(L, "Invalid deco space index %i", index);
	}
	lsi->gameModel->SetDecoSpace(index);
	return 0;
}

template<class Accessor>
struct LuaBlockMapHelper
{
	using ItemType = std::remove_reference_t<std::invoke_result_t<Accessor, Vec2<int>>>;
};

template<bool Clamp, class Accessor, class ItemType = typename LuaBlockMapHelper<Accessor>::ItemType>
static int LuaBlockMapImpl(lua_State *L, ItemType minValue, ItemType maxValue, Accessor accessor)
{
	auto pos = Vec2{ luaL_checkint(L, 1), luaL_checkint(L, 2) };
	if (!CELLS.OriginRect().Contains(pos))
	{
		return luaL_error(L, "Coordinates (%i, %i) out of range", pos.X, pos.Y);
	}
	auto argc = lua_gettop(L);
	if (argc == 2)
	{
		if constexpr (std::is_integral_v<ItemType>)
		{
			lua_pushinteger(L, lua_Integer(accessor(pos)));
		}
		else
		{
			lua_pushnumber(L, lua_Number(accessor(pos)));
		}
		return 1;
	}
	auto size = Vec2{ 1, 1 };
	auto valuePos = 3;
	if (argc > 3)
	{
		size = Vec2{ luaL_checkint(L, 3), luaL_checkint(L, 4) };
		valuePos = 5;
	}
	ItemType value;
	if constexpr (std::is_integral_v<ItemType>)
	{
		value = ItemType(luaL_checkint(L, valuePos));
	}
	else
	{
		value = ItemType(luaL_checknumber(L, valuePos));
	}
	if constexpr (Clamp)
	{
		if (value > maxValue) value = maxValue;
		if (value < minValue) value = minValue;
	}
	for (auto p : CELLS.OriginRect() & RectSized(pos, size))
	{
		accessor(p) = value;
	}
	return 0;
}

template<class Accessor, class ItemType = typename LuaBlockMapHelper<Accessor>::ItemType>
static int LuaBlockMap(lua_State *L, ItemType minValue, ItemType maxValue, Accessor accessor)
{
	return LuaBlockMapImpl<true>(L, minValue, maxValue, accessor);
}

template<class Accessor, class ItemType = typename LuaBlockMapHelper<Accessor>::ItemType>
static int LuaBlockMap(lua_State *L, Accessor accessor)
{
	return LuaBlockMapImpl<false>(L, ItemType(0), ItemType(0), accessor);
}

static int velocityX(lua_State *L)
{
	auto *lsi = GetLSI();
	return LuaBlockMap(L, MIN_PRESSURE, MAX_PRESSURE, [lsi](Vec2<int> p) -> float & {
		return lsi->sim->vx[p.Y][p.X];
	});
}

static int velocityY(lua_State *L)
{
	auto *lsi = GetLSI();
	return LuaBlockMap(L, MIN_PRESSURE, MAX_PRESSURE, [lsi](Vec2<int> p) -> float & {
		return lsi->sim->vy[p.Y][p.X];
	});
}

static int ambientHeat(lua_State *L)
{
	auto *lsi = GetLSI();
	return LuaBlockMap(L, MIN_TEMP, MAX_TEMP, [lsi](Vec2<int> p) -> float & {
		return lsi->sim->hv[p.Y][p.X];
	});
}

static int pressure(lua_State *L)
{
	auto *lsi = GetLSI();
	return LuaBlockMap(L, MIN_PRESSURE, MAX_PRESSURE, [lsi](Vec2<int> p) -> float & {
		return lsi->sim->pv[p.Y][p.X];
	});
}

static int gravityMass(lua_State *L)
{
	auto *lsi = GetLSI();
	return LuaBlockMap(L, [lsi](Vec2<int> p) -> float & {
		return lsi->sim->gravIn.mass[p];
	});
}

static int gravityField(lua_State *L)
{
	auto *lsi = GetLSI();
	auto pos = Vec2{ luaL_checkint(L, 1), luaL_checkint(L, 2) };
	if (!CELLS.OriginRect().Contains(pos))
	{
		return luaL_error(L, "Coordinates (%i, %i) out of range", pos.X, pos.Y);
	}
	lua_pushnumber(L, lsi->sim->gravOut.forceX[pos]);
	lua_pushnumber(L, lsi->sim->gravOut.forceY[pos]);
	return 2;
}

static int elecMap(lua_State *L)
{
	auto *lsi = GetLSI();
	return LuaBlockMap(L, [lsi](Vec2<int> p) -> unsigned char & {
		return lsi->sim->emap[p.Y][p.X];
	});
}

static int wallMap(lua_State *L)
{
	auto *lsi = GetLSI();
	return LuaBlockMap(L, 0, UI_WALLCOUNT - 1, [lsi](Vec2<int> p) -> unsigned char & {
		return lsi->sim->bmap[p.Y][p.X];
	});
}

static int fanVelocityX(lua_State *L)
{
	auto *lsi = GetLSI();
	return LuaBlockMap(L, [lsi](Vec2<int> p) -> float & {
		return lsi->sim->fvx[p.Y][p.X];
	});
}

static int fanVelocityY(lua_State *L)
{	auto *lsi = GetLSI();
	return LuaBlockMap(L, [lsi](Vec2<int> p) -> float & {
		return lsi->sim->fvy[p.Y][p.X];
	});
}

static int partNeighbors(lua_State *L)
{
	auto *lsi = GetLSI();
	auto *sim = lsi->sim;
	lua_newtable(L);
	int id = 1;
	int x = lua_tointeger(L, 1), y = lua_tointeger(L, 2), r = lua_tointeger(L, 3), rx, ry, n;
	if(lua_gettop(L) == 5) // this is one more than the number of arguments because a table has just been pushed onto the stack with lua_newtable(L);
	{
		int t = lua_tointeger(L, 4);
		for (rx = -r; rx <= r; rx++)
			for (ry = -r; ry <= r; ry++)
				if (x+rx >= 0 && y+ry >= 0 && x+rx < XRES && y+ry < YRES && (rx || ry))
				{
					n = sim->pmap[y+ry][x+rx];
					if (!n || TYP(n) != t)
						n = sim->photons[y+ry][x+rx];
					if (n && TYP(n) == t)
					{
						lua_pushinteger(L, ID(n));
						lua_rawseti(L, -2, id++);
					}
				}

	}
	else
	{
		for (rx = -r; rx <= r; rx++)
			for (ry = -r; ry <= r; ry++)
				if (x+rx >= 0 && y+ry >= 0 && x+rx < XRES && y+ry < YRES && (rx || ry))
				{
					n = sim->pmap[y+ry][x+rx];
					if (!n)
						n = sim->photons[y+ry][x+rx];
					if (n)
					{
						lua_pushinteger(L, ID(n));
						lua_rawseti(L, -2, id++);
					}
				}
	}
	return 1;
}

static int partChangeType(lua_State *L)
{
	auto *lsi = GetLSI();
	int partIndex = lua_tointeger(L, 1);
	if(partIndex < 0 || partIndex >= NPART || !lsi->sim->parts[partIndex].type)
		return 0;
	lsi->sim->part_change_type(partIndex, int(lsi->sim->parts[partIndex].x+0.5f), int(lsi->sim->parts[partIndex].y+0.5f), lua_tointeger(L, 2));
	return 0;
}

static int partCreate(lua_State *L)
{
	auto *lsi = GetLSI();
	int newID = lua_tointeger(L, 1);
	if (newID >= NPART || newID < -3)
	{
		lua_pushinteger(L, -1);
		return 1;
	}
	if (newID >= 0 && !lsi->sim->parts[newID].type)
	{
		lua_pushinteger(L, -1);
		return 1;
	}
	int type = lua_tointeger(L, 4);
	int v = -1;
	if (lua_gettop(L) >= 5)
	{
		v = lua_tointeger(L, 5);
	}
	else if (ID(type))
	{
		v = ID(type);
		type = TYP(type);
	}
	lua_pushinteger(L, lsi->sim->create_part(newID, lua_tointeger(L, 2), lua_tointeger(L, 3), type, v));
	return 1;
}

static int partID(lua_State *L)
{
	auto *lsi = GetLSI();
	int x = lua_tointeger(L, 1);
	int y = lua_tointeger(L, 2);

	if(x < 0 || x >= XRES || y < 0 || y >= YRES)
	{
		lua_pushnil(L);
		return 1;
	}

	int amalgam = lsi->sim->pmap[y][x];
	if(!amalgam)
		amalgam = lsi->sim->photons[y][x];
	if (!amalgam)
		lua_pushnil(L);
	else
		lua_pushinteger(L, ID(amalgam));
	return 1;
}

static int partPosition(lua_State *L)
{
	auto *lsi = GetLSI();
	auto *sim = lsi->sim;
	int particleID = lua_tointeger(L, 1);
	int argCount = lua_gettop(L);
	if (particleID < 0 || particleID >= NPART || !sim->parts[particleID].type)
	{
		if(argCount == 1)
		{
			lua_pushnil(L);
			lua_pushnil(L);
			return 2;
		} else {
			return 0;
		}
	}

	if (argCount == 3)
	{
		float x = sim->parts[particleID].x;
		float y = sim->parts[particleID].y;
		sim->move(particleID, (int)(x + 0.5f), (int)(y + 0.5f), lua_tonumber(L, 2), lua_tonumber(L, 3));

		return 0;
	}
	else
	{
		lua_pushnumber(L, sim->parts[particleID].x);
		lua_pushnumber(L, sim->parts[particleID].y);
		return 2;
	}
}

static int partProperty(lua_State *L)
{
	auto *lsi = GetLSI();
	int argCount = lua_gettop(L);
	int particleID = luaL_checkinteger(L, 1);
	StructProperty property;

	if (particleID < 0 || particleID >= NPART || !lsi->sim->parts[particleID].type)
	{
		if (argCount == 3)
		{
			lua_pushnil(L);
			return 1;
		}
		else
		{
			return 0;
		}
	}

	auto &properties = Particle::GetProperties();
	auto prop = properties.end();

	//Get field
	if (lua_type(L, 2) == LUA_TNUMBER)
	{
		int fieldID = lua_tointeger(L, 2);
		if (fieldID < 0 || fieldID >= (int)properties.size())
			return luaL_error(L, "Invalid field ID (%d)", fieldID);
		prop = properties.begin() + fieldID;
	}
	else if (lua_type(L, 2) == LUA_TSTRING)
	{
		ByteString fieldName = tpt_lua_toByteString(L, 2);
		for (auto &alias : Particle::GetPropertyAliases())
		{
			if (fieldName == alias.from)
			{
				fieldName = alias.to;
			}
		}
		prop = std::find_if(properties.begin(), properties.end(), [&fieldName](StructProperty const &p) {
			return p.Name == fieldName;
		});
		if (prop == properties.end())
			return luaL_error(L, "Unknown field (%s)", fieldName.c_str());
	}
	else
	{
		return luaL_error(L, "Field ID must be an name (string) or identifier (integer)");
	}

	//Calculate memory address of property
	intptr_t propertyAddress = (intptr_t)(((unsigned char*)&lsi->sim->parts[particleID]) + prop->Offset);

	if (argCount == 3)
	{
		LuaSetParticleProperty(L, particleID, *prop, propertyAddress, 3);
		return 0;
	}
	else
	{
		LuaGetProperty(L, *prop, propertyAddress);
		return 1;
	}
}

static int partKill(lua_State *L)
{
	auto *lsi = GetLSI();
	if(lua_gettop(L)==2)
		lsi->sim->delete_part(lua_tointeger(L, 1), lua_tointeger(L, 2));
	else
	{
		int i = lua_tointeger(L, 1);
		if (i>=0 && i<NPART)
			lsi->sim->kill_part(i);
	}
	return 0;
}

static int partExists(lua_State *L)
{
	auto *lsi = GetLSI();
	int i = luaL_checkinteger(L, 1);
	lua_pushboolean(L, i >= 0 && i < NPART && lsi->sim->parts[i].type);
	return 1;
}

static int createParts(lua_State *L)
{
	auto *lsi = GetLSI();
	int x = luaL_optint(L,1,-1);
	int y = luaL_optint(L,2,-1);
	int rx = luaL_optint(L,3,5);
	int ry = luaL_optint(L,4,5);
	int c = luaL_optint(L,5,lsi->gameModel->GetActiveTool(0)->ToolID);
	int brushID = luaL_optint(L,6,BRUSH_CIRCLE);
	int flags = luaL_optint(L,7,lsi->sim->replaceModeFlags);

	Brush *brush = lsi->gameModel->GetBrushByID(brushID);
	if (!brush)
		return luaL_error(L, "Invalid brush id '%d'", brushID);
	auto newBrush = brush->Clone();
	newBrush->SetRadius(ui::Point(rx, ry));

	int ret = lsi->sim->CreateParts(x, y, c, *newBrush, flags);
	lua_pushinteger(L, ret);
	return 1;
}

static int createLine(lua_State *L)
{
	auto *lsi = GetLSI();
	int x1 = luaL_optint(L,1,-1);
	int y1 = luaL_optint(L,2,-1);
	int x2 = luaL_optint(L,3,-1);
	int y2 = luaL_optint(L,4,-1);
	int rx = luaL_optint(L,5,5);
	int ry = luaL_optint(L,6,5);
	int c = luaL_optint(L,7,lsi->gameModel->GetActiveTool(0)->ToolID);
	int brushID = luaL_optint(L,8,BRUSH_CIRCLE);
	int flags = luaL_optint(L,9,lsi->sim->replaceModeFlags);

	Brush *brush = lsi->gameModel->GetBrushByID(brushID);
	if (!brush)
		return luaL_error(L, "Invalid brush id '%d'", brushID);
	auto newBrush = brush->Clone();
	newBrush->SetRadius(ui::Point(rx, ry));

	lsi->sim->CreateLine(x1, y1, x2, y2, c, *newBrush, flags);
	return 0;
}

static int createBox(lua_State *L)
{
	auto *lsi = GetLSI();
	int x1 = luaL_optint(L,1,-1);
	int y1 = luaL_optint(L,2,-1);
	int x2 = luaL_optint(L,3,-1);
	int y2 = luaL_optint(L,4,-1);
	int c = luaL_optint(L,5,lsi->gameModel->GetActiveTool(0)->ToolID);
	int flags = luaL_optint(L,6,lsi->sim->replaceModeFlags);

	lsi->sim->CreateBox(x1, y1, x2, y2, c, flags);
	return 0;
}

static int floodParts(lua_State *L)
{
	auto *lsi = GetLSI();
	int x = luaL_optint(L,1,-1);
	int y = luaL_optint(L,2,-1);
	int c = luaL_optint(L,3,lsi->gameModel->GetActiveTool(0)->ToolID);
	int cm = luaL_optint(L,4,-1);
	int flags = luaL_optint(L,5,lsi->sim->replaceModeFlags);

	if (x < 0 || x >= XRES || y < 0 || y >= YRES)
		return luaL_error(L, "coordinates out of range (%d,%d)", x, y);

	int ret = lsi->sim->FloodParts(x, y, c, cm, flags);
	lua_pushinteger(L, ret);
	return 1;
}

static int createWalls(lua_State *L)
{
	int x = luaL_optint(L,1,-1);
	int y = luaL_optint(L,2,-1);
	int rx = luaL_optint(L,3,0);
	int ry = luaL_optint(L,4,0);
	int c = luaL_optint(L,5,8);

	if (x < 0 || x >= XRES || y < 0 || y >= YRES)
		return luaL_error(L, "coordinates out of range (%d,%d)", x, y);
	if (c < 0 || c >= UI_WALLCOUNT)
		return luaL_error(L, "Unrecognised wall id '%d'", c);

	auto *lsi = GetLSI();
	int ret = lsi->sim->CreateWalls(x, y, rx, ry, c, NULL);
	lua_pushinteger(L, ret);
	return 1;
}

static int createWallLine(lua_State *L)
{
	int x1 = luaL_optint(L,1,-1);
	int y1 = luaL_optint(L,2,-1);
	int x2 = luaL_optint(L,3,-1);
	int y2 = luaL_optint(L,4,-1);
	int rx = luaL_optint(L,5,0);
	int ry = luaL_optint(L,6,0);
	int c = luaL_optint(L,7,8);

	if (x1 < 0 || x2 < 0 || x1 >= XRES || x2 >= XRES || y1 < 0 || y2 < 0 || y1 >= YRES || y2 >= YRES)
		return luaL_error(L, "coordinates out of range (%d,%d),(%d,%d)", x1, y1, x2, y2);
	if (c < 0 || c >= UI_WALLCOUNT)
		return luaL_error(L, "Unrecognised wall id '%d'", c);

	auto *lsi = GetLSI();
	lsi->sim->CreateWallLine(x1, y1, x2, y2, rx, ry, c, NULL);
	return 0;
}

static int createWallBox(lua_State *L)
{
	int x1 = luaL_optint(L,1,-1);
	int y1 = luaL_optint(L,2,-1);
	int x2 = luaL_optint(L,3,-1);
	int y2 = luaL_optint(L,4,-1);
	int c = luaL_optint(L,5,8);

	if (x1 < 0 || x2 < 0 || x1 >= XRES || x2 >= XRES || y1 < 0 || y2 < 0 || y1 >= YRES || y2 >= YRES)
		return luaL_error(L, "coordinates out of range (%d,%d),(%d,%d)", x1, y1, x2, y2);
	if (c < 0 || c >= UI_WALLCOUNT)
		return luaL_error(L, "Unrecognised wall id '%d'", c);

	auto *lsi = GetLSI();
	lsi->sim->CreateWallBox(x1, y1, x2, y2, c);
	return 0;
}

static int floodWalls(lua_State *L)
{
	int x = luaL_optint(L,1,-1);
	int y = luaL_optint(L,2,-1);
	int c = luaL_optint(L,3,8);
	int bm = luaL_optint(L,4,-1);
	if (x < 0 || x >= XRES || y < 0 || y >= YRES)
		return luaL_error(L, "coordinates out of range (%d,%d)", x, y);
	if (c < 0 || c >= UI_WALLCOUNT)
		return luaL_error(L, "Unrecognised wall id '%d'", c);
	if (c == WL_STREAM)
	{
		lua_pushinteger(L, 0);
		return 1;
	}
	auto *lsi = GetLSI();
	int ret = lsi->sim->FloodWalls(x, y, c, bm);
	lua_pushinteger(L, ret);
	return 1;
}

static int toolBrush(lua_State *L)
{
	int x = luaL_optint(L,1,-1);
	int y = luaL_optint(L,2,-1);
	int rx = luaL_optint(L,3,5);
	int ry = luaL_optint(L,4,5);
	int tool = luaL_optint(L,5,0);
	int brushID = luaL_optint(L,6,BRUSH_CIRCLE);
	float strength = luaL_optnumber(L,7,1.0f);
	auto *lsi = GetLSI();
	auto *toolPtr = lsi->gameModel->GetToolByIndex(tool);
	if (!toolPtr)
	{
		return luaL_error(L, "Invalid tool id '%d'", tool);
	}

	Brush *brush = lsi->gameModel->GetBrushByID(brushID);
	if (!brush)
		return luaL_error(L, "Invalid brush id '%d'", brushID);
	auto newBrush = brush->Clone();
	newBrush->SetRadius(ui::Point(rx, ry));

	toolPtr->Strength = strength;
	toolPtr->Draw(lsi->sim, *newBrush, { x, y });
	return 0;
}

static int toolLine(lua_State *L)
{
	int x1 = luaL_optint(L,1,-1);
	int y1 = luaL_optint(L,2,-1);
	int x2 = luaL_optint(L,3,-1);
	int y2 = luaL_optint(L,4,-1);
	int rx = luaL_optint(L,5,5);
	int ry = luaL_optint(L,6,5);
	int tool = luaL_optint(L,7,0);
	int brushID = luaL_optint(L,8,BRUSH_CIRCLE);
	float strength = luaL_optnumber(L,9,1.0f);

	if (x1 < 0 || x2 < 0 || x1 >= XRES || x2 >= XRES || y1 < 0 || y2 < 0 || y1 >= YRES || y2 >= YRES)
		return luaL_error(L, "coordinates out of range (%d,%d),(%d,%d)", x1, y1, x2, y2);
	auto *lsi = GetLSI();
	auto *toolPtr = lsi->gameModel->GetToolByIndex(tool);
	if (!toolPtr)
	{
		return luaL_error(L, "Invalid tool id '%d'", tool);
	}

	Brush *brush = lsi->gameModel->GetBrushByID(brushID);
	if (!brush)
		return luaL_error(L, "Invalid brush id '%d'", brushID);
	auto newBrush = brush->Clone();
	newBrush->SetRadius(ui::Point(rx, ry));
	toolPtr->Strength = strength;
	toolPtr->DrawLine(lsi->sim, *newBrush, { x1, y1 }, { x2, y2 }, false);
	return 0;
}

static int toolBox(lua_State *L)
{
	int x1 = luaL_optint(L,1,-1);
	int y1 = luaL_optint(L,2,-1);
	int x2 = luaL_optint(L,3,-1);
	int y2 = luaL_optint(L,4,-1);
	if (x1 < 0 || x2 < 0 || x1 >= XRES || x2 >= XRES || y1 < 0 || y2 < 0 || y1 >= YRES || y2 >= YRES)
		return luaL_error(L, "coordinates out of range (%d,%d),(%d,%d)", x1, y1, x2, y2);
	int tool = luaL_optint(L,5,0);
	float strength = luaL_optnumber(L,6,1.0f);
	int brushID = luaL_optint(L,7,BRUSH_CIRCLE);
	int rx = luaL_optint(L,5,0);
	int ry = luaL_optint(L,6,0);
	auto *lsi = GetLSI();
	Brush *brush = lsi->gameModel->GetBrushByID(brushID);
	if (!brush)
	{
		return luaL_error(L, "Invalid brush id '%d'", brushID);
	}
	auto *toolPtr = lsi->gameModel->GetToolByIndex(tool);
	if (!toolPtr)
	{
		return luaL_error(L, "Invalid tool id '%d'", tool);
	}
	auto newBrush = brush->Clone();
	newBrush->SetRadius(ui::Point(rx, ry));
	toolPtr->Strength = strength;
	toolPtr->DrawRect(lsi->sim, *newBrush, { x1, y1 }, { x2, y2 });
	return 0;
}

static int decoBrush(lua_State *L)
{
	int x = luaL_optint(L,1,-1);
	int y = luaL_optint(L,2,-1);
	int rx = luaL_optint(L,3,5);
	int ry = luaL_optint(L,4,5);
	int r = luaL_optint(L,5,255);
	int g = luaL_optint(L,6,255);
	int b = luaL_optint(L,7,255);
	int a = luaL_optint(L,8,255);
	int tool = luaL_optint(L,9,DECO_DRAW);
	int brushID = luaL_optint(L,10,BRUSH_CIRCLE);

	auto *lsi = GetLSI();
	Brush *brush = lsi->gameModel->GetBrushByID(brushID);
	if (!brush)
		return luaL_error(L, "Invalid brush id '%d'", brushID);
	auto newBrush = brush->Clone();
	newBrush->SetRadius(ui::Point(rx, ry));

	lsi->sim->ApplyDecorationPoint(x, y, r, g, b, a, tool, *newBrush);
	return 0;
}

static int decoLine(lua_State *L)
{
	int x1 = luaL_optint(L,1,-1);
	int y1 = luaL_optint(L,2,-1);
	int x2 = luaL_optint(L,3,-1);
	int y2 = luaL_optint(L,4,-1);
	int rx = luaL_optint(L,5,5);
	int ry = luaL_optint(L,6,5);
	int r = luaL_optint(L,7,255);
	int g = luaL_optint(L,8,255);
	int b = luaL_optint(L,9,255);
	int a = luaL_optint(L,10,255);
	int tool = luaL_optint(L,11,DECO_DRAW);
	int brushID = luaL_optint(L,12,BRUSH_CIRCLE);

	if (x1 < 0 || x2 < 0 || x1 >= XRES || x2 >= XRES || y1 < 0 || y2 < 0 || y1 >= YRES || y2 >= YRES)
		return luaL_error(L, "coordinates out of range (%d,%d),(%d,%d)", x1, y1, x2, y2);

	auto *lsi = GetLSI();
	Brush *brush = lsi->gameModel->GetBrushByID(brushID);
	if (!brush)
		return luaL_error(L, "Invalid brush id '%d'", brushID);
	auto newBrush = brush->Clone();
	newBrush->SetRadius(ui::Point(rx, ry));

	lsi->sim->ApplyDecorationLine(x1, y1, x2, y2, r, g, b, a, tool, *newBrush);
	return 0;
}

static int decoBox(lua_State *L)
{
	int x1 = luaL_optint(L,1,-1);
	int y1 = luaL_optint(L,2,-1);
	int x2 = luaL_optint(L,3,-1);
	int y2 = luaL_optint(L,4,-1);
	int r = luaL_optint(L,5,255);
	int g = luaL_optint(L,6,255);
	int b = luaL_optint(L,7,255);
	int a = luaL_optint(L,8,255);
	int tool = luaL_optint(L,9,0);

	if (x1 < 0 || x2 < 0 || x1 >= XRES || x2 >= XRES || y1 < 0 || y2 < 0 || y1 >= YRES || y2 >= YRES)
		return luaL_error(L, "coordinates out of range (%d,%d),(%d,%d)", x1, y1, x2, y2);

	auto *lsi = GetLSI();
	lsi->sim->ApplyDecorationBox(x1, y1, x2, y2, r, g, b, a, tool);
	return 0;
}

static int decoColor(lua_State *L)
{
	auto *lsi = GetLSI();
	int acount = lua_gettop(L);
	RGBA<uint8_t> color(0, 0, 0, 0);
	if (acount == 0)
	{
		lua_pushnumber(L, lsi->gameModel->GetColourSelectorColour().Pack());
		return 1;
	}
	else if (acount == 1)
		color = RGBA<uint8_t>::Unpack(pixel_rgba(luaL_optnumber(L, 1, 0xFFFF0000)));
	else
	{
		color.Red   = std::clamp(luaL_optint(L, 1, 255), 0, 255);
		color.Green = std::clamp(luaL_optint(L, 2, 255), 0, 255);
		color.Blue  = std::clamp(luaL_optint(L, 3, 255), 0, 255);
		color.Alpha = std::clamp(luaL_optint(L, 4, 255), 0, 255);
	}
	lsi->gameModel->SetColourSelectorColour(color);
	return 0;
}

static int floodDeco(lua_State *L)
{
	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);
	int r = luaL_checkinteger(L, 3);
	int g = luaL_checkinteger(L, 4);
	int b = luaL_checkinteger(L, 5);
	int a = luaL_checkinteger(L, 6);

	if (x < 0 || x >= XRES || y < 0 || y >= YRES)
		return luaL_error(L, "coordinates out of range (%d,%d)", x, y);

	auto *lsi = GetLSI();
	// hilariously broken, intersects with console and all Lua graphics
	auto &rendererFrame = lsi->gameModel->view->GetRendererFrame();
	auto loc = RGB<uint8_t>::Unpack(rendererFrame[{ x, y }]);
	lsi->sim->ApplyDecorationFill(rendererFrame, x, y, r, g, b, a, loc.Red, loc.Green, loc.Blue);
	return 0;
}

static int clearSim(lua_State *L)
{
	auto *lsi = GetLSI();
	lsi->gameController->ClearSim();
	return 0;
}

static int clearRect(lua_State *L)
{
	auto *lsi = GetLSI();
	int x = luaL_checkint(L,1);
	int y = luaL_checkint(L,2);
	int w = luaL_checkint(L,3)-1;
	int h = luaL_checkint(L,4)-1;
	lsi->sim->clear_area(x, y, w, h);
	return 0;
}

static int resetTemp(lua_State *L)
{
	auto *lsi = GetLSI();
	auto *sim = lsi->sim;
	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	bool onlyConductors = luaL_optint(L, 1, 0);
	for (int i = 0; i < sim->parts_lastActiveIndex; i++)
	{
		if (sim->parts[i].type && (elements[sim->parts[i].type].HeatConduct || !onlyConductors))
		{
			sim->parts[i].temp = elements[sim->parts[i].type].DefaultProperties.temp;
		}
	}
	return 0;
}

static int resetPressure(lua_State *L)
{
	int aCount = lua_gettop(L), width = XCELLS, height = YCELLS;
	int x1 = abs(luaL_optint(L, 1, 0));
	int y1 = abs(luaL_optint(L, 2, 0));
	if (aCount > 2)
	{
		width = abs(luaL_optint(L, 3, XCELLS));
		height = abs(luaL_optint(L, 4, YCELLS));
	}
	else if (aCount)
	{
		width = 1;
		height = 1;
	}
	if(x1 > XCELLS-1)
		x1 = XCELLS-1;
	if(y1 > YCELLS-1)
		y1 = YCELLS-1;
	if(x1+width > XCELLS-1)
		width = XCELLS-x1;
	if(y1+height > YCELLS-1)
		height = YCELLS-y1;
	auto *lsi = GetLSI();
	for (int nx = x1; nx<x1+width; nx++)
		for (int ny = y1; ny<y1+height; ny++)
		{
			lsi->sim->pv[ny][nx] = 0;
		}
	return 0;
}

static int saveStamp(lua_State *L)
{
	int x = luaL_optint(L,1,0);
	int y = luaL_optint(L,2,0);
	int w = luaL_optint(L,3,XRES-1);
	int h = luaL_optint(L,4,YRES-1);
	bool includePressure = luaL_optint(L, 5, 1);
	auto *lsi = GetLSI();
	ByteString name = lsi->gameController->StampRegion(ui::Point(x, y), ui::Point(x+w, y+h), includePressure);
	tpt_lua_pushByteString(L, name);
	return 1;
}

static int loadStamp(lua_State *L)
{
	int i = -1;
	int pushed = 1;
	std::unique_ptr<SaveFile> tempfile;
	Vec2<int> partP = {
		luaL_optint(L, 2, 0),
		luaL_optint(L, 3, 0),
	};
	auto hflip = lua_toboolean(L, 4);
	auto rotation = luaL_optint(L, 5, 0) & 3; // [0, 3] rotations
	bool includePressure = luaL_optint(L, 6, 1);
	auto &client = Client::Ref();
	if (lua_isstring(L, 1)) //Load from 10 char name, or full filename
	{
		auto filename = tpt_lua_optByteString(L, 1, "");
		tempfile = client.GetStamp(filename);
	}
	if ((!tempfile || !tempfile->GetGameSave()) && lua_isnumber(L, 1)) //Load from stamp ID
	{
		i = luaL_optint(L, 1, 0);
		auto &stampIDs = client.GetStamps();
		if (i < 0 || i >= int(stampIDs.size()))
			return luaL_error(L, "Invalid stamp ID: %d", i);
		tempfile = client.GetStamp(stampIDs[i]);
	}

	if (tempfile && tempfile->GetGameSave())
	{
		auto gameSave = tempfile->TakeGameSave();
		auto [ quoX, remX ] = floorDiv(partP.X, CELL);
		auto [ quoY, remY ] = floorDiv(partP.Y, CELL);
		if (remX || remY || hflip || rotation)
		{
			auto transform = Mat2<int>::Identity;
			if (hflip)
			{
				transform = Mat2<int>::MirrorX * transform;
			}
			for (auto i = 0; i < rotation; ++i)
			{
				transform = Mat2<int>::CCW * transform;
			}
			gameSave->Transform(transform, { remX, remY });
		}
		auto *lsi = GetLSI();
		lsi->sim->Load(gameSave.get(), includePressure, { quoX, quoY });
		lua_pushinteger(L, 1);

		if (gameSave->authors.size())
		{
			gameSave->authors["type"] = "luastamp";
			client.MergeStampAuthorInfo(gameSave->authors);
		}
	}
	else
	{
		pushed = 2;
		lua_pushnil(L);
		tpt_lua_pushString(L, tempfile ? tempfile->GetError() : "does not exist");
	}
	return pushed;
}

static int deleteStamp(lua_State *L)
{
	auto &client = Client::Ref();
	auto &stampIDs = client.GetStamps();

	if (lua_isstring(L, 1)) //note: lua_isstring returns true on numbers too
	{
		auto filename = tpt_lua_optByteString(L, 1, "");
		for (auto &stampID : stampIDs)
		{
			if (stampID == filename)
			{
				client.DeleteStamp(stampID);
				return 0;
			}
		}
	}
	if (lua_isnumber(L, 1)) //Load from stamp ID
	{
		int i = luaL_optint(L, 1, 0);
		if (i < 0 || i >= int(stampIDs.size()))
			return luaL_error(L, "Invalid stamp ID: %d", i);
		client.DeleteStamp(stampIDs[i]);
		return 0;
	}
	lua_pushnumber(L, -1);
	return 1;
}

static int listStamps(lua_State *L)
{
	lua_newtable(L);
	auto &client = Client::Ref();
	auto &stampIDs = client.GetStamps();
	auto i = 0;
	for (auto &stampID : stampIDs)
	{
		tpt_lua_pushByteString(L, stampID);
		i += 1;
		lua_rawseti(L, -2, i);
	}
	return 1;
}

static int loadSave(lua_State *L)
{
	int saveID = luaL_optint(L,1,0);
	int instant = luaL_optint(L,2,0);
	int history = luaL_optint(L,3,0); //Exact second a previous save was saved
	auto *lsi = GetLSI();
	lsi->gameController->OpenSavePreview(saveID, history, instant ? savePreviewInstant : savePreviewNormal);
	return 0;
}

static int reloadSave(lua_State *L)
{
	auto *lsi = GetLSI();
	lsi->gameController->ReloadSim();
	return 0;
}

static int getSaveID(lua_State *L)
{
	auto *lsi = GetLSI();
	auto *tempSave = lsi->gameModel->GetSave();
	if (tempSave)
	{
		lua_pushinteger(L, tempSave->GetID());
		lua_pushinteger(L, tempSave->Version);
		return 2;
	}
	return 0;
}

static int adjustCoords(lua_State *L)
{
	auto *lsi = GetLSI();
	int x = luaL_optint(L,1,0);
	int y = luaL_optint(L,2,0);
	ui::Point Coords = lsi->gameController->PointTranslate(ui::Point(x, y));
	lua_pushinteger(L, Coords.X);
	lua_pushinteger(L, Coords.Y);
	return 2;
}

static int prettyPowders(lua_State *L)
{
	auto *lsi = GetLSI();
	int acount = lua_gettop(L);
	if (acount == 0)
	{
		lua_pushnumber(L, lsi->sim->pretty_powder);
		return 1;
	}
	int prettyPowder = luaL_optint(L, 1, 0);
	lsi->sim->pretty_powder = prettyPowder;
	lsi->gameModel->UpdateQuickOptions();
	return 0;
}

static int gravityGrid(lua_State *L)
{
	auto *lsi = GetLSI();
	int acount = lua_gettop(L);
	if (acount == 0)
	{
		lua_pushnumber(L, lsi->gameModel->GetGravityGrid());
		return 1;
	}
	int gravityGrid = luaL_optint(L, 1, 0);
	lsi->gameModel->ShowGravityGrid(gravityGrid);
	lsi->gameModel->UpdateQuickOptions();
	return 0;
}

static int edgeMode(lua_State *L)
{
	auto *lsi = GetLSI();
	int acount = lua_gettop(L);
	if (acount == 0)
	{
		lua_pushnumber(L, lsi->gameModel->GetEdgeMode());
		return 1;
	}
	int edgeMode = luaL_optint(L, 1, EDGE_VOID);
	lsi->gameModel->SetEdgeMode(edgeMode);
	return 0;
}

static int gravityMode(lua_State *L)
{
	auto *lsi = GetLSI();
	int acount = lua_gettop(L);
	if (acount == 0)
	{
		lua_pushnumber(L, lsi->sim->gravityMode);
		return 1;
	}
	int gravityMode = luaL_optint(L, 1, GRAV_VERTICAL);
	lsi->sim->gravityMode = gravityMode;
	return 0;
}

static int customGravity(lua_State *L)
{
	auto *lsi = GetLSI();
	auto *sim = lsi->sim;
	int acount = lua_gettop(L);
	if (acount == 0)
	{
		lua_pushnumber(L, sim->customGravityX);
		lua_pushnumber(L, sim->customGravityY);
		return 2;
	}
	else if (acount == 1)
	{
		sim->customGravityX = 0.0f;
		sim->customGravityY = luaL_optnumber(L, 1, 0.0f);
		return 0;
	}
	sim->customGravityX = luaL_optnumber(L, 1, 0.0f);
	sim->customGravityY = luaL_optnumber(L, 2, 0.0f);
	return 0;
}

static int airMode(lua_State *L)
{
	auto *lsi = GetLSI();
	int acount = lua_gettop(L);
	if (acount == 0)
	{
		lua_pushnumber(L, lsi->sim->air->airMode);
		return 1;
	}
	int airMode = luaL_optint(L, 1, AIR_ON);
	lsi->sim->air->airMode = airMode;
	return 0;
}

static int waterEqualization(lua_State *L)
{
	auto *lsi = GetLSI();
	int acount = lua_gettop(L);
	if (acount == 0)
	{
		lua_pushnumber(L, lsi->sim->water_equal_test);
		return 1;
	}
	int waterMode = luaL_optint(L, 1, 0);
	lsi->sim->water_equal_test = waterMode;
	return 0;
}

static int ambientAirTemp(lua_State *L)
{
	auto *lsi = GetLSI();
	int acount = lua_gettop(L);
	if (acount == 0)
	{
		lua_pushnumber(L, lsi->sim->air->ambientAirTemp);
		return 1;
	}
	float ambientAirTemp = restrict_flt(luaL_optnumber(L, 1, R_TEMP + 273.15f), MIN_TEMP, MAX_TEMP);
	lsi->gameModel->SetAmbientAirTemperature(ambientAirTemp);
	return 0;
}

static int elementCount(lua_State *L)
{
	int element = luaL_optint(L, 1, 0);
	if (element < 0 || element >= PT_NUM)
		return luaL_error(L, "Invalid element ID (%d)", element);

	auto *lsi = GetLSI();
	lua_pushnumber(L, lsi->sim->elementCount[element]);
	return 1;
}

static int canMove(lua_State *L)
{
	auto *lsi = GetLSI();
	int movingElement = luaL_checkint(L, 1);
	int destinationElement = luaL_checkint(L, 2);
	if (movingElement < 0 || movingElement >= PT_NUM)
		return luaL_error(L, "Invalid element ID (%d)", movingElement);
	if (destinationElement < 0 || destinationElement >= PT_NUM)
		return luaL_error(L, "Invalid element ID (%d)", destinationElement);

	if (lua_gettop(L) < 3)
	{
		auto &sd = SimulationData::CRef();
		lua_pushnumber(L, sd.can_move[movingElement][destinationElement]);
		return 1;
	}
	else
	{
		int setting = luaL_checkint(L, 3) & 0x7F;
		lsi->customCanMove[movingElement][destinationElement] = setting | 0x80;
		auto &sd = SimulationData::Ref();
		sd.can_move[movingElement][destinationElement] = setting;
		return 0;
	}
}

static int brushClosure(lua_State *L)
{
	// see Simulation::ToolBrush
	int positionX = lua_tointeger(L, lua_upvalueindex(1));
	int positionY = lua_tointeger(L, lua_upvalueindex(2));
	int i = lua_tointeger(L, lua_upvalueindex(3));
	int size = lua_tointeger(L, lua_upvalueindex(4));
	auto points = reinterpret_cast<ui::Point *>(lua_touserdata(L, lua_upvalueindex(5)));

	if (i == size)
		return 0;

	lua_pushnumber(L, i + 1);
	lua_replace(L, lua_upvalueindex(3));

	lua_pushnumber(L, points[i].X + positionX);
	lua_pushnumber(L, points[i].Y + positionY);
	return 2;
}

static int brush(lua_State *L)
{
	auto *lsi = GetLSI();
	int argCount = lua_gettop(L);
	int positionX = luaL_checkint(L, 1);
	int positionY = luaL_checkint(L, 2);
	int brushradiusX, brushradiusY;
	if (argCount >= 4)
	{
		brushradiusX = luaL_checkint(L, 3);
		brushradiusY = luaL_checkint(L, 4);
	}
	else
	{
		ui::Point radius = lsi->gameModel->GetBrush().GetRadius();
		brushradiusX = radius.X;
		brushradiusY = radius.Y;
	}
	int brushID = luaL_optint(L, 5, lsi->gameModel->GetBrushID());

	Brush *brush = lsi->gameModel->GetBrushByID(brushID);
	if (!brush)
		return luaL_error(L, "Invalid brush id '%d'", brushID);
	auto newBrush = brush->Clone();
	newBrush->SetRadius(ui::Point(brushradiusX, brushradiusY));
	lua_pushnumber(L, positionX);
	lua_pushnumber(L, positionY);
	std::vector<ui::Point> points;
	std::copy(newBrush->begin(), newBrush->end(), std::back_inserter(points));
	lua_pushnumber(L, 0); // index
	lua_pushnumber(L, int(points.size()));
	auto points_ud = reinterpret_cast<ui::Point *>(lua_newuserdata(L, points.size() * sizeof(ui::Point)));
	std::copy(points.begin(), points.end(), points_ud);

	lua_pushcclosure(L, brushClosure, 5);
	return 1;
}

static int partsClosure(lua_State *L)
{
	auto *lsi = GetLSI();
	for (int i = lua_tointeger(L, lua_upvalueindex(1)); i <= lsi->sim->parts_lastActiveIndex; ++i)
	{
		if (lsi->sim->parts[i].type)
		{
			lua_pushnumber(L, i + 1);
			lua_replace(L, lua_upvalueindex(1));
			lua_pushnumber(L, i);
			return 1;
		}
	}
	return 0;
}

static int neighboursClosure(lua_State *L)
{
	auto *lsi = GetLSI();
	int cx = lua_tointeger(L, lua_upvalueindex(1));
	int cy = lua_tointeger(L, lua_upvalueindex(2));
	int rx = lua_tointeger(L, lua_upvalueindex(3));
	int ry = lua_tointeger(L, lua_upvalueindex(4));
	int t = lua_tointeger(L, lua_upvalueindex(5));
	int x = lua_tointeger(L, lua_upvalueindex(6));
	int y = lua_tointeger(L, lua_upvalueindex(7));
	while (y <= cy + ry)
	{
		int px = x;
		int py = y;
		x += 1;
		if (x > cx + rx)
		{
			x = cx - rx;
			y += 1;
		}
		int r = lsi->sim->pmap[py][px];
		if (!(r && (!t || TYP(r) == t))) // * If not [exists and is of the correct type]
		{
			r = 0;
		}
		if (!r)
		{
			r = lsi->sim->photons[py][px];
			if (!(r && (!t || TYP(r) == t))) // * If not [exists and is of the correct type]
			{
				r = 0;
			}
		}
		if (cx == px && cy == py)
		{
			r = 0;
		}
		if (r)
		{
			lua_pushnumber(L, x);
			lua_replace(L, lua_upvalueindex(6));
			lua_pushnumber(L, y);
			lua_replace(L, lua_upvalueindex(7));
			lua_pushnumber(L, ID(r));
			lua_pushnumber(L, px);
			lua_pushnumber(L, py);
			return 3;
		}
	}
	return 0;
}

static int neighbors(lua_State *L)
{
	int cx = luaL_checkint(L, 1);
	int cy = luaL_checkint(L, 2);
	int rx = luaL_optint(L, 3, 2);
	int ry = luaL_optint(L, 4, 2);
	int t = luaL_optint(L, 5, PT_NONE);
	if (rx < 0 || ry < 0)
	{
		luaL_error(L, "Invalid radius");
	}
	lua_pushnumber(L, cx);
	lua_pushnumber(L, cy);
	lua_pushnumber(L, rx);
	lua_pushnumber(L, ry);
	lua_pushnumber(L, t);
	lua_pushnumber(L, cx - rx);
	lua_pushnumber(L, cy - ry);
	lua_pushcclosure(L, neighboursClosure, 7);
	return 1;
}

static int parts(lua_State *L)
{
	lua_pushnumber(L, 0);
	lua_pushcclosure(L, partsClosure, 1);
	return 1;
}

static int pmap(lua_State *L)
{
	auto *lsi = GetLSI();
	int x = luaL_checkint(L, 1);
	int y = luaL_checkint(L, 2);
	if (x < 0 || x >= XRES || y < 0 || y >= YRES)
		return luaL_error(L, "coordinates out of range (%d,%d)", x, y);
	int r = lsi->sim->pmap[y][x];
	if (!TYP(r))
		return 0;
	lua_pushnumber(L, ID(r));
	return 1;
}

static int photons(lua_State *L)
{
	auto *lsi = GetLSI();
	int x = luaL_checkint(L, 1);
	int y = luaL_checkint(L, 2);
	if (x < 0 || x >= XRES || y < 0 || y >= YRES)
		return luaL_error(L, "coordinates out of range (%d,%d)", x, y);
	int r = lsi->sim->photons[y][x];
	if (!TYP(r))
		return 0;
	lua_pushnumber(L, ID(r));
	return 1;
}

static int frameRender(lua_State *L)
{
	auto *lsi = GetLSI();
	if (lua_gettop(L) == 0)
	{
		lua_pushinteger(L, lsi->sim->framerender);
		return 1;
	}
	int frames = luaL_checkinteger(L, 1);
	if (frames < 0)
		return luaL_error(L, "Can't simulate a negative number of frames");
	lsi->sim->framerender = frames;
	return 0;
}

static int golSpeedRatio(lua_State *L)
{
	auto *lsi = GetLSI();
	if (lua_gettop(L) == 0)
	{
		lua_pushinteger(L, lsi->sim->GSPEED);
		return 1;
	}
	int gspeed = luaL_checkinteger(L, 1);
	if (gspeed < 1)
		return luaL_error(L, "GSPEED must be at least 1");
	lsi->sim->GSPEED = gspeed;
	return 0;
}

static int takeSnapshot(lua_State *L)
{
	auto *lsi = GetLSI();
	lsi->gameController->HistorySnapshot();
	return 0;
}


static int historyRestore(lua_State *L)
{
	auto *lsi = GetLSI();
	bool successful = lsi->gameController->HistoryRestore();
	lua_pushboolean(L, successful);
	return 1;
}

static int historyForward(lua_State *L)
{
	auto *lsi = GetLSI();
	bool successful = lsi->gameController->HistoryForward();
	lua_pushboolean(L, successful);
	return 1;
}

static int replaceModeFlags(lua_State *L)
{
	auto *lsi = GetLSI();
	if (lua_gettop(L) == 0)
	{
		lua_pushinteger(L, lsi->gameController->GetReplaceModeFlags());
		return 1;
	}
	unsigned int flags = luaL_checkinteger(L, 1);
	if (flags & ~(REPLACE_MODE | SPECIFIC_DELETE))
		return luaL_error(L, "Invalid flags");
	if ((flags & REPLACE_MODE) && (flags & SPECIFIC_DELETE))
		return luaL_error(L, "Cannot set replace mode and specific delete at the same time");
	lsi->gameController->SetReplaceModeFlags(flags);
	return 0;
}

static int listCustomGol(lua_State *L)
{
	auto &sd = SimulationData::CRef();
	int i = 0;
	lua_newtable(L);
	for (auto &cgol : sd.GetCustomGol())
	{
		lua_newtable(L);
		tpt_lua_pushString(L, cgol.nameString);
		lua_setfield(L, -2, "name");
		tpt_lua_pushString(L, SerialiseGOLRule(cgol.rule));
		lua_setfield(L, -2, "rulestr");
		lua_pushnumber(L, cgol.rule);
		lua_setfield(L, -2, "rule");
		lua_pushnumber(L, cgol.colour1.Pack());
		lua_setfield(L, -2, "color1");
		lua_pushnumber(L, cgol.colour2.Pack());
		lua_setfield(L, -2, "color2");
		lua_rawseti(L, -2, ++i);
	}
	return 1;
}

static int addCustomGol(lua_State *L)
{
	auto &sd = SimulationData::CRef();
	int rule;
	String ruleString;
	if (lua_isnumber(L, 1))
	{
		rule = luaL_checkinteger(L, 1);
		ruleString = SerialiseGOLRule(rule);
		rule = ParseGOLString(ruleString);
	}
	else
	{
		ruleString = tpt_lua_checkString(L, 1);
		rule = ParseGOLString(ruleString);
	}
	String nameString = tpt_lua_checkString(L, 2);
	unsigned int color1 = luaL_checkinteger(L, 3);
	unsigned int color2 = luaL_checkinteger(L, 4);

	if (nameString.empty() || !ValidateGOLName(nameString))
		return luaL_error(L, "Invalid name provided");
	if (rule == -1)
		return luaL_error(L, "Invalid rule provided");
	if (sd.GetCustomGOLByRule(rule))
		return luaL_error(L, "This Custom GoL rule already exists");

	auto *lsi = GetLSI();
	if (!lsi->gameModel->AddCustomGol(ruleString, nameString, RGB<uint8_t>::Unpack(color1), RGB<uint8_t>::Unpack(color2)))
		return luaL_error(L, "Duplicate name, cannot add");
	return 0;
}

static int removeCustomGol(lua_State *L)
{
	auto *lsi = GetLSI();
	ByteString nameString = tpt_lua_checkByteString(L, 1);
	bool removedAny = lsi->gameModel->RemoveCustomGol("DEFAULT_PT_LIFECUST_" + nameString);
	lua_pushboolean(L, removedAny);
	return 1;
}

static int lastUpdatedID(lua_State *L)
{
	auto *lsi = GetLSI();
	if (lsi->sim->debug_mostRecentlyUpdated != -1)
	{
		lua_pushinteger(L, lsi->sim->debug_mostRecentlyUpdated);
	}
	else
	{
		lua_pushnil(L);
	}
	return 1;
}

static int updateUpTo(lua_State *L)
{
	// sim.updateUpTo dispatches an update to the range [current, upTo], but GameModel::UpdateUpTo takes a range [current, upTo).
	// As a result, upTo here will be one smaller than it's logical for the duration of this function.
	int upTo = NPART - 1;
	if (lua_gettop(L) > 0)
	{
		upTo = luaL_checkinteger(L, 1);
	}
	if (upTo < -1 || upTo >= NPART) // -1 instead of 0 to allow for the empty range [0, -1] aka [0, 0)
	{
		return luaL_error(L, "ID not in valid range");
	}
	auto *lsi = GetLSI();
	lsi->sim->framerender = 1;
	lsi->gameModel->UpdateUpTo(upTo + 1);
	return 0;
}

static int temperatureScale(lua_State *L)
{
	auto *lsi = GetLSI();
	if (lua_gettop(L) == 0)
	{
		lua_pushinteger(L, lsi->gameModel->GetTemperatureScale());
		return 1;
	}
	int temperatureScale = luaL_checkinteger(L, 1);
	if (temperatureScale < 0 || temperatureScale > 2)
		return luaL_error(L, "Invalid temperature scale");
	lsi->gameModel->SetTemperatureScale(temperatureScale);
	return 0;
}

static int signsIndex(lua_State *L)
{
	ByteString key = tpt_lua_checkByteString(L, 2);

	//Get Raw Index value for element. Maybe there is a way to get the sign index some other way?
	lua_pushliteral(L, "id");
	lua_rawget(L, 1);
	int id = lua_tointeger(L, lua_gettop(L))-1;

	if (id < 0 || id >= MAXSIGNS)
	{
		luaL_error(L, "Invalid sign ID (stop messing with things): %i", id);
		return 0;
	}
	auto *lsi = GetLSI();
	auto *sim = lsi->sim;
	if (id >= (int)sim->signs.size())
	{
		return lua_pushnil(L), 1;
	}

	int x, y, w, h;
	if (byteStringEqualsLiteral(key, "text"))
		return tpt_lua_pushString(L, sim->signs[id].text), 1;
	else if (byteStringEqualsLiteral(key, "displayText"))
		return tpt_lua_pushString(L, sim->signs[id].getDisplayText(sim, x, y, w, h, false)), 1;
	else if (byteStringEqualsLiteral(key, "justification"))
		return lua_pushnumber(L, (int)sim->signs[id].ju), 1;
	else if (byteStringEqualsLiteral(key, "x"))
		return lua_pushnumber(L, sim->signs[id].x), 1;
	else if (byteStringEqualsLiteral(key, "y"))
		return lua_pushnumber(L, sim->signs[id].y), 1;
	else if (byteStringEqualsLiteral(key, "screenX"))
	{
		sim->signs[id].getDisplayText(sim, x, y, w, h);
		lua_pushnumber(L, x);
		return 1;
	}
	else if (byteStringEqualsLiteral(key, "screenY"))
	{
		sim->signs[id].getDisplayText(sim, x, y, w, h);
		lua_pushnumber(L, y);
		return 1;
	}
	else if (byteStringEqualsLiteral(key, "width"))
	{
		sim->signs[id].getDisplayText(sim, x, y, w, h);
		lua_pushnumber(L, w);
		return 1;
	}
	else if (byteStringEqualsLiteral(key, "height"))
	{
		sim->signs[id].getDisplayText(sim, x, y, w, h);
		lua_pushnumber(L, h);
		return 1;
	}
	else
		return lua_pushnil(L), 1;
}

static int signsNewIndex(lua_State *L)
{
	auto *lsi = GetLSI();
	auto *sim = lsi->sim;
	ByteString key = tpt_lua_checkByteString(L, 2);

	//Get Raw Index value for element. Maybe there is a way to get the sign index some other way?
	lua_pushliteral(L, "id");
	lua_rawget(L, 1);
	int id = lua_tointeger(L, lua_gettop(L))-1;

	if (id < 0 || id >= MAXSIGNS)
	{
		luaL_error(L, "Invalid sign ID (stop messing with things)");
		return 0;
	}
	if (id >= (int)sim->signs.size())
	{
		luaL_error(L, "Sign doesn't exist");
	}

	if (byteStringEqualsLiteral(key, "text"))
	{
		auto temp = tpt_lua_checkString(L, 3);
		String cleaned = format::CleanString(temp, false, true, true).Substr(0, 45);
		if (!cleaned.empty())
			sim->signs[id].text = cleaned;
		else
			luaL_error(L, "Text is empty");
		return 0;
	}
	else if (byteStringEqualsLiteral(key, "justification"))
	{
		int ju = luaL_checkinteger(L, 3);
		if (ju >= 0 && ju <= 3)
			return sim->signs[id].ju = (sign::Justification)ju, 1;
		else
			luaL_error(L, "Invalid justification");
		return 0;
	}
	else if (byteStringEqualsLiteral(key, "x"))
	{
		int x = luaL_checkinteger(L, 3);
		if (x >= 0 && x < XRES)
			return sim->signs[id].x = x, 1;
		else
			luaL_error(L, "Invalid X coordinate");
		return 0;
	}
	else if (byteStringEqualsLiteral(key, "y"))
	{
		int y = luaL_checkinteger(L, 3);
		if (y >= 0 && y < YRES)
			return sim->signs[id].y = y, 1;
		else
			luaL_error(L, "Invalid Y coordinate");
		return 0;
	}
	else if (byteStringEqualsLiteral(key, "displayText") ||
	         byteStringEqualsLiteral(key, "screenX") ||
	         byteStringEqualsLiteral(key, "screenY") ||
	         byteStringEqualsLiteral(key, "width") ||
	         byteStringEqualsLiteral(key, "height"))
	{
		luaL_error(L, "That property can't be directly set");
	}
	return 0;
}

// Creates a new sign at the first open index
static int Sign_new(lua_State *L)
{
	auto *lsi = GetLSI();
	if (lsi->sim->signs.size() >= MAXSIGNS)
		return lua_pushnil(L), 1;

	String text = format::CleanString(tpt_lua_checkString(L, 1), false, true, true).Substr(0, 45);
	int x = luaL_checkinteger(L, 2);
	int y = luaL_checkinteger(L, 3);
	int ju = luaL_optinteger(L, 4, 1);
	if (ju < 0 || ju > 3)
		return luaL_error(L, "Invalid justification");
	if (x < 0 || x >= XRES)
		return luaL_error(L, "Invalid X coordinate");
	if (y < 0 || y >= YRES)
		return luaL_error(L, "Invalid Y coordinate");

	lsi->sim->signs.push_back(sign(text, x, y, (sign::Justification)ju));

	lua_pushinteger(L, lsi->sim->signs.size());
	return 1;
}

// Deletes a sign
static int Sign_delete(lua_State *L)
{
	auto *lsi = GetLSI();
	int signID = luaL_checkinteger(L, 1);
	if (signID <= 0 || signID > (int)lsi->sim->signs.size())
		return luaL_error(L, "Sign doesn't exist");

	lsi->sim->signs.erase(lsi->sim->signs.begin()+signID-1);
	return 1;
}

static int resetVelocity(lua_State *L)
{
	int nx, ny;
	int x1, y1, width, height;
	x1 = abs(luaL_optint(L, 1, 0));
	y1 = abs(luaL_optint(L, 2, 0));
	width = abs(luaL_optint(L, 3, XCELLS));
	height = abs(luaL_optint(L, 4, YCELLS));
	if(x1 > XCELLS-1)
		x1 = XCELLS-1;
	if(y1 > YCELLS-1)
		y1 = YCELLS-1;
	if(x1+width > XCELLS-1)
		width = XCELLS-x1;
	if(y1+height > YCELLS-1)
		height = YCELLS-y1;
	auto *lsi = GetLSI();
	for (nx = x1; nx<x1+width; nx++)
		for (ny = y1; ny<y1+height; ny++)
		{
			lsi->sim->vx[ny][nx] = 0;
			lsi->sim->vy[ny][nx] = 0;
		}
	return 0;
}

static int resetSpark(lua_State *L)
{
	auto *lsi = GetLSI();
	lsi->gameController->ResetSpark();
	return 0;
}

static int randomSeed(lua_State *L)
{
	auto *lsi = GetLSI();
	if (lua_gettop(L))
	{
		lsi->sim->rng.state({
			uint32_t(luaL_checkinteger(L, 1)) | (uint64_t(uint32_t(luaL_checkinteger(L, 2))) << 32),
			uint32_t(luaL_checkinteger(L, 3)) | (uint64_t(uint32_t(luaL_checkinteger(L, 4))) << 32),
		});
		return 0;
	}
	auto s = lsi->sim->rng.state();
	lua_pushinteger(L,  s[0]        & UINT32_C(0xFFFFFFFF));
	lua_pushinteger(L, (s[0] >> 32) & UINT32_C(0xFFFFFFFF));
	lua_pushinteger(L,  s[1]        & UINT32_C(0xFFFFFFFF));
	lua_pushinteger(L, (s[1] >> 32) & UINT32_C(0xFFFFFFFF));
	return 4;
}

static int hash(lua_State *L)
{
	auto *lsi = GetLSI();
	lua_pushinteger(L, lsi->sim->CreateSnapshot()->Hash());
	return 1;
}

static int ensureDeterminism(lua_State *L)
{
	auto *lsi = GetLSI();
	if (lua_gettop(L))
	{
		lsi->sim->ensureDeterminism = lua_toboolean(L, 1);
		return 0;
	}
	lua_pushboolean(L, lsi->sim->ensureDeterminism);
	return 1;
}

void LuaSimulation::Open(lua_State *L)
{
	auto *lsi = GetLSI();
	auto &sd = SimulationData::CRef();
	static const luaL_Reg reg[] = {
#define LFUNC(v) { #v, v }
		LFUNC(partNeighbors),
		LFUNC(partChangeType),
		LFUNC(partCreate),
		LFUNC(partProperty),
		LFUNC(partPosition),
		LFUNC(partID),
		LFUNC(partKill),
		LFUNC(partExists),
		LFUNC(pressure),
		LFUNC(ambientHeat),
		LFUNC(ambientHeatSim),
		LFUNC(heatSim),
		LFUNC(newtonianGravity),
		LFUNC(velocityX),
		LFUNC(velocityY),
		LFUNC(createParts),
		LFUNC(createLine),
		LFUNC(createBox),
		LFUNC(floodParts),
		LFUNC(createWalls),
		LFUNC(createWallLine),
		LFUNC(createWallBox),
		LFUNC(floodWalls),
		LFUNC(toolBrush),
		LFUNC(toolLine),
		LFUNC(toolBox),
		LFUNC(decoBrush),
		LFUNC(decoLine),
		LFUNC(decoBox),
		LFUNC(decoColor),
		LFUNC(floodDeco),
		LFUNC(clearSim),
		LFUNC(clearRect),
		LFUNC(resetTemp),
		LFUNC(resetPressure),
		LFUNC(saveStamp),
		LFUNC(loadStamp),
		LFUNC(deleteStamp),
		LFUNC(listStamps),
		LFUNC(loadSave),
		LFUNC(reloadSave),
		LFUNC(getSaveID),
		LFUNC(adjustCoords),
		LFUNC(prettyPowders),
		LFUNC(gravityGrid),
		LFUNC(edgeMode),
		LFUNC(gravityMode),
		LFUNC(customGravity),
		LFUNC(airMode),
		LFUNC(waterEqualization),
		LFUNC(ambientAirTemp),
		LFUNC(elementCount),
		LFUNC(canMove),
		LFUNC(brush),
		LFUNC(parts),
		LFUNC(pmap),
		LFUNC(photons),
		LFUNC(neighbors),
		LFUNC(frameRender),
		LFUNC(golSpeedRatio),
		LFUNC(takeSnapshot),
		LFUNC(historyRestore),
		LFUNC(historyForward),
		LFUNC(replaceModeFlags),
		LFUNC(listCustomGol),
		LFUNC(addCustomGol),
		LFUNC(removeCustomGol),
		LFUNC(lastUpdatedID),
		LFUNC(updateUpTo),
		LFUNC(temperatureScale),
		LFUNC(randomSeed),
		LFUNC(hash),
		LFUNC(ensureDeterminism),
		LFUNC(paused),
		LFUNC(gravityMass),
		LFUNC(gravityField),
		LFUNC(resetSpark),
		LFUNC(resetVelocity),
		LFUNC(wallMap),
		LFUNC(elecMap),
		LFUNC(partCount),
		LFUNC(decoSpace),
		LFUNC(fanVelocityX),
		LFUNC(fanVelocityY),
#undef LFUNC
		{ NULL, NULL }
	};
	lua_newtable(L);
	luaL_register(L, NULL, reg);

#define LCONST(v) lua_pushinteger(L, int(v)); lua_setfield(L, -2, #v)
#define LCONSTF(v) lua_pushnumber(L, float(v)); lua_setfield(L, -2, #v)
#define LCONSTAS(k, v) lua_pushinteger(L, int(v)); lua_setfield(L, -2, k)

	LCONST(CELL);
	LCONST(XCELLS);
	LCONST(YCELLS);
	LCONST(NCELL);
	LCONST(XRES);
	LCONST(YRES);
	LCONST(XCNTR);
	LCONST(YCNTR);
	LCONSTAS("MAX_PARTS", NPART);
	LCONST(NT);
	LCONST(ST);
	LCONSTF(ITH);
	LCONSTF(ITL);
	LCONSTF(IPH);
	LCONSTF(IPL);
	LCONST(PT_NUM);
	LCONSTF(R_TEMP);
	LCONSTF(MAX_TEMP);
	LCONSTF(MIN_TEMP);
	LCONSTF(MAX_PRESSURE);
	LCONSTF(MIN_PRESSURE);
	LCONST(ISTP);
	LCONSTF(CFDS);
	LCONSTF(MAX_VELOCITY);

	LCONST(DECO_DRAW);
	LCONST(DECO_CLEAR);
	LCONST(DECO_ADD);
	LCONST(DECO_SUBTRACT);
	LCONST(DECO_MULTIPLY);
	LCONST(DECO_DIVIDE);
	LCONST(DECO_SMUDGE);

	LCONST(FLAG_STAGNANT);
	LCONST(FLAG_SKIPMOVE);
	LCONST(FLAG_MOVABLE);
	LCONST(FLAG_PHOTDECO);

	LCONST(PMAPBITS);
	LCONST(PMAPMASK);

	LCONST(BRUSH_CIRCLE);
	LCONST(BRUSH_SQUARE);
	LCONST(BRUSH_TRIANGLE);
	LCONST(NUM_DEFAULTBRUSHES);
	LCONSTAS("NUM_BRUSHES", lsi->gameModel->BrushListSize());

	LCONST(EDGE_VOID);
	LCONST(EDGE_SOLID);
	LCONST(EDGE_LOOP);
	LCONST(NUM_EDGEMODES);

	LCONST(AIR_ON);
	LCONST(AIR_PRESSUREOFF);
	LCONST(AIR_VELOCITYOFF);
	LCONST(AIR_OFF);
	LCONST(AIR_NOUPDATE);
	LCONST(NUM_AIRMODES);

	LCONST(GRAV_VERTICAL);
	LCONST(GRAV_OFF);
	LCONST(GRAV_RADIAL);
	LCONST(GRAV_CUSTOM);
	LCONST(NUM_GRAVMODES);

	LCONST(DECOSPACE_SRGB);
	LCONST(DECOSPACE_LINEAR);
	LCONST(DECOSPACE_GAMMA22);
	LCONST(DECOSPACE_GAMMA18);
	LCONST(NUM_DECOSPACES);

	LCONSTAS("CANMOVE_BOUNCE", 0);
	LCONSTAS("CANMOVE_SWAP", 1);
	LCONSTAS("CANMOVE_ENTER", 2);
	LCONSTAS("CANMOVE_BUILTIN", 3);
	LCONSTAS("NUM_CANMOVEMODES", 4);

	{
		lua_newtable(L);
		for (int i = 0; i < UI_WALLCOUNT; i++)
		{
			tpt_lua_pushByteString(L, sd.wtypes[i].identifier);
			lua_pushinteger(L, i);
			lua_settable(L, -3);
			lua_pushinteger(L, i);
			tpt_lua_pushByteString(L, sd.wtypes[i].identifier);
			lua_settable(L, -3);
		}
		lua_setfield(L, -2, "walls");
		LCONSTAS("NUM_WALLS", UI_WALLCOUNT);
	}

#undef LCONSTAS
#undef LCONSTF
#undef LCONST

	{
		int particlePropertiesCount = 0;
		for (auto &prop : Particle::GetProperties())
		{
			tpt_lua_pushByteString(L, "FIELD_" + prop.Name.ToUpper());
			lua_pushinteger(L, particlePropertiesCount++);
			lua_settable(L, -3);
		}
		for (auto &alias : Particle::GetPropertyAliases())
		{
			tpt_lua_pushByteString(L, "FIELD_" + alias.from.ToUpper());
			tpt_lua_pushByteString(L, "FIELD_" + alias.to.ToUpper());
			lua_gettable(L, -3);
			lua_settable(L, -3);
		}
	}
	{
		lua_newtable(L);
		for (int i = 1; i <= MAXSIGNS; i++)
		{
			lua_newtable(L);
			lua_pushinteger(L, i); //set "id" to table index
			lua_setfield(L, -2, "id");
			lua_newtable(L);
			lua_pushcfunction(L, signsIndex);
			lua_setfield(L, -2, "__index");
			lua_pushcfunction(L, signsNewIndex);
			lua_setfield(L, -2, "__newindex");
			lua_setmetatable(L, -2);
			lua_pushinteger(L, i); //table index
			lua_insert(L, -2); //swap k and v
			lua_settable(L, -3); //set metatable to signs[i]
		}
		lua_pushcfunction(L, Sign_new);
		lua_setfield(L, -2, "new");
		lua_pushcfunction(L, Sign_delete);
		lua_setfield(L, -2, "delete");
#define LCONSTAS(k, v) lua_pushinteger(L, int(v)); lua_setfield(L, -2, k)
		LCONSTAS("JUSTMODE_LEFT"  , sign::Left),
		LCONSTAS("JUSTMODE_MIDDLE", sign::Middle),
		LCONSTAS("JUSTMODE_RIGHT" , sign::Right),
		LCONSTAS("JUSTMODE_NONE"  , sign::None),
		LCONSTAS("NUM_JUSTMODES"  , sign::Max),
		LCONSTAS("MAX_SIGNS"      , MAXSIGNS),
#undef LCONSTAS
		lua_setfield(L, -2, "signs");
	}
	lua_pushvalue(L, -1);
	lua_setglobal(L, "simulation");
	lua_setglobal(L, "sim");
}
