#include <element.h>

int update_C5(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if ((r>>PS)>=NPART || !r)
					continue;
				if ((parts[r>>PS].type!=PT_C5 && parts[r>>PS].temp<100)||parts[r>>PS].type==PT_HFLM)
				{
					if (1>rand()%6)
					{
						part_change_type(i,x,y,PT_HFLM);
						parts[r>>PS].temp = parts[i].temp = 0;
						parts[i].life = rand()%150+50;
						pv[y/CELL][x/CELL] += 1.5;
					}
				}
			}
	return 0;
}
