#include "ToolClasses.h"
#include "simulation/Simulation.h"
//#TPT-Directive ToolClass Tool_Tool8 TOOL_TOOL8 8
Tool_Tool8::Tool_Tool8()
{
	Identifier = "DEFAULT_TOOL_TOOL8";
	Name = "T8";
	Colour = PIXPACK(0xEE22EE);
	Description = "Experimental tool.";
}

int Tool_Tool8::Perform(Simulation * sim, Particle * cpart, int x, int y, float strength)
{
	int r = sim->pmap[y][x];
	switch (r & 0xFF)
	{
		case PT_E187:
		case PT_E188:
			if (rand() % 100 != 0)
				return 0;
			sim->parts[r>>8].ctype = 0;
		break;
		default:
			return 0;
	}
	return 1;
}

Tool_Tool8::~Tool_Tool8() {}
