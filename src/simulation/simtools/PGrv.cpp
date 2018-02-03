#include "ToolClasses.h"
#include "simulation/Simulation.h"
//#TPT-Directive ToolClass Tool_PGrv TOOL_PGRV 4
Tool_PGrv::Tool_PGrv()
{
	Identifier = "DEFAULT_TOOL_PGRV";
	Name = "PGRV";
	Colour = PIXPACK(0xCCCCFF);
	Description = "Creates a short-lasting gravity well.";
}

int Tool_PGrv::Perform(Simulation * sim, Particle * cpart, int x, int y, int brushX, int brushY, float strength)
{
	sim->gravmap[((y/CELL)*(XRES/CELL))+(x/CELL)] = strength*5.0f;
	return 1;
}

Tool_PGrv::~Tool_PGrv() {}
