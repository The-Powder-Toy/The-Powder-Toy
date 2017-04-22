#include "ToolClasses.h"
#include "simulation/Simulation.h"
//#TPT-Directive ToolClass Tool_Tool9 TOOL_TOOL9 9
Tool_Tool9::Tool_Tool9()
{
	Identifier = "DEFAULT_TOOL_TOOL9";
	Name = "T9";
	Colour = PIXPACK(0xEE22EE);
	Description = "Experimental tool.";
}

int Tool_Tool9::Perform(Simulation * sim, Particle * cpart, int x, int y, float strength)
{
	int s;
	int r = sim->pmap[y][x];
	switch (r & 0xFF)
	{
		case PT_E187:
		case PT_E188:
			if (rand() % 100 != 0)
				return 0;
			sim->parts[r>>8].tmp |= 1;
		break;
		default:
			return 0;
	}
	return 1;
}

Tool_Tool9::~Tool_Tool9() {}
