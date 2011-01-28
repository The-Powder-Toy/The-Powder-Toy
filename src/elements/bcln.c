#include <element.h>

int update_BCLN(UPDATE_FUNC_ARGS) {
	if (!parts[i].life)
	{
		if (pv[y/CELL][x/CELL]>4.0f)
		{
			float advection = 0.1f;
			parts[i].vx += advection*vx[y/CELL][x/CELL];
			parts[i].vy += advection*vy[y/CELL][x/CELL];
			parts[i].life = rand()%40+80;
		}
	}
	if (!parts[i].ctype)
	{
		int rx, ry;
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES &&
				        pmap[y+ry][x+rx] &&
				        (pmap[y+ry][x+rx]&0xFF)!=PT_CLNE &&
				        (pmap[y+ry][x+rx]&0xFF)!=PT_BCLN &&
				        (pmap[y+ry][x+rx]&0xFF)!=PT_PCLN &&
				        (pmap[y+ry][x+rx]&0xFF)!=PT_STKM &&
				        (pmap[y+ry][x+rx]&0xFF)!=PT_STKM2 &&
				        (pmap[y+ry][x+rx]&0xFF)!=0xFF)
					parts[i].ctype = pmap[y+ry][x+rx]&0xFF;
	}
	else {
		create_part(-1, x+rand()%3-1, y+rand()%3-1, parts[i].ctype);
	}
	return 0;
}
