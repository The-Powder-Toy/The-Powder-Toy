#include "ToolClasses.h"
#include "simulation/Simulation.h"
//#TPT-Directive ToolClass Tool_Tool10 TOOL_TOOL10 10
Tool_Tool10::Tool_Tool10()
{
	Identifier = "DEFAULT_TOOL_TOOL10";
	Name = "T10";
	Colour = PIXPACK(0xEE22EE);
	Description = "Experimental tool.";
}

int Tool_Tool10::Perform(Simulation * sim, Particle * cpart, int x, int y, float strength)
{
	int s;
	int r = sim->pmap[y][x];
	switch (r & 0xFF)
	{
		case PT_E187:
			if (rand() % 100 != 0)
				return 0;
			sim->parts[r>>8].tmp &= 0xFFFFFFFE;
		break;
		default:
			return 0;
	}
	return 1;
}

Tool_Tool10::~Tool_Tool10() {}
