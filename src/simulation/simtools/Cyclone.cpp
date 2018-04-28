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
		(x, y) -- turn 90 deg -> (-y, x)
	*/
	// only trigger once per cell (less laggy)
	if ((x%CELL) == 0 && (y%CELL) == 0)
	{
		if(brushX == x && brushY == y)
			return 1;

		float *vx = &sim->air->vx[y / CELL][x / CELL];
		float *vy = &sim->air->vy[y / CELL][x / CELL];

		float dvx = brushX - x;
		float dvy = brushY - y;
		float invsqr = 1/sqrtf(dvx*dvx + dvy*dvy);

		*vx -= (strength / 16) * (-dvy)*invsqr;
		*vy -= (strength / 16) * dvx*invsqr;

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
