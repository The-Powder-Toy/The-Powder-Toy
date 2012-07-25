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
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
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
	int luacon_mousex, luacon_mousey, luacon_selectedl, luacon_selectedr, luacon_mousebutton, luacon_brushx, luacon_brushy;
	bool luacon_mousedown;
	bool currentCommand;
public:
	lua_State *l;
	LuaScriptInterface(GameModel * m);
	virtual bool OnBrushChanged(int brushType, int rx, int ry);
	virtual bool OnMouseMove(int x, int y, int dx, int dy);
	virtual bool OnMouseDown(int x, int y, unsigned button);
	virtual bool OnMouseUp(int x, int y, unsigned button);
	virtual bool OnMouseWheel(int x, int y, int d);
	virtual bool OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt);
	virtual bool OnKeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt);
	virtual void OnTick();
	virtual int Command(std::string command);
	virtual std::string FormatCommand(std::string command);
	virtual ~LuaScriptInterface();
};


#endif /* LUASCRIPTINTERFACE_H_ */
