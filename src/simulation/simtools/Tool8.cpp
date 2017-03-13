#include "ToolClasses.h"
#include "simulation/Simulation.h"
//#TPT-Directive ToolClass Tool_Tool8 TOOL_TOOL8 8
Tool_Tool8::Tool_Tool8()
{
	Identifier = "DEFAULT_TOOL_TOOL8";
	Name = "FRCT";
	Colour = PIXPACK(0xEE22EE);
	Description = "Fractalizer!";
}

int Tool_Tool8::Perform(Simulation * sim, Particle * cpart, int x, int y, float strength)
{
	if(!cpart)
	{
		int i1 = sim->create_part(-1, x, y, PT_E189, 13);
		if (i1 >= 0)
		{
			sim->parts[i1].ctype = 0xFF000000 | ((x & 0xFF) << 16) | ((y & 0xFF) << 8) | ((x * y) & 0xFF);
		}
	}
}

Tool_Tool8::~Tool_Tool8() {}
