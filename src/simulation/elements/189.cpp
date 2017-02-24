#include "simulation/Elements.h"
#include "simulation/Air.h"
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
	Graphics = &Element_E189::graphics;
	IconGenerator = &Element_E189::iconGen;
}

//#TPT-Directive ElementHeader Element_E189 static int update(UPDATE_FUNC_ARGS)
int Element_E189::update(UPDATE_FUNC_ARGS)
{
	int tron_rx[4] = {-1, 0, 1, 0};
	int tron_ry[4] = { 0,-1, 0, 1};
	int rx, ry, ttan = 0, rlife = parts[i].life, direction, r, ri, rtmp;
	
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
		rtmp = parts[i].tmp;
		if (rtmp & 0x04)
			rtmp &= ~0x04;
		else if (rtmp & 0x01)
		{
			direction = (rtmp >> 5) & 0x3;
			r = pmap[y + tron_ry[direction]][x + tron_rx[direction]];
			if ((r & 0xFF) == PT_E189 && parts[r >> 8].life == 3)
			{
				ri = r >> 8;
				parts[ri].tmp  = rtmp;
				if (ri > i)
					sim->parts[ri].tmp |= 0x04;
				parts[ri].tmp2 = parts[i].tmp2;
			}
			rtmp = 0;
		}
		parts[i].tmp = rtmp;
		break;
	case 3:
		rtmp = parts[i].tmp;
		if (rtmp & 0x04)
			rtmp &= ~0x04;
		else if (rtmp & 0x01)
		{
			direction = (rtmp >> 5) & 0x3;
			ry = y + tron_ry[direction];
			rx = x + tron_rx[direction];
			r = pmap[ry][rx];
			if (!r)
			{
				ri = sim->create_part(-1, rx, ry, PT_TRON);
				if (ri >= 0)
				{
					sim->parts[ri].life = 5;
					sim->parts[ri].tmp  = rtmp;
					if (ri > i)
						sim->parts[ri].tmp |= 0x04;
					sim->parts[ri].tmp2 = parts[i].tmp2;
				}
			}
			rtmp = 0;
		}
		parts[i].tmp = rtmp;
		break;
	}
	
	if(ttan>=2) {
		sim->air->bmap_blockair[y/CELL][x/CELL] = 1;
		sim->air->bmap_blockairh[y/CELL][x/CELL] = 0x8;
	}
		
	return 0;
}

//#TPT-Directive ElementHeader Element_E189 static int graphics(GRAPHICS_FUNC_ARGS)
int Element_E189::graphics(GRAPHICS_FUNC_ARGS)
{
	switch(cpart->life)
	{
	case 0:
	case 1:
		if (nx & ny & 1) 
		{
			*colr = 0x66; *colg = 0x66; *colb = 0x66;
		}
		else
		{
			*colr = 0x99; *colg = 0x99; *colb = 0x99;
		}
		break;
	case 2:
		*colr = 0xFA; *colg = 0x99; *colb = 0x99;
		break;
	case 3:
		*colr = 0x99; *colg = 0xCC; *colb = 0x70;
		break;
	}
	return 0;
}

//#TPT-Directive ElementHeader Element_E189 static VideoBuffer * iconGen(int, int, int)
VideoBuffer * Element_E189::iconGen(int toolID, int width, int height)
{
	VideoBuffer * newTexture = new VideoBuffer(width, height);
	
	for (int j = 0; j < height; j++)
	{
		int r = 100, g = 150, b = 50;
		int rd = 1, gd = -1, bd = -1;
		for (int i = 0; i < width; i++)
		{
			r += 15*rd;
			g += 15*gd;
			b += 15*bd;
			if (r > 200) rd = -1;
			if (g > 200) gd = -1;
			if (b > 200) bd = -1;
			if (r < 15) rd = 1;
			if (g < 15) gd = 1;
			if (b < 15) bd = 1;
			int rc = std::min(150, std::max(0, r));
			int gc = std::min(200, std::max(0, g));
			int bc = std::min(200, std::max(0, b));
			newTexture->SetPixel(i, j, rc, gc, bc, 255);
		}
	}
	
	return newTexture;
}

Element_E189::~Element_E189() {}
