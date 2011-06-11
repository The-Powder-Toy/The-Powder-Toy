#include <element.h>

int update_PCLN(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if ((r>>PS)>=NPART || !r)
					continue;
				if ((r&TYPE)==PT_SPRK)
				{
					if (parts[r>>PS].ctype==PT_PSCN)
						parts[i].life = 10;
					else if (parts[r>>PS].ctype==PT_NSCN)
						parts[i].life = 9;
				}
				if ((r&TYPE)==PT_PCLN)
				{
					if (parts[i].life==10&&parts[r>>PS].life<10&&parts[r>>PS].life>0)
						parts[i].life = 9;
					else if (parts[i].life==0&&parts[r>>PS].life==10)
						parts[i].life = 10;
				}
			}
	if (!parts[i].ctype)
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES)
 	        {
 	          r = photons[y+ry][x+rx];
 	          if (!r || (r>>PS)>=NPART)
 	            r = pmap[y+ry][x+rx];
 	          if (!r || (r>>PS)>=NPART)
 	            continue;
 	          if ((r&TYPE)!=PT_CLNE && (r&TYPE)!=PT_PCLN &&
 	                (r&TYPE)!=PT_BCLN &&  (r&TYPE)!=PT_SPRK &&
 	                (r&TYPE)!=PT_NSCN && (r&TYPE)!=PT_PSCN &&
 	                (r&TYPE)!=PT_STKM && (r&TYPE)!=PT_STKM2 && (r&TYPE)!=PT_PDCL &&
                    (r&TYPE)!=PT_GSCL && (r&TYPE)!=PT_LQCL &&
 	                (r&TYPE)<PT_NUM)
 	          parts[i].ctype = r&TYPE;
 	        }
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
