#include <powder.h>

int update_GLOW(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if ((r>>8)>=NPART || !r)
					continue;
				if ((r&0xFF)==PT_WATR&&5>(rand()%2000))
				{
					parts[i].type = PT_NONE;
					part_change_type(r>>8,x+rx,y+ry,PT_DEUT);
					parts[r>>8].life = 10;
				}
			}
	if (parts[i].type==PT_NONE) {
		kill_part(i);
		return 1;
	}
	return 0;
}
