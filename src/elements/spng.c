#include <element.h>

int update_SPNG(UPDATE_FUNC_ARGS) {
	int r, trade, rx, ry, tmp;
	if (pv[y/CELL][x/CELL]<=3&&pv[y/CELL][x/CELL]>=-3&&parts[i].temp<=374.0f)
	{
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if ((r>>8)>=NPART || !r || parts[i].temp>374.0f)
						continue;
					if ((r&0xFF)==PT_WATR&&33>=rand()/(RAND_MAX/100)+1)
					{
						parts[i].life++;
						kill_part(r>>8);
					}
				}
	}
	else
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if ((r>>8)>=NPART)
						continue;
					if ((bmap[(y+ry)/CELL][(x+rx)/CELL]==WL_WALLELEC||bmap[(y+ry)/CELL][(x+rx)/CELL]==WL_EWALL||bmap[(y+ry)/CELL][(x+rx)/CELL]==WL_DESTROYALL||bmap[(y+ry)/CELL][(x+rx)/CELL]==WL_WALL||
					        bmap[(y+ry)/CELL][(x+rx)/CELL]==WL_ALLOWAIR||bmap[(y+ry)/CELL][(x+rx)/CELL]==WL_ALLOWSOLID||bmap[(y+ry)/CELL][(x+rx)/CELL]==WL_ALLOWGAS))
						continue;
					if ((!r)&&parts[i].life>=1)//if nothing then create water
					{
						create_part(-1,x+rx,y+ry,PT_WATR);
						parts[i].life--;
					}
				}
	for ( trade = 0; trade<9; trade ++)
	{
		rx = rand()%5-2;
		ry = rand()%5-2;
		if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
		{
			r = pmap[y+ry][x+rx];
			if ((r>>8)>=NPART || !r)
				continue;
			if ((r&0xFF)==PT_SPNG&&(parts[i].life>parts[r>>8].life)&&parts[i].life>0)//diffusion
			{
				tmp = parts[i].life - parts[r>>8].life;
				if (tmp ==1)
				{
					parts[r>>8].life ++;
					parts[i].life --;
					trade = 9;
				}
				else if (tmp>0)
				{
					parts[r>>8].life += tmp/2;
					parts[i].life -= tmp/2;
					trade = 9;
				}
			}
		}
	}
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if ((r>>8)>=NPART || !r)
					continue;
				if ((r&0xFF)==PT_FIRE&&parts[i].life>0)
				{
					if (parts[i].life<=2)
						parts[i].life --;
					parts[i].life -= parts[i].life/3;
				}
			}
	if (parts[i].temp>=374)
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if ((r>>8)>=NPART)
						continue;
					if ((bmap[(y+ry)/CELL][(x+rx)/CELL]==WL_WALLELEC||bmap[(y+ry)/CELL][(x+rx)/CELL]==WL_ALLOWLIQUID||bmap[(y+ry)/CELL][(x+rx)/CELL]==WL_DESTROYALL||bmap[(y+ry)/CELL][(x+rx)/CELL]==WL_WALL||bmap[(y+ry)/CELL][(x+rx)/CELL]==WL_ALLOWSOLID))
						continue;
					if ((!r)&&parts[i].life>=1)//if nothing then create steam
					{
						create_part(-1,x+rx,y+ry,PT_WTRV);
						parts[i].life--;
						parts[i].temp -= 40.0f;
					}
				}
	return 0;
}
