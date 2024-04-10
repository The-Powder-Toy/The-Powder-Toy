#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_BCLN()
{
	Identifier = "DEFAULT_PT_BCLN";
	Name = "BCLN";
	Colour = 0xFFD040_rgb;
	MenuVisible = 1;
	MenuSection = SC_SPECIAL;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.97f;
	Loss = 0.50f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 12;

	Weight = 100;

	HeatConduct = 251;
	Description = "Breakable Clone.";

	Properties = TYPE_SOLID | PROP_PHOTPASS | PROP_LIFE_DEC | PROP_LIFE_KILL_DEC | PROP_NOCTYPEDRAW;
	CarriesTypeIn = 1U << FIELD_CTYPE;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &update;
	CtypeDraw = &Element::ctypeDrawVInTmp;
}

constexpr float ADVECTION = 0.1f;

static int update(UPDATE_FUNC_ARGS)
{
	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	if (!parts[i].life && sim->pv[y/CELL][x/CELL]>4.0f)
		parts[i].life = sim->rng.between(80, 119);
	if (parts[i].life)
	{
		parts[i].vx += ADVECTION*sim->vx[y/CELL][x/CELL];
		parts[i].vy += ADVECTION*sim->vy[y/CELL][x/CELL];
	}
	if (parts[i].ctype<=0 || parts[i].ctype>=PT_NUM || !elements[parts[i].ctype].Enabled)
	{
		for (auto rx = -1; rx <= 1; rx++)
		{
			for (auto ry = -1; ry <= 1; ry++)
			{
				auto r = sim->photons[y+ry][x+rx];
				if (!r)
					r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				auto rt = TYP(r);
				if (rt!=PT_CLNE && rt!=PT_PCLN &&
					rt!=PT_BCLN && rt!=PT_STKM &&
					rt!=PT_PBCN && rt!=PT_STKM2 &&
					rt<PT_NUM)
				{
					parts[i].ctype = rt;
					if (rt==PT_LIFE || rt==PT_LAVA)
						parts[i].tmp = parts[ID(r)].ctype;
				}
			}
		}
	}
	else
	{
		if (parts[i].ctype==PT_LIFE) sim->create_part(-1, x + sim->rng.between(-1, 1), y + sim->rng.between(-1, 1), PT_LIFE, parts[i].tmp);
		else if (parts[i].ctype!=PT_LIGH || sim->rng.chance(1, 30))
		{
			int np = sim->create_part(-1, x + sim->rng.between(-1, 1), y + sim->rng.between(-1, 1), TYP(parts[i].ctype));
			if (np>=0)
			{
				if (parts[i].ctype==PT_LAVA && parts[i].tmp>0 && parts[i].tmp<PT_NUM && elements[parts[i].tmp].HighTemperatureTransition==PT_LAVA)
					parts[np].ctype = parts[i].tmp;
			}
		}
	}
	return 0;
}
