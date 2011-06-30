#include <element.h>

int update_FOG(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if ((r>>PS)>=NPART || !r)
					continue;
				if (ptypes[r&TYPE].state==ST_SOLID&&5>=rand()%50&&parts[i].life==0&&!(parts[r>>PS].type==PT_CLNE||parts[r>>PS].type==PT_PCLN)) // TODO: should this also exclude BCLN?
				{
					part_change_type(i,x,y,PT_RIME);
				}
				if (parts[r>>PS].type==PT_SPRK)
				{
					parts[i].life += rand()%20;
				}
			}
	return 0;
}
