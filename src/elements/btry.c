#include <element.h>

int update_BTRY(UPDATE_FUNC_ARGS) {
	int r, rx, ry, rt;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				rt = parts[r>>8].type;
				if (parts_avg(i,r>>8,PT_INSL) != PT_INSL)
				{
					if ((ptypes[rt].properties&PROP_CONDUCTS) && !(rt==PT_WATR||rt==PT_SLTW||rt==PT_NTCT||rt==PT_PTCT||rt==PT_INWR) && parts[r>>8].life==0 && abs(rx)+abs(ry) < 4)
					{
						parts[r>>8].life = 4;
						parts[r>>8].ctype = rt;
						part_change_type(r>>8,x+rx,y+ry,PT_SPRK);
					}
				}
			}
	return 0;
}
