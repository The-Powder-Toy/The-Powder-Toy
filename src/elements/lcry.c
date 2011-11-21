#include <element.h>

int update_LCRY(UPDATE_FUNC_ARGS)
{
	int r, rx, ry;
	if(parts[i].tmp==1 || parts[i].tmp==0)
	{
		if(parts[i].tmp==1)
		{
			if(parts[i].life<=0)
				parts[i].tmp = 0;
			else
			{
				parts[i].life-=2;
				if(parts[i].life < 0)
					parts[i].life = 0;
				parts[i].tmp2 = parts[i].life;
			}	
		}
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((r&0xFF)==PT_LCRY && parts[r>>8].tmp == 3)
					{
						parts[r>>8].tmp = 1;
					}
				}
	}
	else if(parts[i].tmp==2 || parts[i].tmp==3)
	{
		if(parts[i].tmp==2)
		{
			if(parts[i].life>=10)
				parts[i].tmp = 3;
			else
			{
				parts[i].life+=2;
				if(parts[i].life > 10)
					parts[i].life = 10;
				parts[i].tmp2 = parts[i].life;
			}
		}
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((r&0xFF)==PT_LCRY && parts[r>>8].tmp == 0)
					{
						parts[r>>8].tmp = 2;
					}
				}
	}
	return 0;
}
