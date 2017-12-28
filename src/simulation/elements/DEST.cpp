#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_DEST PT_DEST 89
Element_DEST::Element_DEST()
{
	Identifier = "DEFAULT_PT_DEST";
	Name = "DEST";
	Colour = PIXPACK(0xFF3311);
	MenuVisible = 1;
	MenuSection = SC_EXPLOSIVE;
	Enabled = 1;

	Advection = -0.05f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.95f;
	Loss = 0.95f;
	Collision = -0.1f;
	Gravity = 0.4f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 101;

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 150;
	Description = "More destructive Bomb, can break through virtually anything.";

	Properties = TYPE_PART|PROP_LIFE_DEC|PROP_LIFE_KILL_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_DEST::update;
	Graphics = &Element_DEST::graphics;
}

//#TPT-Directive ElementHeader Element_DEST static int update(UPDATE_FUNC_ARGS)
int Element_DEST::update(UPDATE_FUNC_ARGS)
{
	int rx = rand()%5-2;
	int ry = rand()%5-2;
	int r = pmap[y+ry][x+rx];
	if (!r)
		return 0;
	int rt = TYP(r);
	if (rt == PT_DEST || rt == PT_DMND || rt == PT_BCLN  || rt == PT_CLNE  || rt == PT_PCLN  || rt == PT_PBCN)
		return 0;

	if (parts[i].life<=0 || parts[i].life>37)
	{
		parts[i].life=30+rand()%20;
		sim->pv[y/CELL][x/CELL]+=60.0f;
	}
	if (rt == PT_PLUT || rt == PT_DEUT)
	{
		sim->pv[y/CELL][x/CELL]+=20.0f;
		if (rand()%2)
		{
			sim->create_part(ID(r), x+rx, y+ry, PT_NEUT);
			parts[ID(r)].temp = MAX_TEMP;
			sim->pv[y/CELL][x/CELL] += 10.0f;
			parts[i].life-=4;
		}
	}
	else if (rt == PT_INSL)
	{
		sim->create_part(ID(r), x+rx, y+ry, PT_PLSM);
	}
	else if (!(rand()%3))
	{
		sim->kill_part(ID(r));
		parts[i].life -= 4*((sim->elements[rt].Properties&TYPE_SOLID)?3:1);
		if (parts[i].life<=0)
			parts[i].life=1;
	}
	else if (sim->elements[rt].HeatConduct) 
		parts[ID(r)].temp = MAX_TEMP;
	parts[i].temp=MAX_TEMP;
	sim->pv[y/CELL][x/CELL]+=80.0f;
	return 0;
}


//#TPT-Directive ElementHeader Element_DEST static int graphics(GRAPHICS_FUNC_ARGS)
int Element_DEST::graphics(GRAPHICS_FUNC_ARGS)

{
	if(cpart->life)
	{
		*pixel_mode |= PMODE_LFLARE;
	}
	else
	{
		*pixel_mode |= PMODE_SPARK;
	}
	return 0;
}


Element_DEST::~Element_DEST() {}
