#include <element.h>

int update_GLOW(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)==PT_WATR&&5>(rand()%2000))
				{
					parts[i].type = PT_NONE;
					part_change_type(r>>8,x+rx,y+ry,PT_DEUT);
					parts[r>>8].life = 10;
				}
			}
	parts[i].ctype = pv[y/CELL][x/CELL]*16;

	parts[i].tmp = abs((int)((vx[y/CELL][x/CELL]+vy[y/CELL][x/CELL])*16.0f)) + abs((int)((parts[i].vx+parts[i].vy)*64.0f));
	//printf("%f %f\n", parts[i].vx, parts[i].vy);
	if (parts[i].type==PT_NONE) {
		kill_part(i);
		return 1;
	}
	return 0;
}
