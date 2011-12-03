#include <element.h>

int update_BANG(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	if(parts[i].tmp==0)
	{
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((r&0xFF)==PT_FIRE || (r&0xFF)==PT_PLSM)
					{
						parts[i].tmp = 1;
					}
					else if ((r&0xFF)==PT_SPRK || (r&0xFF)==PT_LIGH)
					{
						parts[i].tmp = 1;
					}
				}
	
	}
	else if(parts[i].tmp==1)
	{
		int tempvalue = 2;
		flood_prop(x, y, offsetof(particle, tmp), &tempvalue, 0);
	}
	else if(parts[i].tmp==2)
	{
		parts[i].tmp = 3;
	}
	else if(parts[i].tmp>=3)
	{
		//Explode!!
		pv[y/CELL][x/CELL] += 0.5f;
		if(!(rand()%3))
		{
			if(!(rand()%2))
			{
				parts[i].tmp = 0;
				create_part(i, x, y, PT_FIRE);
			}
			else
			{
				parts[i].tmp = 0;
				create_part(i, x, y, PT_SMKE);
			}
		}
		else
		{
			if(!(rand()%15))
			{
				int nb = create_part(i, x, y, PT_BOMB);
				if (nb!=-1) {
					parts[nb].tmp = 1;
					parts[nb].life = 50;
					//parts[nb].temp = MAX_TEMP;
					parts[nb].vx = rand()%20-10;
					parts[nb].vy = rand()%20-10;
				}
			}
			else
			{
				kill_part(i);
			}
		}
		return 1;
	}
	return 0;
}
