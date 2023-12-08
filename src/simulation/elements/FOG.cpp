#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_FOG()
{
	Identifier = "DEFAULT_PT_FOG";
	Name = "FOG";
	Colour = 0xAAAAAA_rgb;
	MenuVisible = 1;
	MenuSection = SC_GAS;
	Enabled = 1;

	Advection = 0.8f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.4f;
	Loss = 0.70f;
	Collision = -0.1f;
	Gravity = 0.0f;
	Diffusion = 0.99f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 30;

	Weight = 1;

	DefaultProperties.temp = 243.15f;
	HeatConduct = 100;
	Description = "Fog, created when an electric current is passed through RIME.";

	Properties = TYPE_GAS|PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 373.15f;
	HighTemperatureTransition = PT_WTRV;

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	for (auto rx = -1; rx <= 1; rx++)
	{
		for (auto ry = -1; ry <= 1; ry++)
		{
			if (rx || ry)
			{
				auto r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((elements[TYP(r)].Properties&TYPE_SOLID) && sim->rng.chance(1, 10) && parts[i].life==0 && !(TYP(r)==PT_CLNE || TYP(r)==PT_PCLN)) // TODO: should this also exclude BCLN?
				{
					sim->part_change_type(i,x,y,PT_RIME);
				}
				if (TYP(r)==PT_SPRK)
				{
					parts[i].life += sim->rng.between(0, 19);
				}
			}
		}
	}
	return 0;
}
