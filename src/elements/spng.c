#include <element.h>

int update_SPNG(UPDATE_FUNC_ARGS) {
	int r, trade, rx, ry, tmp, np;
	if (pv[y/CELL][x/CELL]<=3&&pv[y/CELL][x/CELL]>=-3&&parts[i].temp<=374.0f)
	{
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
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
			if (!r)
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
	tmp = 0;
	if (parts[i].life>0)
	{
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((r&0xFF)==PT_FIRE)
					{
						tmp++;
						if (parts[r>>8].life>60)
							parts[r>>8].life -= parts[r>>8].life/60;
						else if (parts[r>>8].life>2)
							parts[r>>8].life--;
					}
				}
	}
	if (tmp && parts[i].life>3)
		parts[i].life -= parts[i].life/3;
	if (tmp>1)
		tmp = tmp/2;
	if (tmp || parts[i].temp>=374)
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if ((bmap[(y+ry)/CELL][(x+rx)/CELL]==WL_WALLELEC||bmap[(y+ry)/CELL][(x+rx)/CELL]==WL_ALLOWLIQUID||bmap[(y+ry)/CELL][(x+rx)/CELL]==WL_DESTROYALL||bmap[(y+ry)/CELL][(x+rx)/CELL]==WL_WALL||bmap[(y+ry)/CELL][(x+rx)/CELL]==WL_ALLOWSOLID))
						continue;
					if ((!r)&&parts[i].life>=1)//if nothing then create steam
					{
						np = create_part(-1,x+rx,y+ry,PT_WTRV);
						if (np>-1)
						{
							parts[np].temp = parts[i].temp;
							tmp--;
						}
						parts[i].life--;
						parts[i].temp -= 20.0f;
					}
				}
	if (tmp>0)
	{
		if (parts[i].life>tmp)
			parts[i].life -= tmp;
		else
			parts[i].life = 0;
	}
	return 0;
}
