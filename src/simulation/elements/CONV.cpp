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
	int ctype = parts[i].ctype&0xFF, ctypeExtra = parts[i].ctype>>8;
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
					if ((r&0xFF)!=PT_CLNE && (r&0xFF)!=PT_PCLN &&
					    (r&0xFF)!=PT_BCLN && (r&0xFF)!=PT_STKM &&
					    (r&0xFF)!=PT_PBCN && (r&0xFF)!=PT_STKM2 &&
					    (r&0xFF)!=PT_CONV && (r&0xFF)<PT_NUM)
					{
						parts[i].ctype = r&0xFF;
						if ((r&0xFF)==PT_LIFE)
							parts[i].ctype |= (parts[r>>8].ctype << 8);
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
					if (!r || (restrictElement && (r&0xFF)!=restrictElement))
						r = pmap[y+ry][x+rx];
					if (!r || (restrictElement && (r&0xFF)!=restrictElement))
						continue;
					if((r&0xFF)!=PT_CONV && (r&0xFF)!=PT_DMND && (r&0xFF)!=ctype)
					{
						sim->create_part(r>>8, x+rx, y+ry, parts[i].ctype&0xFF, parts[i].ctype>>8);
					}
				}
	}
	return 0;
}


Element_CONV::~Element_CONV() {}
