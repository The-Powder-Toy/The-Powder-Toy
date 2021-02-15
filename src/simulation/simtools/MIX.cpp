#include "simulation/ToolCommon.h"

#include "common/tpt-rand.h"
#include <cmath>

static int perform(Simulation * sim, Particle * cpart, int x, int y, int brushX, int brushY, float strength);

void SimTool::Tool_MIX()
{
	Identifier = "DEFAULT_TOOL_MIX";
	Name = "MIX";
	Colour = PIXPACK(0xFFD090);
	Description = "Mixes particles.";
	Perform = &perform;
}

static int perform(Simulation * sim, Particle * cpart, int x, int y, int brushX, int brushY, float strength)
{
	int thisPart = sim->pmap[y][x];
	if(!thisPart)
		return 0;

	if(random_gen() % 100 != 0)
		return 0;

	int distance = (int)(std::pow(strength, .5f) * 10);

	if(!(sim->elements[TYP(thisPart)].Properties & (TYPE_PART | TYPE_LIQUID | TYPE_GAS)))
		return 0;

	int newX = x + (random_gen() % distance) - (distance/2);
	int newY = y + (random_gen() % distance) - (distance/2);

	if(newX < 0 || newY < 0 || newX >= XRES || newY >= YRES)
		return 0;

	int thatPart = sim->pmap[newY][newX];
	if(!thatPart)
		return 0;

	if ((sim->elements[TYP(thisPart)].Properties&STATE_FLAGS) != (sim->elements[TYP(thatPart)].Properties&STATE_FLAGS))
		return 0;

	sim->pmap[y][x] = thatPart;
	sim->parts[ID(thatPart)].x = float(x);
	sim->parts[ID(thatPart)].y = float(y);

	sim->pmap[newY][newX] = thisPart;
	sim->parts[ID(thisPart)].x = float(newX);
	sim->parts[ID(thisPart)].y = float(newY);

	return 1;
}
