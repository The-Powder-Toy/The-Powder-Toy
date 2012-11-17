#include "simulation/Tools.h"
#include "simulation/Air.h"
//#TPT-Directive ToolClass Tool_Air TOOL_AIR 3
Tool_Air::Tool_Air()
{
	Identifier = "DEFAULT_TOOL_AIR";
	Name = "AIR";
	Colour = PIXPACK(0xFFFFFF);
	Description = "Creates air pressure";
}

int Tool_Air::Perform(Simulation * sim, Particle * cpart, int x, int y, float strength)
{
	sim->air->pv[y/CELL][x/CELL] += 0.03f*strength;
	if(sim->air->pv[y/CELL][x/CELL] > 256.0f)
		sim->air->pv[y/CELL][x/CELL] = 256.0f;
	if(sim->air->pv[y/CELL][x/CELL] < -256.0f)
		sim->air->pv[y/CELL][x/CELL] = -256.0f;
	return 1;
}

Tool_Air::~Tool_Air() {}