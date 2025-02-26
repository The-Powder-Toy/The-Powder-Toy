#include "simulation/ElementCommon.h"
#include "ETRD.h"
#include <algorithm>

static void initDeltaPos();
static void changeType(ELEMENT_CHANGETYPE_FUNC_ARGS);

void Element::Element_ETRD()
{
	Identifier = "DEFAULT_PT_ETRD";
	Name = "ETRD";
	Colour = 0x404040_rgb;
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

	HeatConduct = 251;
	Description = "Electrode. Creates plasma arcs with electricity. (Use sparingly)";

	Properties = TYPE_SOLID|PROP_CONDUCTS|PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	ChangeType = &changeType;

	initDeltaPos();
}

static void changeType(ELEMENT_CHANGETYPE_FUNC_ARGS)
{
	if (sim->etrd_count_valid)
	{
		if (from == PT_ETRD && sim->parts[i].life == 0)
			sim->etrd_life0_count--;
		if (to == PT_ETRD && sim->parts[i].life == 0)
			sim->etrd_life0_count++;
	}
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

static void initDeltaPos()
{
	deltaPos.clear();
	for (int ry = -maxLength; ry <= maxLength; ry++)
		for (int rx = -maxLength; rx <= maxLength; rx++)
		{
			ui::Point d(rx, ry);
			if (std::abs(d.X) + std::abs(d.Y) <= maxLength)
				deltaPos.push_back(ETRD_deltaWithLength(d, int(std::hypot(d.X, d.Y))));
		}
	std::stable_sort(deltaPos.begin(), deltaPos.end(), [](const ETRD_deltaWithLength &a, const ETRD_deltaWithLength &b) {
		return a.length < b.length;
	});
}

int Element_ETRD_nearestSparkablePart(Simulation *sim, int targetId)
{
	if (!sim->elementCount[PT_ETRD])
		return -1;
	if (sim->etrd_count_valid && sim->etrd_life0_count <= 0)
		return -1;

	Particle *parts = sim->parts;
	if (parts[targetId].tmp2 && parts[targetId].tmp > parts[targetId].tmp2) // Invalid range if max is set
		return -1;

	const int maxDistance = int(std::hypot(XRES, YRES));
	int foundDistance = parts[targetId].tmp2 ? std::min(parts[targetId].tmp2, maxDistance) : maxDistance; // tmp2 sets max distance
	int foundI = -1;
	ui::Point targetPos = ui::Point(int(parts[targetId].x), int(parts[targetId].y));

	if (sim->etrd_count_valid)
	{
		// countLife0 doesn't need recalculating, so just focus on finding the nearest particle

		// If the simulation contains lots of particles, check near the target position first since going through all particles will be slow.
		// Threshold = number of positions checked, *2 because it's likely to access memory all over the place (less cache friendly) and there's extra logic needed
		// TODO: probably not optimal if excessive stacking is used
		if (sim->parts.lastActiveIndex > (int)deltaPos.size()*2)
		{
			for (std::vector<ETRD_deltaWithLength>::iterator iter = deltaPos.begin(), end = deltaPos.end(); iter != end; ++iter)
			{
				ETRD_deltaWithLength delta = (*iter);
				ui::Point checkPos = targetPos + delta.d;
				int checkDistance = delta.length;
				if (parts[targetId].tmp >= checkDistance) // tmp sets min distance
				{
					continue;
				}
				if (foundDistance < checkDistance)
				{
					// deltaPos is sorted in order of ascending length, so foundDistance < checkDistance means all later items are further away.
					break;
				}
				if (InBounds(checkPos.X, checkPos.Y) && checkDistance <= foundDistance)
				{
					int r = sim->pmap[checkPos.Y][checkPos.X];
					if (r && TYP(r) == PT_ETRD && !parts[ID(r)].life && ID(r) != targetId && checkDistance < foundDistance)
					{
						foundDistance = checkDistance;
						foundI = ID(r);
					}
				}
			}
		}
		// If neighbor search didn't find a suitable particle, search all particles
		if (foundI < 0)
		{
			for (int i = 0; i <= sim->parts.lastActiveIndex; i++)
			{
				if (parts[i].type == PT_ETRD && !parts[i].life)
				{
					ui::Point checkPos = ui::Point(int(parts[i].x)-targetPos.X, int(parts[i].y)-targetPos.Y);
					int checkDistance = int(std::hypot(checkPos.X, checkPos.Y));
					if (checkDistance < foundDistance && checkDistance > parts[targetId].tmp && i != targetId) // tmp sets min distance
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
		for (int i = 0; i <= sim->parts.lastActiveIndex; i++)
		{
			if (parts[i].type == PT_ETRD && !parts[i].life)
			{
				countLife0++;
				ui::Point checkPos = ui::Point(int(parts[i].x)-targetPos.X, int(parts[i].y)-targetPos.Y);
				int checkDistance = int(std::hypot(checkPos.X, checkPos.Y));
				if (checkDistance < foundDistance && checkDistance > parts[targetId].tmp && i != targetId) // tmp sets min distance
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
