#include <element.h>

int update_DSTW(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if ((r>>PS)>=NPART || !r)
					continue;
				if (parts[r>>PS].type==PT_SALT && 1>(rand()%250))
				{
					part_change_type(i,x,y,PT_SLTW);
					part_change_type(r>>PS,x+rx,y+ry,PT_SLTW);
				}
				if ((parts[r>>PS].type==PT_WATR||parts[r>>PS].type==PT_SLTW) && 1>(rand()%500))
				{
					part_change_type(i,x,y,PT_WATR);
				}
				if (parts[r>>PS].type==PT_SLTW && 1>(rand()%500))
				{
					part_change_type(i,x,y,PT_SLTW);
				}
				if ((parts[r>>PS].type==PT_RBDM||parts[r>>PS].type==PT_LRBD) && (legacy_enable||parts[i].temp>12.0f) && 1>(rand()%500))
				{
					part_change_type(i,x,y,PT_FIRE);
					parts[i].life = 4;
				}
			}
	return 0;
}
