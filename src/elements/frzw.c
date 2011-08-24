#include <element.h>

int update_FRZW(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)==PT_WATR&&5>rand()%70)
				{
					part_change_type(r>>8,x+rx,y+ry,PT_FRZW);
				}
			}
	if (parts[i].life==0&&13>rand()%2500)
	{
		part_change_type(i,x,y,PT_ICEI);
		parts[i].ctype=PT_FRZW;
		parts[i].temp = restrict_flt(parts[i].temp-200.0f, MIN_TEMP, MAX_TEMP);
	}
	else if ((100-(parts[i].life))>rand()%50000)
	{
		part_change_type(i,x,y,PT_ICEI);
		parts[i].ctype=PT_FRZW;
		parts[i].temp = restrict_flt(parts[i].temp-200.0f, MIN_TEMP, MAX_TEMP);
	}
	return 0;
}
