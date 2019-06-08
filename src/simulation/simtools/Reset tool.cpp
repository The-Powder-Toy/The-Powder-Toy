#include "simulation/ToolCommon.h"

//#TPT-Directive ToolClass Tool_CRCK TOOL_CRCK 1
Tool_CRCK::Tool_CRCK()
{
	Identifier = "DEFAULT_TOOL_CRCK";
	Name = "RESET";
	Colour = PIXPACK(0xDB2020);
	Description = "Reset tool, resets particle's properties to default.";
}

int Tool_CRCK::Perform(Simulation * sim, Particle * cpart, int x, int y, int brushX, int brushY, float strength)

{
	if (!cpart)
	{
		return 0;
	}
	int i = (int)(cpart - sim->parts);
	sim->create_part(i, x, y, cpart->type); // Replace current particle with new particle and default properties.
	return 1;
}

Tool_CRCK::~Tool_CRCK() {}
