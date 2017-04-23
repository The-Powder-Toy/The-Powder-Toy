#include "ToolClasses.h"
#include "simulation/Simulation.h"
//#TPT-Directive ToolClass Tool_Tool12 TOOL_TOOL12 12
Tool_Tool12::Tool_Tool12()
{
	Identifier = "DEFAULT_TOOL_TOOL12";
	Name = "T12";
	Colour = PIXPACK(0xEE22EE);
	Description = "Experimental tool.";
}

int Tool_Tool12::Perform(Simulation * sim, Particle * cpart, int x, int y, float strength)
{
	/* freed */
}

Tool_Tool12::~Tool_Tool12() {}
