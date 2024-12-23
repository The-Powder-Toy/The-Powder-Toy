#include "simulation/ElementCommon.h"
#include "MOTH.h"
static void create(ELEMENT_CREATE_FUNC_ARGS);

void Element::Element_MOTH()
{
	Identifier = "DEFAULT_PT_MOTH";
	Name = "MOTH";
	Colour = 0x755224_rgb;
	MenuVisible = 1;
	MenuSection = SC_SPECIAL;
	Enabled = 1;

	Advection = 1.2f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.99f;
	Loss = 0.30f;
	Collision = -0.1f;
	Gravity = 0.0f;
	Diffusion = 1.3f;
	HotAir = 0.0001f * CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 5;
	Hardness = 1;

	Weight = 90;

	HeatConduct = 150;
	Description = "Moths. Small flying nocturnal creatures.";

	Properties = TYPE_GAS;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 313.15f;
	HighTemperatureTransition = PT_DUST;
	Graphics = &Element_MOTH_graphics;
	Update = &Element_MOTH_update;
	Create = &create;
}
int moth_search_radius = 10;
int SDmultiplier = 3; // only check for target element every this many. expand search radius by times this without searching more cells at the cost of accuracy
int Element_MOTH_graphics(GRAPHICS_FUNC_ARGS)
{
	int z = (cpart->tmp2 - 5) * 16;//speckles!
	*colr += z;
	*colg += z;
	*colb += z;
	return 0;
}
static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	sim->parts[i].tmp2 = sim->rng.between(0, 10);
	sim->parts[i].tmp3 = 0;
	sim->parts[i].life = 400+(sim->rng.between(-30, 0)); // five hundred
}
int Element_MOTH_update(UPDATE_FUNC_ARGS)
{
	sim->parts[i].life -= 1;
	if(parts[i].life <= 0){
		sim->parts[i].type = PT_DUST;

		return 0;
	}
	bool found = false;
	for (int dx = -moth_search_radius; dx <= moth_search_radius && !found; dx++)
    {
        for (int dy = -moth_search_radius; dy <= moth_search_radius; dy++)
        {
            int nx = x + dx*SDmultiplier;
            int ny = y + dy*SDmultiplier;

            if (nx < 0 || ny < 0 || nx >= XRES || ny >= YRES) continue;

            int r = sim->pmap[ny][nx];


            if (r && (TYP(r) == PT_FIRE || TYP(r) == PT_PHOT || TYP(r) == PT_PLNT || TYP(r) == PT_GEL))
            {
				
				sim->parts[i].vx = ((dx < 0) ? parts[i].vx-0.5f : parts[i].vx+0.5f);

				sim->parts[i].vy = ((dy < 0) ? parts[i].vy-0.5f : parts[i].vy+0.5f);

                found = true;
				break;
            }
        }
    }
	for (auto rx = -1; rx <= 1; rx++)
	{
		for (auto ry = -1; ry <= 1; ry++)
		{
			int r = sim->pmap[y+ry][x+rx];
			if (r && (TYP(r) == PT_PLNT || TYP(r) == PT_GEL)){
				sim->kill_part(ID(r));
				sim->parts[i].life = (TYP(r) == PT_GEL)? parts[i].life+30 : parts[i].life+10;
				if (parts[i].life > 500){
					sim->parts[i].life = 500;
				}
			}
		}
	}
	if (parts[i].life > 400 && parts[i].tmp3 == 0){
		sim->create_part(-1, x, y+1, PT_MEGG);
		sim->parts[i].life = 100;
		sim->parts[i].tmp3 = 100;
	}
	sim->parts[i].tmp3 = parts[i].tmp3-1;
	if (parts[i].tmp3 < 0){
		sim->parts[i].tmp3 = 0;
	}

	return 0;
}
