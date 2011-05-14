#include <element.h>

int update_PUMP(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	if (parts[i].life>0 && parts[i].life!=10)
		parts[i].life--;
	if (parts[i].life==10)
	{
		if (parts[i].temp>=256.0+273.15)
			parts[i].temp=256.0+273.15;
		if (parts[i].temp<= -256.0+273.15)
			parts[i].temp = -256.0+273.15;

		if (pv[y/CELL][x/CELL]<(parts[i].temp-273.15))
			pv[y/CELL][x/CELL] += 0.1f*((parts[i].temp-273.15)-pv[y/CELL][x/CELL]);
		if (y+CELL<YRES && pv[y/CELL+1][x/CELL]<(parts[i].temp-273.15))
			pv[y/CELL+1][x/CELL] += 0.1f*((parts[i].temp-273.15)-pv[y/CELL+1][x/CELL]);
		if (x+CELL<XRES)
		{
			pv[y/CELL][x/CELL+1] += 0.1f*((parts[i].temp-273.15)-pv[y/CELL][x/CELL+1]);
			if (y+CELL<YRES)
				pv[y/CELL+1][x/CELL+1] += 0.1f*((parts[i].temp-273.15)-pv[y/CELL+1][x/CELL+1]);
		}
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if ((r>>8)>=NPART || !r)
						continue;
					if ((r&0xFF)==PT_PUMP)
					{
						if (parts[r>>8].life<10&&parts[r>>8].life>0)
							parts[i].life = 9;
						else if (parts[r>>8].life==0)
							parts[r>>8].life = 10;
					}
				}
	}
	return 0;
}
