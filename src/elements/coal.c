#include <element.h>

int update_COAL(UPDATE_FUNC_ARGS) {
	int r, rx, ry, trade, temp;
	if (parts[i].life<=0) {
		create_part(i, x, y, PT_FIRE);
		return 1;
	} else if (parts[i].life < 100) {
		parts[i].life--;
		create_part(-1, x+rand()%3-1, y+rand()%3-1, PT_FIRE);
	}
	if ((pv[y/CELL][x/CELL] > 4.3f)&&parts[i].tmp>40)
		parts[i].tmp=39;
	else if (parts[i].tmp<40&&parts[i].tmp>0)
		parts[i].tmp--;
	else if (parts[i].tmp<=0) {
		create_part(i, x, y, PT_BCOL);
		return 1;
	}
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (((r&0xFF)==PT_FIRE || (r&0xFF)==PT_PLSM) && 1>(rand()%500))
				{
					if (parts[i].life>100) {
						parts[i].life = 99;
					}
				}
				if ((r&0xFF)==PT_LAVA && 1>(rand()%500))
				{
					if (parts[r>>8].ctype == PT_IRON) {
						parts[r>>8].ctype = PT_METL;
						kill_part(i);
                                                return 1;
					}
				}
			}
	/*if(100-parts[i].life > parts[i].tmp2)
		parts[i].tmp2 = 100-parts[i].life;
	if(parts[i].tmp2 < 0) parts[i].tmp2 = 0;
	for ( trade = 0; trade<4; trade ++)
	{
		rx = rand()%5-2;
		ry = rand()%5-2;
		if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
		{
			r = pmap[y+ry][x+rx];
			if (!r)
				continue;
			if (((r&0xFF)==PT_COAL || (r&0xFF)==PT_BCOL)&&(parts[i].tmp2>parts[r>>8].tmp2)&&parts[i].tmp2>0)//diffusion
			{
				int temp = parts[i].tmp2 - parts[r>>8].tmp2;
				if(temp < 10)
					continue;
				if (temp ==1)
				{
					parts[r>>8].tmp2 ++;
					parts[i].tmp2 --;
				}
				else if (temp>0)
				{
					parts[r>>8].tmp2 += temp/2;
					parts[i].tmp2 -= temp/2;
				}
			}
		}
	}*/
	if(parts[i].temp > parts[i].tmp2)
		parts[i].tmp2 = parts[i].temp;
	return 0;
}
