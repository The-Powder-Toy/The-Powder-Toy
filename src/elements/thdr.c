#include <element.h>

int update_THDR(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	int self = parts[i].type;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if ((r>>PS)>=NPART || !r)
					continue;
				if ((parts[r>>PS].properties&PROP_CONDUCTS) && parts[r>>PS].life==0 && !(parts[r>>PS].type==PT_WATR||parts[r>>PS].type==PT_SLTW) && parts[r>>PS].ctype!=PT_SPRK)
				{
					parts[i].type = PT_NONE;
					parts[r>>PS].ctype = parts[r>>PS].type;
					part_change_type(r>>PS,x+rx,y+ry,PT_SPRK);
					parts[r>>PS].life = 4;
				}
				else if (parts[r>>PS].type!=PT_CLNE&&parts[r>>PS].type!=self&&parts[r>>PS].type!=PT_SPRK&&parts[r>>PS].type!=PT_DMND&&parts[r>>PS].type!=PT_FIRE&&parts[r>>PS].type!=PT_NEUT&&parts[r>>PS].type!=PT_PHOT&&parts[r>>PS].type)
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
