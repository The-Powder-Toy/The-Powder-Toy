#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_PROT PT_PROT 173
Element_PROT::Element_PROT()
{
	Identifier = "DEFAULT_PT_PROT";
	Name = "PROT";
	Colour = PIXPACK(0x990000);
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
	HeatConduct = 61;
	Description = "Protons. Transfer heat to materials, and removes sparks.";
	
	State = ST_GAS;
	Properties = TYPE_ENERGY|PROP_LIFE_KILL;
	
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;
	
	Update = &Element_PROT::update;
	Graphics = &Element_PROT::graphics;
}

//#TPT-Directive ElementHeader Element_PROT static int update(UPDATE_FUNC_ARGS)
int Element_PROT::update(UPDATE_FUNC_ARGS)
{
	sim->pv[y/CELL][x/CELL] -= .005f;
	int under = pmap[y][x];
	//set off explosives (only when hot because it wasn't as fun when it made an entire save explode)
	if (parts[i].temp > 273.15f+500.0f && (sim->elements[under&0xFF].Flammable || sim->elements[under&0xFF].Explosive || (under&0xFF) == PT_BANG))
	{
		sim->create_part(under>>8, x, y, PT_FIRE);
		parts[under>>8].temp += restrict_flt(sim->elements[under&0xFF].Flammable*5, MIN_TEMP, MAX_TEMP);
		sim->pv[y/CELL][x/CELL] += 1.00f;
	}
	//remove active sparks
	else if ((under&0xFF) == PT_SPRK)
	{
		sim->part_change_type(under>>8, x, y, parts[under>>8].ctype);
		parts[under>>8].life = 44+parts[under>>8].life;
		parts[under>>8].ctype = 0;
	}
	//prevent inactive sparkable elements from being sparked
	else if ((sim->elements[under&0xFF].Properties&PROP_CONDUCTS) && parts[under>>8].life <= 4)
	{
		parts[under>>8].life = 40+parts[under>>8].life;
	}
	else if ((under&0xFF) == PT_EXOT)
		parts[under>>8].ctype = PT_PROT;

	//make temp of other things closer to it's own temperature. This will change temp of things that don't conduct, and won't change the PROT's temperature
	if (under)
	{
		parts[under>>8].temp -= restrict_flt((parts[under>>8].temp-parts[i].temp)/4.0f, MIN_TEMP, MAX_TEMP);
	}
	//else, slowly kill it if it's not inside an element
	else
		parts[i].life--;
	return 0;
}

//#TPT-Directive ElementHeader Element_PROT static int graphics(GRAPHICS_FUNC_ARGS)
int Element_PROT::graphics(GRAPHICS_FUNC_ARGS)
{
	*firea = 20;
	*firer = 250;
	*fireg = 128;
	*fireb = 128;

	*pixel_mode |= FIRE_ADD;
	return 1;
}

Element_PROT::~Element_PROT() {}
