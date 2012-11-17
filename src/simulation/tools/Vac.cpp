#include "simulation/Tools.h"
#include "simulation/Air.h"
//#TPT-Directive ToolClass Tool_Vac TOOL_VAC 2
Tool_Vac::Tool_Vac()
{
	Identifier = "DEFAULT_TOOL_VAC";
	Name = "VAC";
	Colour = PIXPACK(0x303030);
	Description = "Removes air pressure";
}

int Tool_Vac::Perform(Simulation * sim, Particle * cpart, int x, int y, float strength)
{
	sim->air->pv[y/CELL][x/CELL] -= 0.03f*strength;
	if(sim->air->pv[y/CELL][x/CELL] > 256.0f)
		sim->air->pv[y/CELL][x/CELL] = 256.0f;
	if(sim->air->pv[y/CELL][x/CELL] < -256.0f)
		sim->air->pv[y/CELL][x/CELL] = -256.0f;
	return 1;
}

Tool_Vac::~Tool_Vac() {}
