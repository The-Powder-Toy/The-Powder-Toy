#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_ASPK PT_ASPK 179
Element_ASPK::Element_ASPK()
{
	Identifier = "DEFAULT_PT_ASPK";
	Name = "ASPK";
	Colour = PIXPACK(0x5cb8ff);
	MenuVisible = 1;
	MenuSection = SC_ELEC;
	Enabled = 1;
	
	Advection = 0.4f;
	AirDrag = 0.04f * CFDS;
	AirLoss = 0.94f;
	Loss = 0.95f;
	Collision = -0.1f;
	Gravity = 0.18f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;
	
	Flammable = 0;
	Explosive = 0;
	Meltable = 2;
	Hardness = 2;
	
	Weight = 90;
	
	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 211;
	Description = "Anti-spark. Inhibits conductors on contact.";
	
	State = ST_SOLID;
	Properties = TYPE_PART|PROP_LIFE_DEC;
	
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;
	
	Update = &Element_ASPK::update;
	
}

//#TPT-Directive ElementHeader Element_ASPK static int update(UPDATE_FUNC_ARGS)
int Element_ASPK::update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry, rt, a, b, a1, b1, subx, suby;
	if(parts[i].tmp<=0) parts[i].tmp++;
	if(parts[i].tmp2<=0) parts[i].tmp2++;
	subx = parts[i].tmp * -1;
	a = subx;
	b = parts[i].tmp + 1;
	if(parts[i].ctype==PT_NONE)
	{
		a1 = a;
		b1 = b;
	}
	else
	{
		suby = parts[i].tmp2 * -1;
		a1 = suby;
		b1 = parts[i].tmp2 + 1;
	}
		for (rx=a; rx<b; rx++)
			for (ry=a1; ry<b1; ry++)
				if (BOUNDS_CHECK)
				{
					r = sim->photons[y+ry][x+rx];
					if (!r)
						r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if((r&0xFF)==PT_SPRK)
					{
						if(parts[r>>8].ctype==parts[r>>8].type)
						{
							sim->part_change_type(r>>8, x+rx, y+ry, parts[r>>8].ctype);
						}
						sim->part_change_type(r>>8, x+rx, y+ry, parts[r>>8].ctype);
					}
					if((r&0xFF)==PT_GOLD && parts[i].ctype==PT_GOLD)
					{
						sim->part_change_type(r>>8, x+rx, y+ry, PT_GOLD);
					}
					if((r&0xFF)==PT_INST && parts[r>>8].life < 4)
					{
						parts[r>>8].life=4;
					}
					if((r&0xFF)==PT_FELE) sim->kill_part(r>>8);
					if((r&0xFF)==PT_WIRE && parts[r>>8].ctype!=PT_NONE)
					{
						parts[r>>8].ctype = PT_NONE;
					}
				}
	return(0);
}

Element_ASPK::~Element_ASPK() {}
