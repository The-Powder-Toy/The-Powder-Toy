#include "simulation/Tools.h"
//#TPT-Directive ToolClass Tool_Cool TOOL_COOL 1
Tool_Cool::Tool_Cool()
{
	Identifier = "DEFAULT_TOOL_COOL";
	Name = "COOL";
	Colour = PIXPACK(0x00DDFF);
	Description = "Cools particles";
}

int Tool_Cool::Perform(Simulation * sim, Particle * cpart, int x, int y, float strength)
{
	if(!cpart)
		return 0;
	cpart->temp -= strength;
	return 1;
}

Tool_Cool::~Tool_Cool() {}