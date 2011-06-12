#include <element.h>

int update_IBAT(UPDATE_FUNC_ARGS) {
	int r, rx, ry, rt;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if ((r>>PS)>=NPART || !r)
					continue;
				rt = (r&TYPE);
				if (parts_avg(i,r>>PS,PT_INSL) != PT_INSL)
				{
					if ((ptypes[rt].properties&PROP_CONDUCTS) && !(rt==PT_WATR||rt==PT_SLTW||rt==PT_NTCT||rt==PT_PTCT||rt==PT_INWR) && parts[r>>PS].life==0 && abs(rx)+abs(ry) < 4)
					{
						parts[r>>PS].life = 4;
						parts[r>>PS].ctype = rt;
						part_change_type(r>>PS,x+rx,y+ry,PT_SPRK);
					}
				}
			}
	return 0;
}
