#include "Config.h"
#ifdef LUACONSOLE

#include "client/http/Request.h" // includes curl.h, needs to come first to silence a warning on windows

#include <iomanip>
#include <vector>
#include <algorithm>
#include <locale>

#include "Format.h"
#include "LuaScriptHelper.h"
#include "LuaScriptInterface.h"
#include "LuaSmartRef.h"
#include "PowderToy.h"

#include "client/Client.h"
#include "common/Platform.h"
#include "graphics/Graphics.h"
#include "graphics/Renderer.h"
#include "simulation/ElementCommon.h"
#include "simulation/Gravity.h"
#include "simulation/Simulation.h"
#include "simulation/SimulationData.h"

#include "gui/dialogues/ConfirmPrompt.h"
#include "gui/dialogues/ErrorMessage.h"
#include "gui/dialogues/InformationMessage.h"
#include "gui/dialogues/TextPrompt.h"
#include "gui/game/GameController.h"
#include "gui/game/GameModel.h"
#include "gui/interface/Engine.h"
#include "gui/interface/Keys.h"

std::map<ByteString, StructProperty> legacyPropNames;
std::map<ByteString, StructProperty> legacyTransitionNames;
void initLegacyProps()
{
	std::vector<StructProperty> properties = Element::GetProperties();
	for (auto prop : properties)
	{
		if (prop.Name == "MenuVisible")
			legacyPropNames.insert(std::pair<ByteString, StructProperty>("menu", prop));
		else if (prop.Name == "PhotonReflectWavelengths")
			continue;
		else if (prop.Name == "Temperature")
			legacyPropNames.insert(std::pair<ByteString, StructProperty>("heat", prop));
		else if (prop.Name == "HeatConduct")
			legacyPropNames.insert(std::pair<ByteString, StructProperty>("hconduct", prop));

		// Put all transition stuff into separate map
		else if (prop.Name == "LowPressure")
			legacyTransitionNames.insert(std::pair<ByteString, StructProperty>("presLowValue", prop));
		else if (prop.Name == "LowPressureTransition")
			legacyTransitionNames.insert(std::pair<ByteString, StructProperty>("presLowType", prop));
		else if (prop.Name == "HighPressure")
			legacyTransitionNames.insert(std::pair<ByteString, StructProperty>("presHighValue", prop));
		else if (prop.Name == "HighPressureTransition")
			legacyTransitionNames.insert(std::pair<ByteString, StructProperty>("presHighType", prop));
		else if (prop.Name == "LowTemperature")
			legacyTransitionNames.insert(std::pair<ByteString, StructProperty>("tempLowValue", prop));
		else if (prop.Name == "LowTemperatureTransition")
			legacyTransitionNames.insert(std::pair<ByteString, StructProperty>("tempLowType", prop));
		else if (prop.Name == "HighTemperature")
			legacyTransitionNames.insert(std::pair<ByteString, StructProperty>("tempHighValue", prop));
		else if (prop.Name == "HighTemperatureTransition")
			legacyTransitionNames.insert(std::pair<ByteString, StructProperty>("tempHighType", prop));

		else
		{
			legacyPropNames.insert(std::pair<ByteString, StructProperty>(prop.Name.ToLower(), prop));
		}
	}
}


#ifndef FFI
int luacon_partread(lua_State* l)
{
	int tempinteger, i = cIndex;
	float tempfloat;
	ByteString key = tpt_lua_optByteString(l, 2, "");
	CommandInterface::FormatType format;
	int offset = luacon_ci->GetPropertyOffset(key, format);

	if (i < 0 || i >= NPART)
		return luaL_error(l, "Out of range");
	if (offset == -1)
	{
		if (byteStringEqualsLiteral(key, "id"))
		{
			lua_pushnumber(l, i);
			return 1;
		}
		return luaL_error(l, "Invalid property");
	}

	switch(format)
	{
	case CommandInterface::FormatInt:
	case CommandInterface::FormatElement:
		tempinteger = *((int*)(((unsigned char*)&luacon_sim->parts[i])+offset));
		lua_pushnumber(l, tempinteger);
		break;
	case CommandInterface::FormatFloat:
		tempfloat = *((float*)(((unsigned char*)&luacon_sim->parts[i])+offset));
		lua_pushnumber(l, tempfloat);
		break;
	default:
		break;
	}
	return 1;
}

int luacon_partwrite(lua_State* l)
{
	int i = cIndex;
	ByteString key = tpt_lua_optByteString(l, 2, "");
	CommandInterface::FormatType format;
	int offset = luacon_ci->GetPropertyOffset(key, format);

	if (i < 0 || i >= NPART)
		return luaL_error(l, "Out of range");
	if (!luacon_sim->parts[i].type)
		return luaL_error(l, "Dead particle");
	if (offset == -1)
		return luaL_error(l, "Invalid property");

	switch(format)
	{
	case CommandInterface::FormatInt:
		*((int*)(((unsigned char*)&luacon_sim->parts[i])+offset)) = luaL_optinteger(l, 3, 0);
		break;
	case CommandInterface::FormatFloat:
		*((float*)(((unsigned char*)&luacon_sim->parts[i])+offset)) = luaL_optnumber(l, 3, 0);
		break;
	case CommandInterface::FormatElement:
		luacon_sim->part_change_type(i, int(luacon_sim->parts[i].x + 0.5f), int(luacon_sim->parts[i].y + 0.5f), luaL_optinteger(l, 3, 0));
	default:
		break;
	}
	return 0;
}

int luacon_partsread(lua_State* l)
{
	int i = luaL_optinteger(l, 2, 0);
	if (i < 0 || i >= NPART)
		return luaL_error(l, "array index out of bounds");

	lua_rawgeti(l, LUA_REGISTRYINDEX, *tptPart);
	cIndex = i;
	return 1;
}

int luacon_partswrite(lua_State* l)
{
	return luaL_error(l, "table readonly");
}
#endif

int luacon_transitionread(lua_State* l)
{
	ByteString key = tpt_lua_optByteString(l, 2, "");
	if (legacyTransitionNames.find(key) == legacyTransitionNames.end())
		return luaL_error(l, "Invalid property");
	StructProperty prop = legacyTransitionNames[key];

	//Get Raw Index value for element
	lua_pushliteral(l, "id");
	lua_rawget(l, 1);
	int i = lua_tointeger (l, lua_gettop(l));
	lua_pop(l, 1);
	if (!luacon_sim->IsElement(i))
	{
		return luaL_error(l, "Invalid index");
	}

	intptr_t propertyAddress = (intptr_t)(((unsigned char*)&luacon_sim->elements[i]) + prop.Offset);
	LuaScriptInterface::LuaGetProperty(l, prop, propertyAddress);

	return 1;
}

int luacon_transitionwrite(lua_State* l)
{
	ByteString key = tpt_lua_optByteString(l, 2, "");
	if (legacyTransitionNames.find(key) == legacyTransitionNames.end())
		return luaL_error(l, "Invalid property");
	StructProperty prop = legacyTransitionNames[key];

	//Get Raw Index value for element
	lua_pushliteral(l, "id");
	lua_rawget(l, 1);
	int i = lua_tointeger (l, lua_gettop(l));
	lua_pop(l, 1);
	if (!luacon_sim->IsElement(i))
	{
		return luaL_error(l, "Invalid index");
	}

	if (prop.Type == StructProperty::TransitionType)
	{
		int type = luaL_checkinteger(l, 3);
		if (!luacon_sim->IsElementOrNone(type) && type != NT && type != ST)
		{
			return luaL_error(l, "Invalid element");
		}
	}

	intptr_t propertyAddress = (intptr_t)(((unsigned char*)&luacon_sim->elements[i]) + prop.Offset);
	LuaScriptInterface::LuaSetProperty(l, prop, propertyAddress, 3);

	return 0;
}

int luacon_elementread(lua_State* l)
{
	ByteString key = tpt_lua_optByteString(l, 2, "");
	if (legacyPropNames.find(key) == legacyPropNames.end())
		return luaL_error(l, "Invalid property");
	StructProperty prop = legacyPropNames[key];

	//Get Raw Index value for element
	lua_pushliteral(l, "id");
	lua_rawget(l, 1);
	int i = lua_tointeger (l, lua_gettop(l));
	lua_pop(l, 1);
	if (!luacon_sim->IsElement(i))
	{
		return luaL_error(l, "Invalid index");
	}

	intptr_t propertyAddress = (intptr_t)(((unsigned char*)&luacon_sim->elements[i]) + prop.Offset);
	LuaScriptInterface::LuaGetProperty(l, prop, propertyAddress);

	return 1;
}

int luacon_elementwrite(lua_State* l)
{
	ByteString key = tpt_lua_optByteString(l, 2, "");
	if (legacyPropNames.find(key) == legacyPropNames.end())
		return luaL_error(l, "Invalid property");
	StructProperty prop = legacyPropNames[key];

	//Get Raw Index value for element
	lua_pushliteral(l, "id");
	lua_rawget(l, 1);
	int i = lua_tointeger (l, lua_gettop(l));
	lua_pop(l, 1);
	if (!luacon_sim->IsElement(i))
	{
		return luaL_error(l, "Invalid index");
	}

	if (prop.Name == "type") // i.e. it's .type
	{
		luacon_sim->part_change_type(i, int(luacon_sim->parts[i].x+0.5f), int(luacon_sim->parts[i].y+0.5f), luaL_checkinteger(l, 3));
	}
	else
	{
		intptr_t propertyAddress = (intptr_t)(((unsigned char*)&luacon_sim->elements[i]) + prop.Offset);
		LuaScriptInterface::LuaSetProperty(l, prop, propertyAddress, 3);
	}

	luacon_model->BuildMenus();
	luacon_sim->init_can_move();
	std::fill(&luacon_ren->graphicscache[0], &luacon_ren->graphicscache[PT_NUM], gcache_item());

	return 0;
}

void luacon_hook(lua_State * l, lua_Debug * ar)
{
	if(ar->event == LUA_HOOKCOUNT && Platform::GetTime()-ui::Engine::Ref().LastTick() > 3000)
	{
		if(ConfirmPrompt::Blocking("Script not responding", "The Lua script may have stopped responding. There might be an infinite loop. Press \"Stop\" to stop it", "Stop"))
			luaL_error(l, "Error: Script not responding");
		ui::Engine::Ref().LastTick(Platform::GetTime());
	}
}

String luacon_geterror()
{
	luaL_tostring(luacon_ci->l, -1);
	String err = tpt_lua_optString(luacon_ci->l, -1, "failed to execute");
	lua_pop(luacon_ci->l, 1);
	return err;
}

//tpt. api methods
int luatpt_getelement(lua_State *l)
{
	int t;
	if (lua_isnumber(l, 1))
	{
		t = luaL_optint(l, 1, 1);
		if (!luacon_sim->IsElementOrNone(t))
		{
			return luaL_error(l, "Unrecognised element number '%d'", t);
		}
		tpt_lua_pushString(l, luacon_sim->elements[t].Name);
	}
	else
	{
		luaL_checktype(l, 1, LUA_TSTRING);
		auto name = tpt_lua_optByteString(l, 1, "");
		if ((t = luacon_sim->GetParticleType(name))==-1)
			return luaL_error(l, "Unrecognised element '%s'", name.c_str());
		lua_pushinteger(l, t);
	}
	return 1;
}

int luatpt_error(lua_State* l)
{
	String errorMessage = tpt_lua_optString(l, 1, "Error text");
	ErrorMessage::Blocking("Error", errorMessage);
	return 0;
}

int luatpt_drawtext(lua_State* l)
{
	int textx, texty, textred, textgreen, textblue, textalpha;
	textx = luaL_optint(l, 1, 0);
	texty = luaL_optint(l, 2, 0);
	auto string = tpt_lua_optString(l, 3, "");
	textred = luaL_optint(l, 4, 255);
	textgreen = luaL_optint(l, 5, 255);
	textblue = luaL_optint(l, 6, 255);
	textalpha = luaL_optint(l, 7, 255);
	if (textx<0 || texty<0 || textx>=WINDOWW || texty>=WINDOWH)
		return luaL_error(l, "Screen coordinates out of range (%d,%d)", textx, texty);
	if (textred<0) textred = 0;
	if (textred>255) textred = 255;
	if (textgreen<0) textgreen = 0;
	if (textgreen>255) textgreen = 255;
	if (textblue<0) textblue = 0;
	if (textblue>255) textblue = 255;
	if (textalpha<0) textalpha = 0;
	if (textalpha>255) textalpha = 255;

	luacon_g->drawtext(textx, texty, string, textred, textgreen, textblue, textalpha);
	return 0;
}

int luatpt_create(lua_State* l)
{
	int x, y, retid, t = -1;
	x = abs(luaL_optint(l, 1, 0));
	y = abs(luaL_optint(l, 2, 0));
	if(x < XRES && y < YRES)
	{
		if (lua_isnumber(l, 3))
		{
			t = luaL_optint(l, 3, 0);
			if (!luacon_sim->IsElement(t))
			{
				return luaL_error(l, "Unrecognised element number '%d'", t);
			}
		} else {
			auto name = tpt_lua_optByteString(l, 3, "dust");
			if ((t = luacon_sim->GetParticleType(name)) == -1)
				return luaL_error(l,"Unrecognised element '%s'", name.c_str());
		}
		retid = luacon_sim->create_part(-1, x, y, t);
		// failing to create a particle often happens (e.g. if space is already occupied) and isn't usually important, so don't raise an error
		lua_pushinteger(l, retid);
		return 1;
	}
	return luaL_error(l, "Coordinates out of range (%d,%d)", x, y);
}

int luatpt_setpause(lua_State* l)
{
	int acount = lua_gettop(l);
	if (acount == 0)
	{
		lua_pushnumber(l, luacon_model->GetPaused());
		return 1;
	}
	int pausestate = luaL_checkinteger(l, 1);
	luacon_model->SetPaused(pausestate==0?0:1);
	return 0;
}

int luatpt_togglepause(lua_State* l)
{
	luacon_model->SetPaused(!luacon_model->GetPaused());
	lua_pushnumber(l, luacon_model->GetPaused());
	return 1;
}

int luatpt_togglewater(lua_State* l)
{
	luacon_sim->water_equal_test=!luacon_sim->water_equal_test;
	lua_pushnumber(l, luacon_sim->water_equal_test);
	return 1;
}

int luatpt_setconsole(lua_State* l)
{
	int acount = lua_gettop(l);
	if (acount == 0)
	{
		lua_pushnumber(l, luacon_ci->Window != ui::Engine::Ref().GetWindow());
		return 1;
	}
	if (luaL_checkinteger(l, 1))
		luacon_controller->ShowConsole();
	else
		luacon_controller->HideConsole();
	return 0;
}

int luatpt_log(lua_State* l)
{
	int args = lua_gettop(l);
	String text;
	bool hasText = false;
	for(int i = 1; i <= args; i++)
	{
		luaL_tostring(l, -1);
		if (hasText)
		{
			text = tpt_lua_optString(l, -1, "") + ", " + text;
		}
		else
		{
			text = tpt_lua_optString(l, -1, "");
			hasText = true;
		}
		lua_pop(l, 2);
	}
	if ((*luacon_currentCommand))
	{
		if (luacon_hasLastError)
			*luacon_lastError += "; ";
		*luacon_lastError += text;
		luacon_hasLastError = true;
	}
	else
		luacon_ci->Log(CommandInterface::LogNotice, text);
	return 0;
}

int luatpt_set_pressure(lua_State* l)
{
	int nx, ny;
	int x1, y1, width, height;
	float value;
	x1 = abs(luaL_optint(l, 1, 0));
	y1 = abs(luaL_optint(l, 2, 0));
	width = abs(luaL_optint(l, 3, XRES/CELL));
	height = abs(luaL_optint(l, 4, YRES/CELL));
	value = luaL_optnumber(l, 5, 0.0f);
	if(value > MAX_PRESSURE)
		value = MAX_PRESSURE;
	else if(value < MIN_PRESSURE)
		value = MIN_PRESSURE;

	if(x1 > (XRES/CELL)-1)
		x1 = (XRES/CELL)-1;
	if(y1 > (YRES/CELL)-1)
		y1 = (YRES/CELL)-1;
	if(x1+width > (XRES/CELL)-1)
		width = (XRES/CELL)-x1;
	if(y1+height > (YRES/CELL)-1)
		height = (YRES/CELL)-y1;
	for (nx = x1; nx<x1+width; nx++)
		for (ny = y1; ny<y1+height; ny++)
		{
			luacon_sim->pv[ny][nx] = value;
		}
	return 0;
}

int luatpt_set_gravity(lua_State* l)
{
	int nx, ny;
	int x1, y1, width, height;
	float value;
	x1 = abs(luaL_optint(l, 1, 0));
	y1 = abs(luaL_optint(l, 2, 0));
	width = abs(luaL_optint(l, 3, XRES/CELL));
	height = abs(luaL_optint(l, 4, YRES/CELL));
	value = luaL_optnumber(l, 5, 0.0f);
	if(value > MAX_PRESSURE)
		value = MAX_PRESSURE;
	else if(value < MIN_PRESSURE)
		value = MIN_PRESSURE;

	if(x1 > (XRES/CELL)-1)
		x1 = (XRES/CELL)-1;
	if(y1 > (YRES/CELL)-1)
		y1 = (YRES/CELL)-1;
	if(x1+width > (XRES/CELL)-1)
		width = (XRES/CELL)-x1;
	if(y1+height > (YRES/CELL)-1)
		height = (YRES/CELL)-y1;
	for (nx = x1; nx<x1+width; nx++)
		for (ny = y1; ny<y1+height; ny++)
		{
			luacon_sim->gravmap[ny*(XRES/CELL)+nx] = value;
		}
	return 0;
}

int luatpt_reset_gravity_field(lua_State* l)
{
	int nx, ny;
	int x1, y1, width, height;
	x1 = abs(luaL_optint(l, 1, 0));
	y1 = abs(luaL_optint(l, 2, 0));
	width = abs(luaL_optint(l, 3, XRES/CELL));
	height = abs(luaL_optint(l, 4, YRES/CELL));
	if(x1 > (XRES/CELL)-1)
		x1 = (XRES/CELL)-1;
	if(y1 > (YRES/CELL)-1)
		y1 = (YRES/CELL)-1;
	if(x1+width > (XRES/CELL)-1)
		width = (XRES/CELL)-x1;
	if(y1+height > (YRES/CELL)-1)
		height = (YRES/CELL)-y1;
	for (nx = x1; nx<x1+width; nx++)
		for (ny = y1; ny<y1+height; ny++)
		{
			luacon_sim->gravx[ny*(XRES/CELL)+nx] = 0;
			luacon_sim->gravy[ny*(XRES/CELL)+nx] = 0;
			luacon_sim->gravp[ny*(XRES/CELL)+nx] = 0;
		}
	return 0;
}

int luatpt_reset_velocity(lua_State* l)
{
	int nx, ny;
	int x1, y1, width, height;
	x1 = abs(luaL_optint(l, 1, 0));
	y1 = abs(luaL_optint(l, 2, 0));
	width = abs(luaL_optint(l, 3, XRES/CELL));
	height = abs(luaL_optint(l, 4, YRES/CELL));
	if(x1 > (XRES/CELL)-1)
		x1 = (XRES/CELL)-1;
	if(y1 > (YRES/CELL)-1)
		y1 = (YRES/CELL)-1;
	if(x1+width > (XRES/CELL)-1)
		width = (XRES/CELL)-x1;
	if(y1+height > (YRES/CELL)-1)
		height = (YRES/CELL)-y1;
	for (nx = x1; nx<x1+width; nx++)
		for (ny = y1; ny<y1+height; ny++)
		{
			luacon_sim->vx[ny][nx] = 0;
			luacon_sim->vy[ny][nx] = 0;
		}
	return 0;
}

int luatpt_reset_spark(lua_State* l)
{
	luacon_controller->ResetSpark();
	return 0;
}

int luatpt_set_property(lua_State* l)
{
	int r, i, x, y, w, h, t = 0, nx, ny, partsel = 0;
	float f = 0;
	int acount = lua_gettop(l);
	auto prop = tpt_lua_optByteString(l, 1, "");

	CommandInterface::FormatType format;
	int offset = luacon_ci->GetPropertyOffset(prop, format);
	if (offset == -1)
		return luaL_error(l, "Invalid property '%s'", prop.c_str());

	if (acount > 2)
	{
		if(!lua_isnumber(l, acount) && lua_isstring(l, acount))
		{
			auto name = tpt_lua_optByteString(l, acount, "none");
			if ((partsel = luacon_sim->GetParticleType(name)) == -1)
				return luaL_error(l, "Unrecognised element '%s'", name.c_str());
		}
	}
	if (lua_isnumber(l, 2))
	{
		if (format == CommandInterface::FormatFloat)
			f = luaL_optnumber(l, 2, 0);
		else
			t = luaL_optint(l, 2, 0);

		if (byteStringEqualsLiteral(prop, "type") && !luacon_sim->IsElementOrNone(t))
			return luaL_error(l, "Unrecognised element number '%d'", t);
	}
	else if (lua_isstring(l, 2))
	{
		auto name = tpt_lua_checkByteString(l, 2);
		if ((t = luacon_sim->GetParticleType(name))==-1)
			return luaL_error(l, "Unrecognised element '%s'", name.c_str());
	}
	else
		luaL_error(l, "Expected number or element name as argument 2");

	if (!lua_isnumber(l, 3) || acount >= 6)
	{
		// Got a region
		if (acount < 6)
		{
			i = 0;
			y = 0;
			w = XRES;
			h = YRES;
		}
		else
		{
			i = abs(luaL_checkint(l, 3));
			y = abs(luaL_checkint(l, 4));
			w = abs(luaL_checkint(l, 5));
			h = abs(luaL_checkint(l, 6));
		}
		if (i>=XRES || y>=YRES)
			return luaL_error(l, "Coordinates out of range (%d,%d)", i, y);
		x = i;
		if(x+w > XRES)
			w = XRES-x;
		if(y+h > YRES)
			h = YRES-y;
		Particle * parts = luacon_sim->parts;
		for (i = 0; i < NPART; i++)
		{
			if (parts[i].type)
			{
				nx = (int)(parts[i].x + .5f);
				ny = (int)(parts[i].y + .5f);
				if (nx >= x && nx < x+w && ny >= y && ny < y+h && (!partsel || partsel == parts[i].type))
				{
					if (format == CommandInterface::FormatElement)
						luacon_sim->part_change_type(i, nx, ny, t);
					else if(format == CommandInterface::FormatFloat)
						*((float*)(((unsigned char*)&luacon_sim->parts[i])+offset)) = f;
					else
						*((int*)(((unsigned char*)&luacon_sim->parts[i])+offset)) = t;
				}
			}
		}
	}
	else
	{
		i = abs(luaL_checkint(l, 3));
		// Got coords or particle index
		if (lua_isnumber(l, 4))
		{
			y = abs(luaL_checkint(l, 4));
			if (i>=XRES || y>=YRES)
				return luaL_error(l, "Coordinates out of range (%d,%d)", i, y);
			r = luacon_sim->pmap[y][i];
			if (!r || (partsel && partsel != TYP(r)))
				r = luacon_sim->photons[y][i];
			if (!r || (partsel && partsel != TYP(r)))
				return 0;
			i = ID(r);
		}
		if (i < 0 || i >= NPART)
			return luaL_error(l, "Invalid particle ID '%d'", i);
		if (!luacon_sim->parts[i].type)
			return 0;
		if (partsel && partsel != luacon_sim->parts[i].type)
			return 0;

		if (format == CommandInterface::FormatElement)
			luacon_sim->part_change_type(i, int(luacon_sim->parts[i].x + 0.5f), int(luacon_sim->parts[i].y + 0.5f), t);
		else if (format == CommandInterface::FormatFloat)
			*((float*)(((unsigned char*)&luacon_sim->parts[i])+offset)) = f;
		else
			*((int*)(((unsigned char*)&luacon_sim->parts[i])+offset)) = t;
	}
	return 0;
}

int luatpt_get_wallmap(lua_State* l)
{
	int x1 = abs(luaL_optint(l, 1, 0));
	int y1 = abs(luaL_optint(l, 2, 0));

	if(x1 > (XRES/CELL) || y1 > (YRES/CELL))
		return luaL_error(l, "Out of range");

	lua_pushinteger(l, luacon_sim->bmap[y1][x1]);
	return 1;
}

int luatpt_set_wallmap(lua_State* l)
{
	int args = lua_gettop(l);
	if (args < 3 || args > 7 || args % 2 != 1)
		return luaL_error(l, "Incorrect numbner of arguments");
	int x = luaL_optint(l, 1, 0);
	int y = luaL_optint(l, 2, 0);
	int w = luaL_optint(l, 3, 0);
	int h = luaL_optint(l, 4, 0);
	float fvx = float(luaL_optnumber(l, 5, 0));
	float fvy = float(luaL_optnumber(l, 6, 0));

	int wallType = luaL_optint(l, args, 0);
	if (wallType < 0 || wallType >= UI_WALLCOUNT)
	{
		return luaL_error(l, "Unrecognised wall number %d", wallType);
	}

	bool setFv = args == 7;
	if (args < 5)
	{
		w = 1;
		h = 1;
	}
	if (x < 0              ) x = 0              ;
	if (y < 0              ) y = 0              ;
	if (x > XRES / CELL    ) x = XRES / CELL    ;
	if (y > YRES / CELL    ) y = YRES / CELL    ;
	if (w > XRES / CELL - x) w = XRES / CELL - x;
	if (h > YRES / CELL - y) h = YRES / CELL - y;
	for (int yy = y; yy < y + h; ++yy)
	{
		for (int xx = x; xx < x + w; ++xx)
		{
			luacon_sim->bmap[yy][xx] = wallType;
			if (setFv)
			{
				luacon_sim->fvx[yy][xx] = fvx;
				luacon_sim->fvy[yy][xx] = fvy;
			}
		}
	}
	return 0;
}

int luatpt_set_elecmap(lua_State* l)
{
	int nx, ny, acount;
	int x1, y1, width, height;
	unsigned char value;
	acount = lua_gettop(l);

	x1 = abs(luaL_optint(l, 1, 0));
	y1 = abs(luaL_optint(l, 2, 0));
	width = abs(luaL_optint(l, 3, XRES/CELL));
	height = abs(luaL_optint(l, 4, YRES/CELL));
	value = luaL_optint(l, acount, 0);

	if(acount==5)	//Draw rect
	{
		if(x1 > (XRES/CELL))
			x1 = (XRES/CELL);
		if(y1 > (YRES/CELL))
			y1 = (YRES/CELL);
		if(x1+width > (XRES/CELL))
			width = (XRES/CELL)-x1;
		if(y1+height > (YRES/CELL))
			height = (YRES/CELL)-y1;
		for (nx = x1; nx<x1+width; nx++)
			for (ny = y1; ny<y1+height; ny++)
			{
				luacon_sim->emap[ny][nx] = value;
			}
	}
	else	//Set point
	{
		if(x1 > (XRES/CELL))
			x1 = (XRES/CELL);
		if(y1 > (YRES/CELL))
			y1 = (YRES/CELL);
		luacon_sim->emap[y1][x1] = value;
	}
	return 0;
}

int luatpt_get_elecmap(lua_State* l)
{
	int x1 = abs(luaL_optint(l, 1, 0));
	int y1 = abs(luaL_optint(l, 2, 0));

	if(x1 > (XRES/CELL) || y1 > (YRES/CELL))
		return luaL_error(l, "Out of range");

	lua_pushinteger(l, luacon_sim->emap[y1][x1]);
	return 1;
}

int luatpt_get_property(lua_State* l)
{
	ByteString prop = tpt_lua_optByteString(l, 1, "");
	int i = luaL_optint(l, 2, 0); //x coord or particle index, depending on arguments
	int y = luaL_optint(l, 3, -1);
	if (y!=-1 && y<YRES && y>=0 && i < XRES && i>=0)
	{
		int r = luacon_sim->pmap[y][i];
		if (!r)
		{
			r = luacon_sim->photons[y][i];
			if (!r)
			{
				if (byteStringEqualsLiteral(prop, "type"))
				{
					lua_pushinteger(l, 0);
					return 1;
				}
				return luaL_error(l, "Particle does not exist");
			}
		}
		i = ID(r);
	}
	else if (y != -1)
		return luaL_error(l, "Coordinates out of range (%d,%d)", i, y);
	if (i < 0 || i >= NPART)
		return luaL_error(l, "Invalid particle ID '%d'", i);

	if (luacon_sim->parts[i].type)
	{
		int tempinteger;
		float tempfloat;
		CommandInterface::FormatType format;
		int offset = luacon_ci->GetPropertyOffset(prop, format);

		if (offset == -1)
		{
			if (byteStringEqualsLiteral(prop, "id"))
			{
				lua_pushnumber(l, i);
				return 1;
			}
			else
				return luaL_error(l, "Invalid property");
		}
		switch(format)
		{
		case CommandInterface::FormatInt:
		case CommandInterface::FormatElement:
			tempinteger = *((int*)(((unsigned char*)&luacon_sim->parts[i])+offset));
			lua_pushnumber(l, tempinteger);
			break;
		case CommandInterface::FormatFloat:
			tempfloat = *((float*)(((unsigned char*)&luacon_sim->parts[i])+offset));
			lua_pushnumber(l, tempfloat);
			break;
		default:
			break;
		}
		return 1;
	}
	else if (byteStringEqualsLiteral(prop, "type"))
	{
		lua_pushinteger(l, 0);
		return 1;
	}
	return luaL_error(l, "Particle does not exist");
}

int luatpt_drawpixel(lua_State* l)
{
	int x, y, r, g, b, a;
	x = luaL_optint(l, 1, 0);
	y = luaL_optint(l, 2, 0);
	r = luaL_optint(l, 3, 255);
	g = luaL_optint(l, 4, 255);
	b = luaL_optint(l, 5, 255);
	a = luaL_optint(l, 6, 255);

	if (x<0 || y<0 || x>=WINDOWW || y>=WINDOWH)
		return luaL_error(l, "Screen coordinates out of range (%d,%d)", x, y);
	if (r<0) r = 0;
	else if (r>255) r = 255;
	if (g<0) g = 0;
	else if (g>255) g = 255;
	if (b<0) b = 0;
	else if (b>255) b = 255;
	if (a<0) a = 0;
	else if (a>255) a = 255;
	luacon_g->blendpixel(x, y, r, g, b, a);
	return 0;
}

int luatpt_drawrect(lua_State* l)
{
	int x, y, w, h, r, g, b, a;
	x = luaL_optint(l, 1, 0);
	y = luaL_optint(l, 2, 0);
	w = luaL_optint(l, 3, 10)+1;
	h = luaL_optint(l, 4, 10)+1;
	r = luaL_optint(l, 5, 255);
	g = luaL_optint(l, 6, 255);
	b = luaL_optint(l, 7, 255);
	a = luaL_optint(l, 8, 255);

	if (x<0 || y<0 || x>=WINDOWW || y>=WINDOWH)
		return luaL_error(l, "Screen coordinates out of range (%d,%d)", x, y);
	if(x+w > WINDOWW)
		w = WINDOWW-x;
	if(y+h > WINDOWH)
		h = WINDOWH-y;
	if (r<0) r = 0;
	else if (r>255) r = 255;
	if (g<0) g = 0;
	else if (g>255) g = 255;
	if (b<0) b = 0;
	else if (b>255) b = 255;
	if (a<0) a = 0;
	else if (a>255) a = 255;
	luacon_g->drawrect(x, y, w, h, r, g, b, a);
	return 0;
}

int luatpt_fillrect(lua_State* l)
{
	int x,y,w,h,r,g,b,a;
	x = luaL_optint(l, 1, 0)+1;
	y = luaL_optint(l, 2, 0)+1;
	w = luaL_optint(l, 3, 10)-1;
	h = luaL_optint(l, 4, 10)-1;
	r = luaL_optint(l, 5, 255);
	g = luaL_optint(l, 6, 255);
	b = luaL_optint(l, 7, 255);
	a = luaL_optint(l, 8, 255);

	if (x<0 || y<0 || x>=WINDOWW || y>=WINDOWH)
		return luaL_error(l, "Screen coordinates out of range (%d,%d)", x, y);
	if(x+w > WINDOWW)
		w = WINDOWW-x;
	if(y+h > WINDOWH)
		h = WINDOWH-y;
	if (r<0) r = 0;
	else if (r>255) r = 255;
	if (g<0) g = 0;
	else if (g>255) g = 255;
	if (b<0) b = 0;
	else if (b>255) b = 255;
	if (a<0) a = 0;
	else if (a>255) a = 255;
	luacon_g->fillrect(x, y, w, h, r, g, b, a);
	return 0;
}

int luatpt_drawline(lua_State* l)
{
	int x1,y1,x2,y2,r,g,b,a;
	x1 = luaL_optint(l, 1, 0);
	y1 = luaL_optint(l, 2, 0);
	x2 = luaL_optint(l, 3, 10);
	y2 = luaL_optint(l, 4, 10);
	r = luaL_optint(l, 5, 255);
	g = luaL_optint(l, 6, 255);
	b = luaL_optint(l, 7, 255);
	a = luaL_optint(l, 8, 255);

	//Don't need to check coordinates, as they are checked in blendpixel
	if (r<0) r = 0;
	else if (r>255) r = 255;
	if (g<0) g = 0;
	else if (g>255) g = 255;
	if (b<0) b = 0;
	else if (b>255) b = 255;
	if (a<0) a = 0;
	else if (a>255) a = 255;
	luacon_g->draw_line(x1, y1, x2, y2, r, g, b, a);
	return 0;
}

int luatpt_textwidth(lua_State* l)
{
	auto string = tpt_lua_optString(l, 1, "");
	int strwidth = Graphics::textwidth(string);
	lua_pushinteger(l, strwidth);
	return 1;
}

int luatpt_get_name(lua_State* l)
{
	if (luacon_model->GetUser().UserID)
	{
		tpt_lua_pushByteString(l, luacon_model->GetUser().Username);
		return 1;
	}
	lua_pushliteral(l, "");
	return 1;
}

int luatpt_delete(lua_State* l)
{
	int arg1, arg2;
	arg1 = abs(luaL_optint(l, 1, 0));
	arg2 = luaL_optint(l, 2, -1);
	if (arg2 == -1 && arg1 < NPART)
	{
		luacon_sim->kill_part(arg1);
		return 0;
	}
	arg2 = abs(arg2);
	if(arg2 < YRES && arg1 < XRES)
	{
		luacon_sim->delete_part(arg1, arg2);
		return 0;
	}
	return luaL_error(l,"Invalid coordinates or particle ID");
}

int luatpt_input(lua_State* l)
{
	String title = tpt_lua_optString(l, 1, "Title");
	String prompt = tpt_lua_optString(l, 2, "Enter some text:");
	String text = tpt_lua_optString(l, 3, "");
	String shadow = tpt_lua_optString(l, 4, "");

	String result = TextPrompt::Blocking(title, prompt, text, shadow, false);

	tpt_lua_pushString(l, result);
	return 1;
}

int luatpt_message_box(lua_State* l)
{
	String title = tpt_lua_optString(l, 1, "Title");
	String message = tpt_lua_optString(l, 2, "Message");
	int large = lua_toboolean(l, 3);
	new InformationMessage(title, message, large);
	return 0;
}

int luatpt_confirm(lua_State *l)
{
	String title = tpt_lua_optString(l, 1, "Title");
	String message = tpt_lua_optString(l, 2, "Message");
	String buttonText = tpt_lua_optString(l, 3, "Confirm");
	bool ret = ConfirmPrompt::Blocking(title, message, buttonText);
	lua_pushboolean(l, ret ? 1 : 0);
	return 1;
}

int luatpt_get_numOfParts(lua_State* l)
{
	lua_pushinteger(l, luacon_sim->NUM_PARTS);
	return 1;
}

int luatpt_start_getPartIndex(lua_State* l)
{
	getPartIndex_curIdx = -1;
	return 0;
}

int luatpt_next_getPartIndex(lua_State* l)
{
	while(1)
	{
		getPartIndex_curIdx++;
		if (getPartIndex_curIdx >= NPART)
		{
			getPartIndex_curIdx = -1;
			lua_pushboolean(l, 0);
			return 1;
		}
		if (luacon_sim->parts[getPartIndex_curIdx].type)
			break;

	}

	lua_pushboolean(l, 1);
	return 1;
}

int luatpt_getPartIndex(lua_State* l)
{
	if(getPartIndex_curIdx < 0)
	{
		lua_pushinteger(l, -1);
		return 1;
	}
	lua_pushinteger(l, getPartIndex_curIdx);
	return 1;
}

int luatpt_hud(lua_State* l)
{
	int acount = lua_gettop(l);
	if (acount == 0)
	{
		lua_pushinteger(l, luacon_controller->GetHudEnable());
		return 1;
	}
	int hudstate = luaL_checkint(l, 1);
	if (hudstate)
		luacon_controller->SetHudEnable(1);
	else
		luacon_controller->SetHudEnable(0);
	return 0;
}

int luatpt_gravity(lua_State* l)
{
	int acount = lua_gettop(l);
	if (acount == 0)
	{
		lua_pushinteger(l, luacon_sim->grav->IsEnabled() ? 1 : 0);
		return 1;
	}
	int gravstate = luaL_checkint(l, 1);
	if(gravstate)
		luacon_sim->grav->start_grav_async();
	else
		luacon_sim->grav->stop_grav_async();
	luacon_model->UpdateQuickOptions();
	return 0;
}

int luatpt_airheat(lua_State* l)
{
	int acount = lua_gettop(l);
	if (acount == 0)
	{
		lua_pushinteger(l, luacon_sim->aheat_enable);
		return 1;
	}
	int aheatstate = luaL_checkint(l, 1);
	luacon_sim->aheat_enable = (aheatstate==0?0:1);
	luacon_model->UpdateQuickOptions();
	return 0;
}

int luatpt_active_menu(lua_State* l)
{
	int acount = lua_gettop(l);
	if (acount == 0)
	{
		lua_pushinteger(l, luacon_model->GetActiveMenu());
		return 1;
	}
	int menuid = luaL_checkint(l, 1);
	if (menuid >= 0 && menuid < SC_TOTAL)
		luacon_controller->SetActiveMenu(menuid);
	else
		return luaL_error(l, "Invalid menu");
	return 0;
}

int luatpt_menu_enabled(lua_State* l)
{
	int menusection = luaL_checkint(l, 1);
	if (menusection < 0 || menusection >= SC_TOTAL)
		return luaL_error(l, "Invalid menu");
	int acount = lua_gettop(l);
	if (acount == 1)
	{
		lua_pushboolean(l, luacon_sim->msections[menusection].doshow);
		return 1;
	}
	luaL_checktype(l, 2, LUA_TBOOLEAN);
	int enabled = lua_toboolean(l, 2);
	luacon_sim->msections[menusection].doshow = enabled;
	luacon_model->BuildMenus();
	return 0;
}

int luatpt_num_menus(lua_State* l)
{
	int acount = lua_gettop(l);
	bool onlyEnabled = true;
	if (acount > 0)
	{
		luaL_checktype(l, 1, LUA_TBOOLEAN);
		onlyEnabled = lua_toboolean(l, 1);
	}
	lua_pushinteger(l, luacon_controller->GetNumMenus(onlyEnabled));
	return 1;
}

int luatpt_decorations_enable(lua_State* l)
{
	int acount = lua_gettop(l);
	if (acount == 0)
	{
		lua_pushinteger(l, luacon_model->GetDecoration());
		return 1;
	}
	int decostate = luaL_checkint(l, 1);
	luacon_model->SetDecoration(decostate==0?false:true);
	luacon_model->UpdateQuickOptions();
	return 0;
}

int luatpt_heat(lua_State* l)
{
	int acount = lua_gettop(l);
	if (acount == 0)
	{
		lua_pushinteger(l, !luacon_sim->legacy_enable);
		return 1;
	}
	int heatstate = luaL_checkint(l, 1);
	luacon_sim->legacy_enable = (heatstate==1?0:1);
	return 0;
}

int luatpt_cmode_set(lua_State* l)
{
	int cmode = luaL_optint(l, 1, 3)+1;
	if (cmode == 11)
		cmode = 0;
	if (cmode >= 0 && cmode <= 10)
		luacon_controller->LoadRenderPreset(cmode);
	else
		return luaL_error(l, "Invalid display mode");
	return 0;
}

int luatpt_setfire(lua_State* l)
{
	int firesize = luaL_optint(l, 2, 4);
	float fireintensity = (float)luaL_optnumber(l, 1, 1.0f);
	luacon_model->GetRenderer()->prepare_alpha(firesize, fireintensity);
	return 0;
}

int luatpt_setdebug(lua_State* l)
{
	int debugFlags = luaL_optint(l, 1, 0);
	luacon_controller->SetDebugFlags(debugFlags);
	return 0;
}

int luatpt_setfpscap(lua_State* l)
{
	int acount = lua_gettop(l);
	if (acount == 0)
	{
		lua_pushnumber(l, ui::Engine::Ref().FpsLimit);
		return 1;
	}
	float fpscap = luaL_checknumber(l, 1);
	if (fpscap < 2)
		return luaL_error(l, "fps cap too small");
	ui::Engine::Ref().FpsLimit = fpscap;
	return 0;
}

int luatpt_setdrawcap(lua_State* l)
{
	int acount = lua_gettop(l);
	if (acount == 0)
	{
		lua_pushinteger(l, ui::Engine::Ref().GetDrawingFrequencyLimit());
		return 1;
	}
	int drawcap = luaL_checkint(l, 1);
	if(drawcap < 0)
		return luaL_error(l, "draw cap too small");
	ui::Engine::Ref().SetDrawingFrequencyLimit(drawcap);
	return 0;
}

int luatpt_getscript(lua_State* l)
{
	int scriptID = luaL_checkinteger(l, 1);
	auto filename = tpt_lua_checkByteString(l, 2);
	int runScript = luaL_optint(l, 3, 0);
	int confirmPrompt = luaL_optint(l, 4, 1);

	ByteString url = ByteString::Build(SCHEME "starcatcher.us/scripts/main.lua?get=", scriptID);
	if (confirmPrompt && !ConfirmPrompt::Blocking("Do you want to install script?", url.FromUtf8(), "Install"))
		return 0;

	int ret;
	ByteString scriptData = http::Request::Simple(url, &ret);
	if (!scriptData.size())
	{
		return luaL_error(l, "Server did not return data");
	}
	if (ret != 200)
	{
		return luaL_error(l, http::StatusText(ret).ToUtf8().c_str());
	}

	if (scriptData.Contains("Invalid script ID"))
	{
		return luaL_error(l, "Invalid Script ID");
	}

	if (Platform::FileExists(filename) && confirmPrompt && !ConfirmPrompt::Blocking("File already exists, overwrite?", filename.FromUtf8(), "Overwrite"))
	{
		return 0;
	}
	if (!Platform::WriteFile(std::vector<char>(scriptData.begin(), scriptData.end()), filename))
	{
		return luaL_error(l, "Unable to write to file");
	}
	if (runScript)
	{
		tpt_lua_dostring(l, ByteString::Build("dofile('", filename, "')"));
	}

	return 0;
}

int luatpt_setwindowsize(lua_State* l)
{
	int scale = luaL_optint(l,1,1);
	int kiosk = luaL_optint(l,2,0);
	// TODO: handle this the same way as it's handled in PowderToySDL.cpp
	//   > maybe bind the maximum allowed scale to screen size somehow
	if (scale < 1 || scale > 10) scale = 1;
	if (kiosk!=1) kiosk = 0;
	Client::Ref().SetPref("Scale", scale);
	ui::Engine::Ref().SetScale(scale);
	ui::Engine::Ref().SetFullscreen(kiosk);
	return 0;
}

int screenshotIndex = 0;

int luatpt_screenshot(lua_State* l)
{
	int captureUI = luaL_optint(l, 1, 0);
	int fileType = luaL_optint(l, 2, 0);

	ByteString filename = luacon_controller->TakeScreenshot(captureUI, fileType);
	if (filename.size())
	{
		tpt_lua_pushByteString(l, filename);
		return 1;
	}
	return 0;
}

int luatpt_record(lua_State* l)
{
	if (!lua_isboolean(l, -1))
		return luaL_typerror(l, 1, lua_typename(l, LUA_TBOOLEAN));
	bool record = lua_toboolean(l, -1);
	int recordingFolder = luacon_controller->Record(record);
	lua_pushinteger(l, recordingFolder);
	return 1;
}

int luatpt_perfectCircle(lua_State* l)
{
	if (!lua_gettop(l))
	{
		lua_pushboolean(l, luacon_model->GetPerfectCircle());
		return 1;
	}
	luaL_checktype(l, 1, LUA_TBOOLEAN);
	luacon_model->SetPerfectCircle(lua_toboolean(l, 1));
	return 0;
}

#endif
