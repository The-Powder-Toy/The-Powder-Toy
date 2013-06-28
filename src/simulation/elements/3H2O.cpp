#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_3H2O PT_3H2O 174
Element_3H2O::Element_3H2O()
{
	Identifier = "DEFAULT_PT_3H2O";
	Name = "3H2O";
	Colour = PIXPACK(0x00153F);
	MenuVisible = 1;
	MenuSection = SC_NUCLEAR;
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
	Hardness = 20;
	
	Weight = 31;
	
	Temperature = R_TEMP-2.0f	+273.15f;
	HeatConduct = 251;
	Description = "Tritium oxide. Radioactive super-heavy-water.";
	
	State = ST_LIQUID;
	Properties = TYPE_LIQUID|PROP_NEUTPASS|PROP_DEADLY;
	
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;
	
	Update = &Element_3H2O::update;
	Graphics = &Element_3H2O::graphics;
}

//#TPT-Directive ElementHeader Element_3H2O static int update(UPDATE_FUNC_ARGS)
int Element_3H2O::update(UPDATE_FUNC_ARGS)
 {
	int r, rx, ry, trade, np;
	float gravtot = fabs(sim->gravy[(y/CELL)*(XRES/CELL)+(x/CELL)])+fabs(sim->gravx[(y/CELL)*(XRES/CELL)+(x/CELL)]);
	int maxlife = ((10000/(parts[i].temp + 1))-1);
	if ((10000%((int)parts[i].temp+1))>rand()%((int)parts[i].temp+1))
		maxlife ++;
	// Compress when Newtonian gravity is applied
	// multiplier=1 when gravtot=0, multiplier -> 5 as gravtot -> inf
	maxlife = maxlife*(5.0f - 8.0f/(gravtot+2.0f));
	if (parts[i].life < maxlife)
	{
		int flags;
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r || (parts[i].life >=maxlife))
						continue;
					if(r&0xFF){
						parts[i].tmp2++;
						if(parts[i].tmp == 500){
							sim->create_part(2,x,y,PT_O2);
							sim->CreateParts(x,y,rx,ry,PT_TRIT,flags);
							parts[i].tmp2 = 0;
						}
					}
					if ((r&0xFF)==PT_3H2O&& !(rand()%3))
					{
						if ((parts[i].life + parts[r>>8].life + 1) <= maxlife)
						{
							parts[i].life += parts[r>>8].life + 1;
							sim->kill_part(r>>8);
						}
					}
				}
	}
	else
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (parts[i].life<=maxlife)
						continue;
					if ((!r)&&parts[i].life>=1)//if nothing then create 3H2O
					{
						np = sim->create_part(-1,x+rx,y+ry,PT_3H2O);
						if (np<0) continue;
						parts[i].life--;
						parts[np].temp = parts[i].temp;
						parts[np].life = 0;
					}
				}
	for ( trade = 0; trade<4; trade ++)
	{
		rx = rand()%5-2;
		ry = rand()%5-2;
		if (BOUNDS_CHECK && (rx || ry))
		{
			r = pmap[y+ry][x+rx];
			if (!r)
				continue;
			if ((r&0xFF)==PT_3H2O&&(parts[i].life>parts[r>>8].life)&&parts[i].life>0)//diffusion
			{
				int temp = parts[i].life - parts[r>>8].life;
				if (temp ==1)
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


//#TPT-Directive ElementHeader Element_3H2O static int graphics(GRAPHICS_FUNC_ARGS)
int Element_3H2O::graphics(GRAPHICS_FUNC_ARGS)

{
	if(cpart->life>=700)
	{
		*firea = 60;
		*firer = *colr += cpart->life*1;
		*fireg = *colg += cpart->life*2;
		*fireb = *colb += cpart->life*3;
		*pixel_mode |= PMODE_GLOW | FIRE_ADD;
	}
	else
	{
		*colr += cpart->life*1;
		*colg += cpart->life*2;
		*colb += cpart->life*3;
		*pixel_mode |= PMODE_BLUR;
	}
	return 0;
}


Element_3H2O::~Element_3H2O() {}
