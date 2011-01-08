#include <powder.h>

int update_FWRK(UPDATE_FUNC_ARGS) {
	int r;
	if ((parts[i].temp>400&&(9+parts[i].temp/40)>rand()%100000&&parts[i].life==0&&!pmap[y-1][x])||parts[i].ctype==PT_DUST)
	{
		create_part(-1, x , y-1 , PT_FWRK);
		r = pmap[y-1][x];
		if ((r&0xFF)==PT_FWRK)
		{
			parts[r>>8].vy = rand()%8-22;
			parts[r>>8].vx = rand()%20-rand()%20;
			parts[r>>8].life=rand()%15+25;
			parts[i].type=PT_NONE;
		}
	}
	if (parts[i].life>1)
	{
		if (parts[i].life>=45&&parts[i].type==PT_FWRK)
			parts[i].life=0;
	}
	if ((parts[i].life<3&&parts[i].life>0)||parts[i].vy>6&&parts[i].life>0)
	{
		int q = (rand()%255+1);
		int w = (rand()%255+1);
		int e = (rand()%255+1);
		for (nx=-1; nx<2; nx++)
			for (ny=-2; ny<3; ny++)
				if (x+nx>=0 && y+ny>0 &&
				        x+nx<XRES && y+ny<YRES)
				{
					if (5>=rand()%8)
					{
						if (!pmap[y+ny][x+nx])
						{
							create_part(-1, x+nx, y+ny , PT_DUST);
							pv[y/CELL][x/CELL] += 2.00f*CFDS;
							r = pmap[y+ny][x+nx];
							if (parts[r>>8].type==PT_DUST)
							{
								parts[r>>8].vy = -(rand()%10-1);
								parts[r>>8].vx = ((rand()%2)*2-1)*rand()%(5+5)+(parts[i].vx)*2 ;
								parts[r>>8].life= rand()%37+18;
								parts[r>>8].tmp=q;
								parts[r>>8].flags=w;
								parts[r>>8].ctype=e;
								parts[r>>8].temp= rand()%20+6000;
							}
						}
					}
				}
		parts[i].type=PT_NONE;
	}
	return 0;
}
