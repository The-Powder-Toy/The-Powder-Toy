#include <element.h>

int update_C6(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if ((r>>PS)>=NPART || !r)
					continue;
				if ((r&TYPE!=PT_C6 && parts[r>>PS].temp>1000)||r&TYPE==PT_BFLM ||r&TYPE==PT_PLSM ||r&TYPE==PT_FIRE)
				{
					if (1>rand()%6)
					{
						part_change_type(i,x,y,PT_BFLM);
						parts[r>>PS].temp = parts[i].temp = MAX_TEMP;
						parts[i].life = rand()%150+50;
						pv[y/CELL][x/CELL] += 10.0;
					}
				}
			}
	return 0;
}
