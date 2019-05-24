#include "simulation/ToolCommon.h"

//#TPT-Directive ToolClass Tool_CRCK TOOL_CRCK 1
Tool_CRCK::Tool_CRCK()
{
	Identifier = "DEFAULT_TOOL_CRCK";
	Name = "RESET";
	Colour = PIXPACK(0x0ACAA);
	Description = "Prototype tool, resets the element properties and dcolor.";
}

int Tool_CRCK::Perform(Simulation * sim, Particle * cpart, int x, int y, int brushX, int brushY, float strength)
{
	if (!cpart)
		return 0;
	cpart->temp = 296.0f;
	cpart->life = 0;
	cpart->tmp =  0;
	cpart->tmp2 =  0;
	cpart->ctype= PT_NONE;
	cpart->dcolour = 0x0ACAA;
	if (cpart->PT_GLOW)
	{
		cpart->temp = 316.15f
	}
	if (cpart->PT_LN2)
	{
		cpart->temp = 273.15f
	}
	if (cpart->PT_GLOW)
	{
		cpart->temp = 316.15f
	}

	return 1;
}

Tool_CRCK::~Tool_CRCK() {}
