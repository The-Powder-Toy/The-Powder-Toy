#include <powder.h>

int update_WARP(UPDATE_FUNC_ARGS) {
	int trade, r;
	for ( trade = 0; trade<5; trade ++)
	{
		nx = rand()%3-1;
		ny = rand()%3-1;
		if (x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
		{
			r = pmap[y+ny][x+nx];
			if ((r>>8)>=NPART || !r)
				continue;
			if (parts[r>>8].type!=PT_WARP&&parts[r>>8].type!=PT_STKM&&parts[r>>8].type!=PT_DMND&&parts[r>>8].type!=PT_CLNE&&parts[r>>8].type!=PT_BCLN&&parts[r>>8].type!=PT_PCLN&&(10>=rand()%200))
			{
				parts[i].x = parts[r>>8].x;
				parts[i].y = parts[r>>8].y;
				parts[r>>8].x = x;
				parts[r>>8].y = y;
				parts[i].life += 4;
				trade = 5;
			}
		}
	}
	return 0;
}
