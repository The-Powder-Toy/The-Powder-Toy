#include <element.h>

int update_RIME(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	parts[i].vx = 0;
	parts[i].vy = 0;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if ((r>>PS)>=NPART || !r)
					continue;
				if (parts[r>>PS].type==PT_SPRK)
				{
					part_change_type(i,x,y,PT_FOG);
					parts[i].life = rand()%50 + 60;
				}
				else if (parts[r>>PS].type==PT_FOG&&parts[r>>PS].life>0)
				{
					part_change_type(i,x,y,PT_FOG);
					parts[i].life = parts[r>>PS].life;
				}
			}
	return 0;
}
