#include "ToolClasses.h"
//#TPT-Directive ToolClass Tool_Mix TOOL_MIX 6
Tool_Mix::Tool_Mix()
{
	Identifier = "DEFAULT_TOOL_MIX";
	Name = "MIX";
	Colour = PIXPACK(0xFFD090);
	Description = "Mixes particles.";
}

int Tool_Mix::Perform(Simulation * sim, Particle * cpart, int x, int y, int brushX, int brushY, float strength)
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
	sim->parts[ID(thatPart)].x = x;
	sim->parts[ID(thatPart)].y = y;

	sim->pmap[newY][newX] = thisPart;
	sim->parts[ID(thisPart)].x = newX;
	sim->parts[ID(thisPart)].y = newY;

	return 1;
}

Tool_Mix::~Tool_Mix() {}
