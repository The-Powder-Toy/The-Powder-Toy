#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_GLOW PT_GLOW 66
Element_GLOW::Element_GLOW()
{
	Identifier = "DEFAULT_PT_GLOW";
	Name = "GLOW";
	Colour = PIXPACK(0x445464);
	MenuVisible = 1;
	MenuSection = SC_LIQUID;
	Enabled = 1;

	Advection = 0.3f;
	AirDrag = 0.02f * CFDS;
	AirLoss = 0.98f;
	Loss = 0.80f;
	Collision = 0.0f;
	Gravity = 0.15f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 2;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 2;

	Weight = 40;

	Temperature = R_TEMP+20.0f+273.15f;
	HeatConduct = 44;
	Description = "Glow, Glows under pressure.";

	Properties = TYPE_LIQUID | PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_GLOW::update;
	Graphics = &Element_GLOW::graphics;
}

//#TPT-Directive ElementHeader Element_GLOW static int update(UPDATE_FUNC_ARGS)
int Element_GLOW::update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (TYP(r)==PT_WATR && !(rand()%400))
				{
					sim->kill_part(i);
					sim->part_change_type(ID(r),x+rx,y+ry,PT_DEUT);
					parts[ID(r)].life = 10;
					return 1;
				}
			}
	parts[i].ctype = sim->pv[y/CELL][x/CELL]*16;
	parts[i].tmp = abs((int)((sim->vx[y/CELL][x/CELL]+sim->vy[y/CELL][x/CELL])*16.0f)) + abs((int)((parts[i].vx+parts[i].vy)*64.0f));

	return 0;
}


//#TPT-Directive ElementHeader Element_GLOW static int graphics(GRAPHICS_FUNC_ARGS)
int Element_GLOW::graphics(GRAPHICS_FUNC_ARGS)

{
	*firer = restrict_flt(cpart->temp-(275.13f+32.0f), 0, 128)/50.0f;
	*fireg = restrict_flt(cpart->ctype, 0, 128)/50.0f;
	*fireb = restrict_flt(cpart->tmp, 0, 128)/50.0f;

	*colr = restrict_flt(64.0f+cpart->temp-(275.13f+32.0f), 0, 255);
	*colg = restrict_flt(64.0f+cpart->ctype, 0, 255);
	*colb = restrict_flt(64.0f+cpart->tmp, 0, 255);

	*pixel_mode |= FIRE_ADD;
	return 0;
}


Element_GLOW::~Element_GLOW() {}
