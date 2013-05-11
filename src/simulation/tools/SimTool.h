#ifndef SIMTOOL_H
#define SIMTOOL_H

#include "simulation/Simulation.h"
#include "graphics/Renderer.h"
#include "simulation/Elements.h"

class Simulation;
struct Particle;
class SimTool
{
public:
	char *Identifier;
	char *Name;
	pixel Colour;
	char *Description;
	
	SimTool();
	virtual ~SimTool() {}
	virtual int Perform(Simulation * sim, Particle * cpart, int x, int y, float strength) { return 0; }
};

#endif
