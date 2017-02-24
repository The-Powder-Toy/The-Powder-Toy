#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_E189 PT_E189 189
Element_E189::Element_E189()
{
	Identifier = "DEFAULT_PT_E189";
	Name = "E189";
	Colour = PIXPACK(0xFFB060);
	MenuVisible = 1;
	MenuSection = SC_SPECIAL;
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
	HeatConduct = 0;
	Description = "Experimental element.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_E189::update;
}

int Element_E189::update(UPDATE_FUNC_ARGS)
{
	int tron_rx[4] = {-1, 0, 1, 0};
	int tron_ry[4] = { 0,-1, 0, 1};
	int rx, ry, ttan = 0, rlife = parts[i].life, direction, r, ri;
	
	switch (rlife)
	{
	case 0:
	case 1:
		if (nt<=2)
			ttan = 2;
		else if (rlife)
			ttan = 2;
		else if (nt<=6)
			for (rx=-1; rx<2; rx++) {
				for (ry=-1; ry<2; ry++) {
					if ((!rx != !ry) && BOUNDS_CHECK) {
						if((pmap[y+ry][x+rx]&0xFF)==PT_E189)
							ttan++;
					}
				}
			}
		break;
	case 2:
		int rtmp = parts[i].tmp;
		if (rtmp & 0x04)
			rtmp &= ~0x04;
		else if (rtmp & 0x01)
		{
			rtmp = 0;
			direction = (rtmp >> 5) & 0x3;
			r = pmap[y + tron_ry[direction]][x + tron_rx[direction]];
			if ((r & 0xFF) == PT_E189 && parts[r >> 8].life == 3)
			{
				ri = r >> 8;
				parts[ri].tmp  = parts[i].tmp;
				if (ri > i)
					sim->parts[ri].tmp |= 0x04;
				parts[ri].tmp2 = parts[i].tmp2;
			}
		}
		parts[i].tmp = rtmp;
	case 3:
		int rtmp = parts[i].tmp;
		if (rtmp & 0x04)
			rtmp &= ~0x04;
		else if (rtmp & 0x01)
		{
			rtmp = 0;
			direction = (rtmp >> 5) & 0x3;
			r = pmap[y + tron_ry[direction]][x + tron_rx[direction]];
			if (!r)
			{
				ri = sim->create_part(-1, x, y, PT_TRON);
				if (ri >= 0)
				{
					sim->parts[ri].life = 5;
					sim->parts[ri].tmp  = parts[i].tmp;
					if (ri > i)
						sim->parts[ri].tmp |= 0x04;
					sim->parts[ri].tmp2 = parts[i].tmp2;
				}
			}
		}
		parts[i].tmp = rtmp;
		break;
	default:
		break;
	}
	
	if(ttan>=2) {
		sim->air->bmap_blockair[y/CELL][x/CELL] = 1;
		sim->air->bmap_blockairh[y/CELL][x/CELL] = 0x8;
	}
		
	return 0;
}

Element_E189::~Element_E189() {}
