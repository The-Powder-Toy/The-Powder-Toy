#include <element.h>

int update_IGNT(UPDATE_FUNC_ARGS) {
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
					else if ((r&0xFF)==PT_SPRK || (r&0xFF)==PT_LIGH || ((r&0xFF)==PT_IGNT && parts[r>>8].life==1))
					{
						parts[i].tmp = 1;
					}
				}
	}
	else if(parts[i].life > 0)
	{
		if(rand()%3)
		{
			int nb = create_part(-1, x+rand()%3-1, y+rand()%3-1, PT_BOMB);
			if (nb!=-1) {
				parts[nb].tmp = 1;
				parts[nb].life = 30;
				parts[nb].vx = rand()%20-10;
				parts[nb].vy = rand()%20-10;
			}
		}
		else
		{
			create_part(-1, x+rand()%3-1, y+rand()%3-1, PT_FIRE);
		}
		parts[i].life--;
	}
	return 0;
}
