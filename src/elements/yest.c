#include <element.h>

int update_YEST(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	int self = parts[i].type;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if ((r>>PS)>=NPART || !r)
					continue;
				if (parts[r>>PS].type==PT_DYST && 1>(rand()%30) && !legacy_enable)
				{
					part_change_type(i,x,y,PT_DYST);
				}
			}
	if (parts[i].temp>303&&parts[i].temp<317) {
		create_part(-1, x+rand()%3-1, y+rand()%3-1, self);
	}
	return 0;
}
