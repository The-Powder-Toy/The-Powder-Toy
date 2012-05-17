#include "simulation/Tools.h"
#include "simulation/Simulation.h"
//#TPT-Directive ToolClass Tool_Grav TOOL_GRAV 4
Tool_Grav::Tool_Grav()
{
	Identifier = "DEFAULT_TOOL_GRAV";
	Name = "GRAV";
	Colour = PIXPACK(0xCCCCFF);
	Description = "Creates a short-lasting gravity well";
}

int Tool_Grav::Perform(Simulation * sim, Particle * cpart, int x, int y, float strength)
{
	sim->gravmap[((y/CELL)*(XRES/CELL))+(x/CELL)] += 0.03f*strength;
	return 1;
}

Tool_Grav::~Tool_Grav() {}