#include <element.h>

int update_CLST(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if ((r>>8)>=NPART || !r)
					continue;
				if ((r&0xFF)==PT_WATR && 1>(rand()%1500))
				{
					part_change_type(i,x,y,PT_PSTS);					
					kill_part(r>>8);
				}
				if ((r&0xFF)==PT_CLST)
				{
					parts[i].vx += 0.01*rx; parts[i].vy += 0.01*ry;
				}
			}
	return 0;
}
