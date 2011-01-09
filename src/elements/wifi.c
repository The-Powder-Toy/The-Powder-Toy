#include <powder.h>

int update_WIFI(UPDATE_FUNC_ARGS) {
	int r, rx, ry, temp, temprange = 100;
	for ( temp = 0; temp < MAX_TEMP; temp += temprange)
		if (parts[i].temp-73.15>temp&&parts[i].temp-73.15 <temp+temprange)
			parts[i].tmp = temp/100;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if ((r>>8)>=NPART || !r)
					continue;
				if (wireless[parts[i].tmp][0])
				{
					if ((r&0xFF)==PT_NSCN&&parts[r>>8].life==0 && wireless[parts[i].tmp][0])
					{
						part_change_type(r>>8,x+rx,y+ry,PT_SPRK);
						parts[r>>8].ctype = PT_NSCN;
						parts[r>>8].life = 4;
					}
					else if ((r&0xFF)==PT_PSCN&&parts[r>>8].life==0 && wireless[parts[i].tmp][0])
					{
						part_change_type(r>>8,x+rx,y+ry,PT_SPRK);
						parts[r>>8].ctype = PT_PSCN;
						parts[r>>8].life = 4;
					}
					else if ((r&0xFF)==PT_INWR&&parts[r>>8].life==0 && wireless[parts[i].tmp][0])
					{
						part_change_type(r>>8,x+rx,y+ry,PT_SPRK);
						parts[r>>8].ctype = PT_INWR;
						parts[r>>8].life = 4;
					}
				}
				else
				{
					if ((r&0xFF)==PT_SPRK && parts[r>>8].ctype!=PT_NSCN && parts[r>>8].life>=3)
					{
						//parts[r>>8].type = parts[r>>8].ctype;
						wireless[parts[i].tmp][0] = 1;
						wireless[parts[i].tmp][1] = 1;
						ISWIRE = 1;
					}
				}
			}
	return 0;
}
