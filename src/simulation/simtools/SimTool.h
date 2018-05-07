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
	ByteString Identifier;
	ByteString Name;
	pixel Colour;
	String Description;

	SimTool();
	virtual ~SimTool() {}
	virtual int Perform(Simulation * sim, Particle * cpart, int x, int y, int brushX, int brushY, float strength) { return 0; }
};

#endif
