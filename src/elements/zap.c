#include <element.h>

int update_ZAP(UPDATE_FUNC_ARGS) {
	int rx,ry,r;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if ((r>>8)>=NPART || !r)
					continue;
				if ((ptypes[r&0xFF].properties&PROP_CONDUCTS) && parts[r>>8].life==0 && !((r&0xFF)==PT_WATR||(r&0xFF)==PT_SLTW) && parts[r>>8].ctype!=PT_SPRK)
				{
					parts[i].type = PT_NONE;
					parts[r>>8].ctype = parts[r>>8].type;
					part_change_type(r>>8,x+rx,y+ry,PT_SPRK);
					parts[r>>8].life = 4;
				}
            
			}
    return 0;
}
