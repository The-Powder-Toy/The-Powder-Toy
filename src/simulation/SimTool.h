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

	int (*Perform)(Simulation * sim, Particle * cpart, int x, int y, int brushX, int brushY, float strength);

	SimTool();

#define TOOL_NUMBERS_DECLARE
#include "ToolNumbers.h"
#undef TOOL_NUMBERS_DECLARE
};

#endif
