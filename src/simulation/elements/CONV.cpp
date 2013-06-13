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
	Description = "Solid. Converts everything into whatever it first touches.";
	
	State = ST_NONE;
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
	if (parts[i].ctype<=0 || parts[i].ctype>=PT_NUM || !sim->elements[parts[i].ctype].Enabled || parts[i].ctype==PT_CONV || (parts[i].ctype==PT_LIFE && (parts[i].tmp<0 || parts[i].tmp>=NGOL)))
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
							parts[i].tmp = parts[r>>8].ctype;
					}
				}
	}
	else 
	{
		bool life = parts[i].ctype==PT_LIFE;
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES)
				{
					r = sim->photons[y+ry][x+rx];
					if (!r)
						r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if((r&0xFF)!=PT_CONV && (r&0xFF)!=PT_DMND && (r&0xFF)!=parts[i].ctype)
					{
						if (life) sim->create_part(r>>8, x+rx, y+ry, parts[i].ctype|(parts[i].tmp<<8));
						else sim->create_part(r>>8, x+rx, y+ry, parts[i].ctype);
					}
				}
	}
	return 0;
}


Element_CONV::~Element_CONV() {}
