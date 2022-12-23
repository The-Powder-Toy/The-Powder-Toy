#include "Config.h"
#ifdef LUACONSOLE

#include "client/http/Request.h" // includes curl.h, needs to come first to silence a warning on windows
#include "bzip2/bz2wrap.h"

#include "LuaScriptInterface.h"

#include <vector>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <sstream>

#include "Format.h"
#include "LuaScriptHelper.h"
#include "LuaLuna.h"
#include "LuaBit.h"
#include "LuaButton.h"
#include "LuaCheckbox.h"
#include "LuaEvents.h"
#include "LuaLabel.h"
#include "LuaProgressBar.h"
#include "LuaSlider.h"
#include "LuaTextbox.h"
#include "LuaWindow.h"
#include "LuaTCPSocket.h"
#include "LuaSDLKeys.h"
#include "PowderToy.h"
#include "TPTScriptInterface.h"

#include "client/Client.h"
#include "client/GameSave.h"
#include "client/SaveFile.h"
#include "client/SaveInfo.h"
#include "common/Platform.h"
#include "graphics/Graphics.h"
#include "graphics/Renderer.h"
#include "simulation/Air.h"
#include "simulation/ElementCommon.h"
#include "simulation/ElementClasses.h"
#include "simulation/ElementGraphics.h"
#include "simulation/GOLString.h"
#include "simulation/Simulation.h"
#include "simulation/ToolClasses.h"
#include "simulation/SaveRenderer.h"

#include "gui/interface/Window.h"
#include "gui/interface/Engine.h"
#include "gui/game/GameView.h"
#include "gui/game/GameController.h"
#include "gui/game/GameModel.h"
#include "gui/game/Tool.h"
#include "gui/game/Brush.h"

#ifndef WIN
#include <unistd.h>
#endif

extern "C"
{
#ifdef WIN
#include <direct.h>
#endif
#include <sys/stat.h>
}
#include "eventcompat.lua.h"

// idea from mniip, makes things much simpler
#define SETCONST(L, NAME)\
	lua_pushinteger(L, NAME);\
	lua_setfield(L, -2, #NAME)
#define SETCONSTF(L, NAME)\
	lua_pushnumber(L, NAME);\
	lua_setfield(L, -2, #NAME)

GameModel * luacon_model;
GameController * luacon_controller;
Simulation * luacon_sim;
LuaScriptInterface * luacon_ci;
Graphics * luacon_g;
Renderer * luacon_ren;

bool *luacon_currentCommand;
String *luacon_lastError;
bool luacon_hasLastError;
String lastCode;

int *lua_el_mode;
LuaSmartRef *lua_el_func, *lua_gr_func;
std::vector<LuaSmartRef> luaCtypeDrawHandlers, luaCreateHandlers, luaCreateAllowedHandlers, luaChangeTypeHandlers;

int getPartIndex_curIdx;
int tptProperties; //Table for some TPT properties
int tptPropertiesVersion;
int tptElements; //Table for TPT element names
int tptParts, tptPartsMeta, tptElementTransitions, tptPartsCData, tptPartMeta, cIndex;
LuaSmartRef *tptPart = nullptr;

int atPanic(lua_State *l)
{
	throw std::runtime_error("Unprotected lua panic: " + tpt_lua_toByteString(l, -1));
}

int TptIndexClosure(lua_State *l)
{
	LuaScriptInterface *lsi = (LuaScriptInterface *)lua_touserdata(l, lua_upvalueindex(1));
	return lsi->tpt_index(l);
}

int TptNewindexClosure(lua_State *l)
{
	LuaScriptInterface *lsi = (LuaScriptInterface *)lua_touserdata(l, lua_upvalueindex(1));
	return lsi->tpt_newIndex(l);
}

static int bz2_compress_wrapper(lua_State *L)
{
	auto src = tpt_lua_checkByteString(L, 1);
	auto maxSize = size_t(luaL_optinteger(L, 2, 0));
	std::vector<char> dest;
	auto result = BZ2WCompress(dest, src.data(), src.size(), maxSize);
#define RETURN_ERR(str) lua_pushnil(L); lua_pushinteger(L, int(result)); lua_pushliteral(L, str); return 3
	switch (result)
	{
	case BZ2WCompressOk: break;
	case BZ2WCompressNomem: RETURN_ERR("out of memory");
	case BZ2WCompressLimit: RETURN_ERR("size limit exceeded");
	}
#undef RETURN_ERR
	tpt_lua_pushByteString(L, ByteString(dest.begin(), dest.end()));
	return 1;
}

static int bz2_decompress_wrapper(lua_State *L)
{
	auto src = tpt_lua_checkByteString(L, 1);
	auto maxSize = size_t(luaL_optinteger(L, 2, 0));
	std::vector<char> dest;
	auto result = BZ2WDecompress(dest, src.data(), src.size(), maxSize);
#define RETURN_ERR(str) lua_pushnil(L); lua_pushinteger(L, int(result)); lua_pushliteral(L, str); return 3
	switch (result)
	{
	case BZ2WDecompressOk: break;
	case BZ2WDecompressNomem: RETURN_ERR("out of memory");
	case BZ2WDecompressLimit: RETURN_ERR("size limit exceeded");
	case BZ2WDecompressType:
	case BZ2WDecompressBad:
	case BZ2WDecompressEof: RETURN_ERR("corrupted stream");
	}
#undef RETURN_ERR
	tpt_lua_pushByteString(L, ByteString(dest.begin(), dest.end()));
	return 1;
}

static void initBZ2API(lua_State *L)
{
	luaL_Reg reg[] = {
		{ "compress", bz2_compress_wrapper },
		{ "decompress", bz2_decompress_wrapper },
		{ NULL, NULL },
	};
	lua_newtable(L);
	luaL_register(L, NULL, reg);
#define BZ2_CONST(k, v) lua_pushinteger(L, int(v)); lua_setfield(L, -2, k)
	BZ2_CONST("compressOk", BZ2WCompressOk);
	BZ2_CONST("compressNomem", BZ2WCompressNomem);
	BZ2_CONST("compressLimit", BZ2WCompressLimit);
	BZ2_CONST("decompressOk", BZ2WDecompressOk);
	BZ2_CONST("decompressNomem", BZ2WDecompressNomem);
	BZ2_CONST("decompressLimit", BZ2WDecompressLimit);
	BZ2_CONST("decompressType", BZ2WDecompressType);
	BZ2_CONST("decompressBad", BZ2WDecompressBad);
	BZ2_CONST("decompressEof", BZ2WDecompressEof);
#undef BZ2_CONST
	lua_setglobal(L, "bz2");
}

LuaScriptInterface::LuaScriptInterface(GameController * c, GameModel * m):
	CommandInterface(c, m),
	luacon_mousex(0),
	luacon_mousey(0),
	luacon_mousebutton(0),
	luacon_selectedl(""),
	luacon_selectedr(""),
	luacon_selectedalt(""),
	luacon_selectedreplace(""),
	luacon_mousedown(false),
	currentCommand(false),
	legacy(new TPTScriptInterface(c, m)),
	textInputRefcount(0)
{
	luacon_model = m;
	luacon_controller = c;
	luacon_sim = m->GetSimulation();
	luacon_g = ui::Engine::Ref().g;
	luacon_ren = m->GetRenderer();
	luacon_ci = this;

	for (auto moving = 0; moving < PT_NUM; ++moving)
	{
		for (auto into = 0; into < PT_NUM; ++into)
		{
			custom_can_move[moving][into] = 0;
		}
	}

	//New TPT API
	l = luaL_newstate();
	tpt_lua_setmainthread(l);
	lua_atpanic(l, atPanic);
	luaL_openlibs(l);
	luaopen_bit(l);

	lua_pushliteral(l, "Luacon_ci");
	lua_pushlightuserdata(l, this);
	lua_settable(l, LUA_REGISTRYINDEX);

	initSimulationAPI();
	initInterfaceAPI();
	SetWindow(c->GetView());
	initRendererAPI();
	initElementsAPI();
	initGraphicsAPI();
	initFileSystemAPI();
	initPlatformAPI();
	initEventAPI();
	initHttpAPI();
#ifndef NOHTTP
	initSocketAPI();
#endif

	initBZ2API(l);

	//Old TPT API
	int currentElementMeta, currentElement;
	const static struct luaL_Reg tptluaapi [] = {
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
		{"delete", &luatpt_delete},
		{"input", &luatpt_input},
		{"message_box", &luatpt_message_box},
		{"confirm", &luatpt_confirm},
		{"get_numOfParts", &luatpt_get_numOfParts},
		{"start_getPartIndex", &luatpt_start_getPartIndex},
		{"next_getPartIndex", &luatpt_next_getPartIndex},
		{"getPartIndex", &luatpt_getPartIndex},
		{"hud", &luatpt_hud},
		{"newtonian_gravity", &luatpt_gravity},
		{"ambient_heat", &luatpt_airheat},
		{"active_menu", &luatpt_active_menu},
		{"menu_enabled", &luatpt_menu_enabled},
		{"num_menus", &luatpt_num_menus},
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
		{"record",&luatpt_record},
		{"element",&luatpt_getelement},
		{"get_clipboard", &platform_clipboardCopy},
		{"set_clipboard", &platform_clipboardPaste},
		{"setdrawcap", &luatpt_setdrawcap},
		{"perfectCircleBrush", &luatpt_perfectCircle},
		{NULL,NULL}
	};

	luacon_mousedown = false;
	luacon_mousebutton = 0;

	luacon_currentCommand = &currentCommand;
	luacon_lastError = &lastError;
	luacon_hasLastError = false;

	lastCode = "";

	//Replace print function with our screen logging thingy
	lua_pushcfunction(l, luatpt_log);
	lua_setglobal(l, "print");

	//Register all tpt functions
	luaL_register(l, "tpt", tptluaapi);

	tptProperties = lua_gettop(l);

	lua_newtable(l);
	tptPropertiesVersion = lua_gettop(l);
	lua_pushinteger(l, SAVE_VERSION);
	lua_setfield(l, tptPropertiesVersion, "major");
	lua_pushinteger(l, MINOR_VERSION);
	lua_setfield(l, tptPropertiesVersion, "minor");
	lua_pushinteger(l, BUILD_NUM);
	lua_setfield(l, tptPropertiesVersion, "build");
#if defined(SNAPSHOT) || MOD_ID > 0
	lua_pushinteger(l, SNAPSHOT_ID);
#else
	lua_pushinteger(l, 0);
#endif
	lua_setfield(l, tptPropertiesVersion, "snapshot");
	lua_pushinteger(l, MOD_ID);
	lua_setfield(l, tptPropertiesVersion, "modid");
	lua_setfield(l, tptProperties, "version");

	lua_sethook(l, &luacon_hook, LUA_MASKCOUNT, 200);
#ifdef FFI
	//LuaJIT's ffi gives us direct access to parts data, no need for nested metatables. HOWEVER, this is in no way safe, it's entirely possible for someone to try to read parts[-10]
	lua_pushlightuserdata(l, parts);
	lua_setfield(l, tptProperties, "partsdata");

	tpt_lua_dostring (l, "ffi = require(\"ffi\")\n\
ffi.cdef[[\n\
typedef struct { int type; int life, ctype; float x, y, vx, vy; float temp; int tmp3; int tmp4; int flags; int tmp; int tmp2; unsigned int dcolour; } particle;\n\
]]\n\
tpt.parts = ffi.cast(\"particle *\", tpt.partsdata)\n\
ffi = nil\n\
tpt.partsdata = nil");
	//Since ffi is REALLY REALLY dangrous, we'll remove it from the environment completely (TODO)
	//lua_pushliteral(l, "parts");
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
	{
		int top = lua_gettop(l);
		lua_newtable(l);
		tptPartMeta = lua_gettop(l);
		lua_pushcfunction(l, luacon_partwrite);
		lua_setfield(l, tptPartMeta, "__newindex");
		lua_pushcfunction(l, luacon_partread);
		lua_setfield(l, tptPartMeta, "__index");
		lua_setmetatable(l, top);
	}

	tptPart = new LuaSmartRef(l);
	tptPart->Assign(l, -1);
	lua_pop(l, 1);
#endif

	lua_newtable(l);
	tptElements = lua_gettop(l);
	for (int i = 1; i < PT_NUM; i++)
	{
		tpt_lua_pushString(l, luacon_sim->elements[i].Name.ToLower());
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
		lua_settable(l, tptElements);
	}
	lua_setfield(l, tptProperties, "el");

	lua_newtable(l);
	tptElementTransitions = lua_gettop(l);
	for (int i = 1; i < PT_NUM; i++)
	{
		tpt_lua_pushString(l, luacon_sim->elements[i].Name.ToLower());
		lua_newtable(l);
		currentElement = lua_gettop(l);
		lua_newtable(l);
		currentElementMeta = lua_gettop(l);
		lua_pushinteger(l, i);
		lua_setfield(l, currentElement, "id");
		lua_pushcfunction(l, luacon_transitionwrite);
		lua_setfield(l, currentElementMeta, "__newindex");
		lua_pushcfunction(l, luacon_transitionread);
		lua_setfield(l, currentElementMeta, "__index");
		lua_setmetatable(l, currentElement);
		lua_settable(l, tptElementTransitions);
	}
	lua_setfield(l, tptProperties, "eltransition");

	lua_gr_func_v = std::vector<LuaSmartRef>(PT_NUM, l);
	lua_gr_func = &lua_gr_func_v[0];
	lua_el_func_v = std::vector<LuaSmartRef>(PT_NUM, l);
	lua_el_func = &lua_el_func_v[0];
	lua_el_mode_v = std::vector<int>(PT_NUM, 0);
	lua_el_mode = &lua_el_mode_v[0];

	luaCtypeDrawHandlers = std::vector<LuaSmartRef>(PT_NUM, l);
	luaCreateHandlers = std::vector<LuaSmartRef>(PT_NUM, l);
	luaCreateAllowedHandlers = std::vector<LuaSmartRef>(PT_NUM, l);
	luaChangeTypeHandlers = std::vector<LuaSmartRef>(PT_NUM, l);

	//make tpt.* a metatable
	lua_newtable(l);
	lua_pushlightuserdata(l, this);
	lua_pushcclosure(l, TptIndexClosure, 1);
	lua_setfield(l, -2, "__index");
	lua_pushlightuserdata(l, this);
	lua_pushcclosure(l, TptNewindexClosure, 1);
	lua_setfield(l, -2, "__newindex");
	lua_setmetatable(l, -2);

	initLegacyProps();

	ui::Engine::Ref().LastTick(Platform::GetTime());
	if (luaL_loadbuffer(l, (const char *)eventcompat_lua, eventcompat_lua_size, "@[built-in eventcompat.lua]") || lua_pcall(l, 0, 0, 0))
	{
		throw std::runtime_error(ByteString("failed to load built-in eventcompat: ") + tpt_lua_toByteString(l, -1));
	}
}

void LuaScriptInterface::custom_init_can_move()
{
	luacon_sim->init_can_move();
	for (auto moving = 0; moving < PT_NUM; ++moving)
	{
		for (auto into = 0; into < PT_NUM; ++into)
		{
			if (custom_can_move[moving][into] & 0x80)
			{
				luacon_sim->can_move[moving][into] = custom_can_move[moving][into] & 0x7F;
			}
		}
	}
}

void LuaScriptInterface::Init()
{
	if (Platform::FileExists("autorun.lua"))
	{
		lua_State *l = luacon_ci->l;
		if(luaL_loadfile(l, "autorun.lua") || lua_pcall(l, 0, 0, 0))
			luacon_ci->Log(CommandInterface::LogError, luacon_geterror());
		else
			luacon_ci->Log(CommandInterface::LogWarning, "Loaded autorun.lua");
	}
}

void LuaScriptInterface::SetWindow(ui::Window * window)
{
	Window = window;
}

int LuaScriptInterface::tpt_index(lua_State *l)
{
	ByteString key = tpt_lua_checkByteString(l, 2);
	if (byteStringEqualsLiteral(key, "mousex"))
		return lua_pushnumber(l, c->GetView()->GetMousePosition().X), 1;
	else if (byteStringEqualsLiteral(key, "mousey"))
		return lua_pushnumber(l, c->GetView()->GetMousePosition().Y), 1;
	else if (byteStringEqualsLiteral(key, "selectedl"))
		return tpt_lua_pushByteString(l, m->GetActiveTool(0)->GetIdentifier()), 1;
	else if (byteStringEqualsLiteral(key, "selectedr"))
		return tpt_lua_pushByteString(l, m->GetActiveTool(1)->GetIdentifier()), 1;
	else if (byteStringEqualsLiteral(key, "selecteda"))
		return tpt_lua_pushByteString(l, m->GetActiveTool(2)->GetIdentifier()), 1;
	else if (byteStringEqualsLiteral(key, "selectedreplace"))
		return tpt_lua_pushByteString(l, m->GetActiveTool(3)->GetIdentifier()), 1;
	else if (byteStringEqualsLiteral(key, "brushx"))
		return lua_pushnumber(l, m->GetBrush()->GetRadius().X), 1;
	else if (byteStringEqualsLiteral(key, "brushy"))
		return lua_pushnumber(l, m->GetBrush()->GetRadius().Y), 1;
	else if (byteStringEqualsLiteral(key, "brushID"))
		return lua_pushnumber(l, m->GetBrushID()), 1;
	else if (byteStringEqualsLiteral(key, "decoSpace"))
		return lua_pushnumber(l, m->GetDecoSpace()), 1;

	//if not a special key, return the value in the table
	return lua_rawget(l, 1), 1;
}

int LuaScriptInterface::tpt_newIndex(lua_State *l)
{
	ByteString key = tpt_lua_checkByteString(l, 2);
	if (byteStringEqualsLiteral(key, "selectedl"))
	{
		Tool *t = m->GetToolFromIdentifier(tpt_lua_checkByteString(l, 3));
		if (t)
			c->SetActiveTool(0, t);
		else
			luaL_error(l, "Invalid tool identifier: %s", lua_tostring(l, 3));
	}
	else if (byteStringEqualsLiteral(key, "selectedr"))
	{
		Tool *t = m->GetToolFromIdentifier(tpt_lua_checkByteString(l, 3));
		if (t)
			c->SetActiveTool(1, t);
		else
			luaL_error(l, "Invalid tool identifier: %s", lua_tostring(l, 3));
	}
	else if (byteStringEqualsLiteral(key, "selecteda"))
	{
		Tool *t = m->GetToolFromIdentifier(tpt_lua_checkByteString(l, 3));
		if (t)
			c->SetActiveTool(2, t);
		else
			luaL_error(l, "Invalid tool identifier: %s", lua_tostring(l, 3));
	}
	else if (byteStringEqualsLiteral(key, "selectedreplace"))
	{
		Tool *t = m->GetToolFromIdentifier(tpt_lua_checkByteString(l, 3));
		if( t)
			c->SetActiveTool(3, t);
		else
			luaL_error(l, "Invalid tool identifier: %s", lua_tostring(l, 3));
	}
	else if (byteStringEqualsLiteral(key, "brushx"))
	{
		int brushx = luaL_checkinteger(l, 3);
		if (brushx < 0 || brushx >= XRES)
			luaL_error(l, "Invalid brush width");

		c->SetBrushSize(ui::Point(brushx, m->GetBrush()->GetRadius().Y));
	}
	else if (byteStringEqualsLiteral(key, "brushy"))
	{
		int brushy = luaL_checkinteger(l, 3);
		if (brushy < 0 || brushy >= YRES)
			luaL_error(l, "Invalid brush height");

		c->SetBrushSize(ui::Point(m->GetBrush()->GetRadius().X, brushy));
	}
	else if (byteStringEqualsLiteral(key, "brushID"))
		m->SetBrushID(luaL_checkinteger(l, 3));
	else if (byteStringEqualsLiteral(key, "decoSpace"))
		m->SetDecoSpace(luaL_checkinteger(l, 3));
	else
	{
		//if not a special key, set a value in the table
		return lua_rawset(l, 1), 1;
	}
	return 0;
}

//// Begin Interface API

void LuaScriptInterface::initInterfaceAPI()
{
	struct luaL_Reg interfaceAPIMethods [] = {
		{"showWindow", interface_showWindow},
		{"closeWindow", interface_closeWindow},
		{"addComponent", interface_addComponent},
		{"removeComponent", interface_removeComponent},
		{"grabTextInput", interface_grabTextInput},
		{"dropTextInput", interface_dropTextInput},
		{"textInputRect", interface_textInputRect},
		{NULL, NULL}
	};
	luaL_register(l, "interface", interfaceAPIMethods);

	//Ren shortcut
	lua_getglobal(l, "interface");
	initLuaSDLKeys(l);
	lua_pushinteger(l, GameController::mouseUpNormal); lua_setfield(l, -2, "MOUSE_UP_NORMAL");
	lua_pushinteger(l, GameController::mouseUpBlur); lua_setfield(l, -2, "MOUSE_UP_BLUR");
	lua_pushinteger(l, GameController::mouseUpDrawEnd); lua_setfield(l, -2, "MOUSE_UP_DRAW_END");
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
	void *opaque = nullptr;
	LuaComponent *luaComponent = nullptr;
	if ((opaque = Luna<LuaButton>::tryGet(l, 1)))
		luaComponent = Luna<LuaButton>::get(opaque);
	else if ((opaque = Luna<LuaLabel>::tryGet(l, 1)))
		luaComponent = Luna<LuaLabel>::get(opaque);
	else if ((opaque = Luna<LuaTextbox>::tryGet(l, 1)))
		luaComponent = Luna<LuaTextbox>::get(opaque);
	else if ((opaque = Luna<LuaCheckbox>::tryGet(l, 1)))
		luaComponent = Luna<LuaCheckbox>::get(opaque);
	else if ((opaque = Luna<LuaSlider>::tryGet(l, 1)))
		luaComponent = Luna<LuaSlider>::get(opaque);
	else if ((opaque = Luna<LuaProgressBar>::tryGet(l, 1)))
		luaComponent = Luna<LuaProgressBar>::get(opaque);
	else
		luaL_typerror(l, 1, "Component");
	if (luacon_ci->Window && luaComponent)
	{
		auto ok = luacon_ci->grabbed_components.insert(std::make_pair(luaComponent, LuaSmartRef(l)));
		if (ok.second)
		{
			auto it = ok.first;
			it->second.Assign(l, 1);
			it->first->owner_ref = it->second;
		}
		luacon_ci->Window->AddComponent(luaComponent->GetComponent());
	}
	return 0;
}

int LuaScriptInterface::interface_removeComponent(lua_State * l)
{
	void *opaque = nullptr;
	LuaComponent *luaComponent = nullptr;
	if ((opaque = Luna<LuaButton>::tryGet(l, 1)))
		luaComponent = Luna<LuaButton>::get(opaque);
	else if ((opaque = Luna<LuaLabel>::tryGet(l, 1)))
		luaComponent = Luna<LuaLabel>::get(opaque);
	else if ((opaque = Luna<LuaTextbox>::tryGet(l, 1)))
		luaComponent = Luna<LuaTextbox>::get(opaque);
	else if ((opaque = Luna<LuaCheckbox>::tryGet(l, 1)))
		luaComponent = Luna<LuaCheckbox>::get(opaque);
	else if ((opaque = Luna<LuaSlider>::tryGet(l, 1)))
		luaComponent = Luna<LuaSlider>::get(opaque);
	else if ((opaque = Luna<LuaProgressBar>::tryGet(l, 1)))
		luaComponent = Luna<LuaProgressBar>::get(opaque);
	else
		luaL_typerror(l, 1, "Component");
	if(luacon_ci->Window && luaComponent)
	{
		ui::Component *component = luaComponent->GetComponent();
		luacon_ci->Window->RemoveComponent(component);
		auto it = luacon_ci->grabbed_components.find(luaComponent);
		if (it != luacon_ci->grabbed_components.end())
		{
			it->second.Clear();
			it->first->owner_ref = it->second;
			luacon_ci->grabbed_components.erase(it);
		}
	}
	return 0;
}

int LuaScriptInterface::interface_grabTextInput(lua_State * l)
{
	luacon_ci->textInputRefcount += 1;
	luacon_controller->GetView()->DoesTextInput = luacon_ci->textInputRefcount > 0;
	return 0;
}

int LuaScriptInterface::interface_dropTextInput(lua_State * l)
{
	luacon_ci->textInputRefcount -= 1;
	luacon_controller->GetView()->DoesTextInput = luacon_ci->textInputRefcount > 0;
	return 0;
}

int LuaScriptInterface::interface_textInputRect(lua_State * l)
{
	int x = luaL_checkint(l, 1);
	int y = luaL_checkint(l, 2);
	int w = luaL_checkint(l, 3);
	int h = luaL_checkint(l, 4);
	ui::Engine::Ref().TextInputRect(ui::Point{ x, y }, ui::Point{ w, h });
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
	if (window)
		window->GetWindow()->CloseActiveWindow();
	return 0;
}

//// Begin sim.signs API

int LuaScriptInterface::simulation_signIndex(lua_State *l)
{
	ByteString key = tpt_lua_checkByteString(l, 2);

	//Get Raw Index value for element. Maybe there is a way to get the sign index some other way?
	lua_pushliteral(l, "id");
	lua_rawget(l, 1);
	int id = lua_tointeger(l, lua_gettop(l))-1;

	if (id < 0 || id >= MAXSIGNS)
	{
		luaL_error(l, "Invalid sign ID (stop messing with things): %i", id);
		return 0;
	}
	if (id >= (int)luacon_sim->signs.size())
	{
		return lua_pushnil(l), 1;
	}

	int x, y, w, h;
	if (byteStringEqualsLiteral(key, "text"))
		return tpt_lua_pushString(l, luacon_sim->signs[id].text), 1;
	else if (byteStringEqualsLiteral(key, "displayText"))
		return tpt_lua_pushString(l, luacon_sim->signs[id].getDisplayText(luacon_sim, x, y, w, h, false)), 1;
	else if (byteStringEqualsLiteral(key, "justification"))
		return lua_pushnumber(l, (int)luacon_sim->signs[id].ju), 1;
	else if (byteStringEqualsLiteral(key, "x"))
		return lua_pushnumber(l, luacon_sim->signs[id].x), 1;
	else if (byteStringEqualsLiteral(key, "y"))
		return lua_pushnumber(l, luacon_sim->signs[id].y), 1;
	else if (byteStringEqualsLiteral(key, "screenX"))
	{
		luacon_sim->signs[id].getDisplayText(luacon_sim, x, y, w, h);
		lua_pushnumber(l, x);
		return 1;
	}
	else if (byteStringEqualsLiteral(key, "screenY"))
	{
		luacon_sim->signs[id].getDisplayText(luacon_sim, x, y, w, h);
		lua_pushnumber(l, y);
		return 1;
	}
	else if (byteStringEqualsLiteral(key, "width"))
	{
		luacon_sim->signs[id].getDisplayText(luacon_sim, x, y, w, h);
		lua_pushnumber(l, w);
		return 1;
	}
	else if (byteStringEqualsLiteral(key, "height"))
	{
		luacon_sim->signs[id].getDisplayText(luacon_sim, x, y, w, h);
		lua_pushnumber(l, h);
		return 1;
	}
	else
		return lua_pushnil(l), 1;
}

int LuaScriptInterface::simulation_signNewIndex(lua_State *l)
{
	ByteString key = tpt_lua_checkByteString(l, 2);

	//Get Raw Index value for element. Maybe there is a way to get the sign index some other way?
	lua_pushliteral(l, "id");
	lua_rawget(l, 1);
	int id = lua_tointeger(l, lua_gettop(l))-1;

	if (id < 0 || id >= MAXSIGNS)
	{
		luaL_error(l, "Invalid sign ID (stop messing with things)");
		return 0;
	}
	if (id >= (int)luacon_sim->signs.size())
	{
		luaL_error(l, "Sign doesn't exist");
	}

	if (byteStringEqualsLiteral(key, "text"))
	{
		auto temp = tpt_lua_checkString(l, 3);
		String cleaned = format::CleanString(temp, false, true, true).Substr(0, 45);
		if (!cleaned.empty())
			luacon_sim->signs[id].text = cleaned;
		else
			luaL_error(l, "Text is empty");
		return 0;
	}
	else if (byteStringEqualsLiteral(key, "justification"))
	{
		int ju = luaL_checkinteger(l, 3);
		if (ju >= 0 && ju <= 3)
			return luacon_sim->signs[id].ju = (sign::Justification)ju, 1;
		else
			luaL_error(l, "Invalid justification");
		return 0;
	}
	else if (byteStringEqualsLiteral(key, "x"))
	{
		int x = luaL_checkinteger(l, 3);
		if (x >= 0 && x < XRES)
			return luacon_sim->signs[id].x = x, 1;
		else
			luaL_error(l, "Invalid X coordinate");
		return 0;
	}
	else if (byteStringEqualsLiteral(key, "y"))
	{
		int y = luaL_checkinteger(l, 3);
		if (y >= 0 && y < YRES)
			return luacon_sim->signs[id].y = y, 1;
		else
			luaL_error(l, "Invalid Y coordinate");
		return 0;
	}
	else if (byteStringEqualsLiteral(key, "displayText") ||
	         byteStringEqualsLiteral(key, "screenX") ||
	         byteStringEqualsLiteral(key, "screenY") ||
	         byteStringEqualsLiteral(key, "width") ||
	         byteStringEqualsLiteral(key, "height"))
	{
		luaL_error(l, "That property can't be directly set");
	}
	return 0;
}

// Creates a new sign at the first open index
int LuaScriptInterface::simulation_newsign(lua_State *l)
{
	if (luacon_sim->signs.size() >= MAXSIGNS)
		return lua_pushnil(l), 1;

	String text = format::CleanString(tpt_lua_checkString(l, 1), false, true, true).Substr(0, 45);
	int x = luaL_checkinteger(l, 2);
	int y = luaL_checkinteger(l, 3);
	int ju = luaL_optinteger(l, 4, 1);
	if (ju < 0 || ju > 3)
		return luaL_error(l, "Invalid justification");
	if (x < 0 || x >= XRES)
		return luaL_error(l, "Invalid X coordinate");
	if (y < 0 || y >= YRES)
		return luaL_error(l, "Invalid Y coordinate");

	luacon_sim->signs.push_back(sign(text, x, y, (sign::Justification)ju));

	lua_pushinteger(l, luacon_sim->signs.size());
	return 1;
}

// Deletes a sign
int simulation_deletesign(lua_State *l)
{
	int signID = luaL_checkinteger(l, 1);
	if (signID <= 0 || signID > (int)luacon_sim->signs.size())
		return luaL_error(l, "Sign doesn't exist");

	luacon_sim->signs.erase(luacon_sim->signs.begin()+signID-1);
 	return 1;
 }

//// Begin Simulation API

void LuaScriptInterface::initSimulationAPI()
{
	//Methods
	struct luaL_Reg simulationAPIMethods [] = {
		{"partNeighbours", simulation_partNeighbours},
		{"partNeighbors", simulation_partNeighbours},
		{"partChangeType", simulation_partChangeType},
		{"partCreate", simulation_partCreate},
		{"partProperty", simulation_partProperty},
		{"partPosition", simulation_partPosition},
		{"partID", simulation_partID},
		{"partKill", simulation_partKill},
		{"partExists", simulation_partExists},
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
		{"floodDeco", simulation_floodDeco},
		{"clearSim", simulation_clearSim},
		{"clearRect", simulation_clearRect},
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
		{"customGravity", simulation_customGravity},
		{"airMode", simulation_airMode},
		{"waterEqualisation", simulation_waterEqualisation},
		{"waterEqualization", simulation_waterEqualisation},
		{"ambientAirTemp", simulation_ambientAirTemp},
		{"elementCount", simulation_elementCount},
		{"can_move", simulation_canMove},
		{"brush", simulation_brush},
		{"parts", simulation_parts},
		{"pmap", simulation_pmap},
		{"photons", simulation_photons},
		{"neighbours", simulation_neighbours},
		{"neighbors", simulation_neighbours},
		{"framerender", simulation_framerender},
		{"gspeed", simulation_gspeed},
		{"takeSnapshot", simulation_takeSnapshot},
		{"historyRestore", simulation_historyRestore},
		{"historyForward", simulation_historyForward},
		{"replaceModeFlags", simulation_replaceModeFlags},
		{"listCustomGol", simulation_listCustomGol},
		{"addCustomGol", simulation_addCustomGol},
		{"removeCustomGol", simulation_removeCustomGol},
		{"lastUpdatedID", simulation_lastUpdatedID},
		{"updateUpTo", simulation_updateUpTo},
		{NULL, NULL}
	};
	luaL_register(l, "simulation", simulationAPIMethods);

	//Sim shortcut
	lua_getglobal(l, "simulation");
	lua_setglobal(l, "sim");

	//Static values
	SETCONST(l, XRES);
	SETCONST(l, YRES);
	SETCONST(l, CELL);
	SETCONST(l, NT);
	SETCONST(l, ST);
	SETCONST(l, ITH);
	SETCONST(l, ITL);
	SETCONSTF(l, IPH);
	SETCONSTF(l, IPL);
	SETCONST(l, PT_NUM);
	lua_pushinteger(l, 0); lua_setfield(l, -2, "NUM_PARTS");
	SETCONST(l, R_TEMP);
	SETCONST(l, MAX_TEMP);
	SETCONST(l, MIN_TEMP);
	SETCONSTF(l, MAX_PRESSURE);
	SETCONSTF(l, MIN_PRESSURE);

	SETCONST(l, TOOL_HEAT);
	SETCONST(l, TOOL_COOL);
	SETCONST(l, TOOL_VAC);
	SETCONST(l, TOOL_AIR);
	SETCONST(l, TOOL_PGRV);
	SETCONST(l, TOOL_NGRV);
	SETCONST(l, TOOL_MIX);
	SETCONST(l, TOOL_CYCL);
	lua_pushinteger(l, luacon_sim->tools.size()); lua_setfield(l, -2, "TOOL_WIND");
	SETCONST(l, DECO_DRAW);
	SETCONST(l, DECO_CLEAR);
	SETCONST(l, DECO_ADD);
	SETCONST(l, DECO_SUBTRACT);
	SETCONST(l, DECO_MULTIPLY);
	SETCONST(l, DECO_DIVIDE);
	SETCONST(l, DECO_SMUDGE);

	SETCONST(l, PMAPBITS);
	SETCONST(l, PMAPMASK);

	//Declare FIELD_BLAH constants
	{
		int particlePropertiesCount = 0;
		for (auto &prop : Particle::GetProperties())
		{
			tpt_lua_pushByteString(l, "FIELD_" + prop.Name.ToUpper());
			lua_pushinteger(l, particlePropertiesCount++);
			lua_settable(l, -3);
		}
		for (auto &alias : Particle::GetPropertyAliases())
		{
			tpt_lua_pushByteString(l, "FIELD_" + alias.from.ToUpper());
			tpt_lua_pushByteString(l, "FIELD_" + alias.to.ToUpper());
			lua_gettable(l, -3);
			lua_settable(l, -3);
		}
	}

	lua_newtable(l);
	for (int i = 1; i <= MAXSIGNS; i++)
	{
		lua_newtable(l);
		lua_pushinteger(l, i); //set "id" to table index
		lua_setfield(l, -2, "id");
		lua_newtable(l);
		lua_pushcfunction(l, simulation_signIndex);
		lua_setfield(l, -2, "__index");
		lua_pushcfunction(l, simulation_signNewIndex);
		lua_setfield(l, -2, "__newindex");
		lua_setmetatable(l, -2);
		lua_pushinteger(l, i); //table index
		lua_insert(l, -2); //swap k and v
		lua_settable(l, -3); //set metatable to signs[i]
	}
	lua_pushcfunction(l, simulation_newsign);
	lua_setfield(l, -2, "new");
	lua_pushcfunction(l, simulation_deletesign);
	lua_setfield(l, -2, "delete");
	lua_setfield(l, -2, "signs");
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
					if (!n || TYP(n) != t)
						n = luacon_sim->photons[y+ry][x+rx];
					if (n && TYP(n) == t)
					{
						lua_pushinteger(l, ID(n));
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
					if (!n)
						n = luacon_sim->photons[y+ry][x+rx];
					if (n)
					{
						lua_pushinteger(l, ID(n));
						lua_rawseti(l, -2, id++);
					}
				}
	}
	return 1;
}

int LuaScriptInterface::simulation_partChangeType(lua_State * l)
{
	int partIndex = lua_tointeger(l, 1);
	if(partIndex < 0 || partIndex >= NPART || !luacon_sim->parts[partIndex].type)
		return 0;
	luacon_sim->part_change_type(partIndex, int(luacon_sim->parts[partIndex].x+0.5f), int(luacon_sim->parts[partIndex].y+0.5f), lua_tointeger(l, 2));
	return 0;
}

int LuaScriptInterface::simulation_partCreate(lua_State * l)
{
	int newID = lua_tointeger(l, 1);
	if (newID >= NPART || newID < -3)
	{
		lua_pushinteger(l, -1);
		return 1;
	}
	if (newID >= 0 && !luacon_sim->parts[newID].type)
	{
		lua_pushinteger(l, -1);
		return 1;
	}
	int type = lua_tointeger(l, 4);
	int v = -1;
	if (lua_gettop(l) >= 5)
	{
		v = lua_tointeger(l, 5);
	}
	else if (ID(type))
	{
		v = ID(type);
		type = TYP(type);
	}
	lua_pushinteger(l, luacon_sim->create_part(newID, lua_tointeger(l, 2), lua_tointeger(l, 3), type, v));
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
		lua_pushinteger(l, ID(amalgam));
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
	StructProperty property;

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

	auto &properties = Particle::GetProperties();
	auto prop = properties.end();

	//Get field
	if (lua_type(l, 2) == LUA_TNUMBER)
	{
		int fieldID = lua_tointeger(l, 2);
		if (fieldID < 0 || fieldID >= (int)properties.size())
			return luaL_error(l, "Invalid field ID (%d)", fieldID);
		prop = properties.begin() + fieldID;
	}
	else if (lua_type(l, 2) == LUA_TSTRING)
	{
		ByteString fieldName = tpt_lua_toByteString(l, 2);
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
			return luaL_error(l, "Unknown field (%s)", fieldName.c_str());
	}
	else
	{
		return luaL_error(l, "Field ID must be an name (string) or identifier (integer)");
	}

	//Calculate memory address of property
	intptr_t propertyAddress = (intptr_t)(((unsigned char*)&luacon_sim->parts[particleID]) + prop->Offset);

	if(argCount == 3)
	{
		if (prop == properties.begin() + 0) // i.e. it's .type
		{
			luacon_sim->part_change_type(particleID, int(luacon_sim->parts[particleID].x+0.5f), int(luacon_sim->parts[particleID].y+0.5f), luaL_checkinteger(l, 3));
		}
		else
		{
			LuaSetProperty(l, *prop, propertyAddress, 3);
		}
		return 0;
	}
	else
	{
		LuaGetProperty(l, *prop, propertyAddress);
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

int LuaScriptInterface::simulation_partExists(lua_State * l)
{
	int i = luaL_checkinteger(l, 1);
	lua_pushboolean(l, i >= 0 && i < NPART && luacon_sim->parts[i].type);
	return 1;
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
	if(value > MAX_PRESSURE)
		value = MAX_PRESSURE;
	else if(value < MIN_PRESSURE)
		value = MIN_PRESSURE;

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
	if(value > MAX_PRESSURE)
		value = MAX_PRESSURE;
	else if(value < MIN_PRESSURE)
		value = MIN_PRESSURE;

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
	if(value > MAX_PRESSURE)
		value = MAX_PRESSURE;
	else if(value < MIN_PRESSURE)
		value = MIN_PRESSURE;

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

	if (argCount == 2)
	{
		lua_pushnumber(l, luacon_sim->gravp[y*XRES/CELL+x]);
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

	std::vector<Brush*> brushList = luacon_model->GetBrushList();
	if (brush < 0 || brush >= (int)brushList.size())
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

	std::vector<Brush*> brushList = luacon_model->GetBrushList();
	if (brush < 0 || brush >= (int)brushList.size())
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
	
	if (x < 0 || x >= XRES || y < 0 || y >= YRES)
		return luaL_error(l, "coordinates out of range (%d,%d)", x, y);
	
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

	if (x < 0 || x >= XRES || y < 0 || y >= YRES)
		return luaL_error(l, "coordinates out of range (%d,%d)", x, y);
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

	if (x1 < 0 || x2 < 0 || x1 >= XRES || x2 >= XRES || y1 < 0 || y2 < 0 || y1 >= YRES || y2 >= YRES)
		return luaL_error(l, "coordinates out of range (%d,%d),(%d,%d)", x1, y1, x2, y2);
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

	if (x1 < 0 || x2 < 0 || x1 >= XRES || x2 >= XRES || y1 < 0 || y2 < 0 || y1 >= YRES || y2 >= YRES)
		return luaL_error(l, "coordinates out of range (%d,%d),(%d,%d)", x1, y1, x2, y2);
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
	if (x < 0 || x >= XRES || y < 0 || y >= YRES)
		return luaL_error(l, "coordinates out of range (%d,%d)", x, y);
	if (c < 0 || c >= UI_WALLCOUNT)
		return luaL_error(l, "Unrecognised wall id '%d'", c);
	if (c == WL_STREAM)
	{
		lua_pushinteger(l, 0);
		return 1;
	}
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
	if (tool == (int)luacon_sim->tools.size())
	{
		lua_pushinteger(l, 0);
		return 1;
	}
	else if (tool < 0 || tool > (int)luacon_sim->tools.size())
		return luaL_error(l, "Invalid tool id '%d'", tool);

	std::vector<Brush*> brushList = luacon_model->GetBrushList();
	if (brush < 0 || brush >= (int)brushList.size())
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
	
	if (x1 < 0 || x2 < 0 || x1 >= XRES || x2 >= XRES || y1 < 0 || y2 < 0 || y1 >= YRES || y2 >= YRES)
		return luaL_error(l, "coordinates out of range (%d,%d),(%d,%d)", x1, y1, x2, y2);
	if (tool < 0 || tool >= (int)luacon_sim->tools.size()+1)
		return luaL_error(l, "Invalid tool id '%d'", tool);

	std::vector<Brush*> brushList = luacon_model->GetBrushList();
	if (brush < 0 || brush >= (int)brushList.size())
		return luaL_error(l, "Invalid brush id '%d'", brush);
	ui::Point tempRadius = brushList[brush]->GetRadius();
	brushList[brush]->SetRadius(ui::Point(rx, ry));

	if (tool == (int)luacon_sim->tools.size())
	{
		Tool *windTool = luacon_model->GetToolFromIdentifier("DEFAULT_UI_WIND");
		float oldStrength = windTool->GetStrength();
		windTool->SetStrength(strength);
		windTool->DrawLine(luacon_sim, brushList[brush], ui::Point(x1, y1), ui::Point(x2, y2));
		windTool->SetStrength(oldStrength);
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
	if (x1 < 0 || x2 < 0 || x1 >= XRES || x2 >= XRES || y1 < 0 || y2 < 0 || y1 >= YRES || y2 >= YRES)
		return luaL_error(l, "coordinates out of range (%d,%d),(%d,%d)", x1, y1, x2, y2);
	int tool = luaL_optint(l,5,0);
	float strength = luaL_optnumber(l,6,1.0f);
	if (tool == (int)luacon_sim->tools.size())
	{
		lua_pushinteger(l, 0);
		return 1;
	}
	else if (tool < 0 || tool >= (int)luacon_sim->tools.size())
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

	std::vector<Brush*> brushList = luacon_model->GetBrushList();
	if (brush < 0 || brush >= (int)brushList.size())
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

	if (x1 < 0 || x2 < 0 || x1 >= XRES || x2 >= XRES || y1 < 0 || y2 < 0 || y1 >= YRES || y2 >= YRES)
		return luaL_error(l, "coordinates out of range (%d,%d),(%d,%d)", x1, y1, x2, y2);

	std::vector<Brush*> brushList = luacon_model->GetBrushList();
	if (brush < 0 || brush >= (int)brushList.size())
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

	if (x1 < 0 || x2 < 0 || x1 >= XRES || x2 >= XRES || y1 < 0 || y2 < 0 || y1 >= YRES || y2 >= YRES)
		return luaL_error(l, "coordinates out of range (%d,%d),(%d,%d)", x1, y1, x2, y2);

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

		if (r < 0) r = 0;
		if (r > 255) r = 255;
		if (g < 0) g = 0;
		if (g > 255) g = 255;
		if (b < 0) b = 0;
		if (b > 255) b = 255;
		if (a < 0) a = 0;
		if (a > 255) a = 255;

		color = (a << 24) + PIXRGB(r, g, b);
	}
	luacon_model->SetColourSelectorColour(ui::Colour(PIXR(color), PIXG(color), PIXB(color), color >> 24));
	return 0;
}

int LuaScriptInterface::simulation_floodDeco(lua_State * l)
{
	int x = luaL_checkinteger(l, 1);
	int y = luaL_checkinteger(l, 2);
	int r = luaL_checkinteger(l, 3);
	int g = luaL_checkinteger(l, 4);
	int b = luaL_checkinteger(l, 5);
	int a = luaL_checkinteger(l, 6);

	if (x < 0 || x >= XRES || y < 0 || y >= YRES)
		return luaL_error(l, "coordinates out of range (%d,%d)", x, y);

	// hilariously broken, intersects with console and all Lua graphics
	pixel loc = luacon_ren->vid[x + y * WINDOWW];
	luacon_sim->ApplyDecorationFill(luacon_ren, x, y, r, g, b, a, PIXR(loc), PIXG(loc), PIXB(loc));
	return 0;
}

int LuaScriptInterface::simulation_clearSim(lua_State * l)
{
	luacon_controller->ClearSim();
	return 0;
}

int LuaScriptInterface::simulation_clearRect(lua_State * l)
{
	int x = luaL_checkint(l,1);
	int y = luaL_checkint(l,2);
	int w = luaL_checkint(l,3)-1;
	int h = luaL_checkint(l,4)-1;
	luacon_sim->clear_area(x, y, w, h);
	return 0;
}

int LuaScriptInterface::simulation_resetTemp(lua_State * l)
{
	bool onlyConductors = luaL_optint(l, 1, 0);
	for (int i = 0; i < luacon_sim->parts_lastActiveIndex; i++)
	{
		if (luacon_sim->parts[i].type && (luacon_sim->elements[luacon_sim->parts[i].type].HeatConduct || !onlyConductors))
		{
			luacon_sim->parts[i].temp = luacon_sim->elements[luacon_sim->parts[i].type].DefaultProperties.temp;
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
	ByteString name = luacon_controller->StampRegion(ui::Point(x, y), ui::Point(x+w, y+h));
	tpt_lua_pushByteString(l, name);
	return 1;
}

int LuaScriptInterface::simulation_loadStamp(lua_State * l)
{
	int i = -1;
	int pushed = 1;
	SaveFile * tempfile = NULL;
	int x = luaL_optint(l,2,0);
	int y = luaL_optint(l,3,0);
	if (lua_isstring(l, 1)) //Load from 10 char name, or full filename
	{
		auto filename = tpt_lua_optByteString(l, 1, "");
		tempfile = Client::Ref().GetStamp(filename);
	}
	if ((!tempfile || !tempfile->GetGameSave()) && lua_isnumber(l, 1)) //Load from stamp ID
	{
		i = luaL_optint(l, 1, 0);
		int stampCount = Client::Ref().GetStampsCount();
		if (i < 0 || i >= stampCount)
			return luaL_error(l, "Invalid stamp ID: %d", i);
		tempfile = Client::Ref().GetStamp(Client::Ref().GetStamps(0, stampCount)[i]);
	}

	if (tempfile)
	{
		if (!luacon_sim->Load(tempfile->GetGameSave(), !luacon_controller->GetView()->ShiftBehaviour(), x, y))
		{
			//luacon_sim->sys_pause = (tempfile->GetGameSave()->paused | luacon_model->GetPaused())?1:0;
			lua_pushinteger(l, 1);

			if (tempfile->GetGameSave()->authors.size())
			{
				tempfile->GetGameSave()->authors["type"] = "luastamp";
				Client::Ref().MergeStampAuthorInfo(tempfile->GetGameSave()->authors);
			}
		}
		else
		{
			pushed = 2;
			lua_pushnil(l);
			tpt_lua_pushString(l, luacon_ci->GetLastError());
		}
		delete tempfile;
	}
	else
	{
		pushed = 2;
		lua_pushnil(l);
		lua_pushliteral(l, "Failed to read file");
	}
	return pushed;
}

int LuaScriptInterface::simulation_deleteStamp(lua_State * l)
{
	int stampCount = Client::Ref().GetStampsCount();
	std::vector<ByteString> stamps = Client::Ref().GetStamps(0, stampCount);

	if (lua_isstring(l, 1)) //note: lua_isstring returns true on numbers too
	{
		auto filename = tpt_lua_optByteString(l, 1, "");
		for (auto &stamp : stamps)
		{
			if (stamp == filename)
			{
				Client::Ref().DeleteStamp(stamp);
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
		lua_pushinteger(l, tempSave->Version);
		return 2;
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

int LuaScriptInterface::simulation_customGravity(lua_State * l)
{
	int acount = lua_gettop(l);
	if (acount == 0)
	{
		lua_pushnumber(l, luacon_sim->customGravityX);
		lua_pushnumber(l, luacon_sim->customGravityY);
		return 2;
	}
	else if (acount == 1)
	{
		luacon_sim->customGravityX = 0.0f;
		luacon_sim->customGravityY = luaL_optnumber(l, 1, 0.0f);
		return 0;
	}
	luacon_sim->customGravityX = luaL_optnumber(l, 1, 0.0f);
	luacon_sim->customGravityY = luaL_optnumber(l, 2, 0.0f);
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
	float ambientAirTemp = restrict_flt(luaL_optnumber(l, 1, R_TEMP + 273.15f), MIN_TEMP, MAX_TEMP);
	luacon_model->SetAmbientAirTemperature(ambientAirTemp);
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
		int setting = luaL_checkint(l, 3) & 0x7F;
		luacon_ci->custom_can_move[movingElement][destinationElement] = setting | 0x80;
		luacon_sim->can_move[movingElement][destinationElement] = setting;
		return 0;
	}
}

int BrushClosure(lua_State * l)
{
	// see Simulation::ToolBrush
	int positionX = lua_tointeger(l, lua_upvalueindex(1));
	int positionY = lua_tointeger(l, lua_upvalueindex(2));
	int radiusX = lua_tointeger(l, lua_upvalueindex(3));
	int radiusY = lua_tointeger(l, lua_upvalueindex(4));
	int sizeX = lua_tointeger(l, lua_upvalueindex(5));
	int sizeY = lua_tointeger(l, lua_upvalueindex(6));
	int x = lua_tointeger(l, lua_upvalueindex(7));
	int y = lua_tointeger(l, lua_upvalueindex(8));
	unsigned char *bitmap = (unsigned char *)lua_touserdata(l, lua_upvalueindex(9));


	int yield_x, yield_y;
	while (true)
	{
		if (!(y < sizeY))
			return 0;
		if (x < sizeX)
		{
			bool yield_coords = false;
			if (bitmap[(y*sizeX)+x] && (positionX+(x-radiusX) >= 0 && positionY+(y-radiusY) >= 0 && positionX+(x-radiusX) < XRES && positionY+(y-radiusY) < YRES))
			{
				yield_coords = true;
				yield_x = positionX+(x-radiusX);
				yield_y = positionY+(y-radiusY);
			}
			x++;
			if (yield_coords)
				break;
		}
		else
		{
			x = 0;
			y++;
		}
	}

	lua_pushnumber(l, x);
	lua_replace(l, lua_upvalueindex(7));
	lua_pushnumber(l, y);
	lua_replace(l, lua_upvalueindex(8));

	lua_pushnumber(l, yield_x);
	lua_pushnumber(l, yield_y);
	return 2;
}

int LuaScriptInterface::simulation_brush(lua_State * l)
{
	int argCount = lua_gettop(l);
	int positionX = luaL_checkint(l, 1);
	int positionY = luaL_checkint(l, 2);
	int brushradiusX, brushradiusY;
	if (argCount >= 4 || !luacon_model->GetBrush())
	{
		brushradiusX = luaL_checkint(l, 3);
		brushradiusY = luaL_checkint(l, 4);
	}
	else
	{
		ui::Point radius = luacon_model->GetBrush()->GetRadius();
		brushradiusX = radius.X;
		brushradiusY = radius.Y;
	}
	int brushID = luaL_optint(l, 5, luacon_model->GetBrushID());

	std::vector<Brush *> brushList = luacon_model->GetBrushList();
	if (brushID < 0 || brushID >= (int)brushList.size())
		return luaL_error(l, "Invalid brush id '%d'", brushID);

	ui::Point tempRadius = brushList[brushID]->GetRadius();
	brushList[brushID]->SetRadius(ui::Point(brushradiusX, brushradiusY));
	lua_pushnumber(l, positionX);
	lua_pushnumber(l, positionY);
	int radiusX = brushList[brushID]->GetRadius().X;
	int radiusY = brushList[brushID]->GetRadius().Y;
	int sizeX = brushList[brushID]->GetSize().X;
	int sizeY = brushList[brushID]->GetSize().Y;
	lua_pushnumber(l, radiusX);
	lua_pushnumber(l, radiusY);
	lua_pushnumber(l, sizeX);
	lua_pushnumber(l, sizeY);
	lua_pushnumber(l, 0);
	lua_pushnumber(l, 0);
	int bitmapSize = sizeX * sizeY * sizeof(unsigned char);
	void *bitmapCopy = lua_newuserdata(l, bitmapSize);
	memcpy(bitmapCopy, brushList[brushID]->GetBitmap(), bitmapSize);
	brushList[brushID]->SetRadius(tempRadius);

	lua_pushcclosure(l, BrushClosure, 9);
	return 1;
}

int PartsClosure(lua_State *l)
{
	for (int i = lua_tointeger(l, lua_upvalueindex(1)); i <= luacon_sim->parts_lastActiveIndex; ++i)
	{
		if (luacon_sim->parts[i].type)
		{
			lua_pushnumber(l, i + 1);
			lua_replace(l, lua_upvalueindex(1));
			lua_pushnumber(l, i);
			return 1;
		}
	}
	return 0;
}

static int NeighboursClosure(lua_State *l)
{
	int cx = lua_tointeger(l, lua_upvalueindex(1));
	int cy = lua_tointeger(l, lua_upvalueindex(2));
	int rx = lua_tointeger(l, lua_upvalueindex(3));
	int ry = lua_tointeger(l, lua_upvalueindex(4));
	int t = lua_tointeger(l, lua_upvalueindex(5));
	int x = lua_tointeger(l, lua_upvalueindex(6));
	int y = lua_tointeger(l, lua_upvalueindex(7));
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
		int r = luacon_sim->pmap[py][px];
		if (!(r && (!t || TYP(r) == t))) // * If not [exists and is of the correct type]
		{
			r = 0;
		}
		if (!r)
		{
			r = luacon_sim->photons[py][px];
			if (!(r && (!t || TYP(r) == t))) // * If not [exists and is of the correct type]
			{
				r = 0;
			}
		}
		if (r)
		{
			lua_pushnumber(l, x);
			lua_replace(l, lua_upvalueindex(6));
			lua_pushnumber(l, y);
			lua_replace(l, lua_upvalueindex(7));
			lua_pushnumber(l, ID(r));
			lua_pushnumber(l, px);
			lua_pushnumber(l, py);
			return 3;
		}
	}
	return 0;
}

int LuaScriptInterface::simulation_neighbours(lua_State * l)
{
	int cx = luaL_checkint(l, 1);
	int cy = luaL_checkint(l, 2);
	int rx = luaL_optint(l, 3, 2);
	int ry = luaL_optint(l, 4, 2);
	int t = luaL_optint(l, 5, PT_NONE);
	if (rx < 0 || ry < 0)
	{
		luaL_error(l, "Invalid radius");
	}
	lua_pushnumber(l, cx);
	lua_pushnumber(l, cy);
	lua_pushnumber(l, rx);
	lua_pushnumber(l, ry);
	lua_pushnumber(l, t);
	lua_pushnumber(l, cx - rx);
	lua_pushnumber(l, cy - ry);
	lua_pushcclosure(l, NeighboursClosure, 7);
	return 1;
}

int LuaScriptInterface::simulation_parts(lua_State *l)
{
	lua_pushnumber(l, 0);
	lua_pushcclosure(l, PartsClosure, 1);
	return 1;
}

int LuaScriptInterface::simulation_pmap(lua_State * l)
{
	int x = luaL_checkint(l, 1);
	int y = luaL_checkint(l, 2);
	if (x < 0 || x >= XRES || y < 0 || y >= YRES)
		return luaL_error(l, "coordinates out of range (%d,%d)", x, y);
	int r = luacon_sim->pmap[y][x];
	if (!TYP(r))
		return 0;
	lua_pushnumber(l, ID(r));
	return 1;
}

int LuaScriptInterface::simulation_photons(lua_State * l)
{
	int x = luaL_checkint(l, 1);
	int y = luaL_checkint(l, 2);
	if (x < 0 || x >= XRES || y < 0 || y >= YRES)
		return luaL_error(l, "coordinates out of range (%d,%d)", x, y);
	int r = luacon_sim->photons[y][x];
	if (!TYP(r))
		return 0;
	lua_pushnumber(l, ID(r));
	return 1;
}

int LuaScriptInterface::simulation_framerender(lua_State * l)
{
	if (lua_gettop(l) == 0)
	{
		lua_pushinteger(l, luacon_sim->framerender);
		return 1;
	}
	int frames = luaL_checkinteger(l, 1);
	if (frames < 0)
		return luaL_error(l, "Can't simulate a negative number of frames");
	luacon_sim->framerender = frames;
	return 0;
}

int LuaScriptInterface::simulation_gspeed(lua_State * l)
{
	if (lua_gettop(l) == 0)
	{
		lua_pushinteger(l, luacon_sim->GSPEED);
		return 1;
	}
	int gspeed = luaL_checkinteger(l, 1);
	if (gspeed < 1)
		return luaL_error(l, "GSPEED must be at least 1");
	luacon_sim->GSPEED = gspeed;
	return 0;
}

int LuaScriptInterface::simulation_takeSnapshot(lua_State * l)
{
	luacon_controller->HistorySnapshot();
	return 0;
}


int LuaScriptInterface::simulation_historyRestore(lua_State *l)
{
	bool successful = luacon_controller->HistoryRestore();
	lua_pushboolean(l, successful);
	return 1;
}

int LuaScriptInterface::simulation_historyForward(lua_State *l)
{
	bool successful = luacon_controller->HistoryForward();
	lua_pushboolean(l, successful);
	return 1;
}

int LuaScriptInterface::simulation_replaceModeFlags(lua_State *l)
{
	if (lua_gettop(l) == 0)
	{
		lua_pushinteger(l, luacon_controller->GetReplaceModeFlags());
		return 1;
	}
	unsigned int flags = luaL_checkinteger(l, 1);
	if (flags & ~(REPLACE_MODE | SPECIFIC_DELETE))
		return luaL_error(l, "Invalid flags");
	if ((flags & REPLACE_MODE) && (flags & SPECIFIC_DELETE))
		return luaL_error(l, "Cannot set replace mode and specific delete at the same time");
	luacon_controller->SetReplaceModeFlags(flags);
	return 0;
}

int LuaScriptInterface::simulation_listCustomGol(lua_State *l)
{
	int i = 0;
	lua_newtable(l);
	for (auto &cgol : luacon_sim->GetCustomGol())
	{
		lua_newtable(l);
		tpt_lua_pushString(l, cgol.nameString);
		lua_setfield(l, -2, "name");
		tpt_lua_pushString(l, cgol.ruleString);
		lua_setfield(l, -2, "rulestr");
		lua_pushnumber(l, cgol.rule);
		lua_setfield(l, -2, "rule");
		lua_pushnumber(l, cgol.colour1);
		lua_setfield(l, -2, "color1");
		lua_pushnumber(l, cgol.colour2);
		lua_setfield(l, -2, "color2");
		lua_rawseti(l, -2, ++i);
	}
	return 1;
}

int LuaScriptInterface::simulation_addCustomGol(lua_State *l)
{
	int rule;
	String ruleString;
	if (lua_isnumber(l, 1))
	{
		rule = luaL_checkinteger(l, 1);
		ruleString = SerialiseGOLRule(rule);
		rule = ParseGOLString(ruleString);
	}
	else
	{
		ruleString = tpt_lua_checkString(l, 1);
		rule = ParseGOLString(ruleString);
	}
	String nameString = tpt_lua_checkString(l, 2);
	unsigned int color1 = luaL_checkinteger(l, 3);
	unsigned int color2 = luaL_checkinteger(l, 4);

	if (nameString.empty() || !ValidateGOLName(nameString))
		return luaL_error(l, "Invalid name provided");
	if (rule == -1)
		return luaL_error(l, "Invalid rule provided");
	if (luacon_sim->GetCustomGOLByRule(rule))
		return luaL_error(l, "This Custom GoL rule already exists");

	if (!AddCustomGol(ruleString, nameString, color1, color2))
		return luaL_error(l, "Duplicate name, cannot add");
	luacon_model->BuildMenus();
	return 0;
}

int LuaScriptInterface::simulation_removeCustomGol(lua_State *l)
{
	ByteString nameString = tpt_lua_checkByteString(l, 1);
	bool removedAny = luacon_model->RemoveCustomGOLType("DEFAULT_PT_LIFECUST_" + nameString);
	if (removedAny)
		luacon_model->BuildMenus();
	lua_pushboolean(l, removedAny);
	return 1;
}

int LuaScriptInterface::simulation_lastUpdatedID(lua_State *l)
{
	if (luacon_sim->debug_mostRecentlyUpdated != -1)
	{
		lua_pushinteger(l, luacon_sim->debug_mostRecentlyUpdated);
	}
	else
	{
		lua_pushnil(l);
	}
	return 1;
}

int LuaScriptInterface::simulation_updateUpTo(lua_State *l)
{
	int upTo = NPART - 1;
	if (lua_gettop(l) > 0)
	{
		upTo = luaL_checkinteger(l, 1);
	}
	if (upTo < 0 || upTo >= NPART)
	{
		return luaL_error(l, "ID not in valid range");
	}
	if (upTo < luacon_sim->debug_currentParticle)
	{
		upTo = NPART - 1;
	}
	if (luacon_sim->debug_currentParticle == 0)
	{
		luacon_sim->framerender = 1;
		luacon_sim->BeforeSim();
		luacon_sim->framerender = 0;
	}
	luacon_sim->UpdateParticles(luacon_sim->debug_currentParticle, upTo);
	if (upTo < NPART - 1)
	{
		luacon_sim->debug_currentParticle = upTo + 1;
	}
	else
	{
		luacon_sim->AfterSim();
		luacon_sim->debug_currentParticle = 0;
	}
	return 0;
}

//// Begin Renderer API

void LuaScriptInterface::initRendererAPI()
{
	//Methods
	struct luaL_Reg rendererAPIMethods [] = {
		{"renderModes", renderer_renderModes},
		{"displayModes", renderer_displayModes},
		{"colourMode", renderer_colourMode},
		{"colorMode", renderer_colourMode}, //Duplicate of above to make Americans happy
		{"decorations", renderer_decorations}, //renderer_debugHUD
		{"grid", renderer_grid},
		{"debugHUD", renderer_debugHUD},
		{"showBrush", renderer_showBrush},
		{"depth3d", renderer_depth3d},
		{"zoomEnabled", renderer_zoomEnabled},
		{"zoomWindow", renderer_zoomWindowInfo},
		{"zoomScope", renderer_zoomScopeInfo},
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
		size = lua_objlen(l, 1);

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
		size = lua_objlen(l, 1);

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

int LuaScriptInterface::renderer_showBrush(lua_State * l)
{
	int acount = lua_gettop(l);
	if (acount == 0)
	{
		lua_pushnumber(l, luacon_controller->GetBrushEnable());
		return 1;
	}
	int brush = luaL_optint(l, 1, -1);
	luacon_controller->SetBrushEnable(brush);
	return 0;
}

int LuaScriptInterface::renderer_depth3d(lua_State * l)
{
	return luaL_error(l, "This feature is no longer supported");
}

int LuaScriptInterface::renderer_zoomEnabled(lua_State * l)
{
	if (lua_gettop(l) == 0)
	{
		lua_pushboolean(l, luacon_ren->zoomEnabled);
		return 1;
	}
	else
	{
		luaL_checktype(l, -1, LUA_TBOOLEAN);
		luacon_ren->zoomEnabled = lua_toboolean(l, -1);
		return 0;
	}
}
int LuaScriptInterface::renderer_zoomWindowInfo(lua_State * l)
{
	if (lua_gettop(l) == 0)
	{
		ui::Point location = luacon_ren->zoomWindowPosition;
		lua_pushnumber(l, location.X);
		lua_pushnumber(l, location.Y);
		lua_pushnumber(l, luacon_ren->ZFACTOR);
		lua_pushnumber(l, luacon_ren->zoomScopeSize * luacon_ren->ZFACTOR);
		return 4;
	}
	int x = luaL_optint(l, 1, 0);
	int y = luaL_optint(l, 2, 0);
	int f = luaL_optint(l, 3, 0);
	if (f <= 0)
		return luaL_error(l, "Zoom factor must be greater than 0");

	// To prevent crash when zoom window is outside screen
	if (x < 0 || y < 0 || luacon_ren->zoomScopeSize * f + x > XRES || luacon_ren->zoomScopeSize * f + y > YRES)
		return luaL_error(l, "Zoom window outside of bounds");

	luacon_ren->zoomWindowPosition = ui::Point(x, y);
	luacon_ren->ZFACTOR = f;
	return 0;
}
int LuaScriptInterface::renderer_zoomScopeInfo(lua_State * l)
{
	if (lua_gettop(l) == 0)
	{
		ui::Point location = luacon_ren->zoomScopePosition;
		lua_pushnumber(l, location.X);
		lua_pushnumber(l, location.Y);
		lua_pushnumber(l, luacon_ren->zoomScopeSize);
		return 3;
	}
	int x = luaL_optint(l, 1, 0);
	int y = luaL_optint(l, 2, 0);
	int s = luaL_optint(l, 3, 0);
	if (s <= 0)
		return luaL_error(l, "Zoom scope size must be greater than 0");

	// To prevent crash when zoom or scope window is outside screen
	int windowEdgeRight = luacon_ren->ZFACTOR * s + luacon_ren->zoomWindowPosition.X;
	int windowEdgeBottom = luacon_ren->ZFACTOR * s + luacon_ren->zoomWindowPosition.Y;
	if (x < 0 || y < 0 || x + s > XRES || y + s > YRES)
		return luaL_error(l, "Zoom scope outside of bounds");
	if (windowEdgeRight > XRES || windowEdgeBottom > YRES)
		return luaL_error(l, "Zoom window outside of bounds");

	luacon_ren->zoomScopePosition = ui::Point(x, y);
	luacon_ren->zoomScopeSize = s;
	return 0;
}

void LuaScriptInterface::initElementsAPI()
{
	//Methods
	struct luaL_Reg elementsAPIMethods [] = {
		{"allocate", elements_allocate},
		{"element", elements_element},
		{"property", elements_property},
		{"free", elements_free},
		{"exists", elements_exists},
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
	lua_pushinteger(l, 0); lua_setfield(l, -2, "PROP_DRAWONCTYPE");
	SETCONST(l, PROP_NOCTYPEDRAW);
	SETCONST(l, FLAG_STAGNANT);
	SETCONST(l, FLAG_SKIPMOVE);
	SETCONST(l, FLAG_MOVABLE);
	SETCONST(l, FLAG_PHOTDECO);
	lua_pushinteger(l, 0);
	lua_setfield(l, -2, "ST_NONE");
	lua_pushinteger(l, 0);
	lua_setfield(l, -2, "ST_SOLID");
	lua_pushinteger(l, 0);
	lua_setfield(l, -2, "ST_LIQUID");
	lua_pushinteger(l, 0);
	lua_setfield(l, -2, "ST_GAS");

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
			tpt_lua_pushByteString(l, luacon_sim->elements[i].Identifier);
			lua_pushinteger(l, i);
			lua_settable(l, -3);
			ByteString realIdentifier = ByteString::Build("DEFAULT_PT_", luacon_sim->elements[i].Name.ToUtf8());
			if (i != 0 && i != PT_NBHL && i != PT_NWHL && luacon_sim->elements[i].Identifier != realIdentifier)
			{
				tpt_lua_pushByteString(l, realIdentifier);
				lua_pushinteger(l, i);
				lua_settable(l, -3);
			}
		}
	}
}

void LuaScriptInterface::LuaGetProperty(lua_State* l, StructProperty property, intptr_t propertyAddress)
{
	switch (property.Type)
	{
		case StructProperty::TransitionType:
		case StructProperty::ParticleType:
		case StructProperty::Integer:
			lua_pushnumber(l, *((int*)propertyAddress));
			break;
		case StructProperty::UInteger:
			lua_pushnumber(l, *((unsigned int*)propertyAddress));
			break;
		case StructProperty::Float:
			lua_pushnumber(l, *((float*)propertyAddress));
			break;
		case StructProperty::UChar:
			lua_pushnumber(l, *((unsigned char*)propertyAddress));
			break;
		case StructProperty::BString:
		{
			tpt_lua_pushByteString(l, *((ByteString*)propertyAddress));
			break;
		}
		case StructProperty::String:
		{
			tpt_lua_pushString(l, *((String*)propertyAddress));
			break;
		}
		case StructProperty::Colour:
#if PIXELSIZE == 4
			lua_pushinteger(l, *((unsigned int*)propertyAddress));
#else
			lua_pushinteger(l, *((unsigned short*)propertyAddress));
#endif
			break;
		case StructProperty::Removed:
			lua_pushnil(l);
	}
}

static int32_t int32_truncate(double n)
{
	if (n >= 0x1p31)
	{
		n -= 0x1p32;
	}
	return int32_t(n);
}

void LuaScriptInterface::LuaSetProperty(lua_State* l, StructProperty property, intptr_t propertyAddress, int stackPos)
{
	switch (property.Type)
	{
		case StructProperty::TransitionType:
		case StructProperty::ParticleType:
		case StructProperty::Integer:
			*((int*)propertyAddress) = int32_truncate(luaL_checknumber(l, stackPos));
			break;
		case StructProperty::UInteger:
			*((unsigned int*)propertyAddress) = int32_truncate(luaL_checknumber(l, stackPos));
			break;
		case StructProperty::Float:
			*((float*)propertyAddress) = luaL_checknumber(l, stackPos);
			break;
		case StructProperty::UChar:
			*((unsigned char*)propertyAddress) = int32_truncate(luaL_checknumber(l, stackPos));
			break;
		case StructProperty::BString:
			*((ByteString*)propertyAddress) = tpt_lua_checkByteString(l, stackPos);
			break;
		case StructProperty::String:
			*((String*)propertyAddress) = tpt_lua_checkString(l, stackPos);
			break;
		case StructProperty::Colour:
#if PIXELSIZE == 4
			*((unsigned int*)propertyAddress) = int32_truncate(luaL_checknumber(l, stackPos));
#else
			*((unsigned short*)propertyAddress) = int32_truncate(luaL_checknumber(l, stackPos));
#endif
			break;
		case StructProperty::Removed:
			break;
	}
}

int LuaScriptInterface::elements_loadDefault(lua_State * l)
{
	int args = lua_gettop(l);
	if (args)
	{
		luaL_checktype(l, 1, LUA_TNUMBER);
		int id = lua_tointeger(l, 1);
		if (id < 0 || id >= PT_NUM)
			return luaL_error(l, "Invalid element");

		lua_getglobal(l, "elements");
		tpt_lua_pushByteString(l, luacon_sim->elements[id].Identifier);
		lua_pushnil(l);
		lua_settable(l, -3);

		auto const &elementList = GetElements();
		if (id < (int)elementList.size())
			luacon_sim->elements[id] = elementList[id];
		else
			luacon_sim->elements[id] = Element();

		tpt_lua_pushByteString(l, luacon_sim->elements[id].Identifier);
		lua_pushinteger(l, id);
		lua_settable(l, -3);
		lua_pop(l, 1);
	}
	else
	{
		auto const &elementList = GetElements();
		for (int i = 0; i < PT_NUM; i++)
		{
			if (i < (int)elementList.size())
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
	for (auto moving = 0; moving < PT_NUM; ++moving)
	{
		for (auto into = 0; into < PT_NUM; ++into)
		{
			luacon_ci->custom_can_move[moving][into] = 0;
		}
	}
	luacon_ci->custom_init_can_move();
	std::fill(luacon_ren->graphicscache, luacon_ren->graphicscache+PT_NUM, gcache_item());
	SaveRenderer::Ref().Flush(0, PT_NUM);
	return 0;
}

int LuaScriptInterface::elements_allocate(lua_State * l)
{
	luaL_checktype(l, 1, LUA_TSTRING);
	luaL_checktype(l, 2, LUA_TSTRING);
	auto group = tpt_lua_toByteString(l, 1).ToUpper();
	auto id = tpt_lua_toByteString(l, 2).ToUpper();

	if (id.Contains("_"))
	{
		return luaL_error(l, "The element name may not contain '_'.");
	}
	if (group.Contains("_"))
	{
		return luaL_error(l, "The group name may not contain '_'.");
	}
	if (group == "DEFAULT")
	{
		return luaL_error(l, "You cannot create elements in the 'DEFAULT' group.");
	}

	auto identifier = group + "_PT_" + id;

	for(int i = 0; i < PT_NUM; i++)
	{
		if(luacon_sim->elements[i].Enabled && ByteString(luacon_sim->elements[i].Identifier) == identifier)
			return luaL_error(l, "Element identifier already in use");
	}

	int newID = -1;
	// Start out at 255 so that lua element IDs are still one byte (better save compatibility)
	for (int i = PT_NUM >= 255 ? 255 : PT_NUM; i >= 0; i--)
	{
		if (!luacon_sim->elements[i].Enabled)
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
			if (!luacon_sim->elements[i].Enabled)
			{
				newID = i;
				break;
			}
		}
	}

	if (newID != -1)
	{
		luacon_sim->elements[newID] = Element();
		luacon_sim->elements[newID].Enabled = true;
		luacon_sim->elements[newID].Identifier = identifier;

		lua_getglobal(l, "elements");
		tpt_lua_pushByteString(l, identifier);
		lua_pushinteger(l, newID);
		lua_settable(l, -3);
		lua_pop(l, 1);

		for (auto elem = 0; elem < PT_NUM; ++elem)
		{
			luacon_ci->custom_can_move[elem][newID] = 0;
			luacon_ci->custom_can_move[newID][elem] = 0;
		}
		luacon_model->BuildMenus();
		luacon_ci->custom_init_can_move();
	}

	lua_pushinteger(l, newID);
	return 1;
}

static int luaUpdateWrapper(UPDATE_FUNC_ARGS)
{
	auto *builtinUpdate = GetElements()[parts[i].type].Update;
	if (builtinUpdate && lua_el_mode[parts[i].type] == 1)
	{
		if (builtinUpdate(UPDATE_FUNC_SUBCALL_ARGS))
			return 1;
		x = (int)(parts[i].x+0.5f);
		y = (int)(parts[i].y+0.5f);
	}
	if (lua_el_func[parts[i].type])
	{
		int retval = 0, callret;
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
		if (retval)
		{
			return 1;
		}
		x = (int)(parts[i].x+0.5f);
		y = (int)(parts[i].y+0.5f);
	}
	if (builtinUpdate && lua_el_mode[parts[i].type] == 3)
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
	if (lua_gr_func[cpart->type])
	{
		int cache = 0, callret;
		int i = cpart - ren->sim->parts; // pointer arithmetic be like
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
			bool valid = true;
			for (int i = -10; i < 0; i++)
				if (!lua_isnumber(luacon_ci->l, i) && !lua_isnil(luacon_ci->l, i))
				{
					valid = false;
					break;
				}
			if (valid)
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
			}
			lua_pop(luacon_ci->l, 10);
		}
		return cache;
	}
	return 0;
}

static void luaCreateWrapper(ELEMENT_CREATE_FUNC_ARGS)
{
	if (luaCreateHandlers[sim->parts[i].type])
	{
		lua_rawgeti(luacon_ci->l, LUA_REGISTRYINDEX, luaCreateHandlers[sim->parts[i].type]);
		lua_pushinteger(luacon_ci->l, i);
		lua_pushinteger(luacon_ci->l, x);
		lua_pushinteger(luacon_ci->l, y);
		lua_pushinteger(luacon_ci->l, t);
		lua_pushinteger(luacon_ci->l, v);
		if (lua_pcall(luacon_ci->l, 5, 0, 0))
		{
			luacon_ci->Log(CommandInterface::LogError, "In create func: " + luacon_geterror());
			lua_pop(luacon_ci->l, 1);
		}
	}
}

static bool luaCreateAllowedWrapper(ELEMENT_CREATE_ALLOWED_FUNC_ARGS)
{
	bool ret = false;
	if (luaCreateAllowedHandlers[t])
	{
		lua_rawgeti(luacon_ci->l, LUA_REGISTRYINDEX, luaCreateAllowedHandlers[t]);
		lua_pushinteger(luacon_ci->l, i);
		lua_pushinteger(luacon_ci->l, x);
		lua_pushinteger(luacon_ci->l, y);
		lua_pushinteger(luacon_ci->l, t);
		if (lua_pcall(luacon_ci->l, 4, 1, 0))
		{
			luacon_ci->Log(CommandInterface::LogError, "In create allowed: " + luacon_geterror());
			lua_pop(luacon_ci->l, 1);
		}
		else
		{
			if (lua_isboolean(luacon_ci->l, -1))
				ret = lua_toboolean(luacon_ci->l, -1);
			lua_pop(luacon_ci->l, 1);
		}
	}
	return ret;
}

static void luaChangeTypeWrapper(ELEMENT_CHANGETYPE_FUNC_ARGS)
{
	if (luaChangeTypeHandlers[sim->parts[i].type])
	{
		lua_rawgeti(luacon_ci->l, LUA_REGISTRYINDEX, luaChangeTypeHandlers[sim->parts[i].type]);
		lua_pushinteger(luacon_ci->l, i);
		lua_pushinteger(luacon_ci->l, x);
		lua_pushinteger(luacon_ci->l, y);
		lua_pushinteger(luacon_ci->l, from);
		lua_pushinteger(luacon_ci->l, to);
		if (lua_pcall(luacon_ci->l, 5, 0, 0))
		{
			luacon_ci->Log(CommandInterface::LogError, "In change type: " + luacon_geterror());
			lua_pop(luacon_ci->l, 1);
		}
	}
}

static bool luaCtypeDrawWrapper(CTYPEDRAW_FUNC_ARGS)
{
	bool ret = false;
	if (luaCtypeDrawHandlers[sim->parts[i].type])
	{
		lua_rawgeti(luacon_ci->l, LUA_REGISTRYINDEX, luaCtypeDrawHandlers[sim->parts[i].type]);
		lua_pushinteger(luacon_ci->l, i);
		lua_pushinteger(luacon_ci->l, t);
		lua_pushinteger(luacon_ci->l, v);
		if (lua_pcall(luacon_ci->l, 3, 1, 0))
		{
			luacon_ci->Log(CommandInterface::LogError, luacon_geterror());
			lua_pop(luacon_ci->l, 1);
		}
		else
		{
			if (lua_isboolean(luacon_ci->l, -1))
				ret = lua_toboolean(luacon_ci->l, -1);
			lua_pop(luacon_ci->l, 1);
		}
	}
	return ret;
}

int LuaScriptInterface::elements_element(lua_State * l)
{
	int id = luaL_checkinteger(l, 1);
	if (!luacon_sim->IsElementOrNone(id))
	{
		return luaL_error(l, "Invalid element");
	}

	if (lua_gettop(l) > 1)
	{
		luaL_checktype(l, 2, LUA_TTABLE);
		//Write values from native data to a table
		for (auto &prop : Element::GetProperties())
		{
			tpt_lua_pushByteString(l, prop.Name);
			lua_gettable(l, -2);
			if (lua_type(l, -1) != LUA_TNIL)
			{
				intptr_t propertyAddress = (intptr_t)(((unsigned char*)&luacon_sim->elements[id]) + prop.Offset);
				LuaSetProperty(l, prop, propertyAddress, -1);
			}
			lua_pop(l, 1);
		}

		lua_getfield(l, -1, "Update");
		if (lua_type(l, -1) == LUA_TFUNCTION)
		{
			lua_el_func[id].Assign(l, -1);
			lua_el_mode[id] = 1;
			luacon_sim->elements[id].Update = luaUpdateWrapper;
		}
		else if (lua_type(l, -1) == LUA_TBOOLEAN && !lua_toboolean(l, -1))
		{
			lua_el_func[id].Clear();
			lua_el_mode[id] = 0;
			luacon_sim->elements[id].Update = GetElements()[id].Update;
		}
		lua_pop(l, 1);

		lua_getfield(l, -1, "Graphics");
		if (lua_type(l, -1) == LUA_TFUNCTION)
		{
			lua_gr_func[id].Assign(l, -1);
			luacon_sim->elements[id].Graphics = luaGraphicsWrapper;
		}
		else if (lua_type(l, -1) == LUA_TBOOLEAN && !lua_toboolean(l, -1))
		{
			lua_gr_func[id].Clear();
			luacon_sim->elements[id].Graphics = GetElements()[id].Graphics;
		}
		lua_pop(l, 1);

		lua_getfield(l, -1, "Create");
		if (lua_type(l, -1) == LUA_TFUNCTION)
		{
			luaCreateHandlers[id].Assign(l, -1);
			luacon_sim->elements[id].Create = luaCreateWrapper;
		}
		else if (lua_type(l, -1) == LUA_TBOOLEAN && !lua_toboolean(l, -1))
		{
			luaCreateHandlers[id].Clear();
			luacon_sim->elements[id].Create = GetElements()[id].Create;
		}
		lua_pop(l, 1);

		lua_getfield(l, -1, "CreateAllowed");
		if (lua_type(l, -1) == LUA_TFUNCTION)
		{
			luaCreateAllowedHandlers[id].Assign(l, -1);
			luacon_sim->elements[id].CreateAllowed = luaCreateAllowedWrapper;
		}
		else if (lua_type(l, -1) == LUA_TBOOLEAN && !lua_toboolean(l, -1))
		{
			luaCreateAllowedHandlers[id].Clear();
			luacon_sim->elements[id].CreateAllowed = GetElements()[id].CreateAllowed;
		}
		lua_pop(l, 1);

		lua_getfield(l, -1, "ChangeType");
		if (lua_type(l, -1) == LUA_TFUNCTION)
		{
			luaChangeTypeHandlers[id].Assign(l, -1);
			luacon_sim->elements[id].ChangeType = luaChangeTypeWrapper;
		}
		else if (lua_type(l, -1) == LUA_TBOOLEAN && !lua_toboolean(l, -1))
		{
			luaChangeTypeHandlers[id].Clear();
			luacon_sim->elements[id].ChangeType = GetElements()[id].ChangeType;
		}
		lua_pop(l, 1);

		lua_getfield(l, -1, "CtypeDraw");
		if (lua_type(l, -1) == LUA_TFUNCTION)
		{
			luaCtypeDrawHandlers[id].Assign(l, -1);
			luacon_sim->elements[id].CtypeDraw = luaCtypeDrawWrapper;
		}
		else if (lua_type(l, -1) == LUA_TBOOLEAN && !lua_toboolean(l, -1))
		{
			luaCtypeDrawHandlers[id].Clear();
			luacon_sim->elements[id].CtypeDraw = GetElements()[id].CtypeDraw;
		}
		lua_pop(l, 1);

		lua_getfield(l, -1, "DefaultProperties");
		SetDefaultProperties(l, id, lua_gettop(l));
		lua_pop(l, 1);

		luacon_model->BuildMenus();
		luacon_ci->custom_init_can_move();
		luacon_ren->graphicscache[id].isready = 0;
		SaveRenderer::Ref().Flush(id, id + 1);

		return 0;
	}
	else
	{
		//Write values from native data to a table
		lua_newtable(l);
		for (auto &prop : Element::GetProperties())
		{
			tpt_lua_pushByteString(l, prop.Name);
			intptr_t propertyAddress = (intptr_t)(((unsigned char*)&luacon_sim->elements[id]) + prop.Offset);
			LuaGetProperty(l, prop, propertyAddress);
			lua_settable(l, -3);
		}

		tpt_lua_pushByteString(l, luacon_sim->elements[id].Identifier);
		lua_setfield(l, -2, "Identifier");

		GetDefaultProperties(l, id);
		lua_setfield(l, -2, "DefaultProperties");

		return 1;
	}
}

void LuaScriptInterface::GetDefaultProperties(lua_State * l, int id)
{
	lua_newtable(l);
	for (auto &prop : Particle::GetProperties())
	{
		auto propertyAddress = reinterpret_cast<intptr_t>((reinterpret_cast<unsigned char*>(&luacon_sim->elements[id].DefaultProperties)) + prop.Offset);
		tpt_lua_pushByteString(l, prop.Name);
		LuaGetProperty(l, prop, propertyAddress);
		lua_settable(l, -3);
	}
	for (auto &alias : Particle::GetPropertyAliases())
	{
		tpt_lua_pushByteString(l, alias.from);
		tpt_lua_pushByteString(l, alias.to);
		lua_gettable(l, -3);
		lua_settable(l, -3);
	}
}

void LuaScriptInterface::SetDefaultProperties(lua_State * l, int id, int stackPos)
{
	if (lua_type(l, stackPos) == LUA_TTABLE)
	{
		for (auto &prop : Particle::GetProperties())
		{
			tpt_lua_pushByteString(l, prop.Name);
			lua_gettable(l, stackPos);
			if (lua_type(l, -1) == LUA_TNIL)
			{
				for (auto &alias : Particle::GetPropertyAliases())
				{
					if (alias.to == prop.Name)
					{
						lua_pop(l, 1);
						tpt_lua_pushByteString(l, alias.from);
						lua_gettable(l, stackPos);
					}
				}
			}
			if (lua_type(l, -1) != LUA_TNIL)
			{
				auto propertyAddress = reinterpret_cast<intptr_t>((reinterpret_cast<unsigned char*>(&luacon_sim->elements[id].DefaultProperties)) + prop.Offset);
				LuaSetProperty(l, prop, propertyAddress, -1);
			}
			lua_pop(l, 1);
		}
	}
}

int LuaScriptInterface::elements_property(lua_State * l)
{
	int id = luaL_checkinteger(l, 1);
	if (!luacon_sim->IsElementOrNone(id))
	{
		return luaL_error(l, "Invalid element");
	}
	ByteString propertyName = tpt_lua_checkByteString(l, 2);

	auto &properties = Element::GetProperties();
	auto prop = std::find_if(properties.begin(), properties.end(), [&propertyName](StructProperty const &p) {
		return p.Name == propertyName;
	});

	if (lua_gettop(l) > 2)
	{
		if (prop != properties.end())
		{
			if (lua_type(l, 3) != LUA_TNIL)
			{
				if (prop->Type == StructProperty::TransitionType)
				{
					int type = luaL_checkinteger(l, 3);
					if (!luacon_sim->IsElementOrNone(type) && type != NT && type != ST)
					{
						return luaL_error(l, "Invalid element");
					}
				}

				intptr_t propertyAddress = (intptr_t)(((unsigned char*)&luacon_sim->elements[id]) + prop->Offset);
				LuaSetProperty(l, *prop, propertyAddress, 3);
			}

			luacon_model->BuildMenus();
			luacon_ci->custom_init_can_move();
			luacon_ren->graphicscache[id].isready = 0;
			SaveRenderer::Ref().Flush(id, id + 1);
		}
		else if (propertyName == "Update")
		{
			if (lua_type(l, 3) == LUA_TFUNCTION)
			{
				switch (luaL_optint(l, 4, 0))
				{
				case 2:
					lua_el_mode[id] = 3; //update before
					break;

				case 1:
					lua_el_mode[id] = 2; //replace
					break;

				default:
					lua_el_mode[id] = 1; //update after
					break;
				}
				lua_el_func[id].Assign(l, 3);
				luacon_sim->elements[id].Update = luaUpdateWrapper;
			}
			else if (lua_type(l, 3) == LUA_TBOOLEAN && !lua_toboolean(l, 3))
			{
				lua_el_func[id].Clear();
				lua_el_mode[id] = 0;
				luacon_sim->elements[id].Update = GetElements()[id].Update;
			}
		}
		else if (propertyName == "Graphics")
		{
			if (lua_type(l, 3) == LUA_TFUNCTION)
			{
				lua_gr_func[id].Assign(l, 3);
				luacon_sim->elements[id].Graphics = luaGraphicsWrapper;
			}
			else if (lua_type(l, 3) == LUA_TBOOLEAN && !lua_toboolean(l, 3))
			{
				lua_gr_func[id].Clear();
				luacon_sim->elements[id].Graphics = GetElements()[id].Graphics;
			}
			luacon_ren->graphicscache[id].isready = 0;
			SaveRenderer::Ref().Flush(id, id + 1);
		}
		else if (propertyName == "Create")
		{
			if (lua_type(l, 3) == LUA_TFUNCTION)
			{
				luaCreateHandlers[id].Assign(l, 3);
				luacon_sim->elements[id].Create = luaCreateWrapper;
			}
			else if (lua_type(l, 3) == LUA_TBOOLEAN && !lua_toboolean(l, 3))
			{
				luaCreateHandlers[id].Clear();
				luacon_sim->elements[id].Create = GetElements()[id].Create;
			}
		}
		else if (propertyName == "CreateAllowed")
		{
			if (lua_type(l, 3) == LUA_TFUNCTION)
			{
				luaCreateAllowedHandlers[id].Assign(l, 3);
				luacon_sim->elements[id].CreateAllowed = luaCreateAllowedWrapper;
			}
			else if (lua_type(l, 3) == LUA_TBOOLEAN && !lua_toboolean(l, 3))
			{
				luaCreateAllowedHandlers[id].Clear();
				luacon_sim->elements[id].CreateAllowed = GetElements()[id].CreateAllowed;
			}
		}
		else if (propertyName == "ChangeType")
		{
			if (lua_type(l, 3) == LUA_TFUNCTION)
			{
				luaChangeTypeHandlers[id].Assign(l, 3);
				luacon_sim->elements[id].ChangeType = luaChangeTypeWrapper;
			}
			else if (lua_type(l, 3) == LUA_TBOOLEAN && !lua_toboolean(l, 3))
			{
				luaChangeTypeHandlers[id].Clear();
				luacon_sim->elements[id].ChangeType = GetElements()[id].ChangeType;
			}
		}
		else if (propertyName == "CtypeDraw")
		{
			if (lua_type(l, 3) == LUA_TFUNCTION)
			{
				luaCtypeDrawHandlers[id].Assign(l, 3);
				luacon_sim->elements[id].CtypeDraw = luaCtypeDrawWrapper;
			}
			else if (lua_type(l, 3) == LUA_TBOOLEAN && !lua_toboolean(l, 3))
			{
				luaCtypeDrawHandlers[id].Clear();
				luacon_sim->elements[id].CtypeDraw = GetElements()[id].CtypeDraw;
			}
		}
		else if (propertyName == "DefaultProperties")
		{
			SetDefaultProperties(l, id, 3);
		}
		else
		{
			return luaL_error(l, "Invalid element property");
		}
		return 0;
	}
	else
	{
		if (prop != properties.end())
		{
			intptr_t propertyAddress = (intptr_t)(((unsigned char*)&luacon_sim->elements[id]) + prop->Offset);
			LuaGetProperty(l, *prop, propertyAddress);
			return 1;
		}
		else if (propertyName == "Identifier")
		{
			tpt_lua_pushByteString(l, luacon_sim->elements[id].Identifier);
			return 1;
		}
		else if (propertyName == "DefaultProperties")
		{
			GetDefaultProperties(l, id);
			return 1;
		}
		else
		{
			return luaL_error(l, "Invalid element property");
		}
	}
}

int LuaScriptInterface::elements_free(lua_State * l)
{
	int id = luaL_checkinteger(l, 1);
	if (!luacon_sim->IsElement(id))
	{
		return luaL_error(l, "Invalid element");
	}

	ByteString identifier = luacon_sim->elements[id].Identifier;
	if (identifier.BeginsWith("DEFAULT_PT_"))
	{
		return luaL_error(l, "Cannot free default elements");
	}

	luacon_sim->elements[id].Enabled = false;
	luacon_model->BuildMenus();

	lua_getglobal(l, "elements");
	tpt_lua_pushByteString(l, identifier);
	lua_pushnil(l);
	lua_settable(l, -3);
	lua_pop(l, 1);

	return 0;
}

int LuaScriptInterface::elements_exists(lua_State * l)
{
	lua_pushboolean(l, luacon_sim->IsElement(luaL_checkinteger(l, 1)));
	return 1;
}

void LuaScriptInterface::initGraphicsAPI()
{
	//Methods
	struct luaL_Reg graphicsAPIMethods [] = {
		{"textSize", graphics_textSize},
		{"drawText", graphics_drawText},
		{"drawLine", graphics_drawLine},
		{"drawRect", graphics_drawRect},
		{"fillRect", graphics_fillRect},
		{"drawCircle", graphics_drawCircle},
		{"fillCircle", graphics_fillCircle},
		{"getColors", graphics_getColors},
		{"getHexColor", graphics_getHexColor},
		{"setClipRect", graphics_setClipRect},
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
	auto text = tpt_lua_optString(l, 1, "");
	Graphics::textsize(text, width, height);

	lua_pushinteger(l, width);
	lua_pushinteger(l, height);
	return 2;
}

int LuaScriptInterface::graphics_drawText(lua_State * l)
{
	int x = lua_tointeger(l, 1);
	int y = lua_tointeger(l, 2);
	auto text = tpt_lua_optString(l, 3, "");
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

int LuaScriptInterface::graphics_getColors(lua_State * l)
{
	unsigned int color = int32_truncate(lua_tonumber(l, 1));

	int a = color >> 24;
	int r = (color >> 16)&0xFF;
	int g = (color >> 8)&0xFF;
	int b = color&0xFF;

	lua_pushinteger(l, r);
	lua_pushinteger(l, g);
	lua_pushinteger(l, b);
	lua_pushinteger(l, a);
	return 4;
}

int LuaScriptInterface::graphics_getHexColor(lua_State * l)
{
	int r = lua_tointeger(l, 1);
	int g = lua_tointeger(l, 2);
	int b = lua_tointeger(l, 3);
	int a = 0;
	if (lua_gettop(l) >= 4)
		a = lua_tointeger(l, 4);
	unsigned int color = (a<<24) + (r<<16) + (g<<8) + b;

	lua_pushinteger(l, color);
	return 1;
}

int LuaScriptInterface::graphics_setClipRect(lua_State * l)
{
	int x = luaL_optinteger(l, 1, 0);
	int y = luaL_optinteger(l, 2, 0);
	int w = luaL_optinteger(l, 3, WINDOWW);
	int h = luaL_optinteger(l, 4, WINDOWH);
	luacon_g->SetClipRect(x, y, w, h);
	lua_pushinteger(l, x);
	lua_pushinteger(l, y);
	lua_pushinteger(l, w);
	lua_pushinteger(l, h);
	return 4;
}

void LuaScriptInterface::initFileSystemAPI()
{
	//Methods
	struct luaL_Reg fileSystemAPIMethods [] = {
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
	auto directoryName = tpt_lua_checkByteString(l, 1);
	lua_newtable(l);
	int index = 0;
	for (auto &name : Platform::DirectorySearch(directoryName, "", {}))
	{
		if (name != "." && name != "..")
		{
			index += 1;
			tpt_lua_pushByteString(l, name);
			lua_rawseti(l, -2, index);
		}
	}
	return 1;
}

int LuaScriptInterface::fileSystem_exists(lua_State * l)
{
	auto filename = tpt_lua_checkByteString(l, 1);

	bool ret = Platform::Stat(filename);
	lua_pushboolean(l, ret);
	return 1;
}

int LuaScriptInterface::fileSystem_isFile(lua_State * l)
{
	auto filename = tpt_lua_checkByteString(l, 1);

	bool ret = Platform::FileExists(filename);
	lua_pushboolean(l, ret);
	return 1;
}

int LuaScriptInterface::fileSystem_isDirectory(lua_State * l)
{
	auto dirname = tpt_lua_checkByteString(l, 1);

	bool ret = Platform::DirectoryExists(dirname);
	lua_pushboolean(l, ret);
	return 1;
}

int LuaScriptInterface::fileSystem_makeDirectory(lua_State * l)
{
	auto dirname = tpt_lua_checkByteString(l, 1);

	int ret = 0;
	ret = Platform::MakeDirectory(dirname);
	lua_pushboolean(l, ret == 0);
	return 1;
}

int LuaScriptInterface::fileSystem_removeDirectory(lua_State * l)
{
	auto directory = tpt_lua_checkByteString(l, 1);

	bool ret = Platform::DeleteDirectory(directory);
	lua_pushboolean(l, ret);
	return 1;
}

int LuaScriptInterface::fileSystem_removeFile(lua_State * l)
{
	auto filename = tpt_lua_checkByteString(l, 1);
	lua_pushboolean(l, Platform::RemoveFile(filename));
	return 1;
}

int LuaScriptInterface::fileSystem_move(lua_State * l)
{
	auto filename = tpt_lua_checkByteString(l, 1);
	auto newFilename = tpt_lua_checkByteString(l, 2);
	lua_pushboolean(l, Platform::RenameFile(filename, newFilename));
	return 1;
}

int LuaScriptInterface::fileSystem_copy(lua_State * l)
{
	auto filename = tpt_lua_checkByteString(l, 1);
	auto newFilename = tpt_lua_checkByteString(l, 2);
	std::vector<char> fileData;
	lua_pushboolean(l, Platform::ReadFile(fileData, filename) && Platform::WriteFile(fileData, newFilename));
	return 1;
}

void LuaScriptInterface::initPlatformAPI()
{
	//Methods
	struct luaL_Reg platformAPIMethods [] = {
		{"platform", platform_platform},
		{"ident", platform_ident},
		{"build", platform_build},
		{"releaseType", platform_releaseType},
		{"exeName", platform_exeName},
		{"restart", platform_restart},
		{"openLink", platform_openLink},
		{"clipboardCopy", platform_clipboardCopy},
		{"clipboardPaste", platform_clipboardPaste},
		{NULL, NULL}
	};
	luaL_register(l, "platform", platformAPIMethods);

	//elem shortcut
	lua_getglobal(l, "platform");
	lua_setglobal(l, "plat");
}

int LuaScriptInterface::platform_platform(lua_State * l)
{
	lua_pushliteral(l, IDENT_PLATFORM);
	return 1;
}

int LuaScriptInterface::platform_ident(lua_State * l)
{
	lua_pushliteral(l, IDENT);
	return 1;
}

int LuaScriptInterface::platform_build(lua_State * l)
{
	lua_pushliteral(l, IDENT_BUILD);
	return 1;
}

int LuaScriptInterface::platform_releaseType(lua_State * l)
{
	lua_pushliteral(l, IDENT_RELTYPE);
	return 1;
}

int LuaScriptInterface::platform_exeName(lua_State * l)
{
	ByteString name = Platform::ExecutableName();
	if (name.length())
		tpt_lua_pushByteString(l, name);
	else
		luaL_error(l, "Error, could not get executable name");
	return 1;
}

int LuaScriptInterface::platform_restart(lua_State * l)
{
	Platform::DoRestart();
	return 0;
}

int LuaScriptInterface::platform_openLink(lua_State * l)
{
	auto uri = tpt_lua_checkByteString(l, 1);
	Platform::OpenURI(uri);
	return 0;
}

int LuaScriptInterface::platform_clipboardCopy(lua_State * l)
{
	tpt_lua_pushByteString(l, ClipboardPull());
	return 1;
}

int LuaScriptInterface::platform_clipboardPaste(lua_State * l)
{
	luaL_checktype(l, 1, LUA_TSTRING);
	ClipboardPush(tpt_lua_optByteString(l, 1, ""));
	return 0;
}


//// Begin Event API

void LuaScriptInterface::initEventAPI()
{
	struct luaL_Reg eventAPIMethods [] = {
		{"register", event_register},
		{"unregister", event_unregister},
		{"getmodifiers", event_getmodifiers},
		{NULL, NULL}
	};
	luaL_register(l, "event", eventAPIMethods);

	lua_getglobal(l, "event");
	lua_setglobal(l, "evt");

	lua_pushinteger(l, LuaEvents::keypress); lua_setfield(l, -2, "keypress");
	lua_pushinteger(l, LuaEvents::keyrelease); lua_setfield(l, -2, "keyrelease");
	lua_pushinteger(l, LuaEvents::textinput); lua_setfield(l, -2, "textinput");
	lua_pushinteger(l, LuaEvents::textediting); lua_setfield(l, -2, "textediting");
	lua_pushinteger(l, LuaEvents::mousedown); lua_setfield(l, -2, "mousedown");
	lua_pushinteger(l, LuaEvents::mouseup); lua_setfield(l, -2, "mouseup");
	lua_pushinteger(l, LuaEvents::mousemove); lua_setfield(l, -2, "mousemove");
	lua_pushinteger(l, LuaEvents::mousewheel); lua_setfield(l, -2, "mousewheel");
	lua_pushinteger(l, LuaEvents::tick); lua_setfield(l, -2, "tick");
	lua_pushinteger(l, LuaEvents::blur); lua_setfield(l, -2, "blur");
	lua_pushinteger(l, LuaEvents::close); lua_setfield(l, -2, "close");
	lua_pushinteger(l, LuaEvents::beforesim); lua_setfield(l, -2, "beforesim");
	lua_pushinteger(l, LuaEvents::aftersim); lua_setfield(l, -2, "aftersim");
}

int LuaScriptInterface::event_register(lua_State * l)
{
	int eventName = luaL_checkinteger(l, 1);
	luaL_checktype(l, 2, LUA_TFUNCTION);
	return LuaEvents::RegisterEventHook(l, ByteString::Build("tptevents-", eventName));
}

int LuaScriptInterface::event_unregister(lua_State * l)
{
	int eventName = luaL_checkinteger(l, 1);
	luaL_checktype(l, 2, LUA_TFUNCTION);
	return LuaEvents::UnregisterEventHook(l, ByteString::Build("tptevents-", eventName));
}

int LuaScriptInterface::event_getmodifiers(lua_State * l)
{
	lua_pushnumber(l, GetModifiers());
	return 1;
}

class RequestHandle
{
public:
	enum RequestType
	{
		normal,
		getAuthToken,
	};

private:
	http::Request *request;
	bool dead = false;
	RequestType type;

	RequestHandle() = default;

	void FinishGetAuthToken(ByteString &data, int &status_out, std::vector<ByteString> &headers)
	{
		headers.clear();
		std::istringstream ss(data);
		Json::Value root;
		try
		{
			ss >> root;
			auto status = root["Status"].asString();
			if (status == "OK")
			{
				status_out = 200;
				data = root["Token"].asString();
			}
			else
			{
				status_out = 403;
				data = status;
			}
		}
		catch (std::exception &e)
		{
			std::cerr << "bad auth response: " << e.what() << std::endl;
			status_out = 600;
			data.clear();
		}
	}

public:
	static int Make(lua_State *l, const ByteString &uri, bool isPost, const ByteString &verb, RequestType type, const std::map<ByteString, ByteString> &post_data, const std::vector<ByteString> &headers)
	{
		auto authUser = Client::Ref().GetAuthUser();
		if (type == getAuthToken && !authUser.UserID)
		{
			lua_pushnil(l);
			lua_pushliteral(l, "not authenticated");
			return 2;
		}
		auto *rh = (RequestHandle *)lua_newuserdata(l, sizeof(RequestHandle));
		if (!rh)
		{
			return 0;
		}
		new(rh) RequestHandle();
		rh->type = type;
		rh->request = new http::Request(uri);
		if (verb.size())
		{
			rh->request->Verb(verb);
		}
		for (auto &header : headers)
		{
			rh->request->AddHeader(header);
		}
		if (isPost)
		{
			rh->request->AddPostData(post_data);
		}
		if (type == getAuthToken)
		{
			rh->request->AuthHeaders(ByteString::Build(authUser.UserID), authUser.SessionID);
		}
		rh->request->Start();
		luaL_newmetatable(l, "HTTPRequest");
		lua_setmetatable(l, -2);
		return 1;
	}

	~RequestHandle()
	{
		if (!Dead())
		{
			Cancel();
		}
	}

	bool Dead() const
	{
		return dead;
	}

	bool Done() const
	{
		return dead || request->CheckDone();
	}

	void Progress(int *total, int *done)
	{
		if (!dead)
		{
			request->CheckProgress(total, done);
		}
	}

	void Cancel()
	{
		if (!dead)
		{
			request->Cancel();
			dead = true;
		}
	}

	ByteString Finish(int &status_out, std::vector<ByteString> &headers)
	{
		ByteString data;
		if (!dead)
		{
			if (request->CheckDone())
			{
				data = request->Finish(&status_out, &headers);
				if (type == getAuthToken && status_out == 200)
				{
					FinishGetAuthToken(data, status_out, headers);
				}
				dead = true;
			}
		}
		return data;
	}
};

static int http_request_gc(lua_State *l)
{
	auto *rh = (RequestHandle *)luaL_checkudata(l, 1, "HTTPRequest");
	rh->~RequestHandle();
	return 0;
}

static int http_request_status(lua_State *l)
{
	auto *rh = (RequestHandle *)luaL_checkudata(l, 1, "HTTPRequest");
	if (rh->Dead())
	{
		lua_pushliteral(l, "dead");
	}
	else if (rh->Done())
	{
		lua_pushliteral(l, "done");
	}
	else
	{
		lua_pushliteral(l, "running");
	}
	return 1;
}

static int http_request_progress(lua_State *l)
{
	auto *rh = (RequestHandle *)luaL_checkudata(l, 1, "HTTPRequest");
	if (!rh->Dead())
	{
		int total, done;
		rh->Progress(&total, &done);
		lua_pushinteger(l, total);
		lua_pushinteger(l, done);
		return 2;
	}
	return 0;
}

static int http_request_cancel(lua_State *l)
{
	auto *rh = (RequestHandle *)luaL_checkudata(l, 1, "HTTPRequest");
	if (!rh->Dead())
	{
		rh->Cancel();
	}
	return 0;
}

static int http_request_finish(lua_State *l)
{
	auto *rh = (RequestHandle *)luaL_checkudata(l, 1, "HTTPRequest");
	if (!rh->Dead())
	{
		int status_out;
		std::vector<ByteString> headers;
		ByteString data = rh->Finish(status_out, headers);
		lua_pushlstring(l, &data[0], data.size());
		lua_pushinteger(l, status_out);
		lua_newtable(l);
		for (auto i = 0; i < int(headers.size()); ++i)
		{
			lua_pushlstring(l, headers[i].data(), headers[i].size());
			lua_rawseti(l, -2, i + 1);
		}
		return 3;
	}
	return 0;
}

static int http_request(lua_State *l, bool isPost)
{
	ByteString uri = tpt_lua_checkByteString(l, 1);
	std::map<ByteString, ByteString> post_data;
	auto headersIndex = 2;
	auto verbIndex = 3;

	if (isPost)
	{
		headersIndex += 1;
		verbIndex += 1;
		if (lua_istable(l, 2))
		{
			lua_pushnil(l);
			while (lua_next(l, 2))
			{
				lua_pushvalue(l, -2);
				post_data.emplace(tpt_lua_toByteString(l, -1), tpt_lua_toByteString(l, -2));
				lua_pop(l, 2);
			}
		}
	}

	std::vector<ByteString> headers;
	if (lua_istable(l, headersIndex))
	{
		auto size = lua_objlen(l, headersIndex);
		if (size)
		{
			for (auto i = 0U; i < size; ++i)
			{
				lua_rawgeti(l, headersIndex, i + 1);
				headers.push_back(tpt_lua_toByteString(l, -1));
				lua_pop(l, 1);
			}
		}
		else
		{
			// old dictionary format
			lua_pushnil(l);
			while (lua_next(l, headersIndex))
			{
				lua_pushvalue(l, -2);
				headers.push_back(tpt_lua_toByteString(l, -1) + ByteString(": ") + tpt_lua_toByteString(l, -2));
				lua_pop(l, 2);
			}
		}
	}

	auto verb = tpt_lua_optByteString(l, verbIndex, "");
	return RequestHandle::Make(l, uri, isPost, verb, RequestHandle::normal, post_data, headers);
}

static int http_get_auth_token(lua_State *l)
{
	return RequestHandle::Make(l, SCHEME SERVER "/ExternalAuth.api?Action=Get&Audience=" + format::URLEncode(tpt_lua_checkByteString(l, 1)), false, {}, RequestHandle::getAuthToken, {}, {});
}

int LuaScriptInterface::http_get(lua_State * l)
{
	return http_request(l, false);
}

int LuaScriptInterface::http_post(lua_State * l)
{
	return http_request(l, true);
}

void LuaScriptInterface::initHttpAPI()
{
	luaL_newmetatable(l, "HTTPRequest");
	lua_pushcfunction(l, http_request_gc);
	lua_setfield(l, -2, "__gc");
	lua_newtable(l);
	struct luaL_Reg httpRequestIndexMethods[] = {
		{ "status", http_request_status },
		{ "progress", http_request_progress },
		{ "cancel", http_request_cancel },
		{ "finish", http_request_finish },
		{ NULL, NULL }
	};
	luaL_register(l, NULL, httpRequestIndexMethods);
	lua_setfield(l, -2, "__index");
	lua_pop(l, 1);
	lua_newtable(l);
	struct luaL_Reg httpMethods[] = {
		{ "get", http_get },
		{ "post", http_post },
		{ "getAuthToken", http_get_auth_token },
		{ NULL, NULL }
	};
	luaL_register(l, NULL, httpMethods);
	lua_setglobal(l, "http");
}

bool LuaScriptInterface::HandleEvent(LuaEvents::EventTypes eventType, Event * event)
{
	return LuaEvents::HandleEvent(this, event, ByteString::Build("tptevents-", eventType));
}

void LuaScriptInterface::OnTick()
{
	lua_getglobal(l, "simulation");
	if (lua_istable(l, -1))
	{
		lua_pushinteger(l, luacon_sim->NUM_PARTS);
		lua_setfield(l, -2, "NUM_PARTS");
	}
	lua_pop(l, 1);
	TickEvent ev;
	HandleEvent(LuaEvents::tick, &ev);
}

int LuaScriptInterface::Command(String command)
{
	lastError = "";
	luacon_hasLastError = false;
	if (command[0] == '!')
	{
		int ret = legacy->Command(command.Substr(1));
		lastError = legacy->GetLastError();
		return ret;
	}
	else
	{
		int level = lua_gettop(l), ret = -1;
		currentCommand = true;
		if (lastCode.length())
			lastCode += "\n";
		lastCode += command;
		ByteString tmp = ("return " + lastCode).ToUtf8();
		ui::Engine::Ref().LastTick(Platform::GetTime());
		luaL_loadbuffer(l, tmp.data(), tmp.size(), "@console");
		if (lua_type(l, -1) != LUA_TFUNCTION)
		{
			lua_pop(l, 1);
			ByteString lastCodeUtf8 = lastCode.ToUtf8();
			luaL_loadbuffer(l, lastCodeUtf8.data(), lastCodeUtf8.size(), "@console");
		}
		if (lua_type(l, -1) != LUA_TFUNCTION)
		{
			lastError = luacon_geterror();
			String err = lastError;
			if (err.Contains("near '<eof>'")) //the idea stolen from lua-5.1.5/lua.c
				lastError = "...";
			else
				lastCode = "";
		}
		else
		{
			lastCode = "";
			ret = lua_pcall(l, 0, LUA_MULTRET, 0);
			if (ret)
			{
				lastError = luacon_geterror();
			}
			else
			{
				String text = "";
				bool hasText = false;
				for (level++; level <= lua_gettop(l); level++)
				{
					luaL_tostring(l, level);
					if (hasText)
					{
						text += ", " + tpt_lua_optString(l, -1, "");
					}
					else
					{
						text = tpt_lua_optString(l, -1, "");
						hasText = true;
					}
					lua_pop(l, 1);
				}
				if (text.length())
				{
					if (lastError.length())
						lastError += "; " + text;
					else
						lastError = text;
				}

			}
		}
		currentCommand = false;
		return ret;
	}
}

int strlcmp(const char* a, const char* b, int len)
{
	while(len)
	{
		if(!*b)
			return 1;
		if(*a>*b)
			return -1;
		if(*a<*b)
			return 1;
		a++;
		b++;
		len--;
	}
	if(!*b)
		return 0;
	return -1;
}

String highlight(String command)
{
	StringBuilder result;
	int pos = 0;
	String::value_type const*raw = command.c_str();
	String::value_type c;
	while ((c = raw[pos]))
	{
		if((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_')
		{
			int len = 0;
			String::value_type w;
			String::value_type const* wstart = raw+pos;
			while((w = wstart[len]) && ((w >= 'A' && w <= 'Z') || (w >= 'a' && w <= 'z') || (w >= '0' && w <= '9') || w == '_'))
				len++;
#define CMP(X) (String(wstart, len) == X)
			if(CMP("and") || CMP("break") || CMP("do") || CMP("else") || CMP("elseif") || CMP("end") || CMP("for") || CMP("function") || CMP("if") || CMP("in") || CMP("local") || CMP("not") || CMP("or") || CMP("repeat") || CMP("return") || CMP("then") || CMP("until") || CMP("while"))
				result << "\x0F\xB5\x89\x01" << String(wstart, len) << "\bw";
			else if(CMP("false") || CMP("nil") || CMP("true"))
				result << "\x0F\xCB\x4B\x16" << String(wstart, len) << "\bw";
			else
				result << "\x0F\x2A\xA1\x98" << String(wstart, len) << "\bw";
#undef CMP
			pos += len;
		}
		else if((c >= '0' && c <= '9') || (c == '.' && raw[pos + 1] >= '0' && raw[pos + 1] <= '9'))
		{
			if(c == '0' && raw[pos + 1] == 'x')
			{
				int len = 2;
				String::value_type w;
				String::value_type const* wstart = raw+pos;
				while((w = wstart[len]) && ((w >= '0' && w <= '9') || (w >= 'A' && w <= 'F') || (w >= 'a' && w <= 'f')))
					len++;
				result << "\x0F\xD3\x36\x82" << String(wstart, len) << "\bw";
				pos += len;
			}
			else
			{
				int len = 0;
				String::value_type w;
				String::value_type const* wstart = raw+pos;
				bool seendot = false;
				while((w = wstart[len]) && ((w >= '0' && w <= '9') || w == '.'))
				{
					if(w == '.')
					{
						if(seendot)
							break;
						else
							seendot = true;
					}
					len++;
				}
				if(w == 'e')
				{
					len++;
					w = wstart[len];
					if(w == '+' || w == '-')
						len++;
					while((w = wstart[len]) && (w >= '0' && w <= '9'))
						len++;
				}
				result << "\x0F\xD3\x36\x82" << String(wstart, len) << "\bw";
				pos += len;
			}
		}
		else if(c == '\'' || c == '"' || (c == '[' && (raw[pos + 1] == '[' || raw[pos + 1] == '=')))
		{
			if(c == '[')
			{
				int len = 1, eqs=0;
				String::value_type w;
				String::value_type const* wstart = raw + pos;
				while((w = wstart[len]) && (w == '='))
				{
					eqs++;
					len++;
				}
				while((w = wstart[len]))
				{
					if(w == ']')
					{
						int nlen = 1;
						String::value_type const* cstart = wstart + len;
						while((w = cstart[nlen]) && (w == '='))
							nlen++;
						if(w == ']' && nlen == eqs+1)
						{
							len += nlen+1;
							break;
						}
					}
					len++;
				}
				result << "\x0F\xDC\x32\x2F" << String(wstart, len) << "\bw";
				pos += len;
			}
			else
			{
				int len = 1;
				String::value_type w;
				String::value_type const* wstart = raw+pos;
				while((w = wstart[len]) && (w != c))
				{
					if(w == '\\' && wstart[len + 1])
						len++;
					len++;
				}
				if(w == c)
					len++;
				result << "\x0F\xDC\x32\x2F" << String(wstart, len) << "\bw";
				pos += len;
			}
		}
		else if(c == '-' && raw[pos + 1] == '-')
		{
			if(raw[pos + 2] == '[')
			{
				int len = 3, eqs = 0;
				String::value_type w;
				String::value_type const* wstart = raw + pos;
				while((w = wstart[len]) && (w == '='))
				{
					eqs++;
					len++;
				}
				while((w = wstart[len]))
				{
					if(w == ']')
					{
						int nlen = 1;
						String::value_type const* cstart = wstart + len;
						while((w = cstart[nlen]) && (w == '='))
							nlen++;
						if(w == ']' && nlen == eqs + 1)
						{
							len += nlen+1;
							break;
						}
					}
					len++;
				}
				result << "\x0F\x85\x99\x01" << String(wstart, len) << "\bw";
				pos += len;
			}
			else
			{
				int len = 2;
				String::value_type w;
				String::value_type const* wstart = raw + pos;
				while((w = wstart[len]) && (w != '\n'))
					len++;
				result << "\x0F\x85\x99\x01" << String(wstart, len) << "\bw";
				pos += len;
			}
		}
		else if(c == '{' || c == '}')
		{
			result << "\x0F\xCB\x4B\x16" << c << "\bw";
			pos++;
		}
		else if(c == '.' && raw[pos + 1] == '.' && raw[pos + 2] == '.')
		{
			result << "\x0F\x2A\xA1\x98...\bw";
			pos += 3;
		}
		else
		{
			result << c;
			pos++;
		}
	}
	return result.Build();
}

String LuaScriptInterface::FormatCommand(String command)
{
	if(command.size() && command[0] == '!')
	{
		return "!"+legacy->FormatCommand(command.Substr(1));
	}
	else
		return highlight(command);
}

LuaScriptInterface::~LuaScriptInterface() {
	delete tptPart;
	for (auto &component_and_ref : grabbed_components)
	{
		luacon_ci->Window->RemoveComponent(component_and_ref.first->GetComponent());
		component_and_ref.second.Clear();
		component_and_ref.first->owner_ref = component_and_ref.second;
		component_and_ref.first->SetParentWindow(nullptr);
	}
	luaChangeTypeHandlers.clear();
	luaCreateAllowedHandlers.clear();
	luaCreateHandlers.clear();
	luaCtypeDrawHandlers.clear();
	lua_el_mode_v.clear();
	lua_el_func_v.clear();
	lua_gr_func_v.clear();
	lua_cd_func_v.clear();
	lua_close(l);
	delete legacy;
}

#ifndef NOHTTP
void LuaScriptInterface::initSocketAPI()
{
	LuaTCPSocket::Open(l);
}
#endif

void tpt_lua_pushByteString(lua_State *L, const ByteString &str)
{
	lua_pushlstring(L, str.data(), str.size());
}

void tpt_lua_pushString(lua_State *L, const String &str)
{
	tpt_lua_pushByteString(L, str.ToUtf8());
}

ByteString tpt_lua_toByteString(lua_State *L, int index)
{
	size_t size;
	if (auto *data = lua_tolstring(L, index, &size))
	{
		return ByteString(data, size);
	}
	return {};
}

String tpt_lua_toString(lua_State *L, int index, bool ignoreError)
{
	return tpt_lua_toByteString(L, index).FromUtf8(ignoreError);
}

ByteString tpt_lua_checkByteString(lua_State *L, int index)
{
	size_t size;
	if (auto *data = luaL_checklstring(L, index, &size))
	{
		return ByteString(data, size);
	}
	return {};
}

String tpt_lua_checkString(lua_State *L, int index, bool ignoreError)
{
	return tpt_lua_checkByteString(L, index).FromUtf8(ignoreError);
}

ByteString tpt_lua_optByteString(lua_State *L, int index, ByteString defaultValue)
{
	if (lua_isnoneornil(L, index))
	{
		return defaultValue;
	}
	return tpt_lua_checkByteString(L, index);
}

String tpt_lua_optString(lua_State *L, int index, String defaultValue, bool ignoreError)
{
	if (lua_isnoneornil(L, index))
	{
		return defaultValue;
	}
	return tpt_lua_checkString(L, index, ignoreError);
}

int tpt_lua_loadstring(lua_State *L, const ByteString &str)
{
	return luaL_loadbuffer(L, str.data(), str.size(), str.data());
}

int tpt_lua_dostring(lua_State *L, const ByteString &str)
{
	return tpt_lua_loadstring(L, str) || lua_pcall(L, 0, LUA_MULTRET, 0);
}

bool tpt_lua_equalsString(lua_State *L, int index, const char *data, size_t size)
{
	return lua_isstring(L, index) && lua_objlen(L, index) == size && !memcmp(lua_tostring(L, index), data, size);
}

#endif
