#ifndef SIMTOOL_H
#define SIMTOOL_H

#include "common/String.h"
#include "graphics/Pixel.h"

class Simulation;
struct Particle;
class SimTool
{
public:
	ByteString Identifier;
	String Name;
	pixel Colour;
	String Description;

	SimTool();
	virtual ~SimTool() {}
	virtual int Perform(Simulation * sim, Particle * cpart, int x, int y, int brushX, int brushY, float strength) { return 0; }
};

#endif
