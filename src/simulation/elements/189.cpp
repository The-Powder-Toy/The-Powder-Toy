#include "simulation/Elements.h"
#include "simulation/Air.h"
#include "Probability.h"
#include "font.h"
#include <iostream>

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
	int rsign, rndstore, trade, transfer, rt;
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

		ri = sim->create_part(-3, x + (int)rvx, y + (int)rvy, PT_PHOT);
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
	case 7:
	case 13: // decoration only, no update function
		break;
	case 6: // heater
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if ((!rx != !ry) && BOUNDS_CHECK)
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if (sim->elements[r&0xFF].HeatConduct > 0)
						parts[r>>8].temp = parts[i].temp;
				}
		break;
	case 8: // acts like VIBR
		if (parts[i].tmp > 20000)
		{
			sim->emp_trigger_count += 2;
			parts[i].life = 9;
			parts[i].temp = 0;
		}
		r = sim->photons[y][x];
		rndstore = rand();
		if (r)
		{
			parts[i].tmp += 2;
			if (parts[r>>8].temp > 370.0f)
				parts[i].tmp += (int)parts[r>>8].temp - 369;
			if (3 > (rndstore & 0xF))
				sim->kill_part(r>>8);
			rndstore >>= 4;
		}
		// Pressure absorption code
		if (sim->pv[y/CELL][x/CELL] > 2.5)
		{
			parts[i].tmp += 10;
			sim->pv[y/CELL][x/CELL]--;
		}
		else if (sim->pv[y/CELL][x/CELL] < -2.5)
		{
			sim->pv[y/CELL][x/CELL]++;
		}
		// Neighbor check loop
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (BOUNDS_CHECK && (!rx != !ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if (sim->elements[r&0xFF].HeatConduct > 0)
					{
						transfer = (int)(parts[r>>8].temp - 273.15f);
						parts[i].tmp += transfer;
						parts[r>>8].temp -= (float)transfer;
					}
				}
		for (trade = 0; trade < 9; trade++)
		{
			if (trade%2)
				rndstore = rand();
			rx = rndstore%7-3;
			rndstore >>= 3;
			ry = rndstore%7-3;
			rndstore >>= 3;
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				bool not_self = !((r&0xFF) == PT_E189 && parts[r>>8].life == 8);
				if ((r&0xFF) != PT_VIBR && (r&0xFF) != PT_BVBR && not_self)
					continue;
				if (not_self)
				{
					parts[r>>8].tmp += parts[i].tmp;
					parts[i].tmp = 0;
					break;
				}
				if (parts[i].tmp > parts[r>>8].tmp)
				{
					transfer = parts[i].tmp - parts[r>>8].tmp;
					parts[r>>8].tmp += transfer/2;
					parts[i].tmp -= transfer/2;
					break;
				}
			}
		}
		if (parts[i].tmp < 0)
			parts[i].tmp = 0; // only preventing because negative tmp doesn't save
		break;
	case 9: // VIBR-like explosion
		if (parts[i].temp >= 9600)
		{
			sim->part_change_type(i, x, y, PT_VIBR);
			parts[i].temp = MAX_TEMP;
			parts[i].life = 750;
			parts[i].tmp2 = 0;
			sim->emp2_trigger_count ++;
		}
		parts[i].temp += 12;
		trade = 5;
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					if (trade >= 5)
					{
						rndstore = rand(); trade = 0;
					}
					r = pmap[y+ry][x+rx];
					rt = r & 0xFF;
					if (!r || (sim->elements[rt].Properties & PROP_NODESTRUCT) || rt == PT_VIBR || rt == PT_BVBR || rt == PT_WARP)
						continue;
					if (rt == PT_E189)
					{
						if (parts[r>>8].life == 8)
							parts[r>>8].tmp += 1000;
						continue;
					}
					if (!(rndstore & 0x7))
					{
						sim->part_change_type(r>>8, x+rx, y+ry, PT_E189);
						parts[r>>8].life = 8;
						parts[r>>8].tmp = 21000;
					}
					trade++; rndstore >>= 3;
				}
		break;
	case 10:
		for (int rx = -1; rx <= 1; rx++)
			for (int ry = -1; ry <= 1; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					int r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((r&0xFF) == PT_SPRK && parts[r>>8].life == 3)
					{
						if (rtmp & 1)
							sim->E189_pause |= 2;
						else
							sim->E189_pause |= 1;
						if ((rtmp & 2) && (rx != ry))
							Element_E189::InsertText(sim, i, x, y, -rx, -ry);
					}
				}
		break;
	case 11: // photons emitter
		for (int rx = -1; rx <= 1; rx++)
			for (int ry = -1; ry <= 1; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					int r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((r&0xFF) == PT_SPRK && parts[r>>8].life == 3)
					{
						ri = sim->create_part(-3, x-rx, y-ry, PT_PHOT);
						parts[ri].vx = (float)(-3 * rx);
						parts[ri].vy = (float)(-3 * ry);
						parts[ri].life = parts[i].tmp2;
						parts[ri].temp = parts[i].temp;

						rtmp = parts[i].ctype & 0x3FFFFFFF;
						if (rtmp)
							parts[ri].ctype = rtmp;

						if (ri > i)
							parts[ri].flags |= FLAG_SKIPMOVE;
					}
				}
	case 12: // SPRK reflector
		for (int rx = -1; rx <= 1; rx++)
			for (int ry = -1; ry <= 1; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					int r = pmap[y+ry][x+rx];
					rt = r & 0xFF;
					if (!r)
						continue;
					if (rt == PT_SPRK && parts[r>>8].life == 3)
						parts[i].tmp ^= 1;
					if (parts[i].tmp && sim->elements[rt].Properties&PROP_CONDUCTS && parts[r>>8].life==0)
					{
						parts[r>>8].life = 4;
						parts[r>>8].ctype = rt;
						sim->part_change_type(r>>8,x+rx,y+ry,PT_SPRK);
					}
				}
		break;
	case 14: // dynamic decoration (DECO2)
		switch (parts[i].tmp2 >> 24)
		{
		case 0:
			rtmp = (parts[i].tmp & 0xFFFF) + (parts[i].tmp2 & 0xFFFF);
			rctype = (parts[i].ctype >> 16) + (parts[i].tmp >> 16) + (rtmp >> 16);
			parts[i].tmp2 = (parts[i].tmp2 & ~0xFFFF) | (rtmp & 0xFFFF);
			parts[i].ctype = (parts[i].ctype & 0xFFFF) | ((rctype % 0x0600) << 16);
			break;
		case 1:
			rtmp  = (parts[i].ctype & 0x7F7F7F7F) + (parts[i].tmp & 0x7F7F7F7F);
			rtmp ^= (parts[i].ctype ^ parts[i].tmp) & 0x80808080;
			parts[i].ctype = rtmp;
			break;
		case 2:
			rtmp = parts[i].tmp2 & 0x00FFFFFF;
			rtmp ++;
			if (parts[i].tmp3 <= rtmp)
			{
				rtmp = parts[i].tmp;
				parts[i].tmp = parts[i].ctype;
				parts[i].ctype = rtmp;
				rtmp = 0;
			}
			parts[i].tmp2 = 0x02000000 | rtmp;
			break;
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
	static char excitedtable [16] = {  0, 8, 2,10,12, 4,14, 6, 3,11, 1, 9,15, 7,13, 5 };
	int ptmp, ppos, pexc1;
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
	case 7:
		*colr = 0x52; *colg = 0x52; *colb = 0x52;
		break;
	case 8:
		*colr = 0x06; *colg = 0x52; *colb = 0x06;
		ptmp = cpart->tmp >> 3;
		ppos = (nx & 3) | ((ny & 3) << 2);
		pexc1 = ptmp >> 4;
		if (pexc1 > 5)
			pexc1 = 6;
		else if ((ptmp & 0x0F) > excitedtable[ppos])
			pexc1 ++;
		ptmp = pexc1 << 5;
		*colr += ptmp;
		if (pexc1 < 6)
			*colg += ptmp;
		else
			*colg = 0xFF;
		*colb += ptmp;
		break;
	case 9:
		*colr = *colb = 255 - (int)(0.0091f * cpart->temp);
		*colg = 0xFF;
		*firea = 90; *firer = *colr; *fireg = *colg; *fireb = *colb;
		*pixel_mode = PMODE_NONE;
		*pixel_mode |= FIRE_BLEND;
		break;
	case 10:
		*colr = 0xBC; *colg = 0x00; *colb = 0xBC;
		break;
	case 11:
		*colr = 0x90; *colg = 0x40; *colb = 0xA8;
		break;
	case 12:
		*colr = 0xBF; *colg = 0xFF; *colb = 0x05;
		break;
	case 13:
		switch (cpart->tmp2 & 0x3)
		{
		case 0:
			ptmp = cpart->ctype;
			if (cpart->tmp2 & 0x4)
			{
				unsigned int pcolor = cpart->tmp;
				if ((pcolor >> 24) > (rand() % 255)) // (pcolor >>> 24) > (randint (between 0 and 254 include))
				{
					ptmp &= 0xFF000000;
					ptmp |= pcolor & 0x00FFFFFF;
				}
			}
			else
				ptmp ^= (cpart->tmp << 16);
			*cola = (ptmp >> 24) & 0xFF;
			*colr = (ptmp >> 16) & 0xFF;
			*colg = (ptmp >> 8) & 0xFF;
			*colb = ptmp & 0xFF;
			break;
		case 1:
			int x;
			ptmp = cpart->ctype;
			*colr = *colg = *colb = 0;
			for (x=0; x<12; x++) {
				*colr += (ptmp >> (x+18)) & 1;
				*colb += (ptmp >> x)     & 1;
			}
			for (x=0; x<12; x++)
				*colg += (ptmp >> (x+9))  & 1;

			x = 624/(*colr+*colg+*colb+1);
			*colr *= x; *colg *= x; *colb *= x;
			*cola = ~(cpart->tmp) & 0xFF;
			break;
		case 2:
			{
			Element_E189::HSV2RGB (cpart->ctype, colr, colg, colb);
			*cola = ~(cpart->tmp) & 0xFF;
			break;
			}
		}
		*pixel_mode &= ~PMODE;
		*pixel_mode |= PMODE_BLEND;
		break;
	case 14:
		switch (cpart->tmp2 >> 24)
		{
		case 0:
			Element_E189::HSV2RGB (cpart->ctype, colr, colg, colb);
			*cola = ~(cpart->tmp2 >> 16) & 0xFF;
			break;
		case 1:
			{
			ptmp = cpart->ctype;
			*cola = (ptmp >> 24) & 0xFF;
			float freqr = 0.024543693f * (float)((ptmp >> 16) & 0xFF);
			float freqg = 0.024543693f * (float)((ptmp >>  8) & 0xFF);
			float freqb = 0.024543693f * (float)( ptmp & 0xFF );
			*colr = (int)(128.0f + 127.5f * sinf(freqr));
			*colg = (int)(128.0f + 127.5f * sinf(freqg));
			*colb = (int)(128.0f + 127.5f * sinf(freqb));
			break;
			}
		case 2:
			ptmp = cpart->ctype;
			*cola = (ptmp >> 24) & 0xFF;
			*colr = (ptmp >> 16) & 0xFF;
			*colg = (ptmp >> 8) & 0xFF;
			*colb = ptmp & 0xFF; 
			break;
		}
		*pixel_mode &= ~PMODE;
		*pixel_mode |= PMODE_BLEND;
		break;
	case 15:
		*colr = 0x9F; *colg = 0x05; *colb = 0x00;
		break;
	}
	return 0;
}

//#TPT-Directive ElementHeader Element_E189 static void HSV2RGB(int ctype, int *r, int *g, int *b)
void Element_E189::HSV2RGB (int ctype, int *r, int *g, int *b)
{
	int ptmp = ctype;
	float tmpr, tmpg, tmpb;
	float hh, ss, vv, cc;
	int phue = (ptmp >> 16) % 0x600;
	if (phue < 0)
		phue += 0x600;
	hh = (float)phue / 256.0f;
	ss = (float)((ptmp >> 8) & 0xFF) / 255.0f;
	vv = (float)(ptmp & 0xFF);
	cc = vv * ss;
	int p_add = (int)(vv - cc);
	switch (phue >> 8)
	{
	case 0:
		tmpr = cc;
		tmpg = cc * hh;
		tmpb = 0.0f;
		break;
	case 1:
		tmpr = cc * (2.0f - hh);
		tmpg = cc;
		tmpb = 0.0f;
		break;
	case 2:
		tmpr = 0.0f;
		tmpg = cc;
		tmpb = cc * (hh - 2.0f);
		break;
	case 3:
		tmpr = 0.0f;
		tmpg = cc * (4.0f - hh);
		tmpb = cc;
		break;
	case 4:
		tmpr = cc * (hh - 4.0f);
		tmpg = 0.0f;
		tmpb = cc;
		break;
	case 5:
		tmpr = cc;
		tmpg = 0.0f;
		tmpb = cc * (6.0f - hh);
		break;
	}
	*r = (int)tmpr + p_add;
	*g = (int)tmpg + p_add;
	*b = (int)tmpb + p_add;
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

//#TPT-Directive ElementHeader Element_E189 static void interactDir(Simulation* sim, int i, int x, int y, Particle* part_phot, Particle* part_E189)
void Element_E189::interactDir(Simulation* sim, int i, int x, int y, Particle* part_phot, Particle* part_E189) // photons direction/type changer
{
	int rtmp = part_E189->tmp, rct = part_E189->ctype, mask = 0x3FFFFFFF;
	int ctype, r1, r2;
	float rvx, rvy, rvx2, rvy2, rdif;
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
		rdif = hypotf(part_phot->vx, part_phot->vy);
		if (rtmp & 0x100)
		{
			rvy2 = atan2f(part_phot->vy, part_phot->vx);
			rvx2 = rvx2 - rvy2;
		}
		part_phot->vx = rdif * cosf(rvx2);
		part_phot->vy = rdif * sinf(rvx2);
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
		ctype = part_phot->ctype;
		switch ((rct >> 5) & 15)
		{
		case 0:
			part_phot->ctype <<= (rct & 0x1F); // red shift
			break;
		case 1:
			part_phot->ctype >>= (rct & 0x1F); // blue shift
			break;
		case 2:
			r1 = (rct & 0x1F) % 30;
			part_phot->ctype = (ctype << r1) | (ctype >> (30 - r1)); // rotate red shift
			break;
		case 3:
			r1 = (rct & 0x1F) % 30;
			part_phot->ctype = (ctype >> r1) | (ctype << (30 - r1)); // rotate blue shift
			break;
		case 4:
			part_phot->ctype &= ~(1 << (rct & 0x1F)); // set flag 0
			break;
		case 5:
			part_phot->ctype |=  (1 << (rct & 0x1F)); // set flag 1
			break;
		case 6:
			part_phot->ctype ^=  (1 << (rct & 0x1F)); // toggle flag
			break;
		case 7:
			if (rand() & 1) // random toggle flag
				part_phot->ctype ^=  (1 << (rct & 0x1F));
			break;
		case 8: // reversing wavelength from "Hacker's Delight"
			r1 = part_phot->ctype;
			r2 = (r1 << 15) | (r1 >> 15); // wavelength rotate 15
			r1 = (r2 ^ (r2>>10)) & 0x000F801F; // swap 10
			r2 ^= (r1 | (r1<<10));
			r1 = (r2 ^ (r2>> 3)) & 0x06318C63; // swap 3
			r2 ^= (r1 | (r1<< 3));
			r1 = (r2 ^ (r2>> 1)) & 0x1294A529; // swap 1
			part_phot->ctype = (r1 | (r1<< 1)) ^ r2;
			break;
		case 15: // get "extraLoopsCA" info, without pause state
			if (!sim->extraLoopsCA)
				r1 = 0x1;
			else
				r1 = 0x2 << sim->extraLoopsType;
			if (sim->elementCount[PT_LOVE] > 0)
				r1 |= 0x10;
			if (sim->elementCount[PT_LOLZ] > 0)
				r1 |= 0x20;
			if (sim->elementCount[PT_WIRE] > 0)
				r1 |= 0x40;
			if (sim->elementCount[PT_LIFE] > 0)
				r1 |= 0x80;
			if (sim->player.spwn)
				r1 |= 0x100;
			if (sim->player2.spwn)
				r1 |= 0x200;
			if (sim->elementCount[PT_WIFI] > 0)
				r1 |= 0x400;
			if (sim->elementCount[PT_DMND] > 0)
				r1 |= 0x800;
			if (sim->elementCount[PT_INSL] > 0)
				r1 |= 0x1000;
			if (sim->elementCount[PT_INDI] > 0)
				r1 |= 0x2000;
			part_phot->ctype = r1;
			break;
		}
		part_phot->ctype &= mask;
		break;
	case 5:
		if (!rct) // random wavelength
		{
			ctype = part_phot->ctype;
			r1 = rand();
			r1 += (rand() << 15);
			if ((r1 ^ ctype) & mask == 0)
				rct = 0;
			else
				rct = r1;
		}
		part_phot->ctype ^= rct; // XOR colours
		break;
	case 6:
		sim->part_change_type(i, x, y, rct & 0xFF);
		part_phot->tmp = part_E189->ctype >> 8;
		break;
	case 7: // photon scattering
		sim->part_change_type(i, x, y, PT_E186);
		if (rct & 1)
			part_phot->ctype = 0x1F<<(rand()%26);
		part_phot->tmp2 = part_phot->ctype;
		part_phot->ctype = 0x100;
		rdif = ((float)(rand()%321+192)) / 128.0f; // 1.5 ~ 4 pixels (approx)
		rvx2 = ((float)(rand()& 0x7fff)) * 1.9174760e-4f; // RAND_MAX is at least 32767 on all platforms
		part_phot->vx = rdif*cosf(rvx2);
		part_phot->vy = rdif*sinf(rvx2);
		break;
	}
}

//#TPT-Directive ElementHeader Element_E189 static void duplicatePhotons(Simulation* sim, int i, int x, int y, Particle* part_phot, Particle* part_E189)
void Element_E189::duplicatePhotons(Simulation* sim, int i, int x, int y, Particle* part_phot, Particle* part_E189)
{
	int rtmp = part_E189->tmp, ri;
	if (!rtmp)
		return;
	float rvx = (float)(((rtmp ^ 0x08) & 0x0F) - 0x08);
	float rvy = (float)((((rtmp >> 4) ^ 0x08) & 0x0F) - 0x08);
	float rdif = (float)((((rtmp >> 8) ^ 0x80) & 0xFF) - 0x80);
	
	ri = sim->create_part(-3, (int)((float)x + (float)rvx + (float)part_phot->vx + 0.5f), (int)((float)y + (float)rvy + (float)part_phot->vy + 0.5f), PT_PHOT);
	if (ri < 0)
		return;
	if (ri > i)
		sim->parts[ri].flags |= FLAG_SKIPMOVE;
	sim->parts[ri].vx = rvx * rdif / 16.0f;
	sim->parts[ri].vy = rvy * rdif / 16.0f;
	sim->parts[ri].temp = part_phot->temp;
	sim->parts[ri].tmp  = part_phot->tmp;
	sim->parts[ri].life = part_E189->tmp2;
	if (part_E189->ctype)
		sim->parts[ri].ctype = part_E189->ctype;
	else
		sim->parts[ri].ctype = part_phot->ctype;
}

//#TPT-Directive ElementHeader Element_E189 static int EMPTrigger(Simulation *sim, int triggerCount)
int Element_E189::EMPTrigger(Simulation *sim, int triggerCount)
{
	int t, ct, rx, ry, r1;
	Particle *parts = sim->parts;
	
	float prob_breakPInsulator = Probability::binomial_gte1(triggerCount, 1.0f/200);
	float prob_breakTRONPortal = Probability::binomial_gte1(triggerCount, 1.0f/160);
	float prob_randLaser = Probability::binomial_gte1(triggerCount, 1.0f/40);
	float prob_breakLaser = Probability::binomial_gte1(triggerCount, 1.0f/120);
	float prob_breakDChanger = Probability::binomial_gte1(triggerCount, 1.0f/160);
	float prob_breakHeater = Probability::binomial_gte1(triggerCount, 1.0f/100);
	float prob_breakElectronics = Probability::binomial_gte1(triggerCount, 1.0f/300);

	for (int r = 0; r <=sim->parts_lastActiveIndex; r++)
	{
		t = parts[r].type;
		rx = parts[r].x;
		ry = parts[r].y;
		switch ( t )
		{
		case PT_DMND:
			break;
		case PT_METL:
			if (Probability::randFloat() < prob_breakElectronics)
				sim->part_change_type(r, rx, ry, PT_BMTL);
			break;
		case PT_BMTL:
		case PT_PIPE:
		case PT_PPIP:
			if (Probability::randFloat() < prob_breakElectronics)
				sim->part_change_type(r, rx, ry, PT_BRMT);
			break;
		case PT_GLAS:
		case PT_LCRY:
		case PT_FILT:
			if (Probability::randFloat() < prob_breakElectronics)
				sim->part_change_type(r, rx, ry, PT_BGLA);
			break;
		case PT_QRTZ:
			if (Probability::randFloat() < prob_breakElectronics)
				sim->part_change_type(r, rx, ry, PT_PQRT);
			break;
		case PT_TTAN:
		case PT_GOLD:
		case PT_VOID:
		case PT_PVOD:
		case PT_CONV:
			if (Probability::randFloat() < prob_breakElectronics)
			{
				sim->part_change_type(r, rx, ry, PT_E189);
				parts[r].life = 8;
				parts[r].tmp = 21000;
			}
			break;
		case PT_CRMC:
			if (Probability::randFloat() < prob_breakElectronics)
				sim->part_change_type(r, rx, ry, PT_CLST);
			break;
		case PT_BRCK:
			if (Probability::randFloat() < prob_breakElectronics)
				sim->part_change_type(r, rx, ry, PT_STNE);
			break;
		case PT_PSCN:
		case PT_NSCN:
		case PT_PTCT:
		case PT_NTCT:
		case PT_SWCH:
		case PT_WIFI:
		case PT_SPRK:
			if (Probability::randFloat() < prob_breakElectronics)
				sim->part_change_type(r, rx, ry, PT_BREC);
			break;
		case PT_CLNE:
		case PT_PCLN:
			if (Probability::randFloat() < prob_breakElectronics)
				sim->part_change_type(r, rx, ry, PT_BCLN);
			break;
		case PT_E189:
			switch (parts[r].life)
			{
			case 0:
			case 1:
				if (Probability::randFloat() < prob_breakPInsulator)
				{
					parts[r].life = 8;
					parts[r].tmp = 21000;
				}
				break;
			case 12:
				if (Probability::randFloat() < prob_breakElectronics)
				{
					sim->part_change_type(r, rx, ry, PT_BREC);
				}
				break;
			case 2:
			case 3:
				if (Probability::randFloat() < prob_breakTRONPortal)
				{
					sim->create_part(r, rx, ry, PT_PLSM);
				}
				break;
			case 4:
			case 7:
			case 11:
				if (Probability::randFloat() < prob_randLaser)
				{
					parts[r].ctype += (rand() << 15) + rand();
					parts[r].tmp = (parts[r].tmp + rand()) & 0x0000FFFF;
				}
				if (Probability::randFloat() < prob_breakLaser)
				{
					sim->create_part(r, rx, ry, PT_BRMT);
				}
				break;
			case 5:
				if (Probability::randFloat() < prob_breakDChanger)
				{
					sim->create_part(r, rx, ry, PT_BGLA);
				}
				break;
			case 6:
				if (Probability::randFloat() < prob_breakHeater)
				{
					sim->create_part(r, rx, ry, PT_PLSM);
				}
			}
			break;
		}
	}
}

//#TPT-Directive ElementHeader Element_E189 static int AddCharacter(Simulation *sim, int x, int y, int c, int rgb)
int Element_E189::AddCharacter(Simulation *sim, int x, int y, int c, int rgb)
{
	int i, j, w, bn = 0, ba = 0, _r, xi, yj;
	unsigned char *rp = font_data + font_ptrs[c];
	w = *(rp++);
	for (j=0; j<FONT_H; j++)
		for (i=0; i<w; i++)
		{
			if (!bn)
			{
				ba = *(rp++);
				bn = 8;
			}
			if (ba & 3)
			{
				xi = x + i; yj = y + j;
				_r = sim->pmap[yj][xi];
				if (_r)
				{
					if ((_r&0xFF) == PT_E189 && sim->parts[_r>>8].life == 13)
					{
						if (~ba & 3) // ba & 3 != 3, also only ba == 1 or ba == 2
						{
							int k = sim->parts[_r>>8].ctype;
							int olda = (k >> 24) & 0xFF;
							int oldr = (olda * ((k >> 16) & 0xFF)) >> 8;
							int oldg = (olda * ((k >> 8) & 0xFF)) >> 8;
							int oldb = (olda * (k & 0xFF)) >> 8;
							olda = ba * 0x55;
							int newr = (olda * ((rgb >> 16) & 0xFF) + (0xFF - olda) * oldr) & ~0xFF;
							int newg = (olda * ((rgb >> 8) & 0xFF) + (0xFF - olda) * oldg) & 0xFF00;
							int newb = (olda * (rgb & 0xFF) + (0xFF - olda) * oldb) >> 8;
							sim->parts[_r>>8].ctype = 0xFF000000 | newr << 8 | newg | newb;
						}
						else
							sim->parts[_r].ctype = 0xFF000000 | (rgb & 0x00FFFFFF);
					}
					else if (!(sim->elements[_r>>8].Properties & PROP_NODESTRUCT))
						_r = sim->create_part(_r>>8, xi, yj, PT_E189, 13);
				}
				else
					_r = sim->create_part(-1, xi, yj, PT_E189, 13); // type = 65549 (0x0001000D)
				if (_r >= 0)
				{
					sim->parts[_r].ctype = ((ba & 3) * 0x55000000) | (rgb & 0x00FFFFFF);
				}
			}
			ba >>= 2;
			bn -= 2;
		}
	return x + w;
}

//#TPT-Directive ElementHeader Element_E189 static void InsertText(Simulation *sim, int i, int x, int y, int ix, int iy)
void Element_E189::InsertText(Simulation *sim, int i, int x, int y, int ix, int iy)
{
	// simulation, index, position (x2), direction (x2)
	int ct_x = (sim->parts[i].ctype & 0xFFFF), ct_y = ((sim->parts[i].ctype >> 16) & 0xFFFF);
	int it_x = ct_x, it_r, it_g, it_b, chr_1, esc = 0, pack, bkup, errflag = 0, cfptr;
	int oldr, oldg, oldb, call_ptr = 0;
	short counter = 0;
	short calls[128][5]; /* dynamic */
	it_r = it_g = it_b = 255;
	for (;;)
	{
		x += ix; y += iy;
		int r = sim->pmap[y][x];
		if ((r&0xFF) != PT_E189) // if not "E189" break loop
			break;
		pack = sim->parts[r>>8].life;
		chr_1 = sim->parts[r>>8].ctype;
		if (pack & ~0x1 == 0x2) // if "tron portal"
		{
			if (pack == 2) // if "tron input"
			{
				if (esc == 0)
					esc = 5;
				else if (esc = 1)
					bkup = 1;
			}
			else
			{
				if (esc == 1)
				{
					it_r = oldr; it_g = oldg; it_b = oldb;
				}
				esc = 0;
			}
			continue;
		}
		if (pack == 12) // if "spark reflector"
		{
			switch (chr_1 & 63) // chr_1 : initial ctype
			{
				case 0: ix = 1; iy = 0; break; // go east
				case 1: ix = 0; iy =-1; break; // go north
				case 2: ix =-1; iy = 0; break; // go west
				case 3: ix = 0; iy = 1; break; // go south
				case 4: // turn clockwise
					chr_1 = ix; ix = iy; iy = -chr_1;
				break;
				case 5: // turn counter clockwise
					chr_1 = ix; ix = -iy; iy = chr_1;
				break;
				case 6: /* "/" reflect */
					chr_1 = ix; ix = iy; iy = chr_1;
				break;
				case 7: /* "\" reflect */
					chr_1 = ix; ix = -iy; iy = -chr_1;
				break;
				case 8: /* "|" reflect */
					ix = -ix;
				break;
				case 9: /* "-" reflect */
					iy = -iy;
				break;
				case 10: /* turn 180 */
					ix = -ix; iy = -iy;
				break;
				case 11: /* random turn */
					pack = (rand() & 1) * 2 - 1;
					chr_1 = ix; ix = iy * pack; iy = chr_1 * pack;
				break;
				case 12: /* random straight */
					pack = (rand() & 1) * 2 - 1;
					ix *= pack; iy *= pack;
				break;
				case 13: /* random dir. */
				{
					int turn_rx[4] = {-1, 0, 1, 0};
					int turn_ry[4] = { 0,-1, 0, 1};
					chr_1 = (rand() & 3);
					ix = turn_rx[chr_1]; iy = turn_ry[chr_1];
				}
				case 14: // random vertical
					ix = 0; iy = (rand() & 1) * 2 - 1;
				break;
				case 15: // random horizontal
					iy = 0; ix = (rand() & 1) * 2 - 1;
				break;
				case 16: // trampoline
					x += ix; y += iy;
				break;
				case 17: // trampoline 2
					x += ix * 2; y += iy * 2;
				break;
				case 18: // trampoline 3 ... N
					r = sim->pmap[y+iy][x+ix];
					if ((r & 0xFF) == PT_E189)
					{
						chr_1 = sim->parts[r>>8].life;
						if (chr_1 & ~0x1 == 0x2)
							chr_1 += 2; // trampoline 4, 5
						else if (chr_1 == 12)
							chr_1 = sim->parts[r>>8].ctype; // trampoline N
						else
							chr_1 = 3; // trampoline 3
					}
					else
						chr_1 = 3;
					x += ix * chr_1; y += iy * chr_1;
				break;
				case 19: // random dir. w/o backward
					pack = rand() % 3;
					if (pack)
					{
						pack = pack * 2 - 3;
						chr_1 = ix; ix = iy * pack; iy = chr_1 * pack;
					}
				break;
				case 20: // function call (stack push)
					if (call_ptr >= 128)
					{
						std::cerr << "stack overflow!" << std::endl;
						errflag = 1;
						call_ptr = 0;
						x  = (int)(calls[0][0]);
						y  = (int)(calls[0][1]);
						ix = (int)(calls[0][2]);
						iy = (int)(calls[0][3]);
						cfptr = -1;
					}
					else
					{
						calls[call_ptr][0] = (short)(x+ix);
						calls[call_ptr][1] = (short)(y+iy);
						calls[call_ptr][2] = (short)ix;
						calls[call_ptr][3] = (short)iy;
						calls[call_ptr][4] = (short)cfptr;
						cfptr = call_ptr++;
					}
				break;
				case 21: // function return (stack pop)
					if (call_ptr <= 0)
						goto __break_loop_1;
					call_ptr = cfptr;
					x  = (int)(calls[call_ptr][0]);
					y  = (int)(calls[call_ptr][1]);
					ix = (int)(calls[call_ptr][2]);
					iy = (int)(calls[call_ptr][3]);
					cfptr = (int)(calls[call_ptr][4]);
				break;
				case 22: // push color data (stack push)
					if (call_ptr < 128)
					{
						calls[call_ptr][0] = (short)it_r;
						calls[call_ptr][1] = (short)it_g;
						calls[call_ptr][2] = (short)it_b;
						call_ptr++;
					}
					else
					{
						std::cerr << "stack overflow!" << std::endl;
						errflag = 1; call_ptr = 0;
					}
				break;
				case 23: // pop color data (stack pop)
					if (call_ptr <= 0)
						goto __break_loop_1;
					call_ptr --;
					it_r = (int)(calls[call_ptr][0]);
					it_g = (int)(calls[call_ptr][1]);
					it_b = (int)(calls[call_ptr][2]);
				break;
				case 24: // push counter register
					if (call_ptr < 128)
					{
						calls[call_ptr][0] = counter;
						call_ptr++;
					}
					else
					{
						std::cerr << "stack overflow!" << std::endl;
						errflag = 1; call_ptr = 0;
					}
				break;
				case 25: // pop counter register
					if (call_ptr <= 0)
						goto __break_loop_1;
					call_ptr --;
					counter = (int)(calls[call_ptr][0]);
				break;
				case 26: // if stack overflow then trampoline
					if (errflag) { x += ix; y += iy; }
				break;
				case 27: // if stack not overflow then trampoline
					if (!errflag) { x += ix; y += iy; }
				break;
				case 28: // if counter is non-zero then trampoline
					r = sim->pmap[y+iy][x+ix];
					if ((r & 0xFF) == PT_E189)
					{
						pack = sim->parts[r>>8].life;
						if (pack & ~0x1 == 0x2)
						{
							x += ix; y += iy;
						}
					}
					if (counter) { x += ix; y += iy; }
				break;
				case 29: // if counter is zero then trampoline
					if ((r & 0xFF) == PT_E189)
					{
						pack = sim->parts[r>>8].life;
						if (pack & ~0x1 == 0x2)
						{
							x += ix; y += iy;
						}
					}
					if (!counter) { x += ix; y += iy; }
				break;
				case 30: // counter increment by 1
					counter++;
				break;
				case 31: // counter decrement by 1
					counter--;
				break;
				case 32: // set counter value / set error flag
					r = sim->pmap[y+iy][x+ix];
					if ((r & 0xFF) == PT_E189)
					{
						pack = sim->parts[r>>8].life;
						if (pack == 12)
						{
							x += ix; y += iy;
							counter = (short)sim->parts[r>>8].ctype;
						}
						else if (pack & ~0x1 == 0x2)
						{
							x += ix; y += iy;
							errflag = pack & 0x1;
						}
					}
				break;
				case 33: // toggle error flag
					errflag = !errflag;
				break;
				case 34: // push from stack
					call_ptr = (call_ptr + 1) % 128;
				break;
				case 35: // pop from stack
					call_ptr = (call_ptr + 127) % 128;
				break;
				case 36: // add constant
					r = sim->pmap[y+iy][x+ix];
					pack = sim->parts[r>>8].life;
					if ((r & 0xFF) == PT_E189 && pack == 12)
					{
						x += ix; y += iy;
						counter += (short)sim->parts[r>>8].ctype;
					}
					else
						counter += calls[call_ptr-1][0];
				break;
				case 37: // subtract constant
					r = sim->pmap[y+iy][x+ix];
					pack = sim->parts[r>>8].life;
					if ((r & 0xFF) == PT_E189 && pack == 12)
					{
						x += ix; y += iy;
						counter -= (short)sim->parts[r>>8].ctype;
					}
					else
						counter -= calls[call_ptr-1][0];
				break;
				case 38: // multiply constant
					r = sim->pmap[y+iy][x+ix];
					pack = sim->parts[r>>8].life;
					if ((r & 0xFF) == PT_E189 && pack == 12)
					{
						x += ix; y += iy;
						counter *= (short)sim->parts[r>>8].ctype;
					}
					else
						counter *= calls[call_ptr-1][0];
				break;
				case 39: // bitwise and
					r = sim->pmap[y+iy][x+ix];
					pack = sim->parts[r>>8].life;
					if ((r & 0xFF) == PT_E189 && pack == 12)
					{
						x += ix; y += iy;
						counter &= (short)sim->parts[r>>8].ctype;
					}
					else
						counter &= calls[call_ptr-1][0];
				break;
				case 40: // bitwise or
					r = sim->pmap[y+iy][x+ix];
					pack = sim->parts[r>>8].life;
					if ((r & 0xFF) == PT_E189 && pack == 12)
					{
						x += ix; y += iy;
						counter |= (short)sim->parts[r>>8].ctype;
					}
					else
						counter |= calls[call_ptr-1][0];
				break;
				case 41: // bitwise xor
					r = sim->pmap[y+iy][x+ix];
					pack = sim->parts[r>>8].life;
					if ((r & 0xFF) == PT_E189 && pack == 12)
					{
						x += ix; y += iy;
						counter ^= (short)sim->parts[r>>8].ctype;
					}
					else
						counter ^= calls[call_ptr-1][0];
				break;
				case 42: // bitwise shift
					r = sim->pmap[y+iy][x+ix];
					pack = sim->parts[r>>8].life;
					if ((r & 0xFF) == PT_E189 && pack == 12)
					{
						x += ix; y += iy;
						pack = sim->parts[r>>8].ctype;
					}
					else
						pack = calls[call_ptr-1][0];
					switch ((pack >> 4) & 3)
					{
						case 0: counter = counter << (pack & 0xF); break;
						case 1: counter = (signed short)counter >> (pack & 0xF); break;
						case 2: counter = (unsigned short)counter >> (pack & 0xF); break;
						case 3: counter = (counter << (pack & 0xF)) | ((unsigned short)counter >> (-pack & 0xF)); break;
					}
				break;
				case 43: // swap counter / stack
					pack = (int)calls[call_ptr-1][0];
					calls[call_ptr-1][0] = counter;
					counter = (short)pack;
				break;
				case 44: // if stack top less than counter then trampoline
					if (calls[call_ptr-1][0] < counter) { x += ix; y += iy; }
				break;
				case 45: // if counter less than stack top then trampoline
					if (calls[call_ptr-1][0] > counter) { x += ix; y += iy; }
				break;
				case 46: // if stack top and counter is equal then trampoline
					if (calls[call_ptr-1][0] == counter) { x += ix; y += iy; }
				break;
				case 47: // if stack top and counter is not equal then trampoline
					if (calls[call_ptr-1][0] != counter) { x += ix; y += iy; }
				break;
				}
			continue;
		}
		if (pack != 10)
			break;
		if (!esc)
		{
			switch (chr_1)
			{
			case 0: // no operation
				break;
			case 10: // "\n": newline
				ct_x = it_x;
				ct_y += FONT_H+2; // usually 12 pixels
				break;
			case 15:
				esc = 1;
				bkup = 0;
				break;
			case 256:
				esc = 2;
				break;
			case 257:
				esc = 3;
				break;
			case 258:
				esc = 4;
				break;
			case 259: // random character
				ct_x = Element_E189::AddCharacter(sim, ct_x, ct_y, (rand() & 0xFF), (it_r << 16) | (it_g << 8) | it_b);
				break;
			case 260: // random ASCII
				ct_x = Element_E189::AddCharacter(sim, ct_x, ct_y, (rand() % 95 + ' '), (it_r << 16) | (it_g << 8) | it_b);
				break;
			case 261: // random number
				ct_x = Element_E189::AddCharacter(sim, ct_x, ct_y, (rand() % 10 + '0'), (it_r << 16) | (it_g << 8) | it_b);
				break;
			case 262: // random uppercase
				ct_x = Element_E189::AddCharacter(sim, ct_x, ct_y, (rand() % 26 + 'A'), (it_r << 16) | (it_g << 8) | it_b);
				break;
			case 263: // random lowercase
				ct_x = Element_E189::AddCharacter(sim, ct_x, ct_y, (rand() % 26 + 'a'), (it_r << 16) | (it_g << 8) | it_b);
				break;
			case 264: // random mixed alphabet
				pack = rand();
				ct_x = Element_E189::AddCharacter(sim, ct_x, ct_y, ((pack >> 1) % 26 + ((pack % 2) << 5) + 'A'), (it_r << 16) | (it_g << 8) | it_b);
				break;
			case 265: // random mixed alphanumeric
				pack = rand() % 62;
				pack += ((pack > 36) ? 29 : (pack > 10) ? 87 : '0');
				ct_x = Element_E189::AddCharacter(sim, ct_x, ct_y, pack, (it_r << 16) | (it_g << 8) | it_b);
				break;
			case 266:
				ct_x = Element_E189::AddCharacter(sim, ct_x, ct_y, counter & 0xFF, (it_r << 16) | (it_g << 8) | it_b);
				break;
			case 267:
				ct_x = Element_E189::AddCharacter(sim, ct_x, ct_y, calls[call_ptr-1][0] & 0xFF, (it_r << 16) | (it_g << 8) | it_b);
				break;
			case 268: // print number
				chr_1 = (int)counter;
				pack = (it_r << 16) | (it_g << 8) | it_b;
				if (pack < 0)
				{
					chr_1 = -chr_1; ct_x = Element_E189::AddCharacter(sim, ct_x, ct_y, '-', pack);
				}
				while (chr_1)
				{
					ct_x = Element_E189::AddCharacter(sim, ct_x, ct_y, chr_1 % 10, pack);
					chr_1 /= 10;
				}
				break;
			default:
				ct_x = Element_E189::AddCharacter(sim, ct_x, ct_y, chr_1, (it_r << 16) | (it_g << 8) | it_b);
			}
		}
		else
		{
			switch (esc)
			{
			case 1: // set color
				if (bkup)
				{
					oldr = it_r; oldg = it_g; oldb = it_b;
				}
				it_r = (chr_1 >> 16) & 0xFF;
				it_g = (chr_1 >> 8) & 0xFF;
				it_b =  chr_1 & 0xFF;
				esc = 0;
				break;
			case 2: // set location (fixed)
				ct_x =  chr_1 & 0xFFFF;
				ct_y = (chr_1 >> 16) & 0xFFFF;
				esc = 0;
				break;
			case 3: // reinitial location
				it_x =  chr_1 & 0xFFFF;
				ct_x =  it_x;
				ct_y = (chr_1 >> 16) & 0xFFFF;
				esc = 0;
				break;
			case 4: // set location (relative)
				ct_x += (signed short)( chr_1 & 0xFFFF);
				ct_y += (signed short)((chr_1 >> 16) & 0xFFFF);
				esc = 0;
				break;
			case 5: // packed
				pack = (it_r << 16) | (it_g << 8) | it_b;
				ct_x = Element_E189::AddCharacter(sim, ct_x, ct_y, chr_1 & 0xFF, pack);
				ct_x = Element_E189::AddCharacter(sim, ct_x, ct_y, (chr_1 >> 8) & 0xFF, pack);
				ct_x = Element_E189::AddCharacter(sim, ct_x, ct_y, (chr_1 >> 16) & 0xFF, pack);
				ct_x = Element_E189::AddCharacter(sim, ct_x, ct_y, (chr_1 >> 24) & 0xFF, pack);
				break;
			}
		}
	}
	__break_loop_1:
	std::cout << "output complete!" << std::endl;
}

Element_E189::~Element_E189() {}
