#include <element.h>

int update_WATR(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if ((r>>8)>=NPART || !r)
					continue;
				if ((r&0xFF)==PT_SALT && 1>(rand()%250))
				{
					part_change_type(i,x,y,PT_SLTW);
					part_change_type(r>>8,x+rx,y+ry,PT_SLTW);
				}
				if (((r&0xFF)==PT_RBDM||(r&0xFF)==PT_LRBD) && (legacy_enable||parts[i].temp>(273.15f+12.0f)) && 1>(rand()%500))
				{
					part_change_type(i,x,y,PT_FIRE);
					parts[i].life = 4;
				}
				/*if ((r&0xFF)==PT_CNCT && 1>(rand()%500))	Concrete+Water to paste, not very popular
				{
					part_change_type(i,x,y,PT_PSTE);
					kill_part(r>>8);
				}*/
			}
	return 0;
}
