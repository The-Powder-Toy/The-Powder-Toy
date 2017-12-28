#pragma once

#include "gui/interface/Point.h"
#include "common/tpt-compat.h"

class DebugInfo
{
public:
	DebugInfo(unsigned int id):debugID(id) { }
	virtual ~DebugInfo() { }
	unsigned int debugID;
	virtual void Draw() {}
	// currentMouse doesn't belong but I don't want to create more hooks at the moment
	virtual bool KeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt, ui::Point currentMouse) { return true; }
};
