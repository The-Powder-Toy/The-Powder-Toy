#pragma once

#include "DebugInfo.h"

class Simulation;
class DebugParts : public DebugInfo
{
	Simulation * sim;
public:
	DebugParts(unsigned int id, Simulation * sim);
	virtual void Draw();
	virtual ~DebugParts();
};
