#include "simulation/Tools.h"
//#TPT-Directive ToolClass Tool_Heat TOOL_HEAT 0
Tool_Heat::Tool_Heat()
{
	Identifier = "DEFAULT_TOOL_HEAT";
	Name = "HEAT";
	Colour = PIXPACK(0xFFDD00);
	Description = "Heats particles";
}

int Tool_Heat::Perform(Simulation * sim, Particle * cpart, int x, int y, float strength)
{
	if(!cpart)
		return 0;
	cpart->temp += strength;
	if(cpart->temp > MAX_TEMP)
		cpart->temp = MAX_TEMP;
	if(cpart->temp < 0)
		cpart->temp = 0;
	return 1;
}

Tool_Heat::~Tool_Heat() {}