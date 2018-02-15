#include "ToolClasses.h"
#include "simulation/Air.h"
//#TPT-Directive ToolClass Tool_Cycl TOOL_CYCL 7
Tool_Cycl::Tool_Cycl()
{
	Identifier = "DEFAULT_TOOL_CYCL";
	Name = "CYCL";
	Colour = PIXPACK(0x132f5b);
	Description = "Cyclone, produces swirling air currents";
}

int Tool_Cycl::Perform(Simulation * sim, Particle * cpart, int x, int y, int brushX, int brushY, float strength)
{
	/* 
		Air velocity calculation.
		Air velocity X = cosine of cell angle
		Angle of cell is calculated via cells X/Y relation to the brush center and arctangent
		Angle has 1.57 radians added to it (90 degrees) in order to make the velocity be at 90 degrees to the centerpoint.
		Ditto for X, except X uses sine
	*/
	// only trigger once per cell (less laggy)
	if ((x%CELL) == 0 && (y%CELL) == 0)
	{
		float *vx = &sim->air->vx[y/CELL][x/CELL];
		float *vy = &sim->air->vy[y/CELL][x/CELL];

		*vx -= (strength / 16) * (cos(1.57f + (atan2(brushY - y, brushX - x))));
		*vy -= (strength / 16) * (sin(1.57f + (atan2(brushY - y, brushX - x))));

		// Clamp velocities
		if (*vx > 256.0f)
			*vx = 256.0f;
		else if (*vx < -256.0f)
			*vx = -256.0f;
		if (*vy > 256.0f)
			*vy = 256.0f;
		else if (*vy < -256.0f)
			*vy = -256.0f;

	}

	return 1;
}

Tool_Cycl::~Tool_Cycl() {}
