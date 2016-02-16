#include "ToolClasses.h"
#include "simulation/Air.h"
#include "Lang.h"
//#TPT-Directive ToolClass Tool_Air TOOL_AIR 2
Tool_Air::Tool_Air()
{
	Identifier = "DEFAULT_TOOL_AIR";
	Name = "AIR";
	Colour = PIXPACK(0xFFFFFF);
	Description = TEXT_TOOL_AIR_DESC;
}

int Tool_Air::Perform(Simulation * sim, Particle * cpart, int x, int y, float strength)
{
	sim->air->pv[y/CELL][x/CELL] += strength*0.05f;

	if (sim->air->pv[y/CELL][x/CELL] > 256.0f)
		sim->air->pv[y/CELL][x/CELL] = 256.0f;
	else if (sim->air->pv[y/CELL][x/CELL] < -256.0f)
		sim->air->pv[y/CELL][x/CELL] = -256.0f;
	return 1;
}

Tool_Air::~Tool_Air() {}
