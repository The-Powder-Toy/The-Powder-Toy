#include "ToolClasses.h"
//#TPT-Directive ToolClass Tool_Scol TOOL_SCOL 6
Tool_Scol::Tool_Scol()
{
  Identifier = "DEFAULT_TOOL_Scol";
	Name = "SCOL";
	Colour = PIXPACK(0x00DDFF);
	Description = "Superfast Cool. Cools the targeted element.";
}

int Tool_Scol::Perform(Simulation * sim, Particle * cpart, int x, int y, float strength)
{
	if(!cpart)
		return 0;
	if (cpart->type == PT_PUMP || cpart->type == PT_GPMP)
		cpart->temp -= strength*.1f;
	else
		cpart->temp -= strength*10.0f;

	if (cpart->temp > MAX_TEMP)
		cpart->temp = MAX_TEMP;
	else if (cpart->temp < 0)
		cpart->temp = 0;
	return 1;
}

Tool_Scol::~Tool_Scol() {}
