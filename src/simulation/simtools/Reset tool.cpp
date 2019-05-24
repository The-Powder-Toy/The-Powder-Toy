#include "simulation/ToolCommon.h"

//#TPT-Directive ToolClass Tool_CRCK TOOL_CRCK 1
Tool_CRCK::Tool_CRCK()
{
	Identifier = "DEFAULT_TOOL_CRCK";
	Name = "RESET";
	Colour = PIXPACK(0xDB2020);
	Description = "Reset tool, resets element's properties to default.";
}

int Tool_CRCK::Perform(Simulation * sim, Particle * cpart, int x, int y, int brushX, int brushY, float strength)
{
	int r, rx, ry, np;
	if (!cpart)
	{
		return 0;
	}
	cpart->temp = 296.0f;
	cpart->life = 0;
	cpart->tmp = 0;
	cpart->tmp2 = 0;
	cpart->ctype = PT_NONE;
	cpart->dcolour = 0x0ACAA;

	 if  (PT_LNTG)
	{
		cpart->temp = 70.15f;
		cpart->dcolour = 0x0ACAA;
	}

	if (PT_GLOW)
	{
		cpart->temp = R_TEMP + 20.0f + 273.15f;
		cpart->dcolour = 0x0ACAA;
	}

	if (PT_LO2)
	{
		cpart->temp = 80.0f;
		cpart->dcolour = 0x0ACAA;
	}
	if (PT_LAVA)
	{
		cpart->temp = R_TEMP + 1500.0f + 273.15f;
		cpart->ctype = PT_NONE;
		cpart->dcolour = 0x0ACAA;
	}
	if (PT_VIRS)
	{
		cpart->dcolour = 0x0ACAA;
		cpart->temp = 72.0f + 273.15f;
	}
	if (PT_PHOT)
	{
		R_TEMP + 900.0f + 273.15f;
	}
	
	return 1;
}

Tool_CRCK::~Tool_CRCK() {}
