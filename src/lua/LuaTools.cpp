#include "LuaScriptInterface.h"
#include "gui/game/GameModel.h"
#include "simulation/SimTool.h"
#include "simulation/Simulation.h"
#include <type_traits>

static int allocate(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TSTRING);
	luaL_checktype(L, 2, LUA_TSTRING);
	auto group = tpt_lua_toByteString(L, 1).ToUpper();
	auto name = tpt_lua_toByteString(L, 2).ToUpper();
	if (name.Contains("_"))
	{
		return luaL_error(L, "The tool name may not contain '_'.");
	}
	if (group.Contains("_"))
	{
		return luaL_error(L, "The group name may not contain '_'.");
	}
	if (group == "DEFAULT")
	{
		return luaL_error(L, "You cannot create tools in the 'DEFAULT' group.");
	}
	auto *lsi = GetLSI();
	auto identifier = group + "_TOOL_" + name;
	{
		SimTool tool;
		tool.Identifier = identifier;
		lsi->gameModel->AllocTool(std::make_unique<SimTool>(tool));
	}
	lsi->gameModel->BuildMenus();
	auto index = *lsi->gameModel->GetToolIndex(lsi->gameModel->GetToolFromIdentifier(identifier));
	lsi->customTools.resize(std::max(int(lsi->customTools.size()), index + 1));
	lsi->customTools[index].valid = true;
	lua_pushinteger(L, index);
	return 1;
}

static bool IsCustom(int index)
{
	auto *lsi = GetLSI();
	return index >= 0 && index < int(lsi->customTools.size()) && lsi->customTools[index].valid;
}

static int ffree(lua_State *L)
{
	int index = luaL_checkinteger(L, 1);
	auto *lsi = GetLSI();
	auto *tool = lsi->gameModel->GetToolByIndex(index);
	if (!tool)
	{
		return luaL_error(L, "Invalid tool");
	}
	if (!IsCustom(index))
	{
		return luaL_error(L, "Can only free custom tools");
	}
	lsi->customTools[index] = {};
	lsi->gameModel->FreeTool(tool);
	lsi->gameModel->BuildMenus();
	return 0;
}

static int luaPerformWrapper(SimTool *tool, Simulation *sim, Particle *cpart, int x, int y, int brushX, int brushY, float strength)
{
	int ok = 0;
	auto *lsi = GetLSI();
	auto L = lsi->L;
	auto index = *lsi->gameModel->GetToolIndex(tool);
	auto &customTools = lsi->customTools;
	if (customTools[index].perform)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, customTools[index].perform);
		if (cpart)
		{
			lua_pushinteger(L, cpart - &lsi->sim->parts[0]);
		}
		else
		{
			lua_pushnil(L);
		}
		lua_pushinteger(L, x);
		lua_pushinteger(L, y);
		lua_pushnumber(L, tool->Strength);
		lua_pushboolean(L, tool->shiftBehaviour);
		lua_pushboolean(L, tool->ctrlBehaviour);
		lua_pushboolean(L, tool->altBehaviour);
		lua_pushinteger(L, brushX);
		lua_pushinteger(L, brushY);
		if (tpt_lua_pcall(L, 9, 1, 0, eventTraitNone))
		{
			lsi->Log(CommandInterface::LogError, "In perform func: " + LuaGetError());
			lua_pop(L, 1);
		}
		else
		{
			if (lua_isboolean(L, -1))
			{
				ok = lua_toboolean(L, -1);
			}
			lua_pop(L, 1);
		}
	}
	return ok;
}

static void luaClickWrapper(SimTool *tool, Simulation *sim, const Brush &brush, ui::Point position)
{
	auto *lsi = GetLSI();
	auto L = lsi->L;
	auto index = *lsi->gameModel->GetToolIndex(tool);
	auto &customTools = lsi->customTools;
	if (customTools[index].click)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, customTools[index].click);
		lua_pushinteger(L, lsi->gameModel->GetBrushIndex(brush));
		lua_pushinteger(L, position.X);
		lua_pushinteger(L, position.Y);
		lua_pushnumber(L, tool->Strength);
		lua_pushboolean(L, tool->shiftBehaviour);
		lua_pushboolean(L, tool->ctrlBehaviour);
		lua_pushboolean(L, tool->altBehaviour);
		if (tpt_lua_pcall(L, 7, 0, 0, eventTraitNone))
		{
			lsi->Log(CommandInterface::LogError, "In click func: " + LuaGetError());
			lua_pop(L, 1);
		}
	}
}

static void luaDragWrapper(SimTool *tool, Simulation *sim, const Brush &brush, ui::Point position1, ui::Point position2)
{
	auto *lsi = GetLSI();
	auto L = lsi->L;
	auto index = *lsi->gameModel->GetToolIndex(tool);
	auto &customTools = lsi->customTools;
	if (customTools[index].drag)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, customTools[index].drag);
		lua_pushinteger(L, lsi->gameModel->GetBrushIndex(brush));
		lua_pushinteger(L, position1.X);
		lua_pushinteger(L, position1.Y);
		lua_pushinteger(L, position2.X);
		lua_pushinteger(L, position2.Y);
		lua_pushnumber(L, tool->Strength);
		lua_pushboolean(L, tool->shiftBehaviour);
		lua_pushboolean(L, tool->ctrlBehaviour);
		lua_pushboolean(L, tool->altBehaviour);
		if (tpt_lua_pcall(L, 9, 0, 0, eventTraitNone))
		{
			lsi->Log(CommandInterface::LogError, "In drag func: " + LuaGetError());
			lua_pop(L, 1);
		}
	}
}

static void luaDrawWrapper(SimTool *tool, Simulation *sim, const Brush &brush, ui::Point position)
{
	auto *lsi = GetLSI();
	auto L = lsi->L;
	auto index = *lsi->gameModel->GetToolIndex(tool);
	auto &customTools = lsi->customTools;
	if (customTools[index].draw)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, customTools[index].draw);
		lua_pushinteger(L, lsi->gameModel->GetBrushIndex(brush));
		lua_pushinteger(L, position.X);
		lua_pushinteger(L, position.Y);
		lua_pushnumber(L, tool->Strength);
		lua_pushboolean(L, tool->shiftBehaviour);
		lua_pushboolean(L, tool->ctrlBehaviour);
		lua_pushboolean(L, tool->altBehaviour);
		if (tpt_lua_pcall(L, 7, 0, 0, eventTraitNone))
		{
			lsi->Log(CommandInterface::LogError, "In draw func: " + LuaGetError());
			lua_pop(L, 1);
		}
	}
}

static void luaDrawLineWrapper(SimTool *tool, Simulation *sim, const Brush &brush, ui::Point position1, ui::Point position2, bool dragging)
{
	auto *lsi = GetLSI();
	auto L = lsi->L;
	auto index = *lsi->gameModel->GetToolIndex(tool);
	auto &customTools = lsi->customTools;
	if (customTools[index].drawLine)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, customTools[index].drawLine);
		lua_pushinteger(L, lsi->gameModel->GetBrushIndex(brush));
		lua_pushinteger(L, position1.X);
		lua_pushinteger(L, position1.Y);
		lua_pushinteger(L, position2.X);
		lua_pushinteger(L, position2.Y);
		lua_pushnumber(L, tool->Strength);
		lua_pushboolean(L, tool->shiftBehaviour);
		lua_pushboolean(L, tool->ctrlBehaviour);
		lua_pushboolean(L, tool->altBehaviour);
		if (tpt_lua_pcall(L, 9, 0, 0, eventTraitNone))
		{
			lsi->Log(CommandInterface::LogError, "In drawLine func: " + LuaGetError());
			lua_pop(L, 1);
		}
	}
}

static void luaDrawRectWrapper(SimTool *tool, Simulation *sim, const Brush &brush, ui::Point position1, ui::Point position2)
{
	auto *lsi = GetLSI();
	auto L = lsi->L;
	auto index = *lsi->gameModel->GetToolIndex(tool);
	auto &customTools = lsi->customTools;
	if (customTools[index].drawRect)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, customTools[index].drawRect);
		lua_pushinteger(L, lsi->gameModel->GetBrushIndex(brush));
		lua_pushinteger(L, position1.X);
		lua_pushinteger(L, position1.Y);
		lua_pushinteger(L, position2.X);
		lua_pushinteger(L, position2.Y);
		lua_pushnumber(L, tool->Strength);
		lua_pushboolean(L, tool->shiftBehaviour);
		lua_pushboolean(L, tool->ctrlBehaviour);
		lua_pushboolean(L, tool->altBehaviour);
		if (tpt_lua_pcall(L, 9, 0, 0, eventTraitNone))
		{
			lsi->Log(CommandInterface::LogError, "In drawRect func: " + LuaGetError());
			lua_pop(L, 1);
		}
	}
}

static void luaDrawFillWrapper(SimTool *tool, Simulation *sim, const Brush &brush, ui::Point position)
{
	auto *lsi = GetLSI();
	auto L = lsi->L;
	auto index = *lsi->gameModel->GetToolIndex(tool);
	auto &customTools = lsi->customTools;
	if (customTools[index].drawFill)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, customTools[index].drawFill);
		lua_pushinteger(L, lsi->gameModel->GetBrushIndex(brush));
		lua_pushinteger(L, position.X);
		lua_pushinteger(L, position.Y);
		lua_pushnumber(L, tool->Strength);
		lua_pushboolean(L, tool->shiftBehaviour);
		lua_pushboolean(L, tool->ctrlBehaviour);
		lua_pushboolean(L, tool->altBehaviour);
		if (tpt_lua_pcall(L, 7, 0, 0, eventTraitNone))
		{
			lsi->Log(CommandInterface::LogError, "In drawFill func: " + LuaGetError());
			lua_pop(L, 1);
		}
	}
}

static void luaSelectWrapper(SimTool *tool, int toolSelection)
{
	auto *lsi = GetLSI();
	auto L = lsi->L;
	auto index = *lsi->gameModel->GetToolIndex(tool);
	auto &customTools = lsi->customTools;
	if (customTools[index].select)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, customTools[index].select);
		lua_pushinteger(L, toolSelection);
		if (tpt_lua_pcall(L, 1, 0, 0, eventTraitNone))
		{
			lsi->Log(CommandInterface::LogError, "In select func: " + LuaGetError());
			lua_pop(L, 1);
		}
	}
}

template <typename T>
struct DependentFalse : std::false_type
{
};

static int property(lua_State *L)
{
	auto *lsi = GetLSI();
	int index = luaL_checkinteger(L, 1);
	auto *tool = lsi->gameModel->GetToolByIndex(index);
	if (!tool)
	{
		return luaL_error(L, "Invalid tool");
	}
	if (lua_gettop(L) > 2 && !IsCustom(index))
	{
		return luaL_error(L, "Can only change properties of custom tools");
	}
	ByteString propertyName = tpt_lua_checkByteString(L, 2);
	auto handleCallback = [lsi, L, index, tool, &propertyName](
		auto customToolMember,
		auto simToolMember,
		auto wrapper,
		const char *luaPropertyName
	) {
		if (propertyName == luaPropertyName)
		{
			if (lua_gettop(L) > 2)
			{
				if (lua_type(L, 3) == LUA_TFUNCTION)
				{
					(lsi->customTools[index].*customToolMember).Assign(L, 3);
					(static_cast<SimTool *>(tool)->*simToolMember) = wrapper;
				}
				else if (lua_type(L, 3) == LUA_TBOOLEAN && !lua_toboolean(L, 3))
				{
					(lsi->customTools[index].*customToolMember).Clear();
					(static_cast<SimTool *>(tool)->*simToolMember) = SimTool().*simToolMember;
				}
				return true;
			}
			luaL_error(L, "Invalid tool property");
		}
		return false;
	};
	if (handleCallback(&CustomTool::perform , &SimTool::Perform        , luaPerformWrapper , "Perform" ) ||
	    handleCallback(&CustomTool::click   , &SimTool::PerformClick   , luaClickWrapper   , "Click"   ) ||
	    handleCallback(&CustomTool::drag    , &SimTool::PerformDrag    , luaDragWrapper    , "Drag"    ) ||
	    handleCallback(&CustomTool::draw    , &SimTool::PerformDraw    , luaDrawWrapper    , "Draw"    ) ||
	    handleCallback(&CustomTool::drawLine, &SimTool::PerformDrawLine, luaDrawLineWrapper, "DrawLine") ||
	    handleCallback(&CustomTool::drawRect, &SimTool::PerformDrawRect, luaDrawRectWrapper, "DrawRect") ||
	    handleCallback(&CustomTool::drawFill, &SimTool::PerformDrawFill, luaDrawFillWrapper, "DrawFill") ||
	    handleCallback(&CustomTool::select  , &SimTool::PerformSelect  , luaSelectWrapper  , "Select"  ))
	{
		return 0;
	}
	int returnValueCount = 0;
	auto handleProperty = [L, lsi, tool, &propertyName, &returnValueCount](auto simToolMember, const char *luaPropertyName, bool buildMenusIfChanged) {
		if (propertyName == luaPropertyName)
		{
			auto &thing = tool->*simToolMember;
			using PropertyType = std::remove_reference_t<decltype(thing)>;
			if (lua_gettop(L) > 2)
			{
				if      constexpr (std::is_same_v<PropertyType, String      >) thing = tpt_lua_checkString(L, 3);
				else if constexpr (std::is_same_v<PropertyType, bool        >) thing = lua_toboolean(L, 3);
				else if constexpr (std::is_same_v<PropertyType, int         >) thing = luaL_checkinteger(L, 3);
				else if constexpr (std::is_same_v<PropertyType, RGB<uint8_t>>) thing = RGB<uint8_t>::Unpack(luaL_checkinteger(L, 3));
				else static_assert(DependentFalse<PropertyType>::value);
				if (buildMenusIfChanged)
				{
					lsi->gameModel->BuildMenus();
				}
			}
			else
			{
				if      constexpr (std::is_same_v<PropertyType, String      >) tpt_lua_pushString(L, thing);
				else if constexpr (std::is_same_v<PropertyType, bool        >) lua_pushboolean(L, thing);
				else if constexpr (std::is_same_v<PropertyType, int         >) lua_pushinteger(L, thing);
				else if constexpr (std::is_same_v<PropertyType, RGB<uint8_t>>) lua_pushinteger(L, thing.Pack());
				else static_assert(DependentFalse<PropertyType>::value);
				returnValueCount = 1;
			}
			return true;
		}
		return false;
	};
	if (handleProperty(&SimTool::Name       , "Name"       ,  true) ||
	    handleProperty(&SimTool::Description, "Description",  true) ||
	    handleProperty(&SimTool::Colour     , "Colour"     ,  true) ||
	    handleProperty(&SimTool::Colour     , "Color"      ,  true) ||
	    handleProperty(&SimTool::MenuSection, "MenuSection",  true) ||
	    handleProperty(&SimTool::MenuVisible, "MenuVisible",  true))
	{
		return returnValueCount;
	}
	if (propertyName == "Identifier")
	{
		tpt_lua_pushByteString(L, tool->Identifier);
		return 1;
	}
	return luaL_error(L, "Invalid tool property");
}

static int exists(lua_State *L)
{
	int index = luaL_checkinteger(L, 1);
	auto *lsi = GetLSI();
	lua_pushboolean(L, bool(lsi->gameModel->GetToolByIndex(index)));
	return 1;
}

static int isCustom(lua_State *L)
{
	auto *lsi = GetLSI();
	int index = luaL_checkinteger(L, 1);
	auto *tool = lsi->gameModel->GetToolByIndex(index);
	if (!tool)
	{
		return luaL_error(L, "Invalid tool");
	}
	lua_pushboolean(L, IsCustom(index));
	return 1;
}

void LuaTools::Open(lua_State *L)
{
	auto *lsi = GetLSI();
	static const luaL_Reg reg[] = {
#define LFUNC(v) { #v, v }
		LFUNC(allocate),
		LFUNC(property),
		LFUNC(exists),
		LFUNC(isCustom),
#undef LFUNC
		{ "free", ffree },
		{ NULL, NULL }
	};
	lua_newtable(L);
	luaL_register(L, NULL, reg);
	lua_newtable(L);
	lua_setfield(L, -2, "index");
	lua_setglobal(L, "tools");
	auto &toolList = lsi->gameModel->GetTools();
	for (int i = 0; i < int(toolList.size()); ++i)
	{
		if (!toolList[i])
		{
			continue;
		}
		SetToolIndex(L, toolList[i]->Identifier, i);
	}
}

void LuaTools::SetToolIndex(lua_State *L, ByteString identifier, std::optional<int> index)
{
	lua_getglobal(L, "tools");
	lua_getfield(L, -1, "index");
	tpt_lua_pushByteString(L, identifier);
	if (index)
	{
		lua_pushinteger(L, *index);
	}
	else
	{
		lua_pushnil(L);
	}
	lua_settable(L, -3);
	lua_pop(L, 2);
}
