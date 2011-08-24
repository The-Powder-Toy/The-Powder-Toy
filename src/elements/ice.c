#include <element.h>

int update_ICEI(UPDATE_FUNC_ARGS) { //currently used for snow as well
	int r, rx, ry;
	if (parts[i].ctype==PT_FRZW)//get colder if it is from FRZW
	{
		parts[i].temp = restrict_flt(parts[i].temp-1.0f, MIN_TEMP, MAX_TEMP);
	}
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (((r&0xFF)==PT_SALT || (r&0xFF)==PT_SLTW) && 1>(rand()%1000))
				{
					part_change_type(i,x,y,PT_SLTW);
					part_change_type(r>>8,x+rx,y+ry,PT_SLTW);
				}
			}
	return 0;
}
