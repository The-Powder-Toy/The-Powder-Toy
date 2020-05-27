#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);
static void create(ELEMENT_CREATE_FUNC_ARGS);

void Element::Element_UVRD()
{
	Identifier = "DEFAULT_PT_UVRD";
	Name = "UV";
	Colour = PIXPACK(0x4B0082);
	MenuVisible = 1;
	MenuSection = SC_NUCLEAR;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 1.00f;
	Loss = 1.00f;
	Collision = -0.99f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = -1;

	DefaultProperties.temp = R_TEMP + 100.0f + 273.15f;
	HeatConduct = 251;
	Description = "UV rays emitted by SUN, reacts differently with different elements.";

	Properties = TYPE_ENERGY | PROP_LIFE_DEC | PROP_LIFE_KILL_DEC;

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
	int r, rx, ry, np, rndstore;
	for (rx = -1; rx < 2; rx++)
		for (ry = -1; ry < 2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y + ry][x + rx];
				switch (TYP(r))
				{
				case PT_PLNT:
					if (RNG::Ref().chance(1, 40))
					{
						np = sim->create_part(ID(r), x + rx, y + ry, PT_VINE);
						if (np < 0) continue;
						parts[np].life = 0;
					}
					break;
				case PT_WATR:
				case PT_DSTW:
				case PT_CBNW:
				case PT_SLTW:
				{
					np = sim->create_part(ID(r), x + rx, y + ry, PT_WTRV);
					if (np < 0) continue;
					parts[np].life = 0;
				}
				break;
				case PT_PSCN:
				{
					np = sim->create_part(ID(r), x + rx, y + ry, PT_SPRK);
				}
				break;
				case PT_STKM:
				case PT_FIGH:
				case PT_STKM2:
				{
					parts[ID(r)].life = 100;
				}
				break;
				}
			}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
	{
		*pixel_mode |= NO_DECO;
		return 1;
	}
static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	float a = RNG::Ref().between(0, 359) * 3.14159f / 180.0f;
	sim->parts[i].life = 680;
	sim->parts[i].vx = 2.0f * cosf(a);
	sim->parts[i].vy = 2.0f * sinf(a);
}
