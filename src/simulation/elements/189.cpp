#include "simulation/Elements.h"
#include "simulation/E189_update.h"
#include "Probability.h"

//#TPT-Directive ElementClass Element_E189 PT_E189 189
Element_E189::Element_E189()
{
	Identifier = "DEFAULT_PT_E189";
	Name = "E189";
	Colour = PIXPACK(0xFFB060);
	MenuVisible = 1;
	MenuSection = SC_SPECIAL;
#if defined(DEBUG) || defined(SNAPSHOT)
	Enabled = 1;
#else
	Enabled = 0;
#endif

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

	Update = &E189_Update::update;
	Graphics = &Element_E189::graphics;
	IconGenerator = &Element_E189::iconGen;
	// Notice: Exotic solid!
	// Properties without PROP_LIFE_DEC and PROP_LIFE_KILL_DEC, has reason.
}

//#TPT-Directive ElementHeader Element_E189 static bool useDefaultPart
bool Element_E189::useDefaultPart = false;

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
	case 16:
		if (cpart->ctype == 3 && cpart->tmp)
		{
			*colr = 0xFF; *colg = 0x5A; *colb = 0x65;
			return 0;
		}
		if (cpart->tmp2)
		{
			*colr = 0x65; *colg = 0xFF; *colb = 0x5A;
			return 0;
		}
		*colr = 0xAA; *colg = 0x05; *colb = 0x10;
		break;
	case 17:
		if ((nx ^ (ny << 1)) & 3)
			{ *colr = 0x9B; *colg = 0x40; *colb = 0x00; }
		else
			{ *colr = 0xAA; *colg = 0x55; *colb = 0x10; }
		break;
	case 18:
		*colr = cpart->ctype;
		*colg = cpart->tmp;
		*colb = cpart->tmp2;
		break;
	case 19:
		*colr = 0xFF; *colg = 0x44; *colb = 0x22;
		break;
	case 20:
		*colr = 0xFF; *colg = 0xCC; *colb = 0x11;
		break;
	case 21:
		*colr = 0x77; *colg = 0x70; *colb = 0x70;
		break;
	case 22:
		if (!(((nx + ny) & 3) && ((nx - ny) & 3)))
			{ *colr = *colg = *colb = 0xEE; }
		else
			{ *colr = *colg = *colb = 0x8C; }
		break;
	case 23:
		if ((nx ^ ny) & 2)
			{ *colr = 0xEE; *colg = 0xB2; *colb = 0x66; }
		else
			{ *colr = 0xAA; *colg = 0x80; *colb = 0x48; }
		break;
	case 24:
		*colr = 0xF0; *colg = 0xF0; *colb = 0x78;
		break;
	case 25:
		*colr = 0xF0; *colg = 0xA8; *colb = 0x20;
		break;
	case 26:
		ptmp = cpart->tmp;
		if (ptmp < 0) ptmp = 0;
		if (ptmp < 7)
		{
			*colr = 0x78 + ((ptmp * 0x89) >> 3);
			*colg = 0x74 + ((ptmp * 0x10B) >> 4);
			*colb = 0x70 + (ptmp << 4);
		}
		else
			{ *colr = 0xF0; *colg = 0xE8; *colb = 0xE0; }
		break;
	case 27:
		*colr = 0x20; *colg = 0x33; *colb = 0xCC;
		break;
	case 28:
		*colr = 0xFF; *colg = 0xDD; *colb = 0x80;
		break;
	case 29:
		*colr = 0xD4; *colg = 0xE7; *colb = 0x08;
		break;
	case 30:
		*colr = 0x70; *colg = 0x99; *colb = 0xCC;
		break;
	case 31:
		*colr = 0x99; *colg = 0x70; *colb = 0xD0;
		break;
	case 32:
		*colr = 0xFF; *colg = 0x00; *colb = 0xFF;
		break;
	case 33:
		{
		static float FREQUENCY = 0.0628f;
		int q = (int)((cpart->temp-73.15f)/100+1);
		*colr = sin(FREQUENCY*q + 0) * 127 + 128;
		*colg = sin(FREQUENCY*q + 2) * 127 + 128;
		*colb = sin(FREQUENCY*q + 4) * 127 + 128;
		}
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
	int ctype, r1, r2, r3;
	float rvx, rvy, rvx2, rvy2, rdif, multipler = 1.0f;
	long long int lsb;
	if (!((rtmp >> 22) & 1))
	{
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
				if (sim->elementCount[PT_PINS] > 0)
					r1 |= 0x4000;
				if (sim->elementCount[PT_PINVIS] > 0)
					r1 |= 0x8000;
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
			if (!(rct & 2))
			{
				part_phot->vx = rdif*cosf(rvx2);
				part_phot->vy = rdif*sinf(rvx2);
			}
			break;
		}
	}
	else
	{
		switch (rtmp & 0x1F)
		{
			case 0: // no photons operation
				break;
			case 1: // 50% turn left
				if (rand() & 1)
				{
					rdif = part_phot->vx;
					part_phot->vx = part_phot->vy;
					part_phot->vy = -rdif;
				}
				break;
			case 2: // 50% turn right
				if (rand() & 1)
				{
					rdif = part_phot->vx;
					part_phot->vx = -part_phot->vy;
					part_phot->vy = rdif;
				}
				break;
			case 3: // 50% turn left, 50% turn right
				rvx = part_phot->vx;
				rvy = (rand() & 1) ? 1.0 : -1.0;
				part_phot->vx =  rvy * part_phot->vy;
				part_phot->vy = -rvy * rdif;
				break;
			case 4: // turn left + go straight + turn right = 100%
				r1 = rand() % 3;
				if (r1)
				{
					rvx = part_phot->vx;
					rvy = (r1 & 1) ? 1.0 : -1.0;
					part_phot->vx =  rvy * part_phot->vy;
					part_phot->vy = -rvy * rdif;
				}
				break;
			case 5: // random "energy" particle
				part_phot->ctype = 0x101;
				sim->part_change_type(i, x, y, PT_E186);
				break;
			case 6: // photons absorber
				sim->kill_part(i);
				break;
			case 7: // PHOT->NEUT
				part_phot->ctype = 0x102;
				sim->part_change_type(i, x, y, PT_E186);
				break;
			case 8: // PHOT->ELEC
				part_phot->ctype = 0x103;
				sim->part_change_type(i, x, y, PT_E186);
				break;
			case 9: // PHOT->PROT
				part_phot->ctype = 0x104;
				sim->part_change_type(i, x, y, PT_E186);
				break;
			case 10: // PHOT->GRVT
				part_phot->ctype = 0x105;
				sim->part_change_type(i, x, y, PT_E186);
				break;
			case 11: // PHOT (tmp: 0 -> 1)
				part_phot->tmp |= 0x1;
				break;
			case 12: // PHOT (tmp: 1 -> 0)
				part_phot->tmp &= ~0x1;
				break;
			case 13: // set PHOT life
				part_phot->life = part_E189->ctype;
				break;
			case 14: // PHOT life extender (positive)
				if (part_phot->life > 0)
				{
					part_phot->life += part_E189->ctype;
					if (part_phot->life < 0)
						part_phot->life = 0;
				}
				break;
			case 15: // PHOT life extender (negative)
				if (part_phot->life > 0)
				{
					part_phot->life -= part_E189->ctype;
					if (part_phot->life < 0)
						sim->kill_part(i);
				}
				break;
			case 16: // velocity to wavelength converter
				rvx = part_phot->vx;
				rvy = part_phot->vy;
				rdif = rvx * rvx + rvy * rvy;
				ctype = part_phot->ctype;
				// int's length is 32-bit
				// float's length is 32-bit
				r1 = * (int*) &rdif; // r1 = "bit level hack"
				r2 = ((r1 - 0x40cba592) >> 23); // 0x40cba592: for (9 / sqrt(2))
				if (r2 > 0)
				{
					// blue shift and decelerate
#ifdef __GNUC__
					r1 = (31 - __builtin_clz (ctype)) / 3;
					r3 = (r1 < r2 ? r1 : r2);
					ctype >>= 3 * r3;
#else
					r3 = 0;
					while (r2 && (ctype & 0xFFFFFFF8))
					{
						ctype >>= 3;
						r2--; r3++;
					}
#endif
					multipler = powf(0.5f, r1 * 0.5f);
					part_phot->vx = rvx * multipler;
					part_phot->vy = rvy * multipler;
				}
				else if (r2 < 0)
				{
					// red shift and accelerate
#ifdef __GNUC__
					r1 = (29 - __builtin_ctz (ctype)) / 3;
					r3 = (r1 < -r2 ? r1 : -r2);
					ctype <<= 3 * r3;
#else
					r3 = 0;
					while (r2 && (ctype & 0x07FFFFFF))
					{
						ctype <<= 3;
						r2++; r3++;
					}
#endif
					multipler = powf(2.0f, r1 * 0.5f);
					ctype &= mask;
					part_phot->vx = rvx * multipler;
					part_phot->vy = rvy * multipler;
				}
				part_phot->ctype = ctype;
				break;
		}
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
	
	ri = sim->create_part(-3, x, y, PT_PHOT);
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
		if (sim->elements[t].Properties & PROP_NODESTRUCT)
			continue; // particle's type is PT_DMND and PT_INDI are indestructible.
		rx = parts[r].x;
		ry = parts[r].y;

		switch ( t )
		{
		case PT_DMND:
		case PT_INDI:
			break;
		case PT_METL:
			if (Probability::randFloat() < prob_breakElectronics)
				sim->part_change_type(r, rx, ry, PT_BMTL);
			break;
		case PT_COAL:
			if (Probability::randFloat() < prob_breakElectronics)
				sim->part_change_type(r, rx, ry, PT_BCOL);
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
			if (Probability::randFloat() < prob_breakElectronics)
			{
				sim->create_part(r, rx, ry, PT_E189, 8);
				parts[r].tmp = 21000;
			}
			break;
		case PT_VOID:
		case PT_PVOD:
		case PT_CONV:
			if (Probability::randFloat() < prob_breakElectronics)
			{
				sim->create_part(r, rx, ry, PT_WARP);
				parts[r].tmp2 = 6000;
			}
			break;
		case PT_CRMC:
			if (Probability::randFloat() < prob_breakElectronics)
			{
				if (rand() & 1)
					sim->part_change_type(r, rx, ry, PT_CLST);
				else
					sim->part_change_type(r, rx, ry, PT_PQRT);
			}
			break;
		case PT_BRCK:
			if (Probability::randFloat() < prob_breakElectronics)
				sim->part_change_type(r, rx, ry, PT_STNE);
			break;
		case PT_DLAY:
			if (Probability::randFloat() < prob_breakElectronics)
				parts[r].temp = (rand()%512) + 274.15f; // Randomize delay
			break;
		case PT_EMP:
			if (triggerCount > rand() % 1000)
				sim->part_change_type(r, rx, ry, PT_BREC);
			break;
		case PT_PSCN: case PT_NSCN:
		case PT_PTCT: case PT_NTCT:
		case PT_SWCH: case PT_DTEC:
		case PT_PSNS: case PT_TSNS:
		case PT_FRME: case PT_PSTN:
			if (Probability::randFloat() < prob_breakElectronics)
				sim->part_change_type(r, rx, ry, PT_BREC);
			break;
		case PT_WIFI:
		case PT_PRTI: case PT_PRTO:
			if (Probability::randFloat() < prob_breakElectronics)
			{
				// Randomize channel
				parts[r].temp = rand()%MAX_TEMP;
			}
			else if (prob_breakTRONPortal)
			{
				sim->part_change_type(r, rx, ry, PT_WARP);
			}
			break;
		case PT_SPRK:
			if (!(sim->elements[parts[r].ctype].Properties & PROP_NODESTRUCT) &&
			    (Probability::randFloat() < prob_breakElectronics))
				sim->part_change_type(r, rx, ry, PT_BREC);
			break;
		case PT_CLNE:
			if (Probability::randFloat() < prob_breakElectronics)
				sim->part_change_type(r, rx, ry, PT_BCLN);
			break;
		case PT_PCLN:
			if (Probability::randFloat() < prob_breakElectronics)
				sim->part_change_type(r, rx, ry, PT_PBCN);
			break;
		case PT_BCLN:
			if (triggerCount > rand() % 1000)
				parts[r].life = rand()%40+80;
			break;
		case PT_PBCN:
			if (triggerCount > rand() % 1000)
				parts[r].tmp2 = rand()%40+80;
			break;
		case PT_SPNG:
		case PT_BTRY:
			if (Probability::randFloat() < prob_breakElectronics)
				sim->create_part(r, rx, ry, PT_PLSM);
			break;
		case PT_VIBR:
			if (Probability::randFloat() < prob_breakElectronics * 0.2)
				sim->part_change_type(r, rx, ry, PT_BVBR);
			break;
		case PT_BVBR:
			if (Probability::randFloat() < prob_breakElectronics * 0.1)
			{
				sim->part_change_type(r, rx, ry, PT_VIBR);
				parts[r].life = 1000;
			}
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
			case 16:
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
			case 33:
				if (Probability::randFloat() < prob_breakElectronics)
				{
					sim->part_change_type(r, rx, ry, PT_WIFI);
					parts[r].ctype = 0;
				}
			}
			break;
		}
	}
}

//#TPT-Directive ElementHeader Element_E189 static void FloodButton(Simulation *sim, int i, int x, int y)
void Element_E189::FloodButton(Simulation *sim, int i, int x, int y)
{
	int coord_stack_limit = XRES*YRES;
	unsigned short (*coord_stack)[2];
	int coord_stack_size = 0;
	int x1, x2, r;
	
	Particle * parts = sim->parts;
	int (*pmap)[XRES] = sim->pmap;
	
	coord_stack = new unsigned short[coord_stack_limit][2];
	coord_stack[coord_stack_size][0] = x;
	coord_stack[coord_stack_size][1] = y;
	coord_stack_size++;
	
	if ((parts[i].type != PT_E189) || (parts[i].life != 26) || parts[i].tmp)
	{
		delete[] coord_stack;
		return;
	}
	
	do
	{
		coord_stack_size--;
		x = coord_stack[coord_stack_size][0];
		y = coord_stack[coord_stack_size][1];
		x1 = x2 = x;
		
		// go left as far as possible
		while (x1 >= 0)
		{
			r = pmap[y][x1-1];
			if ((r&0xFF) != PT_E189 || parts[r>>8].life != 26)
			{
				break;
			}
			x1--;
		}
		// go right as far as possible
		while (x2 < XRES)
		{
			r = pmap[y][x2+1];
			if ((r&0xFF) != PT_E189 || parts[r>>8].life != 26)
			{
				break;
			}
			x2++;
		}
		
		// fill span
		for (x=x1; x<=x2; x++)
		{
			r = pmap[y][x]>>8;
			parts[r].tmp = 8;
			// parts[r].flags |= 0x80000000;
		}
		
		// add adjacent pixels to stack
		if (y >= 1)
			for (x=x1-1; x<=x2+1; x++)
			{
				r = pmap[y-1][x];
				if ((r&0xFF) == PT_E189 && parts[r>>8].life == 26 && !parts[r>>8].tmp)
				{
					coord_stack[coord_stack_size][0] = x;
					coord_stack[coord_stack_size][1] = y-1;
					coord_stack_size++;
					if (coord_stack_size>=coord_stack_limit)
					{
						delete[] coord_stack;
						return;
					}
				}
			}
		if (y < YRES-1)
			for (x=x1-1; x<=x2+1; x++)
			{
				r = pmap[y+1][x];
				if ((r&0xFF) == PT_E189 && parts[r>>8].life == 26 && !parts[r>>8].tmp)
				{
					coord_stack[coord_stack_size][0] = x;
					coord_stack[coord_stack_size][1] = y+1;
					coord_stack_size++;
					if (coord_stack_size>=coord_stack_limit)
					{
						delete[] coord_stack;
						return;
					}
				}
			}
		
	} while (coord_stack_size>0);
	delete[] coord_stack;
}

Element_E189::~Element_E189() {}
