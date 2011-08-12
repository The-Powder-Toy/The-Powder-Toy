#include <element.h>

int update_H2(UPDATE_FUNC_ARGS)
{
	int r,rx,ry,rt;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				rt = (r&0xFF);
				if ((r>>8)>=NPART || !r)
					continue;
				if (pv[y/CELL][x/CELL] > 8.0f && rt == PT_DESL) // This will not work. DESL turns to fire above 5.0 pressure
				{
					part_change_type(r>>8,x+rx,y+ry,PT_WATR);
					part_change_type(i,x,y,PT_OIL);
				}
				if ((r&0xFF)==PT_FIRE)
				{
					parts[r>>8].temp=2473.15;
					if(parts[r>>8].tmp&0x02)
					parts[r>>8].temp=3473;
					parts[r>>8].tmp |= 1;
				}
				if ((r&0xFF)==PT_FIRE || (r&0xFF)==PT_PLSM || (r&0xFF)==PT_LAVA)
				{
					create_part(i,x,y,PT_FIRE);
					parts[i].temp+=(rand()/(RAND_MAX/100));
					parts[i].tmp |= 1;
				}
			}
	return 0;
}
