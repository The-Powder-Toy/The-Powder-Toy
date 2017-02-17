#include "ToolClasses.h"
#include "simulation/Simulation.h"
//#TPT-Directive ToolClass Tool_Tool7 TOOL_TOOL7 7
Tool_Tool7::Tool_Tool7()
{
	Identifier = "DEFAULT_TOOL_TOOL7";
	Name = "T7";
	Colour = PIXPACK(0xCCCCFF);
	Description = "Experimental tool.";
}

int Tool_Tool7::Perform(Simulation * sim, Particle * cpart, int x, int y, float strength)
{
	int r = sim->pmap[y][x];
	switch (r & 0xFF)
	{
		case PT_E187:
			if (rand() % 100 != 0)
				return 0;
			sim->parts[r>>8].ctype ^= 1;
		break;
		default:
			return 0;
	}
	return 1;
}

Tool_Tool7::~Tool_Tool7() {}
