#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_BANG PT_BANG 139
Element_BANG::Element_BANG()
{
	Identifier = "DEFAULT_PT_BANG";
	Name = "TNT";
	Colour = PIXPACK(0xC05050);
	MenuVisible = 1;
	MenuSection = SC_EXPLOSIVE;
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
	HeatConduct = 88;
	Description = "TNT, explodes all at once.";

	Properties = TYPE_SOLID | PROP_NEUTPENETRATE;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_BANG::update;
}

//#TPT-Directive ElementHeader Element_BANG static int update(UPDATE_FUNC_ARGS)
int Element_BANG::update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry;
	if(parts[i].tmp==0)
	{
		if(parts[i].temp>=673.0f)
			parts[i].tmp = 1;
		else
			for (rx=-1; rx<2; rx++)
				for (ry=-1; ry<2; ry++)
					if (BOUNDS_CHECK && (rx || ry))
					{
						r = pmap[y+ry][x+rx];
						if (!r)
							continue;
						if (TYP(r)==PT_FIRE || TYP(r)==PT_PLSM || TYP(r)==PT_SPRK || TYP(r)==PT_LIGH)
						{
							parts[i].tmp = 1;
						}
					}

	}
	else if(parts[i].tmp==1)
	{
		if ((ID(pmap[y][x]) == i))
		{
			PropertyValue value;
			value.Integer = 2;
			sim->flood_prop(x, y, offsetof(Particle, tmp), value, StructProperty::Integer);
		}
		parts[i].tmp = 2;
	}
	else if(parts[i].tmp==2)
	{
		parts[i].tmp = 3;
	}
	else
	{
		float otemp = parts[i].temp-273.15f;
		//Explode!!
		sim->pv[y/CELL][x/CELL] += 0.5f;
		parts[i].tmp = 0;
		if(!(rand()%3))
		{
			if(!(rand()%2))
			{
				sim->create_part(i, x, y, PT_FIRE);
			}
			else
			{
				sim->create_part(i, x, y, PT_SMKE);
				parts[i].life = rand()%50+500;
			}
			parts[i].temp = restrict_flt((MAX_TEMP/4)+otemp, MIN_TEMP, MAX_TEMP);
		}
		else
		{
			if(!(rand()%15))
			{
				sim->create_part(i, x, y, PT_EMBR);
				parts[i].tmp = 0;
				parts[i].life = 50;
				parts[i].temp = restrict_flt((MAX_TEMP/3)+otemp, MIN_TEMP, MAX_TEMP);
				parts[i].vx = rand()%20-10;
				parts[i].vy = rand()%20-10;
			}
			else
			{
				sim->kill_part(i);
			}
		}
		return 1;
	}
	return 0;
}


Element_BANG::~Element_BANG() {}
