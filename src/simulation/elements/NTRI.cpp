#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_NTRI PT_NTRI 179
Element_NTRI::Element_NTRI()
{
	Identifier = "DEFAULT_PT_NTRI";
	Name = "NTRI";
	Colour = PIXPACK(0x304030);
	MenuVisible = 1;
	MenuSection = SC_NUCLEAR;
	Enabled = 1;
	
	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 1.00f;
	Loss = 1.00f;
	Collision = -.99f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;
	
	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;
	
	Weight = -1;
	
	Temperature = R_TEMP+273.15f;
	HeatConduct = 0;
	Description = "Neutrinos. Reverts NEUT reactions, passes through some walls, receives heat.";
	
	State = ST_GAS;
	Properties = TYPE_ENERGY;
	
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;
	
	Update = &Element_NTRI::update;
	Graphics = &Element_NTRI::graphics;
}

//#TPT-Directive ElementHeader Element_NTRI static int update(UPDATE_FUNC_ARGS)
int Element_NTRI::update(UPDATE_FUNC_ARGS)
{
	int under = pmap[y][x];
	if ((under&0xFF) == PT_DEUT)
	{
		if ((-((int)sim->pv[y/CELL][x/CELL]-4)+(parts[under>>8].life/100)) > rand()%200)
		{
			DeutImplosion(sim, parts[under>>8].life, x, y, restrict_flt(parts[under>>8].temp + parts[under>>8].life*500, MIN_TEMP, MAX_TEMP), PT_NTRI);
			sim->kill_part(under>>8);
		}
	}
    if (under)
	{
		if ((under&0xFF) == PT_HSWC)
		{
            parts[i].temp = parts[under>>8].temp;
		}
		else if ((under&0xFF) == PT_FILT)
        {
            parts[under>>8].temp = parts[i].temp;
        }
		else
		{
			if (sim->elements[(under&0xFF)].HeatConduct) parts[i].temp = restrict_flt(parts[i].temp+(parts[under>>8].temp-parts[i].temp)/16.0f, MIN_TEMP, MAX_TEMP);
		}
	}
	else if (parts[i].life)
	{
		if (!--parts[i].life)
			sim->kill_part(i);
	}
    switch (under&0xFF)
    {
    case PT_DSTW:
        if (3>(rand()%20))
            sim->part_change_type(under>>8,x,y,PT_WATR);
    case PT_URAN:
        sim->part_change_type(under>>8,x,y,PT_PLUT);
        parts[under>>8].tmp = 1;
        sim->pv[y/CELL][x/CELL] -= 1.0f * CFDS;
        break;
    case PT_DUST:
        if (3>(rand()%200))
            sim->part_change_type(under>>8,x,y,PT_GUNP);
        break;
    case PT_YEST:
        if (3>(rand()%200))
            sim->part_change_type(under>>8,x,y,PT_DYST);
        break;
    case PT_DYST:
        sim->part_change_type(under>>8,x,y,PT_YEST);
        break;
    case PT_GOO:
        if (3>(rand()%200))
            sim->part_change_type(under>>8,x,y,PT_PLEX);
        break;
    case PT_DESL:
        if (3>(rand()%200))
            sim->part_change_type(under>>8,x,y,PT_NITR);
        break;
    case PT_GAS:
        if (3>(rand()%200))
            sim->part_change_type(under>>8,x,y,rand()%2 ? PT_DESL : PT_OIL);
        break;
    case PT_WOOD:
        if (3>(rand()%200))
            sim->part_change_type(under>>8,x,y,rand()%2 ? PT_COAL : PT_PLNT);
        if (parts[under>>8].type == PT_COAL) {
            parts[under>>8].tmp = 50;
            parts[under>>8].life = 110;
        }
        break;
    case PT_FWRK:
        if (!(rand()%20))
            sim->part_change_type(under>>8, x, y, PT_DUST);
        break;
    case PT_ISOZ:
        if (!(rand()%20))
            sim->create_part(under>>8, x, y, PT_ACID);
        break;
    default:
        break;
    }
	return 0;
}

//#TPT-Directive ElementHeader Element_NTRI static int DeutImplosion(Simulation * sim, int n, int x, int y, float temp, int t)
int Element_NTRI::DeutImplosion(Simulation * sim, int n, int x, int y, float temp, int t)
{
	int i;
	n = (n/50);
	if (n<1)
		n = 1;
	else if (n>340)
		n = 340;

	for (int c=0; c<n; c++)
	{
		i = sim->create_part(-3, x, y, t);
		if (i >= 0)
			sim->parts[i].temp = temp;

		if (sim->grav->ngrav_enable)
            sim->gravmap[(y/CELL)*(XRES/CELL)+(x/CELL)] = restrict_flt(sim->gravmap[(y/CELL)*(XRES/CELL)+(x/CELL)] + (4.0f * CFDS),-120,120);
        else
            sim->pv[y/CELL][x/CELL] += 5.0f * CFDS;
	}
	return 0;
}

//#TPT-Directive ElementHeader Element_NTRI static int graphics(GRAPHICS_FUNC_ARGS)
int Element_NTRI::graphics(GRAPHICS_FUNC_ARGS)
{
	*firea = 20;
	*firer = 150;
	*fireg = 150;
	*fireb = 15;

	*pixel_mode |= FIRE_BLEND;
	return 1;
}

Element_NTRI::~Element_NTRI() {}
