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
#include <fstream>
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

//#include "virtualmachine/VirtualMachine.h"
#include "pim/Parser.h"
#include "pim/Machine.h"

#include "LuaBit.h"

#include "LuaWindow.h"
#include "LuaButton.h"
#include "LuaLabel.h"
#include "LuaTextbox.h"
#include "LuaCheckbox.h"
#include "LuaSlider.h"
#include "LuaProgressBar.h"

#ifdef __unix__
#include <unistd.h>
#endif

extern "C"
{
#ifdef WIN
#include <direct.h>
#endif
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
}

GameModel * luacon_model;
Simulation * luacon_sim;
LuaScriptInterface * luacon_ci;
Graphics * luacon_g;
Renderer * luacon_ren;

bool *luacon_currentCommand;
std::string *luacon_lastError;

int *lua_el_func, *lua_el_mode, *lua_gr_func;

int getPartIndex_curIdx;
int step_functions[6] = {0, 0, 0, 0, 0, 0};
int keypress_function_count = 0;
int *keypress_functions = NULL;
int mouseclick_function_count = 0;
int *mouseclick_functions = NULL;
int tptProperties; //Table for some TPT properties
int tptPropertiesVersion;
int tptElements; //Table for TPT element names
int tptParts, tptPartsMeta, tptElementTransitions, tptPartsCData, tptPartMeta, tptPart, cIndex;

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
	initVirtualMachineAPI();
	initGraphicsAPI();
	initFileSystemAPI();

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
	if(Client::Ref().FileExists("autorun.lua"))		
		if(luacon_eval("dofile(\"autorun.lua\")"))
			luacon_ci->Log(CommandInterface::LogError, luacon_geterror());
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
	Luna<LuaCheckbox>::Register(l);
	Luna<LuaSlider>::Register(l);
	Luna<LuaProgressBar>::Register(l);
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
	else if(luaComponent = Luna<LuaCheckbox>::tryGet(l, 1))
		component = Luna<LuaCheckbox>::get(luaComponent)->GetComponent();
	else if(luaComponent = Luna<LuaSlider>::tryGet(l, 1))
		component = Luna<LuaSlider>::get(luaComponent)->GetComponent();
	else if(luaComponent = Luna<LuaProgressBar>::tryGet(l, 1))
		component = Luna<LuaProgressBar>::get(luaComponent)->GetComponent();
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
		{"decorations", renderer_decorations},
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
		return 0;
	}
	else
	{
		lua_pushinteger(l, luacon_ren->GetColourMode());
		return 1;
	}
}

int LuaScriptInterface::renderer_decorations(lua_State * l)
{
	int args = lua_gettop(l);
	if(args)
	{
		luacon_ren->decorations_enable = lua_toboolean(l, 1);
		return 0;
	}
	else
	{
		lua_pushboolean(l, luacon_ren->decorations_enable);
		return 1;
	}
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
	lua_pushinteger(l, SC_SENSOR);		lua_setfield(l, elementsAPI, "SC_SENSOR");

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

pim::VirtualMachine * LuaScriptInterface::updateVirtualMachines[PT_NUM];

int LuaScriptInterface::updateVM(UPDATE_FUNC_ARGS)
{
	pim::VirtualMachine * machine = updateVirtualMachines[parts[i].type];

	machine->CSPush(i);
	machine->CSPush(x);
	machine->CSPush(y);
	machine->Call(0);


	/*vm::VirtualMachine * vMachine = updateVirtualMachines[parts[i].type];

	vm::word w;
	int argAddr = 0, argCount = 5;
	vMachine->sim = sim;
	
	vMachine->OpPUSH(w);  //Pointless null in stack
	w.int4 = (argCount + 2) * sizeof(vm::word);
	vMachine->OpENTER(w);
	argAddr = 8;

	//Arguments
	w.int4 = i; vMachine->Marshal(argAddr, w); argAddr += 4;
	w.int4 = x; vMachine->Marshal(argAddr, w); argAddr += 4;
	w.int4 = y; vMachine->Marshal(argAddr, w); argAddr += 4;
	w.int4 = nt; vMachine->Marshal(argAddr, w); argAddr += 4;
	w.int4 = surround_space; vMachine->Marshal(argAddr, w); argAddr += 4;
	
	w.int4 = 0;
	vMachine->Push(w);
	
	vMachine->OpCALL(w);
	vMachine->Run();
	w.int4 = (argCount + 2) * sizeof(vm::word);
	vMachine->OpLEAVE(w);
	vMachine->OpPOP(w);   //Pop pointless null
	vMachine->End();*/

	return 0;
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
		lua_pushnil(l);
		lua_setglobal(l, "elem");

		lua_getglobal(l, "package");
		lua_getfield(l, -1, "loaded");
		lua_pushnil(l);
		lua_setfield(l, -2, "elements");

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
			else if(lua_type(l, 3) == LUA_TLIGHTUSERDATA)
			{
				updateVirtualMachines[id] = (pim::VirtualMachine*)lua_touserdata(l, 3);
				luacon_sim->elements[id].Update = &updateVM;
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

void LuaScriptInterface::initVirtualMachineAPI()
{
	//Methods
	struct luaL_reg vmAPIMethods [] = {
		{"loadProgram", virtualMachine_loadProgram},
		{NULL, NULL}
	};
	luaL_register(l, "virtualMachine", vmAPIMethods);

	//elem shortcut
	lua_getglobal(l, "virtualMachine");
	lua_setglobal(l, "vm");

	int vmAPI = lua_gettop(l);
}

int LuaScriptInterface::virtualMachine_loadProgram(lua_State * l)
{
	/*luaL_checktype(l, 1, LUA_TSTRING);

	vm::VirtualMachine * newVM = new vm::VirtualMachine(1);
	try
	{
		const char * tempString = lua_tostring(l, 1);
		int tempStringLength = lua_strlen(l, 1);
		std::vector<char> programData(tempString, tempString+tempStringLength);
		newVM->LoadProgram(programData);
	}
	catch(std::exception & e)
	{
		return luaL_error(l, "Unable to load program");
	}
	lua_pushlightuserdata(l, newVM);*/
	std::string programSource(lua_tostring(l, 1));
	std::stringstream input(programSource);


	pim::compiler::Parser * parser = new pim::compiler::Parser(input);
	
	std::vector<unsigned char> programData = parser->Compile();

	pim::VirtualMachine * machine = new pim::VirtualMachine(luacon_sim);
	machine->LoadProgram(programData);

	lua_pushlightuserdata(l, machine);
	return 1;
}

void LuaScriptInterface::initGraphicsAPI()
{
	//Methods
	struct luaL_reg graphicsAPIMethods [] = {
		{"textSize", graphics_textSize},
		{"drawText", graphics_drawText},
		{"drawLine", graphics_drawLine},
		{"drawRect", graphics_drawRect},
		{"fillRect", graphics_fillRect},
		{NULL, NULL}
	};
	luaL_register(l, "graphics", graphicsAPIMethods);

	//elem shortcut
	lua_getglobal(l, "graphics");
	lua_setglobal(l, "gfx");

	int graphicsAPI = lua_gettop(l);

	lua_pushinteger(l, XRES+BARSIZE);	lua_setfield(l, graphicsAPI, "WIDTH");
	lua_pushinteger(l, YRES+MENUSIZE);	lua_setfield(l, graphicsAPI, "HEIGHT");
}

int LuaScriptInterface::graphics_textSize(lua_State * l)
{
    char * text;
    int width, height;
	text = (char*)lua_tostring(l, 1);
	Graphics::textsize(text, width, height);

	lua_pushinteger(l, width);
	lua_pushinteger(l, height);
	return 2;
}

int LuaScriptInterface::graphics_drawText(lua_State * l)
{
    char * text;
	int x, y, r, g, b, a;
	x = lua_tointeger(l, 1);
	y = lua_tointeger(l, 2);
	text = (char*)lua_tostring(l, 3);
	r = luaL_optint(l, 4, 255);
	g = luaL_optint(l, 5, 255);
	b = luaL_optint(l, 6, 255);
	a = luaL_optint(l, 7, 255);
	
	if (r<0) r = 0;
	if (r>255) r = 255;
	if (g<0) g = 0;
	if (g>255) g = 255;
	if (b<0) b = 0;
	if (b>255) b = 255;
	if (a<0) a = 0;
	if (a>255) a = 255;

	luacon_g->drawtext(x, y, text, r, g, b, a);
	return 0;
}

int LuaScriptInterface::graphics_drawLine(lua_State * l)
{
	int x1, y1, x2, y2, r, g, b, a;
	x1 = lua_tointeger(l, 1);
	y1 = lua_tointeger(l, 2);
	x2 = lua_tointeger(l, 3);
	y2 = lua_tointeger(l, 4);
	r = luaL_optint(l, 5, 255);
	g = luaL_optint(l, 6, 255);
	b = luaL_optint(l, 7, 255);
	a = luaL_optint(l, 8, 255);

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

int LuaScriptInterface::graphics_drawRect(lua_State * l)
{
	int x, y, w, h, r, g, b, a;
	x = lua_tointeger(l, 1);
	y = lua_tointeger(l, 2);
	w = lua_tointeger(l, 3);
	h = lua_tointeger(l, 4);
	r = luaL_optint(l, 5, 255);
	g = luaL_optint(l, 6, 255);
	b = luaL_optint(l, 7, 255);
	a = luaL_optint(l, 8, 255);

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

int LuaScriptInterface::graphics_fillRect(lua_State * l)
{
	int x, y, w, h, r, g, b, a;
	x = lua_tointeger(l, 1);
	y = lua_tointeger(l, 2);
	w = lua_tointeger(l, 3);
	h = lua_tointeger(l, 4);
	r = luaL_optint(l, 5, 255);
	g = luaL_optint(l, 6, 255);
	b = luaL_optint(l, 7, 255);
	a = luaL_optint(l, 8, 255);

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

void LuaScriptInterface::initFileSystemAPI()
{
	//Methods
	struct luaL_reg fileSystemAPIMethods [] = {
		{"list", fileSystem_list},
		{"exists", fileSystem_exists},
		{"isFile", fileSystem_isFile},
		{"isDirectory", fileSystem_isDirectory},
		{"makeDirectory", fileSystem_makeDirectory},
		{"removeDirectory", fileSystem_removeDirectory},
		{"removeFile", fileSystem_removeFile},
		{"move", fileSystem_move},
		{"copy", fileSystem_copy},
		{NULL, NULL}
	};
	luaL_register(l, "fileSystem", fileSystemAPIMethods);

	//elem shortcut
	lua_getglobal(l, "fileSystem");
	lua_setglobal(l, "fs");

	int fileSystemAPI = lua_gettop(l);
}

int LuaScriptInterface::fileSystem_list(lua_State * l)
{
	const char * directoryName = lua_tostring(l, 1);

	int index = 1;
	lua_newtable(l);

	DIR * directory;
	struct dirent * entry;

	directory = opendir(directoryName);
	if (directory != NULL)
	{
		while (entry = readdir(directory))
		{
			if(strncmp(entry->d_name, "..", 3) && strncmp(entry->d_name, ".", 2))
			{
				lua_pushstring(l, entry->d_name);
				lua_rawseti(l, -2, index++);
			}
		}
		closedir(directory);
	}
	else
	{
		lua_pushnil(l);
	}

	return 1;
}

int LuaScriptInterface::fileSystem_exists(lua_State * l)
{
	const char * filename = lua_tostring(l, 1);

	bool exists = false;
#ifdef WIN
	struct _stat s;
	if(_stat(filename, &s) == 0)
#else
	struct stat s;
	if(stat(filename, &s) == 0)
#endif
	{
		if(s.st_mode & S_IFDIR)
		{
			exists = true;
		}
		else if(s.st_mode & S_IFREG)
		{
			exists = true;
		}
		else
		{
			exists = true;
		}
	}
	else
	{
		exists = false;
	}

	lua_pushboolean(l, exists);
	return 1;
}

int LuaScriptInterface::fileSystem_isFile(lua_State * l)
{
	const char * filename = lua_tostring(l, 1);

	bool exists = false;
#ifdef WIN
	struct _stat s;
	if(_stat(filename, &s) == 0)
#else
	struct stat s;
	if(stat(filename, &s) == 0)
#endif
	{
		if(s.st_mode & S_IFDIR)
		{
			exists = true;
		}
		else if(s.st_mode & S_IFREG)
		{
			exists = false;
		}
		else
		{
			exists = false;
		}
	}
	else
	{
		exists = false;
	}

	lua_pushboolean(l, exists);
	return 1;
}

int LuaScriptInterface::fileSystem_isDirectory(lua_State * l)
{
	const char * filename = lua_tostring(l, 1);

	bool exists = false;
#ifdef WIN
	struct _stat s;
	if(_stat(filename, &s) == 0)
#else
	struct stat s;
	if(stat(filename, &s) == 0)
#endif
	{
		if(s.st_mode & S_IFDIR)
		{
			exists = false;
		}
		else if(s.st_mode & S_IFREG)
		{
			exists = true;
		}
		else
		{
			exists = false;
		}
	}
	else
	{
		exists = false;
	}

	lua_pushboolean(l, exists);
	return 1;
}

int LuaScriptInterface::fileSystem_makeDirectory(lua_State * l)
{
	const char * dirname = lua_tostring(l, 1);

	int ret = 0;
	ret = Client::Ref().MakeDirectory(dirname);
	lua_pushboolean(l, ret == 0);
	return 1;
}

int LuaScriptInterface::fileSystem_removeDirectory(lua_State * l)
{
	const char * filename = lua_tostring(l, 1);

	int ret = 0;
#ifdef WIN
	ret = _rmdir(filename);
#else
	ret = rmdir(filename);
#endif
	lua_pushboolean(l, ret == 0);
	return 1;
}

int LuaScriptInterface::fileSystem_removeFile(lua_State * l)
{
	const char * filename = lua_tostring(l, 1);

	int ret = 0;
#ifdef WIN
	ret = _unlink(filename);
#else
	ret = unlink(filename);
#endif
	lua_pushboolean(l, ret == 0);
	return 1;
}

int LuaScriptInterface::fileSystem_move(lua_State * l)
{
	const char * filename = lua_tostring(l, 1);
	const char * newFilename = lua_tostring(l, 2);
	int ret = 0;

	ret = rename(filename, newFilename);

	lua_pushboolean(l, ret == 0);
	return 1;
}

int LuaScriptInterface::fileSystem_copy(lua_State * l)
{
	const char * filename = lua_tostring(l, 1);
	const char * newFilename = lua_tostring(l, 2);
	int ret = 0;

	try
	{
		std::ifstream source(filename, std::ios::binary);
		std::ofstream dest(newFilename, std::ios::binary);
		source.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		dest.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		std::istreambuf_iterator<char> begin_source(source);
		std::istreambuf_iterator<char> end_source;
		std::ostreambuf_iterator<char> begin_dest(dest); 
		std::copy(begin_source, end_source, begin_dest);

		source.close();
		dest.close();

		ret = 0;
	}
	catch (std::exception & e)
	{
		ret = 1;
	}

	lua_pushboolean(l, ret == 0);
	return 1;
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
