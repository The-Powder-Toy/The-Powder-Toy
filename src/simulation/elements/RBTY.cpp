#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_RBTY PT_RBTY 173
Element_RBTY::Element_RBTY()
{
	Identifier = "DEFAULT_PT_RBTY";
	Name = "RBTY";
	Colour = PIXPACK(0x959615);
	MenuVisible = 1;
	MenuSection = SC_ELEC;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.00f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f * CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 20;

	Weight = 100;

	Temperature = R_TEMP+0.0f  +273.15f;
	HeatConduct = 100;
	Description = "Rechargeable battery.";

	State = ST_SOLID;
	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 2273.0f;
	HighTemperatureTransition = PT_PLSM;

	Update = &Element_RBTY::update;

}
//#TPT-Directive ElementHeader Element_RBTY static int update(UPDATE_FUNC_ARGS)
int Element_RBTY::update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry;
	// Okay so just a looparound
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if( sim->pv[x/CELL][y/CELL] >= 200.0f)
					sim->part_change_type(i, x, y, PT_PLEX);  
				/*if(parts[i].temp<=173.15)
					parts[i].tmp=1;*/
				while((r&0xFF)==PT_SPRK && parts[r>>8].ctype==PT_PSCN)
				{
					parts[i].life++;
					parts[i].tmp2=1;
					parts[i].tmp=1;
					parts[i].ctype=PT_BTRY;
					break;
				}
				while((r&0xFF)==PT_PSCN && parts[r>>8].life<=0 && (r&0xFF)!=PT_SPRK)
				{
					parts[i].tmp=0;
					parts[i].tmp2=0;
					break;
				}
				/*if(parts[i].ctype==PT_LIFE && parts[i].tmp2==0 && (r&0xFF)==PT_RBTY)
				{
					parts[r>>8].tmp2=0;
					if(parts[r>>8].life!=0) parts[i].life=parts[r>>8].life;
				}*/
				if((r&0xFF)!=PT_PSCN && parts[i].life>0 && (sim->elements[r&0xFF].Properties&PROP_CONDUCTS) && parts[r>>8].life<=0 && parts[i].tmp2!=1)
				{
					parts[r>>8].life = 4;
					parts[r>>8].ctype = r&0xFF;
					sim->part_change_type(r>>8,x+rx,y+ry,PT_SPRK);
					parts[i].life--;
					if(parts[i].ctype!=PT_BTRY) parts[i].ctype=PT_SPRK;
				}
				if((r&0xFF)!=PT_PSCN && (sim->elements[r&0xFF].Properties&PROP_CONDUCTS) && parts[r>>8].life<=0 && parts[i].ctype!=PT_BTRY)
				{
					parts[i].ctype=PT_SPRK;
				}
				if((r&0xFF)==PT_RBTY && parts[i].life>=parts[r>>8].life && parts[i].ctype==PT_METL)
				{
					if(parts[i].life>parts[r>>8].life && parts[r>>8].ctype==PT_METL && parts[i].tmp==1) 
					{
							parts[r>>8].tmp=1;
							if(parts[i].life>parts[r>>8].life) parts[r>>8].life=parts[i].life;
							parts[i].tmp=0;
					}
					if(parts[i].life>parts[r>>8].life && parts[r>>8].ctype==PT_SPRK && parts[i].tmp==1)
					{
							if(parts[i].life>parts[r>>8].life) parts[r>>8].life=parts[i].life;
							parts[i].tmp=0;
					}
					if(parts[i].life==parts[r>>8].life && parts[r>>8].life>0 && parts[i].tmp==1 && parts[i].life>0 && parts[r>>8].ctype==PT_SPRK)
					{
						parts[i].tmp=0;
					}
					if(parts[i].life>parts[r>>8].life && parts[i].tmp==0 && parts[r>>8].tmp==0)
					{
						parts[i].life=parts[r>>8].life;
					}
				}
				if((r&0xFF)==PT_RBTY && parts[i].ctype==PT_SPRK && parts[r>>8].ctype==PT_METL)
				{
					if(parts[r>>8].tmp==1) parts[i].tmp2=1;
					if(parts[r>>8].tmp==0) parts[i].tmp2=0;
				}
				if((r&0xFF)==PT_RBTY && parts[i].life>=parts[r>>8].life && parts[r>>8].ctype==PT_NONE && parts[i].ctype==PT_METL)
				{
					parts[r>>8].tmp2=parts[i].tmp2;
					if(parts[i].life>parts[r>>8].life) parts[r>>8].life=parts[i].life;
					parts[r>>8].ctype=PT_METL;
				}
				if((r&0xFF)==PT_RBTY && parts[r>>8].ctype==PT_SPRK && parts[i].ctype!=PT_BTRY)
				{
					if(parts[r>>8].life<parts[i].life && parts[i].tmp!=1) parts[i].life=parts[r>>8].life;
					parts[i].ctype=PT_METL;
				}
				if((r&0xFF)==PT_RBTY && parts[i].ctype==PT_BTRY)
				{
					if(parts[i].life>parts[r>>8].life && parts[i].tmp==1)
					{
							parts[r>>8].life=parts[i].life;
					}
					if(parts[r>>8].ctype==PT_METL) parts[r>>8].tmp=1;
					parts[r>>8].tmp2=parts[i].tmp2;
					if(parts[r>>8].ctype==PT_NONE) parts[r>>8].ctype=PT_METL;
					if(parts[i].tmp==0)
					{
						if(parts[i].life>parts[r>>8].life) parts[i].life=parts[r>>8].life;
						parts[r>>8].tmp=0;
					}
				}
			}
	return 0;
}
Element_RBTY::~Element_RBTY() {}
