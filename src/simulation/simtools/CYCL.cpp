#include "simulation/ToolCommon.h"
#include "simulation/Air.h"

#include <cmath>

static int perform(Simulation * sim, Particle * cpart, int x, int y, int brushX, int brushY, float strength);

void SimTool::Tool_CYCL()
{
	Identifier = "DEFAULT_TOOL_CYCL";
	Name = "CYCL";
	Colour = PIXPACK(0x132f5b);
	Description = "Cyclone, produces swirling air currents";
	Perform = &perform;
}

static int perform(Simulation * sim, Particle * cpart, int x, int y, int brushX, int brushY, float strength)
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

		auto dvx = float(brushX - x);
		auto dvy = float(brushY - y);
		float invsqr = 1/sqrtf(dvx*dvx + dvy*dvy);

		*vx -= (strength / 16) * (-dvy)*invsqr;
		*vy -= (strength / 16) * dvx*invsqr;

		// Clamp velocities
		if (*vx > MAX_PRESSURE)
			*vx = MAX_PRESSURE;
		else if (*vx < MIN_PRESSURE)
			*vx = MIN_PRESSURE;
		if (*vy > MAX_PRESSURE)
			*vy = MAX_PRESSURE;
		else if (*vy < MIN_PRESSURE)
			*vy = MIN_PRESSURE;

	}

	return 1;
}
