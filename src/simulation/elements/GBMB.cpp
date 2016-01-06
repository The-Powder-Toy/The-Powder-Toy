#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_GBMB PT_GBMB 157
Element_GBMB::Element_GBMB()
{
	Identifier = "DEFAULT_PT_GBMB";
	Name = "GBMB";
	Colour = PIXPACK(0x1144BB);
	MenuVisible = 1;
	MenuSection = SC_FORCE;
	Enabled = 1;

	Advection = 0.6f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.98f;
	Loss = 0.95f;
	Collision = 0.0f;
	Gravity = 0.1f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 20;

	Weight = 30;

	Temperature = R_TEMP-2.0f	+273.15f;
	HeatConduct = 29;
	Description = "Gravity bomb. Sticks to the first object it touches then produces a strong gravity push.";

	Properties = TYPE_PART|PROP_LIFE_DEC|PROP_LIFE_KILL_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_GBMB::update;
	Graphics = &Element_GBMB::graphics;
}

//#TPT-Directive ElementHeader Element_GBMB static int update(UPDATE_FUNC_ARGS)
int Element_GBMB::update(UPDATE_FUNC_ARGS)
{
	int rx,ry,r;
	if (parts[i].life<=0)
	{
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
			{
				if (BOUNDS_CHECK)
				{
					r = pmap[y+ry][x+rx];
					if(!r)
						continue;
					if((r&0xFF)!=PT_BOMB && (r&0xFF)!=PT_GBMB &&
					   (r&0xFF)!=PT_CLNE && (r&0xFF)!=PT_PCLN &&
					   (r&0xFF)!=PT_DMND)
					{
						parts[i].life=60;
						break;
					}
				}
			}
	}
	if (parts[i].life>20)
		sim->gravmap[(y/CELL)*(XRES/CELL)+(x/CELL)] = 20;
	else if (parts[i].life>=1)
		sim->gravmap[(y/CELL)*(XRES/CELL)+(x/CELL)] = -80;
	return 0;
}


//#TPT-Directive ElementHeader Element_GBMB static int graphics(GRAPHICS_FUNC_ARGS)
int Element_GBMB::graphics(GRAPHICS_FUNC_ARGS)

{
	if (cpart->life <= 0) {
		*pixel_mode |= PMODE_FLARE;
	}
	else
	{
		*pixel_mode |= PMODE_SPARK;
	}
	return 0;
}


Element_GBMB::~Element_GBMB() {}
