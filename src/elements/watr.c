#include <element.h>

int update_WATR(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if ((r>>PS)>=NPART || !r)
					continue;
				if ((r&TYPE)==PT_SALT && 1>(rand()%250))
				{
					part_change_type(i,x,y,PT_SLTW);
					part_change_type(r>>PS,x+rx,y+ry,PT_SLTW);
				}
				if (((r&TYPE)==PT_RBDM||(r&TYPE)==PT_LRBD) && (legacy_enable||parts[i].temp>(273.15f+12.0f)) && 1>(rand()%500))
				{
					part_change_type(i,x,y,PT_FIRE);
					parts[i].life = 4;
				}
				/*if ((r&TYPE)==PT_CNCT && 1>(rand()%500))
				{
					part_change_type(i,x,y,PT_PSTE);
					kill_part(r>>PS);
				}*/
			}
	return 0;
}
