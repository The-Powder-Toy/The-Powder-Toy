#include <element.h>

int update_SING(UPDATE_FUNC_ARGS) {
	int r, rx, ry, cry, crx, rad, nxi, nxj, nb;
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
	if (parts[i].life<1) {
		//Pop!
		for (rx=-2; rx<3; rx++) {
			crx = (x/CELL)+rx;
			for (ry=-2; ry<3; ry++) {
				cry = (y/CELL)+ry;
				if (cry > 0 && crx > 0 && crx < (XRES/CELL) && cry < (YRES/CELL)) {
					pv[cry][crx] += (float)parts[i].tmp;
				}
			}
		}
		rad = (parts[i].tmp>255)?255:parts[i].tmp;
		if (rad>=1) {
			rad = (int)(((float)rad)/8.0f);
		}
		if (rad>=1) {
			for (nxj=-(rad+1); nxj<=(rad+1); nxj++)
				for (nxi=-(rad+1); nxi<=(rad+1); nxi++)
					if ((pow(nxi,2))/(pow((rad+1),2))+(pow(nxj,2))/(pow((rad+1),2))<=1) {
						if (rand()%2) {
							nb = create_part(-1, x+nxi, y+nxj, PT_PHOT);
						} else {
							nb = create_part(-1, x+nxi, y+nxj, PT_NEUT);
						}
						if (nb!=-1) {
							parts[nb].life = (rand()%300);
							parts[nb].temp = MAX_TEMP/2;
							parts[nb].vx = ((float)(rand()%100-50))/10.0f;
							parts[nb].vy = ((float)(rand()%100-50))/10.0f;
						}
					}
		}
		kill_part(i);
		return 1;
	}
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if ((r>>8)>=NPART || !r)
					continue;
				if ((r&0xFF)!=PT_DMND&&33>=rand()/(RAND_MAX/100)+1)
				{
					if ((r&0xFF)==PT_SING && parts[r>>8].life >10)
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
								int np;
								np = create_part(r>>8,x+rx,y+ry,PT_SING);
								parts[np].life = rand()%50+60;
							}
							continue;
						}
						parts[i].life += 3;
						parts[i].tmp++;
					}
					parts[i].temp = restrict_flt(parts[r>>8].temp+parts[i].temp, MIN_TEMP, MAX_TEMP);
					kill_part(r>>8);
				}
			}
	return 0;
}
