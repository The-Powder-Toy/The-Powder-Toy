#include <element.h>

int update_CBNW(UPDATE_FUNC_ARGS) {
	int r, rx, ry, oldt;
	oldt = parts[i].tmp;
    if (pv[y/CELL][x/CELL]<=3)
    {
        if(20>(rand()%80000))
    	{
            part_change_type(i,x,y,PT_CO2);
           	parts[i].ctype = 5;
        	pv[y/CELL][x/CELL] += 0.5f;
        }
        else if(pv[y/CELL][x/CELL]<=-0.5)
    	{
            part_change_type(i,x,y,PT_CO2);
           	parts[i].ctype = 5;
        	pv[y/CELL][x/CELL] += 0.5f;
        }
	}
	if (parts[i].tmp>0)
		parts[i].tmp--;
	if(!(rand()%200))
	{
		parts[i].tmp2 = rand()%40;
	} else if(parts[i].tmp2!=20) {
		parts[i].tmp2 -= (parts[i].tmp2>20)?1:-1;
	}
	if(oldt==1)
	{
		//Explode
		if(rand()%4)
		{
            part_change_type(i,x,y,PT_CO2);
           	parts[i].ctype = 5;
        	pv[y/CELL][x/CELL] += 0.2f;
		}
	}
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (ptypes[r&0xFF].properties&TYPE_PART && parts[i].tmp == 0 && 1>(rand()%250))
				{
					//Start explode
					parts[i].tmp = rand()%25;//(rand()%100)+50;
				}
				else if(ptypes[r&0xFF].properties&TYPE_SOLID && (r&0xFF)!=PT_DMND && (r&0xFF)!=PT_GLAS && parts[i].tmp == 0 && (2-pv[y/CELL][x/CELL])>(rand()%20000))
				{
					if(rand()%2)
					{
                        part_change_type(i,x,y,PT_CO2);
                       	parts[i].ctype = 5;
                    	pv[y/CELL][x/CELL] += 0.2f;
					}
				}
				if ((r&0xFF)==PT_CBNW)
				{
					if(!parts[i].tmp && parts[r>>8].tmp)
					{
						parts[i].tmp = parts[r>>8].tmp;
						if((r>>8)>i) //If the other particle hasn't been life updated
							parts[i].tmp--;
					}
					else if(parts[i].tmp && !parts[r>>8].tmp)
					{
						parts[r>>8].tmp = parts[i].tmp;
						if((r>>8)>i) //If the other particle hasn't been life updated
							parts[r>>8].tmp++;
					}
				}
				if (((r&0xFF)==PT_RBDM||(r&0xFF)==PT_LRBD) && (legacy_enable||parts[i].temp>(273.15f+12.0f)) && 1>(rand()%500))
				{
					part_change_type(i,x,y,PT_FIRE);
					parts[i].life = 4;
					parts[i].ctype = PT_WATR;
				}
				if ((r&0xFF)==PT_FIRE && parts[r>>8].ctype!=PT_WATR){
					kill_part(r>>8);
						if(1>(rand()%150)){
							kill_part(i);
							return 1;
						}
				}
			}
	return 0;
}
