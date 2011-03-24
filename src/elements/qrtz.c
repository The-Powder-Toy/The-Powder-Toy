#include <element.h>

int update_QRTZ(UPDATE_FUNC_ARGS) {
	int r, tmp, trade, rx, ry, np;
	parts[i].pavg[0] = parts[i].pavg[1];
	parts[i].pavg[1] = pv[y/CELL][x/CELL];
	if (parts[i].pavg[1]-parts[i].pavg[0] > 0.05*(parts[i].temp/3) || parts[i].pavg[1]-parts[i].pavg[0] < -0.05*(parts[i].temp/3))
	{
		part_change_type(i,x,y,PT_PQRT);
	}
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if ((r>>8)>=NPART || !r)
					continue;
				else if ((r&0xFF)==PT_SLTW && (1>rand()%2500))
				{
					kill_part(r>>8);
					parts[i].life ++;
				}
			}
	if (parts[i].life>0)
	{
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if ((r>>8)>=NPART || r || parts[i].life==0) continue;
					np = create_part(-1,x+rx,y+ry,PT_QRTZ);
					if (np<0) continue;
					parts[np].tmp = parts[i].tmp;
					parts[i].life = 0;
				}
	}
	for ( trade = 0; trade<9; trade ++)
	{
		rx = rand()%5-2;
		ry = rand()%5-2;
		if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
		{
			r = pmap[y+ry][x+rx];
			if ((r>>8)>=NPART || !r)
				continue;
			if ((r&0xFF)==PT_QRTZ&&(parts[i].life>parts[r>>8].life)&&parts[i].life>0)//diffusion
			{
				tmp = parts[i].life - parts[r>>8].life;
				if (tmp ==1)
				{
					parts[r>>8].life ++;
					parts[i].life --;
					trade = 9;
				}
				if (tmp>0)
				{
					parts[r>>8].life += tmp/2;
					parts[i].life -= tmp/2;
					trade = 9;
				}
			}
		}
	}
	return 0;
}
