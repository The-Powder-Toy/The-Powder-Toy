#include <element.h>

int update_PCLN(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if ((r>>8)>=NPART || !r)
					continue;
				if ((r&0xFF)==PT_SPRK)
				{
					if (parts[r>>8].ctype==PT_PSCN)
						parts[i].life = 10;
					else if (parts[r>>8].ctype==PT_NSCN)
						parts[i].life = 9;
				}
				if ((r&0xFF)==PT_PCLN)
				{
					if (parts[i].life==10&&parts[r>>8].life<10&&parts[r>>8].life>0)
						parts[i].life = 9;
					else if (parts[i].life==0&&parts[r>>8].life==10)
						parts[i].life = 10;
				}
			}
	if (!parts[i].ctype)
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES &&
				        pmap[y+ry][x+rx] &&
				        (pmap[y+ry][x+rx]&0xFF)!=PT_CLNE &&
				        (pmap[y+ry][x+rx]&0xFF)!=PT_PCLN &&
				        (pmap[y+ry][x+rx]&0xFF)!=PT_BCLN &&
				        (pmap[y+ry][x+rx]&0xFF)!=PT_SPRK &&
				        (pmap[y+ry][x+rx]&0xFF)!=PT_NSCN &&
				        (pmap[y+ry][x+rx]&0xFF)!=PT_PSCN &&
				        (pmap[y+ry][x+rx]&0xFF)!=PT_STKM &&
				        (pmap[y+ry][x+rx]&0xFF)!=PT_STKM2 &&
				        (pmap[y+ry][x+rx]&0xFF)!=0xFF)
					parts[i].ctype = pmap[y+ry][x+rx]&0xFF;
	if (parts[i].ctype && parts[i].life==10) {
		if (parts[i].ctype==PT_PHOT) {//create photons a different way
			for (rx=-1; rx<2; rx++) {
				for (ry=-1; ry<2; ry++) {
					int r = create_part(-1, x+rx, y+ry, parts[i].ctype);
					if (r!=-1) {
						parts[r].vx = rx*3;
						parts[r].vy = ry*3;
					}
				}
			}
		}
		else if (ptypes[parts[i].ctype].properties&PROP_LIFE) {//create life a different way
			for (rx=-1; rx<2; rx++) {
				for (ry=-1; ry<2; ry++) {
					create_part(-1, x+rx, y+ry, parts[i].ctype);
				}
			}
		} else {
			create_part(-1, x+rand()%3-1, y+rand()%3-1, parts[i].ctype);
		}
	}
	return 0;
}
