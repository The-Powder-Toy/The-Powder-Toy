#include "ToolClasses.h"
#include "simulation/Simulation.h"
#include "Lang.h"
//#TPT-Directive ToolClass Tool_NGrv TOOL_NGRV 5
Tool_NGrv::Tool_NGrv()
{
	Identifier = "DEFAULT_TOOL_NGRV";
	Name = "NGRV";
	Colour = PIXPACK(0xAACCFF);
	Description = TEXT_TOOL_NGRV_DESC;
}

int Tool_NGrv::Perform(Simulation * sim, Particle * cpart, int x, int y, float strength)
{
	sim->gravmap[((y/CELL)*(XRES/CELL))+(x/CELL)] = strength*-5.0f;
	return 1;
}

Tool_NGrv::~Tool_NGrv() {}
