#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_FELE PT_FELE 174
Element_FELE::Element_FELE()
{
	Identifier = "DEFAULT_PT_FELE";
	Name = "FELE";
	Colour = PIXPACK(0xFFFFA0);
	MenuVisible = 1;
	MenuSection = SC_ELEC;
	Enabled = 1;
	
	Advection = 0.4f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 1.0f;
	Loss = 0.30f;
	Collision = 0.0f;
	Gravity = 0.1f;
	Diffusion = 0.0f;
	HotAir = 0.000f	* CFDS;
	Falldown = 2;
	
	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;
	
	Weight = 1;
	
	Temperature = R_TEMP+273.15f;
	HeatConduct = 1;
	Description = "Liquid Electricity.";
	
	State = ST_LIQUID;
	Properties = TYPE_LIQUID|PROP_LIFE_KILL_DEC;
	
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;
	
	Update = &Element_FELE::update;
	Graphics = &Element_FELE::graphics;
}

//#TPT-Directive ElementHeader Element_FELE static int update(UPDATE_FUNC_ARGS)
int Element_FELE::update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry, rt;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				rt = r&0xFF;
				if ((sim->elements[r&0xFF].Properties&PROP_CONDUCTS) && parts[r>>8].life==0 && parts[r>>8].ctype!=PT_SPRK)
				{
					parts[r>>8].ctype = parts[r>>8].type;
					sim->part_change_type(r>>8,x+rx,y+ry,PT_SPRK);
					parts[r>>8].life = 4;
				}

				if(parts[i].tmp==1 && (r&0xFF)==PT_SPRK && parts[r>>8].life==1)
				{
					sim->kill_part(i);
				}

				if(parts[i].tmp2==1 && (r&0xFF)==PT_FELE)
				{
					parts[r>>8].vx += isign(rx);
					parts[r>>8].vy += isign(ry);
				}

			}
	return(0);
}


//#TPT-Directive ElementHeader Element_FELE static int graphics(GRAPHICS_FUNC_ARGS)
int Element_FELE::graphics(GRAPHICS_FUNC_ARGS)

{
	*firea = 160;
	*fireg = 192;
	*fireb = 255;
	*firer = 144;
	*pixel_mode |= FIRE_ADD;
	return NULL;
}


Element_FELE::~Element_FELE() {}
