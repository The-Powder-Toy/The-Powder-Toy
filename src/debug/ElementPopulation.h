#pragma once

#include "DebugInfo.h"

class Simulation;
class ElementPopulationDebug : public DebugInfo
{
	Simulation * sim;
	float maxAverage;
public:
	ElementPopulationDebug(unsigned int id, Simulation * sim);
	void Draw() override;
	virtual ~ElementPopulationDebug();
};
