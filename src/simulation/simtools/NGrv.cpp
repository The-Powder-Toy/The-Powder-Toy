#include "ToolClasses.h"
#include "simulation/Simulation.h"
//#TPT-Directive ToolClass Tool_NGrv TOOL_NGRV 5
Tool_NGrv::Tool_NGrv()
{
	Identifier = "DEFAULT_TOOL_NGRV";
	Name = "NGRV";
	Colour = PIXPACK(0xAACCFF);
	Description = "Creates a short-lasting negative gravity well.";
}

int Tool_NGrv::Perform(Simulation * sim, Particle * cpart, int x, int y, int brushX, int brushYy, float strength)
{
	sim->gravmap[((y/CELL)*(XRES/CELL))+(x/CELL)] = strength*-5.0f;
	return 1;
}

Tool_NGrv::~Tool_NGrv() {}
