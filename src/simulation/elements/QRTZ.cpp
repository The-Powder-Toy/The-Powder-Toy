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

	Temperature = R_TEMP+273.15f;
	HeatConduct = 3;
	Description = "Quartz, breakable mineral. Conducts but becomes brittle at lower temperatures.";

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
	int r, tmp, trade, rx, ry, np, t = parts[i].type;
	if (t == PT_QRTZ)
	{
		parts[i].pavg[0] = parts[i].pavg[1];
		parts[i].pavg[1] = sim->pv[y/CELL][x/CELL];
		if (parts[i].pavg[1]-parts[i].pavg[0] > 0.05*(parts[i].temp/3) || parts[i].pavg[1]-parts[i].pavg[0] < -0.05*(parts[i].temp/3))
		{
			sim->part_change_type(i,x,y,PT_PQRT);
			parts[i].life = 5; //timer before it can grow or diffuse again
		}
	}
	if (parts[i].life>5)
		parts[i].life = 5;
	// absorb SLTW
	if (parts[i].tmp != -1)
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					else if (TYP(r)==PT_SLTW && !(rand()%500))
					{
						sim->kill_part(ID(r));
						parts[i].tmp++;
					}
				}
	// grow and diffuse
	if (parts[i].tmp > 0 && (parts[i].vx*parts[i].vx + parts[i].vy*parts[i].vy)<0.2f && parts[i].life<=0)
	{
		bool stopgrow = false;
		int rnd, sry, srx;
		for (trade = 0; trade < 9; trade++)
		{
			rnd = rand()%0x3FF;
			rx = (rnd%5)-2;
			srx = (rnd%3)-1;
			rnd >>= 3;
			ry = (rnd%5)-2;
			sry = (rnd%3)-1;
			if (BOUNDS_CHECK && (rx || ry))
			{
				if (!stopgrow)//try to grow
				{
					if (!pmap[y+sry][x+srx] && parts[i].tmp!=0)
					{
						np = sim->create_part(-1,x+srx,y+sry,PT_QRTZ);
						if (np>-1)
						{
							parts[np].temp = parts[i].temp;
							parts[np].tmp2 = parts[i].tmp2;
							parts[i].tmp--;
							if (t == PT_PQRT)
							{
								// If PQRT is stationary and has started growing particles of QRTZ, the PQRT is basically part of a new QRTZ crystal. So turn it back into QRTZ so that it behaves more like part of the crystal.
								sim->part_change_type(i,x,y,PT_QRTZ);
							}
							if (rand()%2)
							{
								parts[np].tmp=-1;//dead qrtz
							}
							else if (!parts[i].tmp && !(rand()%15))
							{
								parts[i].tmp=-1;
							}
							stopgrow=true;
						}
					}
				}
				//diffusion
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				else if (TYP(r)==PT_QRTZ && (parts[i].tmp>parts[ID(r)].tmp) && parts[ID(r)].tmp>=0)
				{
					tmp = parts[i].tmp - parts[ID(r)].tmp;
					if (tmp ==1)
					{
						parts[ID(r)].tmp++;
						parts[i].tmp--;
						break;
					}
					if (tmp>0)
					{
						parts[ID(r)].tmp += tmp/2;
						parts[i].tmp -= tmp/2;
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
	int z = (cpart->tmp2 - 5) * 16;//speckles!
	*colr += z;
	*colg += z;
	*colb += z;
	return 0;
}


Element_QRTZ::~Element_QRTZ() {}
