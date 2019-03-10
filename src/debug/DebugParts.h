#pragma once

#include "DebugInfo.h"

class Simulation;
class DebugParts : public DebugInfo
{
	Simulation * sim;
public:
	DebugParts(unsigned int id, Simulation * sim);
	void Draw() override;
	virtual ~DebugParts();
};
