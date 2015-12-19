#include <algorithm>
#include "simulation/Elements.h"

//#TPT-Directive ElementClass Element_ETRD PT_ETRD 50
Element_ETRD::Element_ETRD()
{
	Identifier = "DEFAULT_PT_ETRD";
	Name = "ETRD";
	Colour = PIXPACK(0x404040);
	MenuVisible = 1;
	MenuSection = SC_ELEC;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.90f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 1;

	Weight = 100;

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 251;
	Description = "Electrode. Creates a surface that allows Plasma arcs. (Use sparingly)";

	Properties = TYPE_SOLID|PROP_CONDUCTS|PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = NULL;

	Element_ETRD::initDeltaPos();
}

class ETRD_deltaWithLength
{
public:
	ETRD_deltaWithLength(ui::Point a, int b):
		d(a),
		length(b)
	{

	}

	ui::Point d;
	int length;
};

const int maxLength = 12;
std::vector<ETRD_deltaWithLength> deltaPos;

bool compareFunc(const ETRD_deltaWithLength &a, const ETRD_deltaWithLength &b)
{
	return a.length < b.length;
}

//#TPT-Directive ElementHeader Element_ETRD static void initDeltaPos()
void Element_ETRD::initDeltaPos()
{
	deltaPos.clear();
	for (int ry = -maxLength; ry <= maxLength; ry++)
		for (int rx = -maxLength; rx <= maxLength; rx++)
		{
			ui::Point d(rx, ry);
			if (std::abs(d.X) + std::abs(d.Y) <= maxLength)
				deltaPos.push_back(ETRD_deltaWithLength(d, std::abs(d.X) + std::abs(d.Y)));
		}
	std::stable_sort(deltaPos.begin(), deltaPos.end(), compareFunc);
}

//#TPT-Directive ElementHeader Element_ETRD static int nearestSparkablePart(Simulation *sim, int targetId)
int Element_ETRD::nearestSparkablePart(Simulation *sim, int targetId)
{
	if (!sim->elementCount[PT_ETRD])
		return -1;
	if (sim->etrd_count_valid && sim->etrd_life0_count <= 0)
		return -1;

	Particle *parts = sim->parts;
	int foundDistance = XRES + YRES;
	int foundI = -1;
	ui::Point targetPos = ui::Point(parts[targetId].x, parts[targetId].y);

	if (sim->etrd_count_valid)
	{
		// countLife0 doesn't need recalculating, so just focus on finding the nearest particle

		// If the simulation contains lots of particles, check near the target position first since going through all particles will be slow.
		// Threshold = number of positions checked, *2 because it's likely to access memory all over the place (less cache friendly) and there's extra logic needed
		// TODO: probably not optimal if excessive stacking is used
		if (sim->parts_lastActiveIndex > (int)deltaPos.size()*2)
		{
			for (std::vector<ETRD_deltaWithLength>::iterator iter = deltaPos.begin(), end = deltaPos.end(); iter != end; ++iter)
			{
				ETRD_deltaWithLength delta = (*iter);
				ui::Point checkPos = targetPos + delta.d;
				int checkDistance = delta.length;
				if (foundDistance < checkDistance)
				{
					// deltaPos is sorted in order of ascending length, so foundDistance < checkDistance means all later items are further away.
					break;
				}
				if (sim->InBounds(checkPos.X, checkPos.Y) && checkDistance <= foundDistance)
				{
					int r = sim->pmap[checkPos.Y][checkPos.X];
					if (r && (r&0xFF) == PT_ETRD && !parts[r>>8].life && r>>8 != targetId && checkDistance < foundDistance)
					{
						foundDistance = checkDistance;
						foundI = r>>8;
					}
				}
			}
		}
		// If neighbor search didn't find a suitable particle, search all particles
		if (foundI < 0)
		{
			for (int i = 0; i <= sim->parts_lastActiveIndex; i++)
			{
				if (parts[i].type == PT_ETRD && !parts[i].life)
				{
					ui::Point checkPos = ui::Point(parts[i].x-targetPos.X, parts[i].y-targetPos.Y);
					int checkDistance = std::abs(checkPos.X) + std::abs(checkPos.Y);
					if (checkDistance < foundDistance && i != targetId)
					{
						foundDistance = checkDistance;
						foundI = i;
					}
				}
			}
		}
	}
	else
	{
		// Recalculate countLife0, and search for the closest suitable particle
		int countLife0 = 0;
		for (int i = 0; i <= sim->parts_lastActiveIndex; i++)
		{
			if (parts[i].type == PT_ETRD && !parts[i].life)
			{
				countLife0++;
				ui::Point checkPos = ui::Point(parts[i].x-targetPos.X, parts[i].y-targetPos.Y);
				int checkDistance = std::abs(checkPos.X) + std::abs(checkPos.Y);
				if (checkDistance < foundDistance && i != targetId)
				{
					foundDistance = checkDistance;
					foundI = i;
				}
			}
		}
		sim->etrd_life0_count = countLife0;
		sim->etrd_count_valid = true;
	}
	return foundI;
}

Element_ETRD::~Element_ETRD() {}
