#include <element.h>

//int active = 0;
int update_FF50(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	   for(rx=-1; rx<2; rx++)
		   for(ry=-1; ry<2; ry++)
			   if(x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES &&
				   pmap[y+ry][x+rx] &&
				   (pmap[y+ry][x+rx]&0xFF)!=0xFF) {
		   r = pmap[y+ry][x+rx];
		   if (parts[i].life < 0)
			   parts[i].life = 2;
		   if (parts[i].life == 0)
			   parts[i].life = 256;
		   /*if (parts[i].life > 256)
			   parts[i].life = 256;*/
		   /*if (active == 0) {
			    parts[i].life = 256;
		   }
		   else if (active == 1) {
				parts[i].life--;
		   }
		   else if (active == 2) {
			    if (parts[i].life < 256)
					parts[i].life++;
				else
					active = 0;
		   }*/
		   if(parts[r>>8].type == PT_CCTV) {
			    //active = 1;
			   parts[i].life--;
		   }
		   /*else if(parts[r>>8].type == PT_FF50) {
				parts[i].life = parts[r>>8].life;
		   }*/
		   if (parts[i].life == 1) {
			    parts[i].life = 10000;
				parts[i].tmp = 50;
				parts[i].type = PT_FIRE;
				//active = 2;
		   }
		   if (parts[r>>8].type == PT_FIRE && parts[r>>8].tmp == 50) {
			    parts[i].life = 10000;
			    parts[i].type = PT_FIRE;
				//active = 2;
		   }

	   }
	   return 0;
}