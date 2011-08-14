#include <element.h>

int update_O2(UPDATE_FUNC_ARGS)
{
	int r,rx,ry;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if ((r>>8)>=NPART || !r)
					continue;

				if ((r&0xFF)==PT_FIRE)
				{
					parts[r>>8].temp+=(rand()/(RAND_MAX/100));
					if(parts[r>>8].tmp&0x01)
					parts[r>>8].temp=3473;
					parts[r>>8].tmp |= 2;
				}
				if ((r&0xFF)==PT_FIRE || (r&0xFF)==PT_PLSM)
				{
					create_part(i,x,y,PT_FIRE);
					parts[i].temp+=(rand()/(RAND_MAX/100));
					parts[i].tmp |= 2;
				}

			}
	return 0;
}
