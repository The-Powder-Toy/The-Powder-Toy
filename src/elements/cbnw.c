#include <element.h>

int update_CBNW(UPDATE_FUNC_ARGS) {
	int r, rx, ry, oldt;
	oldt = parts[i].tmp;
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
		if(!(rand()%2))
		{
			part_change_type(i,x,y,PT_WATR);
		} else {
			pv[y/CELL][x/CELL] += 0.5f;
			part_change_type(i,x,y,PT_CO2);
		}
	}
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if ((r>>8)>=NPART || !r)
					continue;
				if ((r&0xFF)==PT_SALT && parts[i].tmp == 0 && 1>(rand()%250))
				{
					//Start explode
					parts[i].tmp = rand()%50;//(rand()%100)+50;
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
				}
				if ((r&0xFF)==PT_FIRE){
					kill_part(r>>8);
						if(1>(rand()%150)){
							kill_part(i);
							return 1;
						}
				}
				/*if ((r&0xFF)==PT_CNCT && 1>(rand()%500))	Concrete+Water to paste, not very popular
				{
					part_change_type(i,x,y,PT_PSTE);
					kill_part(r>>8);
				}*/
			}
	return 0;
}
