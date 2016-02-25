#pragma once

#include "gui/interface/Point.h"
#include "Config.h"

class DebugInfo
{
public:
	DebugInfo(unsigned int id):ID(id) { }
	virtual ~DebugInfo() { }
	unsigned int ID;
	virtual void Draw() {}
	// currentMouse doesn't belong but I don't want to create more hooks at the moment
	virtual bool KeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt, ui::Point currentMouse) { return true; }
};
