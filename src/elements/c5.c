#include <powder.h>

int update_C5(UPDATE_FUNC_ARGS) {
	int r;
	for (nx=-2; nx<3; nx++)
		for (ny=-2; ny<3; ny++)
			if (x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
			{
				r = pmap[y+ny][x+nx];
				if ((r>>8)>=NPART || !r)
					continue;
				if ((parts[r>>8].temp<100 && parts[r>>8].type!=PT_C5)||parts[r>>8].type==PT_HFLM)
				{
					if (1>rand()%6)
					{
						parts[i].type = PT_HFLM;
						parts[r>>8].temp = parts[i].temp = 0;
						parts[i].life = rand()%150+50;
						pv[y/CELL][x/CELL] += 1.5;
					}
				}
			}
	return 0;
}
