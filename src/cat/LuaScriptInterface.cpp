/*
 * LuaScriptInterface.cpp
 *
 *  Created on: Feb 11, 2012
 *      Author: Simon
 */

#include <string>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <locale>
#include "Config.h"
#include "Format.h"
#include "LuaLuna.h"
#include "LuaScriptInterface.h"
#include "TPTScriptInterface.h"
#include "dialogues/ErrorMessage.h"
#include "dialogues/InformationMessage.h"
#include "dialogues/TextPrompt.h"
#include "dialogues/ConfirmPrompt.h" 
#include "simulation/Simulation.h"
#include "game/GameModel.h"
#include "LuaScriptHelper.h"
#include "client/HTTP.h"

#include "LuaBit.h"

#include "LuaWindow.h"
#include "LuaButton.h"
#include "LuaLabel.h"
#include "LuaTextbox.h"

#ifdef WIN
#include <direct.h>
#else
#include <sys/stat.h>
#endif
#include <time.h>

LuaScriptInterface::LuaScriptInterface(GameController * c, GameModel * m):
	CommandInterface(c, m),
	currentCommand(false),
	legacy(new TPTScriptInterface(c, m))
{
	luacon_model = m;
	luacon_sim = m->GetSimulation();
	luacon_g = ui::Engine::Ref().g;
	luacon_ren = m->GetRenderer();
	luacon_ci = this;


	//New TPT API
	l = lua_open();
	luaL_openlibs(l);
	luaopen_bit(l);

	lua_pushstring(l, "Luacon_ci");
	lua_pushlightuserdata(l, this);
	lua_settable(l, LUA_REGISTRYINDEX);

	initSimulationAPI();
	initInterfaceAPI();
	initRendererAPI();
	initElementsAPI();

	//Old TPT API
	int i = 0, j;
	char tmpname[12];
	int currentElementMeta, currentElement;
	const static struct luaL_reg tptluaapi [] = {
		{"test", &luatpt_test},
		{"drawtext", &luatpt_drawtext},
		{"create", &luatpt_create},
		{"set_pause", &luatpt_setpause},
		{"toggle_pause", &luatpt_togglepause},
		{"set_console", &luatpt_setconsole},
		{"log", &luatpt_log},
		{"set_pressure", &luatpt_set_pressure},
		{"set_gravity", &luatpt_set_gravity},
		{"reset_gravity_field", &luatpt_reset_gravity_field},
		{"reset_velocity", &luatpt_reset_velocity},
		{"reset_spark", &luatpt_reset_spark},
		{"set_property", &luatpt_set_property},
		{"get_property", &luatpt_get_property},
		{"set_wallmap", &luatpt_set_wallmap},
		{"get_wallmap", &luatpt_get_wallmap},
		{"set_elecmap", &luatpt_set_elecmap},
		{"get_elecmap", &luatpt_get_elecmap},
		{"drawpixel", &luatpt_drawpixel},
		{"drawrect", &luatpt_drawrect},
		{"fillrect", &luatpt_fillrect},
		{"drawline", &luatpt_drawline},
		{"textwidth", &luatpt_textwidth},
		{"get_name", &luatpt_get_name},
		{"set_shortcuts", &luatpt_set_shortcuts},
		{"delete", &luatpt_delete},
		{"register_step", &luatpt_register_step},
		{"unregister_step", &luatpt_unregister_step},
		{"register_mouseclick", &luatpt_register_mouseclick},
		{"unregister_mouseclick", &luatpt_unregister_mouseclick},
		{"register_keypress", &luatpt_register_keypress},
		{"unregister_keypress", &luatpt_unregister_keypress},
		{"register_mouseevent", &luatpt_register_mouseclick},
		{"unregister_mouseevent", &luatpt_unregister_mouseclick},
		{"register_keyevent", &luatpt_register_keypress},
		{"unregister_keyevent", &luatpt_unregister_keypress},
		{"input", &luatpt_input},
		{"message_box", &luatpt_message_box},
		{"get_numOfParts", &luatpt_get_numOfParts},
		{"start_getPartIndex", &luatpt_start_getPartIndex},
		{"next_getPartIndex", &luatpt_next_getPartIndex},
		{"getPartIndex", &luatpt_getPartIndex},
		{"hud", &luatpt_hud},
		{"newtonian_gravity", &luatpt_gravity},
		{"ambient_heat", &luatpt_airheat},
		{"active_menu", &luatpt_active_menu},
		{"decorations_enable", &luatpt_decorations_enable},
		{"display_mode", &luatpt_cmode_set},
		{"throw_error", &luatpt_error},
		{"heat", &luatpt_heat},
		{"setfire", &luatpt_setfire},
		{"setdebug", &luatpt_setdebug},
		{"setfpscap",&luatpt_setfpscap},
		{"getscript",&luatpt_getscript},
		{"setwindowsize",&luatpt_setwindowsize},
		{"watertest",&luatpt_togglewater},
		{"screenshot",&luatpt_screenshot},
		{"element",&luatpt_getelement},
		{"element_func",&luatpt_element_func},
		{"graphics_func",&luatpt_graphics_func},
		{NULL,NULL}
	};

	luacon_mousedown = false;
	luacon_mousebutton = 0;

	luacon_currentCommand = &currentCommand;
	luacon_lastError = &lastError;

	//Replace print function with our screen logging thingy
	lua_pushcfunction(l, luatpt_log);
	lua_setglobal(l, "print");

	//Register all tpt functions
	luaL_register(l, "tpt", tptluaapi);

	tptProperties = lua_gettop(l);

	lua_pushinteger(l, 0);
	lua_setfield(l, tptProperties, "mousex");
	lua_pushinteger(l, 0);
	lua_setfield(l, tptProperties, "mousey");
	lua_pushinteger(l, 0);
	lua_setfield(l, tptProperties, "selectedl");
	lua_pushinteger(l, 0);
	lua_setfield(l, tptProperties, "selectedr");

	lua_newtable(l);
	tptPropertiesVersion = lua_gettop(l);
	lua_pushinteger(l, SAVE_VERSION);
	lua_setfield(l, tptPropertiesVersion, "major");
	lua_pushinteger(l, MINOR_VERSION);
	lua_setfield(l, tptPropertiesVersion, "minor");
	lua_pushinteger(l, BUILD_NUM);
	lua_setfield(l, tptPropertiesVersion, "build");
	lua_setfield(l, tptProperties, "version");

	lua_sethook(l, &luacon_hook, LUA_MASKCOUNT, 200);
#ifdef FFI
	//LuaJIT's ffi gives us direct access to parts data, no need for nested metatables. HOWEVER, this is in no way safe, it's entirely possible for someone to try to read parts[-10]
	lua_pushlightuserdata(l, parts);
	lua_setfield(l, tptProperties, "partsdata");

	luaL_dostring (l, "ffi = require(\"ffi\")\n\
ffi.cdef[[\n\
typedef struct { int type; int life, ctype; float x, y, vx, vy; float temp; float pavg[2]; int flags; int tmp; int tmp2; unsigned int dcolour; } particle;\n\
]]\n\
tpt.parts = ffi.cast(\"particle *\", tpt.partsdata)\n\
ffi = nil\n\
tpt.partsdata = nil");
	//Since ffi is REALLY REALLY dangrous, we'll remove it from the environment completely (TODO)
	//lua_pushstring(l, "parts");
	//tptPartsCData = lua_gettable(l, tptProperties);
#else
	lua_newtable(l);
	tptParts = lua_gettop(l);
	lua_newtable(l);
	tptPartsMeta = lua_gettop(l);
	lua_pushcfunction(l, luacon_partswrite);
	lua_setfield(l, tptPartsMeta, "__newindex");
	lua_pushcfunction(l, luacon_partsread);
	lua_setfield(l, tptPartsMeta, "__index");
	lua_setmetatable(l, tptParts);
	lua_setfield(l, tptProperties, "parts");

	lua_newtable(l);
	tptPart = lua_gettop(l);
	lua_newtable(l);
	tptPartMeta = lua_gettop(l);
	lua_pushcfunction(l, luacon_partwrite);
	lua_setfield(l, tptPartMeta, "__newindex");
	lua_pushcfunction(l, luacon_partread);
	lua_setfield(l, tptPartMeta, "__index");
	lua_setmetatable(l, tptPart);

	tptPart = luaL_ref(l, LUA_REGISTRYINDEX);
#endif

	lua_newtable(l);
	tptElements = lua_gettop(l);
	for(i = 1; i < PT_NUM; i++)
	{
		for(j = 0; j < strlen(luacon_sim->elements[i].Name); j++)
			tmpname[j] = tolower(luacon_sim->elements[i].Name[j]);
		tmpname[strlen(luacon_sim->elements[i].Name)] = 0;

		lua_newtable(l);
		currentElement = lua_gettop(l);
		lua_pushinteger(l, i);
		lua_setfield(l, currentElement, "id");

		lua_newtable(l);
		currentElementMeta = lua_gettop(l);
		lua_pushcfunction(l, luacon_elementwrite);
		lua_setfield(l, currentElementMeta, "__newindex");
		lua_pushcfunction(l, luacon_elementread);
		lua_setfield(l, currentElementMeta, "__index");
		lua_setmetatable(l, currentElement);

		lua_setfield(l, tptElements, tmpname);
	}
	lua_setfield(l, tptProperties, "el");

	lua_newtable(l);
	tptElementTransitions = lua_gettop(l);
	for(i = 1; i < PT_NUM; i++)
	{
		for(j = 0; j < strlen(luacon_sim->elements[i].Name); j++)
			tmpname[j] = tolower(luacon_sim->elements[i].Name[j]);
		tmpname[strlen(luacon_sim->elements[i].Name)] = 0;

		lua_newtable(l);
		currentElement = lua_gettop(l);
		lua_newtable(l);
		currentElementMeta = lua_gettop(l);
		lua_pushinteger(l, i);
		lua_setfield(l, currentElement, "value");
		lua_pushcfunction(l, luacon_transitionwrite);
		lua_setfield(l, currentElementMeta, "__newindex");
		lua_pushcfunction(l, luacon_transitionread);
		lua_setfield(l, currentElementMeta, "__index");
		lua_setmetatable(l, currentElement);

		lua_setfield(l, tptElementTransitions, tmpname);
	}
	lua_setfield(l, tptProperties, "eltransition");

	lua_el_func = (int*)calloc(PT_NUM, sizeof(int));
	lua_el_mode = (int*)calloc(PT_NUM, sizeof(int));
	lua_gr_func = (int*)calloc(PT_NUM, sizeof(int));
	for(i = 0; i < PT_NUM; i++)
	{
		lua_el_mode[i] = 0;
	}

}

void LuaScriptInterface::Init()
{
	if(luacon_eval("dofile(\"autorun.lua\")"))
	{
		luacon_ci->Log(CommandInterface::LogError, luacon_geterror());
	}
}

void LuaScriptInterface::SetWindow(ui::Window * window)
{
	Window = window;
}

//// Begin Interface API

void LuaScriptInterface::initInterfaceAPI()
{
	struct luaL_reg interfaceAPIMethods [] = {
		{"showWindow", interface_showWindow},
		{"closeWindow", interface_closeWindow},
		{"addComponent", interface_addComponent},
		{NULL, NULL}
	};
	luaL_register(l, "interface", interfaceAPIMethods);

	//Ren shortcut
	lua_getglobal(l, "interface");
	lua_setglobal(l, "ui");

	Luna<LuaWindow>::Register(l);
	Luna<LuaButton>::Register(l);
	Luna<LuaLabel>::Register(l);
	Luna<LuaTextbox>::Register(l);
}

int LuaScriptInterface::interface_addComponent(lua_State * l)
{
	void * luaComponent = NULL;
	ui::Component * component = NULL;
	if(luaComponent = Luna<LuaButton>::tryGet(l, 1))
		component = Luna<LuaButton>::get(luaComponent)->GetComponent();
	else if(luaComponent = Luna<LuaLabel>::tryGet(l, 1))
		component = Luna<LuaLabel>::get(luaComponent)->GetComponent();
	else if(luaComponent = Luna<LuaTextbox>::tryGet(l, 1))
		component = Luna<LuaTextbox>::get(luaComponent)->GetComponent();
	else
		luaL_typerror(l, 1, "Component");
	if(luacon_ci->Window && component)
		luacon_ci->Window->AddComponent(component);
	return 0;
}

int LuaScriptInterface::interface_showWindow(lua_State * l)
{
	LuaWindow * window = Luna<LuaWindow>::check(l, 1);
	if(window && ui::Engine::Ref().GetWindow()!=window->GetWindow())
		ui::Engine::Ref().ShowWindow(window->GetWindow());
	return 0;
}

int LuaScriptInterface::interface_closeWindow(lua_State * l)
{
	LuaWindow * window = Luna<LuaWindow>::check(l, 1);
	if(window && ui::Engine::Ref().GetWindow()==window->GetWindow())
		ui::Engine::Ref().CloseWindow();
	return 0;
}

//// Begin Simulation API

void LuaScriptInterface::initSimulationAPI()
{
	//Methods
	struct luaL_reg simulationAPIMethods [] = {
		{"partNeighbours", simulation_partNeighbours},
		{"partChangeType", simulation_partChangeType},
		{"partCreate", simulation_partCreate},
		{"partKill", simulation_partKill},
		{NULL, NULL}
	};
	luaL_register(l, "simulation", simulationAPIMethods);
	int simulationAPI = lua_gettop(l);

	//Sim shortcut
	lua_getglobal(l, "simulation");
	lua_setglobal(l, "sim");

}

int LuaScriptInterface::simulation_partNeighbours(lua_State * l)
{
	int ids = 0;
	if(lua_gettop(l) == 4)
	{
		int x = lua_tointeger(l, 1), y = lua_tointeger(l, 2), r = lua_tointeger(l, 3), t = lua_tointeger(l, 4), rx, ry, n;
		for (rx = -r; rx <= r; rx++)
			for (ry = -r; ry <= r; ry++)
				if (x+rx >= 0 && y+ry >= 0 && x+rx < XRES && y+ry < YRES && (rx || ry))
				{
					n = luacon_sim->pmap[y+ry][x+rx];
					if(n && (n&0xFF) == t)
					{
						ids++;
						lua_pushinteger(l, n>>8);
					}
				}

	}
	else
	{
		int x = lua_tointeger(l, 1), y = lua_tointeger(l, 2), r = lua_tointeger(l, 3), rx, ry, n;
		std::cout << x << " " << y << " " << r << std::endl;
		for (rx = -r; rx <= r; rx++)
			for (ry = -r; ry <= r; ry++)
				if (x+rx >= 0 && y+ry >= 0 && x+rx < XRES && y+ry < YRES && (rx || ry))
				{
					n = luacon_sim->pmap[y+ry][x+rx];
					if(n)
					{
						ids++;
						lua_pushinteger(l, n>>8);
					}
				}
	}
	return ids;
}

int LuaScriptInterface::simulation_partChangeType(lua_State * l)
{
	int partIndex = lua_tointeger(l, 1), x, y;
	if(partIndex < 0 || partIndex >= NPART || !luacon_sim->parts[partIndex].type)
		return 0;
	luacon_sim->part_change_type(partIndex, luacon_sim->parts[partIndex].x+0.5f, luacon_sim->parts[partIndex].y+0.5f, lua_tointeger(l, 2));
	return 0;
}

int LuaScriptInterface::simulation_partCreate(lua_State * l)
{
	int newID = lua_tointeger(l, 1);
	if(newID >= NPART || newID < -3)
	{
		lua_pushinteger(l, -1);
		return 1;
	}
	lua_pushinteger(l, luacon_sim->create_part(newID, lua_tointeger(l, 2), lua_tointeger(l, 3), lua_tointeger(l, 4)));
	return 1;
}

int LuaScriptInterface::simulation_partKill(lua_State * l)
{
	if(lua_gettop(l)==2)
		luacon_sim->delete_part(lua_tointeger(l, 1), lua_tointeger(l, 2), 0);
	else
		luacon_sim->kill_part(lua_tointeger(l, 1));
	return 0;
}


//// Begin Renderer API

void LuaScriptInterface::initRendererAPI()
{
	//Methods
	struct luaL_reg rendererAPIMethods [] = {
		{"renderModes", renderer_renderModes},
		{"displayModes", renderer_displayModes},
		{"colourMode", renderer_colourMode},
		{"colorMode", renderer_colourMode}, //Duplicate of above to make americans happy
		{NULL, NULL}
	};
	luaL_register(l, "renderer", rendererAPIMethods);

	//Ren shortcut
	lua_getglobal(l, "renderer");
	lua_setglobal(l, "ren");

	int rendererAPI = lua_gettop(l);

	//Static values
	//Particle pixel modes/fire mode/effects
	lua_pushinteger(l, PMODE); lua_setfield(l, rendererAPI, "PMODE");
	lua_pushinteger(l, PMODE_NONE); lua_setfield(l, rendererAPI, "PMODE_NONE");
	lua_pushinteger(l, PMODE_FLAT); lua_setfield(l, rendererAPI, "PMODE_FLAT");
	lua_pushinteger(l, PMODE_BLOB); lua_setfield(l, rendererAPI, "PMODE_BLOB");
	lua_pushinteger(l, PMODE_BLUR); lua_setfield(l, rendererAPI, "PMODE_BLUR");
	lua_pushinteger(l, PMODE_GLOW); lua_setfield(l, rendererAPI, "PMODE_GLOW");
	lua_pushinteger(l, PMODE_SPARK); lua_setfield(l, rendererAPI, "PMODE_SPARK");
	lua_pushinteger(l, PMODE_FLARE); lua_setfield(l, rendererAPI, "PMODE_FLARE");
	lua_pushinteger(l, PMODE_LFLARE); lua_setfield(l, rendererAPI, "PMODE_LFLARE");
	lua_pushinteger(l, PMODE_ADD); lua_setfield(l, rendererAPI, "PMODE_ADD");
	lua_pushinteger(l, PMODE_BLEND); lua_setfield(l, rendererAPI, "PMODE_BLEND");
	lua_pushinteger(l, PSPEC_STICKMAN); lua_setfield(l, rendererAPI, "PSPEC_STICKMAN");
	lua_pushinteger(l, OPTIONS); lua_setfield(l, rendererAPI, "OPTIONS");
	lua_pushinteger(l, NO_DECO); lua_setfield(l, rendererAPI, "NO_DECO");
	lua_pushinteger(l, DECO_FIRE); lua_setfield(l, rendererAPI, "DECO_FIRE");
	lua_pushinteger(l, FIREMODE); lua_setfield(l, rendererAPI, "FIREMODE");
	lua_pushinteger(l, FIRE_ADD); lua_setfield(l, rendererAPI, "FIRE_ADD");
	lua_pushinteger(l, FIRE_BLEND); lua_setfield(l, rendererAPI, "FIRE_BLEND");
	lua_pushinteger(l, EFFECT); lua_setfield(l, rendererAPI, "EFFECT");
	lua_pushinteger(l, EFFECT_GRAVIN); lua_setfield(l, rendererAPI, "EFFECT_GRAVIN");
	lua_pushinteger(l, EFFECT_GRAVOUT); lua_setfield(l, rendererAPI, "EFFECT_GRAVOUT");
	lua_pushinteger(l, EFFECT_LINES); lua_setfield(l, rendererAPI, "EFFECT_LINES");
	lua_pushinteger(l, EFFECT_DBGLINES); lua_setfield(l, rendererAPI, "EFFECT_DBGLINES");

	//Display/Render/Colour modes
	lua_pushinteger(l, RENDER_EFFE); lua_setfield(l, rendererAPI, "RENDER_EFFE");
	lua_pushinteger(l, RENDER_FIRE); lua_setfield(l, rendererAPI, "RENDER_FIRE");
	lua_pushinteger(l, RENDER_GLOW); lua_setfield(l, rendererAPI, "RENDER_GLOW");
	lua_pushinteger(l, RENDER_BLUR); lua_setfield(l, rendererAPI, "RENDER_BLUR");
	lua_pushinteger(l, RENDER_BLOB); lua_setfield(l, rendererAPI, "RENDER_BLOB");
	lua_pushinteger(l, RENDER_BASC); lua_setfield(l, rendererAPI, "RENDER_BASC");
	lua_pushinteger(l, RENDER_NONE); lua_setfield(l, rendererAPI, "RENDER_NONE");
	lua_pushinteger(l, COLOUR_HEAT); lua_setfield(l, rendererAPI, "COLOUR_HEAT");
	lua_pushinteger(l, COLOUR_LIFE); lua_setfield(l, rendererAPI, "COLOUR_LIFE");
	lua_pushinteger(l, COLOUR_GRAD); lua_setfield(l, rendererAPI, "COLOUR_GRAD");
	lua_pushinteger(l, COLOUR_BASC); lua_setfield(l, rendererAPI, "COLOUR_BASC");
	lua_pushinteger(l, COLOUR_DEFAULT); lua_setfield(l, rendererAPI, "COLOUR_DEFAULT");
	lua_pushinteger(l, DISPLAY_AIRC); lua_setfield(l, rendererAPI, "DISPLAY_AIRC");
	lua_pushinteger(l, DISPLAY_AIRP); lua_setfield(l, rendererAPI, "DISPLAY_AIRP");
	lua_pushinteger(l, DISPLAY_AIRV); lua_setfield(l, rendererAPI, "DISPLAY_AIRV");
	lua_pushinteger(l, DISPLAY_AIRH); lua_setfield(l, rendererAPI, "DISPLAY_AIRH");
	lua_pushinteger(l, DISPLAY_AIR); lua_setfield(l, rendererAPI, "DISPLAY_AIR");
	lua_pushinteger(l, DISPLAY_WARP); lua_setfield(l, rendererAPI, "DISPLAY_WARP");
	lua_pushinteger(l, DISPLAY_PERS); lua_setfield(l, rendererAPI, "DISPLAY_PERS");
	lua_pushinteger(l, DISPLAY_EFFE); lua_setfield(l, rendererAPI, "DISPLAY_EFFE");
}

//get/set render modes list
int LuaScriptInterface::renderer_renderModes(lua_State * l)
{
	int args = lua_gettop(l);
	if(args)
	{
		int size = 0;
		luaL_checktype(l, 1, LUA_TTABLE);
		size = luaL_getn(l, 1);
		
		std::vector<unsigned int> renderModes;
		for(int i = 1; i <= size; i++)
		{
			lua_rawgeti(l, 1, i);
			renderModes.push_back(lua_tointeger(l, -1));
			lua_pop(l, 1);
		}
		luacon_ren->SetRenderMode(renderModes);
		return 0;
	}
	else
	{
		lua_newtable(l);
		std::vector<unsigned int> renderModes = luacon_ren->GetRenderMode();
		int i = 1;
		for(std::vector<unsigned int>::iterator iter = renderModes.begin(), end = renderModes.end(); iter != end; ++iter)
		{
			lua_pushinteger(l, *iter);
			lua_rawseti(l, -2, i++);
		}
		return 1;
	}
}

int LuaScriptInterface::renderer_displayModes(lua_State * l)
{
	int args = lua_gettop(l);
	if(args)
	{
		int size = 0;
		luaL_checktype(l, 1, LUA_TTABLE);
		size = luaL_getn(l, 1);
		
		std::vector<unsigned int> displayModes;
		for(int i = 1; i <= size; i++)
		{
			lua_rawgeti(l, 1, i);
			displayModes.push_back(lua_tointeger(l, -1));
			lua_pop(l, 1);
		}
		luacon_ren->SetDisplayMode(displayModes);
		return 0;
	}
	else
	{
		lua_newtable(l);
		std::vector<unsigned int> displayModes = luacon_ren->GetDisplayMode();
		int i = 1;
		for(std::vector<unsigned int>::iterator iter = displayModes.begin(), end = displayModes.end(); iter != end; ++iter)
		{
			lua_pushinteger(l, *iter);
			lua_rawseti(l, -2, i++);
		}
		return 1;
	}
}

int LuaScriptInterface::renderer_colourMode(lua_State * l)
{
	int args = lua_gettop(l);
	if(args)
	{
		luaL_checktype(l, 1, LUA_TNUMBER);
		luacon_ren->SetColourMode(lua_tointeger(l, 1));
		lua_pop(l, 1);
		return 0;
	}
	else
	{
		lua_pushinteger(l, luacon_ren->GetColourMode());
		return 1;
	}
	return luaL_error(l, "Not implemented");
}

void LuaScriptInterface::initElementsAPI()
{
	//Methods
	struct luaL_reg elementsAPIMethods [] = {
		{"allocate", elements_allocate},
		{"element", elements_element},
		{"property", elements_property},
		{"free", elements_free},
		{"loadDefault", elements_loadDefault},
		{NULL, NULL}
	};
	luaL_register(l, "elements", elementsAPIMethods);

	//elem shortcut
	lua_getglobal(l, "elements");
	lua_setglobal(l, "elem");

	int elementsAPI = lua_gettop(l);

	//Static values
	//Element types/properties/states
	lua_pushinteger(l, TYPE_PART); 			lua_setfield(l, elementsAPI, "TYPE_PART");
	lua_pushinteger(l, TYPE_LIQUID); 		lua_setfield(l, elementsAPI, "TYPE_LIQUID");
	lua_pushinteger(l, TYPE_SOLID); 		lua_setfield(l, elementsAPI, "TYPE_SOLID");
	lua_pushinteger(l, TYPE_GAS); 			lua_setfield(l, elementsAPI, "TYPE_GAS");
	lua_pushinteger(l, TYPE_ENERGY); 		lua_setfield(l, elementsAPI, "TYPE_ENERGY");
	lua_pushinteger(l, PROP_CONDUCTS); 		lua_setfield(l, elementsAPI, "PROP_CONDUCTS");
	lua_pushinteger(l, PROP_BLACK); 		lua_setfield(l, elementsAPI, "PROP_BLACK");
	lua_pushinteger(l, PROP_NEUTPENETRATE); lua_setfield(l, elementsAPI, "PROP_NEUTPENETRATE");
	lua_pushinteger(l, PROP_NEUTABSORB); 	lua_setfield(l, elementsAPI, "PROP_NEUTABSORB");
	lua_pushinteger(l, PROP_NEUTPASS); 		lua_setfield(l, elementsAPI, "PROP_NEUTPASS");
	lua_pushinteger(l, PROP_DEADLY); 		lua_setfield(l, elementsAPI, "PROP_DEADLY");
	lua_pushinteger(l, PROP_HOT_GLOW); 		lua_setfield(l, elementsAPI, "PROP_HOT_GLOW");
	lua_pushinteger(l, PROP_LIFE); 			lua_setfield(l, elementsAPI, "PROP_LIFE");
	lua_pushinteger(l, PROP_RADIOACTIVE); 	lua_setfield(l, elementsAPI, "PROP_RADIOACTIVE");
	lua_pushinteger(l, PROP_LIFE_DEC); 		lua_setfield(l, elementsAPI, "PROP_LIFE_DEC");
	lua_pushinteger(l, PROP_LIFE_KILL); 	lua_setfield(l, elementsAPI, "PROP_LIFE_KILL");
	lua_pushinteger(l, PROP_LIFE_KILL_DEC); lua_setfield(l, elementsAPI, "PROP_LIFE_KILL_DEC");
	lua_pushinteger(l, PROP_SPARKSETTLE); 	lua_setfield(l, elementsAPI, "PROP_SPARKSETTLE");
	lua_pushinteger(l, PROP_NOAMBHEAT); 	lua_setfield(l, elementsAPI, "PROP_NOAMBHEAT");
	lua_pushinteger(l, FLAG_STAGNANT); 		lua_setfield(l, elementsAPI, "FLAG_STAGNANT");
	lua_pushinteger(l, FLAG_SKIPMOVE); 		lua_setfield(l, elementsAPI, "FLAG_SKIPMOVE");
	lua_pushinteger(l, FLAG_MOVABLE); 		lua_setfield(l, elementsAPI, "FLAG_MOVABLE");
	lua_pushinteger(l, ST_NONE); 			lua_setfield(l, elementsAPI, "ST_NONE");
	lua_pushinteger(l, ST_SOLID); 			lua_setfield(l, elementsAPI, "ST_SOLID");
	lua_pushinteger(l, ST_LIQUID); 			lua_setfield(l, elementsAPI, "ST_LIQUID");
	lua_pushinteger(l, ST_GAS); 			lua_setfield(l, elementsAPI, "ST_GAS");

	lua_pushinteger(l, SC_WALL);		lua_setfield(l, elementsAPI, "SC_WALL");
	lua_pushinteger(l, SC_ELEC);		lua_setfield(l, elementsAPI, "SC_ELEC");
	lua_pushinteger(l, SC_POWERED);		lua_setfield(l, elementsAPI, "SC_POWERED");
	lua_pushinteger(l, SC_FORCE);		lua_setfield(l, elementsAPI, "SC_FORCE");
	lua_pushinteger(l, SC_EXPLOSIVE);	lua_setfield(l, elementsAPI, "SC_EXPLOSIVE");
	lua_pushinteger(l, SC_GAS);			lua_setfield(l, elementsAPI, "SC_GAS");
	lua_pushinteger(l, SC_LIQUID);		lua_setfield(l, elementsAPI, "SC_LIQUID");
	lua_pushinteger(l, SC_POWDERS);		lua_setfield(l, elementsAPI, "SC_POWDERS");
	lua_pushinteger(l, SC_SOLIDS);		lua_setfield(l, elementsAPI, "SC_SOLIDS");
	lua_pushinteger(l, SC_NUCLEAR);		lua_setfield(l, elementsAPI, "SC_NUCLEAR");
	lua_pushinteger(l, SC_SPECIAL);		lua_setfield(l, elementsAPI, "SC_SPECIAL");
	lua_pushinteger(l, SC_LIFE);		lua_setfield(l, elementsAPI, "SC_LIFE");
	lua_pushinteger(l, SC_TOOL);		lua_setfield(l, elementsAPI, "SC_TOOL");
	lua_pushinteger(l, SC_DECO);		lua_setfield(l, elementsAPI, "SC_DECO");

	//Element identifiers
	for(int i = 0; i < PT_NUM; i++)
	{
		if(luacon_sim->elements[i].Enabled)
		{
			lua_pushinteger(l, i);
			lua_setfield(l, elementsAPI, luacon_sim->elements[i].Identifier);
		}
	}
}

int LuaScriptInterface::elements_loadDefault(lua_State * l)
{
	int args = lua_gettop(l);
	if(args)
	{
		luaL_checktype(l, 1, LUA_TNUMBER);
		int id = lua_tointeger(l, 1);
		if(id < 0 || id >= PT_NUM)
			return luaL_error(l, "Invalid element");

		lua_getglobal(l, "elements");
		lua_pushnil(l);
		lua_setfield(l, -2, luacon_sim->elements[id].Identifier);

		std::vector<Element> elementList = GetElements();
		if(id < elementList.size())
			luacon_sim->elements[id] = elementList[id];
		else
			luacon_sim->elements[id] = Element();

		lua_pushinteger(l, id);
		lua_setfield(l, -2, luacon_sim->elements[id].Identifier);
		lua_pop(l, 1);
	}
	else
	{
		std::vector<Element> elementList = GetElements();
		for(int i = 0; i < PT_NUM; i++)
		{
			if(i < elementList.size())
				luacon_sim->elements[i] = elementList[i];
			else
				luacon_sim->elements[i] = Element();
		}
		lua_pushnil(l);
		lua_setglobal(l, "elements");
		luacon_ci->initElementsAPI();
	}

	luacon_model->BuildMenus();
	luacon_sim->init_can_move();
	std::fill(luacon_ren->graphicscache, luacon_ren->graphicscache+PT_NUM, gcache_item());

}

int LuaScriptInterface::elements_allocate(lua_State * l)
{
	std::string group, id, identifier;
	luaL_checktype(l, 1, LUA_TSTRING);
	luaL_checktype(l, 2, LUA_TSTRING);
	group = std::string(lua_tostring(l, 1));
	std::transform(group.begin(), group.end(), group.begin(), ::toupper);
	id = std::string(lua_tostring(l, 2));
	std::transform(id.begin(), id.end(), id.begin(), ::toupper);

	if(group == "DEFAULT")
		return luaL_error(l, "You cannot create elements in the 'default' group.");

	identifier = group + "_PT_" + id;

	for(int i = 0; i < PT_NUM; i++)
	{
		if(luacon_sim->elements[i].Enabled && std::string(luacon_sim->elements[i].Identifier) == identifier)
			return luaL_error(l, "Element identifier already in use");
	}

	int newID = -1;
	for(int i = PT_NUM-1; i >= 0; i--)
	{
		if(!luacon_sim->elements[i].Enabled)
		{
			newID = i;
			luacon_sim->elements[i] = Element();
			luacon_sim->elements[i].Enabled = true;
			luacon_sim->elements[i].Identifier = strdup(identifier.c_str());
			break;
		}
	}

	if(newID != -1)
	{	
		lua_getglobal(l, "elements");
		lua_pushinteger(l, newID);
		lua_setfield(l, -2, identifier.c_str());
		lua_pop(l, 1);
	}

	lua_pushinteger(l, newID);
	return 1;
}

int LuaScriptInterface::elements_element(lua_State * l)
{
	int args = lua_gettop(l);
	int id;
	luaL_checktype(l, 1, LUA_TNUMBER);
	id = lua_tointeger(l, 1);

	if(id < 0 || id >= PT_NUM || !luacon_sim->elements[id].Enabled)
		return luaL_error(l, "Invalid element");

	if(args > 1)
	{
		luaL_checktype(l, 2, LUA_TTABLE);
		std::vector<StructProperty> properties = Element::GetProperties();
		//Write values from native data to a table
		for(std::vector<StructProperty>::iterator iter = properties.begin(), end = properties.end(); iter != end; ++iter)
		{
			lua_getfield(l, -1, (*iter).Name.c_str());
			if(lua_type(l, -1) != LUA_TNIL)
			{
				intptr_t offset = (*iter).Offset;
				switch((*iter).Type)
				{
					case StructProperty::ParticleType:
					case StructProperty::Integer:
						*((int*)(((unsigned char*)&luacon_sim->elements[id])+offset)) = lua_tointeger(l, -1);
						break;
					case StructProperty::UInteger:
						*((unsigned int*)(((unsigned char*)&luacon_sim->elements[id])+offset)) = lua_tointeger(l, -1);
						break;
					case StructProperty::Float:
						*((float*)(((unsigned char*)&luacon_sim->elements[id])+offset)) = lua_tonumber(l, -1);
						break;
					case StructProperty::Char:
						*((char*)(((unsigned char*)&luacon_sim->elements[id])+offset)) = lua_tointeger(l, -1);
						break;
					case StructProperty::UChar:
						*((unsigned char*)(((unsigned char*)&luacon_sim->elements[id])+offset)) = lua_tointeger(l, -1);
						break;
					case StructProperty::String:
						*((char**)(((unsigned char*)&luacon_sim->elements[id])+offset)) = strdup(lua_tostring(l, -1));
						break;
					case StructProperty::Colour:
#if PIXELSIZE == 4
						*((unsigned int*)(((unsigned char*)&luacon_sim->elements[id])+offset)) = lua_tointeger(l, -1);
#else
						*((unsigned short*)(((unsigned char*)&luacon_sim->elements[id])+offset)) = lua_tointeger(l, -1);
#endif
						break;
				}
				lua_pop(l, 1);
			}
		}

		lua_getfield(l, -1, "Update");
		if(lua_type(l, -1) == LUA_TFUNCTION)
		{
			lua_el_func[id] = luaL_ref(l, LUA_REGISTRYINDEX);
			luacon_sim->elements[id].Update = &luacon_elementReplacement;
		}
		else if(lua_type(l, -1) == LUA_TBOOLEAN && !lua_toboolean(l, -1))
		{
			lua_el_func[id] = 0;
			luacon_sim->elements[id].Update = NULL;
		}
		else
			lua_pop(l, 1);

		lua_getfield(l, -1, "Graphics");
		if(lua_type(l, -1) == LUA_TFUNCTION)
		{
			lua_el_func[id] = luaL_ref(l, LUA_REGISTRYINDEX);
			luacon_sim->elements[id].Graphics = &luacon_graphicsReplacement;
		}
		else if(lua_type(l, -1) == LUA_TBOOLEAN && !lua_toboolean(l, -1))
		{
			lua_el_func[id] = 0;
			luacon_sim->elements[id].Graphics = NULL;
		}
		else
			lua_pop(l, 1);

		luacon_model->BuildMenus();
		luacon_sim->init_can_move();
		std::fill(luacon_ren->graphicscache, luacon_ren->graphicscache+PT_NUM, gcache_item());

		lua_pop(l, 1);
		return 0;
	}
	else
	{
		std::vector<StructProperty> properties = Element::GetProperties();
		//Write values from native data to a table
		lua_newtable(l);
		for(std::vector<StructProperty>::iterator iter = properties.begin(), end = properties.end(); iter != end; ++iter)
		{
			intptr_t offset = (*iter).Offset;
			switch((*iter).Type)
			{
				case StructProperty::ParticleType:
				case StructProperty::Integer:
					lua_pushinteger(l, *((int*)(((unsigned char*)&luacon_sim->elements[id])+offset)));
					break;
				case StructProperty::UInteger:
					lua_pushinteger(l, *((unsigned int*)(((unsigned char*)&luacon_sim->elements[id])+offset)));
					break;
				case StructProperty::Float:
					lua_pushnumber(l, *((float*)(((unsigned char*)&luacon_sim->elements[id])+offset)));
					break;
				case StructProperty::Char:
					lua_pushinteger(l, *((char*)(((unsigned char*)&luacon_sim->elements[id])+offset)));
					break;
				case StructProperty::UChar:
					lua_pushinteger(l, *((unsigned char*)(((unsigned char*)&luacon_sim->elements[id])+offset)));
					break;
				case StructProperty::String:
					lua_pushstring(l, *((char**)(((unsigned char*)&luacon_sim->elements[id])+offset)));
					break;
				case StructProperty::Colour:
#if PIXELSIZE == 4
					lua_pushinteger(l, *((unsigned int*)(((unsigned char*)&luacon_sim->elements[id])+offset)));
#else
					lua_pushinteger(l, *((unsigned short*)(((unsigned char*)&luacon_sim->elements[id])+offset)));
#endif
					break;
				default:
					lua_pushnil(l);
			}
			lua_setfield(l, -2, (*iter).Name.c_str());
		}
		return 1;
	}
}

int LuaScriptInterface::elements_property(lua_State * l)
{
	int args = lua_gettop(l);
	int id;
	std::string propertyName;
	luaL_checktype(l, 1, LUA_TNUMBER);
	id = lua_tointeger(l, 1);
	luaL_checktype(l, 2, LUA_TSTRING);
	propertyName = std::string(lua_tostring(l, 2));

	if(id < 0 || id >= PT_NUM || !luacon_sim->elements[id].Enabled)
		return luaL_error(l, "Invalid element");



	if(args > 2)
	{
		StructProperty property;
		bool propertyFound = false;
		std::vector<StructProperty> properties = Element::GetProperties();

		for(std::vector<StructProperty>::iterator iter = properties.begin(), end = properties.end(); iter != end; ++iter)
		{
			if((*iter).Name == propertyName)
			{
				property = *iter;
				propertyFound = true;
				break;
			}
		}

		if(propertyFound)
		{
			if(lua_type(l, 3) != LUA_TNIL)
			{
				intptr_t offset = property.Offset;
				switch(property.Type)
				{
					case StructProperty::ParticleType:
					case StructProperty::Integer:
						*((int*)(((unsigned char*)&luacon_sim->elements[id])+offset)) = lua_tointeger(l, 3);
						break;
					case StructProperty::UInteger:
						*((unsigned int*)(((unsigned char*)&luacon_sim->elements[id])+offset)) = lua_tointeger(l, 3);
						break;
					case StructProperty::Float:
						*((float*)(((unsigned char*)&luacon_sim->elements[id])+offset)) = lua_tonumber(l, 3);
						break;
					case StructProperty::Char:
						*((char*)(((unsigned char*)&luacon_sim->elements[id])+offset)) = lua_tointeger(l, 3);
						break;
					case StructProperty::UChar:
						*((unsigned char*)(((unsigned char*)&luacon_sim->elements[id])+offset)) = lua_tointeger(l, 3);
						break;
					case StructProperty::String:
						*((char**)(((unsigned char*)&luacon_sim->elements[id])+offset)) = strdup(lua_tostring(l, 3));
						break;
					case StructProperty::Colour:
	#if PIXELSIZE == 4
						*((unsigned int*)(((unsigned char*)&luacon_sim->elements[id])+offset)) = lua_tointeger(l, 3);
	#else
						*((unsigned short*)(((unsigned char*)&luacon_sim->elements[id])+offset)) = lua_tointeger(l, 3);
	#endif
						break;
				}
			}

			luacon_model->BuildMenus();
			luacon_sim->init_can_move();
			std::fill(luacon_ren->graphicscache, luacon_ren->graphicscache+PT_NUM, gcache_item());

			return 0;
		}
		else if(propertyName == "Update")
		{
			if(lua_type(l, 3) == LUA_TFUNCTION)
			{
				lua_pushvalue(l, 3);
				lua_el_func[id] = luaL_ref(l, LUA_REGISTRYINDEX);
				luacon_sim->elements[id].Update = &luacon_elementReplacement;
			}
			else if(lua_type(l, 3) == LUA_TBOOLEAN && !lua_toboolean(l, 3))
			{
				lua_el_func[id] = 0;
				luacon_sim->elements[id].Update = NULL;
			}
		}
		else if(propertyName == "Graphics")
		{
			if(lua_type(l, 3) == LUA_TFUNCTION)
			{
				lua_pushvalue(l, 3);
				lua_el_func[id] = luaL_ref(l, LUA_REGISTRYINDEX);
				luacon_sim->elements[id].Graphics = &luacon_graphicsReplacement;
			}
			else if(lua_type(l, 3) == LUA_TBOOLEAN && !lua_toboolean(l, -1))
			{
				lua_el_func[id] = 0;
				luacon_sim->elements[id].Graphics = NULL;
			}
			std::fill(luacon_ren->graphicscache, luacon_ren->graphicscache+PT_NUM, gcache_item());
		}
		else
			return luaL_error(l, "Invalid element property");
	}
	else
	{
		StructProperty property;
		bool propertyFound = false;
		std::vector<StructProperty> properties = Element::GetProperties();

		for(std::vector<StructProperty>::iterator iter = properties.begin(), end = properties.end(); iter != end; ++iter)
		{
			if((*iter).Name == propertyName)
			{
				property = *iter;
				propertyFound = true;
				break;
			}
		}

		if(propertyFound)
		{
			intptr_t offset = property.Offset;
			switch(property.Type)
			{
				case StructProperty::ParticleType:
				case StructProperty::Integer:
					lua_pushinteger(l, *((int*)(((unsigned char*)&luacon_sim->elements[id])+offset)));
					break;
				case StructProperty::UInteger:
					lua_pushinteger(l, *((unsigned int*)(((unsigned char*)&luacon_sim->elements[id])+offset)));
					break;
				case StructProperty::Float:
					lua_pushnumber(l, *((float*)(((unsigned char*)&luacon_sim->elements[id])+offset)));
					break;
				case StructProperty::Char:
					lua_pushinteger(l, *((char*)(((unsigned char*)&luacon_sim->elements[id])+offset)));
					break;
				case StructProperty::UChar:
					lua_pushinteger(l, *((unsigned char*)(((unsigned char*)&luacon_sim->elements[id])+offset)));
					break;
				case StructProperty::String:
					lua_pushstring(l, *((char**)(((unsigned char*)&luacon_sim->elements[id])+offset)));
					break;
				case StructProperty::Colour:
#if PIXELSIZE == 4
					lua_pushinteger(l, *((unsigned int*)(((unsigned char*)&luacon_sim->elements[id])+offset)));
#else
					lua_pushinteger(l, *((unsigned short*)(((unsigned char*)&luacon_sim->elements[id])+offset)));
#endif
					break;
				default:
					lua_pushnil(l);
			}
			return 1;
		}
		else
			return luaL_error(l, "Invalid element property");
	}
}

int LuaScriptInterface::elements_free(lua_State * l)
{
	int id;
	luaL_checktype(l, 1, LUA_TNUMBER);
	id = lua_tointeger(l, 1);
	
	if(id < 0 || id >= PT_NUM || !luacon_sim->elements[id].Enabled)
		return luaL_error(l, "Invalid element");

	std::string identifier = luacon_sim->elements[id].Identifier;
	if(identifier.length()>7 && identifier.substr(0, 7) == "DEFAULT")
		return luaL_error(l, "Cannot free default elements");

	luacon_sim->elements[id].Enabled = false;

	lua_getglobal(l, "elements");
	lua_pushnil(l);
	lua_setfield(l, -2, identifier.c_str());
	lua_pop(l, 1);

	return 0;
}


bool LuaScriptInterface::OnBrushChanged(int brushType, int rx, int ry)
{
	luacon_brushx = rx;
	luacon_brushy = ry;
	return true;
}

bool LuaScriptInterface::OnMouseMove(int x, int y, int dx, int dy)
{
	luacon_mousex = x;
	luacon_mousey = y;
	return true;
}

bool LuaScriptInterface::OnMouseDown(int x, int y, unsigned button)
{
	luacon_mousedown = true;
	luacon_mousebutton = button;
	return luacon_mouseevent(x, y, button, LUACON_MDOWN, 0);
}

bool LuaScriptInterface::OnMouseUp(int x, int y, unsigned button)
{
	luacon_mousedown = false;
	return luacon_mouseevent(x, y, button, LUACON_MUP, 0);
}

bool LuaScriptInterface::OnMouseWheel(int x, int y, int d)
{
	return luacon_mouseevent(x, y, luacon_mousedown?luacon_mousebutton:0, 0, d);
}

bool LuaScriptInterface::OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	int modifiers = 0;
	if(shift)
		modifiers |= 0x001;
	if(ctrl)
		modifiers |= 0x040;
	if(alt)
		modifiers |= 0x100;
	return luacon_keyevent(key, modifiers, LUACON_KDOWN);
}

bool LuaScriptInterface::OnKeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	int modifiers = 0;
	if(shift)
		modifiers |= 0x001;
	if(ctrl)
		modifiers |= 0x040;
	if(alt)
		modifiers |= 0x100;
	return luacon_keyevent(key, modifiers, LUACON_KUP);
}

void LuaScriptInterface::OnTick()
{
	ui::Engine::Ref().LastTick(clock());
	if(luacon_mousedown)
		luacon_mouseevent(luacon_mousex, luacon_mousey, luacon_mousebutton, LUACON_MPRESS, 0);
	luacon_step(luacon_mousex, luacon_mousey, luacon_selectedl, luacon_selectedr, luacon_brushx, luacon_brushy);
}

int LuaScriptInterface::Command(std::string command)
{
	if(command[0] == '!')
	{
		lastError = "";
		int ret = legacy->Command(command.substr(1));
		lastError = legacy->GetLastError();
		return ret;
	}
	else
	{
		int ret;
		lastError = "";
		currentCommand = true;
		ui::Engine::Ref().LastTick(clock());
		if((ret = luaL_dostring(l, command.c_str())))
		{
			lastError = luacon_geterror();
			//Log(LogError, lastError);
		}
		currentCommand = false;
		return ret;
	}
}

std::string LuaScriptInterface::FormatCommand(std::string command)
{
	if(command[0] == '!')
	{
		return "!"+legacy->FormatCommand(command.substr(1));
	}
	else
		return command;
}

LuaScriptInterface::~LuaScriptInterface() {
	delete legacy;
}

#ifndef FFI
int luacon_partread(lua_State* l){
	int format, offset, tempinteger;
	float tempfloat;
	int i;
	char * key = mystrdup((char*)luaL_optstring(l, 2, ""));
	offset = luacon_particle_getproperty(key, &format);

	i = cIndex;

	if(i < 0 || i >= NPART || offset==-1)
	{
		if(i < 0 || i >= NPART) {
			free(key);
			return luaL_error(l, "Out of range");
		} else if(strcmp(key, "id")==0) {
			free(key);
			lua_pushnumber(l, i);
			return 1;
		} else {
			free(key);
			return luaL_error(l, "Invalid property");
		}
	}
	free(key);
	switch(format)
	{
	case 0:
		tempinteger = *((int*)(((unsigned char*)&luacon_sim->parts[i])+offset));
		lua_pushnumber(l, tempinteger);
		break;
	case 1:
		tempfloat = *((float*)(((unsigned char*)&luacon_sim->parts[i])+offset));
		lua_pushnumber(l, tempfloat);
		break;
	}
	return 1;
}
int luacon_partwrite(lua_State* l){
	int format, offset;
	int i;
	char * key = mystrdup((char*)luaL_optstring(l, 2, ""));
	offset = luacon_particle_getproperty(key, &format);

	i = cIndex;

	if(i < 0 || i >= NPART || offset==-1)
	{
		if(i < 0 || i >= NPART) {
			free(key);
			return luaL_error(l, "array index out of bounds");
		} else {
			free(key);
			return luaL_error(l, "Invalid property");
		}
	}
	free(key);
	switch(format)
	{
	case 0:
		*((int*)(((unsigned char*)&luacon_sim->parts[i])+offset)) = luaL_optinteger(l, 3, 0);
		break;
	case 1:
		*((float*)(((unsigned char*)&luacon_sim->parts[i])+offset)) = luaL_optnumber(l, 3, 0);
		break;
	}
	return 1;
}
int luacon_partsread(lua_State* l){
	int format, offset;
	char * tempstring;
	int tempinteger;
	float tempfloat;
	int i, currentPart, currentPartMeta;

	i = luaL_optinteger(l, 2, 0);

	if(i<0 || i>=NPART)
	{
		return luaL_error(l, "array index out of bounds");
	}

	lua_rawgeti(l, LUA_REGISTRYINDEX, tptPart);
	cIndex = i;
	return 1;
}
int luacon_partswrite(lua_State* l){
	return luaL_error(l, "table readonly");
}
#endif
int luacon_particle_getproperty(char * key, int * format)
{
	int offset;
	if (strcmp(key, "type")==0){
		offset = offsetof(Particle, type);
		*format = 0;
	} else if (strcmp(key, "life")==0){
		offset = offsetof(Particle, life);
		*format = 0;
	} else if (strcmp(key, "ctype")==0){
		offset = offsetof(Particle, ctype);
		*format = 0;
	} else if (strcmp(key, "temp")==0){
		offset = offsetof(Particle, temp);
		*format = 1;
	} else if (strcmp(key, "tmp")==0){
		offset = offsetof(Particle, tmp);
		*format = 0;
	} else if (strcmp(key, "tmp2")==0){
		offset = offsetof(Particle, tmp2);
		*format = 0;
	} else if (strcmp(key, "vy")==0){
		offset = offsetof(Particle, vy);
		*format = 1;
	} else if (strcmp(key, "vx")==0){
		offset = offsetof(Particle, vx);
		*format = 1;
	} else if (strcmp(key, "x")==0){
		offset = offsetof(Particle, x);
		*format = 1;
	} else if (strcmp(key, "y")==0){
		offset = offsetof(Particle, y);
		*format = 1;
	} else if (strcmp(key, "dcolour")==0){
		offset = offsetof(Particle, dcolour);
		*format = 0;
	} else if (strcmp(key, "dcolor")==0){
		offset = offsetof(Particle, dcolour);
		*format = 0;
	} else {
		offset = -1;
	}
	return offset;
}
int luacon_transition_getproperty(char * key, int * format)
{
	int offset;
	if (strcmp(key, "presHighValue")==0){
		offset = offsetof(Element, HighPressure);
		*format = 1;
	} else if (strcmp(key, "presHighType")==0){
		offset = offsetof(Element, HighPressureTransition);
		*format = 0;
	} else if (strcmp(key, "presLowValue")==0){
		offset = offsetof(Element, LowPressure);
		*format = 1;
	} else if (strcmp(key, "presLowType")==0){
		offset = offsetof(Element, LowPressureTransition);
		*format = 0;
	} else if (strcmp(key, "tempHighValue")==0){
		offset = offsetof(Element, HighTemperature);
		*format = 1;
	} else if (strcmp(key, "tempHighType")==0){
		offset = offsetof(Element, HighTemperatureTransition);
		*format = 0;
	} else if (strcmp(key, "tempLowValue")==0){
		offset = offsetof(Element, LowTemperature);
		*format = 1;
	} else if (strcmp(key, "tempLowType")==0){
		offset = offsetof(Element, LowTemperatureTransition);
		*format = 0;
	} else {
		offset = -1;
	}
	return offset;
}
int luacon_transitionread(lua_State* l){
	int format, offset;
	int tempinteger;
	float tempfloat;
	int i;
	char * key = mystrdup((char*)luaL_optstring(l, 2, ""));
	offset = luacon_transition_getproperty(key, &format);
	free(key);

	//Get Raw Index value for element
	lua_pushstring(l, "value");
	lua_rawget(l, 1);

	i = lua_tointeger(l, lua_gettop(l));

	lua_pop(l, 1);

	if(i < 0 || i >= PT_NUM || offset==-1)
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
int luacon_transitionwrite(lua_State* l){
	int format, offset;
	int tempinteger;
	float tempfloat;
	int i;
	char * key = mystrdup((char*)luaL_optstring(l, 2, ""));
	offset = luacon_transition_getproperty(key, &format);
	free(key);

	//Get Raw Index value for element
	lua_pushstring(l, "value");
	lua_rawget(l, 1);

	i = lua_tointeger (l, lua_gettop(l));

	lua_pop(l, 1);

	if(i < 0 || i >= PT_NUM || offset==-1)
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
int luacon_element_getproperty(char * key, int * format, unsigned int * modified_stuff)
{
	int offset;
	if (strcmp(key, "name")==0){
		offset = offsetof(Element, Name);
		*format = 2;
		if(modified_stuff)
			*modified_stuff |= LUACON_EL_MODIFIED_MENUS;
	}
	else if (strcmp(key, "color")==0){
		offset = offsetof(Element, Colour);
		*format = 0;
		if (modified_stuff)
			*modified_stuff |= LUACON_EL_MODIFIED_GRAPHICS;
	}
	else if (strcmp(key, "colour")==0){
		offset = offsetof(Element, Colour);
		*format = 0;
		if (modified_stuff)
			*modified_stuff |= LUACON_EL_MODIFIED_GRAPHICS;
	}
	else if (strcmp(key, "advection")==0){
		offset = offsetof(Element, Advection);
		*format = 1;
	}
	else if (strcmp(key, "airdrag")==0){
		offset = offsetof(Element, AirDrag);
		*format = 1;
	}
	else if (strcmp(key, "airloss")==0){
		offset = offsetof(Element, AirLoss);
		*format = 1;
	}
	else if (strcmp(key, "loss")==0){
		offset = offsetof(Element, Loss);
		*format = 1;
	}
	else if (strcmp(key, "collision")==0){
		offset = offsetof(Element, Collision);
		*format = 1;
	}
	else if (strcmp(key, "gravity")==0){
		offset = offsetof(Element, Gravity);
		*format = 1;
	}
	else if (strcmp(key, "diffusion")==0){
		offset = offsetof(Element, Diffusion);
		*format = 1;
	}
	else if (strcmp(key, "hotair")==0){
		offset = offsetof(Element, HotAir);
		*format = 1;
	}
	else if (strcmp(key, "falldown")==0){
		offset = offsetof(Element, Falldown);
		*format = 0;
	}
	else if (strcmp(key, "flammable")==0){
		offset = offsetof(Element, Flammable);
		*format = 0;
	}
	else if (strcmp(key, "explosive")==0){
		offset = offsetof(Element, Explosive);
		*format = 0;
	}
	else if (strcmp(key, "meltable")==0){
		offset = offsetof(Element, Meltable);
		*format = 0;
	}
	else if (strcmp(key, "hardness")==0){
		offset = offsetof(Element, Hardness);
		*format = 0;
	}
	else if (strcmp(key, "menu")==0){
		offset = offsetof(Element, MenuVisible);
		*format = 0;
		if (modified_stuff)
			*modified_stuff |= LUACON_EL_MODIFIED_MENUS;
	}
	else if (strcmp(key, "enabled")==0){
		offset = offsetof(Element, Enabled);
		*format = 0;
	}
	else if (strcmp(key, "weight")==0){
		offset = offsetof(Element, Weight);
		*format = 0;
		if (modified_stuff)
			*modified_stuff |= LUACON_EL_MODIFIED_CANMOVE;
	}
	else if (strcmp(key, "menusection")==0){
		offset = offsetof(Element, MenuSection);
		*format = 0;
		if (modified_stuff)
			*modified_stuff |= LUACON_EL_MODIFIED_MENUS;
	}
	else if (strcmp(key, "heat")==0){
		offset = offsetof(Element, Temperature);
		*format = 1;
	}
	else if (strcmp(key, "hconduct")==0){
		offset = offsetof(Element, HeatConduct);
		*format = 3;
	}
	else if (strcmp(key, "state")==0){
		offset = offsetof(Element, State);
		*format = 3;
	}
	else if (strcmp(key, "properties")==0){
		offset = offsetof(Element, Properties);
		*format = 0;
		if (modified_stuff)
			*modified_stuff |= LUACON_EL_MODIFIED_GRAPHICS | LUACON_EL_MODIFIED_CANMOVE;
	}
	else if (strcmp(key, "description")==0){
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
int luacon_elementread(lua_State* l){
	int format, offset;
	char * tempstring;
	int tempinteger;
	float tempfloat;
	int i;
	char * key = mystrdup((char*)luaL_optstring(l, 2, ""));
	offset = luacon_element_getproperty(key, &format, NULL);
	free(key);

	//Get Raw Index value for element
	lua_pushstring(l, "id");
	lua_rawget(l, 1);

	i = lua_tointeger (l, lua_gettop(l));

	lua_pop(l, 1);

	if(i < 0 || i >= PT_NUM || offset==-1)
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
	}
	return 1;
}
int luacon_elementwrite(lua_State* l){
	int format, offset;
	char * tempstring;
	int tempinteger;
	float tempfloat;
	int i;
	unsigned int modified_stuff = 0;
	char * key = mystrdup((char*)luaL_optstring(l, 2, ""));
	offset = luacon_element_getproperty(key, &format, &modified_stuff);

	//Get Raw Index value for element
	lua_pushstring(l, "id");
	lua_rawget(l, 1);

	i = lua_tointeger (l, lua_gettop(l));

	lua_pop(l, 1);

	if(i < 0 || i >= PT_NUM || offset==-1)
	{
		free(key);
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
		if(strcmp(key, "name")==0)
		{
			int j = 0;
			//Convert to upper case
			for(j = 0; j < strlen(tempstring); j++)
				tempstring[j] = toupper(tempstring[j]);
			if(strlen(tempstring)>4)
			{
				free(tempstring);
				free(key);
				return luaL_error(l, "Name too long");
			}
			if(luacon_ci->GetParticleType(tempstring) == -1)
			{
				free(tempstring);
				free(key);
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
	free(key);
	return 0;
}
int luacon_keyevent(int key, int modifier, int event){
	int i = 0, kpcontinue = 1, callret;
	char tempkey[] = {key, 0};
	if(keypress_function_count){
		for(i = 0; i < keypress_function_count && kpcontinue; i++){
			lua_rawgeti(luacon_ci->l, LUA_REGISTRYINDEX, keypress_functions[i]);
			lua_pushstring(luacon_ci->l, tempkey);
			lua_pushinteger(luacon_ci->l, key);
			lua_pushinteger(luacon_ci->l, modifier);
			lua_pushinteger(luacon_ci->l, event);
			callret = lua_pcall(luacon_ci->l, 4, 1, 0);
			if (callret)
			{
				luacon_ci->Log(CommandInterface::LogError, luacon_geterror());
			}
			if(lua_isboolean(luacon_ci->l, -1)){
				kpcontinue = lua_toboolean(luacon_ci->l, -1);
			}
			lua_pop(luacon_ci->l, 1);
		}
	}
	return kpcontinue;
}
int luacon_mouseevent(int mx, int my, int mb, int event, int mouse_wheel){
	int i = 0, mpcontinue = 1, callret;
	if(mouseclick_function_count){
		for(i = 0; i < mouseclick_function_count && mpcontinue; i++){
			lua_rawgeti(luacon_ci->l, LUA_REGISTRYINDEX, mouseclick_functions[i]);
			lua_pushinteger(luacon_ci->l, mx);
			lua_pushinteger(luacon_ci->l, my);
			lua_pushinteger(luacon_ci->l, mb);
			lua_pushinteger(luacon_ci->l, event);
			lua_pushinteger(luacon_ci->l, mouse_wheel);
			callret = lua_pcall(luacon_ci->l, 5, 1, 0);
			if (callret)
			{
				luacon_ci->Log(CommandInterface::LogError, luacon_geterror());
			}
			if(lua_isboolean(luacon_ci->l, -1)){
				mpcontinue = lua_toboolean(luacon_ci->l, -1);
			}
			lua_pop(luacon_ci->l, 1);
		}
	}
	return mpcontinue;
}

int luacon_step(int mx, int my, int selectl, int selectr, int bsx, int bsy){
	int tempret = 0, tempb, i, callret;
	lua_pushinteger(luacon_ci->l, bsy);
	lua_pushinteger(luacon_ci->l, bsx);
	lua_pushinteger(luacon_ci->l, selectr);
	lua_pushinteger(luacon_ci->l, selectl);
	lua_pushinteger(luacon_ci->l, my);
	lua_pushinteger(luacon_ci->l, mx);
	lua_setfield(luacon_ci->l, tptProperties, "mousex");
	lua_setfield(luacon_ci->l, tptProperties, "mousey");
	lua_setfield(luacon_ci->l, tptProperties, "selectedl");
	lua_setfield(luacon_ci->l, tptProperties, "selectedr");
	lua_setfield(luacon_ci->l, tptProperties, "brushx");
	lua_setfield(luacon_ci->l, tptProperties, "brushy");
	for(i = 0; i<6; i++){
		if(step_functions[i]){
			lua_rawgeti(luacon_ci->l, LUA_REGISTRYINDEX, step_functions[i]);
			callret = lua_pcall(luacon_ci->l, 0, 0, 0);
			if (callret)
			{
				if (!strcmp(luacon_geterror(),"Error: Script not responding"))
				{
					ui::Engine::Ref().LastTick(clock());
					lua_pushcfunction(luacon_ci->l, &luatpt_unregister_step);
					lua_rawgeti(luacon_ci->l, LUA_REGISTRYINDEX, step_functions[i]);
					lua_pcall(luacon_ci->l, 1, 0, 0);
				}
				luacon_ci->Log(CommandInterface::LogError, luacon_geterror());
			}
		}
	}
	return 0;
}


int luacon_eval(char *command){
	ui::Engine::Ref().LastTick(clock());
	return luaL_dostring (luacon_ci->l, command);
}

void luacon_hook(lua_State * l, lua_Debug * ar)
{
	if(ar->event == LUA_HOOKCOUNT && clock()-ui::Engine::Ref().LastTick() > CLOCKS_PER_SEC*3)
	{
		if(ConfirmPrompt::Blocking("Script not responding", "The Lua script may have stopped responding. There might be an infinite loop. Press \"Stop\" to stop it", "Stop"))
			luaL_error(l, "Error: Script not responding");
		ui::Engine::Ref().LastTick(clock());
	}
}

char *luacon_geterror(){
	char *error = (char*)lua_tostring(luacon_ci->l, -1);
	if(error==NULL || !error[0]){
		error = "failed to execute";
	}
	return error;
}
/*void luacon_close(){
	lua_close(l);
}*/

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
	if(lua_isnumber(l, 1))
	{
		t = luaL_optint(l, 1, 1);
		if (t<0 || t>=PT_NUM)
			return luaL_error(l, "Unrecognised element number '%d'", t);
		lua_pushstring(l, luacon_sim->elements[t].Name);
	}
	else
	{
		char* name = (char*)luaL_optstring(l, 1, "dust");
		if ((t = luacon_ci->GetParticleType(name))==-1)
			return luaL_error(l, "Unrecognised element '%s'", name);
		lua_pushinteger(l, t);
	}
	return 1;
}

int luacon_elementReplacement(UPDATE_FUNC_ARGS)
{
	int retval = 0;
	if(lua_el_func[parts[i].type]){
		lua_rawgeti(luacon_ci->l, LUA_REGISTRYINDEX, lua_el_func[parts[i].type]);
		lua_pushinteger(luacon_ci->l, i);
		lua_pushinteger(luacon_ci->l, x);
		lua_pushinteger(luacon_ci->l, y);
		lua_pushinteger(luacon_ci->l, surround_space);
		lua_pushinteger(luacon_ci->l, nt);
		lua_pcall(luacon_ci->l, 5, 1, 0);
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
		int function;
		lua_pushvalue(l, 1);
		function = luaL_ref(l, LUA_REGISTRYINDEX);
		if(element > 0 && element < PT_NUM)
		{
			lua_el_func[element] = function;
			luacon_sim->elements[element].Update = &luacon_elementReplacement;
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

int luacon_graphicsReplacement(GRAPHICS_FUNC_ARGS)
{
	int cache = 0;
	lua_rawgeti(luacon_ci->l, LUA_REGISTRYINDEX, lua_gr_func[cpart->type]);
	lua_pushinteger(luacon_ci->l, 0);
	lua_pushinteger(luacon_ci->l, *colr);
	lua_pushinteger(luacon_ci->l, *colg);
	lua_pushinteger(luacon_ci->l, *colb);
	lua_pcall(luacon_ci->l, 4, 10, 0);

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
		if(element > 0 && element < PT_NUM)
		{
			lua_gr_func[element] = function;
			luacon_ren->graphicscache[element].isready = 0;
			luacon_sim->elements[element].Graphics = &luacon_graphicsReplacement;
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
    char *string;
	int textx, texty, textred, textgreen, textblue, textalpha;
	textx = luaL_optint(l, 1, 0);
	texty = luaL_optint(l, 2, 0);
	string = (char*)luaL_optstring(l, 3, "");
	textred = luaL_optint(l, 4, 255);
	textgreen = luaL_optint(l, 5, 255);
	textblue = luaL_optint(l, 6, 255);
	textalpha = luaL_optint(l, 7, 255);
	if (textx<0 || texty<0 || textx>=XRES+BARSIZE || texty>=YRES+MENUSIZE)
		return luaL_error(l, "Screen coordinates out of range (%d,%d)", textx, texty);
	if (textred<0) textred = 0;
	if (textred>255) textred = 255;
	if (textgreen<0) textgreen = 0;
	if (textgreen>255) textgreen = 255;
	if (textblue<0) textblue = 0;
	if (textblue>255) textblue = 255;
	if (textalpha<0) textalpha = 0;
	if (textalpha>255) textalpha = 255;
	if(luacon_g!=NULL){
		luacon_g->drawtext(textx, texty, string, textred, textgreen, textblue, textalpha);
		return 0;
	}
	return luaL_error(l, "Screen buffer does not exist");
}

int luatpt_create(lua_State* l)
{
	int x, y, retid, t = -1;
	char * name;
	x = abs(luaL_optint(l, 1, 0));
	y = abs(luaL_optint(l, 2, 0));
	if(x < XRES && y < YRES){
		if(lua_isnumber(l, 3)){
			t = luaL_optint(l, 3, 0);
			if (t<0 || t >= PT_NUM || !luacon_sim->elements[t].Enabled)
				return luaL_error(l, "Unrecognised element number '%d'", t);
		} else {
			name = (char*)luaL_optstring(l, 3, "dust");
			if ((t = luacon_ci->GetParticleType(std::string(name))) == -1)
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
	int pausestate;
	pausestate = luaL_optint(l, 1, 0);
	luacon_model->SetPaused(pausestate==0?0:1);
	return 0;
}

int luatpt_togglepause(lua_State* l)
{
	luacon_model->SetPaused(!luacon_model->GetPaused());
	//sys_pause=!sys_pause;
	return 0;
}

int luatpt_togglewater(lua_State* l)
{
	luacon_sim->water_equal_test=!luacon_sim->water_equal_test;
	return 0;
}

int luatpt_setconsole(lua_State* l)
{
	/*int consolestate;
	consolestate = luaL_optint(l, 1, 0);
	console_mode = (consolestate==0?0:1);
	return 0;*/
	//TODO IMPLEMENT
	return 0;
}

int luatpt_log(lua_State* l)
{
	int args = lua_gettop(l);
	for(int i = 1; i <= args; i++)
	{
		if((*luacon_currentCommand))
		{
			if(!(*luacon_lastError).length())
				(*luacon_lastError) = luaL_optstring(l, i, "");
			else
				(*luacon_lastError) += ", " + std::string(luaL_optstring(l, i, ""));
		}
		else
			luacon_ci->Log(CommandInterface::LogNotice, luaL_optstring(l, i, ""));
	}
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
	int i;
	for (i=0; i<NPART; i++)
	{
		if (luacon_sim->parts[i].type==PT_SPRK)
		{
			if (luacon_sim->parts[i].ctype >= 0 && luacon_sim->parts[i].ctype < PT_NUM)
			{
				luacon_sim->parts[i].type = luacon_sim->parts[i].ctype;
				luacon_sim->parts[i].life = 0;
			}
			else
				luacon_sim->kill_part(i);
		}
	}
	return 0;
}

int luatpt_set_property(lua_State* l)
{
	char *prop, *name;
	int r, i, x, y, w, h, t, nx, ny, partsel = 0, acount;
	float f;
	size_t offset;
	acount = lua_gettop(l);
	prop = (char*)luaL_optstring(l, 1, "");
	if(lua_isnumber(l, 3))
		i = abs(luaL_optint(l, 3, -1));
	else
		i = -1;
	if(lua_isnumber(l, 4))
		y = abs(luaL_optint(l, 4, -1));
	else
		y = -1;
	if(lua_isnumber(l, 5))
		w = abs(luaL_optint(l, 5, -1));
	else
		w = -1;
	if(lua_isnumber(l, 6))
		h = abs(luaL_optint(l, 6, -1));
	else
		h = -1;

	CommandInterface::FormatType format;
	offset = luacon_ci->GetPropertyOffset(prop, format);
	if(offset == -1)
		return luaL_error(l, "Invalid property '%s'", prop);
	if(acount>2){
		if(!lua_isnumber(l, acount) && lua_isstring(l, acount)){
			name = (char*)luaL_optstring(l, acount, "none");
			if((partsel = luacon_ci->GetParticleType(std::string(name)))==-1)
				return luaL_error(l, "Unrecognised element '%s'", name);
		}
	}
	if(lua_isnumber(l, 2)){
		if(format == CommandInterface::FormatFloat){
			f = luaL_optnumber(l, 2, 0);
		} else {
			t = luaL_optint(l, 2, 0);
		}
		//TODO Element ID check
		//if (format == 3 && (t<0 || t>=PT_NUM))
		//	return luaL_error(l, "Unrecognised element number '%d'", t);
	} else {
		name = (char*)luaL_optstring(l, 2, "dust");
		//if (!console_parse_type(name, &t, NULL))
		if((t = luacon_ci->GetParticleType(std::string(name)))==-1)
			return luaL_error(l, "Unrecognised element '%s'", name);
	}
	if(i == -1 || (w != -1 && h != -1)){
		// Got a region
		if(i == -1){
			i = 0;
			y = 0;
			w = XRES;
			h = YRES;
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
					if(format == CommandInterface::FormatFloat){
						*((float*)(((unsigned char*)&luacon_sim->parts[i])+offset)) = f;
					} else {
						*((int*)(((unsigned char*)&luacon_sim->parts[i])+offset)) = t;
					}
				}
			}
		}

	} else {
		// Got coords or particle index
		if(i != -1 && y != -1){
			if (i>=XRES || y>=YRES)
				return luaL_error(l, "Coordinates out of range (%d,%d)", i, y);
			r = luacon_sim->pmap[y][i];
			if (!r || (partsel && partsel != luacon_sim->parts[r>>8].type))
				r = luacon_sim->photons[y][i];
			if (!r || (partsel && partsel != luacon_sim->parts[r>>8].type))
				return 0;
			i = r>>8;
		}
		if (i < 0 || i >= NPART)
			return luaL_error(l, "Invalid particle ID '%d'", i);
		if (!luacon_sim->parts[i].type)
			return 0;
		if (partsel && partsel != luacon_sim->parts[i].type)
			return 0;
		if(format == CommandInterface::FormatFloat){
			*((float*)(((unsigned char*)&luacon_sim->parts[i])+offset)) = f;
		} else {
			*((int*)(((unsigned char*)&luacon_sim->parts[i])+offset)) = t;
		}
	}
	return 0;
}

int luatpt_set_wallmap(lua_State* l)
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
				luacon_sim->bmap[ny][nx] = value;
			}
	}
	else	//Set point
	{
		if(x1 > (XRES/CELL))
			x1 = (XRES/CELL);
		if(y1 > (YRES/CELL))
			y1 = (YRES/CELL);
		luacon_sim->bmap[y1][x1] = value;
	}
	return 0;
}

int luatpt_get_wallmap(lua_State* l)
{
	int nx, ny, acount;
	int x1, y1, width, height;
	float value;
	acount = lua_gettop(l);

	x1 = abs(luaL_optint(l, 1, 0));
	y1 = abs(luaL_optint(l, 2, 0));

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
	int nx, ny, acount;
	int x1, y1, width, height;
	float value;
	acount = lua_gettop(l);

	x1 = abs(luaL_optint(l, 1, 0));
	y1 = abs(luaL_optint(l, 2, 0));

	if(x1 > (XRES/CELL) || y1 > (YRES/CELL))
		return luaL_error(l, "Out of range");

	lua_pushinteger(l, luacon_sim->emap[y1][x1]);
	return 1;
}

int luatpt_get_property(lua_State* l)
{
	int i, r, y;
	char *prop;
	prop = (char*)luaL_optstring(l, 1, "");
	i = luaL_optint(l, 2, 0);
	y = luaL_optint(l, 3, -1);
	if(y!=-1 && y < YRES && y >= 0 && i < XRES && i >= 0){
		r = luacon_sim->pmap[y][i];
		if (!r)
			r = luacon_sim->photons[y][i];
		if (!r)
		{
			if (strcmp(prop,"type")==0){
				lua_pushinteger(l, 0);
				return 1;
			}
			return luaL_error(l, "Particle does not exist");
		}
		i = r>>8;
	}
	else if (y!=-1)
		return luaL_error(l, "Coordinates out of range (%d,%d)", i, y);
	if (i < 0 || i >= NPART)
		return luaL_error(l, "Invalid particle ID '%d'", i);
	if (luacon_sim->parts[i].type)
	{
		//TODO: Use particle_getproperty
		if (strcmp(prop,"type")==0){
			lua_pushinteger(l, luacon_sim->parts[i].type);
			return 1;
		}
		if (strcmp(prop,"life")==0){
			lua_pushinteger(l, luacon_sim->parts[i].life);
			return 1;
		}
		if (strcmp(prop,"ctype")==0){
			lua_pushinteger(l, luacon_sim->parts[i].ctype);
			return 1;
		}
		if (strcmp(prop,"temp")==0){
			lua_pushnumber(l, luacon_sim->parts[i].temp);
			return 1;
		}
		if (strcmp(prop,"tmp")==0){
			lua_pushinteger(l, luacon_sim->parts[i].tmp);
			return 1;
		}
		if (strcmp(prop,"tmp2")==0){
			lua_pushinteger(l, luacon_sim->parts[i].tmp2);
			return 1;
		}
		if (strcmp(prop,"vy")==0){
			lua_pushnumber(l, (double)luacon_sim->parts[i].vy);
			return 1;
		}
		if (strcmp(prop,"vx")==0){
			lua_pushnumber(l, (double)luacon_sim->parts[i].vx);
			return 1;
		}
		if (strcmp(prop,"x")==0){
			lua_pushnumber(l, luacon_sim->parts[i].x);
			return 1;
		}
		if (strcmp(prop,"y")==0){
			lua_pushnumber(l, luacon_sim->parts[i].y);
			return 1;
		}
		if (strcmp(prop,"dcolour")==0){
			lua_pushinteger(l, luacon_sim->parts[i].dcolour);
			return 1;
		}
		if (strcmp(prop,"dcolor")==0){
			lua_pushinteger(l, luacon_sim->parts[i].dcolour);
			return 1;
		}
		if (strcmp(prop,"id")==0){
			lua_pushnumber(l, i);
			return 1;
		}
	}
	else if (strcmp(prop,"type")==0){
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

	if (x<0 || y<0 || x>=XRES+BARSIZE || y>=YRES+MENUSIZE)
		return luaL_error(l, "Screen coordinates out of range (%d,%d)", x, y);
	if (r<0) r = 0;
	if (r>255) r = 255;
	if (g<0) g = 0;
	if (g>255) g = 255;
	if (b<0) b = 0;
	if (b>255) b = 255;
	if (a<0) a = 0;
	if (a>255) a = 255;
	luacon_g->blendpixel(x, y, r, g, b, a);
	return 0;
}

int luatpt_drawrect(lua_State* l)
{
	int x, y, w, h, r, g, b, a;
	x = luaL_optint(l, 1, 0);
	y = luaL_optint(l, 2, 0);
	w = luaL_optint(l, 3, 10);
	h = luaL_optint(l, 4, 10);
	r = luaL_optint(l, 5, 255);
	g = luaL_optint(l, 6, 255);
	b = luaL_optint(l, 7, 255);
	a = luaL_optint(l, 8, 255);

	if (x<0 || y<0 || x>=XRES+BARSIZE || y>=YRES+MENUSIZE)
		return luaL_error(l, "Screen coordinates out of range (%d,%d)", x, y);
	if(x+w > XRES+BARSIZE)
		w = XRES+BARSIZE-x;
	if(y+h > YRES+MENUSIZE)
		h = YRES+MENUSIZE-y;
	if (r<0) r = 0;
	if (r>255) r = 255;
	if (g<0) g = 0;
	if (g>255) g = 255;
	if (b<0) b = 0;
	if (b>255) b = 255;
	if (a<0) a = 0;
	if (a>255) a = 255;
	luacon_g->drawrect(x, y, w, h, r, g, b, a);
	return 0;
}

int luatpt_fillrect(lua_State* l)
{
	int x,y,w,h,r,g,b,a;
	x = luaL_optint(l, 1, 0);
	y = luaL_optint(l, 2, 0);
	w = luaL_optint(l, 3, 10);
	h = luaL_optint(l, 4, 10);
	r = luaL_optint(l, 5, 255);
	g = luaL_optint(l, 6, 255);
	b = luaL_optint(l, 7, 255);
	a = luaL_optint(l, 8, 255);

	if (x<0 || y<0 || x>=XRES+BARSIZE || y>=YRES+MENUSIZE)
		return luaL_error(l, "Screen coordinates out of range (%d,%d)", x, y);
	if(x+w > XRES+BARSIZE)
		w = XRES+BARSIZE-x;
	if(y+h > YRES+MENUSIZE)
		h = YRES+MENUSIZE-y;
	if (r<0) r = 0;
	if (r>255) r = 255;
	if (g<0) g = 0;
	if (g>255) g = 255;
	if (b<0) b = 0;
	if (b>255) b = 255;
	if (a<0) a = 0;
	if (a>255) a = 255;
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
	if (r>255) r = 255;
	if (g<0) g = 0;
	if (g>255) g = 255;
	if (b<0) b = 0;
	if (b>255) b = 255;
	if (a<0) a = 0;
	if (a>255) a = 255;
	luacon_g->draw_line(x1, y1, x2, y2, r, g, b, a);
	return 0;
}

int luatpt_textwidth(lua_State* l)
{
	char * string;
	int strwidth = 0;
	string = (char*)luaL_optstring(l, 1, "");
	strwidth = Graphics::textwidth(string);
	lua_pushinteger(l, strwidth);
	return 1;
}

int luatpt_get_name(lua_State* l)
{
	if (luacon_model->GetUser().ID){
		lua_pushstring(l, luacon_model->GetUser().Username.c_str());
		return 1;
	}
	lua_pushstring(l, "");
	return 1;
}

int luatpt_set_shortcuts(lua_State* l)
{
	return luaL_error(l, "set_shortcuts: deprecated");
}

int luatpt_delete(lua_State* l)
{
	int arg1, arg2;
	arg1 = abs(luaL_optint(l, 1, 0));
	arg2 = luaL_optint(l, 2, -1);
	if(arg2 == -1 && arg1 < NPART){
		luacon_sim->kill_part(arg1);
		return 0;
	}
	arg2 = abs(arg2);
	if(arg2 < YRES && arg1 < XRES){
		luacon_sim->delete_part(arg1, arg2, 0);
		return 0;
	}
	return luaL_error(l,"Invalid coordinates or particle ID");
}

int luatpt_register_step(lua_State* l)
{
	int ref, i, ifree = -1;
	if(lua_isfunction(l, 1)){
		for(i = 0; i<6; i++){
			if(!step_functions[i]){
				if (ifree<0) ifree = i;
			} else {
				lua_rawgeti(l, LUA_REGISTRYINDEX, step_functions[i]);
				if(lua_equal(l, 1, lua_gettop(l))){
					lua_pop(l, 1);
					return luaL_error(l, "Function already registered");
				}
				lua_pop(l, 1);
			}
		}
		if (ifree>=0)
		{
			ref = luaL_ref(l, LUA_REGISTRYINDEX);
			step_functions[ifree] = ref;
			return 0;
		}
		else return luaL_error(l, "Step function limit reached");
	}
	return 0;
}
int luatpt_unregister_step(lua_State* l)
{
	int i;
	if(lua_isfunction(l, 1)){
		for(i = 0; i<6; i++){
			if (step_functions[i]){
				lua_rawgeti(l, LUA_REGISTRYINDEX, step_functions[i]);
				if(lua_equal(l, 1, lua_gettop(l))){
					lua_pop(l, 1);
					luaL_unref(l, LUA_REGISTRYINDEX, step_functions[i]);
					step_functions[i] = 0;
				}
				else lua_pop(l, 1);
			}
		}
	}
	return 0;
}
int luatpt_register_keypress(lua_State* l)
{
	int *newfunctions, i;
	if(lua_isfunction(l, 1)){
		for(i = 0; i<keypress_function_count; i++){
			lua_rawgeti(l, LUA_REGISTRYINDEX, keypress_functions[i]);
			if(lua_equal(l, 1, lua_gettop(l))){
				lua_pop(l, 1);
				return luaL_error(l, "Function already registered");
			}
			lua_pop(l, 1);
		}
		newfunctions = (int*)calloc(keypress_function_count+1, sizeof(int));
		if(keypress_functions){
			memcpy(newfunctions, keypress_functions, keypress_function_count*sizeof(int));
			free(keypress_functions);
		}
		newfunctions[keypress_function_count] = luaL_ref(l, LUA_REGISTRYINDEX);
		keypress_function_count++;
		keypress_functions = newfunctions;
	}
	return 0;
}
int luatpt_unregister_keypress(lua_State* l)
{
	int *newfunctions, i, functionindex = -1;
	if(lua_isfunction(l, 1)){
		for(i = 0; i<keypress_function_count; i++){
			lua_rawgeti(l, LUA_REGISTRYINDEX, keypress_functions[i]);
			if(lua_equal(l, 1, lua_gettop(l))){
				functionindex = i;
			}
			lua_pop(l, 1);
		}
	}
	if(functionindex != -1){
		luaL_unref(l, LUA_REGISTRYINDEX, keypress_functions[functionindex]);
		if(functionindex != keypress_function_count-1){
			memmove(keypress_functions+functionindex+1, keypress_functions+functionindex+1, (keypress_function_count-functionindex-1)*sizeof(int));
		}
		if(keypress_function_count-1 > 0){
			newfunctions = (int*)calloc(keypress_function_count-1, sizeof(int));
			memcpy(newfunctions, keypress_functions, (keypress_function_count-1)*sizeof(int));
			free(keypress_functions);
			keypress_functions = newfunctions;
		} else {
			free(keypress_functions);
			keypress_functions = NULL;
		}
		keypress_function_count--;
	} else {
		return luaL_error(l, "Function not registered");
	}
	return 0;
}
int luatpt_register_mouseclick(lua_State* l)
{
	int *newfunctions, i;
	if(lua_isfunction(l, 1)){
		for(i = 0; i<mouseclick_function_count; i++){
			lua_rawgeti(l, LUA_REGISTRYINDEX, mouseclick_functions[i]);
			if(lua_equal(l, 1, lua_gettop(l))){
				lua_pop(l, 1);
				return luaL_error(l, "Function already registered");
			}
			lua_pop(l, 1);
		}
		newfunctions = (int*)calloc(mouseclick_function_count+1, sizeof(int));
		if(mouseclick_functions){
			memcpy(newfunctions, mouseclick_functions, mouseclick_function_count*sizeof(int));
			free(mouseclick_functions);
		}
		newfunctions[mouseclick_function_count] = luaL_ref(l, LUA_REGISTRYINDEX);
		mouseclick_function_count++;
		mouseclick_functions = newfunctions;
	}
	return 0;
}
int luatpt_unregister_mouseclick(lua_State* l)
{
	int *newfunctions, i, functionindex = -1;
	if(lua_isfunction(l, 1)){
		for(i = 0; i<mouseclick_function_count; i++){
			lua_rawgeti(l, LUA_REGISTRYINDEX, mouseclick_functions[i]);
			if(lua_equal(l, 1, lua_gettop(l))){
				functionindex = i;
			}
			lua_pop(l, 1);
		}
	}
	if(functionindex != -1){
		luaL_unref(l, LUA_REGISTRYINDEX, mouseclick_functions[functionindex]);
		if(functionindex != mouseclick_function_count-1){
			memmove(mouseclick_functions+functionindex+1, mouseclick_functions+functionindex+1, (mouseclick_function_count-functionindex-1)*sizeof(int));
		}
		if(mouseclick_function_count-1 > 0){
			newfunctions = (int*)calloc(mouseclick_function_count-1, sizeof(int));
			memcpy(newfunctions, mouseclick_functions, (mouseclick_function_count-1)*sizeof(int));
			free(mouseclick_functions);
			mouseclick_functions = newfunctions;
		} else {
			free(mouseclick_functions);
			mouseclick_functions = NULL;
		}
		mouseclick_function_count--;
	} else {
		return luaL_error(l, "Function not registered");
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
	new InformationMessage(title, message);
	return 0;
}
int luatpt_get_numOfParts(lua_State* l)
{
    lua_pushinteger(l, luacon_sim->parts_lastActiveIndex);
    return 1;
}
int luatpt_start_getPartIndex(lua_State* l)
{
    getPartIndex_curIdx = -1;
    return 1;
}
int luatpt_next_getPartIndex(lua_State* l)
{
    while(1)
    {
        getPartIndex_curIdx++;
        if(getPartIndex_curIdx >= NPART)
        {
            getPartIndex_curIdx = 0;
            lua_pushboolean(l, 0);
            return 1;
        }
        if(luacon_sim->parts[getPartIndex_curIdx].type)
            break;

    }

    lua_pushboolean(l, 1);
    return 1;
}
int luatpt_getPartIndex(lua_State* l)
{
    if(getPartIndex_curIdx < 0)
    {
        lua_pushinteger(l, 0);
        return 1;
    }
    lua_pushinteger(l, getPartIndex_curIdx);
    return 1;
}
int luatpt_hud(lua_State* l)
{
    /*int hudstate;
    hudstate = luaL_optint(l, 1, 0);
    hud_enable = (hudstate==0?0:1);
    return 0;*/
	//TODO IMPLEMENT
	return 0;
}
int luatpt_gravity(lua_State* l)
{
    int gravstate;
    gravstate = luaL_optint(l, 1, 0);
    if(gravstate)
        luacon_sim->grav->start_grav_async();
    else
        luacon_sim->grav->stop_grav_async();
    return 0;
}
int luatpt_airheat(lua_State* l)
{
    int aheatstate;
    aheatstate = luaL_optint(l, 1, 0);
    luacon_sim->aheat_enable = (aheatstate==0?0:1);
    return 0;
}
int luatpt_active_menu(lua_State* l)
{
    int menuid;
    menuid = luaL_optint(l, 1, -1);
    if (menuid < SC_TOTAL && menuid >= 0)
    	luacon_model->SetActiveMenu(luacon_model->GetMenuList()[menuid]);
    else
        return luaL_error(l, "Invalid menu");
    return 0;
}
int luatpt_decorations_enable(lua_State* l)
{
	int decostate;
	decostate = luaL_optint(l, 1, 0);
	luacon_model->SetDecoration(decostate==0?false:true);
	return 0;
}

int luatpt_heat(lua_State* l)
{
	int heatstate;
	heatstate = luaL_optint(l, 1, 0);
	luacon_sim->legacy_enable = (heatstate==1?0:1);
	return 0;
}
int luatpt_cmode_set(lua_State* l)
{
	//TODO IMPLEMENT
    return luaL_error(l, "cmode_set: Deprecated");
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
	return luaL_error(l, "setdebug: Deprecated");
}
int luatpt_setfpscap(lua_State* l)
{
	int fpscap = luaL_optint(l, 1, 0);
	if (fpscap < 2)
		return luaL_error(l, "fps cap too small");
	ui::Engine::Ref().FpsLimit = fpscap;
	return 0;
}
int luatpt_getscript(lua_State* l)
{
	char *filedata = NULL, *fileuri = NULL, *filename = NULL, *lastError = NULL, *luacommand = NULL;
	std::string fileauthor = "", fileid = "";
	int len, ret,run_script;
	FILE * outputfile;

	fileauthor = std::string(luaL_optstring(l, 1, ""));
	fileid = std::string(luaL_optstring(l, 2, ""));
	run_script = luaL_optint(l, 3, 0);
	if(!fileauthor.length() || !fileid.length())
		goto fin;
	if(!ConfirmPrompt::Blocking("Do you want to install script?", fileid, "Install"))
		goto fin;

	fileuri = new char[strlen(SCRIPTSERVER)+fileauthor.length()+fileid.length()+44];
	sprintf(fileuri, "http://" SCRIPTSERVER "/GetScript.api?Author=%s&Filename=%s", fileauthor.c_str(), fileid.c_str());

	//filedata = http_auth_get(fileuri, svf_user_id, NULL, svf_session_id, &ret, &len);
	filedata = http_auth_get(fileuri, NULL, NULL, NULL, &ret, &len);

	if(len <= 0 || !filedata)
	{
		lastError = "Server did not return data.";
		goto fin;
	}
	if(ret != 200)
	{
		lastError = http_ret_text(ret);
		goto fin;
	}

	filename = new char[fileauthor.length()+fileid.length()+strlen(PATH_SEP)+strlen(LOCAL_LUA_DIR)+6];
	sprintf(filename, LOCAL_LUA_DIR PATH_SEP "%s_%s.lua", fileauthor.c_str(), fileid.c_str());

#ifdef WIN
	_mkdir(LOCAL_LUA_DIR);
#else
	mkdir(LOCAL_LUA_DIR, 0755);
#endif

	outputfile = fopen(filename, "r");
	if(outputfile)
	{
		fclose(outputfile);
		outputfile = NULL;
		if(ConfirmPrompt::Blocking("File already exists, overwrite?", filename, "Overwrite"))
		{
			outputfile = fopen(filename, "w");
		}
		else
		{
			goto fin;
		}
	}
	else
	{
		outputfile = fopen(filename, "w");
	}

	if(!outputfile)
	{
		lastError = "Unable to write to file";
		goto fin;
	}


	fputs(filedata, outputfile);
	fclose(outputfile);
	outputfile = NULL;
	if(run_script)
	{
		luacommand = new char[strlen(filename)+20];
		sprintf(luacommand,"dofile(\"%s\")",filename);
		luaL_dostring (l, luacommand);
    }

fin:
	if(filedata) free(filedata);
	if(fileuri) delete[] fileuri;
	if(filename) delete[] filename;
	if(luacommand) delete[] luacommand;
	luacommand = NULL;

	if(lastError) return luaL_error(l, lastError);
	return 0;
}

int luatpt_setwindowsize(lua_State* l)
{
	int scale = luaL_optint(l,1,1), kiosk = luaL_optint(l,2,0);
	if (scale!=2) scale = 1;
	if (kiosk!=1) kiosk = 0;
	ui::Engine::Ref().SetScale(scale);
	ui::Engine::Ref().SetFullscreen(kiosk);
	return 0;
}

int screenshotIndex = 0;

int luatpt_screenshot(lua_State* l)
{
	//TODO Implement
	int captureUI = luaL_optint(l, 1, 0);
	std::vector<char> data;
	if(captureUI)
	{
		VideoBuffer screenshot(ui::Engine::Ref().g->DumpFrame());
		data = format::VideoBufferToPNG(screenshot);
	}
	else
	{
		VideoBuffer screenshot(luacon_ren->DumpFrame());
		data = format::VideoBufferToPNG(screenshot);
	}
	std::stringstream filename;
	filename << "screenshot_";
	filename << std::setfill('0') << std::setw(6) << (screenshotIndex++);
	filename << ".png";
	Client::Ref().WriteFile(data, filename.str());
	return 0;
}

