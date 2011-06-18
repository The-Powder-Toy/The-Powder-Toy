#include <element.h>

int update_MERC(UPDATE_FUNC_ARGS) {
	int r, rx, ry, trade, np;
	int maxtmp = ((10000/(parts[i].temp + 1))-1);
	if ((10000%((int)parts[i].temp+1))>rand()%((int)parts[i].temp+1))
		maxtmp ++;
	if (parts[i].tmp < maxtmp)
	{
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if ((r>>8)>=NPART || !r || (parts[i].tmp >=maxtmp))
						continue;
					if ((r&0xFF)==PT_MERC&&33>=rand()/(RAND_MAX/100)+1)
					{
						if ((parts[i].tmp + parts[r>>8].tmp + 1) <= maxtmp)
						{
							parts[i].tmp += parts[r>>8].tmp + 1;
							kill_part(r>>8);
						}
					}
				}
	}
	else
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if ((r>>8)>=NPART || (parts[i].tmp<=maxtmp))
						continue;
					if ((bmap[(y+ry)/CELL][(x+rx)/CELL]==WL_WALLELEC||bmap[(y+ry)/CELL][(x+rx)/CELL]==WL_EWALL||bmap[(y+ry)/CELL][(x+rx)/CELL]==WL_DESTROYALL||bmap[(y+ry)/CELL][(x+rx)/CELL]==WL_WALL||
					        bmap[(y+ry)/CELL][(x+rx)/CELL]==WL_ALLOWAIR||bmap[(y+ry)/CELL][(x+rx)/CELL]==WL_ALLOWSOLID||bmap[(y+ry)/CELL][(x+rx)/CELL]==WL_ALLOWGAS))
						continue;
					if ((!r)&&parts[i].tmp>=1)//if nothing then create deut
					{
						np = create_part(-1,x+rx,y+ry,PT_MERC);
						if (np<0) continue;
						parts[i].tmp--;
						parts[np].temp = parts[i].temp;
						parts[np].tmp = 0;
					}
				}
	for ( trade = 0; trade<4; trade ++)
	{
		rx = rand()%5-2;
		ry = rand()%5-2;
		if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
		{
			r = pmap[y+ry][x+rx];
			if ((r>>8)>=NPART || !r)
				continue;
			if ((r&0xFF)==PT_MERC&&(parts[i].tmp>parts[r>>8].tmp)&&parts[i].tmp>0)//diffusion
			{
				int temp = parts[i].tmp - parts[r>>8].tmp;
				if (temp ==1)
				{
					parts[r>>8].tmp ++;
					parts[i].tmp --;
				}
				else if (temp>0)
				{
					parts[r>>8].tmp += temp/2;
					parts[i].tmp -= temp/2;
				}
			}
		}
	}
	return 0;
}
