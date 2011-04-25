#include <element.h>

int update_CNVR(UPDATE_FUNC_ARGS) {
	if (!parts[i].ctype)
	{
		int rx, ry;
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES &&
				        pmap[y+ry][x+rx] &&
				        (pmap[y+ry][x+rx]&0xFF)!=PT_CLNE &&
                        (pmap[y+ry][x+rx]&0xFF)!=PT_CNVR &&
				        (pmap[y+ry][x+rx]&0xFF)!=PT_BCLN &&
                        (pmap[y+ry][x+rx]&0xFF)!=PT_PDCL &&
                        (pmap[y+ry][x+rx]&0xFF)!=PT_GSCL &&
                        (pmap[y+ry][x+rx]&0xFF)!=PT_LQCL &&
				        (pmap[y+ry][x+rx]&0xFF)!=PT_PCLN &&
				        (pmap[y+ry][x+rx]&0xFF)!=PT_STKM &&
				        (pmap[y+ry][x+rx]&0xFF)!=PT_STKM2 &&
				        (pmap[y+ry][x+rx]&0xFF)!=0xFF)
                        parts[i].ctype = pmap[y+ry][x+rx]&0xFF;
	}
	else {
        int r, rx, ry;
        for (rx=-2; rx<3; rx++)
            for (ry=-2; ry<3; ry++)
                if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
                {
                    r = pmap[y+ry][x+rx];
                    if ((r>>8)>=NPART || !r)
                        continue;
                    if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES &&
				        pmap[y+ry][x+rx] &&
				        (pmap[y+ry][x+rx]&0xFF)!=PT_CLNE &&
                        (pmap[y+ry][x+rx]&0xFF)!=PT_CNVR &&
				        (pmap[y+ry][x+rx]&0xFF)!=PT_BCLN &&
                        (pmap[y+ry][x+rx]&0xFF)!=PT_PDCL &&
                        (pmap[y+ry][x+rx]&0xFF)!=PT_GSCL &&
                        (pmap[y+ry][x+rx]&0xFF)!=PT_LQCL &&
				        (pmap[y+ry][x+rx]&0xFF)!=PT_PCLN &&
				        (pmap[y+ry][x+rx]&0xFF)!=PT_STKM &&
				        (pmap[y+ry][x+rx]&0xFF)!=PT_STKM2 &&
				        (pmap[y+ry][x+rx]&0xFF)!=0xFF){
                            parts[r>>8].type = parts[i].ctype;
                    }
                    if ((r&0xFF)==PT_CNVR){
                        parts[r>>8].ctype=parts[i].ctype;
                    }
                }
	}
	return 0;
}
