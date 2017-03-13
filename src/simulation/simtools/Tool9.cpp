#include "ToolClasses.h"
#include "simulation/Simulation.h"
//#TPT-Directive ToolClass Tool_Tool9 TOOL_TOOL9 9
Tool_Tool9::Tool_Tool9()
{
	Identifier = "DEFAULT_TOOL_TOOL9";
	Name = "T9";
	Colour = PIXPACK(0xEE22EE);
	Description = "Experimental tool.";
}

int Tool_Tool9::Perform(Simulation * sim, Particle * cpart, int x, int y, float strength)
{
	/* freed */
}

Tool_Tool9::~Tool_Tool9() {}
