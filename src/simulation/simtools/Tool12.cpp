#include "ToolClasses.h"
#include "simulation/Simulation.h"
//#TPT-Directive ToolClass Tool_Tool12 TOOL_TOOL12 12
Tool_Tool12::Tool_Tool12()
{
	Identifier = "DEFAULT_TOOL_TOOL12";
	Name = "T12";
	Colour = PIXPACK(0xEE22EE);
	Description = "Experimental tool.";
}

int Tool_Tool12::Perform(Simulation * sim, Particle * cpart, int x, int y, float strength)
{
	int s;
	int r = sim->pmap[y][x];
	switch (r & 0xFF)
	{
		case PT_E187:
		case PT_E188:
			if (rand() % 100 != 0)
				return 0;
			sim->parts[r>>8].tmp &= 0xFFFFFFFD;
		break;
		default:
			return 0;
	}
	return 1;
}

Tool_Tool12::~Tool_Tool12() {}
