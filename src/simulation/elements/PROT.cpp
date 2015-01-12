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
	Properties = TYPE_ENERGY;
	
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
	sim->pv[y/CELL][x/CELL] -= .003f;
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
	else if ((under&0xFF) == PT_DEUT)
	{
		if ((-((int)sim->pv[y/CELL][x/CELL]-4)+(parts[under>>8].life/100)) > rand()%200)
		{
			DeutImplosion(sim, parts[under>>8].life, x, y, restrict_flt(parts[under>>8].temp + parts[under>>8].life*500, MIN_TEMP, MAX_TEMP), PT_PROT);
			sim->kill_part(under>>8);
		}
	}
	//prevent inactive sparkable elements from being sparked
	else if ((sim->elements[under&0xFF].Properties&PROP_CONDUCTS) && parts[under>>8].life <= 4)
	{
		parts[under>>8].life = 40+parts[under>>8].life;
	}
	//Powered LCRY reaction: PROT->PHOT
	else if ((under&0xFF) == PT_LCRY && parts[under>>8].life > 5 && !(rand()%10))
	{
		sim->part_change_type(i, x, y, PT_PHOT);
		parts[i].life *= 2;
		parts[i].ctype = 0x3FFFFFFF;
	} 
	else if ((under&0xFF) == PT_EXOT)
		parts[under>>8].ctype = PT_PROT;

	//make temp of other things closer to it's own temperature. This will change temp of things that don't conduct, and won't change the PROT's temperature
	if (under)
	{
		//now changed so that PROT goes through portal, so only the WIFI part applies
		if ((under&0xFF) == PT_WIFI/* || (under&0xFF) == PT_PRTI || (under&0xFF) == PT_PRTO*/)
		{
			float change;
			if (parts[i].temp<173.15f) change = -1000.0f;
			else if (parts[i].temp<273.15f) change = -100.0f;
			else if (parts[i].temp>473.15f) change = 1000.0f;
			else if (parts[i].temp>373.15f) change = 100.0f;
			else change = 0.0f;
			parts[under>>8].temp = restrict_flt(parts[under>>8].temp+change, MIN_TEMP, MAX_TEMP);
		}
		else
		{
			parts[under>>8].temp = restrict_flt(parts[under>>8].temp-(parts[under>>8].temp-parts[i].temp)/4.0f, MIN_TEMP, MAX_TEMP);
		}
	}
	//else, slowly kill it if it's not inside an element
	else if (parts[i].life)
	{
		if (!--parts[i].life)
			sim->kill_part(i);
	}
	
	//if this proton has collided with another last frame, change it into a heavier element
	if (parts[i].tmp)
	{
		int newID, element;
		if (parts[i].tmp > 500000)
			element = PT_SING; //particle accelerators are known to create earth-destroying black holes
		else if (parts[i].tmp > 500)
			element = PT_PLUT;
		else if (parts[i].tmp > 320)
			element = PT_URAN;
		else if (parts[i].tmp > 150)
			element = PT_PLSM;
		else if (parts[i].tmp > 50)
			element = PT_O2;
		else if (parts[i].tmp > 20)
			element = PT_CO2;
		else
			element = PT_NBLE;
		newID = sim->create_part(-1, x+rand()%3-1, y+rand()%3-1, element);
		parts[newID].temp = restrict_flt(100.0f*parts[i].tmp, MIN_TEMP, MAX_TEMP);
		sim->kill_part(i);
		return 1;
	}
	//collide with other protons to make heavier materials
	int ahead = sim->photons[y][x];
	if ((ahead>>8) != i && (ahead&0xFF) == PT_PROT)
	{
		float velocity1 = powf(parts[i].vx, 2.0f)+powf(parts[i].vy, 2.0f);
		float velocity2 = powf(parts[ahead>>8].vx, 2.0f)+powf(parts[ahead>>8].vy, 2.0f);
		float direction1 = atan2f(-parts[i].vy, parts[i].vx);
		float direction2 = atan2f(-parts[ahead>>8].vy, parts[ahead>>8].vx);
		float difference = direction1 - direction2; if (difference < 0) difference += 6.28319f;

		if (difference > 3.12659f && difference < 3.15659f && velocity1 + velocity2 > 10.0f)
		{
			parts[ahead>>8].tmp += (int)(velocity1 + velocity2);
			sim->kill_part(i);
			return 1;
		}
	}
	return 0;
}

//#TPT-Directive ElementHeader Element_PROT static int DeutImplosion(Simulation * sim, int n, int x, int y, float temp, int t)
int Element_PROT::DeutImplosion(Simulation * sim, int n, int x, int y, float temp, int t)
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

		sim->pv[y/CELL][x/CELL] -= 6.0f * CFDS;
	}
	return 0;
}

//#TPT-Directive ElementHeader Element_PROT static int graphics(GRAPHICS_FUNC_ARGS)
int Element_PROT::graphics(GRAPHICS_FUNC_ARGS)
{
	*firea = 7;
	*firer = 250;
	*fireg = 170;
	*fireb = 170;

	*pixel_mode |= FIRE_BLEND;
	return 1;
}

Element_PROT::~Element_PROT() {}
