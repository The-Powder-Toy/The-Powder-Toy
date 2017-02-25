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
	Description = "Experimental element. has multi-purpose.";

	Properties = TYPE_SOLID | PROP_NOSLOWDOWN | PROP_TRANSPARENT;

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
	int rx, ry, ttan = 0, rlife = parts[i].life, direction, r, ri, rtmp, rctype;
	int rsign;
	float rvx, rvy, rdif;
	rtmp = parts[i].tmp;
	
	switch (rlife)
	{
	case 0: // acts like TTAN
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
	case 2: // TRON input
		if (rtmp & 0x04)
			rtmp &= ~0x04;
		else if (rtmp & 0x01)
		{
			direction = ((rtmp >> 5) + (rtmp >> 17)) & 0x3;
			r = pmap[y + tron_ry[direction]][x + tron_rx[direction]];
			if ((r & 0xFF) == PT_E189 && (parts[r >> 8].life & ~0x1) == 2)
			{
				ri = r >> 8;
				parts[ri].tmp &= 0x60000;
				parts[ri].tmp |= (rtmp & 0x1FF9F) | (direction << 5);
				if (ri > i)
					sim->parts[ri].tmp |= 0x04;
				parts[ri].tmp2 = parts[i].tmp2;
			}
			rtmp &= 0x60000;
		}
		parts[i].tmp = rtmp;
		break;
	case 3: // TRON output
		if (rtmp & 0x04)
			rtmp &= ~0x04;
		else if (rtmp & 0x01)
		{
			direction = (rtmp >> 5) & 0x3;
			ry = y + tron_ry[direction];
			rx = x + tron_rx[direction];
			r = pmap[ry][rx];
			if (r)
			{
				direction = (direction + (rand()%2) * 2 + 1) % 4;
				ry = y + tron_ry[direction];
				rx = x + tron_rx[direction];
				r = pmap[ry][rx];
				if (r)
				{
					direction = direction ^ 0x2; // bitwise xor
					ry = y + tron_ry[direction];
					rx = x + tron_rx[direction];
					r = pmap[ry][rx];
				}
				if (r)
				{
					parts[i].tmp = 0;
					break;
				}
			}
			if (!r)
			{
				ri = sim->create_part(-1, rx, ry, PT_TRON);
				if (ri >= 0)
				{
					parts[ri].life = 5;
					parts[ri].tmp  = rtmp & 0x1FF9F | (direction << 5);
					if (ri > i)
						parts[ri].tmp |= 0x04;
					parts[ri].tmp2 = parts[i].tmp2;
				}
			}
			rtmp = 0;
		}
		parts[i].tmp = rtmp;
		break;
	case 4: // photon laser
		if (!rtmp)
			break;

		rvx = (float)(((rtmp ^ 0x08) & 0x0F) - 0x08);
		rvy = (float)((((rtmp >> 4) ^ 0x08) & 0x0F) - 0x08);
		rdif = (float)((((rtmp >> 8) ^ 0x80) & 0xFF) - 0x80);

		ri = sim->create_part(-3, x + rvx, y + rvy, PT_PHOT);
		if (ri < 0)
			break;
		if (ri > i)
			parts[ri].flags |= FLAG_SKIPMOVE;
		parts[ri].vx = rvx * rdif / 16.0f;
		parts[ri].vy = rvy * rdif / 16.0f;
		rctype = parts[i].ctype;
		rtmp = rctype & 0x3FFFFFFF;
		rctype >>= 30;
		if (rtmp)
			parts[ri].ctype = rtmp;
		parts[ri].temp = parts[i].temp;
		parts[ri].life = parts[i].tmp2;
		parts[ri].tmp = parts[i].ctype & 3;
		
		break;
	case 5: // reserved for Simulation.cpp
		break;
	case 6: // heater
		for (rx=-1; rx<2; rx++) {
			for (ry=-1; ry<2; ry++) {
				if ((!rx != !ry) && BOUNDS_CHECK) {
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if (sim->elements[r&0xFF].HeatConduct > 0)
						parts[r>>8].temp = parts[i].temp;
				}
			}
		}
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
	case 4:
		*colr = 0x70; *colg = 0x20; *colb = 0x88;
		break;
	case 5:
		*colr = 0x90; *colg = 0x40; *colb = 0xA8;
		break;
	case 6:
		if ((nx ^ ny) & 1) 
		{
			*colr = 0xDC; *colg = 0x66; *colb = 0x66;
		}
		else
		{
			*colr = 0xFF; *colg = 0x99; *colb = 0x99;
		}
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

//#TPT-Directive ElementHeader Element_E189 static int interactDir(Simulation* sim, int i, int x, int y, Particle* part_phot, Particle* part_E189)
int Element_E189::interactDir(Simulation* sim, int i, int x, int y, Particle* part_phot, Particle* part_E189) // photons direction/type changer
{
	int rtmp = part_E189->tmp, rct = part_E189->ctype, mask = 0x3FFFFFFF;
	int ctype, r1;
	float rvx, rvy, rvx2, rvy2;
	long long int lsb;
	rvx = (float)(((rtmp ^ 0x80) & 0xFF) - 0x80) / 16.0f;
	rvy = (float)((((rtmp >> 8) ^ 0x80) & 0xFF) - 0x80) / 16.0f;
	switch ((rtmp >> 16) & 3)
	{
	case 0:
		part_phot->vx = rvx;
		part_phot->vy = rvy;
		break;
	case 1:
		part_phot->vx += rvx;
		part_phot->vy += rvy;
		break;
	case 2:
		rvx2 = part_phot->vx;
		rvy2 = part_phot->vy;
		part_phot->vx = rvx2 * rvx - rvy2 * rvy;
		part_phot->vy = rvx2 * rvy + rvy2 * rvx;
		break;
	case 3:
		rvx2 = rvx * 0.39269908f;
		rvy2 = hypotf(part_phot->vx, part_phot->vy);
		part_phot->vx = rvy2 * cosf(rvx2);
		part_phot->vy = rvy2 * sinf(rvx2);
		break;
	}
	switch (rtmp >> 18)
	{
	case 0: // Assign Colour
		if (rct)
			part_phot->ctype = rct;
		break;
	case 1: // Filter Colour
		if (rct)
			part_phot->ctype &= rct;
		break;
	case 2: // Add Colour
		if (rct)
			part_phot->ctype |= rct;
		break;
	case 3: // Subtract colour
		if (rct)
			part_phot->ctype &= ~rct;
		else
			part_phot->ctype = (~part_phot->ctype) & mask; // Invert colours
		break;
	case 4:
		if (rct & 0x20)
			part_phot->ctype >>= (lsb & 0x1F); // blue shift
		else
			part_phot->ctype <<= (lsb & 0x1F); // red shift
		part_phot->ctype &= mask;
		break;
	case 5:
		if (!rct) // random wavelength
		{
			ctype = part_phot->ctype;
			r1 = rand();
			r1 += (rand() << 15);
			if (rct & mask == ctype & mask)
				rct = 0;
		}
		part_phot->ctype ^= rct; // XOR colours
		break;
	case 6:
		sim->part_change_type(i, x, y, rct & 0xFF);
		part_phot->tmp = part_E189->ctype >> 8;
		break;
	}
}

Element_E189::~Element_E189() {}
