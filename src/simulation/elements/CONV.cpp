#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_CONV()
{
	Identifier = "DEFAULT_PT_CONV";
	Name = "CONV";
	Colour = 0x0AAB0A_rgb;
	MenuVisible = 1;
	MenuSection = SC_SPECIAL;
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
	Description = "Converter. Converts everything into whatever it first touches.";

	Properties = TYPE_SOLID | PROP_NOCTYPEDRAW;
	CarriesTypeIn = (1U << FIELD_CTYPE) | (1U << FIELD_TMP);

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &update;
	CtypeDraw = &Element::ctypeDrawVInCtype;
}

static int update(UPDATE_FUNC_ARGS)
{
	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	int ctype = TYP(parts[i].ctype);
	if (ctype<=0 || ctype>=PT_NUM || !elements[ctype].Enabled || ctype==PT_CONV)
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
				int rt = TYP(r);
				if (rt != PT_CLNE && rt != PT_PCLN &&
				    rt != PT_BCLN && rt != PT_STKM &&
				    rt != PT_PBCN && rt != PT_STKM2 &&
				    rt != PT_CONV && rt < PT_NUM)
				{
					parts[i].ctype = rt;
					if (rt == PT_LIFE)
						parts[i].ctype |= PMAPID(parts[ID(r)].ctype);
				}
			}
		}
	}
	else
	{
		int restrictElement = sd.IsElement(parts[i].tmp) ? parts[i].tmp : 0;
		for (auto rx = -1; rx <= 1; rx++)
		{
			for (auto ry = -1; ry <= 1; ry++)
			{
				if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES)
				{
					auto r = sim->photons[y+ry][x+rx];
					if (!r || (restrictElement && ((TYP(r) == restrictElement) == (parts[i].tmp2 == 1))))
						r = pmap[y+ry][x+rx];
					if (!r || (restrictElement && ((TYP(r) == restrictElement) == (parts[i].tmp2 == 1))))
						continue;
					if (TYP(r) != PT_CONV && TYP(r) != PT_DMND && TYP(r) != ctype)
					{
						sim->create_part(ID(r), x+rx, y+ry, TYP(parts[i].ctype), ID(parts[i].ctype));
					}
				}
			}
		}
	}
	return 0;
}
