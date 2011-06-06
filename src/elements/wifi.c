#include <element.h>

int update_WIFI(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	parts[i].tmp = (int)((parts[i].temp-73.15f)/100+1);
	if (parts[i].tmp>=CHANNELS) parts[i].tmp = CHANNELS-1;
	else if (parts[i].tmp<0) parts[i].tmp = 0;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if ((r>>PS)>=NPART || !r)
					continue;
				if (wireless[parts[i].tmp][0])
				{
					if ((r&TYPE==PT_NSCN||r&TYPE==PT_PSCN||r&TYPE==PT_INWR)&&parts[r>>PS].life==0 && wireless[parts[i].tmp][0])
					{
						parts[r>>PS].ctype = r&TYPE;
						part_change_type(r>>PS,x+rx,y+ry,PT_SPRK);
						parts[r>>PS].life = 4;
					}
				}
				else
				{
					if (r&TYPE==PT_SPRK && parts[r>>PS].ctype!=PT_NSCN && parts[r>>PS].life>=3)
					{
						wireless[parts[i].tmp][0] = 1;
						wireless[parts[i].tmp][1] = 1;
						ISWIRE = 1;
					}
				}
			}
	return 0;
}
