#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_QRTZ PT_QRTZ 132
Element_QRTZ::Element_QRTZ()
{
    Identifier = "DEFAULT_PT_QRTZ";
    Name = "QRTZ";
    Colour = PIXPACK(0xAADDDD);
    MenuVisible = 1;
    MenuSection = SC_SOLIDS;
    Enabled = 1;
    
    Advection = 0.0f;
    AirDrag = 0.00f * CFDS;
    AirLoss = 0.90f;
    Loss = 0.00f;
    Collision = 0.0f;
    Gravity = 0.0f;
    Diffusion = 0.00f;
    HotAir = 0.000f	* CFDS;
    Falldown = 0;
    
    Flammable = 0;
    Explosive = 0;
    Meltable = 0;
    Hardness = 0;
    
    Weight = 100;
    
    Temperature = R_TEMP+0.0f	+273.15f;
    HeatConduct = 3;
    Description = "Quartz, breakable mineral. Conducts but becomes brittle at lower temperatures.";
    
    State = ST_SOLID;
    Properties = TYPE_SOLID|PROP_HOT_GLOW|PROP_LIFE_DEC;
    
    LowPressure = IPL;
    LowPressureTransition = NT;
    HighPressure = IPH;
    HighPressureTransition = NT;
    LowTemperature = ITL;
    LowTemperatureTransition = NT;
    HighTemperature = 2573.15f;
    HighTemperatureTransition = PT_LAVA;
    
    Update = &Element_QRTZ::update;
    Graphics = &Element_QRTZ::graphics;
}

//#TPT-Directive ElementHeader Element_QRTZ static int update(UPDATE_FUNC_ARGS)
int Element_QRTZ::update(UPDATE_FUNC_ARGS)
 {
	int r, tmp, trade, rx, ry, np, t;
	t = parts[i].type;
	if (t == PT_QRTZ)
	{
		parts[i].pavg[0] = parts[i].pavg[1];
		parts[i].pavg[1] = sim->pv[y/CELL][x/CELL];
		if (parts[i].pavg[1]-parts[i].pavg[0] > 0.05*(parts[i].temp/3) || parts[i].pavg[1]-parts[i].pavg[0] < -0.05*(parts[i].temp/3))
		{
			sim->part_change_type(i,x,y,PT_PQRT);
		}
	}
	// absorb SLTW
	if (parts[i].ctype!=-1)
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					else if ((r&0xFF)==PT_SLTW && (1>rand()%2500))
					{
						sim->kill_part(r>>8);
						parts[i].ctype ++;
					}
				}
	// grow if absorbed SLTW
	if (parts[i].ctype>0)
	{
		for ( trade = 0; trade<5; trade ++)
		{
			rx = rand()%3-1;
			ry = rand()%3-1;
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r && parts[i].ctype!=0)
				{
					np = sim->create_part(-1,x+rx,y+ry,PT_QRTZ);
					if (np>-1)
					{
						parts[np].tmp = parts[i].tmp;
						parts[i].ctype--;
						if (5>rand()%10)
						{
							parts[np].ctype=-1;//dead qrtz
						}
						else if (!parts[i].ctype && 1>rand()%15)
						{
							parts[i].ctype=-1;
						}

						break;
					}
				}
			}
		}
	}
	// diffuse absorbed SLTW
	if (parts[i].ctype>0)
	{
		for ( trade = 0; trade<9; trade ++)
		{
			rx = rand()%5-2;
			ry = rand()%5-2;
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)==t && (parts[i].ctype>parts[r>>8].ctype) && parts[r>>8].ctype>=0 )//diffusion
				{
					tmp = parts[i].ctype - parts[r>>8].ctype;
					if (tmp ==1)
					{
						parts[r>>8].ctype ++;
						parts[i].ctype --;
						break;
					}
					if (tmp>0)
					{
						parts[r>>8].ctype += tmp/2;
						parts[i].ctype -= tmp/2;
						break;
					}
				}
			}
		}
	}
	return 0;
}


//#TPT-Directive ElementHeader Element_QRTZ static int graphics(GRAPHICS_FUNC_ARGS)
int Element_QRTZ::graphics(GRAPHICS_FUNC_ARGS)
 //QRTZ and PQRT
{
	int t = cpart->type, z = cpart->tmp - 5;//speckles!
	/*if (cpart->temp>(ptransitions[t].thv-800.0f))//hotglow for quartz
	{
		float frequency = 3.1415/(2*ptransitions[t].thv-(ptransitions[t].thv-800.0f));
		int q = (cpart->temp>ptransitions[t].thv)?ptransitions[t].thv-(ptransitions[t].thv-800.0f):cpart->temp-(ptransitions[t].thv-800.0f);
		*colr += sin(frequency*q) * 226 + (z * 16);
		*colg += sin(frequency*q*4.55 +3.14) * 34 + (z * 16);
		*colb += sin(frequency*q*2.22 +3.14) * 64 + (z * 16);
	}
	else*/
	{
		*colr += z * 16;
		*colg += z * 16;
		*colb += z * 16;
	}
	return 0;
}


Element_QRTZ::~Element_QRTZ() {}