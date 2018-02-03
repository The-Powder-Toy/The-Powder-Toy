#include "ToolClasses.h"
#include "simulation/Air.h"
//#TPT-Directive ToolClass Tool_Cycl TOOL_CYCL 8
Tool_Cycl::Tool_Cycl()
{
	Identifier = "DEFAULT_TOOL_CYCL";
	Name = "CYCL";
	Colour = PIXPACK(0x132f5b);
	Description = "Cyclone. Produces Swirling Air Currents";
}

int Tool_Cycl::Perform(Simulation * sim, Particle * cpart, int x, int y, int brushX, int brushY, float strength)
{
	/* 
		Air velocity calculation.
		Air velocity Y = Sine of cell angle
		Angle of cell is calculated via cells X/Y relation to the brush center and ArcTangent
		Angle has 1.57 Radians added to it (90 degrees) in order to make the velocity be at 90 degrees to the centerpoint.
		Ditto for X, except X uses Cosine

		Strength is changed on CELL != 4 due to the fact that the calculation will be run 16x as much.
	*/
	
	// Cell size of 4 optimization (greatly needed)
	
#if CELL == 4
	if ((x & 3) == 0 && (y & 3) == 0)
	{
		sim->air->vy[y / CELL][x / CELL] -= (strength / 16) * (sin(1.57f + (atan2(brushY - y, brushX - x))));
		sim->air->vx[y / CELL][x / CELL] -= (strength / 16) * (cos(1.57f + (atan2(brushY - y, brushX - x))));
	}


#else
	sim->air->vy[y / CELL][x / CELL] -= (strength / 256) * (sin(1.57f + (atan2(brushY - y, brushX - x))));
	sim->air->vx[y / CELL][x / CELL] -= (strength / 256) * (cos(1.57f + (atan2(brushY - y, brushX - x))));

#endif /* CELL == 4*/

	// Clamp velocities

	if (sim->air->vx[y / CELL][x / CELL] > 256.0f)
	{
		sim->air->vx[y / CELL][x / CELL] = 256.0f;
	}
	if (sim->air->vy[y / CELL][x / CELL] > 256.0f)
	{
		sim->air->vy[y / CELL][x / CELL] = 256.0f;
	}

	if (sim->air->vx[y / CELL][x / CELL] < -256.0f)
	{
		sim->air->vx[y / CELL][x / CELL] = -256.0f;
	}
	if (sim->air->vy[y / CELL][x / CELL] < -256.0f)
	{
		sim->air->vy[y / CELL][x / CELL] = -256.0f;
	}


	return 1;
}

Tool_Cycl::~Tool_Cycl() {}
