#include <powder.h>

int update_SING(UPDATE_FUNC_ARGS) {
	int r;
	int singularity = -parts[i].life;

	if (pv[y/CELL][x/CELL]<singularity)
		pv[y/CELL][x/CELL] += 0.1f*(singularity-pv[y/CELL][x/CELL]);
	if (y+CELL<YRES && pv[y/CELL+1][x/CELL]<singularity)
		pv[y/CELL+1][x/CELL] += 0.1f*(singularity-pv[y/CELL+1][x/CELL]);
	if (x+CELL<XRES)
	{
		pv[y/CELL][x/CELL+1] += 0.1f*(singularity-pv[y/CELL][x/CELL+1]);
		if (y+CELL<YRES)
			pv[y/CELL+1][x/CELL+1] += 0.1f*(singularity-pv[y/CELL+1][x/CELL+1]);
	}
	if (y+CELL>0 && pv[y/CELL-1][x/CELL]<singularity)
		pv[y/CELL-1][x/CELL] += 0.1f*(singularity-pv[y/CELL-1][x/CELL]);
	if (x+CELL>0)
	{
		pv[y/CELL][x/CELL-1] += 0.1f*(singularity-pv[y/CELL][x/CELL-1]);
		if (y+CELL>0)
			pv[y/CELL-1][x/CELL-1] += 0.1f*(singularity-pv[y/CELL-1][x/CELL-1]);
	}
	for (nx=-1; nx<2; nx++)
		for (ny=-1; ny<2; ny++)
			if (x+nx>=0 && y+ny>0 &&
			        x+nx<XRES && y+ny<YRES && (nx || ny))
			{
				r = pmap[y+ny][x+nx];
				if ((r>>8)>=NPART || !r)
					continue;
				if (parts[r>>8].type!=PT_DMND&&33>=rand()/(RAND_MAX/100)+1)
				{
					if (parts[r>>8].type==PT_SING && parts[r>>8].life >10)
					{
						if (parts[i].life+parts[r>>8].life > 255)
							continue;
						parts[i].life += parts[r>>8].life;
					}
					else
					{
						if (parts[i].life+3 > 255)
						{
							if (parts[r>>8].type!=PT_SING && 1>rand()%100)
							{
								parts[r>>8].type = PT_SING;
								parts[r>>8].life = rand()%50+60;
							}
							continue;
						}
						parts[i].life += 3;
					}
					parts[i].temp = restrict_flt(parts[r>>8].temp+parts[i].temp, MIN_TEMP, MAX_TEMP);
					parts[r>>8].type=PT_NONE;
				}
			}
	return 0;
}
