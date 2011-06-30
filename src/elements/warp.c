#include <element.h>

int update_WARP(UPDATE_FUNC_ARGS) {
	int trade, r, rx, ry;
	for ( trade = 0; trade<5; trade ++)
	{
		rx = rand()%3-1;
		ry = rand()%3-1;
		if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
		{
			r = pmap[y+ry][x+rx];
			if ((r>>PS)>=NPART || !r)
				continue;
			if (parts[r>>PS].type!=PT_WARP&&parts[r>>PS].type!=PT_STKM&&parts[r>>PS].type!=PT_STKM2&&parts[r>>PS].type!=PT_DMND&&parts[r>>PS].type!=PT_CLNE&&parts[r>>PS].type!=PT_BCLN&&parts[r>>PS].type!=PT_PCLN&&(10>=rand()%200))
			{
				parts[i].x = parts[r>>PS].x;
				parts[i].y = parts[r>>PS].y;
				parts[r>>PS].x = x;
				parts[r>>PS].y = y;
				parts[i].life += 4;
				pmap[y][x] = r;
				pmap[y+ry][x+rx] = (i<<PS)|parts[i].type;
				trade = 5;
			}
		}
	}
	return 0;
}
