#pragma once
#include "Config.h"

#include "gui/interface/Point.h"

class DebugInfo
{
public:
	DebugInfo(unsigned int id):debugID(id) { }
	virtual ~DebugInfo() { }
	unsigned int debugID;
	virtual void Draw() {}
	// currentMouse doesn't belong but I don't want to create more hooks at the moment
	virtual bool KeyPress(int key, int scan, bool shift, bool ctrl, bool alt, ui::Point currentMouse) { return true; }
};
