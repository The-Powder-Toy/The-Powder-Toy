#ifdef LUACONSOLE
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
#include "gui/dialogues/ErrorMessage.h"
#include "gui/dialogues/InformationMessage.h"
#include "gui/dialogues/TextPrompt.h"
#include "gui/dialogues/ConfirmPrompt.h" 
#include "simulation/Simulation.h"
#include "simulation/Air.h"
#include "ToolClasses.h"
#include "gui/game/GameModel.h"
#include "gui/game/Tool.h"
#include "LuaScriptHelper.h"
#include "client/HTTP.h"
#include "Misc.h"
#include "PowderToy.h"

#include "LuaBit.h"

#include "LuaWindow.h"
#include "LuaButton.h"
#include "LuaLabel.h"
#include "LuaTextbox.h"
#include "LuaCheckbox.h"
#include "LuaSlider.h"
#include "LuaProgressBar.h"

#ifndef WIN
#include <unistd.h>
#endif

extern "C"
{
#ifdef WIN
#include <direct.h>
#endif
#include <sys/stat.h>
#include <dirent.h>
#include "socket/luasocket.h"
}
#include "socket/socket.lua.h"

GameModel * luacon_model;
GameController * luacon_controller;
Simulation * luacon_sim;
LuaScriptInterface * luacon_ci;
Graphics * luacon_g;
Renderer * luacon_ren;

bool *luacon_currentCommand;
std::string *luacon_lastError;
std::string lastCode;

int *lua_el_func, *lua_el_mode, *lua_gr_func;

int getPartIndex_curIdx;
int tptProperties; //Table for some TPT properties
int tptPropertiesVersion;
int tptElements; //Table for TPT element names
int tptParts, tptPartsMeta, tptElementTransitions, tptPartsCData, tptPartMeta, tptPart, cIndex;

int atPanic(lua_State *l)
{
	throw std::runtime_error("Unprotected lua panic: " + std::string(lua_tostring(l, -1)));
}

LuaScriptInterface::LuaScriptInterface(GameController * c, GameModel * m):
	CommandInterface(c, m),
	currentCommand(false),
	legacy(new TPTScriptInterface(c, m)),
	luacon_mousex(0),
	luacon_mousey(0),
	luacon_mousebutton(0),
	luacon_brushx(0),
	luacon_brushy(0),
	luacon_selectedl(""),
	luacon_selectedr(""),
	luacon_selectedalt(""),
	luacon_selectedreplace(""),
	luacon_mousedown(false)
{
	luacon_model = m;
	luacon_controller = c;
	luacon_sim = m->GetSimulation();
	luacon_g = ui::Engine::Ref().g;
	luacon_ren = m->GetRenderer();
	luacon_ci = this;

	//New TPT API
	l = luaL_newstate();
	lua_atpanic(l, atPanic);
	luaL_openlibs(l);
	luaopen_bit(l);
	luaopen_socket_core(l);
	lua_getglobal(l, "package");
	lua_pushstring(l, "loaded");
	lua_rawget(l, -2);
	lua_pushstring(l, "socket");
	lua_rawget(l, -2);
	lua_pushstring(l, "socket.core");
	lua_pushvalue(l, -2);
	lua_rawset(l, -4);
	lua_pop(l, 3);
	luaopen_socket(l);

	lua_pushstring(l, "Luacon_ci");
	lua_pushlightuserdata(l, this);
	lua_settable(l, LUA_REGISTRYINDEX);

	initSimulationAPI();
	initInterfaceAPI();
	initRendererAPI();
	initElementsAPI();
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
		{"get_clipboard", &luatpt_getclip}, 
		{"set_clipboard", &luatpt_setclip},
		{NULL,NULL}
	};

	luacon_mousedown = false;
	luacon_mousebutton = 0;

	luacon_currentCommand = &currentCommand;
	luacon_lastError = &lastError;

	lastCode = "";

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
	lua_pushstring(l, "DEFAULT_PT_DUST");
	lua_setfield(l, tptProperties, "selectedl");
	lua_pushstring(l, "DEFAULT_PT_NONE");
	lua_setfield(l, tptProperties, "selectedr");
	lua_pushstring(l, "DEFAULT_PT_NONE");
	lua_setfield(l, tptProperties, "selecteda");
	lua_pushstring(l, "DEFAULT_PT_NONE");
	lua_setfield(l, tptProperties, "selectedreplace");

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
		lua_gr_func[i] = 0;
	}

}

void LuaScriptInterface::Init()
{
	if(Client::Ref().FileExists("autorun.lua"))
	{
		lua_State *l = luacon_ci->l;
		if(luaL_loadfile(l, "autorun.lua") || lua_pcall(l, 0, 0, 0))
			luacon_ci->Log(CommandInterface::LogError, luacon_geterror());
		else
			luacon_ci->Log(CommandInterface::LogNotice, "Loaded autorun.lua");
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
		{"removeComponent", interface_addComponent},
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

int LuaScriptInterface::interface_removeComponent(lua_State * l)
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
		luacon_ci->Window->RemoveComponent(component);
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

StructProperty * LuaScriptInterface::particleProperties;
int LuaScriptInterface::particlePropertiesCount;

void LuaScriptInterface::initSimulationAPI()
{
	//Methods
	struct luaL_reg simulationAPIMethods [] = {
		{"partNeighbours", simulation_partNeighbours},
		{"partNeighbors", simulation_partNeighbours},
		{"partChangeType", simulation_partChangeType},
		{"partCreate", simulation_partCreate},
		{"partProperty", simulation_partProperty},
		{"partPosition", simulation_partPosition},
		{"partID", simulation_partID},
		{"partKill", simulation_partKill},
		{"pressure", simulation_pressure},
		{"ambientHeat", simulation_ambientHeat},
		{"velocityX", simulation_velocityX},
		{"velocityY", simulation_velocityY},
		{"gravMap", simulation_gravMap},
		{"createParts", simulation_createParts},
		{"createLine", simulation_createLine},
		{"createBox", simulation_createBox},
		{"floodParts", simulation_floodParts},
		{"createWalls", simulation_createWalls},
		{"createWallLine", simulation_createWallLine},
		{"createWallBox", simulation_createWallBox},
		{"floodWalls", simulation_floodWalls},
		{"toolBrush", simulation_toolBrush},
		{"toolLine", simulation_toolLine},
		{"toolBox", simulation_toolBox},
		{"decoBrush", simulation_decoBrush},
		{"decoLine", simulation_decoLine},
		{"decoBox", simulation_decoBox},
		{"decoColor", simulation_decoColor},
		{"decoColour", simulation_decoColor},
		{"clearSim", simulation_clearSim},
		{"resetTemp", simulation_resetTemp},
		{"resetPressure", simulation_resetPressure},
		{"saveStamp", simulation_saveStamp},
		{"loadStamp", simulation_loadStamp},
		{"deleteStamp", simulation_deleteStamp},
		{"loadSave", simulation_loadSave},
		{"reloadSave", simulation_reloadSave},
		{"getSaveID", simulation_getSaveID},
		{"adjustCoords", simulation_adjustCoords},
		{"prettyPowders", simulation_prettyPowders},
		{"gravityGrid", simulation_gravityGrid},
		{"edgeMode", simulation_edgeMode},
		{"gravityMode", simulation_gravityMode},
		{"airMode", simulation_airMode},
		{"waterEqualisation", simulation_waterEqualisation},
		{"waterEqualization", simulation_waterEqualisation},
		{"ambientAirTemp", simulation_ambientAirTemp},
		{"elementCount", simulation_elementCount},
		{"can_move", simulation_canMove},
		{"parts", simulation_parts},
		{"pmap", simulation_pmap},
		{"neighbours", simulation_neighbours},
		{"neighbors", simulation_neighbours},
		{NULL, NULL}
	};
	luaL_register(l, "simulation", simulationAPIMethods);

	//Sim shortcut
	lua_getglobal(l, "simulation");
	lua_setglobal(l, "sim");

	//Static values
	SETCONST(l, XRES);
	SETCONST(l, YRES);
	SETCONST(l, PT_NUM);
	lua_pushinteger(l, 0); lua_setfield(l, -2, "NUM_PARTS");
	SETCONST(l, R_TEMP);
	SETCONST(l, MAX_TEMP);
	SETCONST(l, MIN_TEMP);

	SETCONST(l, TOOL_HEAT);
	SETCONST(l, TOOL_COOL);
	SETCONST(l, TOOL_VAC);
	SETCONST(l, TOOL_AIR);
	SETCONST(l, TOOL_PGRV);
	SETCONST(l, TOOL_NGRV);
	lua_pushinteger(l, luacon_sim->tools.size()); lua_setfield(l, -2, "TOOL_WIND");
	SETCONST(l, DECO_DRAW);
	SETCONST(l, DECO_CLEAR);
	SETCONST(l, DECO_ADD);
	SETCONST(l, DECO_SUBTRACT);
	SETCONST(l, DECO_MULTIPLY);
	SETCONST(l, DECO_DIVIDE);
	SETCONST(l, DECO_SMUDGE);

	//Declare FIELD_BLAH constants
	std::vector<StructProperty> particlePropertiesV = Particle::GetProperties(); 
	particlePropertiesCount = 0;
	particleProperties = new StructProperty[particlePropertiesV.size()];
	for(std::vector<StructProperty>::iterator iter = particlePropertiesV.begin(), end = particlePropertiesV.end(); iter != end; ++iter)
	{
		std::string propertyName = (*iter).Name;
		std::transform(propertyName.begin(), propertyName.end(), propertyName.begin(), ::toupper);
		lua_pushinteger(l, particlePropertiesCount);
		lua_setfield(l, -2, ("FIELD_"+propertyName).c_str());
		particleProperties[particlePropertiesCount++] = *iter;
	}
}

void LuaScriptInterface::set_map(int x, int y, int width, int height, float value, int map) // A function so this won't need to be repeated many times later
{
	int nx, ny;
	if(x > (XRES/CELL)-1)
		x = (XRES/CELL)-1;
	if(y > (YRES/CELL)-1)
		y = (YRES/CELL)-1;
	if(x+width > (XRES/CELL)-1)
		width = (XRES/CELL)-x;
	if(y+height > (YRES/CELL)-1)
		height = (YRES/CELL)-y;
	for (nx = x; nx<x+width; nx++)
		for (ny = y; ny<y+height; ny++)
		{
			if (map == 1)
				luacon_sim->pv[ny][nx] = value;
			else if (map == 2)
				luacon_sim->hv[ny][nx] = value;
			else if (map == 3)
				luacon_sim->vx[ny][nx] = value;
			else if (map == 4)
				luacon_sim->vy[ny][nx] = value;
			else if (map == 5)
				luacon_sim->gravmap[ny*XRES/CELL+nx] = value; //gravx/y don't seem to work, but this does. opposite of tpt
		}
}

int LuaScriptInterface::simulation_partNeighbours(lua_State * l)
{
	lua_newtable(l);
	int id = 0;
	int x = lua_tointeger(l, 1), y = lua_tointeger(l, 2), r = lua_tointeger(l, 3), rx, ry, n;
	if(lua_gettop(l) == 5) // this is one more than the number of arguments because a table has just been pushed onto the stack with lua_newtable(l);
	{
		int t = lua_tointeger(l, 4);
		for (rx = -r; rx <= r; rx++)
			for (ry = -r; ry <= r; ry++)
				if (x+rx >= 0 && y+ry >= 0 && x+rx < XRES && y+ry < YRES && (rx || ry))
				{
					n = luacon_sim->pmap[y+ry][x+rx];
					if(n && (n&0xFF) == t)
					{
						lua_pushinteger(l, n>>8);
						lua_rawseti(l, -2, id++);
					}
				}

	}
	else
	{
		for (rx = -r; rx <= r; rx++)
			for (ry = -r; ry <= r; ry++)
				if (x+rx >= 0 && y+ry >= 0 && x+rx < XRES && y+ry < YRES && (rx || ry))
				{
					n = luacon_sim->pmap[y+ry][x+rx];
					if(n)
					{
						lua_pushinteger(l, n>>8);
						lua_rawseti(l, -2, id++);
					}
				}
	}
	return 1;
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

int LuaScriptInterface::simulation_partID(lua_State * l)
{
	int x = lua_tointeger(l, 1);
	int y = lua_tointeger(l, 2);

	if(x < 0 || x >= XRES || y < 0 || y >= YRES)
	{
		lua_pushnil(l);
		return 1;
	}

	int amalgam = luacon_sim->pmap[y][x];
	if(!amalgam)
		amalgam = luacon_sim->photons[y][x];
	if (!amalgam)
		lua_pushnil(l);
	else
		lua_pushinteger(l, amalgam >> 8);
	return 1;
}

int LuaScriptInterface::simulation_partPosition(lua_State * l)
{
	int particleID = lua_tointeger(l, 1);
	int argCount = lua_gettop(l);
	if(particleID < 0 || particleID >= NPART || !luacon_sim->parts[particleID].type)
	{
		if(argCount == 1)
		{
			lua_pushnil(l);
			lua_pushnil(l);
			return 2;
		} else {
			return 0;
		}
	}
	
	if(argCount == 3)
	{
		luacon_sim->parts[particleID].x = lua_tonumber(l, 2);
		luacon_sim->parts[particleID].y = lua_tonumber(l, 3);
		return 0;
	}
	else
	{
		lua_pushnumber(l, luacon_sim->parts[particleID].x);
		lua_pushnumber(l, luacon_sim->parts[particleID].y);
		return 2;
	}
}

int LuaScriptInterface::simulation_partProperty(lua_State * l)
{
	int argCount = lua_gettop(l);
	int particleID = luaL_checkinteger(l, 1);
	StructProperty * property = NULL;

	if(particleID < 0 || particleID >= NPART || !luacon_sim->parts[particleID].type)
	{
		if(argCount == 3)
		{
			lua_pushnil(l);
			return 1;
		} else {
			return 0;
		}
	}

	//Get field
	if(lua_type(l, 2) == LUA_TNUMBER)
	{
		int fieldID = lua_tointeger(l, 2);
		if(fieldID < 0 || fieldID >= particlePropertiesCount)
			return luaL_error(l, "Invalid field ID (%d)", fieldID);
		property = &particleProperties[fieldID];
	} else if(lua_type(l, 2) == LUA_TSTRING) {
		std::string fieldName = lua_tostring(l, 2);
		for(int i = particlePropertiesCount-1; i >= 0; i--)
		{
			if(particleProperties[i].Name == fieldName)
				property = &particleProperties[i];
		}
		if(!property)
			return luaL_error(l, "Unknown field (%s)", fieldName.c_str());
	} else {
		return luaL_error(l, "Field ID must be an name (string) or identifier (integer)");
	}

	//Calculate memory address of property
	intptr_t propertyAddress = (intptr_t)(((unsigned char*)&luacon_sim->parts[particleID])+property->Offset);

	if(argCount == 3)
	{
		//Set
		switch(property->Type)
		{
			case StructProperty::ParticleType:
			case StructProperty::Integer:
				*((int*)propertyAddress) = lua_tointeger(l, 3);
				break;
			case StructProperty::UInteger:
				*((unsigned int*)propertyAddress) = lua_tointeger(l, 3);
				break;
			case StructProperty::Float:
				*((float*)propertyAddress) = lua_tonumber(l, 3);
				break;
			case StructProperty::Char:
				*((char*)propertyAddress) = lua_tointeger(l, 3);
				break;
			case StructProperty::UChar:
				*((unsigned char*)propertyAddress) = lua_tointeger(l, 3);
				break;
			case StructProperty::String:
				*((char**)propertyAddress) = strdup(lua_tostring(l, 3));
				break;
			case StructProperty::Colour:
	#if PIXELSIZE == 4
				*((unsigned int*)propertyAddress) = lua_tointeger(l, 3);
	#else
				*((unsigned short*)propertyAddress) = lua_tointeger(l, 3);
	#endif
				break;
		}
		return 0;
	} 
	else
	{
		//Get
		switch(property->Type)
		{
			case StructProperty::ParticleType:
			case StructProperty::Integer:
				lua_pushinteger(l, *((int*)propertyAddress));
				break;
			case StructProperty::UInteger:
				lua_pushinteger(l, *((unsigned int*)propertyAddress));
				break;
			case StructProperty::Float:
				lua_pushnumber(l, *((float*)propertyAddress));
				break;
			case StructProperty::Char:
				lua_pushinteger(l, *((char*)propertyAddress));
				break;
			case StructProperty::UChar:
				lua_pushinteger(l, *((unsigned char*)propertyAddress));
				break;
			case StructProperty::String:
				lua_pushstring(l, *((char**)propertyAddress));
				break;
			case StructProperty::Colour:
	#if PIXELSIZE == 4
				lua_pushinteger(l, *((unsigned int*)propertyAddress));
	#else
				lua_pushinteger(l, *((unsigned short*)propertyAddress));
	#endif
				break;
			default:
				lua_pushnil(l);
		}
		return 1;
	}
}

int LuaScriptInterface::simulation_partKill(lua_State * l)
{
	if(lua_gettop(l)==2)
		luacon_sim->delete_part(lua_tointeger(l, 1), lua_tointeger(l, 2));
	else
	{
		int i = lua_tointeger(l, 1);
		if (i>=0 && i<NPART)
			luacon_sim->kill_part(i);
	}
	return 0;
}

int LuaScriptInterface::simulation_pressure(lua_State* l)
{
	int argCount = lua_gettop(l);
	luaL_checktype(l, 1, LUA_TNUMBER);
	luaL_checktype(l, 2, LUA_TNUMBER);
	int x = lua_tointeger(l, 1);
	int y = lua_tointeger(l, 2);
	if (x*CELL<0 || y*CELL<0 || x*CELL>=XRES || y*CELL>=YRES)
		return luaL_error(l, "coordinates out of range (%d,%d)", x, y);

	if (argCount == 2)
	{
		lua_pushnumber(l, luacon_sim->pv[y][x]);
		return 1;
	}
	int width = 1, height = 1;
	float value;
	luaL_checktype(l, 3, LUA_TNUMBER);
	if (argCount == 3)
		value = (float)lua_tonumber(l, 3);
	else
	{
		luaL_checktype(l, 4, LUA_TNUMBER);
		luaL_checktype(l, 5, LUA_TNUMBER);
		width = lua_tointeger(l, 3);
		height = lua_tointeger(l, 4);
		value = (float)lua_tonumber(l, 5);
	}
	if(value > 256.0f)
		value = 256.0f;
	else if(value < -256.0f)
		value = -256.0f;

	set_map(x, y, width, height, value, 1);
	return 0;
}

int LuaScriptInterface::simulation_ambientHeat(lua_State* l)
{
	int argCount = lua_gettop(l);
	luaL_checktype(l, 1, LUA_TNUMBER);
	luaL_checktype(l, 2, LUA_TNUMBER);
	int x = lua_tointeger(l, 1);
	int y = lua_tointeger(l, 2);
	if (x*CELL<0 || y*CELL<0 || x*CELL>=XRES || y*CELL>=YRES)
		return luaL_error(l, "coordinates out of range (%d,%d)", x, y);

	if (argCount == 2)
	{
		lua_pushnumber(l, luacon_sim->hv[y][x]);
		return 1;
	}
	int width = 1, height = 1;
	float value;
	luaL_checktype(l, 3, LUA_TNUMBER);
	if (argCount == 3)
		value = (float)lua_tonumber(l, 3);
	else
	{
		luaL_checktype(l, 4, LUA_TNUMBER);
		luaL_checktype(l, 5, LUA_TNUMBER);
		width = lua_tointeger(l, 3);
		height = lua_tointeger(l, 4);
		value = (float)lua_tonumber(l, 5);
	}
	if(value > MAX_TEMP)
		value = MAX_TEMP;
	else if(value < MIN_TEMP)
		value = MIN_TEMP;

	set_map(x, y, width, height, value, 2);
	return 0;
}

int LuaScriptInterface::simulation_velocityX(lua_State* l)
{
	int argCount = lua_gettop(l);
	luaL_checktype(l, 1, LUA_TNUMBER);
	luaL_checktype(l, 2, LUA_TNUMBER);
	int x = lua_tointeger(l, 1);
	int y = lua_tointeger(l, 2);
	if (x*CELL<0 || y*CELL<0 || x*CELL>=XRES || y*CELL>=YRES)
		return luaL_error(l, "coordinates out of range (%d,%d)", x, y);

	if (argCount == 2)
	{
		lua_pushnumber(l, luacon_sim->vx[y][x]);
		return 1;
	}
	int width = 1, height = 1;
	float value;
	luaL_checktype(l, 3, LUA_TNUMBER);
	if (argCount == 3)
		value = (float)lua_tonumber(l, 3);
	else
	{
		luaL_checktype(l, 4, LUA_TNUMBER);
		luaL_checktype(l, 5, LUA_TNUMBER);
		width = lua_tointeger(l, 3);
		height = lua_tointeger(l, 4);
		value = (float)lua_tonumber(l, 5);
	}
	if(value > 256.0f)
		value = 256.0f;
	else if(value < -256.0f)
		value = -256.0f;

	set_map(x, y, width, height, value, 3);
	return 0;
}

int LuaScriptInterface::simulation_velocityY(lua_State* l)
{
	int argCount = lua_gettop(l);
	luaL_checktype(l, 1, LUA_TNUMBER);
	luaL_checktype(l, 2, LUA_TNUMBER);
	int x = lua_tointeger(l, 1);
	int y = lua_tointeger(l, 2);
	if (x*CELL<0 || y*CELL<0 || x*CELL>=XRES || y*CELL>=YRES)
		return luaL_error(l, "coordinates out of range (%d,%d)", x, y);

	if (argCount == 2)
	{
		lua_pushnumber(l, luacon_sim->vy[y][x]);
		return 1;
	}
	int width = 1, height = 1;
	float value;
	luaL_checktype(l, 3, LUA_TNUMBER);
	if (argCount == 3)
		value = (float)lua_tonumber(l, 3);
	else
	{
		luaL_checktype(l, 4, LUA_TNUMBER);
		luaL_checktype(l, 5, LUA_TNUMBER);
		width = lua_tointeger(l, 3);
		height = lua_tointeger(l, 4);
		value = (float)lua_tonumber(l, 5);
	}
	if(value > 256.0f)
		value = 256.0f;
	else if(value < -256.0f)
		value = -256.0f;

	set_map(x, y, width, height, value, 4);
	return 0;
}

int LuaScriptInterface::simulation_gravMap(lua_State* l)
{
	int argCount = lua_gettop(l);
	luaL_checktype(l, 1, LUA_TNUMBER);
	luaL_checktype(l, 2, LUA_TNUMBER);
	int x = lua_tointeger(l, 1);
	int y = lua_tointeger(l, 2);
	if (x*CELL<0 || y*CELL<0 || x*CELL>=XRES || y*CELL>=YRES)
		return luaL_error(l, "coordinates out of range (%d,%d)", x, y);

	/*if (argCount == 2)
	{
		lua_pushnumber(l, luacon_sim->gravmap[y*XRES/CELL+x]);
		return 1;
	}*/
	int width = 1, height = 1;
	float value;
	luaL_checktype(l, 3, LUA_TNUMBER);
	if (argCount == 3)
		value = (float)lua_tonumber(l, 3);
	else
	{
		luaL_checktype(l, 4, LUA_TNUMBER);
		luaL_checktype(l, 5, LUA_TNUMBER);
		width = lua_tointeger(l, 3);
		height = lua_tointeger(l, 4);
		value = (float)lua_tonumber(l, 5);
	}

	set_map(x, y, width, height, value, 5);
	return 0;
}

int LuaScriptInterface::simulation_createParts(lua_State * l)
{
	int x = luaL_optint(l,1,-1);
	int y = luaL_optint(l,2,-1);
	int rx = luaL_optint(l,3,5);
	int ry = luaL_optint(l,4,5);
	int c = luaL_optint(l,5,luacon_model->GetActiveTool(0)->GetToolID());
	int brush = luaL_optint(l,6,CIRCLE_BRUSH);
	int flags = luaL_optint(l,7,luacon_sim->replaceModeFlags);

	vector<Brush*> brushList = luacon_model->GetBrushList();
	if (brush < 0 || brush >= brushList.size())
		return luaL_error(l, "Invalid brush id '%d'", brush);
	ui::Point tempRadius = brushList[brush]->GetRadius();
	brushList[brush]->SetRadius(ui::Point(rx, ry));

	int ret = luacon_sim->CreateParts(x, y, c, brushList[brush], flags);
	brushList[brush]->SetRadius(tempRadius);
	lua_pushinteger(l, ret);
	return 1;
}

int LuaScriptInterface::simulation_createLine(lua_State * l)
{
	int x1 = luaL_optint(l,1,-1);
	int y1 = luaL_optint(l,2,-1);
	int x2 = luaL_optint(l,3,-1);
	int y2 = luaL_optint(l,4,-1);
	int rx = luaL_optint(l,5,5);
	int ry = luaL_optint(l,6,5);
	int c = luaL_optint(l,7,luacon_model->GetActiveTool(0)->GetToolID());
	int brush = luaL_optint(l,8,CIRCLE_BRUSH);
	int flags = luaL_optint(l,9,luacon_sim->replaceModeFlags);

	vector<Brush*> brushList = luacon_model->GetBrushList();
	if (brush < 0 || brush >= brushList.size())
		return luaL_error(l, "Invalid brush id '%d'", brush);
	ui::Point tempRadius = brushList[brush]->GetRadius();
	brushList[brush]->SetRadius(ui::Point(rx, ry));

	luacon_sim->CreateLine(x1, y1, x2, y2, c, brushList[brush], flags);
	brushList[brush]->SetRadius(tempRadius);
	return 0;
}

int LuaScriptInterface::simulation_createBox(lua_State * l)
{
	int x1 = luaL_optint(l,1,-1);
	int y1 = luaL_optint(l,2,-1);
	int x2 = luaL_optint(l,3,-1);
	int y2 = luaL_optint(l,4,-1);
	int c = luaL_optint(l,5,luacon_model->GetActiveTool(0)->GetToolID());
	int flags = luaL_optint(l,6,luacon_sim->replaceModeFlags);

	luacon_sim->CreateBox(x1, y1, x2, y2, c, flags);
	return 0;
}

int LuaScriptInterface::simulation_floodParts(lua_State * l)
{
	int x = luaL_optint(l,1,-1);
	int y = luaL_optint(l,2,-1);
	int c = luaL_optint(l,3,luacon_model->GetActiveTool(0)->GetToolID());
	int cm = luaL_optint(l,4,-1);
	int flags = luaL_optint(l,5,luacon_sim->replaceModeFlags);
	int ret = luacon_sim->FloodParts(x, y, c, cm, flags);
	lua_pushinteger(l, ret);
	return 1;
}

int LuaScriptInterface::simulation_createWalls(lua_State * l)
{
	int x = luaL_optint(l,1,-1);
	int y = luaL_optint(l,2,-1);
	int rx = luaL_optint(l,3,0);
	int ry = luaL_optint(l,4,0);
	int c = luaL_optint(l,5,8);
	if (c < 0 || c >= UI_WALLCOUNT)
		return luaL_error(l, "Unrecognised wall id '%d'", c);

	int ret = luacon_sim->CreateWalls(x, y, rx, ry, c, NULL);
	lua_pushinteger(l, ret);
	return 1;
}

int LuaScriptInterface::simulation_createWallLine(lua_State * l)
{
	int x1 = luaL_optint(l,1,-1);
	int y1 = luaL_optint(l,2,-1);
	int x2 = luaL_optint(l,3,-1);
	int y2 = luaL_optint(l,4,-1);
	int rx = luaL_optint(l,5,0);
	int ry = luaL_optint(l,6,0);
	int c = luaL_optint(l,7,8);
	if (c < 0 || c >= UI_WALLCOUNT)
		return luaL_error(l, "Unrecognised wall id '%d'", c);

	luacon_sim->CreateWallLine(x1, y1, x2, y2, rx, ry, c, NULL);
	return 0;
}

int LuaScriptInterface::simulation_createWallBox(lua_State * l)
{
	int x1 = luaL_optint(l,1,-1);
	int y1 = luaL_optint(l,2,-1);
	int x2 = luaL_optint(l,3,-1);
	int y2 = luaL_optint(l,4,-1);
	int c = luaL_optint(l,5,8);
	if (c < 0 || c >= UI_WALLCOUNT)
		return luaL_error(l, "Unrecognised wall id '%d'", c);

	luacon_sim->CreateWallBox(x1, y1, x2, y2, c);
	return 0;
}

int LuaScriptInterface::simulation_floodWalls(lua_State * l)
{
	int x = luaL_optint(l,1,-1);
	int y = luaL_optint(l,2,-1);
	int c = luaL_optint(l,3,8);
	int bm = luaL_optint(l,4,-1);
	if (c < 0 || c >= UI_WALLCOUNT)
		return luaL_error(l, "Unrecognised wall id '%d'", c);
	int ret = luacon_sim->FloodWalls(x, y, c, bm);
	lua_pushinteger(l, ret);
	return 1;
}

int LuaScriptInterface::simulation_toolBrush(lua_State * l)
{
	int x = luaL_optint(l,1,-1);
	int y = luaL_optint(l,2,-1);
	int rx = luaL_optint(l,3,5);
	int ry = luaL_optint(l,4,5);
	int tool = luaL_optint(l,5,0);
	int brush = luaL_optint(l,6,CIRCLE_BRUSH);
	float strength = luaL_optnumber(l,7,1.0f);
	if (tool == luacon_sim->tools.size())
	{
		lua_pushinteger(l, 0);
		return 1;
	}
	else if (tool < 0 || tool >= luacon_sim->tools.size())
		return luaL_error(l, "Invalid tool id '%d'", tool);

	vector<Brush*> brushList = luacon_model->GetBrushList();
	if (brush < 0 || brush >= brushList.size())
		return luaL_error(l, "Invalid brush id '%d'", brush);
	ui::Point tempRadius = brushList[brush]->GetRadius();
	brushList[brush]->SetRadius(ui::Point(rx, ry));

	int ret = luacon_sim->ToolBrush(x, y, tool, brushList[brush], strength);
	brushList[brush]->SetRadius(tempRadius);
	lua_pushinteger(l, ret);
	return 1;
}

int LuaScriptInterface::simulation_toolLine(lua_State * l)
{
	int x1 = luaL_optint(l,1,-1);
	int y1 = luaL_optint(l,2,-1);
	int x2 = luaL_optint(l,3,-1);
	int y2 = luaL_optint(l,4,-1);
	int rx = luaL_optint(l,5,5);
	int ry = luaL_optint(l,6,5);
	int tool = luaL_optint(l,7,0);
	int brush = luaL_optint(l,8,CIRCLE_BRUSH);
	float strength = luaL_optnumber(l,9,1.0f);
	if (tool < 0 || tool >= luacon_sim->tools.size()+1)
		return luaL_error(l, "Invalid tool id '%d'", tool);

	vector<Brush*> brushList = luacon_model->GetBrushList();
	if (brush < 0 || brush >= brushList.size())
		return luaL_error(l, "Invalid brush id '%d'", brush);
	ui::Point tempRadius = brushList[brush]->GetRadius();
	brushList[brush]->SetRadius(ui::Point(rx, ry));

	if (tool == luacon_sim->tools.size())
	{
		Tool *WindTool = luacon_model->GetToolFromIdentifier("DEFAULT_UI_WIND");
		WindTool->DrawLine(luacon_sim, brushList[brush], ui::Point(x1, y1), ui::Point(x2, y2));
		return 1;
	}
	else
		luacon_sim->ToolLine(x1, y1, x2, y2, tool, brushList[brush], strength);
	brushList[brush]->SetRadius(tempRadius);
	return 0;
}

int LuaScriptInterface::simulation_toolBox(lua_State * l)
{
	int x1 = luaL_optint(l,1,-1);
	int y1 = luaL_optint(l,2,-1);
	int x2 = luaL_optint(l,3,-1);
	int y2 = luaL_optint(l,4,-1);
	int tool = luaL_optint(l,5,0);
	float strength = luaL_optnumber(l,6,1.0f);
	if (tool == luacon_sim->tools.size())
	{
		lua_pushinteger(l, 0);
		return 1;
	}
	else if (tool < 0 || tool >= luacon_sim->tools.size())
		return luaL_error(l, "Invalid tool id '%d'", tool);

	luacon_sim->ToolBox(x1, y1, x2, y2, tool, strength);
	return 0;
}

int LuaScriptInterface::simulation_decoBrush(lua_State * l)
{
	int x = luaL_optint(l,1,-1);
	int y = luaL_optint(l,2,-1);
	int rx = luaL_optint(l,3,5);
	int ry = luaL_optint(l,4,5);
	int r = luaL_optint(l,5,255);
	int g = luaL_optint(l,6,255);
	int b = luaL_optint(l,7,255);
	int a = luaL_optint(l,8,255);
	int tool = luaL_optint(l,9,DECO_DRAW);
	int brush = luaL_optint(l,10,CIRCLE_BRUSH);

	vector<Brush*> brushList = luacon_model->GetBrushList();
	if (brush < 0 || brush >= brushList.size())
		return luaL_error(l, "Invalid brush id '%d'", brush);
	ui::Point tempRadius = brushList[brush]->GetRadius();
	brushList[brush]->SetRadius(ui::Point(rx, ry));

	luacon_sim->ApplyDecorationPoint(x, y, r, g, b, a, tool, brushList[brush]);
	brushList[brush]->SetRadius(tempRadius);
	return 0;
}

int LuaScriptInterface::simulation_decoLine(lua_State * l)
{
	int x1 = luaL_optint(l,1,-1);
	int y1 = luaL_optint(l,2,-1);
	int x2 = luaL_optint(l,3,-1);
	int y2 = luaL_optint(l,4,-1);
	int rx = luaL_optint(l,5,5);
	int ry = luaL_optint(l,6,5);
	int r = luaL_optint(l,7,255);
	int g = luaL_optint(l,8,255);
	int b = luaL_optint(l,9,255);
	int a = luaL_optint(l,10,255);
	int tool = luaL_optint(l,11,DECO_DRAW);
	int brush = luaL_optint(l,12,CIRCLE_BRUSH);

	vector<Brush*> brushList = luacon_model->GetBrushList();
	if (brush < 0 || brush >= brushList.size())
		return luaL_error(l, "Invalid brush id '%d'", brush);
	ui::Point tempRadius = brushList[brush]->GetRadius();
	brushList[brush]->SetRadius(ui::Point(rx, ry));

	luacon_sim->ApplyDecorationLine(x1, y1, x2, y2, r, g, b, a, tool, brushList[brush]);
	brushList[brush]->SetRadius(tempRadius);
	return 0;
}

int LuaScriptInterface::simulation_decoBox(lua_State * l)
{
	int x1 = luaL_optint(l,1,-1);
	int y1 = luaL_optint(l,2,-1);
	int x2 = luaL_optint(l,3,-1);
	int y2 = luaL_optint(l,4,-1);
	int r = luaL_optint(l,5,255);
	int g = luaL_optint(l,6,255);
	int b = luaL_optint(l,7,255);
	int a = luaL_optint(l,8,255);
	int tool = luaL_optint(l,9,0);

	luacon_sim->ApplyDecorationBox(x1, y1, x2, y2, r, g, b, a, tool);
	return 0;
}

int LuaScriptInterface::simulation_decoColor(lua_State * l)
{
	int acount = lua_gettop(l);
	unsigned int color;
	if (acount == 0)
	{
		ui::Colour tempColor = luacon_model->GetColourSelectorColour();
		unsigned int color = (tempColor.Alpha << 24) | PIXRGB(tempColor.Red, tempColor.Green, tempColor.Blue);
		lua_pushnumber(l, color);
		return 1;
	}
	else if (acount == 1)
		color = (unsigned int)luaL_optnumber(l, 1, 0xFFFF0000);
	else
	{
		int r, g, b, a;
		r = luaL_optint(l, 1, 255);
		g = luaL_optint(l, 2, 255);
		b = luaL_optint(l, 3, 255);
		a = luaL_optint(l, 4, 255);

		if (r < 0) r = 0; if (r > 255) r = 255;
		if (g < 0) g = 0; if (g > 255) g = 255;
		if (b < 0) b = 0; if (b > 255) b = 255;
		if (a < 0) a = 0; if (a > 255) a = 255;

		color = (a << 24) + PIXRGB(r, g, b);
	}
	luacon_model->SetColourSelectorColour(ui::Colour(PIXR(color), PIXG(color), PIXB(color), color >> 24));
	return 0;
}

int LuaScriptInterface::simulation_clearSim(lua_State * l)
{
	luacon_sim->clear_sim();
	return 0;
}

int LuaScriptInterface::simulation_resetTemp(lua_State * l)
{
	bool onlyConductors = luaL_optint(l, 1, 0);
	for (int i = 0; i < luacon_sim->parts_lastActiveIndex; i++)
	{
		if (luacon_sim->parts[i].type && (luacon_sim->elements[luacon_sim->parts[i].type].HeatConduct || !onlyConductors))
		{
			luacon_sim->parts[i].temp = luacon_sim->elements[luacon_sim->parts[i].type].Temperature;
		}
	}
	return 0;
}

int LuaScriptInterface::simulation_resetPressure(lua_State * l)
{
	int aCount = lua_gettop(l), width = XRES/CELL, height = YRES/CELL;
	int x1 = abs(luaL_optint(l, 1, 0));
	int y1 = abs(luaL_optint(l, 2, 0));
	if (aCount > 2)
	{
		width = abs(luaL_optint(l, 3, XRES/CELL));
		height = abs(luaL_optint(l, 4, YRES/CELL));
	}
	else if (aCount)
	{
		width = 1;
		height = 1;
	}
	if(x1 > (XRES/CELL)-1)
		x1 = (XRES/CELL)-1;
	if(y1 > (YRES/CELL)-1)
		y1 = (YRES/CELL)-1;
	if(x1+width > (XRES/CELL)-1)
		width = (XRES/CELL)-x1;
	if(y1+height > (YRES/CELL)-1)
		height = (YRES/CELL)-y1;
	for (int nx = x1; nx<x1+width; nx++)
		for (int ny = y1; ny<y1+height; ny++)
		{
			luacon_sim->air->pv[ny][nx] = 0;
		}
	return 0;
}

int LuaScriptInterface::simulation_saveStamp(lua_State * l)
{
	int x = luaL_optint(l,1,0);
	int y = luaL_optint(l,2,0);
	int w = luaL_optint(l,3,XRES-1);
	int h = luaL_optint(l,4,YRES-1);
	std::string name = luacon_controller->StampRegion(ui::Point(x, y), ui::Point(x+w, y+h));
	lua_pushstring(l, name.c_str());
	return 1;
}

int LuaScriptInterface::simulation_loadStamp(lua_State * l)
{
	int i = -1, j, x, y;
	SaveFile * tempfile;
	x = luaL_optint(l,2,0);
	y = luaL_optint(l,3,0);
	if (lua_isstring(l, 1)) //Load from 10 char name, or full filename
	{
		const char * filename = luaL_optstring(l, 1, "");
		tempfile = Client::Ref().GetStamp(filename);
	}
	if (!tempfile && lua_isnumber(l, 1)) //Load from stamp ID
	{
		i = luaL_optint(l, 1, 0);
		int stampCount = Client::Ref().GetStampsCount();
		if (i < 0 || i >= stampCount)
			return luaL_error(l, "Invalid stamp ID: %d", i);
		tempfile = Client::Ref().GetStamp(Client::Ref().GetStamps(0, stampCount)[i]);
	}

	if (tempfile)
	{
		if (!luacon_sim->Load(x, y, tempfile->GetGameSave()))
		{
			//luacon_sim->sys_pause = (tempfile->GetGameSave()->paused | luacon_model->GetPaused())?1:0;
			lua_pushinteger(l, 1);
		}
		else
			lua_pushnil(l);
		delete tempfile;
	}
	else
		lua_pushnil(l);
	return 1;
}

int LuaScriptInterface::simulation_deleteStamp(lua_State * l)
{
	int stampCount = Client::Ref().GetStampsCount();
	std::vector<std::string> stamps = Client::Ref().GetStamps(0, stampCount);

	if (lua_isstring(l, 1)) //note: lua_isstring returns true on numbers too
	{
		const char * filename = luaL_optstring(l, 1, "");
		for (std::vector<std::string>::const_iterator iterator = stamps.begin(), end = stamps.end(); iterator != end; ++iterator)
		{
			if (*iterator == filename)
			{
				Client::Ref().DeleteStamp(*iterator);
				return 0;
			}
		}
	}
	if (lua_isnumber(l, 1)) //Load from stamp ID
	{
		int i = luaL_optint(l, 1, 0);
		if (i < 0 || i >= stampCount)
			return luaL_error(l, "Invalid stamp ID: %d", i);
		Client::Ref().DeleteStamp(stamps[i]);
		return 0;
	}
	lua_pushnumber(l, -1);
	return 1;
}

int LuaScriptInterface::simulation_loadSave(lua_State * l)
{
	int saveID = luaL_optint(l,1,0);
	int instant = luaL_optint(l,2,0);
	int history = luaL_optint(l,3,0); //Exact second a previous save was saved
	luacon_controller->OpenSavePreview(saveID, history, instant?true:false);
	return 0;
}

int LuaScriptInterface::simulation_reloadSave(lua_State * l)
{
	luacon_controller->ReloadSim();
	return 0;
}

int LuaScriptInterface::simulation_getSaveID(lua_State *l)
{
	SaveInfo *tempSave = luacon_model->GetSave();
	if (tempSave)
	{
		lua_pushinteger(l, tempSave->GetID());
		return 1;
	}
	return 0;
}

int LuaScriptInterface::simulation_adjustCoords(lua_State * l)
{
	int x = luaL_optint(l,1,0);
	int y = luaL_optint(l,2,0);
	ui::Point Coords = luacon_controller->PointTranslate(ui::Point(x, y));
	lua_pushinteger(l, Coords.X);
	lua_pushinteger(l, Coords.Y);
	return 2;
}

int LuaScriptInterface::simulation_prettyPowders(lua_State * l)
{
	int acount = lua_gettop(l);
	if (acount == 0)
	{
		lua_pushnumber(l, luacon_sim->pretty_powder);
		return 1;
	}
	int prettyPowder = luaL_optint(l, 1, 0);
	luacon_sim->pretty_powder = prettyPowder;
	luacon_model->UpdateQuickOptions();
	return 0;
}

int LuaScriptInterface::simulation_gravityGrid(lua_State * l)
{
	int acount = lua_gettop(l);
	if (acount == 0)
	{
		lua_pushnumber(l, luacon_model->GetGravityGrid());
		return 1;
	}
	int gravityGrid = luaL_optint(l, 1, 0);
	luacon_model->ShowGravityGrid(gravityGrid);
	luacon_model->UpdateQuickOptions();
	return 0;
}

int LuaScriptInterface::simulation_edgeMode(lua_State * l)
{
	int acount = lua_gettop(l);
	if (acount == 0)
	{
		lua_pushnumber(l, luacon_model->GetEdgeMode());
		return 1;
	}
	int edgeMode = luaL_optint(l, 1, 0);
	luacon_model->SetEdgeMode(edgeMode);
	return 0;
}

int LuaScriptInterface::simulation_gravityMode(lua_State * l)
{
	int acount = lua_gettop(l);
	if (acount == 0)
	{
		lua_pushnumber(l, luacon_sim->gravityMode);
		return 1;
	}
	int gravityMode = luaL_optint(l, 1, 0);
	luacon_sim->gravityMode = gravityMode;
	return 0;
}

int LuaScriptInterface::simulation_airMode(lua_State * l)
{
	int acount = lua_gettop(l);
	if (acount == 0)
	{
		lua_pushnumber(l, luacon_sim->air->airMode);
		return 1;
	}
	int airMode = luaL_optint(l, 1, 0);
	luacon_sim->air->airMode = airMode;
	return 0;
}

int LuaScriptInterface::simulation_waterEqualisation(lua_State * l)
{
	int acount = lua_gettop(l);
	if (acount == 0)
	{
		lua_pushnumber(l, luacon_sim->water_equal_test);
		return 1;
	}
	int waterMode = luaL_optint(l, 1, 0);
	luacon_sim->water_equal_test = waterMode;
	return 0;
}

int LuaScriptInterface::simulation_ambientAirTemp(lua_State * l)
{
	int acount = lua_gettop(l);
	if (acount == 0)
	{
		lua_pushnumber(l, luacon_sim->air->ambientAirTemp);
		return 1;
	}
	int ambientAirTemp = luaL_optint(l, 1, 295.15f);
	luacon_sim->air->ambientAirTemp = ambientAirTemp;
	return 0;
}

int LuaScriptInterface::simulation_elementCount(lua_State * l)
{
	int element = luaL_optint(l, 1, 0);
	if (element < 0 || element >= PT_NUM)
		return luaL_error(l, "Invalid element ID (%d)", element);

	lua_pushnumber(l, luacon_sim->elementCount[element]);
	return 1;
}

int LuaScriptInterface::simulation_canMove(lua_State * l)
{
	int movingElement = luaL_checkint(l, 1);
	int destinationElement = luaL_checkint(l, 2);
	if (movingElement < 0 || movingElement >= PT_NUM)
		return luaL_error(l, "Invalid element ID (%d)", movingElement);
	if (destinationElement < 0 || destinationElement >= PT_NUM)
		return luaL_error(l, "Invalid element ID (%d)", destinationElement);
	
	if (lua_gettop(l) < 3)
	{
		lua_pushnumber(l, luacon_sim->can_move[movingElement][destinationElement]);
		return 1;
	}
	else
	{
		luacon_sim->can_move[movingElement][destinationElement] = luaL_checkint(l, 3);
		return 0;
	}
}

int PartsClosure(lua_State * l)
{
	int i = lua_tointeger(l, lua_upvalueindex(1));
	do
	{
		if(i>=NPART)
			return 0;
		else
			i++;
	} while(!luacon_sim->parts[i].type);
	lua_pushnumber(l, i);
	lua_replace(l, lua_upvalueindex(1));
	lua_pushnumber(l, i);
	return 1;
}

int LuaScriptInterface::simulation_parts(lua_State * l)
{
	lua_pushnumber(l, -1);
	lua_pushcclosure(l, PartsClosure, 1);
	return 1;
}

int LuaScriptInterface::simulation_pmap(lua_State * l)
{
	int x=luaL_checkint(l, 1);
	int y=luaL_checkint(l, 2);
	int r;
	if(x < 0 || x >= XRES || y < 0 || y >= YRES)
		return luaL_error(l, "coordinates out of range (%d,%d)", x, y);
	r=luacon_sim->pmap[y][x];
	if(!(r&0xFF))
		return 0;
	lua_pushnumber(l, r>>8);
	return 1;
}


int NeighboursClosure(lua_State * l)
{
	int rx=lua_tointeger(l, lua_upvalueindex(1));
	int ry=lua_tointeger(l, lua_upvalueindex(2));
	int sx=lua_tointeger(l, lua_upvalueindex(3));
	int sy=lua_tointeger(l, lua_upvalueindex(4));
	int x=lua_tointeger(l, lua_upvalueindex(5));
	int y=lua_tointeger(l, lua_upvalueindex(6));
	int i = 0;
	do
	{
		x++;
		if(x>rx)
		{
			x=-rx;
			y++;
			if(y>ry)
				return 0;
		}
		if(!(x || y) || sx+x<0 || sy+y<0 || sx+x>=XRES*CELL || sy+y>=YRES*CELL)
		{
			continue;
		}
		i=luacon_sim->pmap[y+sy][x+sx];
	} while(!(i&0xFF));
	lua_pushnumber(l, x);
	lua_replace(l, lua_upvalueindex(5));
	lua_pushnumber(l, y);
	lua_replace(l, lua_upvalueindex(6));
	lua_pushnumber(l, i>>8);
	lua_pushnumber(l, x+sx);
	lua_pushnumber(l, y+sy);
	return 3;
}

int LuaScriptInterface::simulation_neighbours(lua_State * l)
{
	int x=luaL_checkint(l, 1);
	int y=luaL_checkint(l, 2);
	int rx=luaL_optint(l, 3, 2);
	int ry=luaL_optint(l, 4, 2);
	lua_pushnumber(l, rx);
	lua_pushnumber(l, ry);
	lua_pushnumber(l, x);
	lua_pushnumber(l, y);
	lua_pushnumber(l, -rx-1);
	lua_pushnumber(l, -ry);
	lua_pushcclosure(l, NeighboursClosure, 6);
	return 1;
}

//// Begin Renderer API

void LuaScriptInterface::initRendererAPI()
{
	//Methods
	struct luaL_reg rendererAPIMethods [] = {
		{"renderModes", renderer_renderModes},
		{"displayModes", renderer_displayModes},
		{"colourMode", renderer_colourMode},
		{"colorMode", renderer_colourMode}, //Duplicate of above to make Americans happy
		{"decorations", renderer_decorations}, //renderer_debugHUD
		{"grid", renderer_grid},
		{"debugHUD", renderer_debugHUD},
		{NULL, NULL}
	};
	luaL_register(l, "renderer", rendererAPIMethods);

	//Ren shortcut
	lua_getglobal(l, "renderer");
	lua_setglobal(l, "ren");

	//Static values
	//Particle pixel modes/fire mode/effects
	SETCONST(l, PMODE);
	SETCONST(l, PMODE_NONE);
	SETCONST(l, PMODE_FLAT);
	SETCONST(l, PMODE_BLOB);
	SETCONST(l, PMODE_BLUR);
	SETCONST(l, PMODE_GLOW);
	SETCONST(l, PMODE_SPARK);
	SETCONST(l, PMODE_FLARE);
	SETCONST(l, PMODE_LFLARE);
	SETCONST(l, PMODE_ADD);
	SETCONST(l, PMODE_BLEND);
	SETCONST(l, PSPEC_STICKMAN);
	SETCONST(l, OPTIONS);
	SETCONST(l, NO_DECO);
	SETCONST(l, DECO_FIRE);
	SETCONST(l, FIREMODE);
	SETCONST(l, FIRE_ADD);
	SETCONST(l, FIRE_BLEND);
	SETCONST(l, EFFECT);
	SETCONST(l, EFFECT_GRAVIN);
	SETCONST(l, EFFECT_GRAVOUT);
	SETCONST(l, EFFECT_LINES);
	SETCONST(l, EFFECT_DBGLINES);

	//Display/Render/Colour modes
	SETCONST(l, RENDER_EFFE);
	SETCONST(l, RENDER_FIRE);
	SETCONST(l, RENDER_GLOW);
	SETCONST(l, RENDER_BLUR);
	SETCONST(l, RENDER_BLOB);
	SETCONST(l, RENDER_BASC);
	SETCONST(l, RENDER_NONE);
	SETCONST(l, COLOUR_HEAT);
	SETCONST(l, COLOUR_LIFE);
	SETCONST(l, COLOUR_GRAD);
	SETCONST(l, COLOUR_BASC);
	SETCONST(l, COLOUR_DEFAULT);
	SETCONST(l, DISPLAY_AIRC);
	SETCONST(l, DISPLAY_AIRP);
	SETCONST(l, DISPLAY_AIRV);
	SETCONST(l, DISPLAY_AIRH);
	SETCONST(l, DISPLAY_AIR);
	SETCONST(l, DISPLAY_WARP);
	SETCONST(l, DISPLAY_PERS);
	SETCONST(l, DISPLAY_EFFE);
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

int LuaScriptInterface::renderer_grid(lua_State * l)
{
	int acount = lua_gettop(l);
	if (acount == 0)
	{
		lua_pushnumber(l, luacon_ren->GetGridSize());
		return 1;
	}
	int grid = luaL_optint(l, 1, -1);
	luacon_ren->SetGridSize(grid);
	return 0;
}

int LuaScriptInterface::renderer_debugHUD(lua_State * l)
{
	int acount = lua_gettop(l);
	if (acount == 0)
	{
		lua_pushnumber(l, luacon_controller->GetDebugHUD());
		return 1;
	}
	int debug = luaL_optint(l, 1, -1);
	luacon_controller->SetDebugHUD(debug);
	return 0;
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

	//Static values
	//Element types/properties/states
	SETCONST(l, TYPE_PART);
	SETCONST(l, TYPE_LIQUID);
	SETCONST(l, TYPE_SOLID);
	SETCONST(l, TYPE_GAS);
	SETCONST(l, TYPE_ENERGY);
	SETCONST(l, PROP_CONDUCTS);
	SETCONST(l, PROP_BLACK);
	SETCONST(l, PROP_NEUTPENETRATE);
	SETCONST(l, PROP_NEUTABSORB);
	SETCONST(l, PROP_NEUTPASS);
	SETCONST(l, PROP_DEADLY);
	SETCONST(l, PROP_HOT_GLOW);
	SETCONST(l, PROP_LIFE);
	SETCONST(l, PROP_RADIOACTIVE);
	SETCONST(l, PROP_LIFE_DEC);
	SETCONST(l, PROP_LIFE_KILL);
	SETCONST(l, PROP_LIFE_KILL_DEC);
	SETCONST(l, PROP_SPARKSETTLE);
	SETCONST(l, PROP_NOAMBHEAT);
	SETCONST(l, PROP_DRAWONCTYPE);
	SETCONST(l, PROP_NOCTYPEDRAW);
	SETCONST(l, FLAG_STAGNANT);
	SETCONST(l, FLAG_SKIPMOVE);
	SETCONST(l, FLAG_MOVABLE);
	SETCONST(l, ST_NONE);
	SETCONST(l, ST_SOLID);
	SETCONST(l, ST_LIQUID);
	SETCONST(l, ST_GAS);

	SETCONST(l, SC_WALL);
	SETCONST(l, SC_ELEC);
	SETCONST(l, SC_POWERED);
	SETCONST(l, SC_SENSOR);
	SETCONST(l, SC_FORCE);
	SETCONST(l, SC_EXPLOSIVE);
	SETCONST(l, SC_GAS);
	SETCONST(l, SC_LIQUID);
	SETCONST(l, SC_POWDERS);
	SETCONST(l, SC_SOLIDS);
	SETCONST(l, SC_NUCLEAR);
	SETCONST(l, SC_SPECIAL);
	SETCONST(l, SC_LIFE);
	SETCONST(l, SC_TOOL);
	SETCONST(l, SC_DECO);

	//Element identifiers
	for(int i = 0; i < PT_NUM; i++)
	{
		if(luacon_sim->elements[i].Enabled)
		{
			lua_pushinteger(l, i);
			lua_setfield(l, -2, luacon_sim->elements[i].Identifier);
			char realIdentifier[20];
			sprintf(realIdentifier, "DEFAULT_PT_%s", luacon_sim->elements[i].Name);
			if (i != 0 && i != PT_NBHL && i != PT_NWHL && strcmp(luacon_sim->elements[i].Identifier, realIdentifier))
			{
				lua_pushinteger(l, i);
				lua_setfield(l, -2, realIdentifier);
			}
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
			lua_el_mode[id] = 1;
		}
		else if(lua_type(l, -1) == LUA_TBOOLEAN && !lua_toboolean(l, -1))
		{
			lua_el_func[id] = 0;
			lua_el_mode[id] = 0;
			luacon_sim->elements[id].Update = NULL;
		}
		else
			lua_pop(l, 1);

		lua_getfield(l, -1, "Graphics");
		if(lua_type(l, -1) == LUA_TFUNCTION)
		{
			lua_gr_func[id] = luaL_ref(l, LUA_REGISTRYINDEX);
		}
		else if(lua_type(l, -1) == LUA_TBOOLEAN && !lua_toboolean(l, -1))
		{
			lua_gr_func[id] = 0;
			luacon_sim->elements[id].Graphics = NULL;
		}
		else
			lua_pop(l, 1);

		luacon_model->BuildMenus();
		luacon_sim->init_can_move();
		luacon_ren->graphicscache[id].isready = 0;

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
		lua_pushstring(l, luacon_sim->elements[id].Identifier);
		lua_setfield(l, -2, "Identifier");
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
			luacon_ren->graphicscache[id].isready = 0;

			return 0;
		}
		else if(propertyName == "Update")
		{
			if(lua_type(l, 3) == LUA_TFUNCTION)
			{
				lua_pushvalue(l, 3);
				lua_el_func[id] = luaL_ref(l, LUA_REGISTRYINDEX);
				if (args > 3)
				{
					luaL_checktype(l, 4, LUA_TNUMBER);
					int replace = lua_tointeger(l, 4);
					if (replace == 1)
						lua_el_mode[id] = 2;
					else
						lua_el_mode[id] = 1;
				}
				else
					lua_el_mode[id] = 1;
			}
			else if(lua_type(l, 3) == LUA_TBOOLEAN && !lua_toboolean(l, 3))
			{
				lua_el_func[id] = 0;
				lua_el_mode[id] = 0;
				luacon_sim->elements[id].Update = NULL;
			}
		}
		else if(propertyName == "Graphics")
		{
			if(lua_type(l, 3) == LUA_TFUNCTION)
			{
				lua_pushvalue(l, 3);
				lua_gr_func[id] = luaL_ref(l, LUA_REGISTRYINDEX);
			}
			else if(lua_type(l, 3) == LUA_TBOOLEAN && !lua_toboolean(l, -1))
			{
				lua_gr_func[id] = 0;
				luacon_sim->elements[id].Graphics = NULL;
			}
			luacon_ren->graphicscache[id].isready = 0;
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
		else if(propertyName == "Identifier")
		{
			lua_pushstring(l, luacon_sim->elements[id].Identifier);
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

void LuaScriptInterface::initGraphicsAPI()
{
	//Methods
	struct luaL_reg graphicsAPIMethods [] = {
		{"textSize", graphics_textSize},
		{"drawText", graphics_drawText},
		{"drawLine", graphics_drawLine},
		{"drawRect", graphics_drawRect},
		{"fillRect", graphics_fillRect},
		{"drawCircle", graphics_drawCircle},
		{"fillCircle", graphics_fillCircle},
		{NULL, NULL}
	};
	luaL_register(l, "graphics", graphicsAPIMethods);

	//elem shortcut
	lua_getglobal(l, "graphics");
	lua_setglobal(l, "gfx");

	lua_pushinteger(l, WINDOWW);	lua_setfield(l, -2, "WIDTH");
	lua_pushinteger(l, WINDOWH);	lua_setfield(l, -2, "HEIGHT");
}

int LuaScriptInterface::graphics_textSize(lua_State * l)
{
	int width, height;
	const char* text = luaL_optstring(l, 1, "");
	Graphics::textsize(text, width, height);

	lua_pushinteger(l, width);
	lua_pushinteger(l, height);
	return 2;
}

int LuaScriptInterface::graphics_drawText(lua_State * l)
{
	int x = lua_tointeger(l, 1);
	int y = lua_tointeger(l, 2);
	const char * text = luaL_optstring(l, 3, "");
	int r = luaL_optint(l, 4, 255);
	int g = luaL_optint(l, 5, 255);
	int b = luaL_optint(l, 6, 255);
	int a = luaL_optint(l, 7, 255);
	
	if (r<0) r = 0;
	else if (r>255) r = 255;
	if (g<0) g = 0;
	else if (g>255) g = 255;
	if (b<0) b = 0;
	else if (b>255) b = 255;
	if (a<0) a = 0;
	else if (a>255) a = 255;

	luacon_g->drawtext(x, y, text, r, g, b, a);
	return 0;
}

int LuaScriptInterface::graphics_drawLine(lua_State * l)
{
	int x1 = lua_tointeger(l, 1);
	int y1 = lua_tointeger(l, 2);
	int x2 = lua_tointeger(l, 3);
	int y2 = lua_tointeger(l, 4);
	int r = luaL_optint(l, 5, 255);
	int g = luaL_optint(l, 6, 255);
	int b = luaL_optint(l, 7, 255);
	int a = luaL_optint(l, 8, 255);

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

int LuaScriptInterface::graphics_drawRect(lua_State * l)
{
	int x = lua_tointeger(l, 1);
	int y = lua_tointeger(l, 2);
	int width = lua_tointeger(l, 3);
	int height = lua_tointeger(l, 4);
	int r = luaL_optint(l, 5, 255);
	int g = luaL_optint(l, 6, 255);
	int b = luaL_optint(l, 7, 255);
	int a = luaL_optint(l, 8, 255);

	if (r<0) r = 0;
	else if (r>255) r = 255;
	if (g<0) g = 0;
	else if (g>255) g = 255;
	if (b<0) b = 0;
	else if (b>255) b = 255;
	if (a<0) a = 0;
	else if (a>255) a = 255;

	luacon_g->drawrect(x, y, width, height, r, g, b, a);
	return 0;
}

int LuaScriptInterface::graphics_fillRect(lua_State * l)
{
	int x = lua_tointeger(l, 1);
	int y = lua_tointeger(l, 2);
	int width = lua_tointeger(l, 3);
	int height = lua_tointeger(l, 4);
	int r = luaL_optint(l, 5, 255);
	int g = luaL_optint(l, 6, 255);
	int b = luaL_optint(l, 7, 255);
	int a = luaL_optint(l, 8, 255);

	if (r<0) r = 0;
	else if (r>255) r = 255;
	if (g<0) g = 0;
	else if (g>255) g = 255;
	if (b<0) b = 0;
	else if (b>255) b = 255;
	if (a<0) a = 0;
	else if (a>255) a = 255;

	luacon_g->fillrect(x, y, width, height, r, g, b, a);
	return 0;
}

int LuaScriptInterface::graphics_drawCircle(lua_State * l)
{
	int x = lua_tointeger(l, 1);
	int y = lua_tointeger(l, 2);
	int rx = lua_tointeger(l, 3);
	int ry = lua_tointeger(l, 4);
	int r = luaL_optint(l, 5, 255);
	int g = luaL_optint(l, 6, 255);
	int b = luaL_optint(l, 7, 255);
	int a = luaL_optint(l, 8, 255);

	if (r<0) r = 0;
	else if (r>255) r = 255;
	if (g<0) g = 0;
	else if (g>255) g = 255;
	if (b<0) b = 0;
	else if (b>255) b = 255;
	if (a<0) a = 0;
	else if (a>255) a = 255;

	luacon_g->drawcircle(x, y, abs(rx), abs(ry), r, g, b, a);
	return 0;
}

int LuaScriptInterface::graphics_fillCircle(lua_State * l)
{
	int x = lua_tointeger(l, 1);
	int y = lua_tointeger(l, 2);
	int rx = lua_tointeger(l, 3);
	int ry = lua_tointeger(l, 4);
	int r = luaL_optint(l, 5, 255);
	int g = luaL_optint(l, 6, 255);
	int b = luaL_optint(l, 7, 255);
	int a = luaL_optint(l, 8, 255);

	if (r<0) r = 0;
	else if (r>255) r = 255;
	if (g<0) g = 0;
	else if (g>255) g = 255;
	if (b<0) b = 0;
	else if (b>255) b = 255;
	if (a<0) a = 0;
	else if (a>255) a = 255;

	luacon_g->fillcircle(x, y, abs(rx), abs(ry), r, g, b, a);
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

	bool isFile = false;
#ifdef WIN
	struct _stat s;
	if(_stat(filename, &s) == 0)
#else
	struct stat s;
	if(stat(filename, &s) == 0)
#endif
	{
		if(s.st_mode & S_IFREG)
		{
			isFile = true; //Is file
		}
		else
		{
			isFile = false; //Is directory or something else
		}
	}
	else
	{
		isFile = false; //Doesn't exist
	}

	lua_pushboolean(l, isFile);
	return 1;
}

int LuaScriptInterface::fileSystem_isDirectory(lua_State * l)
{
	const char * filename = lua_tostring(l, 1);

	bool isDir = false;
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
			isDir = true; //Is directory
		}
		else
		{
			isDir = false; //Is file or something else
		}
	}
	else
	{
		isDir = false; //Doesn't exist
	}

	lua_pushboolean(l, isDir);
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

bool LuaScriptInterface::OnActiveToolChanged(int toolSelection, Tool * tool)
{
	if (toolSelection == 0)
		luacon_selectedl = tool->GetIdentifier();
	else if (toolSelection == 1)
		luacon_selectedr = tool->GetIdentifier();
	else if (toolSelection == 2)
		luacon_selectedalt = tool->GetIdentifier();
	else if (toolSelection == 3)
		luacon_selectedreplace = tool->GetIdentifier();
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
	if (button == 3)
		button = 4;
	luacon_mousedown = true;
	luacon_mousebutton = button;
	return luacon_mouseevent(x, y, button, LUACON_MDOWN, 0);
}

bool LuaScriptInterface::OnMouseUp(int x, int y, unsigned button)
{
	if (button == 3)
		button = 4;
	luacon_mousedown = false;
	luacon_mousebutton = 0;
	return luacon_mouseevent(x, y, button, LUACON_MUP, 0);
}

bool LuaScriptInterface::OnMouseWheel(int x, int y, int d)
{
	return luacon_mouseevent(x, y, luacon_mousedown?luacon_mousebutton:0, 0, d);
}

bool LuaScriptInterface::OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	return luacon_keyevent(key, GetModifiers(), LUACON_KDOWN);
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
	lua_getglobal(l, "simulation");
	lua_pushinteger(l, luacon_sim->NUM_PARTS); lua_setfield(l, -2, "NUM_PARTS");
	lua_pop(l, 1);
	ui::Engine::Ref().LastTick(gettime());
	if(luacon_mousedown)
		luacon_mouseevent(luacon_mousex, luacon_mousey, luacon_mousebutton, LUACON_MPRESS, 0);
	luacon_step(luacon_mousex, luacon_mousey, luacon_selectedl, luacon_selectedr, luacon_selectedalt, luacon_selectedreplace, luacon_brushx, luacon_brushy);
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
		int level = lua_gettop(l), ret = -1;
		std::string text = "";
		lastError = "";
		currentCommand = true;
		if(lastCode.length())
			lastCode += "\n";
		lastCode += command;
		std::string tmp = "return " + lastCode;
		ui::Engine::Ref().LastTick(gettime());
		luaL_loadbuffer(l, tmp.c_str(), tmp.length(), "@console");
		if(lua_type(l, -1) != LUA_TFUNCTION)
		{
			lua_pop(l, 1);
			luaL_loadbuffer(l, lastCode.c_str(), lastCode.length(), "@console");
		}
		if(lua_type(l, -1) != LUA_TFUNCTION)
		{
			lastError = luacon_geterror();
			if(std::string(lastError).find("near '<eof>'")!=-1) //the idea stolen from lua-5.1.5/lua.c
				lastError = "...";
			else
				lastCode = "";
		}
		else
		{
			lastCode = "";
			ret = lua_pcall(l, 0, LUA_MULTRET, 0);
			if(ret)
				lastError = luacon_geterror();
			else
			{
				for(level++;level<=lua_gettop(l);level++)
				{
					luaL_tostring(l, level);
					if(text.length())
						text += ", " + std::string(luaL_optstring(l, -1, ""));
					else
						text = std::string(luaL_optstring(l, -1, ""));
					lua_pop(l, 1);
				}
				if(text.length())
					if(lastError.length())
						lastError += "; " + text;
					else
						lastError = text;

			}
		}
		currentCommand = false;
		return ret;
	}
}

std::string LuaScriptInterface::FormatCommand(std::string command)
{
	if(command != "" && command[0] == '!')
	{
		return "!"+legacy->FormatCommand(command.substr(1));
	}
	else
		return command;
}

LuaScriptInterface::~LuaScriptInterface() {
	lua_close(l);
	delete legacy;
}
#endif
