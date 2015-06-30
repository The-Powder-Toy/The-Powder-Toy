#pragma once

#include "gui/interface/Point.h"

class DebugInfo
{
public:
	DebugInfo(unsigned int id):ID(id) { }
	virtual ~DebugInfo() { }
	unsigned int ID;
	virtual void Draw() {}
};
