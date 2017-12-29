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
	int utype = TYP(under);
	int uID = ID(under);
	switch (utype)
	{
	case PT_SPRK:
	{
		//remove active sparks
		int sparked = parts[uID].ctype;
		if (sparked > 0 && sparked < PT_NUM && sim->elements[sparked].Enabled)
		{
			sim->part_change_type(uID, x, y, sparked);
			parts[uID].life = 44 + parts[uID].life;
			parts[uID].ctype = 0;
		}
		break;
	}
	case PT_DEUT:
		if ((-((int)sim->pv[y / CELL][x / CELL] - 4) + (parts[uID].life / 100)) > rand() % 200)
		{
			DeutImplosion(sim, parts[uID].life, x, y, restrict_flt(parts[uID].temp + parts[uID].life * 500, MIN_TEMP, MAX_TEMP), PT_PROT);
			sim->kill_part(uID);
		}
		break;
	case PT_LCRY:
		//Powered LCRY reaction: PROT->PHOT
		if (parts[uID].life > 5 && !(rand() % 10))
		{
			sim->part_change_type(i, x, y, PT_PHOT);
			parts[i].life *= 2;
			parts[i].ctype = 0x3FFFFFFF;
		}
		break;
	case PT_EXOT:
		parts[uID].ctype = PT_PROT;
		break;
	case PT_WIFI:
		float change;
		if (parts[i].temp < 173.15f) change = -1000.0f;
		else if (parts[i].temp < 273.15f) change = -100.0f;
		else if (parts[i].temp > 473.15f) change = 1000.0f;
		else if (parts[i].temp > 373.15f) change = 100.0f;
		else change = 0.0f;
		parts[uID].temp = restrict_flt(parts[uID].temp + change, MIN_TEMP, MAX_TEMP);
		break;
	case PT_NONE:
		//slowly kill if it's not inside an element
		if (parts[i].life)
		{
			if (!--parts[i].life)
				sim->kill_part(i);
		}
		break;
	default:
		//set off explosives (only when hot because it wasn't as fun when it made an entire save explode)
		if (parts[i].temp > 273.15f + 500.0f && (sim->elements[utype].Flammable || sim->elements[utype].Explosive || utype == PT_BANG))
		{
			sim->create_part(uID, x, y, PT_FIRE);
			parts[uID].temp += restrict_flt(sim->elements[utype].Flammable * 5, MIN_TEMP, MAX_TEMP);
			sim->pv[y / CELL][x / CELL] += 1.00f;
		}
		//prevent inactive sparkable elements from being sparked
		else if ((sim->elements[utype].Properties&PROP_CONDUCTS) && parts[uID].life <= 4)
		{
			parts[uID].life = 40 + parts[uID].life;
		}
		break;
	}
	//make temp of other things closer to it's own temperature. This will change temp of things that don't conduct, and won't change the PROT's temperature
	if (utype && utype != PT_WIFI)
		parts[uID].temp = restrict_flt(parts[uID].temp-(parts[uID].temp-parts[i].temp)/4.0f, MIN_TEMP, MAX_TEMP);
 

	//if this proton has collided with another last frame, change it into a heavier element
	if (parts[i].tmp)
	{
		int newID, element;
		if (parts[i].tmp > 500000)
			element = PT_SING; //particle accelerators are known to create earth-destroying black holes
		else if (parts[i].tmp > 700)
			element = PT_PLUT;
		else if (parts[i].tmp > 420)
			element = PT_URAN;
		else if (parts[i].tmp > 310)
			element = PT_POLO;
		else if (parts[i].tmp > 250)
			element = PT_PLSM;
		else if (parts[i].tmp > 100)
			element = PT_O2;
		else if (parts[i].tmp > 50)
			element = PT_CO2;
		else
			element = PT_NBLE;
		newID = sim->create_part(-1, x+rand()%3-1, y+rand()%3-1, element);
		if (newID >= 0)
			parts[newID].temp = restrict_flt(100.0f*parts[i].tmp, MIN_TEMP, MAX_TEMP);
		sim->kill_part(i);
		return 1;
	}
	//collide with other protons to make heavier materials
	int ahead = sim->photons[y][x];
	if (ID(ahead) != i && TYP(ahead) == PT_PROT)
	{
		float velocity1 = powf(parts[i].vx, 2.0f)+powf(parts[i].vy, 2.0f);
		float velocity2 = powf(parts[ID(ahead)].vx, 2.0f)+powf(parts[ID(ahead)].vy, 2.0f);
		float direction1 = atan2f(-parts[i].vy, parts[i].vx);
		float direction2 = atan2f(-parts[ID(ahead)].vy, parts[ID(ahead)].vx);
		float difference = direction1 - direction2; if (difference < 0) difference += 6.28319f;

		if (difference > 3.12659f && difference < 3.15659f && velocity1 + velocity2 > 10.0f)
		{
			parts[ID(ahead)].tmp += (int)(velocity1 + velocity2);
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
	if (n < 1)
		n = 1;
	else if (n > 340)
		n = 340;

	for (int c = 0; c < n; c++)
	{
		i = sim->create_part(-3, x, y, t);
		if (i >= 0)
			sim->parts[i].temp = temp;
		else if (sim->pfree < 0)
			break;
	}
	sim->pv[y/CELL][x/CELL] -= (6.0f * CFDS)*n;
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
