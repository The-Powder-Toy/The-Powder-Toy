#include <powder.h>

int update_DEUT(UPDATE_FUNC_ARGS) {
	int r, trade;
	int maxlife = ((10000/(parts[i].temp + 1))-1);
	if ((10000%((int)parts[i].temp+1))>rand()%((int)parts[i].temp+1))
		maxlife ++;
	if (parts[i].life < maxlife)
	{
		for (nx=-1; nx<2; nx++)
			for (ny=-1; ny<2; ny++)
				if (x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
				{
					r = pmap[y+ny][x+nx];
					if ((r>>8)>=NPART || !r || (parts[i].life >=maxlife))
						continue;
					if (parts[r>>8].type==PT_DEUT&&33>=rand()/(RAND_MAX/100)+1)
					{
						if ((parts[i].life + parts[r>>8].life + 1) <= maxlife)
						{
							parts[i].life += parts[r>>8].life + 1;
							parts[r>>8].type=PT_NONE;
						}
					}
				}
	}
	else
		for (nx=-1; nx<2; nx++)
			for (ny=-1; ny<2; ny++)
				if (x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
				{
					r = pmap[y+ny][x+nx];
					if ((r>>8)>=NPART || (parts[i].life<=maxlife))
						continue;
					if ((bmap[(y+ny)/CELL][(x+nx)/CELL]==WL_WALLELEC||bmap[(y+ny)/CELL][(x+nx)/CELL]==WL_EWALL||bmap[(y+ny)/CELL][(x+nx)/CELL]==WL_DESTROYALL||bmap[(y+ny)/CELL][(x+nx)/CELL]==WL_WALL||
					        bmap[(y+ny)/CELL][(x+nx)/CELL]==WL_ALLOWAIR||bmap[(y+ny)/CELL][(x+nx)/CELL]==WL_ALLOWSOLID||bmap[(y+ny)/CELL][(x+nx)/CELL]==WL_ALLOWGAS))
						continue;
					if ((!r)&&parts[i].life>=1)//if nothing then create deut
					{
						create_part(-1,x+nx,y+ny,PT_DEUT);
						parts[i].life--;
						parts[pmap[y+ny][x+nx]>>8].temp = parts[i].temp;
						parts[pmap[y+ny][x+nx]>>8].life = 0;
					}
				}
	for ( trade = 0; trade<4; trade ++)
	{
		nx = rand()%5-2;
		ny = rand()%5-2;
		if (x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
		{
			r = pmap[y+ny][x+nx];
			if ((r>>8)>=NPART || !r)
				continue;
			if (parts[r>>8].type==PT_DEUT&&(parts[i].life>parts[r>>8].life)&&parts[i].life>0)//diffusion
			{
				int temp = parts[i].life - parts[r>>8].life;
				if (temp ==1)
				{
					parts[r>>8].life ++;
					parts[i].life --;
				}
				else if (temp>0)
				{
					parts[r>>8].life += temp/2;
					parts[i].life -= temp/2;
				}
			}
		}
	}
	return 0;
}
