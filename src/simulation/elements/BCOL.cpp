#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_BCOL PT_BCOL 73
Element_BCOL::Element_BCOL()
{
    Identifier = "DEFAULT_PT_BCOL";
    Name = "BCOL";
    Colour = PIXPACK(0x333333);
    MenuVisible = 1;
    MenuSection = SC_POWDERS;
    Enabled = 1;
    
    Advection = 0.4f;
    AirDrag = 0.04f * CFDS;
    AirLoss = 0.94f;
    Loss = 0.95f;
    Collision = -0.1f;
    Gravity = 0.3f;
    Diffusion = 0.00f;
    HotAir = 0.000f	* CFDS;
    Falldown = 1;
    
    Flammable = 0;
    Explosive = 0;
    Meltable = 5;
    Hardness = 2;
    
    Weight = 90;
    
    Temperature = R_TEMP+0.0f	+273.15f;
    HeatConduct = 150;
    Description = "Broken Coal. Heavy particles. See COAL";
    
    State = ST_SOLID;
    Properties = TYPE_PART;
    
    LowPressure = IPL;
    LowPressureTransition = NT;
    HighPressure = IPH;
    HighPressureTransition = NT;
    LowTemperature = ITL;
    LowTemperatureTransition = NT;
    HighTemperature = ITH;
    HighTemperatureTransition = NT;
    
    Update = &Element_BCOL::update;
    Graphics = &Element_BCOL::graphics;
}

//#TPT-Directive ElementHeader Element_BCOL static int update(UPDATE_FUNC_ARGS)
int Element_BCOL::update(UPDATE_FUNC_ARGS)
 {
	int r, rx, ry, trade, temp;
	if (parts[i].life<=0) {
		sim->create_part(i, x, y, PT_FIRE);
		return 1;
	} else if (parts[i].life < 100) {
		parts[i].life--;
		sim->create_part(-1, x+rand()%3-1, y+rand()%3-1, PT_FIRE);
	}

	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (((r&0xFF)==PT_FIRE || (r&0xFF)==PT_PLSM) && 1>(rand()%500))
				{
					if (parts[i].life>100) {
						parts[i].life = 99;
					}
				}
				if ((r&0xFF)==PT_LAVA && 1>(rand()%500))
				{
					if (parts[r>>8].ctype == PT_IRON) {
						parts[r>>8].ctype = PT_METL;
						sim->kill_part(i);
                                                return 1;
					}
				}
			}
	/*if(100-parts[i].life > parts[i].tmp2)
		parts[i].tmp2 = 100-parts[i].life;
	if(parts[i].tmp2 < 0) parts[i].tmp2 = 0;
	for ( trade = 0; trade<4; trade ++)
	{
		rx = rand()%5-2;
		ry = rand()%5-2;
		if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
		{
			r = pmap[y+ry][x+rx];
			if (!r)
				continue;
			if (((r&0xFF)==PT_COAL || (r&0xFF)==PT_BCOL)&&(parts[i].tmp2>parts[r>>8].tmp2)&&parts[i].tmp2>0)//diffusion
			{
				int temp = parts[i].tmp2 - parts[r>>8].tmp2;
				if(temp < 10)
					continue;
				if (temp ==1)
				{
					parts[r>>8].tmp2 ++;
					parts[i].tmp2 --;
				}
				else if (temp>0)
				{
					parts[r>>8].tmp2 += temp/2;
					parts[i].tmp2 -= temp/2;
				}
			}
		}
	}*/
	if(parts[i].temp > parts[i].tmp2)
		parts[i].tmp2 = parts[i].temp;
	return 0;
}


//#TPT-Directive ElementHeader Element_BCOL static int graphics(GRAPHICS_FUNC_ARGS)
int Element_BCOL::graphics(GRAPHICS_FUNC_ARGS)
 //Both COAL and Broken Coal
{
	*colr += (cpart->tmp2-295.15f)/3;
	
	if (*colr > 170)
		*colr = 170;
	if (*colr < *colg)
		*colr = *colg;
		
	*colg = *colb = *colr;

	if((cpart->temp-295.15f) > 300.0f-200.0f)
	{
		float frequency = 3.1415/(2*300.0f-(300.0f-200.0f));
		int q = ((cpart->temp-295.15f)>300.0f)?300.0f-(300.0f-200.0f):(cpart->temp-295.15f)-(300.0f-200.0f);

		*colr += sin(frequency*q) * 226;
		*colg += sin(frequency*q*4.55 +3.14) * 34;
		*colb += sin(frequency*q*2.22 +3.14) * 64;
	}
	return 0;
}



Element_BCOL::~Element_BCOL() {}