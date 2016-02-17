#include "ToolClasses.h"
#include "Lang.h"
//#TPT-Directive ToolClass Tool_Cool TOOL_COOL 1
Tool_Cool::Tool_Cool()
{
	Identifier = "DEFAULT_TOOL_COOL";
	Name = "COOL";
	Colour = PIXPACK(0x00DDFF);
	Description = TEXT_TOOL_COOL_DESC;
}

int Tool_Cool::Perform(Simulation * sim, Particle * cpart, int x, int y, float strength)
{
	if(!cpart)
		return 0;
	if (cpart->type == PT_PUMP || cpart->type == PT_GPMP)
		cpart->temp -= strength*.1f;
	else
		cpart->temp -= strength*2.0f;

	if (cpart->temp > MAX_TEMP)
		cpart->temp = MAX_TEMP;
	else if (cpart->temp < 0)
		cpart->temp = 0;
	return 1;
}

Tool_Cool::~Tool_Cool() {}
