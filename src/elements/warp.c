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
			if (r&TYPE!=PT_WARP&&r&TYPE!=PT_STKM&&r&TYPE!=PT_STKM2&&r&TYPE!=PT_DMND&&r&TYPE!=PT_CLNE&&r&TYPE!=PT_BCLN&&r&TYPE!=PT_PCLN&&(10>=rand()%200))
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
