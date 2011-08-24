#include <element.h>

int update_THDR(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((ptypes[r&0xFF].properties&PROP_CONDUCTS) && parts[r>>8].life==0 && !((r&0xFF)==PT_WATR||(r&0xFF)==PT_SLTW) && parts[r>>8].ctype!=PT_SPRK)
				{
					parts[i].type = PT_NONE;
					parts[r>>8].ctype = parts[r>>8].type;
					part_change_type(r>>8,x+rx,y+ry,PT_SPRK);
					parts[r>>8].life = 4;
				}
				else if ((r&0xFF)!=PT_CLNE&&(r&0xFF)!=PT_THDR&&(r&0xFF)!=PT_SPRK&&(r&0xFF)!=PT_DMND&&(r&0xFF)!=PT_FIRE&&(r&0xFF)!=PT_NEUT&&(r&0xFF)!=PT_PHOT&&(r&0xFF))
				{
					pv[y/CELL][x/CELL] += 100.0f;
					if (legacy_enable&&1>(rand()%200))
					{
						parts[i].life = rand()%50+120;
						part_change_type(i,x,y,PT_FIRE);
					}
					else
					{
						parts[i].type = PT_NONE;
					}
				}
			}
	if (parts[i].type==PT_NONE) {
		kill_part(i);
		return 1;
	}
	return 0;
}
