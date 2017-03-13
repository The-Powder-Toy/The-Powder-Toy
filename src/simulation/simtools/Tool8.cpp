#include "ToolClasses.h"
#include "simulation/Simulation.h"
//#TPT-Directive ToolClass Tool_Tool8 TOOL_TOOL8 8
Tool_Tool8::Tool_Tool8()
{
	Identifier = "DEFAULT_TOOL_TOOL8";
	Name = "T8";
	Colour = PIXPACK(0xEE22EE);
	Description = "Experimental tool.";
}

int Tool_Tool8::Perform(Simulation * sim, Particle * cpart, int x, int y, float strength)
{
	/* freed */
}

Tool_Tool8::~Tool_Tool8() {}
