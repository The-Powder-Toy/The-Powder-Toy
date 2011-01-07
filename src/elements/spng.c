#include <powder.h>

int update_SPNG(UPDATE_FUNC_ARGS) {
	int r, trade;
	if (pv[y/CELL][x/CELL]<=3&&pv[y/CELL][x/CELL]>=-3)
	{
		for (nx=-1; nx<2; nx++)
			for (ny=-1; ny<2; ny++)
				if (x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
				{
					r = pmap[y+ny][x+nx];
					if ((r>>8)>=NPART || !r || parts[i].temp>374.0f)
						continue;
					if (parts[r>>8].type==PT_WATR&&33>=rand()/(RAND_MAX/100)+1)
					{
						parts[i].life++;
						parts[r>>8].type=PT_NONE;
					}
				}
	}
	else
		for (nx=-1; nx<2; nx++)
			for (ny=-1; ny<2; ny++)
				if (x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
				{
					r = pmap[y+ny][x+nx];
					if ((r>>8)>=NPART)
						continue;
					if ((bmap[(y+ny)/CELL][(x+nx)/CELL]==WL_WALLELEC||bmap[(y+ny)/CELL][(x+nx)/CELL]==WL_EWALL||bmap[(y+ny)/CELL][(x+nx)/CELL]==WL_DESTROYALL||bmap[(y+ny)/CELL][(x+nx)/CELL]==WL_WALL||
					        bmap[(y+ny)/CELL][(x+nx)/CELL]==WL_ALLOWAIR||bmap[(y+ny)/CELL][(x+nx)/CELL]==WL_ALLOWSOLID||bmap[(y+ny)/CELL][(x+nx)/CELL]==WL_ALLOWGAS))
						continue;
					if ((!r)&&parts[i].life>=1)//if nothing then create water
					{
						create_part(-1,x+nx,y+ny,PT_WATR);
						parts[i].life--;
					}
				}
	for ( trade = 0; trade<9; trade ++)
	{
		nx = rand()%5-2;
		ny = rand()%5-2;
		if (x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
		{
			r = pmap[y+ny][x+nx];
			if ((r>>8)>=NPART || !r)
				continue;
			if (parts[r>>8].type==PT_SPNG&&(parts[i].life>parts[r>>8].life)&&parts[i].life>0)//diffusion
			{
				int temp = parts[i].life - parts[r>>8].life;
				if (temp ==1)
				{
					parts[r>>8].life ++;
					parts[i].life --;
					trade = 9;
				}
				else if (temp>0)
				{
					parts[r>>8].life += temp/2;
					parts[i].life -= temp/2;
					trade = 9;
				}
			}
		}
	}
	for (nx=-1; nx<2; nx++)
		for (ny=-1; ny<2; ny++)
			if (x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
			{
				r = pmap[y+ny][x+nx];
				if ((r>>8)>=NPART || !r)
					continue;
				if (parts[r>>8].type==PT_FIRE&&parts[i].life>0)
				{
					if (parts[i].life<=2)
						parts[i].life --;
					parts[i].life -= parts[i].life/3;
				}
			}
	if (parts[i].temp>=374)
		for (nx=-1; nx<2; nx++)
			for (ny=-1; ny<2; ny++)
				if (x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
				{
					r = pmap[y+ny][x+nx];
					if ((r>>8)>=NPART)
						continue;
					if ((bmap[(y+ny)/CELL][(x+nx)/CELL]==WL_WALLELEC||bmap[(y+ny)/CELL][(x+nx)/CELL]==WL_ALLOWLIQUID||bmap[(y+ny)/CELL][(x+nx)/CELL]==WL_DESTROYALL||bmap[(y+ny)/CELL][(x+nx)/CELL]==WL_WALL||bmap[(y+ny)/CELL][(x+nx)/CELL]==WL_ALLOWSOLID))
						continue;
					if ((!r)&&parts[i].life>=1)//if nothing then create steam
					{
						create_part(-1,x+nx,y+ny,PT_WTRV);
						parts[i].life--;
						parts[i].temp -= 40.0f;
					}
				}
	return 0;
}
