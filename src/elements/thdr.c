#include <element.h>

int update_THDR(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if ((r>>PS)>=NPART || !r)
					continue;
				if ((ptypes[r&TYPE].properties&PROP_CONDUCTS) && parts[r>>PS].life==0 && !((r&TYPE)==PT_WATR||(r&TYPE)==PT_SLTW) && parts[r>>PS].ctype!=PT_SPRK)
				{
					kill_part(i);
					parts[r>>PS].ctype = parts[r>>PS].type;
					part_change_type(r>>PS,x+rx,y+ry,PT_SPRK);
					parts[r>>PS].life = 4;
				}
				else if ((r&TYPE)!=PT_CLNE&&(r&TYPE)!=PT_THDR&&(r&TYPE)!=PT_SPRK&&(r&TYPE)!=PT_DMND&&(r&TYPE)!=PT_C0&&(r&TYPE)!=PT_FIRE&&(r&TYPE)!=PT_BSHL&&(r&TYPE)!=PT_PDCL&&(r&TYPE)!=PT_GSCL&&(r&TYPE)!=PT_LQCL&&(r&TYPE)!=PT_NEUT&&(r&TYPE)!=PT_PHOT&&(r&TYPE))
				{
					pv[y/CELL][x/CELL] += 100.0f;
					if (legacy_enable&&1>(rand()%200))
					{
						parts[i].life = rand()%50+120;
						part_change_type(i,x,y,PT_FIRE);
					}
					else
					{
						kill_part(i);
					}
				}
			}
	if (parts[i].type==PT_NONE) {
		kill_part(i);
		return 1;
	}
	return 0;
}
