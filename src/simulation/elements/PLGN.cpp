#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_PLGN PT_PLGN 173
Element_PLGN::Element_PLGN()
{
	Identifier = "DEFAULT_PT_PLGN";
	Name = "PLGN";
	Colour = PIXPACK(0xFFFFFF);
	MenuVisible = 1;
	MenuSection = SC_GAS;
	Enabled = 1;
	
	Advection = 2.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.99f;
	Loss = 0.30f;
	Collision = -0.1f;
	Gravity = -0.04f;
	Diffusion = 2.0f;
	HotAir = 0.001f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 1;

	Temperature = R_TEMP+273.15f;
	HeatConduct = 100;
	Description = "Phlogiston. Extinguishes fire. Highly toxic.";
	
	State = ST_GAS;
	Properties = TYPE_GAS|PROP_DEADLY|PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_PLGN::update;
}


int Element_PLGN::update(UPDATE_FUNC_ARGS)
{
	int r,rx,ry;
	for(rx=-1; rx<2; rx++)
		for(ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
				continue;

				if ((r&0xFF)==PT_YEST)
				{
					sim->part_change_type(r>>8, x+rx, y+ry, PT_DYST);
				}
				
				if ((r&0xFF)==PT_LIFE)
				{
					if (parts[i].tmp2 == 1)
					{
						if(parts[r>>8].life > 0) parts[i].life = parts[r>>8].life;
						if(parts[i].life == 0)
						{
							sim->kill_part(i);
							return(1);
						}
					}
					sim->create_part(r>>8, x+rx, y+ry, PT_PLGN);
					sim->kill_part(PT_LIFE);
				}

				if ((r&0xFF)==PT_CO2 && parts[i].ctype==PT_SMKE)
				{
					sim->part_change_type(r>>8, x, y, PT_SMKE);
					sim->kill_part(i);
				}

				if (parts[i].tmp2 == 1 && (r&0xFF)!=PT_PLGN)
				{
					if(parts[r>>8].life > 0) parts[i].life = parts[r>>8].life;
					if(parts[i].life == 0)
					{
						sim->kill_part(i);
						return(1);
					}
				}

				if((r&0xFF)==PT_O2 && (parts[i].ctype==PT_FIRE || parts[i].tmp==1))
				{
					sim->part_change_type(i, x, y, PT_FIRE);
				}
			}

	for(rx=-4; rx<5; rx++)
		for(ry=-4; ry<5; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
				continue;

				if ((r&0xFF)==PT_PLNT)
				{
						sim->part_change_type(r>>8, x+rx, y+ry, PT_WOOD);
				}

				if((r&0xFF)==PT_BMTL && parts[r>>8].tmp == 1)
				{
						parts[r>>8].tmp = 0;
						sim->part_change_type(r>>8, x+rx, y+ry, PT_IRON);
				}

				if((r&0xFF)==PT_PLGN)
				{
					if(parts[r>>8].tmp >= 1) parts[i].tmp = parts[r>>8].tmp;
					if(parts[r>>8].tmp2 >= 1) parts[i].tmp2 = parts[r>>8].tmp2;
				}
				if((r&0xFF)==PT_PLSM)
				{
				if(((r&0xFF)==PT_FUSE || (r&0xFF)==PT_FSEP) && parts[r>>8].life<50)
				{
					sim->create_part(r>>8, x+rx, y+ry, PT_PLGN);
					sim->kill_part(PT_PLSM);
					parts[r>>8].life = 50;
				}
				}
				if(((r&0xFF)==PT_FUSE || (r&0xFF)==PT_FSEP) && parts[r>>8].life<50)
				{
					parts[r>>8].life = 50;
				}

			}

	for(rx=-2; rx<3; rx++)
		for(ry=-2; ry<3; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
				continue;

			if ((r&0xFF)==PT_FIRE && parts[i].tmp<=0)
				{
					if (parts[i].tmp2 == 1)
					{
						if(parts[i].life > 0) parts[i].life = parts[r>>8].life;
						if(parts[i].life == 0) 
						{
							sim->kill_part(i);
							return(1);
						}
					}
					sim->create_part(r>>8, x+rx, y+ry, PT_PLGN);
					sim->kill_part(PT_FIRE);
					parts[i].ctype == PT_SMKE;
					if((r&0xFF)==PT_IGNT && (parts[r>>8].life<3 || parts[r>>8].tmp==1))
					{
						parts[r>>8].tmp = 0;
						parts[r>>8].life = 3;
					}
					if(((r&0xFF)==PT_COAL || (r&0xFF)==PT_BCOL) && parts[r>>8].life<100)
					{
						parts[r>>8].life = 110;
					}
				}
			if((r&0xFF)==PT_EMBR)
			{
				sim->create_part(r>>8, x+rx, y+ry, PT_PLGN);
				sim->kill_part(PT_EMBR);
				if((r&0xFF)==PT_IGNT && (parts[r>>8].life<3 || parts[r>>8].tmp==1))
					{
						parts[r>>8].tmp = 0;
						parts[r>>8].life = 3;
					}
			}
			if((r&0xFF)==PT_IGNT && (parts[r>>8].life<3 || parts[r>>8].tmp==1))
					{
						parts[r>>8].tmp = 0;
						parts[r>>8].life = 3;
					}
		}
	return(0);		
}

Element_PLGN::~Element_PLGN() {}