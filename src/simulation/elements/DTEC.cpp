#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_DTEC PT_DTEC 162
Element_DTEC::Element_DTEC()
{
	Identifier = "DEFAULT_PT_DTEC";
	Name = "DTEC";
	Colour = PIXPACK(0xFD9D18);
	MenuVisible = 1;
	MenuSection = SC_SENSOR;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.96f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 1;

	Weight = 100;

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 0;
	Description = "Detector, creates a spark when something with its ctype is nearby.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_DTEC::update;
}

//#TPT-Directive ElementHeader Element_DTEC static int update(UPDATE_FUNC_ARGS)
int Element_DTEC::update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry, rt, rd = parts[i].tmp2, pavg;
	int nx, ny, ntmp;
	if (rd > 25) parts[i].tmp2 = rd = 25;

	rt = parts[i].tmp3 & 1;
	if (!rt != !parts[i].life) // if ( tmp3 == 0 and detected ) or ( tmp3 == 1 and undetected )
	{
		parts[i].life = rt;
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					rt = r&0xFF;
					pavg = sim->parts_avg(i,r>>8,PT_INSL);
					if (pavg != PT_INSL && pavg != PT_INDI)
					{
						if ((sim->elements[rt].Properties&PROP_CONDUCTS) && !(rt==PT_WATR||rt==PT_SLTW||rt==PT_NTCT||rt==PT_PTCT||rt==PT_INWR) && parts[r>>8].life==0)
						{
							parts[r>>8].life = 4;
							parts[r>>8].ctype = rt;
							sim->part_change_type(r>>8,x+rx,y+ry,PT_SPRK);
						}
					}
				}
	}
	bool setFilt = false;
	int photonWl = 0;
	for (rx=-rd; rx<rd+1; rx++)
		for (ry=-rd; ry<rd+1; ry++)
			if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if(!r)
					r = sim->photons[y+ry][x+rx];
				if(!r)
					continue;
				if ((r&0xFF) == parts[i].ctype && (parts[i].ctype != PT_LIFE || parts[i].tmp == parts[r>>8].ctype || !parts[i].tmp))
					parts[i].life = 1;
				if ((r&0xFF) == PT_PHOT || ((r&0xFF) == PT_BRAY && parts[r>>8].tmp!=2))
				{
					setFilt = true;
					photonWl = parts[r>>8].ctype;
				}
			}
	if (setFilt)
	{
		int tempPhotWl;
#ifdef __GNUC__
		tempPhotWl = __builtin_ctz(photonWl) & 0x1F;
#else
		static char DTEC_ntztable[32] = { 0, 1, 2,24, 3,19, 6,25, 22, 4,20,10,16, 7,12,26,  31,23,18, 5,21, 9,15,11, 30,17, 8,14,29,13,28,27};
		// from "Hacker's Delight"
		tempPhotWl = (photonWl & -photonWl)*0x04D7651F;
		tempPhotWl = DTEC_ntztable[(tempPhotWl >> 27) & 31];
#endif
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					nx = x+rx;
					ny = y+ry;
					if ((r&0xFF)==PT_FILT)
					{
						while ((r&0xFF)==PT_FILT)
						{
							parts[r>>8].ctype = photonWl;
							nx += rx;
							ny += ry;
							if (nx<0 || ny<0 || nx>=XRES || ny>=YRES)
								break;
							r = pmap[ny][nx];
						}
					}
					else
					{
						while ((r&0xFF)==PT_E189)
						{
							if (parts[r>>8].life != 5)
								break;
							ntmp = parts[r>>8].tmp>>18;
							if (ntmp < 0 || ntmp > 5)
								break;
							if (ntmp == 4)
							{
								parts[r>>8].ctype &= 0x1E0;
								parts[r>>8].ctype |= tempPhotWl;
							}
							else
							{
								parts[r>>8].ctype = photonWl;
							}
							nx += rx;
							ny += ry;
							if (nx<0 || ny<0 || nx>=XRES || ny>=YRES)
								break;
							r = pmap[ny][nx];
						}
					}
				}
	}
	return 0;
}



Element_DTEC::~Element_DTEC() {}
