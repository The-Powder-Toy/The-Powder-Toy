#include "simulation/ToolCommon.h"

static int perform(Simulation * sim, Particle * cpart, int x, int y, int brushX, int brushY, float strength);

void SimTool::Tool_RSET()
{
	Identifier = "DEFAULT_TOOL_RSET";
	Name = "RSET";
	Colour = PIXPACK(0xDB2020);
	Description = "Resets the elements under brush to their original state.";
	Perform = &perform;
}

static int perform(Simulation * sim, Particle * cpart, int x, int y, int brushX, int brushY, float strength)
{
	if (!cpart)
	{
		return 0;
	}
	int i = (int)(cpart - sim->parts);
	sim->create_part(i, x, y, cpart->type); // Replace current particle with new particle and default properties.
	return 1;
}