#include <element.h>

int update_FRZZ(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)==PT_WATR&&5>rand()%100)
				{
					part_change_type(r>>8,x+rx,y+ry,PT_FRZW);
					parts[r>>8].life = 100;
					parts[i].type = PT_NONE;
				}

			}
	if (parts[i].type==PT_NONE) {
		kill_part(i);
		return 1;
	}
	return 0;
}
