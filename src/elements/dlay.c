#include <element.h>

int update_DLAY(UPDATE_FUNC_ARGS) {
	int r, rx, ry, oldl;
	oldl = parts[i].life;
	if (parts[i].life>0)
		parts[i].life--;
	if (parts[i].tmp&0xFF00)
		parts[i].tmp-=0x0100;
	//if (parts[i].life==1)
	//{
		if (parts[i].temp>=256.0+273.15)
			parts[i].temp=256.0+273.15;
		if (parts[i].temp<= -256.0+273.15)
			parts[i].temp = -256.0+273.15;

		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if ((r>>8)>=NPART || !r || parts_avg(r>>8, i, PT_INSL)==PT_INSL)
						continue;
					if ((r&0xFF)==PT_SPRK && parts[i].life==0 && !(parts[i].tmp&0xFF00) &&
						(parts[r>>8].ctype==PT_PSCN || parts[r>>8].ctype==PT_NSCN))
					{
						parts[i].tmp = ((rx+2)<<4)|(ry+2);
						parts[i].life = (int)(parts[i].temp-273.15);
					}
					else if (oldl==1 && (parts[i].tmp&0xFF)!=(((rx+2)<<4)|(ry+2))) {
						if ((r&0xFF)==PT_PSCN || (r&0xFF)==PT_NSCN || ((r&0xFF)==PT_SWCH && parts[r>>8].life==10)) {
							parts[i].tmp |= 0x0500;
							parts[r>>8].ctype = r&0xFF;
							part_change_type(r>>8, x+rx, y+ry, PT_SPRK);
							parts[r>>8].life = 4;
						}
						else if ((r&0xFF)==PT_DLAY) {
							parts[r>>8].tmp = ((-rx+2)<<4)|(-ry+2);
							parts[r>>8].life = (int)(parts[r>>8].temp-273.15);
						}
					}
				}
	//}
	return 0;
}
