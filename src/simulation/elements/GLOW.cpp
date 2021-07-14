#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_GLOW()
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

	DefaultProperties.temp = R_TEMP + 20.0f + 273.15f;
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

	Update = &update;
	Graphics = &graphics;
}

static int update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (TYP(r)==PT_WATR && RNG::Ref().chance(1, 400))
				{
					sim->kill_part(i);
					sim->part_change_type(ID(r),x+rx,y+ry,PT_DEUT);
					parts[ID(r)].life = 10;
					return 1;
				}
			}
	int ctype = int(sim->pv[y/CELL][x/CELL]*16);
	if (ctype < 0)
		ctype = 0;
	parts[i].ctype = ctype;
	parts[i].tmp = abs((int)((sim->vx[y/CELL][x/CELL]+sim->vy[y/CELL][x/CELL])*16.0f)) + abs((int)((parts[i].vx+parts[i].vy)*64.0f));

	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{

	*firer = 16+int(restrict_flt(cpart->temp-(275.13f+32.0f), 0, 128)/2.0f);
	*fireg = 16+int(restrict_flt(float(cpart->ctype), 0, 128)/2.0f);
	*fireb = 16+int(restrict_flt(float(cpart->tmp), 0, 128)/2.0f);
	*firea = 64;

	*colr = int(restrict_flt(64.0f+cpart->temp-(275.13f+32.0f), 0, 255));
	*colg = int(restrict_flt(64.0f+cpart->ctype, 0, 255));
	*colb = int(restrict_flt(64.0f+cpart->tmp, 0, 255));

	int rng = RNG::Ref().between(1, 32); //
	if(((*colr) + (*colg) + (*colb)) > (256 + rng)) {
		*colr -= 54;
		*colg -= 54;
		*colb -= 54;
		*pixel_mode |= FIRE_ADD;
		*pixel_mode |= PMODE_GLOW | PMODE_ADD;
		*pixel_mode &= ~PMODE_FLAT;
	} else {
		*pixel_mode |= PMODE_BLUR;
	}
	return 0;
}
