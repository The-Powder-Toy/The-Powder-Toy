#include "simulation/Elements.h"
#include "simulation/E189_update.h"

pixel tempPartColor;

int E189_Update::graphics(GRAPHICS_FUNC_ARGS)
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
	case 34:
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
	case 35:
		ptmp = cpart->ctype & 0xFF;
		ppos = cpart->ctype >> 8;
		if (ptmp > 0 && ptmp < PT_NUM && ren->sim->elements[ptmp].Enabled)
		{
			if (ppos >= 0 && ppos < NGOL)
			{
				tempPartColor = Element_LIFE::Element_GOL_colour[ppos];
				*colr = PIXR(tempPartColor);
				*colg = PIXG(tempPartColor);
				*colb = PIXB(tempPartColor);
			}
			else
			{
				tempPartColor = ren->sim->elements[ptmp].Colour;
				*colr = PIXR(tempPartColor);
				*colg = PIXG(tempPartColor);
				*colb = PIXB(tempPartColor);
			}
		}
		else
		{
			*colr = 0xFF; *colg = 0xFF; *colb = 0x00;
		}
		break;
	}
	return 0;
}