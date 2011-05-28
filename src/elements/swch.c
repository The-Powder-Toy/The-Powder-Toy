#include <element.h>

int update_SWCH(UPDATE_FUNC_ARGS) {
	int r, rt, rx, ry;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if ((r>>PS)>=NPART || !r)
					continue;
				if (parts_avg(i,r>>PS,PT_INSL)!=PT_INSL) {
					rt = r&TYPE;
					if (rt==PT_SWCH)
					{
						if (parts[i].life>=10&&parts[r>>PS].life<10&&parts[r>>PS].life>0)
							parts[i].life = 9;
						else if (parts[i].life==0&&parts[r>>PS].life==10)
							parts[i].life = 10;
					}
					else if (rt==PT_SPRK&&parts[i].life==10&&parts[r>>PS].ctype!=PT_PSCN&&parts[r>>PS].ctype!=PT_NSCN) {
						part_change_type(i,x,y,PT_SPRK);
						parts[i].ctype = PT_SWCH;
						parts[i].life = 4;
					}
				}
			}
	//turn off SWCH from two red BRAYS
	if (parts[i].life==10 && (!(pmap[y-1][x-1]&TYPE) && ((pmap[y-1][x]&TYPE)==PT_BRAY&&parts[pmap[y-1][x]>>PS].tmp==2) && !(pmap[y-1][x+1]&TYPE) && ((pmap[y][x+1]&TYPE)==PT_BRAY&&parts[pmap[y][x+1]>>PS].tmp==2)))
	{
		parts[i].life = 9;
	}
	//turn on SWCH from two red BRAYS
	else if (parts[i].life<=5 && (!(pmap[y-1][x-1]&TYPE) && (((pmap[y-1][x]&TYPE)==PT_BRAY&&parts[pmap[y-1][x]>>PS].tmp==2) || ((pmap[y+1][x]&TYPE)==PT_BRAY&&parts[pmap[y+1][x]>>PS].tmp==2)) && !(pmap[y-1][x+1]&TYPE) && (((pmap[y][x+1]&TYPE)==PT_BRAY&&parts[pmap[y][x+1]>>PS].tmp==2) || ((pmap[y][x-1]&TYPE)==PT_BRAY&&parts[pmap[y][x-1]>>PS].tmp==2))))
	{
		parts[i].life = 14;
	}
	return 0;
}
