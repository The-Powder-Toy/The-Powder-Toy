#include "ToolClasses.h"
#include "simulation/Simulation.h"
//#TPT-Directive ToolClass Tool_Tool10 TOOL_TOOL10 10
Tool_Tool10::Tool_Tool10()
{
	Identifier = "DEFAULT_TOOL_TOOL10";
	Name = "T10";
	Colour = PIXPACK(0xEE22EE);
	Description = "Experimental tool.";
}

int Tool_Tool10::Perform(Simulation * sim, Particle * cpart, int x, int y, float strength)
{
	/* freed */
}

Tool_Tool10::~Tool_Tool10() {}
