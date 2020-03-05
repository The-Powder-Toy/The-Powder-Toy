#include "common/tpt-minmax.h"
#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_VINE()
{
	Identifier = "DEFAULT_PT_VINE";
	Name = "VINE";
	Colour = PIXPACK(0x079A00);
	MenuVisible = 1;
	MenuSection = SC_SOLIDS;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.95f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 20;
	Explosive = 0;
	Meltable = 0;
	Hardness = 10;

	Weight = 100;

	HeatConduct = 65;
	Description = "Vine, can grow along WOOD.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 573.0f;
	HighTemperatureTransition = PT_FIRE;

	DefaultProperties.tmp = 1;

	Update = &update;
	Graphics = &graphics; // this used to be missing, maybe for a reason?
}

static int update(UPDATE_FUNC_ARGS)
{
	int r, np, rx, ry, rndstore = RNG::Ref().gen();
	rx = (rndstore % 3) - 1;
	rndstore >>= 2;
	ry = (rndstore % 3) - 1;
	rndstore >>= 2;
	if (BOUNDS_CHECK && (rx || ry))
	{
		r = pmap[y+ry][x+rx];
		if (!(rndstore % 15))
			sim->part_change_type(i, x, y, PT_PLNT);
		else if (!r)
		{
			np = sim->create_part(-1,x+rx,y+ry,PT_VINE);
			if (np<0) return 0;
			parts[np].temp = parts[i].temp;
			sim->part_change_type(i,x,y,PT_PLNT);
		}
	}
	if (parts[i].temp > 350 && parts[i].temp > parts[i].tmp2)
		parts[i].tmp2 = (int)parts[i].temp;
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	float maxtemp = std::max((float)cpart->tmp2, cpart->temp);
	if (maxtemp > 300)
	{
		*colr += (int)restrict_flt((maxtemp-300)/5,0,58);
		*colg -= (int)restrict_flt((maxtemp-300)/2,0,102);
		*colb += (int)restrict_flt((maxtemp-300)/5,0,70);
	}
	if (maxtemp < 273)
	{
		*colg += (int)restrict_flt((273-maxtemp)/4,0,255);
		*colb += (int)restrict_flt((273-maxtemp)/1.5,0,255);
	}
	return 0;
}
