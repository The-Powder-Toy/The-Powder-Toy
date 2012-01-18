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
				if (!r)
					continue;
				// wireless[][0] - whether channel is active on this frame
				// wireless[][1] - whether channel should be active on next frame
				if (wireless[parts[i].tmp][0])
				{
					if (((r&0xFF)==PT_NSCN||(r&0xFF)==PT_PSCN||(r&0xFF)==PT_INWR)&&parts[r>>8].life==0 && wireless[parts[i].tmp][0])
					{
						parts[r>>8].ctype = r&0xFF;
						part_change_type(r>>8,x+rx,y+ry,PT_SPRK);
						parts[r>>8].life = 4;
					}
				}
				else
				{
					if ((r&0xFF)==PT_SPRK && parts[r>>8].ctype!=PT_NSCN && parts[r>>8].life>=3)
					{
						wireless[parts[i].tmp][1] = 1;
						ISWIRE = 2;
					}
				}
			}
	return 0;
}
