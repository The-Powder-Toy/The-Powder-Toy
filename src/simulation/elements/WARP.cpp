#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);
static void create(ELEMENT_CREATE_FUNC_ARGS);

void Element::Element_WARP()
{
	Identifier = "DEFAULT_PT_WARP";
	Name = "WARP";
	Colour = PIXPACK(0x101010);
	MenuVisible = 1;
	MenuSection = SC_NUCLEAR;
	Enabled = 1;

	Advection = 0.8f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.9f;
	Loss = 0.70f;
	Collision = -0.1f;
	Gravity = 0.0f;
	Diffusion = 3.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 30;

	Weight = 1;

	HeatConduct = 100;
	Description = "Displaces other elements.";

	Properties = TYPE_GAS|PROP_LIFE_DEC|PROP_LIFE_KILL;

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
	Create = &create;
}

static int update(UPDATE_FUNC_ARGS)
{
	int trade, r, rx, ry;
	if (parts[i].tmp2>2000)
	{
		parts[i].temp = 10000;
		sim->pv[y/CELL][x/CELL] += (parts[i].tmp2/5000) * CFDS;
		if (RNG::Ref().chance(1, 50))
			sim->create_part(-3, x, y, PT_ELEC);
	}
	for ( trade = 0; trade<5; trade ++)
	{
		rx = RNG::Ref().between(-1, 1);
		ry = RNG::Ref().between(-1, 1);
		if (BOUNDS_CHECK && (rx || ry))
		{
			r = pmap[y+ry][x+rx];
			if (!r)
				continue;
			if (TYP(r)!=PT_WARP&&TYP(r)!=PT_STKM&&TYP(r)!=PT_STKM2&&TYP(r)!=PT_DMND&&TYP(r)!=PT_CLNE&&TYP(r)!=PT_BCLN&&TYP(r)!=PT_PCLN)
			{
				parts[i].x = parts[ID(r)].x;
				parts[i].y = parts[ID(r)].y;
				parts[ID(r)].x = float(x);
				parts[ID(r)].y = float(y);
				parts[ID(r)].vx = RNG::Ref().chance(-2, 1) + 0.5f;
				parts[ID(r)].vy = float(RNG::Ref().between(-2, 1));
				parts[i].life += 4;
				pmap[y][x] = r;
				pmap[y+ry][x+rx] = PMAP(i, parts[i].type);
				trade = 5;
			}
		}
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	*colr = *colg = *colb = *cola = 0;
	*pixel_mode &= ~PMODE;
	return 0;
}

static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	sim->parts[i].life = RNG::Ref().between(70, 164);
}
