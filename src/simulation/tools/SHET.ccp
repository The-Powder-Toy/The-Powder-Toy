#include "ToolClasses.h"
//#TPT-Directive ToolClass Tool_Shet TOOL_SHET 7
Tool_Shet::Tool_Shet()
{
  Identifier = "DEFAULT_TOOL_SHET";
	Name = "SHET";
	Colour = PIXPACK(0xFFDD00);
	Description = "Heats the targeted element superfast.";
}

int Tool_Shet::Perform(Simulation * sim, Particle * cpart, int x, int y, float strength)
{
	if(!cpart)
		return 0;
	if (cpart->type == PT_PUMP || cpart->type == PT_GPMP)
		cpart->temp += strength*.1f;
	else
		cpart->temp += strength*10.0f;

	if (cpart->temp > MAX_TEMP)
		cpart->temp = MAX_TEMP;
	else if (cpart->temp < 0)
		cpart->temp = 0;
	return 1;
}

Tool_Shet::~Tool_Shet() {}
