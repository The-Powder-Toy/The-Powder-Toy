#ifdef LUACONSOLE
#include <string>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <locale>

#include "client/HTTP.h"
#include "Format.h"
#include "LuaScriptInterface.h"
#include "LuaScriptHelper.h"
#include "Platform.h"
#include "PowderToy.h"

#include "gui/dialogues/ErrorMessage.h"
#include "gui/dialogues/InformationMessage.h"
#include "gui/dialogues/TextPrompt.h"
#include "gui/dialogues/ConfirmPrompt.h"
#include "gui/game/GameModel.h"
#include "gui/interface/Keys.h"
#include "simulation/Simulation.h"


#ifndef FFI
int luacon_partread(lua_State* l)
{
	int tempinteger, i = cIndex;
	float tempfloat;
	std::string key = luaL_optstring(l, 2, "");
	CommandInterface::FormatType format;
	int offset = luacon_ci->GetPropertyOffset(key, format);

	if (i < 0 || i >= NPART)
		return luaL_error(l, "Out of range");
	if (offset == -1)
	{
		if (!key.compare("id"))
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
	std::string key = luaL_optstring(l, 2, "");
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
		luacon_sim->part_change_type(i, luacon_sim->parts[i].x, luacon_sim->parts[i].y, luaL_optinteger(l, 3, 0));
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

	lua_rawgeti(l, LUA_REGISTRYINDEX, tptPart);
	cIndex = i;
	return 1;
}

int luacon_partswrite(lua_State* l)
{
	return luaL_error(l, "table readonly");
}
#endif

int luacon_transition_getproperty(const char * key, int * format)
{
	int offset;
	if (!strcmp(key, "presHighValue")) {
		offset = offsetof(Element, HighPressure);
		*format = 1;
	} else if (!strcmp(key, "presHighType")) {
		offset = offsetof(Element, HighPressureTransition);
		*format = 0;
	} else if (!strcmp(key, "presLowValue")) {
		offset = offsetof(Element, LowPressure);
		*format = 1;
	} else if (!strcmp(key, "presLowType")) {
		offset = offsetof(Element, LowPressureTransition);
		*format = 0;
	} else if (!strcmp(key, "tempHighValue")) {
		offset = offsetof(Element, HighTemperature);
		*format = 1;
	} else if (!strcmp(key, "tempHighType")) {
		offset = offsetof(Element, HighTemperatureTransition);
		*format = 0;
	} else if (!strcmp(key, "tempLowValue")) {
		offset = offsetof(Element, LowTemperature);
		*format = 1;
	} else if (!strcmp(key, "tempLowType")) {
		offset = offsetof(Element, LowTemperatureTransition);
		*format = 0;
	} else {
		offset = -1;
	}
	return offset;
}

int luacon_transitionread(lua_State* l)
{
	int format, offset;
	int tempinteger;
	float tempfloat;
	int i;
	const char * key = luaL_optstring(l, 2, "");
	offset = luacon_transition_getproperty(key, &format);

	//Get Raw Index value for element
	lua_pushstring(l, "value");
	lua_rawget(l, 1);

	i = lua_tointeger(l, lua_gettop(l));

	lua_pop(l, 1);

	if (i < 0 || i >= PT_NUM || offset==-1)
	{
		return luaL_error(l, "Invalid property");
	}
	switch(format)
	{
	case 0:
		tempinteger = *((int*)(((unsigned char*)&luacon_sim->elements[i])+offset));
		lua_pushnumber(l, tempinteger);
		break;
	case 1:
		tempfloat = *((float*)(((unsigned char*)&luacon_sim->elements[i])+offset));
		lua_pushnumber(l, tempfloat);
		break;
	}
	return 1;
}

int luacon_transitionwrite(lua_State* l)
{
	int format, offset;
	int i;
	const char * key = luaL_optstring(l, 2, "");
	offset = luacon_transition_getproperty(key, &format);

	//Get Raw Index value for element
	lua_pushstring(l, "value");
	lua_rawget(l, 1);

	i = lua_tointeger(l, lua_gettop(l));

	lua_pop(l, 1);

	if (i < 0 || i >= PT_NUM || offset==-1)
	{
		return luaL_error(l, "Invalid property");
	}
	switch(format)
	{
	case 0:
		*((int*)(((unsigned char*)&luacon_sim->elements[i])+offset)) = luaL_optinteger(l, 3, 0);
		break;
	case 1:
		*((float*)(((unsigned char*)&luacon_sim->elements[i])+offset)) = luaL_optnumber(l, 3, 0);
		break;
	}
	return 0;
}

int luacon_element_getproperty(const char * key, int * format, unsigned int * modified_stuff)
{
	int offset;
	if (!strcmp(key, "name")) {
		offset = offsetof(Element, Name);
		*format = 2;
		if(modified_stuff)
			*modified_stuff |= LUACON_EL_MODIFIED_MENUS;
	}
	else if (!strcmp(key, "color")) {
		offset = offsetof(Element, Colour);
		*format = 0;
		if (modified_stuff)
			*modified_stuff |= LUACON_EL_MODIFIED_GRAPHICS | LUACON_EL_MODIFIED_MENUS;
	}
	else if (!strcmp(key, "colour")) {
		offset = offsetof(Element, Colour);
		*format = 0;
		if (modified_stuff)
			*modified_stuff |= LUACON_EL_MODIFIED_GRAPHICS;
	}
	else if (!strcmp(key, "advection")) {
		offset = offsetof(Element, Advection);
		*format = 1;
	}
	else if (!strcmp(key, "airdrag")) {
		offset = offsetof(Element, AirDrag);
		*format = 1;
	}
	else if (!strcmp(key, "airloss")) {
		offset = offsetof(Element, AirLoss);
		*format = 1;
	}
	else if (!strcmp(key, "loss")) {
		offset = offsetof(Element, Loss);
		*format = 1;
	}
	else if (!strcmp(key, "collision")) {
		offset = offsetof(Element, Collision);
		*format = 1;
	}
	else if (!strcmp(key, "gravity")) {
		offset = offsetof(Element, Gravity);
		*format = 1;
	}
	else if (!strcmp(key, "diffusion")) {
		offset = offsetof(Element, Diffusion);
		*format = 1;
	}
	else if (!strcmp(key, "hotair")) {
		offset = offsetof(Element, HotAir);
		*format = 1;
	}
	else if (!strcmp(key, "falldown")) {
		offset = offsetof(Element, Falldown);
		*format = 0;
	}
	else if (!strcmp(key, "flammable")) {
		offset = offsetof(Element, Flammable);
		*format = 0;
	}
	else if (!strcmp(key, "explosive")) {
		offset = offsetof(Element, Explosive);
		*format = 0;
	}
	else if (!strcmp(key, "meltable")) {
		offset = offsetof(Element, Meltable);
		*format = 0;
	}
	else if (!strcmp(key, "hardness")) {
		offset = offsetof(Element, Hardness);
		*format = 0;
	}
	// Not sure if this should be enabled
	// Also, needs a new format type for unsigned ints
	/*else if (!strcmp(key, "photonreflectwavelengths")) {
		offset = offsetof(Element, PhotonReflectWavelengths);
		*format = ;
	}*/
	else if (!strcmp(key, "menu")) {
		offset = offsetof(Element, MenuVisible);
		*format = 0;
		if (modified_stuff)
			*modified_stuff |= LUACON_EL_MODIFIED_MENUS;
	}
	else if (!strcmp(key, "enabled")) {
		offset = offsetof(Element, Enabled);
		*format = 0;
	}
	else if (!strcmp(key, "weight")) {
		offset = offsetof(Element, Weight);
		*format = 0;
		if (modified_stuff)
			*modified_stuff |= LUACON_EL_MODIFIED_CANMOVE;
	}
	else if (!strcmp(key, "menusection")) {
		offset = offsetof(Element, MenuSection);
		*format = 0;
		if (modified_stuff)
			*modified_stuff |= LUACON_EL_MODIFIED_MENUS;
	}
	else if (!strcmp(key, "heat")) {
		offset = offsetof(Element, Temperature);
		*format = 1;
	}
	else if (!strcmp(key, "hconduct")) {
		offset = offsetof(Element, HeatConduct);
		*format = 3;
	}
	else if (!strcmp(key, "state")) {
		offset = 0;
		*format = -1;
	}
	else if (!strcmp(key, "properties")) {
		offset = offsetof(Element, Properties);
		*format = 0;
		if (modified_stuff)
			*modified_stuff |= LUACON_EL_MODIFIED_GRAPHICS | LUACON_EL_MODIFIED_CANMOVE;
	}
	else if (!strcmp(key, "description")) {
		offset = offsetof(Element, Description);
		*format = 2;
		if(modified_stuff)
			*modified_stuff |= LUACON_EL_MODIFIED_MENUS;
	}
	else {
		return -1;
	}
	return offset;
}

int luacon_elementread(lua_State* l)
{
	int format, offset;
	char * tempstring;
	int tempinteger;
	float tempfloat;
	int i;
	const char * key = luaL_optstring(l, 2, "");
	offset = luacon_element_getproperty(key, &format, NULL);

	//Get Raw Index value for element
	lua_pushstring(l, "id");
	lua_rawget(l, 1);

	i = lua_tointeger (l, lua_gettop(l));

	lua_pop(l, 1);

	if (i < 0 || i >= PT_NUM || offset==-1)
	{
		return luaL_error(l, "Invalid property");
	}
	switch(format)
	{
	case 0:
		tempinteger = *((int*)(((unsigned char*)&luacon_sim->elements[i])+offset));
		lua_pushnumber(l, tempinteger);
		break;
	case 1:
		tempfloat = *((float*)(((unsigned char*)&luacon_sim->elements[i])+offset));
		lua_pushnumber(l, tempfloat);
		break;
	case 2:
		tempstring = *((char**)(((unsigned char*)&luacon_sim->elements[i])+offset));
		lua_pushstring(l, tempstring);
		break;
	case 3:
		tempinteger = *((unsigned char*)(((unsigned char*)&luacon_sim->elements[i])+offset));
		lua_pushnumber(l, tempinteger);
		break;
	default:
		lua_pushnumber(l, 0);
	}
	return 1;
}

int luacon_elementwrite(lua_State* l)
{
	int format, offset;
	char * tempstring;
	int i;
	unsigned int modified_stuff = 0;
	const char * key = luaL_optstring(l, 2, "");
	offset = luacon_element_getproperty(key, &format, &modified_stuff);

	//Get Raw Index value for element
	lua_pushstring(l, "id");
	lua_rawget(l, 1);

	i = lua_tointeger (l, lua_gettop(l));

	lua_pop(l, 1);

	if (i < 0 || i >= PT_NUM || offset==-1)
	{
		return luaL_error(l, "Invalid property");
	}
	switch(format)
	{
	case 0:
		*((int*)(((unsigned char*)&luacon_sim->elements[i])+offset)) = luaL_optinteger(l, 3, 0);
		break;
	case 1:
		*((float*)(((unsigned char*)&luacon_sim->elements[i])+offset)) = luaL_optnumber(l, 3, 0);
		break;
	case 2:
		tempstring = mystrdup((char*)luaL_optstring(l, 3, ""));
		if (!strcmp(key, "name"))
		{
			//Convert to upper case
			for (size_t j = 0; j < strlen(tempstring); j++)
				tempstring[j] = toupper(tempstring[j]);
			if(luacon_sim->GetParticleType(tempstring) != -1)
			{
				free(tempstring);
				return luaL_error(l, "Name in use");
			}
		}
		*((char**)(((unsigned char*)&luacon_sim->elements[i])+offset)) = tempstring;
		//Need some way of cleaning up previous values
		break;
	case 3:
		*((unsigned char*)(((unsigned char*)&luacon_sim->elements[i])+offset)) = luaL_optinteger(l, 3, 0);
		break;
	}
	if (modified_stuff)
	{
		if (modified_stuff & LUACON_EL_MODIFIED_MENUS)
			luacon_model->BuildMenus();
		if (modified_stuff & LUACON_EL_MODIFIED_CANMOVE)
			luacon_sim->init_can_move();
		if (modified_stuff & LUACON_EL_MODIFIED_GRAPHICS)
			memset(luacon_ren->graphicscache, 0, sizeof(gcache_item)*PT_NUM);
	}
	return 0;
}

bool shortcuts = true;
int luacon_keyevent(int key, Uint16 character, int modifier, int event)
{
	ui::Engine::Ref().LastTick(Platform::GetTime());
	int kycontinue = 1;
	lua_State* l=luacon_ci->l;
	lua_pushstring(l, "keyfunctions");
	lua_rawget(l, LUA_REGISTRYINDEX);
	if(!lua_istable(l, -1))
	{
		lua_pop(l, 1);
		lua_newtable(l);
		lua_pushstring(l, "keyfunctions");
		lua_pushvalue(l, -2);
		lua_rawset(l, LUA_REGISTRYINDEX);
	}
	int len = lua_objlen(l, -1);
	for (int i = 1; i <= len && kycontinue; i++)
	{
		lua_rawgeti(l, -1, i);
		if ((modifier & KMOD_CTRL) && (character < ' ' || character > '~') && key < 256)
			lua_pushlstring(l, (const char*)&key, 1);
		else
			lua_pushlstring(l, (const char*)&character, 1);
		lua_pushinteger(l, key);
		lua_pushinteger(l, modifier);
		lua_pushinteger(l, event);
		int callret = lua_pcall(l, 4, 1, 0);
		if (callret)
		{
			if (!strcmp(luacon_geterror(), "Error: Script not responding"))
			{
				ui::Engine::Ref().LastTick(Platform::GetTime());
				for (int j = i; j <= len-1; j++)
				{
					lua_rawgeti(l, -2, j+1);
					lua_rawseti(l, -3, j);
				}
				lua_pushnil(l);
				lua_rawseti(l, -3, len);
				i--;
			}
			luacon_ci->Log(CommandInterface::LogError, luacon_geterror());
			lua_pop(l, 1);
		}
		else
		{
			if(!lua_isnoneornil(l, -1))
				kycontinue = lua_toboolean(l, -1);
			lua_pop(l, 1);
		}
		len = lua_objlen(l, -1);
	}
	lua_pop(l, 1);
	return kycontinue && shortcuts;
}

int luacon_mouseevent(int mx, int my, int mb, int event, int mouse_wheel)
{
	ui::Engine::Ref().LastTick(Platform::GetTime());
	int mpcontinue = 1;
	lua_State* l=luacon_ci->l;
	lua_pushstring(l, "mousefunctions");
	lua_rawget(l, LUA_REGISTRYINDEX);
	if(!lua_istable(l, -1))
	{
		lua_pop(l, 1);
		lua_newtable(l);
		lua_pushstring(l, "mousefunctions");
		lua_pushvalue(l, -2);
		lua_rawset(l, LUA_REGISTRYINDEX);
	}
	int len = lua_objlen(l, -1);
	for (int i = 1; i <= len && mpcontinue; i++)
	{
		lua_rawgeti(l, -1, i);
		lua_pushinteger(l, mx);
		lua_pushinteger(l, my);
		lua_pushinteger(l, mb);
		lua_pushinteger(l, event);
		lua_pushinteger(l, mouse_wheel);
		int callret = lua_pcall(l, 5, 1, 0);
		if (callret)
		{
			if (!strcmp(luacon_geterror(), "Error: Script not responding"))
			{
				ui::Engine::Ref().LastTick(Platform::GetTime());
				for (int j = i; j <= len-1; j++)
				{
					lua_rawgeti(l, -2, j+1);
					lua_rawseti(l, -3, j);
				}
				lua_pushnil(l);
				lua_rawseti(l, -3, len);
				i--;
			}
			luacon_ci->Log(CommandInterface::LogError, luacon_geterror());
			lua_pop(l, 1);
		}
		else
		{
			if(!lua_isnoneornil(l, -1))
				mpcontinue = lua_toboolean(l, -1);
			lua_pop(l, 1);
		}
		len = lua_objlen(l, -1);
	}
	lua_pop(l, 1);
	return mpcontinue;
}

int luacon_step(int mx, int my)
{
	ui::Engine::Ref().LastTick(Platform::GetTime());
	lua_State* l = luacon_ci->l;
	lua_pushinteger(l, my);
	lua_pushinteger(l, mx);
	lua_setfield(l, tptProperties, "mousex");
	lua_setfield(l, tptProperties, "mousey");
	lua_pushstring(l, "stepfunctions");
	lua_rawget(l, LUA_REGISTRYINDEX);
	if (!lua_istable(l, -1))
	{
		lua_pop(l, 1);
		lua_newtable(l);
		lua_pushstring(l, "stepfunctions");
		lua_pushvalue(l, -2);
		lua_rawset(l, LUA_REGISTRYINDEX);
	}
	int len = lua_objlen(l, -1);
	for (int i = 1; i <= len; i++)
	{
		lua_rawgeti(l, -1, i);
		int callret = lua_pcall(l, 0, 0, 0);
		if (callret)
		{
			if (!strcmp(luacon_geterror(), "Error: Script not responding"))
			{
				ui::Engine::Ref().LastTick(Platform::GetTime());
				for (int j = i; j <= len-1; j++)
				{
					lua_rawgeti(l, -2, j+1);
					lua_rawseti(l, -3, j);
				}
				lua_pushnil(l);
				lua_rawseti(l, -3, len);
				i--;
			}
			luacon_ci->Log(CommandInterface::LogError, luacon_geterror());
			lua_pop(l, 1);
		}
		len = lua_objlen(l, -1);
	}
	lua_pop(l, 1);
	return 0;
}


int luacon_eval(const char *command)
{
	ui::Engine::Ref().LastTick(Platform::GetTime());
	return luaL_dostring (luacon_ci->l, command);
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

int luaL_tostring (lua_State *L, int n)
{
	luaL_checkany(L, n);
	switch (lua_type(L, n))
	{
		case LUA_TNUMBER:
			lua_pushstring(L, lua_tostring(L, n));
			break;
		case LUA_TSTRING:
			lua_pushvalue(L, n);
			break;
		case LUA_TBOOLEAN:
			lua_pushstring(L, (lua_toboolean(L, n) ? "true" : "false"));
			break;
		case LUA_TNIL:
			lua_pushliteral(L, "nil");
			break;
		default:
			lua_pushfstring(L, "%s: %p", luaL_typename(L, n), lua_topointer(L, n));
			break;
	}
	return 1;
}

const char *luacon_geterror()
{
	luaL_tostring(luacon_ci->l, -1);
	const char* err = luaL_optstring(luacon_ci->l, -1, "failed to execute");
	lua_pop(luacon_ci->l, 1);
	return err;
}

//TPT Interface methods
int luatpt_test(lua_State* l)
{
	int testint = 0;
	testint = luaL_optint(l, 1, 0);
	printf("Test successful, got %d\n", testint);
	return 0;
}

int luatpt_getelement(lua_State *l)
{
	int t;
	if (lua_isnumber(l, 1))
	{
		t = luaL_optint(l, 1, 1);
		if (t<0 || t>=PT_NUM)
			return luaL_error(l, "Unrecognised element number '%d'", t);
		lua_pushstring(l, luacon_sim->elements[t].Name);
	}
	else
	{
		luaL_checktype(l, 1, LUA_TSTRING);
		const char* name = luaL_optstring(l, 1, "");
		if ((t = luacon_sim->GetParticleType(name))==-1)
			return luaL_error(l, "Unrecognised element '%s'", name);
		lua_pushinteger(l, t);
	}
	return 1;
}

int luacon_elementReplacement(UPDATE_FUNC_ARGS)
{
	int retval = 0, callret;
	if (lua_el_func[parts[i].type])
	{
		lua_rawgeti(luacon_ci->l, LUA_REGISTRYINDEX, lua_el_func[parts[i].type]);
		lua_pushinteger(luacon_ci->l, i);
		lua_pushinteger(luacon_ci->l, x);
		lua_pushinteger(luacon_ci->l, y);
		lua_pushinteger(luacon_ci->l, surround_space);
		lua_pushinteger(luacon_ci->l, nt);
		callret = lua_pcall(luacon_ci->l, 5, 1, 0);
		if (callret)
			luacon_ci->Log(CommandInterface::LogError, luacon_geterror());
		if(lua_isboolean(luacon_ci->l, -1)){
			retval = lua_toboolean(luacon_ci->l, -1);
		}
		lua_pop(luacon_ci->l, 1);
	}
	return retval;
}

int luatpt_element_func(lua_State *l)
{
	if(lua_isfunction(l, 1))
	{
		int element = luaL_optint(l, 2, 0);
		int replace = luaL_optint(l, 3, 0);
		int function;
		lua_pushvalue(l, 1);
		function = luaL_ref(l, LUA_REGISTRYINDEX);
		if(element > 0 && element < PT_NUM)
		{
			lua_el_func[element] = function;
			if (replace == 2)
				lua_el_mode[element] = 3; //update before
			else if (replace)
				lua_el_mode[element] = 2; //replace
			else
				lua_el_mode[element] = 1; //update after
			return 0;
		}
		else
		{
			return luaL_error(l, "Invalid element");
		}
	}
	else if(lua_isnil(l, 1))
	{
		int element = luaL_optint(l, 2, 0);
		if(element > 0 && element < PT_NUM)
		{
			lua_el_func[element] = 0;
			lua_el_mode[element] = 0;
		}
		else
		{
			return luaL_error(l, "Invalid element");
		}
	}
	else
		return luaL_error(l, "Not a function");
	return 0;
}

int luacon_graphicsReplacement(GRAPHICS_FUNC_ARGS, int i)
{
	int cache = 0, callret;
	lua_rawgeti(luacon_ci->l, LUA_REGISTRYINDEX, lua_gr_func[cpart->type]);
	lua_pushinteger(luacon_ci->l, i);
	lua_pushinteger(luacon_ci->l, *colr);
	lua_pushinteger(luacon_ci->l, *colg);
	lua_pushinteger(luacon_ci->l, *colb);
	callret = lua_pcall(luacon_ci->l, 4, 10, 0);
	if (callret)
	{
		luacon_ci->Log(CommandInterface::LogError, luacon_geterror());
		lua_pop(luacon_ci->l, 1);
	}
	else
	{
		cache = luaL_optint(luacon_ci->l, -10, 0);
		*pixel_mode = luaL_optint(luacon_ci->l, -9, *pixel_mode);
		*cola = luaL_optint(luacon_ci->l, -8, *cola);
		*colr = luaL_optint(luacon_ci->l, -7, *colr);
		*colg = luaL_optint(luacon_ci->l, -6, *colg);
		*colb = luaL_optint(luacon_ci->l, -5, *colb);
		*firea = luaL_optint(luacon_ci->l, -4, *firea);
		*firer = luaL_optint(luacon_ci->l, -3, *firer);
		*fireg = luaL_optint(luacon_ci->l, -2, *fireg);
		*fireb = luaL_optint(luacon_ci->l, -1, *fireb);
		lua_pop(luacon_ci->l, 10);
	}
	return cache;
}

int luatpt_graphics_func(lua_State *l)
{
	if(lua_isfunction(l, 1))
	{
		int element = luaL_optint(l, 2, 0);
		int function;
		lua_pushvalue(l, 1);
		function = luaL_ref(l, LUA_REGISTRYINDEX);
		if (element > 0 && element < PT_NUM)
		{
			lua_gr_func[element] = function;
			luacon_ren->graphicscache[element].isready = 0;
			return 0;
		}
		else
		{
			return luaL_error(l, "Invalid element");
		}
	}
	else if (lua_isnil(l, 1))
	{
		int element = luaL_optint(l, 2, 0);
		if (element > 0 && element < PT_NUM)
		{
			lua_gr_func[element] = 0;
			luacon_ren->graphicscache[element].isready = 0;
			return 0;
		}
		else
		{
			return luaL_error(l, "Invalid element");
		}
	}
	else
		return luaL_error(l, "Not a function");
	return 0;
}

int luatpt_error(lua_State* l)
{
	std::string errorMessage = std::string(luaL_optstring(l, 1, "Error text"));
	ErrorMessage::Blocking("Error", errorMessage);
	return 0;
}

int luatpt_drawtext(lua_State* l)
{
	const char *string;
	int textx, texty, textred, textgreen, textblue, textalpha;
	textx = luaL_optint(l, 1, 0);
	texty = luaL_optint(l, 2, 0);
	string = luaL_optstring(l, 3, "");
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
		if(lua_isnumber(l, 3))
		{
			t = luaL_optint(l, 3, 0);
			if (t<0 || t >= PT_NUM || !luacon_sim->elements[t].Enabled)
				return luaL_error(l, "Unrecognised element number '%d'", t);
		} else {
			const char* name = luaL_optstring(l, 3, "dust");
			if ((t = luacon_sim->GetParticleType(std::string(name))) == -1)
				return luaL_error(l,"Unrecognised element '%s'", name);
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
	std::string text = "";
	for(int i = 1; i <= args; i++)
	{
		luaL_tostring(l, -1);
		if(text.length())
			text=std::string(luaL_optstring(l, -1, "")) + ", " + text;
		else
			text=std::string(luaL_optstring(l, -1, ""));
		lua_pop(l, 2);
	}
	if((*luacon_currentCommand))
	{
		if(luacon_lastError->length())
			*luacon_lastError += "; ";
		*luacon_lastError += text;
	}
	else
		luacon_ci->Log(CommandInterface::LogNotice, text.c_str());
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
	value = (float)luaL_optint(l, 5, 0.0f);
	if(value > 256.0f)
		value = 256.0f;
	else if(value < -256.0f)
		value = -256.0f;

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
	value = (float)luaL_optint(l, 5, 0.0f);
	if(value > 256.0f)
		value = 256.0f;
	else if(value < -256.0f)
		value = -256.0f;

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
	const char *name;
	int r, i, x, y, w, h, t, nx, ny, partsel = 0;
	float f;
	int acount = lua_gettop(l);
	const char* prop = luaL_optstring(l, 1, "");

	CommandInterface::FormatType format;
	int offset = luacon_ci->GetPropertyOffset(prop, format);
	if (offset == -1)
		return luaL_error(l, "Invalid property '%s'", prop);

	if (acount > 2)
	{
		if(!lua_isnumber(l, acount) && lua_isstring(l, acount))
		{
			name = luaL_optstring(l, acount, "none");
			if ((partsel = luacon_sim->GetParticleType(std::string(name))) == -1)
				return luaL_error(l, "Unrecognised element '%s'", name);
		}
	}
	if (lua_isnumber(l, 2))
	{
		if (format == CommandInterface::FormatFloat)
			f = luaL_optnumber(l, 2, 0);
		else
			t = luaL_optint(l, 2, 0);

		if (!strcmp(prop,"type") && (t<0 || t>=PT_NUM || !luacon_sim->elements[t].Enabled))
			return luaL_error(l, "Unrecognised element number '%d'", t);
	}
	else
	{
		name = luaL_checklstring(l, 2, NULL);
		if ((t = luacon_sim->GetParticleType(std::string(name)))==-1)
			return luaL_error(l, "Unrecognised element '%s'", name);
	}
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
			luacon_sim->part_change_type(i, luacon_sim->parts[i].x, luacon_sim->parts[i].y, t);
		else if (format == CommandInterface::FormatFloat)
			*((float*)(((unsigned char*)&luacon_sim->parts[i])+offset)) = f;
		else
			*((int*)(((unsigned char*)&luacon_sim->parts[i])+offset)) = t;
	}
	return 0;
}

int luatpt_set_wallmap(lua_State* l)
{
	int nx, ny, acount;
	int x1, y1, width, height, wallType;
	acount = lua_gettop(l);

	x1 = abs(luaL_optint(l, 1, 0));
	y1 = abs(luaL_optint(l, 2, 0));
	width = abs(luaL_optint(l, 3, XRES/CELL));
	height = abs(luaL_optint(l, 4, YRES/CELL));
	wallType = luaL_optint(l, acount, 0);
	if (wallType < 0 || wallType >= UI_WALLCOUNT)
		return luaL_error(l, "Unrecognised wall number %d", wallType);

	if (acount == 5)	//Draw rect
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
				luacon_sim->bmap[ny][nx] = wallType;
			}
	}
	else	//Set point
	{
		if(x1 > (XRES/CELL))
			x1 = (XRES/CELL);
		if(y1 > (YRES/CELL))
			y1 = (YRES/CELL);
		luacon_sim->bmap[y1][x1] = wallType;
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

int luatpt_set_elecmap(lua_State* l)
{
	int nx, ny, acount;
	int x1, y1, width, height;
	float value;
	acount = lua_gettop(l);

	x1 = abs(luaL_optint(l, 1, 0));
	y1 = abs(luaL_optint(l, 2, 0));
	width = abs(luaL_optint(l, 3, XRES/CELL));
	height = abs(luaL_optint(l, 4, YRES/CELL));
	value = (float)luaL_optint(l, acount, 0);

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
	std::string prop = luaL_optstring(l, 1, "");
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
				if (!prop.compare("type"))
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
			if (!prop.compare("id"))
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
	else if (!prop.compare("type"))
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
	int strwidth = 0;
	const char* string = luaL_optstring(l, 1, "");
	strwidth = Graphics::textwidth(string);
	lua_pushinteger(l, strwidth);
	return 1;
}

int luatpt_get_name(lua_State* l)
{
	if (luacon_model->GetUser().UserID)
	{
		lua_pushstring(l, luacon_model->GetUser().Username.c_str());
		return 1;
	}
	lua_pushstring(l, "");
	return 1;
}

int luatpt_set_shortcuts(lua_State* l)
{
	int acount = lua_gettop(l);
	if (acount == 0)
	{
		lua_pushnumber(l, shortcuts);
		return 1;
	}
	int shortcut = luaL_checkinteger(l, 1);
	shortcuts = shortcut?true:false;
	return 0;
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

int luatpt_register_step(lua_State* l)
{
	if (lua_isfunction(l, 1))
	{
		lua_pushstring(l, "stepfunctions");
		lua_rawget(l, LUA_REGISTRYINDEX);
		if (!lua_istable(l, -1))
		{
			lua_pop(l, 1);
			lua_newtable(l);
			lua_pushstring(l, "stepfunctions");
			lua_pushvalue(l, -2);
			lua_rawset(l, LUA_REGISTRYINDEX);
		}
		int c = lua_objlen(l, -1);
		lua_pushvalue(l, 1);
		lua_rawseti(l, -2, c+1);
	}
	return 0;
}

int luatpt_unregister_step(lua_State* l)
{
	if (lua_isfunction(l, 1))
	{
		lua_pushstring(l, "stepfunctions");
		lua_rawget(l, LUA_REGISTRYINDEX);
		if (!lua_istable(l, -1))
		{
			lua_pop(l, -1);
			lua_newtable(l);
			lua_pushstring(l, "stepfunctions");
			lua_pushvalue(l, -2);
			lua_rawset(l, LUA_REGISTRYINDEX);
		}
		int len = lua_objlen(l, -1);
		int adjust = 0;
		for (int i = 1; i <= len; i++)
		{
			lua_rawgeti(l, -1, i+adjust);
			//unregister the function
			if (lua_equal(l, 1, -1))
			{
				lua_pop(l, 1);
				adjust++;
				i--;
			}
			//else, move everything down if we removed something earlier
			else
			{
				lua_rawseti(l, -2, i);
			}
		}
	}
	return 0;
}

int luatpt_register_keypress(lua_State* l)
{
	if (lua_isfunction(l, 1))
	{
		lua_pushstring(l, "keyfunctions");
		lua_rawget(l, LUA_REGISTRYINDEX);
		if (!lua_istable(l, -1))
		{
			lua_pop(l, 1);
			lua_newtable(l);
			lua_pushstring(l, "keyfunctions");
			lua_pushvalue(l, -2);
			lua_rawset(l, LUA_REGISTRYINDEX);
		}
		int c = lua_objlen(l, -1);
		lua_pushvalue(l, 1);
		lua_rawseti(l, -2, c+1);
	}
	return 0;
}

int luatpt_unregister_keypress(lua_State* l)
{
	if (lua_isfunction(l, 1))
	{
		lua_pushstring(l, "keyfunctions");
		lua_rawget(l, LUA_REGISTRYINDEX);
		if (!lua_istable(l, -1))
		{
			lua_pop(l, 1);
			lua_newtable(l);
			lua_pushstring(l, "keyfunctions");
			lua_pushvalue(l, -2);
			lua_rawset(l, LUA_REGISTRYINDEX);
		}
		int c = lua_objlen(l, -1);
		int d = 0;
		int i = 0;
		for (i=1;i<=c;i++)
		{
			lua_rawgeti(l, -1, i+d);
			if (lua_equal(l, 1, -1))
			{
				lua_pop(l, 1);
				d++;
				i--;
			}
			else
				lua_rawseti(l, -2, i);
		}
	}
	return 0;
}

int luatpt_register_mouseclick(lua_State* l)
{
	if (lua_isfunction(l, 1))
	{
		lua_pushstring(l, "mousefunctions");
		lua_rawget(l, LUA_REGISTRYINDEX);
		if (!lua_istable(l, -1))
		{
			lua_pop(l, 1);
			lua_newtable(l);
			lua_pushstring(l, "mousefunctions");
			lua_pushvalue(l, -2);
			lua_rawset(l, LUA_REGISTRYINDEX);
		}
		int c = lua_objlen(l, -1);
		lua_pushvalue(l, 1);
		lua_rawseti(l, -2, c+1);
	}
	return 0;
}

int luatpt_unregister_mouseclick(lua_State* l)
{
	if (lua_isfunction(l, 1))
	{
		lua_pushstring(l, "mousefunctions");
		lua_rawget(l, LUA_REGISTRYINDEX);
		if (!lua_istable(l, -1))
		{
			lua_pop(l, 1);
			lua_newtable(l);
			lua_pushstring(l, "mousefunctions");
			lua_pushvalue(l, -2);
			lua_rawset(l, LUA_REGISTRYINDEX);
		}
		int c = lua_objlen(l, -1);
		int d = 0;
		int i = 0;
		for (i=1;i<=c;i++)
		{
			lua_rawgeti(l, -1, i+d);
			if (lua_equal(l, 1, -1))
			{
				lua_pop(l, 1);
				d++;
				i--;
			}
			else
				lua_rawseti(l, -2, i);
		}
	}
	return 0;
}

int luatpt_input(lua_State* l)
{
	std::string prompt, title, result, shadow, text;
	title = std::string(luaL_optstring(l, 1, "Title"));
	prompt = std::string(luaL_optstring(l, 2, "Enter some text:"));
	text = std::string(luaL_optstring(l, 3, ""));
	shadow = std::string(luaL_optstring(l, 4, ""));

	result = TextPrompt::Blocking(title, prompt, text, shadow, false);

	lua_pushstring(l, result.c_str());
	return 1;
}

int luatpt_message_box(lua_State* l)
{
	std::string title = std::string(luaL_optstring(l, 1, "Title"));
	std::string message = std::string(luaL_optstring(l, 2, "Message"));
	int large = lua_toboolean(l, 3);
	new InformationMessage(title, message, large);
	return 0;
}

int luatpt_confirm(lua_State *l)
{
	std::string title = std::string(luaL_optstring(l, 1, "Title"));
	std::string message = std::string(luaL_optstring(l, 2, "Message"));
	std::string buttonText = std::string(luaL_optstring(l, 3, "Confirm"));
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
		lua_pushinteger(l, luacon_sim->grav->ngrav_enable);
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
		lua_pushinteger(l, ui::Engine::Ref().FpsLimit);
		return 1;
	}
	int fpscap = luaL_checkint(l, 1);
	if (fpscap < 2)
		return luaL_error(l, "fps cap too small");
	ui::Engine::Ref().FpsLimit = fpscap;
	return 0;
}

int luatpt_getscript(lua_State* l)
{
	int scriptID = luaL_checkinteger(l, 1);
	const char *filename = luaL_checkstring(l, 2);
	int runScript = luaL_optint(l, 3, 0);
	int confirmPrompt = luaL_optint(l, 4, 1);

	std::stringstream url;
	url << "http://starcatcher.us/scripts/main.lua?get=" << scriptID;
	if (confirmPrompt && !ConfirmPrompt::Blocking("Do you want to install script?", url.str(), "Install"))
		return 0;

	int ret, len;
	char *scriptData = http_simple_get(url.str().c_str(), &ret, &len);
	if (len <= 0 || !filename)
	{
		free(scriptData);
		return luaL_error(l, "Server did not return data");
	}
	if (ret != 200)
	{
		free(scriptData);
		return luaL_error(l, http_ret_text(ret));
	}

	if (!strcmp(scriptData, "Invalid script ID\r\n"))
	{
		free(scriptData);
		return luaL_error(l, "Invalid Script ID");
	}

	FILE *outputfile = fopen(filename, "r");
	if (outputfile)
	{
		fclose(outputfile);
		outputfile = NULL;
		if (!confirmPrompt || ConfirmPrompt::Blocking("File already exists, overwrite?", filename, "Overwrite"))
		{
			outputfile = fopen(filename, "wb");
		}
		else
		{
			free(scriptData);
			return 0;
		}
	}
	else
	{
		outputfile = fopen(filename, "wb");
	}
	if (!outputfile)
	{
		free(scriptData);
		return luaL_error(l, "Unable to write to file");
	}

	fputs(scriptData, outputfile);
	fclose(outputfile);
	outputfile = NULL;
	if (runScript)
	{
		std::stringstream luaCommand;
		luaCommand << "dofile('" << filename << "')";
		luaL_dostring(l, luaCommand.str().c_str());
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
	ui::Engine::Ref().SetScale(scale);
	ui::Engine::Ref().SetFullscreen(kiosk);
	return 0;
}

int screenshotIndex = 0;

int luatpt_screenshot(lua_State* l)
{
	int captureUI = luaL_optint(l, 1, 0);
	int fileType = luaL_optint(l, 2, 0);
	std::vector<char> data;
	if(captureUI)
	{
		VideoBuffer screenshot(ui::Engine::Ref().g->DumpFrame());
		if(fileType == 1) {
			data = format::VideoBufferToBMP(screenshot);
		} else if(fileType == 2) {
			data = format::VideoBufferToPPM(screenshot);
		} else {
			data = format::VideoBufferToPNG(screenshot);
		}
	}
	else
	{
		VideoBuffer screenshot(luacon_ren->DumpFrame());
		if(fileType == 1) {
			data = format::VideoBufferToBMP(screenshot);
		} else if(fileType == 2) {
			data = format::VideoBufferToPPM(screenshot);
		} else {
			data = format::VideoBufferToPNG(screenshot);
		}
	}
	std::stringstream filename;
	filename << "screenshot_";
	filename << std::setfill('0') << std::setw(6) << (screenshotIndex++);
	if(fileType == 1) {
		filename << ".bmp";
	} else if(fileType == 2) {
		filename << ".ppm";
	} else {
		filename << ".png";
	}
	Client::Ref().WriteFile(data, filename.str());
	lua_pushstring(l, filename.str().c_str());
	return 1;
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

#endif
