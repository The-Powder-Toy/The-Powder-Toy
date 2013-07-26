#pragma once

#include "DebugInfo.h"

class Simulation;
class ElementPopulationDebug : public DebugInfo
{
	Simulation * sim;
	float maxAverage;
public:
	ElementPopulationDebug(Simulation * sim);
	virtual void Draw(ui::Point position);
	virtual ~ElementPopulationDebug();
};
