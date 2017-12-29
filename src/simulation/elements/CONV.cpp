#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_CONV PT_CONV 85
Element_CONV::Element_CONV()
{
	Identifier = "DEFAULT_PT_CONV";
	Name = "CONV";
	Colour = PIXPACK(0x0AAB0A);
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

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 251;
	Description = "Converter. Converts everything into whatever it first touches.";

	Properties = TYPE_SOLID|PROP_DRAWONCTYPE|PROP_NOCTYPEDRAW;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_CONV::update;
}

//#TPT-Directive ElementHeader Element_CONV static int update(UPDATE_FUNC_ARGS)
int Element_CONV::update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry;
	int ctype = TYP(parts[i].ctype), ctypeExtra = ID(parts[i].ctype);
	if (ctype<=0 || ctype>=PT_NUM || !sim->elements[ctype].Enabled || ctype==PT_CONV || (ctype==PT_LIFE && (ctypeExtra<0 || ctypeExtra>=NGOL)))
	{
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (BOUNDS_CHECK)
				{
					r = sim->photons[y+ry][x+rx];
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
	else 
	{
		int restrictElement = sim->IsValidElement(parts[i].tmp) ? parts[i].tmp : 0;
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES)
				{
					r = sim->photons[y+ry][x+rx];
					if (!r || (restrictElement && TYP(r) != restrictElement))
						r = pmap[y+ry][x+rx];
					if (!r || (restrictElement && TYP(r) != restrictElement))
						continue;
					if (TYP(r) != PT_CONV && TYP(r) != PT_DMND && TYP(r) != ctype)
					{
						sim->create_part(ID(r), x+rx, y+ry, TYP(parts[i].ctype), ID(parts[i].ctype));
					}
				}
	}
	return 0;
}


Element_CONV::~Element_CONV() {}
