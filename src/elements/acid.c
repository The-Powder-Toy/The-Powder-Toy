#include <element.h>

int update_ACID(UPDATE_FUNC_ARGS) {
	int r,rx,ry;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if ((r>>PS)>=NPART || !r)
					continue;
				if ((r&TYPE)!=PT_ACID)
				{
					if ((r&TYPE)==PT_PLEX || (r&TYPE)==PT_NITR || (r&TYPE)==PT_GUNP || (r&TYPE)==PT_RBDM || (r&TYPE)==PT_LRBD)
					{
						part_change_type(i,x,y,PT_FIRE);
						part_change_type(r>>PS,x+rx,y+ry,PT_FIRE);
						parts[i].life = 4;
						parts[r>>PS].life = 4;
					}
					else if (((r&TYPE)!=PT_CLNE && (r&TYPE)!=PT_ACLOUD && (r&TYPE)!=PT_PCLN && (r&TYPE)!=PT_BSHL && (r&TYPE)!=PT_GSCL && (r&TYPE)!=PT_LQCL && (r&TYPE)!=PT_PDCL && ptypes[r&TYPE].hardness>(rand()%1000))&&parts[i].life>=50)
					{
						if (parts_avg(i, r>>PS,PT_GLAS)!= PT_GLAS)//GLAS protects stuff from acid
						{
							parts[i].life--;
							kill_part(r>>PS);
						}
					}
					else if (parts[i].life<=50)
					{
						kill_part(i);
						return 1;
					}
				}
			}
	return 0;
}
