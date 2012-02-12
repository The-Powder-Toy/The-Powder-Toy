/*
 * LuaScriptInterface.h
 *
 *  Created on: Feb 11, 2012
 *      Author: Simon
 */

#ifndef LUASCRIPTINTERFACE_H_
#define LUASCRIPTINTERFACE_H_

extern "C"
{
#include <lua5.1/lua.h>
#include <lua5.1/lauxlib.h>
#include <lua5.1/lualib.h>
}

#include "CommandInterface.h"
#include "simulation/Simulation.h";

//Because lua only has bindings for C, we're going to have to go outside "outside" the LuaScriptInterface, this means we can only have one instance :(

#define LOCAL_LUA_DIR "Lua"

#define LUACON_MDOWN 1
#define LUACON_MUP 2
#define LUACON_MPRESS 3
#define LUACON_KDOWN 1
#define LUACON_KUP 2

//Bitmasks for things that might need recalculating after changes to tpt.el
#define LUACON_EL_MODIFIED_CANMOVE 0x1
#define LUACON_EL_MODIFIED_GRAPHICS 0x2
#define LUACON_EL_MODIFIED_MENUS 0x4

class LuaScriptInterface: public CommandInterface {
public:
	lua_State *l;
	LuaScriptInterface(GameModel * m);
	virtual void Tick();
	virtual int Command(std::string command);
	virtual std::string FormatCommand(std::string command);
	virtual ~LuaScriptInterface();
};


#endif /* LUASCRIPTINTERFACE_H_ */
