#include "ToolClasses.h"
#include "simulation/Simulation.h"
//#TPT-Directive ToolClass Tool_Tool11 TOOL_TOOL11 11
Tool_Tool11::Tool_Tool11()
{
	Identifier = "DEFAULT_TOOL_TOOL11";
	Name = "T11";
	Colour = PIXPACK(0xEE22EE);
	Description = "Experimental tool.";
}

int Tool_Tool11::Perform(Simulation * sim, Particle * cpart, int x, int y, float strength)
{
	int s;
	int r = sim->pmap[y][x];
	switch (r & 0xFF)
	{
		case PT_E187:
			if (rand() % 100 != 0)
				return 0;
			sim->parts[r>>8].tmp |= 0x2;
		break;
		default:
			return 0;
	}
	return 1;
}

Tool_Tool11::~Tool_Tool11() {}
