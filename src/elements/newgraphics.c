#include <element.h>
#include "hmap.h"

int graphics_QRTZ(GRAPHICS_FUNC_ARGS) //QRTZ and PQRT
{
	int t = cpart->type, z = cpart->tmp - 5;//speckles!
	if (cpart->temp>(ptransitions[t].thv-800.0f))//hotglow for quartz
	{
		float frequency = 3.1415/(2*ptransitions[t].thv-(ptransitions[t].thv-800.0f));
		int q = (cpart->temp>ptransitions[t].thv)?ptransitions[t].thv-(ptransitions[t].thv-800.0f):cpart->temp-(ptransitions[t].thv-800.0f);
		*colr += sin(frequency*q) * 226 + (z * 16);
		*colg += sin(frequency*q*4.55 +3.14) * 34 + (z * 16);
		*colb += sin(frequency*q*2.22 +3.14) * 64 + (z * 16);
	}
	else
	{
		*colr += z * 16;
		*colg += z * 16;
		*colb += z * 16;
	}
	return 0;
}
int graphics_CLST(GRAPHICS_FUNC_ARGS)
{
	int z = cpart->tmp - 5;//speckles!
	*colr += z * 16;
	*colg += z * 16;
	*colb += z * 16;
	return 0;
}
int graphics_CBNW(GRAPHICS_FUNC_ARGS)
{
	int z = cpart->tmp2 - 20;//speckles!
	*colr += z * 1;
	*colg += z * 2;
	*colb += z * 8;
	return 0;
}
int graphics_SPNG(GRAPHICS_FUNC_ARGS)
{
	*colr -= cpart->life*15;
	*colg -= cpart->life*15;
	*colb -= cpart->life*15;
	if (*colr<=50)
		*colr = 50;
	if (*colg<=50)
		*colg = 50;
	if (*colb<=20)
		*colb = 20;
	return 0;
}
int graphics_LIFE(GRAPHICS_FUNC_ARGS)
{
	pixel pc;
	if (cpart->ctype==NGT_LOTE)//colors for life states
	{
		if (cpart->tmp==2)
			pc = PIXRGB(255, 128, 0);
		else if (cpart->tmp==1)
			pc = PIXRGB(255, 255, 0);
		else
			pc = PIXRGB(255, 0, 0);
	}
	else if (cpart->ctype==NGT_FRG2)//colors for life states
	{
		if (cpart->tmp==2)
			pc = PIXRGB(0, 100, 50);
		else
			pc = PIXRGB(0, 255, 90);
	}
	else if (cpart->ctype==NGT_STAR)//colors for life states
	{
		if (cpart->tmp==4)
			pc = PIXRGB(0, 0, 128);
		else if (cpart->tmp==3)
			pc = PIXRGB(0, 0, 150);
		else if (cpart->tmp==2)
			pc = PIXRGB(0, 0, 190);
		else if (cpart->tmp==1)
			pc = PIXRGB(0, 0, 230);
		else
			pc = PIXRGB(0, 0, 70);
	}
	else if (cpart->ctype==NGT_FROG)//colors for life states
	{
		if (cpart->tmp==2)
			pc = PIXRGB(0, 100, 0);
		else
			pc = PIXRGB(0, 255, 0);
	}
	else if (cpart->ctype==NGT_BRAN)//colors for life states
	{
		if (cpart->tmp==1)
			pc = PIXRGB(150, 150, 0);
		else
			pc = PIXRGB(255, 255, 0);
	} else {
		pc = gmenu[cpart->ctype].colour;
	}
	*colr = PIXR(pc);
	*colg = PIXG(pc);
	*colb = PIXB(pc);
	return 0;
}
int graphics_DUST(GRAPHICS_FUNC_ARGS)
{
	if(cpart->life >= 1)
	{
		*firea = 120;
		*firer = *colr = cpart->flags;
		*fireg = *colg = cpart->tmp;
		*fireb = *colb = cpart->ctype;
		if (decorations_enable && cpart->dcolour)
		{
			int a = (cpart->dcolour>>24)&0xFF;
			*firer = *colr = (a*((cpart->dcolour>>16)&0xFF) + (255-a)**colr) >> 8;
			*fireg = *colg = (a*((cpart->dcolour>>8)&0xFF) + (255-a)**colg) >> 8;
			*fireb = *colb = (a*((cpart->dcolour)&0xFF) + (255-a)**colb) >> 8;
		}
		*pixel_mode |= PMODE_GLOW | FIRE_ADD;
		/**firea = 255;
		*firer = *colr;
		*fireg = *colg;
		*fireb = *colb;*/
	}
	return 0;
}
int graphics_GRAV(GRAPHICS_FUNC_ARGS)
{
	*colr = 20;
	*colg = 20;
	*colb = 20;
	if (cpart->vx>0)
	{
		*colr += (cpart->vx)*GRAV_R;
		*colg += (cpart->vx)*GRAV_G;
		*colb += (cpart->vx)*GRAV_B;
	}
	if (cpart->vy>0)
	{
		*colr += (cpart->vy)*GRAV_G;
		*colg += (cpart->vy)*GRAV_B;
		*colb += (cpart->vy)*GRAV_R;

	}
	if (cpart->vx<0)
	{
		*colr -= (cpart->vx)*GRAV_B;
		*colg -= (cpart->vx)*GRAV_R;
		*colb -= (cpart->vx)*GRAV_G;

	}
	if (cpart->vy<0)
	{
		*colr -= (cpart->vy)*GRAV_R2;
		*colg -= (cpart->vy)*GRAV_G2;
		*colb -= (cpart->vy)*GRAV_B2;
	}
	return 0;
}
int graphics_WIFI(GRAPHICS_FUNC_ARGS)
{
	float frequency = 0.0628;
	int q = cpart->tmp;
	*colr = sin(frequency*q + 0) * 127 + 128;
	*colg = sin(frequency*q + 2) * 127 + 128;
	*colb = sin(frequency*q + 4) * 127 + 128;
	*pixel_mode |= EFFECT_LINES;
	return 0;
}
int graphics_PRTI(GRAPHICS_FUNC_ARGS)
{
	*firea = 8;
	*firer = 255;
	*fireg = 0;
	*fireb = 0;
	*pixel_mode |= EFFECT_GRAVIN;
	*pixel_mode |= EFFECT_LINES;
	*pixel_mode &= ~PMODE;
	*pixel_mode |= PMODE_ADD;
	return 1;
}
int graphics_PRTO(GRAPHICS_FUNC_ARGS)
{
	*firea = 8;
	*firer = 0;
	*fireg = 0;
	*fireb = 255;
	*pixel_mode |= EFFECT_GRAVOUT;
	*pixel_mode |= EFFECT_LINES;
	*pixel_mode &= ~PMODE;
	*pixel_mode |= PMODE_ADD;
	return 1;
}
int graphics_BIZR(GRAPHICS_FUNC_ARGS) //BIZR, BIZRG, BIZRS
{
	int x = 0;
	*colg = 0;
	*colb = 0;
	*colr = 0;
	for (x=0; x<12; x++) {
		*colr += (cpart->ctype >> (x+18)) & 1;
		*colb += (cpart->ctype >>  x)     & 1;
	}
	for (x=0; x<12; x++)
		*colg += (cpart->ctype >> (x+9))  & 1;
	x = 624/(*colr+*colg+*colb+1);
	*colr *= x;
	*colg *= x;
	*colb *= x;
	if(fabs(cpart->vx)+fabs(cpart->vy)>0)
	{
		*firea = 255;
		*fireg = *colg/5 * fabs(cpart->vx)+fabs(cpart->vy);
		*fireb = *colb/5 * fabs(cpart->vx)+fabs(cpart->vy);
		*firer = *colr/5 * fabs(cpart->vx)+fabs(cpart->vy);
		*pixel_mode |= FIRE_ADD;
	}
	return 0;
}
int graphics_INVS(GRAPHICS_FUNC_ARGS)
{
	if(pv[ny/CELL][nx/CELL]>4.0f || pv[ny/CELL][nx/CELL]<-4.0f)
	{
		*cola = 100;
		*colr = 15;
		*colg = 0;
		*colb = 150;
		*pixel_mode &= PMODE;
		*pixel_mode |= PMODE_BLEND;
	} 
	return 0;
}
int graphics_ACID(GRAPHICS_FUNC_ARGS)
{
	int s = cpart->life;
	if (s>75) s = 75; //These two should not be here.
	if (s<49) s = 49;
	s = (s-49)*3;
	if (s==0) s = 1;
	*colr += s*4;
	*colg += s*1;
	*colb += s*2;
	*pixel_mode |= PMODE_BLUR;
	return 0;
}
int graphics_FILT(GRAPHICS_FUNC_ARGS)
{
	int x, temp_bin = (int)((cpart->temp-273.0f)*0.025f);
	if (temp_bin < 0) temp_bin = 0;
	if (temp_bin > 25) temp_bin = 25;
	cpart->ctype = 0x1F << temp_bin;
	*colg = 0;
	*colb = 0;
	*colr = 0;
	for (x=0; x<12; x++) {
		*colr += (cpart->ctype >> (x+18)) & 1;
		*colb += (cpart->ctype >>  x)     & 1;
	}
	for (x=0; x<12; x++)
		*colg += (cpart->ctype >> (x+9))  & 1;
	x = 624/(*colr+*colg+*colb+1);
	*cola = 127;
	*colr *= x;
	*colg *= x;
	*colb *= x;
	*pixel_mode &= ~PMODE;
	*pixel_mode |= PMODE_BLEND;
	return 0;
}
int graphics_BRAY(GRAPHICS_FUNC_ARGS)
{
	int x, trans = 255;
	if(cpart->tmp==0)
	{
		trans = cpart->life * 7;
		if (trans>255) trans = 255;
		if (cpart->ctype) {
			*colg = 0;
			*colb = 0;
			*colr = 0;
			for (x=0; x<12; x++) {
				*colr += (cpart->ctype >> (x+18)) & 1;
				*colb += (cpart->ctype >>  x)     & 1;
			}
			for (x=0; x<12; x++)
				*colg += (cpart->ctype >> (x+9))  & 1;
			x = 624/(*colr+*colg+*colb+1);
			*colr *= x;
			*colg *= x;
			*colb *= x;
		}
	}
	else if(cpart->tmp==1)
	{
		trans = cpart->life/4;
		if (trans>255) trans = 255;
		if (cpart->ctype) {
			*colg = 0;
			*colb = 0;
			*colr = 0;
			for (x=0; x<12; x++) {
				*colr += (cpart->ctype >> (x+18)) & 1;
				*colb += (cpart->ctype >>  x)     & 1;
			}
			for (x=0; x<12; x++)
				*colg += (cpart->ctype >> (x+9))  & 1;
			x = 624/(*colr+*colg+*colb+1);
			*colr *= x;
			*colg *= x;
			*colb *= x;
		}
	}
	else if(cpart->tmp==2)
	{
		trans = cpart->life*100;
		if (trans>255) trans = 255;
		*colr = 255;
		*colg = 150;
		*colb = 50;
	}
	*cola = trans;
	*pixel_mode &= ~PMODE;
	*pixel_mode |= PMODE_BLEND | PMODE_GLOW;
	return 0;
}
int graphics_SWCH(GRAPHICS_FUNC_ARGS)
{
	if(cpart->life >= 10)
	{
		*colr = 17;
		*colg = 217;
		*colb = 24;
		*pixel_mode |= PMODE_GLOW;
	}
	return 0;
}
int graphics_THDR(GRAPHICS_FUNC_ARGS)
{
	*firea = 160;
	*fireg = 192;
	*fireb = 255;
	*firer = 144;
	*pixel_mode |= FIRE_ADD;
	return 1;
}
int graphics_GLOW(GRAPHICS_FUNC_ARGS)
{
	*firer = restrict_flt(cpart->temp-(275.13f+32.0f), 0, 128)/50.0f;
	*fireg = restrict_flt(cpart->ctype, 0, 128)/50.0f;
	*fireb = restrict_flt(cpart->tmp, 0, 128)/50.0f;

	*colr = restrict_flt(64.0f+cpart->temp-(275.13f+32.0f), 0, 255);
	*colg = restrict_flt(64.0f+cpart->ctype, 0, 255);
	*colb = restrict_flt(64.0f+cpart->tmp, 0, 255);
	
	*pixel_mode |= FIRE_ADD;
	return 0;
}
int graphics_LCRY(GRAPHICS_FUNC_ARGS)
{
	if(decorations_enable && cpart->dcolour && cpart->dcolour&0xFF000000)
	{
		*colr = (cpart->dcolour>>16)&0xFF;
		*colg = (cpart->dcolour>>8)&0xFF;
		*colb = (cpart->dcolour)&0xFF;

		if(cpart->tmp2<10){
			*colr /= 10-cpart->tmp2;
			*colg /= 10-cpart->tmp2;
			*colb /= 10-cpart->tmp2;
		}
		
	}
	else
	{
		*colr = *colg = *colb = 0x50+((cpart->tmp2>10?10:cpart->tmp2)*10);
	}
	*pixel_mode |= NO_DECO;
	return 0;
					
	/*int lifemod = ((cpart->tmp2>10?10:cpart->tmp2)*10);
	*colr += lifemod; 
	*colg += lifemod; 
	*colb += lifemod; 
	if(decorations_enable && cpart->dcolour && cpart->dcolour&0xFF000000)
	{
		lifemod *= 2.5f;
		if(lifemod < 40)
			lifemod = 40;
		*colr = (lifemod*((cpart->dcolour>>16)&0xFF) + (255-lifemod)**colr) >> 8;
		*colg = (lifemod*((cpart->dcolour>>8)&0xFF) + (255-lifemod)**colg) >> 8;
		*colb = (lifemod*((cpart->dcolour)&0xFF) + (255-lifemod)**colb) >> 8;
	}
	*pixel_mode |= NO_DECO;
	return 0;*/
}
int graphics_PCLN(GRAPHICS_FUNC_ARGS)
{
	int lifemod = ((cpart->life>10?10:cpart->life)*10);
	*colr += lifemod;
	*colg += lifemod;
	return 0;
}
int graphics_PBCN(GRAPHICS_FUNC_ARGS)
{
	int lifemod = ((cpart->life>10?10:cpart->life)*10);
	*colr += lifemod;
	*colg += lifemod/2;
	return 0;
}
int graphics_DLAY(GRAPHICS_FUNC_ARGS)
{
	int stage = (int)(((float)cpart->life/(cpart->temp-273.15))*100.0f);
	*colr += stage;
	*colg += stage;
	*colb += stage;
	return 0;
}
int graphics_HSWC(GRAPHICS_FUNC_ARGS)
{
	int lifemod = ((cpart->life>10?10:cpart->life)*19);
	*colr += lifemod;
	return 0;
}
int graphics_PVOD(GRAPHICS_FUNC_ARGS)
{
	int lifemod = ((cpart->life>10?10:cpart->life)*16);
	*colr += lifemod;
	return 0;
}
int graphics_STOR(GRAPHICS_FUNC_ARGS)
{
	if(cpart->tmp){
		*pixel_mode |= PMODE_GLOW;
		*colr = 0x50;
		*colg = 0xDF;
		*colb = 0xDF;
	} else {
		*colr = 0x20;
		*colg = 0xAF;
		*colb = 0xAF;
	}
	return 0;
}
int graphics_PUMP(GRAPHICS_FUNC_ARGS)
{
	int lifemod = ((cpart->life>10?10:cpart->life)*19);
	*colb += lifemod;
	return 0;
}
int graphics_GPMP(GRAPHICS_FUNC_ARGS)
{
	int lifemod = ((cpart->life>10?10:cpart->life)*19);
	*colg += lifemod;
	*colb += lifemod;
	return 0;
}
int graphics_HFLM(GRAPHICS_FUNC_ARGS)
{
	int caddress = restrict_flt(restrict_flt((float)((int)(cpart->life/2)), 0.0f, 200.0f)*3, 0.0f, (200.0f*3)-3);
	*colr = (unsigned char)hflm_data[caddress];
	*colg = (unsigned char)hflm_data[caddress+1];
	*colb = (unsigned char)hflm_data[caddress+2];
	
	*firea = 255;
	*firer = *colr;
	*fireg = *colg;
	*fireb = *colb;
	
	*pixel_mode = PMODE_NONE; //Clear default, don't draw pixel
	*pixel_mode |= FIRE_ADD;
	//Returning 0 means dynamic, do not cache
	return 0;
}
int graphics_FIRW(GRAPHICS_FUNC_ARGS)
{
	if(cpart->tmp>=3)
	{
		int caddress = restrict_flt(restrict_flt((float)(cpart->tmp-4), 0.0f, 200.0f)*3, 0.0f, (200.0f*3)-3);
		*colr = (unsigned char)firw_data[caddress];
		*colg = (unsigned char)firw_data[caddress+1];
		*colb = (unsigned char)firw_data[caddress+2];
		
		if (decorations_enable && cpart->dcolour)
		{
			int a = (cpart->dcolour>>24)&0xFF;
			*colr = (a*((cpart->dcolour>>16)&0xFF) + (255-a)**colr) >> 8;
			*colg = (a*((cpart->dcolour>>8)&0xFF) + (255-a)**colg) >> 8;
			*colb = (a*((cpart->dcolour)&0xFF) + (255-a)**colb) >> 8;
		}
		
		*firea = cpart->life*4;
		if(*firea > 240)
			*firea = 240;
		*firer = *colr;
		*fireg = *colg;
		*fireb = *colb;
		
		*pixel_mode = PMODE_NONE; //Clear default, don't draw pixel
		*pixel_mode |= FIRE_ADD;
		//Returning 0 means dynamic, do not cache
	}
	else if(cpart->tmp > 0)
	{
		*pixel_mode |= PMODE_GLOW;
	}
	return 0;
}
int graphics_GBMB(GRAPHICS_FUNC_ARGS)
{
	if (cpart->life <= 0) {
		*pixel_mode |= PMODE_FLARE;
	}
	else
	{
		*pixel_mode |= PMODE_SPARK;
	}
	return 0;
}
int graphics_COAL(GRAPHICS_FUNC_ARGS) //Both COAL and Broken Coal
{
	*colr += (cpart->tmp2-295.15f)/3;
	
	if (*colr > 170)
		*colr = 170;
	if (*colr < *colg)
		*colr = *colg;
		
	*colg = *colb = *colr;

	if((cpart->temp-295.15f) > 300.0f-200.0f)
	{
		float frequency = 3.1415/(2*300.0f-(300.0f-200.0f));
		int q = ((cpart->temp-295.15f)>300.0f)?300.0f-(300.0f-200.0f):(cpart->temp-295.15f)-(300.0f-200.0f);

		*colr += sin(frequency*q) * 226;
		*colg += sin(frequency*q*4.55 +3.14) * 34;
		*colb += sin(frequency*q*2.22 +3.14) * 64;
	}
	return 0;
}

