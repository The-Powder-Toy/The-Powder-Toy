#ifdef LUACONSOLE
#ifdef _MSC_VER
#include "Platform.h"
#include "resource.h"
#endif
#include "lua/LuaCompat.h"
void luaopen_crackerk(lua_State *l)
{

#ifndef _MSC_VER
	int eventcompat_luac_sz = 4319;
	const char* eventcompat_luac = "local toggle = Button:new(330,0,15,10, \"TG\", \"Toggle additional menus.\")\ntoggle:action(function(sender)\ntpt.set_pause(1)\ntpt.hud(0)\nlocal FPS = Button:new(330,40,100,10, \"Frame limit\", \" Set frame limit to 120.\")\nlocal info= Button:new(330,100,100,10,\"Version check\", \"\")\nlocal hide= Button:new(330,140,15,10, \"^\", \"Hide.\")\nlocal reset= Button:new(330,80 ,100,10,\"Reset\", \"Reset everything.\")\nlocal deletesparkButton = Button:new(330,20,100,10,\"Interface\", \"shows UI related stuff.\")\nlocal UI = Button:new(613,1,15,421, \"Hidden\", \"UI\")\nlocal UI2 = Button:new(1,385,612,38,\"Hidden\", \"UI\")\nlocal UI3 = Button:new(13,13,110,10,\"Hidden\", \"UI\")\nlocal screen1= Button:new(440,60,20,10,\"Yes\", \"Take the screen shot.\")\nlocal screen2= Button:new(440,70,20,10,\"No\", \"Cancel the screen shot.\")\nlocal screen= Button:new(330,60,100,10,\"Screenshot\", \"Take a screen shot.\")\nlocal UIhidey = Button:new(440,20,40,10,\"Hide\", \"Hides the UI\")\nlocal UIhiden = Button:new(440,30,40,10,\"Show\", \"Shows the UI\")\nlocal Ruler = Button:new(330,120,100,10, \"Ruler\", \"Toggles in game hud ruler.\")\nlocal Ry = Button:new(440,120,40,10,\"Hide\", \"Hides the Ruler\")\nlocal Rn = Button:new(440,130,40,10,\"Show\", \"Shows the Ruler\")\n\n\nRuler:action(function(sender)\ninterface.addComponent(Ry)\ninterface.addComponent(Rn)\nend)\ninterface.addComponent(Ruler)\n\nRy:action(function(sender)\ntpt.setdebug(0X0)\ninterface.removeComponent(Ry)\ninterface.removeComponent(Rn)\nend)\n\nRn:action(function(sender)\n\ntpt.setdebug(0X4)\ninterface.removeComponent(Ry)\ninterface.removeComponent(Rn)\nend)\n\n\n\ndeletesparkButton:action(function(sender)\ntpt.hud(0)\ninterface.addComponent(UIhidey)\ninterface.addComponent(UIhiden)\nend)\ninterface.addComponent(deletesparkButton)\n\nUIhidey:action(function(sender)\ntpt.hud(0)\ninterface.addComponent(UI)\ninterface.addComponent(UI2)\ninterface.addComponent(UI3)\ninterface.removeComponent(UIhiden)\ninterface.removeComponent(UIhidey)\nend)\n\nUIhiden:action(function(sender)\ntpt.hud(0)\ninterface.removeComponent(UI)\ninterface.removeComponent(UI2)\ninterface.removeComponent(UI3)\ninterface.removeComponent(UIhiden)\ninterface.removeComponent(UIhidey)\nend)\n\nFPS:action(function(sender)\ntpt.setfpscap(120)\nend)\ninterface.addComponent(FPS)\n\nscreen1:action(function(sender)\ninterface.removeComponent(screen1)\ninterface.removeComponent(screen2)\ninterface.removeComponent(reset)\ninterface.removeComponent(screen)\ninterface.removeComponent(FPS)\ninterface.removeComponent(deletesparkButton)\ninterface.removeComponent(hide)\ninterface.removeComponent(info)\ninterface.removeComponent(UIhiden)\ninterface.removeComponent(UIhidey)\ninterface.removeComponent(Ruler)\ninterface.removeComponent(Ry)\ninterface.removeComponent(Rn)\n\ntpt.drawtext(270,360,\"Shot taken using Cracker1000's script\", 0, 0,255, 255)\ntpt.fillrect(1,1,710,710,55,124,255,30)\n\ntpt.screenshot(0)\nend)\n\nscreen2:action(function(sender)\ninterface.removeComponent(screen1)\ninterface.removeComponent(screen2)\nend)\n\nscreen:action(function(sender)\ninterface.addComponent(screen1)\ninterface.addComponent(screen2)\nend)\ninterface.addComponent(screen)\n\nreset:action(function(sender)\ntpt.display_mode(3)\ntpt.watertest(0)\nsim.edgeMode(0) \ntpt.setfpscap(60)\ntpt.setwindowsize(1)\ntpt.newtonian_gravity(0)\ntpt.decorations_enable(0)\nsim.resetPressure()\ntpt.ambient_heat(0)\nsim.resetTemp()\ntpt.reset_velocity(1,380,300,300)\ninterface.removeComponent(UI)\ninterface.removeComponent(UI2)\ninterface.removeComponent(UI3)\ninterface.removeComponent(UIhiden)\ninterface.removeComponent(UIhidey)\ninterface.removeComponent(Ry)\ninterface.removeComponent(Rn)\ntpt.setdebug(0X0)\n\nsim.clearSim()\nend)\ninterface.addComponent(reset)\n\n\ninfo:action(function(sender)\ntpt.message_box(\"Cracker1000's Mod script help\",\"Version 2.0 beta\")\nend)\ninterface.addComponent(info)\n\nhide:action(function(sender)\ninterface.removeComponent(reset)\ninterface.removeComponent(screen)\ninterface.removeComponent(FPS)\ninterface.removeComponent(deletesparkButton)\ninterface.removeComponent(hide)\ninterface.removeComponent(info)\ninterface.removeComponent(screen1)\ninterface.removeComponent(screen2)\ninterface.removeComponent(UIhiden)\ninterface.removeComponent(UIhidey)\ninterface.removeComponent(Ruler)\ninterface.removeComponent(Ry)\ninterface.removeComponent(Rn)\n\ntpt.hud(1)\ntpt.set_pause(0)\nend)\ninterface.addComponent(hide)\n\nend)\ninterface.addComponent(toggle)\n\n\n\n\n\n\n\n";
	luaL_loadbuffer(l, eventcompat_luac, eventcompat_luac_sz, "@crackerk.lua");
	lua_pcall(l, 0, 0, 0);
#else
	unsigned int size = 0;
	const char* data = NULL;
	Platform::LoadFileInResource(IDI_CRK, LUASCRIPT, size, data);
	char *buffer = new char[size + 1];
	::memcpy(buffer, data, size);
	buffer[size] = 0;
	luaL_loadbuffer(l, buffer, size, "@crackerk.lua");
	lua_pcall(l, 0, 0, 0);
	delete[] buffer;
#endif
}
#endif
