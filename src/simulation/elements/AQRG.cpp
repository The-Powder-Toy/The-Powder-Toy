#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_AQRG PT_AQRG 190
Element_AQRG::Element_AQRG()
{
	Identifier = "DEFAULT_PT_AQRG";
	Name = "AQRG";
	Colour = PIXPACK(0xFF0000);
	MenuVisible = 1;
	MenuSection = SC_LIQUID;
	Enabled = 1;
	
	Advection = 0.6f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.98f;
	Loss = 0.95f;
	Collision = 0.0f;
	Gravity = 0.1f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 2;
	
	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 1;
	
	Weight = 10;
	
	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 34;
	Description = "Aqua Regia. Dissolves certain metals. Reversable with soap.";
	
	State = ST_LIQUID;
	Properties = TYPE_LIQUID|PROP_DEADLY;
	
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;
	
	Update = &Element_AQRG::update;
	Graphics = &Element_AQRG::graphics;
}

//#TPT-Directive ElementHeader Element_AQRG static int update(UPDATE_FUNC_ARGS)
int Element_AQRG::update(UPDATE_FUNC_ARGS)
 {
	int r, rx, ry, trade, np;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if((r&0xFF)==PT_GOLD&&parts[i].life>=50)
				{
					parts[i].ctype=r&0xFF;
					float newtemp = ((60.0f-(float)sim->elements[r&0xFF].Hardness))*7.0f;
							if(newtemp < 0)
							{
								newtemp = 0;
							}
							if(!(rand()%250)) 
							{
							parts[i].temp += newtemp;
							parts[i].life--;
							if(!(rand()%50))
							{
							sim->part_change_type(r>>8,x+rx,y+ry,PT_SMKE);
							}
							else
							{
								sim->kill_part(r>>8);
							}
							}
				}
				if((r&0xFF)==PT_SOAP)
				{
					sim->part_change_type(i,x,y,parts[i].ctype);
				}
				if (parts[i].life<=50)
							{
								sim->part_change_type(i,x,y,PT_MERC);
								return 1;
							}
			}
		for ( trade = 0; trade<2; trade ++)
		{
		rx = rand()%5-2;
		ry = rand()%5-2;
		if (BOUNDS_CHECK && (rx || ry))
		{
			r = pmap[y+ry][x+rx];
			if (!r)
				continue;
			if ((r&0xFF)==PT_AQRG && (parts[i].life>parts[r>>8].life) && parts[i].life>0)//diffusion
			{
				int temp = parts[i].life - parts[r>>8].life;
				if (temp==1)
				{
					parts[r>>8].life ++;
					parts[i].life --;
				}
				else if (temp>0)
				{
					parts[r>>8].life += temp/2;
					parts[i].life -= temp/2;
				}
			}
		}
  }
		return 0;
}

int Element_AQRG::graphics(GRAPHICS_FUNC_ARGS)
{
	int s = cpart->life;
	if (s>75) s = 75; //These two should not be here.
	if (s<49) s = 49;
	s = (s-49)*3;
	if (s==0) s = 1;
	*colr += s*4;
	*colg += s*1;
	*colb += s*2;
	*pixel_mode |= PMODE_BLUR;
	return 0;
}


Element_AQRG::~Element_AQRG() {}