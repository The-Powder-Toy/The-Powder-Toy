#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_PQRT PT_PQRT 133
Element_PQRT::Element_PQRT()
{
	Identifier = "DEFAULT_PT_PQRT";
	Name = "PQRT";
	Colour = PIXPACK(0x88BBBB);
	MenuVisible = 1;
	MenuSection = SC_POWDERS;
	Enabled = 1;
	
	Advection = 0.4f;
	AirDrag = 0.04f * CFDS;
	AirLoss = 0.94f;
	Loss = 0.95f;
	Collision = -0.1f;
	Gravity = 0.27f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;
	
	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;
	
	Weight = 90;
	
	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 3;
	Description = "Powdered quartz, broken form of QRTZ.";
	
	State = ST_SOLID;
	Properties = TYPE_PART| PROP_HOT_GLOW;
	
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 2573.15f;
	HighTemperatureTransition = PT_LAVA;
	
	Update = &Element_PQRT::update;
	Graphics = &Element_PQRT::graphics;
}

//#TPT-Directive ElementHeader Element_PQRT static int update(UPDATE_FUNC_ARGS)
int Element_PQRT::update(UPDATE_FUNC_ARGS)
{
	int r, tmp, trade, rx, ry, np;
	// absorb SLTW
	if (parts[i].ctype!=-1)
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					else if ((r&0xFF)==PT_SLTW && !(rand()%833))
					{
						sim->kill_part(r>>8);
						parts[i].ctype ++;
					}
				}
	// grow and diffuse
	if (parts[i].ctype>0)
	{
		bool stopgrow=false;
		int rnd, sry, srx;
		for ( trade = 0; trade<9; trade ++)
		{
			rnd = rand()%0x3FF;
			rx = (rnd%5)-2;
			srx = (rnd%3)-1;
			rnd = rnd>>3;
			ry = (rnd%5)-2;
			sry = (rnd%3)-1;
			if (BOUNDS_CHECK && (rx || ry))
			{
				if (!stopgrow)//try to grow
				{
					if (!pmap[y+sry][x+srx] && parts[i].ctype!=0)
					{
						np = sim->create_part(-1,x+srx,y+sry,PT_QRTZ);
						if (np>-1)
						{
							parts[np].tmp = parts[i].tmp;
							parts[i].ctype--;
							if (rand()%2)
							{
								parts[np].ctype=-1;//dead qrtz
							}
							else if (!parts[i].ctype && !(rand()%15))
							{
								parts[i].ctype=-1;
							}
							stopgrow=true;
						}
					}
				}
				//diffusion
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				else if ((r&0xFF)==PT_PQRT && (parts[i].ctype>parts[r>>8].ctype) && parts[r>>8].ctype>=0 )
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


//#TPT-Directive ElementHeader Element_PQRT static int graphics(GRAPHICS_FUNC_ARGS)
int Element_PQRT::graphics(GRAPHICS_FUNC_ARGS)
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


Element_PQRT::~Element_PQRT() {}
