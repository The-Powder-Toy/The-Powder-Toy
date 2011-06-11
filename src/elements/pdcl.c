#include <element.h>

int update_PDCL(UPDATE_FUNC_ARGS) {
	if (!parts[i].ctype)
	{
		int r, rx, ry;
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
                    (r&TYPE)!=PT_BCLN && (r&TYPE)!=PT_STKM &&
                    (r&TYPE)!=PT_STKM2 && (r&TYPE)!=PT_PDCL &&
                    (r&TYPE)!=PT_GSCL && (r&TYPE)!=PT_LQCL && (r&TYPE)<PT_NUM)
                        parts[i].ctype = parts[r>>PS].type;
                }
	}
	else {
		create_part(-1, x+rand()%3-1, y+rand()%3-1, parts[i].ctype);
	}
	return 0;
}
