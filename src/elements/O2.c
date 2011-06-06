#include <element.h>

int update_O2(UPDATE_FUNC_ARGS)
{
	int r,rx,ry;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if ((r>>PS)>=NPART || !r)
					continue;

				if (parts[r>>PS].type==PT_FIRE)
				{
					parts[r>>PS].temp+=(rand()/(RAND_MAX/100));
					parts[r>>PS].tmp |= 2;
                }
                if (parts[r>>PS].type==PT_FIRE || parts[r>>PS].type==PT_PLSM)
                {
					create_part(i,x,y,PT_FIRE);
					parts[i].temp+=(rand()/(RAND_MAX/100));
					parts[i].tmp |= 2;
				}

			}
	return 0;
}
