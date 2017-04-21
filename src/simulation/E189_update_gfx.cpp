#include "simulation/Elements.h"
#include "simulation/E189_update.h"

pixel tempPartColor;

#define NUM_SPC 36

pixel special_colors [NUM_SPC] = {
	PIXPACK(0x999999), PIXPACK(0x9C9C9C), PIXPACK(0xFA9999), PIXPACK(0x99CC70), PIXPACK(0x702088),
	PIXPACK(0x9040A8), PIXPACK(0xFF9999), PIXPACK(0x525252), PIXPACK(0x065206), PIXPACK(0xD2FFCF), //  10
	PIXPACK(0xBC00BC), PIXPACK(0x9040A8), PIXPACK(0xBFFF05), PIXPACK(0xFF33FF), PIXPACK(0xFF22FF),
	PIXPACK(0x9F0500), PIXPACK(0xAA0510), PIXPACK(0x9B4000), PIXPACK(0xEC00EC), PIXPACK(0xFF4422), //  20
	PIXPACK(0xFFCC11), PIXPACK(0x777070), PIXPACK(0x8C868C), PIXPACK(0xAA8048), PIXPACK(0xF0F078),
	PIXPACK(0xF0A820), PIXPACK(0xF0E8E0), PIXPACK(0x2033CC), PIXPACK(0xFFDD80), PIXPACK(0xD4E708), //  30
	PIXPACK(0x7099CC), PIXPACK(0x9970D0), PIXPACK(0xFF00FF), PIXPACK(0x40A060), PIXPACK(0xFF00FF),
	PIXPACK(0xF9F906)
};

int E189_Update::graphics(GRAPHICS_FUNC_ARGS)
{
	static char excitedtable [16] = {  0, 8, 2,10,12, 4,14, 6, 3,11, 1, 9,15, 7,13, 5 };
	int ptmp, ppos, pexc1, temp;
	int clife = cpart->life;
	if (clife >= 0 && clife < NUM_SPC) // pre-decoration
	{
		tempPartColor = special_colors[clife];
		*colr = PIXR(tempPartColor); *colg = PIXG(tempPartColor); *colb = PIXB(tempPartColor); 
	}
	switch(clife)
	{
	case 0:
	case 1:
		if (nx & ny & 1) 
		{
			*colr = 0x66; *colg = 0x66; *colb = 0x66;
		}
		break;
	case 6:
		if ((nx ^ ny) & 1) 
		{
			*colr = 0xDC; *colg = 0x66; *colb = 0x66;
		}
		break;
	case 8:
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
		// *colg = 0xFF;
		*firea = 90; *firer = *colr; *fireg = *colg; *fireb = *colb;
		*pixel_mode = PMODE_NONE;
		*pixel_mode |= FIRE_BLEND;
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
	case 16:
		if (cpart->ctype == 3 && cpart->tmp)
		{
			*colr = 0xFF; *colg = 0x5A; *colb = 0x65;
			return 0;
		}
		temp = !cpart->ctype && (cpart->tmp & 0x100);
		if (cpart->tmp2)
		{
			*colr = 0x65; *colg = 0xFF; *colb = 0x5A;
			if (temp)
			{
				ptmp  = *colg;
				*colg = *colb;
				*colb =  ptmp;
			}
			return 0;
		}
		if (temp)
		{
			*colg = *colr - *colg;
		}
		break;
	case 17:
		if ( !((nx ^ (ny << 1)) & 3) )
			{ *colr = 0xAA; *colg = 0x55; *colb = 0x10; }
		break;
	case 18:
		*colr = cpart->ctype;
		*colg = cpart->tmp;
		*colb = cpart->tmp2;
		break;
	case 22:
		if (!(((nx + ny) & 3) && ((nx - ny) & 3)))
			{ *colr = *colg = *colb = 0xEE; }
		break;
	case 23:
		if ((nx ^ ny) & 2)
			{ *colr = 0xEE; *colg = 0xB2; *colb = 0x66; }
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
	case 35:
		ptmp = cpart->ctype & 0xFF;
		ppos = cpart->ctype >> 8;
		if (ptmp > 0 && ptmp < PT_NUM && ren->sim->elements[ptmp].Enabled)
		{
			tempPartColor = ren->sim->elements[ptmp].Colour;
			
			// some special cases:
			switch (ptmp)
			{
			case PT_LIFE:
				if (ppos >= 0 && ppos < NGOL)
					tempPartColor = Element_LIFE::Element_GOL_colour[ppos];
				break;
			case PT_E189:
				if (ppos >= 0 && ppos < NUM_SPC)
					tempPartColor = special_colors[ppos];
				break;
			}
			
			*colr = PIXR(tempPartColor);
			*colg = PIXG(tempPartColor);
			*colb = PIXB(tempPartColor);
		}
		break;
	}
	return 0;
}