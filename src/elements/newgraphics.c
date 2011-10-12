#include <element.h>
int graphics_QRTZ(GRAPHICS_FUNC_ARGS) //QRTZ and PQRT
{
	int t = parts[i].type, z = parts[i].tmp - 5;//speckles!
	if (parts[i].temp>(ptransitions[t].thv-800.0f))//hotglow for quartz
	{
		float frequency = 3.1415/(2*ptransitions[t].thv-(ptransitions[t].thv-800.0f));
		int q = (parts[i].temp>ptransitions[t].thv)?ptransitions[t].thv-(ptransitions[t].thv-800.0f):parts[i].temp-(ptransitions[t].thv-800.0f);
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
	int z = parts[i].tmp - 5;//speckles!
	*colr += z * 16;
	*colg += z * 16;
	*colb += z * 16;
	return 0;
}
int graphics_CBNW(GRAPHICS_FUNC_ARGS)
{
	int z = parts[i].tmp2 - 20;//speckles!
	*colr += z * 1;
	*colg += z * 2;
	*colb += z * 8;
	return 0;
}
int graphics_SPNG(GRAPHICS_FUNC_ARGS)
{
	*colr -= parts[i].life*15;
	*colg -= parts[i].life*15;
	*colb -= parts[i].life*15;
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
	if (parts[i].ctype==NGT_LOTE)//colors for life states
	{
		if (parts[i].tmp==2)
			pc = PIXRGB(255, 128, 0);
		else if (parts[i].tmp==1)
			pc = PIXRGB(255, 255, 0);
		else
			pc = PIXRGB(255, 0, 0);
	}
	else if (parts[i].ctype==NGT_FRG2)//colors for life states
	{
		if (parts[i].tmp==2)
			pc = PIXRGB(0, 100, 50);
		else
			pc = PIXRGB(0, 255, 90);
	}
	else if (parts[i].ctype==NGT_STAR)//colors for life states
	{
		if (parts[i].tmp==4)
			pc = PIXRGB(0, 0, 128);
		else if (parts[i].tmp==3)
			pc = PIXRGB(0, 0, 150);
		else if (parts[i].tmp==2)
			pc = PIXRGB(0, 0, 190);
		else if (parts[i].tmp==1)
			pc = PIXRGB(0, 0, 230);
		else
			pc = PIXRGB(0, 0, 70);
	}
	else if (parts[i].ctype==NGT_FROG)//colors for life states
	{
		if (parts[i].tmp==2)
			pc = PIXRGB(0, 100, 0);
		else
			pc = PIXRGB(0, 255, 0);
	}
	else if (parts[i].ctype==NGT_BRAN)//colors for life states
	{
		if (parts[i].tmp==1)
			pc = PIXRGB(150, 150, 0);
		else
			pc = PIXRGB(255, 255, 0);
	} else {
		pc = gmenu[parts[i].ctype].colour;
	}
	*colr = PIXR(pc);
	*colg = PIXG(pc);
	*colb = PIXB(pc);
	return 0;
}
int graphics_DUST(GRAPHICS_FUNC_ARGS)
{
	if(parts[i].life >= 1)
	{
		*colr = parts[i].flags;
		*colg = parts[i].tmp;
		*colb = parts[i].ctype;
		if (decorations_enable && parts[i].dcolour)
		{
			int a = (parts[i].dcolour>>24)&0xFF;
			*colr = (a*((parts[i].dcolour>>16)&0xFF) + (255-a)**colr) >> 8;
			*colg = (a*((parts[i].dcolour>>8)&0xFF) + (255-a)**colg) >> 8;
			*colb = (a*((parts[i].dcolour)&0xFF) + (255-a)**colb) >> 8;
		}
		*firea = 64;
		*firer = *colr;
		*fireg = *colg;
		*fireb = *colb;
	}
	return 0;
}
int graphics_GRAV(GRAPHICS_FUNC_ARGS)
{
	*colr = 20;
	*colg = 20;
	*colb = 20;
	if (parts[i].vx>0)
	{
		*colr += (parts[i].vx)*GRAV_R;
		*colg += (parts[i].vx)*GRAV_G;
		*colb += (parts[i].vx)*GRAV_B;
	}
	if (parts[i].vy>0)
	{
		*colr += (parts[i].vy)*GRAV_G;
		*colg += (parts[i].vy)*GRAV_B;
		*colb += (parts[i].vy)*GRAV_R;

	}
	if (parts[i].vx<0)
	{
		*colr -= (parts[i].vx)*GRAV_B;
		*colg -= (parts[i].vx)*GRAV_R;
		*colb -= (parts[i].vx)*GRAV_G;

	}
	if (parts[i].vy<0)
	{
		*colr -= (parts[i].vy)*GRAV_R2;
		*colg -= (parts[i].vy)*GRAV_G2;
		*colb -= (parts[i].vy)*GRAV_B2;
	}
	return 0;
}
int graphics_WIFI(GRAPHICS_FUNC_ARGS)
{
	float frequency = 0.0628;
	int q = parts[i].tmp;
	*colr = sin(frequency*q + 0) * 127 + 128;
	*colg = sin(frequency*q + 2) * 127 + 128;
	*colb = sin(frequency*q + 4) * 127 + 128;
	return 0;
}
int graphics_PRTI(GRAPHICS_FUNC_ARGS)
{
	*firea = 1;
	*firer = 255;
	*fireg = 0;
	*fireb = 0;
	*pixel_mode |= EFFECT_GRAVIN;
	*pixel_mode &= ~PMODE;
	*pixel_mode |= PMODE_ADD;
	return 1;
}
int graphics_PRTO(GRAPHICS_FUNC_ARGS)
{
	*firea = 1;
	*firer = 0;
	*fireg = 0;
	*fireb = 255;
	*pixel_mode |= EFFECT_GRAVOUT;
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
		*colr += (parts[i].ctype >> (x+18)) & 1;
		*colb += (parts[i].ctype >>  x)     & 1;
	}
	for (x=0; x<12; x++)
		*colg += (parts[i].ctype >> (x+9))  & 1;
	x = 624/(*colr+*colg+*colb+1);
	*colr *= x;
	*colg *= x;
	*colb *= x;
	if(fabs(parts[i].vx)+fabs(parts[i].vy)>0)
	{
		*firea = 255;
		*fireg = *colg/40 * fabs(parts[i].vx)+fabs(parts[i].vy);
		*fireb = *colb/40 * fabs(parts[i].vx)+fabs(parts[i].vy);
		*firer = *colr/40 * fabs(parts[i].vx)+fabs(parts[i].vy);
		*pixel_mode |= FIRE_ADD;
	}
	return 0;
}
int graphics_PIPE(GRAPHICS_FUNC_ARGS)
{
	if (parts[i].ctype==2)
	{
		*colr = 50;
		*colg = 1;
		*colb = 1;
	}
	else if (parts[i].ctype==3)
	{
		*colr = 1;
		*colg = 50;
		*colb = 1;
	}
	else if (parts[i].ctype==4)
	{
		*colr = 1;
		*colg = 1;
		*colb = 50;
	}
	else if (parts[i].temp<272.15&&parts[i].ctype!=1)
	{
		if (parts[i].temp>173.25&&parts[i].temp<273.15)
		{
			*colr = 50;
			*colg = 1;
			*colb = 1;
		}
		if (parts[i].temp>73.25&&parts[i].temp<=173.15)
		{
			*colr = 1;
			*colg = 50;
			*colb = 1;
		}
		if (parts[i].temp>=0&&parts[i].temp<=73.15)
		{
			*colr = 1;
			*colg = 1;
			*colb = 50;
		}
	}
	if ((parts[i].tmp&0xFF)>0 && (parts[i].tmp&0xFF)<PT_NUM)
	{
		//Maybe use a subcall to get the info of the embedded particle?
		*colr = PIXR(ptypes[parts[i].tmp&0xFF].pcolors);
		*colg = PIXG(ptypes[parts[i].tmp&0xFF].pcolors);
		*colb = PIXB(ptypes[parts[i].tmp&0xFF].pcolors);
	}
	return 0;
}
int graphics_INVS(GRAPHICS_FUNC_ARGS)
{
	if(pv[ny/CELL][nx/CELL]>4.0f || pv[ny/CELL][nx/CELL]<-4.0f)
	{
		*colr = 15;
		*colg = 0;
		*colb = 150;
	} 
	return 0;
}
int graphics_ACID(GRAPHICS_FUNC_ARGS)
{
	int s = parts[i].life;
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
	int x, temp_bin = (int)((parts[i].temp-273.0f)*0.025f);
	if (temp_bin < 0) temp_bin = 0;
	if (temp_bin > 25) temp_bin = 25;
	parts[i].ctype = 0x1F << temp_bin;
	*colg = 0;
	*colb = 0;
	*colr = 0;
	for (x=0; x<12; x++) {
		*colr += (parts[i].ctype >> (x+18)) & 1;
		*colb += (parts[i].ctype >>  x)     & 1;
	}
	for (x=0; x<12; x++)
		*colg += (parts[i].ctype >> (x+9))  & 1;
	x = 624/(*colr+*colg+*colb+1);
	*colr *= x;
	*colg *= x;
	*colb *= x;
	return 0;
}
int graphics_BRAY(GRAPHICS_FUNC_ARGS)
{
	int x, trans = 255;
	if(parts[i].tmp==0)
	{
		trans = parts[i].life * 7;
		if (trans>255) trans = 255;
		if (parts[i].ctype) {
			*colg = 0;
			*colb = 0;
			*colr = 0;
			for (x=0; x<12; x++) {
				*colr += (parts[i].ctype >> (x+18)) & 1;
				*colb += (parts[i].ctype >>  x)     & 1;
			}
			for (x=0; x<12; x++)
				*colg += (parts[i].ctype >> (x+9))  & 1;
			x = 624/(*colr+*colg+*colb+1);
			*colr *= x;
			*colg *= x;
			*colb *= x;
		}
	}
	else if(parts[i].tmp==1)
	{
		trans = parts[i].life/4;
		if (trans>255) trans = 255;
		if (parts[i].ctype) {
			*colg = 0;
			*colb = 0;
			*colr = 0;
			for (x=0; x<12; x++) {
				*colr += (parts[i].ctype >> (x+18)) & 1;
				*colb += (parts[i].ctype >>  x)     & 1;
			}
			for (x=0; x<12; x++)
				*colg += (parts[i].ctype >> (x+9))  & 1;
			x = 624/(*colr+*colg+*colb+1);
			*colr *= x;
			*colg *= x;
			*colb *= x;
		}
	}
	else if(parts[i].tmp==2)
	{
		trans = parts[i].life*100;
		if (trans>255) trans = 255;
		*colr = 255;
		*colr = 150;
		*colr = 50;
	}
	//*cola = trans;
	*pixel_mode &= ~PMODE;
	*pixel_mode |= PMODE_BLEND;
	return 0;
}
int graphics_SWCH(GRAPHICS_FUNC_ARGS)
{
	if(parts[i].life >= 10)
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
	*firea = 20;
	*fireg = 192;
	*fireb = 255;
	*firer = 144;
	*pixel_mode |= FIRE_ADD;
	return 1;
}
int graphics_GLOW(GRAPHICS_FUNC_ARGS)
{
	*firer = restrict_flt(parts[i].temp-(275.13f+32.0f), 0, 128)/50.0f;
	*fireg = restrict_flt(parts[i].ctype, 0, 128)/50.0f;
	*fireb = restrict_flt(parts[i].tmp, 0, 128)/50.0f;

	*colr = restrict_flt(64.0f+parts[i].temp-(275.13f+32.0f), 0, 255);
	*colg = restrict_flt(64.0f+parts[i].ctype, 0, 255);
	*colb = restrict_flt(64.0f+parts[i].tmp, 0, 255);
	
	*pixel_mode |= FIRE_ADD;
	return 0;
}
int graphics_LCRY(GRAPHICS_FUNC_ARGS)
{
	int lifemod = ((parts[i].life>10?10:parts[i].life)*10);
	if(parts[i].dcolour && parts[i].dcolour&0xFF000000)
	{
		*colr += (lifemod * (255-(parts[i].dcolour>>16)&0xFF))>>8; 
		*colg += (lifemod * (255-(parts[i].dcolour>>8)&0xFF))>>8; 
		*colb += (lifemod * (255-(parts[i].dcolour)&0xFF))>>8;  
	}
	else
	{
		*colr += lifemod; 
		*colg += lifemod; 
		*colb += lifemod; 
	}
	*pixel_mode |= NO_DECO;
	return 0;
}
int graphics_PCLN(GRAPHICS_FUNC_ARGS)
{
	int lifemod = ((parts[i].life>10?10:parts[i].life)*10);
	*colr += lifemod;
	*colg += lifemod;
	return 0;
}
int graphics_PBCN(GRAPHICS_FUNC_ARGS)
{
	int lifemod = ((parts[i].life>10?10:parts[i].life)*10);
	*colr += lifemod;
	*colg += lifemod/2;
	return 0;
}
int graphics_DLAY(GRAPHICS_FUNC_ARGS)
{
	int stage = (int)(((float)parts[i].life/(parts[i].temp-273.15))*100.0f);
	*colr += stage;
	*colg += stage;
	*colb += stage;
	return 0;
}
int graphics_HSWC(GRAPHICS_FUNC_ARGS)
{
	int lifemod = ((parts[i].life>10?10:parts[i].life)*19);
	*colr += lifemod;
	return 0;
}
int graphics_PVOD(GRAPHICS_FUNC_ARGS)
{
	int lifemod = ((parts[i].life>10?10:parts[i].life)*16);
	*colr += lifemod;
	return 0;
}
int graphics_STOR(GRAPHICS_FUNC_ARGS)
{
	if(parts[i].tmp){
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
	int lifemod = ((parts[i].life>10?10:parts[i].life)*19);
	*colb += lifemod;
	return 0;
}
int graphics_GPMP(GRAPHICS_FUNC_ARGS)
{
	int lifemod = ((parts[i].life>10?10:parts[i].life)*19);
	*colg += lifemod;
	*colb += lifemod;
	return 0;
}
int graphics_HFLM(GRAPHICS_FUNC_ARGS)
{
	int caddress = restrict_flt(restrict_flt((float)((int)(parts[i].life/2)), 0.0f, 200.0f)*3, 0.0f, (200.0f*3)-3);
	*colr = (unsigned char)hflm_data[caddress];
	*colg = (unsigned char)hflm_data[caddress+1];
	*colb = (unsigned char)hflm_data[caddress+2];
	
	*firea = 255;
	*firer = *colr/8;
	*fireg = *colg/8;
	*fireb = *colb/8;
	
	*pixel_mode = PMODE_NONE; //Clear default, don't draw pixel
	*pixel_mode |= FIRE_ADD;
	//Returning 0 means dynamic, do not cache
	return 0;
}
int graphics_FIRW(GRAPHICS_FUNC_ARGS)
{
	int caddress = restrict_flt(restrict_flt((float)((int)(parts[i].life/2)), 0.0f, 200.0f)*3, 0.0f, (200.0f*3)-3);
	*colr = (unsigned char)firw_data[caddress];
	*colg = (unsigned char)firw_data[caddress+1];
	*colb = (unsigned char)firw_data[caddress+2];
	
	if (decorations_enable && parts[i].dcolour)
	{
		int a = (parts[i].dcolour>>24)&0xFF;
		*colr = (a*((parts[i].dcolour>>16)&0xFF) + (255-a)**colr) >> 8;
		*colg = (a*((parts[i].dcolour>>8)&0xFF) + (255-a)**colg) >> 8;
		*colb = (a*((parts[i].dcolour)&0xFF) + (255-a)**colb) >> 8;
	}
	
	*firea = 255;
	*firer = *colr/8;
	*fireg = *colg/8;
	*fireb = *colb/8;
	
	*pixel_mode = PMODE_NONE; //Clear default, don't draw pixel
	*pixel_mode |= FIRE_ADD;
	//Returning 0 means dynamic, do not cache
	return 0;
}
int graphics_BOMB(GRAPHICS_FUNC_ARGS)
{
	if (parts[i].tmp==0) {
		*pixel_mode |= PMODE_FLARE;
	}
	else
	{
		*pixel_mode |= PMODE_SPARK;
	}
	return 0;
}
int graphics_GBMB(GRAPHICS_FUNC_ARGS)
{
	if (parts[i].life <= 0) {
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
	*colr += (parts[i].tmp2-295.15f)/3;
	
	if (*colr > 170)
		*colr = 170;
	if (*colr < *colg)
		*colr = *colg;
		
	*colg = *colb = *colr;

	if((parts[i].temp-295.15f) > 300.0f-200.0f)
	{
		float frequency = 3.1415/(2*300.0f-(300.0f-200.0f));
		int q = ((parts[i].temp-295.15f)>300.0f)?300.0f-(300.0f-200.0f):(parts[i].temp-295.15f)-(300.0f-200.0f);

		*colr += sin(frequency*q) * 226;
		*colg += sin(frequency*q*4.55 +3.14) * 34;
		*colb += sin(frequency*q*2.22 +3.14) * 64;
	}
	return 0;
}

