#include "ToolClasses.h"
//#TPT-Directive ToolClass Tool_Heat TOOL_HEAT 0
Tool_Heat::Tool_Heat()
{
	Identifier = "DEFAULT_TOOL_HEAT";
	Name = "HEAT";
	Colour = PIXPACK(0xFFDD00);
	Description = "Heats the targeted element.";
}

int Tool_Heat::Perform(Simulation * sim, Particle * cpart, int x, int y, int brushX, int brushY, float strength)
{
	if(!cpart)
		return 0;
	if (cpart->type == PT_PUMP || cpart->type == PT_GPMP)
		cpart->temp += strength*.1f;
	else
		cpart->temp += strength*2.0f;

	if (cpart->temp > MAX_TEMP)
		cpart->temp = MAX_TEMP;
	else if (cpart->temp < 0)
		cpart->temp = 0;
	return 1;
}

Tool_Heat::~Tool_Heat() {}
