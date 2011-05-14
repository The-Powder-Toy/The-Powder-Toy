#include <element.h>

int update_LEAF(UPDATE_FUNC_ARGS) {
	int r, trade, rx, ry, tmp, np;
	if (1==1)
	{
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if ((r>>8)>=NPART || !r)
						continue;
                    if ((r&0xFF)==PT_WATR || (r&0xFF)==PT_WTRV || (r&0xFF)==PT_DSTW){
                        if(parts[i].life>0)
                        parts[i].life--;
                    }
					if (parts[i].temp>774.0f&&33>=rand()/(RAND_MAX/100)+1)
					{
						parts[i].life++;
						if ((r&0xFF)==PT_FIRE)
                        {
                            tmp++;
                            parts[i].type=PT_FIRE;
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
					if ((r>>8)>=NPART)
						continue;
					if ((bmap[(y+ry)/CELL][(x+rx)/CELL]==WL_WALLELEC||bmap[(y+ry)/CELL][(x+rx)/CELL]==WL_EWALL||bmap[(y+ry)/CELL][(x+rx)/CELL]==WL_DESTROYALL||bmap[(y+ry)/CELL][(x+rx)/CELL]==WL_WALL||
					        bmap[(y+ry)/CELL][(x+rx)/CELL]==WL_ALLOWAIR||bmap[(y+ry)/CELL][(x+rx)/CELL]==WL_ALLOWSOLID||bmap[(y+ry)/CELL][(x+rx)/CELL]==WL_ALLOWGAS))
						continue;
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
	tmp = 0;
 	if (parts[i].life>0)
        {
            for (rx=-1; rx<2; rx++)
                for (ry=-1; ry<2; ry++)
                    if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
                    if ((r>>8)>=NPART || !r)
                        continue;
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
					if ((r>>8)>=NPART)
						continue;
					if ((bmap[(y+ry)/CELL][(x+rx)/CELL]==WL_WALLELEC||bmap[(y+ry)/CELL][(x+rx)/CELL]==WL_ALLOWLIQUID||bmap[(y+ry)/CELL][(x+rx)/CELL]==WL_DESTROYALL||bmap[(y+ry)/CELL][(x+rx)/CELL]==WL_WALL||bmap[(y+ry)/CELL][(x+rx)/CELL]==WL_ALLOWSOLID))
						continue;
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
