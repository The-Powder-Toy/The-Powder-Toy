#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_O2()
{
	Identifier = "DEFAULT_PT_O2";
	Name = "OXYG";
	Colour = 0x80A0FF_rgb;
	MenuVisible = 1;
	MenuSection = SC_GAS;
	Enabled = 1;

	Advection = 2.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.99f;
	Loss = 0.30f;
	Collision = -0.1f;
	Gravity = 0.0f;
	Diffusion = 3.0f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 1;

	HeatConduct = 70;
	Description = "Oxygen gas. Ignites easily.";

	Properties = TYPE_GAS;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = 90.0f;
	LowTemperatureTransition = PT_LO2;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	auto &sd = SimulationData::CRef();
	auto &can_move = sd.can_move;
	for (auto rx = -2; rx <= 2; rx++)
	{
		for (auto ry = -2; ry <= 2; ry++)
		{
			if (rx || ry)
			{
				auto r = pmap[y+ry][x+rx];
				if (!r)
					continue;

				if (TYP(r)==PT_FIRE)
				{
					parts[ID(r)].temp += sim->rng.between(0, 99);
					if (parts[ID(r)].tmp & 0x01)
						parts[ID(r)].temp = 3473;
					parts[ID(r)].tmp |= 2;

					sim->create_part(i,x,y,PT_FIRE);
					parts[i].temp += sim->rng.between(0, 99);
					parts[i].tmp |= 2;
				}
				else if (TYP(r)==PT_PLSM && !(parts[ID(r)].tmp&4))
				{
					sim->create_part(i,x,y,PT_FIRE);
					parts[i].temp += sim->rng.between(0, 99);
					parts[i].tmp |= 2;
				}
			}
		}
	}
	if (parts[i].temp > 9973.15 && sim->pv[y/CELL][x/CELL] > 250.0f)
	{
		auto gravx = sim->gravOut.forceX[Vec2{ x, y } / CELL];
		auto gravy = sim->gravOut.forceY[Vec2{ x, y } / CELL];
		if (gravx*gravx + gravy*gravy > 400)
		{
			if (sim->rng.chance(1, 5))
			{
				int j;
				sim->create_part(i,x,y,PT_BRMT);

				j = sim->create_part(-3,x,y,PT_NEUT);
				if (j != -1)
					parts[j].temp = MAX_TEMP;
				j = sim->create_part(-3,x,y,PT_PHOT);
				if (j != -1)
				{
					parts[j].temp = MAX_TEMP;
					parts[j].tmp = 0x1;
				}
				auto rx = x + sim->rng.between(-1, 1), ry = y + sim->rng.between(-1, 1), r = TYP(pmap[ry][rx]);
				if (can_move[PT_PLSM][r] || r == PT_O2)
				{
					j = sim->create_part(-3,rx,ry,PT_PLSM);
					if (j > -1)
					{
						parts[j].temp = MAX_TEMP;
						parts[j].tmp |= 4;
					}
				}
				j = sim->create_part(-3,x,y,PT_GRVT);
				if (j != -1)
					parts[j].temp = MAX_TEMP;
				parts[i].temp = MAX_TEMP;
				sim->pv[y/CELL][x/CELL] = MAX_PRESSURE;
			}
		}
	}
	return 0;
}
