#include <element.h>

int update_FWRK(UPDATE_FUNC_ARGS) {
	int r, rx, ry, np;
	if ((parts[i].temp>400&&(9+parts[i].temp/40)>rand()%100000&&parts[i].life==0&&!pmap[y-1][x])||parts[i].ctype==PT_DUST)
	{
		np = create_part(-1, x , y-1 , PT_FWRK);
		if (np!=-1)
		{
			parts[r>>8].vy = rand()%8-22;
			parts[r>>8].vx = rand()%20-rand()%20;
			parts[r>>8].life=rand()%15+25;
			kill_part(i);
			return 1;
		}
	}
	if (parts[i].life>=45)
		parts[i].life=0;
	if ((parts[i].life<3&&parts[i].life>0)||parts[i].vy>6&&parts[i].life>0)
	{
		int q = (rand()%255+1);
		int w = (rand()%255+1);
		int e = (rand()%255+1);
		for (rx=-1; rx<2; rx++)
			for (ry=-2; ry<3; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					if (5>=rand()%8)
					{
						if (!pmap[y+ry][x+rx])
						{
							np = create_part(-1, x+rx, y+ry , PT_DUST);
							pv[y/CELL][x/CELL] += 2.00f*CFDS;
							if (np!=-1)
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
		kill_part(i);
		return 1;
	}
	return 0;
}
