#include "LuaScriptInterface.h"
#include "gui/game/GameModel.h"
#include "simulation/ElementClasses.h"
#include "simulation/ElementCommon.h"
#include "simulation/SimulationData.h"
#include <mutex>

static void getDefaultProperties(lua_State *L, int id)
{
	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	lua_newtable(L);
	for (auto &prop : Particle::GetProperties())
	{
		auto propertyAddress = reinterpret_cast<intptr_t>((reinterpret_cast<const unsigned char*>(&elements[id].DefaultProperties)) + prop.Offset);
		tpt_lua_pushByteString(L, prop.Name);
		LuaGetProperty(L, prop, propertyAddress);
		lua_settable(L, -3);
	}
	for (auto &alias : Particle::GetPropertyAliases())
	{
		tpt_lua_pushByteString(L, alias.from);
		tpt_lua_pushByteString(L, alias.to);
		lua_gettable(L, -3);
		lua_settable(L, -3);
	}
}

static void setDefaultProperties(lua_State *L, int id, int stackPos)
{
	auto &sd = SimulationData::Ref();
	auto &elements = sd.elements;
	if (lua_type(L, stackPos) == LUA_TTABLE)
	{
		for (auto &prop : Particle::GetProperties())
		{
			tpt_lua_pushByteString(L, prop.Name);
			lua_gettable(L, stackPos);
			if (lua_type(L, -1) == LUA_TNIL)
			{
				for (auto &alias : Particle::GetPropertyAliases())
				{
					if (alias.to == prop.Name)
					{
						lua_pop(L, 1);
						tpt_lua_pushByteString(L, alias.from);
						lua_gettable(L, stackPos);
					}
				}
			}
			if (lua_type(L, -1) != LUA_TNIL)
			{
				auto propertyAddress = reinterpret_cast<intptr_t>((reinterpret_cast<unsigned char*>(&elements[id].DefaultProperties)) + prop.Offset);
				LuaSetProperty(L, prop, propertyAddress, -1);
			}
			lua_pop(L, 1);
		}
	}
}

static void manageElementIdentifier(lua_State *L, int id, bool add)
{
	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	if (elements[id].Enabled)
	{
		lua_getglobal(L, "elements");
		tpt_lua_pushByteString(L, elements[id].Identifier);
		if (add)
		{
			lua_pushinteger(L, id);
		}
		else
		{
			lua_pushnil(L);
		}
		lua_settable(L, -3);
		if (elements[id].Identifier.BeginsWith("DEFAULT_PT_"))
		{
			ByteString realIdentifier = ByteString::Build("DEFAULT_PT_", elements[id].Name.ToUtf8());
			if (id != 0 && id != PT_NBHL && id != PT_NWHL && elements[id].Identifier != realIdentifier)
			{
				tpt_lua_pushByteString(L, realIdentifier);
				if (add)
				{
					lua_pushinteger(L, id);
				}
				else
				{
					lua_pushnil(L);
				}
				lua_settable(L, -3);
			}
		}
		lua_pop(L, 1);
	}
}

static int luaUpdateWrapper(UPDATE_FUNC_ARGS)
{
	if (!sim->useLuaCallbacks)
	{
		return 0;
	}
	auto *lsi = GetLSI();
	auto &builtinElements = GetElements();
	auto *builtinUpdate = builtinElements[parts[i].type].Update;
	auto &customElements = lsi->customElements;
	if (builtinUpdate && customElements[parts[i].type].updateMode == UPDATE_AFTER)
	{
		if (builtinUpdate(UPDATE_FUNC_SUBCALL_ARGS))
			return 1;
		x = (int)(parts[i].x+0.5f);
		y = (int)(parts[i].y+0.5f);
	}
	if (customElements[parts[i].type].update)
	{
		int retval = 0, callret;
		lua_rawgeti(lsi->L, LUA_REGISTRYINDEX, customElements[parts[i].type].update);
		lua_pushinteger(lsi->L, i);
		lua_pushinteger(lsi->L, x);
		lua_pushinteger(lsi->L, y);
		lua_pushinteger(lsi->L, surround_space);
		lua_pushinteger(lsi->L, nt);
		callret = tpt_lua_pcall(lsi->L, 5, 1, 0, eventTraitSimRng);
		if (callret)
			lsi->Log(CommandInterface::LogError, LuaGetError());
		if(lua_isboolean(lsi->L, -1)){
			retval = lua_toboolean(lsi->L, -1);
		}
		lua_pop(lsi->L, 1);
		if (retval)
		{
			return 1;
		}
		x = (int)(parts[i].x+0.5f);
		y = (int)(parts[i].y+0.5f);
	}
	if (builtinUpdate && customElements[parts[i].type].updateMode == UPDATE_BEFORE)
	{
		if (builtinUpdate(UPDATE_FUNC_SUBCALL_ARGS))
			return 1;
		x = (int)(parts[i].x+0.5f);
		y = (int)(parts[i].y+0.5f);
	}
	return 0;
}

static int luaGraphicsWrapper(GRAPHICS_FUNC_ARGS)
{
	if (!gfctx.sim->useLuaCallbacks)
	{
		return Element::defaultGraphics(GRAPHICS_FUNC_SUBCALL_ARGS);
	}
	auto *lsi = GetLSI();
	auto &customElements = lsi->customElements;
	auto *sim = lsi->sim;
	if (customElements[cpart->type].graphics)
	{
		auto *pipeSubcallWcpart = gfctx.pipeSubcallCpart ? sim->parts + (gfctx.pipeSubcallCpart - gfctx.sim->parts) : nullptr;
		if (pipeSubcallWcpart)
		{
			std::swap(*pipeSubcallWcpart, *gfctx.pipeSubcallTpart);
			cpart = pipeSubcallWcpart;
		}
		int cache = 0, callret;
		int i = cpart - gfctx.sim->parts; // pointer arithmetic be like
		lua_rawgeti(lsi->L, LUA_REGISTRYINDEX, customElements[cpart->type].graphics);
		lua_pushinteger(lsi->L, i);
		lua_pushinteger(lsi->L, *colr);
		lua_pushinteger(lsi->L, *colg);
		lua_pushinteger(lsi->L, *colb);
		callret = tpt_lua_pcall(lsi->L, 4, 10, 0, eventTraitSimGraphics);
		if (callret)
		{
			lsi->Log(CommandInterface::LogError, LuaGetError());
			lua_pop(lsi->L, 1);
		}
		else
		{
			bool valid = true;
			for (int i = -10; i < 0; i++)
				if (!lua_isnumber(lsi->L, i) && !lua_isnil(lsi->L, i))
				{
					valid = false;
					break;
				}
			if (valid)
			{
				cache = luaL_optint(lsi->L, -10, 0);
				*pixel_mode = luaL_optint(lsi->L, -9, *pixel_mode);
				*cola = luaL_optint(lsi->L, -8, *cola);
				*colr = luaL_optint(lsi->L, -7, *colr);
				*colg = luaL_optint(lsi->L, -6, *colg);
				*colb = luaL_optint(lsi->L, -5, *colb);
				*firea = luaL_optint(lsi->L, -4, *firea);
				*firer = luaL_optint(lsi->L, -3, *firer);
				*fireg = luaL_optint(lsi->L, -2, *fireg);
				*fireb = luaL_optint(lsi->L, -1, *fireb);
			}
			lua_pop(lsi->L, 10);
		}
		if (pipeSubcallWcpart)
		{
			std::swap(*pipeSubcallWcpart, *gfctx.pipeSubcallTpart);
		}
		return cache;
	}
	return 0;
}

static void luaCreateWrapper(ELEMENT_CREATE_FUNC_ARGS)
{
	if (!sim->useLuaCallbacks)
	{
		return;
	}
	auto *lsi = GetLSI();
	auto &customElements = lsi->customElements;
	if (customElements[sim->parts[i].type].create)
	{
		lua_rawgeti(lsi->L, LUA_REGISTRYINDEX, customElements[sim->parts[i].type].create);
		lua_pushinteger(lsi->L, i);
		lua_pushinteger(lsi->L, x);
		lua_pushinteger(lsi->L, y);
		lua_pushinteger(lsi->L, t);
		lua_pushinteger(lsi->L, v);
		if (tpt_lua_pcall(lsi->L, 5, 0, 0, eventTraitSimRng))
		{
			lsi->Log(CommandInterface::LogError, "In create func: " + LuaGetError());
			lua_pop(lsi->L, 1);
		}
	}
}

static bool luaCreateAllowedWrapper(ELEMENT_CREATE_ALLOWED_FUNC_ARGS)
{
	if (!sim->useLuaCallbacks)
	{
		// Nothing really bad can happen, no callbacks are allowed anyway. The worst thing that can happen
		// is that a well-crafted save looks odd in previews because it has multiple Element::defaultGraphics-rendered
		// instances of something that should be limited to one instance.
		return 1;
	}
	auto *lsi = GetLSI();
	auto &customElements = lsi->customElements;
	bool ret = false;
	if (customElements[t].createAllowed)
	{
		lua_rawgeti(lsi->L, LUA_REGISTRYINDEX, customElements[t].createAllowed);
		lua_pushinteger(lsi->L, i);
		lua_pushinteger(lsi->L, x);
		lua_pushinteger(lsi->L, y);
		lua_pushinteger(lsi->L, t);
		if (tpt_lua_pcall(lsi->L, 4, 1, 0, eventTraitSimRng))
		{
			lsi->Log(CommandInterface::LogError, "In create allowed: " + LuaGetError());
			lua_pop(lsi->L, 1);
		}
		else
		{
			if (lua_isboolean(lsi->L, -1))
				ret = lua_toboolean(lsi->L, -1);
			lua_pop(lsi->L, 1);
		}
	}
	return ret;
}

static void luaChangeTypeWrapper(ELEMENT_CHANGETYPE_FUNC_ARGS)
{
	if (!sim->useLuaCallbacks)
	{
		return;
	}
	auto *lsi = GetLSI();
	auto &customElements = lsi->customElements;
	if (customElements[sim->parts[i].type].changeType)
	{
		lua_rawgeti(lsi->L, LUA_REGISTRYINDEX, customElements[sim->parts[i].type].changeType);
		lua_pushinteger(lsi->L, i);
		lua_pushinteger(lsi->L, x);
		lua_pushinteger(lsi->L, y);
		lua_pushinteger(lsi->L, from);
		lua_pushinteger(lsi->L, to);
		if (tpt_lua_pcall(lsi->L, 5, 0, 0, eventTraitSimRng))
		{
			lsi->Log(CommandInterface::LogError, "In change type: " + LuaGetError());
			lua_pop(lsi->L, 1);
		}
	}
}

static bool luaCtypeDrawWrapper(CTYPEDRAW_FUNC_ARGS)
{
	if (!sim->useLuaCallbacks)
	{
		return false;
	}
	auto *lsi = GetLSI();
	auto &customElements = lsi->customElements;
	bool ret = false;
	if (customElements[sim->parts[i].type].ctypeDraw)
	{
		lua_rawgeti(lsi->L, LUA_REGISTRYINDEX, customElements[sim->parts[i].type].ctypeDraw);
		lua_pushinteger(lsi->L, i);
		lua_pushinteger(lsi->L, t);
		lua_pushinteger(lsi->L, v);
		if (tpt_lua_pcall(lsi->L, 3, 1, 0, eventTraitSimRng))
		{
			lsi->Log(CommandInterface::LogError, LuaGetError());
			lua_pop(lsi->L, 1);
		}
		else
		{
			if (lua_isboolean(lsi->L, -1))
				ret = lua_toboolean(lsi->L, -1);
			lua_pop(lsi->L, 1);
		}
	}
	return ret;
}

static int allocate(lua_State *L)
{
	auto *lsi = GetLSI();
	luaL_checktype(L, 1, LUA_TSTRING);
	luaL_checktype(L, 2, LUA_TSTRING);
	auto group = tpt_lua_toByteString(L, 1).ToUpper();
	auto id = tpt_lua_toByteString(L, 2).ToUpper();

	if (id.Contains("_"))
	{
		return luaL_error(L, "The element name may not contain '_'.");
	}
	if (group.Contains("_"))
	{
		return luaL_error(L, "The group name may not contain '_'.");
	}
	if (group == "DEFAULT")
	{
		return luaL_error(L, "You cannot create elements in the 'DEFAULT' group.");
	}

	auto identifier = group + "_PT_" + id;

	int newID = -1;
	{
		auto &sd = SimulationData::CRef();
		auto &elements = sd.elements;
		for(int i = 0; i < PT_NUM; i++)
		{
			if(elements[i].Enabled && ByteString(elements[i].Identifier) == identifier)
				return luaL_error(L, "Element identifier already in use");
		}

		// Start out at 255 so that lua element IDs are still one byte (better save compatibility)
		for (int i = PT_NUM >= 255 ? 255 : PT_NUM; i >= 0; i--)
		{
			if (!elements[i].Enabled)
			{
				newID = i;
				break;
			}
		}
		// If not enough space, then we start with the new maimum ID
		if (newID == -1)
		{
			for (int i = PT_NUM-1; i >= 255; i--)
			{
				if (!elements[i].Enabled)
				{
					newID = i;
					break;
				}
			}
		}
	}

	if (newID != -1)
	{
		{
			auto &sd = SimulationData::Ref();
			std::unique_lock lk(sd.elementGraphicsMx);
			auto &elements = sd.elements;
			elements[newID] = Element();
			elements[newID].Enabled = true;
			elements[newID].Identifier = identifier;
		}

		lua_getglobal(L, "elements");
		tpt_lua_pushByteString(L, identifier);
		lua_pushinteger(L, newID);
		lua_settable(L, -3);
		lua_pop(L, 1);

		for (auto elem = 0; elem < PT_NUM; ++elem)
		{
			lsi->customCanMove[elem][newID] = 0;
			lsi->customCanMove[newID][elem] = 0;
		}
		lsi->gameModel->AllocElementTool(newID);
		lsi->gameModel->BuildMenus();
		lsi->InitCustomCanMove();
	}

	lua_pushinteger(L, newID);
	return 1;
}

static int element(lua_State *L)
{
	auto &builtinElements = GetElements();
	auto *lsi = GetLSI();
	auto &customElements = lsi->customElements;
	int id = luaL_checkinteger(L, 1);
	if (!SimulationData::CRef().IsElementOrNone(id))
	{
		return luaL_error(L, "Invalid element");
	}

	if (lua_gettop(L) > 1)
	{
		{
			auto &sd = SimulationData::Ref();
			std::unique_lock lk(sd.elementGraphicsMx);
			auto &elements = sd.elements;
			luaL_checktype(L, 2, LUA_TTABLE);
			//Write values from native data to a table
			for (auto &prop : Element::GetProperties())
			{
				tpt_lua_pushByteString(L, prop.Name);
				lua_gettable(L, -2);
				if (lua_type(L, -1) != LUA_TNIL)
				{
					intptr_t propertyAddress = (intptr_t)(((unsigned char*)&elements[id]) + prop.Offset);
					LuaSetProperty(L, prop, propertyAddress, -1);
				}
				lua_pop(L, 1);
			}

			lua_getfield(L, -1, "Update");
			if (lua_type(L, -1) == LUA_TFUNCTION)
			{
				customElements[id].update.Assign(L, -1);
				customElements[id].updateMode = UPDATE_AFTER;
				elements[id].Update = luaUpdateWrapper;
			}
			else if (lua_type(L, -1) == LUA_TBOOLEAN && !lua_toboolean(L, -1))
			{
				customElements[id].update.Clear();
				customElements[id].updateMode = UPDATE_AFTER;
				elements[id].Update = builtinElements[id].Update;
			}
			lua_pop(L, 1);

			lua_getfield(L, -1, "Graphics");
			if (lua_type(L, -1) == LUA_TFUNCTION)
			{
				customElements[id].graphics.Assign(L, -1);
				elements[id].Graphics = luaGraphicsWrapper;
			}
			else if (lua_type(L, -1) == LUA_TBOOLEAN && !lua_toboolean(L, -1))
			{
				customElements[id].graphics.Clear();
				elements[id].Graphics = builtinElements[id].Graphics;
			}
			lua_pop(L, 1);

			lua_getfield(L, -1, "Create");
			if (lua_type(L, -1) == LUA_TFUNCTION)
			{
				customElements[id].create.Assign(L, -1);
				elements[id].Create = luaCreateWrapper;
			}
			else if (lua_type(L, -1) == LUA_TBOOLEAN && !lua_toboolean(L, -1))
			{
				customElements[id].create.Clear();
				elements[id].Create = builtinElements[id].Create;
			}
			lua_pop(L, 1);

			lua_getfield(L, -1, "CreateAllowed");
			if (lua_type(L, -1) == LUA_TFUNCTION)
			{
				customElements[id].createAllowed.Assign(L, -1);
				elements[id].CreateAllowed = luaCreateAllowedWrapper;
			}
			else if (lua_type(L, -1) == LUA_TBOOLEAN && !lua_toboolean(L, -1))
			{
				customElements[id].createAllowed.Clear();
				elements[id].CreateAllowed = builtinElements[id].CreateAllowed;
			}
			lua_pop(L, 1);

			lua_getfield(L, -1, "ChangeType");
			if (lua_type(L, -1) == LUA_TFUNCTION)
			{
				customElements[id].changeType.Assign(L, -1);
				elements[id].ChangeType = luaChangeTypeWrapper;
			}
			else if (lua_type(L, -1) == LUA_TBOOLEAN && !lua_toboolean(L, -1))
			{
				customElements[id].changeType.Clear();
				elements[id].ChangeType = builtinElements[id].ChangeType;
			}
			lua_pop(L, 1);

			lua_getfield(L, -1, "CtypeDraw");
			if (lua_type(L, -1) == LUA_TFUNCTION)
			{
				customElements[id].ctypeDraw.Assign(L, -1);
				elements[id].CtypeDraw = luaCtypeDrawWrapper;
			}
			else if (lua_type(L, -1) == LUA_TBOOLEAN && !lua_toboolean(L, -1))
			{
				customElements[id].ctypeDraw.Clear();
				elements[id].CtypeDraw = builtinElements[id].CtypeDraw;
			}
			lua_pop(L, 1);

			lua_getfield(L, -1, "DefaultProperties");
			setDefaultProperties(L, id, lua_gettop(L));
			lua_pop(L, 1);

			sd.graphicscache[id].isready = 0;
		}
		lsi->gameModel->AllocElementTool(id);
		lsi->gameModel->BuildMenus();
		lsi->InitCustomCanMove();

		return 0;
	}
	else
	{
		auto &sd = SimulationData::CRef();
		auto &elements = sd.elements;
		//Write values from native data to a table
		lua_newtable(L);
		for (auto &prop : Element::GetProperties())
		{
			tpt_lua_pushByteString(L, prop.Name);
			intptr_t propertyAddress = (intptr_t)(((unsigned char*)&elements[id]) + prop.Offset);
			LuaGetProperty(L, prop, propertyAddress);
			lua_settable(L, -3);
		}

		tpt_lua_pushByteString(L, elements[id].Identifier);
		lua_setfield(L, -2, "Identifier");

		getDefaultProperties(L, id);
		lua_setfield(L, -2, "DefaultProperties");

		return 1;
	}
}

static int property(lua_State *L)
{
	auto &builtinElements = GetElements();
	auto *lsi = GetLSI();
	auto &customElements = lsi->customElements;
	int id = luaL_checkinteger(L, 1);
	if (!SimulationData::CRef().IsElementOrNone(id))
	{
		return luaL_error(L, "Invalid element");
	}
	ByteString propertyName = tpt_lua_checkByteString(L, 2);

	auto &properties = Element::GetProperties();
	auto prop = std::find_if(properties.begin(), properties.end(), [&propertyName](StructProperty const &p) {
		return p.Name == propertyName;
	});

	if (lua_gettop(L) > 2)
	{
		auto &sd = SimulationData::Ref();
		std::unique_lock lk(sd.elementGraphicsMx);
		auto &elements = sd.elements;
		if (prop != properties.end())
		{
			if (lua_type(L, 3) != LUA_TNIL)
			{
				if (prop->Type == StructProperty::TransitionType)
				{
					int type = luaL_checkinteger(L, 3);
					if (!SimulationData::CRef().IsElementOrNone(type) && type != NT && type != ST)
					{
						return luaL_error(L, "Invalid element");
					}
				}
				intptr_t propertyAddress = (intptr_t)(((unsigned char*)&elements[id]) + prop->Offset);
				manageElementIdentifier(L, id, false);
				LuaSetProperty(L, *prop, propertyAddress, 3);
				manageElementIdentifier(L, id, true);
				lsi->gameModel->AllocElementTool(id);
				lsi->gameModel->BuildMenus();
				lsi->InitCustomCanMove();
				sd.graphicscache[id].isready = 0;
			}
		}
		else if (propertyName == "Update")
		{
			if (lua_type(L, 3) == LUA_TFUNCTION)
			{
				switch (luaL_optint(L, 4, 0))
				{
				case 2:
					customElements[id].updateMode = UPDATE_BEFORE;
					break;

				case 1:
					customElements[id].updateMode = UPDATE_REPLACE;
					break;

				default:
					customElements[id].updateMode = UPDATE_AFTER;
					break;
				}
				customElements[id].update.Assign(L, 3);
				elements[id].Update = luaUpdateWrapper;
			}
			else if (lua_type(L, 3) == LUA_TBOOLEAN && !lua_toboolean(L, 3))
			{
				customElements[id].update.Clear();
				customElements[id].updateMode = UPDATE_AFTER;
				elements[id].Update = builtinElements[id].Update;
			}
		}
		else if (propertyName == "Graphics")
		{
			if (lua_type(L, 3) == LUA_TFUNCTION)
			{
				customElements[id].graphics.Assign(L, 3);
				elements[id].Graphics = luaGraphicsWrapper;
			}
			else if (lua_type(L, 3) == LUA_TBOOLEAN && !lua_toboolean(L, 3))
			{
				customElements[id].graphics.Clear();
				elements[id].Graphics = builtinElements[id].Graphics;
			}
			sd.graphicscache[id].isready = 0;
		}
		else if (propertyName == "Create")
		{
			if (lua_type(L, 3) == LUA_TFUNCTION)
			{
				customElements[id].create.Assign(L, 3);
				elements[id].Create = luaCreateWrapper;
			}
			else if (lua_type(L, 3) == LUA_TBOOLEAN && !lua_toboolean(L, 3))
			{
				customElements[id].create.Clear();
				elements[id].Create = builtinElements[id].Create;
			}
		}
		else if (propertyName == "CreateAllowed")
		{
			if (lua_type(L, 3) == LUA_TFUNCTION)
			{
				customElements[id].createAllowed.Assign(L, 3);
				elements[id].CreateAllowed = luaCreateAllowedWrapper;
			}
			else if (lua_type(L, 3) == LUA_TBOOLEAN && !lua_toboolean(L, 3))
			{
				customElements[id].createAllowed.Clear();
				elements[id].CreateAllowed = builtinElements[id].CreateAllowed;
			}
		}
		else if (propertyName == "ChangeType")
		{
			if (lua_type(L, 3) == LUA_TFUNCTION)
			{
				customElements[id].changeType.Assign(L, 3);
				elements[id].ChangeType = luaChangeTypeWrapper;
			}
			else if (lua_type(L, 3) == LUA_TBOOLEAN && !lua_toboolean(L, 3))
			{
				customElements[id].changeType.Clear();
				elements[id].ChangeType = builtinElements[id].ChangeType;
			}
		}
		else if (propertyName == "CtypeDraw")
		{
			if (lua_type(L, 3) == LUA_TFUNCTION)
			{
				customElements[id].ctypeDraw.Assign(L, 3);
				elements[id].CtypeDraw = luaCtypeDrawWrapper;
			}
			else if (lua_type(L, 3) == LUA_TBOOLEAN && !lua_toboolean(L, 3))
			{
				customElements[id].ctypeDraw.Clear();
				elements[id].CtypeDraw = builtinElements[id].CtypeDraw;
			}
		}
		else if (propertyName == "DefaultProperties")
		{
			setDefaultProperties(L, id, 3);
		}
		else
		{
			return luaL_error(L, "Invalid element property");
		}
		return 0;
	}
	else
	{
		auto &sd = SimulationData::CRef();
		auto &elements = sd.elements;
		if (prop != properties.end())
		{
			intptr_t propertyAddress = (intptr_t)(((const unsigned char*)&elements[id]) + prop->Offset);
			LuaGetProperty(L, *prop, propertyAddress);
			return 1;
		}
		else if (propertyName == "Identifier")
		{
			tpt_lua_pushByteString(L, elements[id].Identifier);
			return 1;
		}
		else if (propertyName == "DefaultProperties")
		{
			getDefaultProperties(L, id);
			return 1;
		}
		else
		{
			return luaL_error(L, "Invalid element property");
		}
	}
}

static int ffree(lua_State *L)
{
	int id = luaL_checkinteger(L, 1);
	ByteString identifier;
	{
		auto &sd = SimulationData::CRef();
		if (!sd.IsElement(id))
		{
			return luaL_error(L, "Invalid element");
		}

		identifier = sd.elements[id].Identifier;
		if (identifier.BeginsWith("DEFAULT_PT_"))
		{
			return luaL_error(L, "Cannot free default elements");
		}
	}

	auto *lsi = GetLSI();
	{
		auto &sd = SimulationData::Ref();
		std::unique_lock lk(sd.elementGraphicsMx);
		sd.elements[id].Enabled = false;
	}
	lsi->customElements[id] = {};
	lsi->gameModel->FreeTool(lsi->gameModel->GetToolFromIdentifier(identifier));
	lsi->gameModel->BuildMenus();

	lua_getglobal(L, "elements");
	tpt_lua_pushByteString(L, identifier);
	lua_pushnil(L);
	lua_settable(L, -3);
	lua_pop(L, 1);

	return 0;
}

static int exists(lua_State *L)
{
	auto &sd = SimulationData::CRef();
	lua_pushboolean(L, sd.IsElement(luaL_checkinteger(L, 1)));
	return 1;
}

static int loadDefault(lua_State *L)
{
	auto &sd = SimulationData::Ref();
	std::unique_lock lk(sd.elementGraphicsMx);
	auto &elements = sd.elements;
	auto &builtinElements = GetElements();
	auto *lsi = GetLSI();
	{
		auto loadDefaultOne = [lsi, L, &elements, &builtinElements](int id) {
			lua_getglobal(L, "elements");
			ByteString identifier = elements[id].Identifier;
			tpt_lua_pushByteString(L, identifier);
			lua_pushnil(L);
			lua_settable(L, -3);

			manageElementIdentifier(L, id, false);
			if (id < (int)builtinElements.size())
			{
				elements[id] = builtinElements[id];
			}
			else
			{
				elements[id] = Element();
				lsi->gameModel->FreeTool(lsi->gameModel->GetToolFromIdentifier(identifier));
			}
			manageElementIdentifier(L, id, true);

			tpt_lua_pushByteString(L, identifier);
			lua_pushinteger(L, id);
			lua_settable(L, -3);
			lua_pop(L, 1);
		};
		int args = lua_gettop(L);
		if (args)
		{
			luaL_checktype(L, 1, LUA_TNUMBER);
			int id = lua_tointeger(L, 1);
			if (id < 0 || id >= PT_NUM)
				return luaL_error(L, "Invalid element");
			loadDefaultOne(id);
		}
		else
		{
			for (int i = 0; i < PT_NUM; i++)
			{
				loadDefaultOne(i);
			}
		}
	}

	lsi->gameModel->BuildMenus();
	for (auto moving = 0; moving < PT_NUM; ++moving)
	{
		for (auto into = 0; into < PT_NUM; ++into)
		{
			lsi->customCanMove[moving][into] = 0;
		}
	}
	lsi->InitCustomCanMove();
	sd.graphicscache = std::array<gcache_item, PT_NUM>();
	return 0;
}

static int getByName(lua_State *L)
{
	lua_pushinteger(L, SimulationData::CRef().GetParticleType(tpt_lua_checkByteString(L, 1)));
	return 1;
}

void LuaElements::Open(lua_State *L)
{
	auto &sd = SimulationData::CRef();
	static const luaL_Reg reg[] = {
#define LFUNC(v) { #v, v }
		LFUNC(allocate),
		LFUNC(element),
		LFUNC(property),
		LFUNC(exists),
		LFUNC(loadDefault),
		LFUNC(getByName),
#undef LFUNC
		{ "free", ffree },
		{ NULL, NULL }
	};
	lua_newtable(L);
	luaL_register(L, NULL, reg);
#define LCONST(v) lua_pushinteger(L, int(v)); lua_setfield(L, -2, #v)
#define LCONSTAS(k, v) lua_pushinteger(L, int(v)); lua_setfield(L, -2, k)
	LCONST(TYPE_PART);
	LCONST(TYPE_LIQUID);
	LCONST(TYPE_SOLID);
	LCONST(TYPE_GAS);
	LCONST(TYPE_ENERGY);
	LCONST(PROP_CONDUCTS);
	LCONST(PROP_PHOTPASS);
	LCONST(PROP_NEUTPENETRATE);
	LCONST(PROP_NEUTABSORB);
	LCONST(PROP_NEUTPASS);
	LCONST(PROP_DEADLY);
	LCONST(PROP_HOT_GLOW);
	LCONST(PROP_LIFE);
	LCONST(PROP_RADIOACTIVE);
	LCONST(PROP_LIFE_DEC);
	LCONST(PROP_LIFE_KILL);
	LCONST(PROP_LIFE_KILL_DEC);
	LCONST(PROP_SPARKSETTLE);
	LCONST(PROP_NOAMBHEAT);
	LCONST(PROP_NOCTYPEDRAW);
	LCONST(SC_WALL);
	LCONST(SC_ELEC);
	LCONST(SC_POWERED);
	LCONST(SC_SENSOR);
	LCONST(SC_FORCE);
	LCONST(SC_EXPLOSIVE);
	LCONST(SC_GAS);
	LCONST(SC_LIQUID);
	LCONST(SC_POWDERS);
	LCONST(SC_SOLIDS);
	LCONST(SC_NUCLEAR);
	LCONST(SC_SPECIAL);
	LCONST(SC_LIFE);
	LCONST(SC_TOOL);
	LCONST(SC_DECO);
	LCONSTAS("NUM_MENUSECTIONS", int(sd.msections.size()));
	LCONST(UPDATE_AFTER);
	LCONST(UPDATE_REPLACE);
	LCONST(UPDATE_BEFORE);
	LCONST(NUM_UPDATEMODES);
#undef LCONSTAS
#undef LCONST
	lua_pushvalue(L, -1);
	lua_setglobal(L, "elements");
	lua_setglobal(L, "elem");
	for (int i = 0; i < PT_NUM; i++)
	{
		manageElementIdentifier(L, i, true);
	}
}
