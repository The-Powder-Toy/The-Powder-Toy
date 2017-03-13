#include "ToolClasses.h"
#include "simulation/Simulation.h"
//#TPT-Directive ToolClass Tool_Tool11 TOOL_TOOL11 11
Tool_Tool11::Tool_Tool11()
{
	Identifier = "DEFAULT_TOOL_TOOL11";
	Name = "T11";
	Colour = PIXPACK(0xEE22EE);
	Description = "Experimental tool.";
}

int Tool_Tool11::Perform(Simulation * sim, Particle * cpart, int x, int y, float strength)
{
	/* freed */
}

Tool_Tool11::~Tool_Tool11() {}
